#include "mbed.h"
#include "temperatureThread.h"
#include "displayThread.h"
#include "awsThread.h"
#include "Dht11.h"
#include "DS1820.h"
//extern void awsSendUpdateLight(f lightLevel);
//extern void awsSendUpdateHumid(int relHumid);

#ifdef TARGET_CY8CPROTO_062_4343W
#define THERMISTOR
#define DHT11Present
//#define DS1820Present
DS1820 ds1820(P10_4);    // substitute D8 with actual mbed pin name connected 1-wire bus
Dht11 humid(P10_3);
#endif

#ifdef TARGET_CY8CKIT_062_WIFI_BT
#define TMP36
#endif

static int currLightLevel = 0;
static int currentRelHumid = 0;
static float temperatureC;
static float setPoint = 20.5;
static float controlMode = 0;
AnalogIn lightLevel(P10_0);
static float relHumid = 0;


static void readTemp();
static void readHumidity();

typedef enum {
    CMD_setPointDelta,
    CMD_setPoint,
    CMD_setMode,

} command_t;


typedef struct {
    command_t cmd;
    float    value;   /* eg ADC result of measured voltage */
} msg_t;


static Queue<msg_t, 32> queue;
static MemoryPool<msg_t, 16> mpool;

void tempSendDeltaSetpointF(float delta)
{
    msg_t *message = mpool.alloc();
    if(message)
    {
        message->cmd = CMD_setPointDelta;
        message->value = delta;
        queue.put(message);
    }
}

void tempSendUpdateSetpointF(float setPoint)
{
    msg_t *message = mpool.alloc();
    if(message)
    {
        message->cmd = CMD_setPoint;
        message->value = setPoint;
        queue.put(message);
    }
}

void modeControlSetMode(float controlMode)
{
      msg_t *message = mpool.alloc();
    if(message)
    {
        message->cmd = CMD_setMode;
        message->value = controlMode;
        queue.put(message);
    }
  
}
void temperatureThread()
{

    char buffer[128];
    static int lastTemp = - 50;
    static float lastControlMode = 0;
    static int lastDelta = 0;
    static int lastLightLevel = 0;
    static int lastRelHumid = 0;
    int checker;
    displaySendUpdateTemp(temperatureC);
    displaySendUpdateHumid(currentRelHumid);
    displaySendUpdateLight(currLightLevel);
    displaySendUpdateSetPoint(setPoint);
    displaySendUpdateMode(0);
    awsSendIPAddress();
    awsSendUpdateSetPoint(setPoint);
    awsSendAnnounce1();
    awsSendAnnounce2();
#ifdef DS1820Present

    ds1820.begin();
#endif
#ifdef DHT11Present

#endif
    while(1)
    {
        osEvent evt = queue.get(200);
        if (evt.status == osEventMessage) {
            msg_t *message = (msg_t*)evt.value.p;
            switch(message->cmd)
            {
                case CMD_setPointDelta:
                    setPoint += message->value;
                    displaySendUpdateSetPoint(setPoint);
                    awsSendUpdateSetPoint(setPoint);
                break;
                case CMD_setPoint:
                    setPoint = message->value;
                    displaySendUpdateSetPoint(setPoint);
                    awsSendUpdateSetPoint(setPoint);
                break;
                case CMD_setMode:
                    controlMode = message->value;
                    displaySendUpdateMode(controlMode);
                    awsSendUpdateMode(controlMode);
                break;
            }
            mpool.free(message);

        }
        else
        {
            readTemp();
            float deltaTemp = setPoint - temperatureC;
            // Control the HVAC system with +- 0.5 degree of Hystersis
            if(temperatureC < setPoint - 0.5) controlMode = -1.0;
            
            else if (temperatureC > setPoint + 0.5) controlMode = 1.0;
           
            else controlMode = 0;
            
            if (controlMode != lastControlMode) {
                modeControlSetMode(controlMode);
                awsSendUpdateDelta(deltaTemp);
                lastControlMode =  controlMode;
            }
            displaySendUpdateTemp(temperatureC); 
            checker = (int) (temperatureC * 10 );
            if ( checker != lastTemp ) {
                awsSendUpdateTemperature(temperatureC);
                awsSendUpdateDelta(deltaTemp);
                lastTemp = checker;
            }
            checker = (int) (deltaTemp * 10);
            if (checker != lastDelta) {
                awsSendUpdateDelta(deltaTemp);
                lastDelta = checker;
            }
 
            currLightLevel = (int)(100 - lightLevel * 100);
            if ( currLightLevel != lastLightLevel ) {
                lastLightLevel = currLightLevel;
                displaySendUpdateLight(currLightLevel); 
                awsSendUpdateLight(currLightLevel);
            }
            readHumidity();
            if ( currentRelHumid != lastRelHumid ) {
                lastRelHumid = currentRelHumid;
                awsSendUpdateHumid( currentRelHumid);
                displaySendUpdateHumid(currentRelHumid); 

            }


        }
    }

}


#ifdef THERMISTOR

/* Reference resistor in series with the thermistor is of 10 KOhm */
#define R_REFERENCE (float)(10000)

/* Beta constant of this thermistor is 3380 Kelvin. See the thermistor
   (NCP18XH103F03RB) data sheet for more details. */
#define B_CONSTANT (float)(3380)

/* Resistance of the thermistor is 10K at 25 degrees C (from data sheet)
   Therefore R0 = 10000 Ohm, and T0 = 298.15 Kelvin, which gives
   R_INFINITY = R0 e^(-B_CONSTANT / T0) = 0.1192855 */
#define R_INFINITY (float)(0.1192855)

/* Zero Kelvin in degree C */
#define ABSOLUTE_ZERO (float)(-273.15)

//static DigitalIn thermVDD(P10_0);  // if wing is detached and powered from 3.3v
//static DigitalIn thermGND(P10_3);  // don't need to control power to thermistor
                                     // freeing up some inputs P10_0, P10_1, P10_3
static AnalogIn thermOut1(P10_2);
// static AnalogIn thermOut2(P10_7);
// static AnalogIn testing(P10_4);

static void readTemp()
{
//    thermVDD = 1;
    static int lcount = 0;
    static float Therm[8];
    float thermValue;
//    uint16_t intThermValue;
    float rThermistor;
    thermValue = thermOut1.read()*2.4/3.3;
    Therm[lcount] = (B_CONSTANT / (logf((R_REFERENCE / ( 1 / thermValue - 1 )) / 
                                               R_INFINITY))) + ABSOLUTE_ZERO;
    lcount = (lcount + 1) & 0x7;
    if (lcount == 0) {
        float TempAverage = 0;
        for (int i = 0; i < 8; i++){ 
            TempAverage += Therm[i]; 
            //printf("%d ", (int)(Therm[i]*10));
        }
//        intThermValue = thermOut1.read_u16();
//        printf(" %d ", intThermValue & 0xfff);

        temperatureC = TempAverage/8;
    }
/*    float refVoltage = thermOut.read() * 2.4; // Range of ADC 0->2*Vref
    float refCurrent = refVoltage  / 10000.0; // 10k Reference Resistor
    float thermVoltage = 3.3 - refVoltage;    // Assume supply voltage is 3.3v
    float thermResistance = thermVoltage / refCurrent; 
    float logrT = (float32_t)log((float64_t)thermResistance);

    // Calculate temperature from the resistance of thermistor using Steinhart-Hart Equation 
    float stEqn = (float32_t)((0.0009032679) + ((0.000248772) * logrT) + 
                             ((2.041094E-07) * pow((float64)logrT, (int)3)));

    temperatureC = (float32_t)(((1.0 / stEqn) - 273.15)  + 0.5);
    float temperatureF = (temperatureC * 9.0/5.0) + 32;
 */
}
#endif

#ifdef TMP36
static AnalogIn tmp36(P10_6);
static void readTemp()
{
    float volts;
    
    volts = tmp36.read() * 2.4;
    temperatureC = 1/.01 * volts - 50.0;
    float temperatureF = (temperatureC * 9.0/5.0) + 32;
}
#endif
#ifdef DHT11Present
static void readHumidity()
{
    if (humid.read() == 0) currentRelHumid = (int)humid.getHumidity();
}

#endif
#ifdef DS1820Present

static void readTemp()
{
//    ds1820.begin();
    float tempDS1820;
    ds1820.startConversion();   // start temperature conversion from analog to digital
    ThisThread::sleep_for(1);                  // let DS1820 complete the temperature conversion
    int result = ds1820.read(tempDS1820); // read temperature from DS1820 and perform cyclic redundancy check (CRC)
    switch(result) {
        case 0:
        temperatureC = tempDS1820;
        break;
        default:
        printf("Ds1820 Error");
    }
 }
#endif