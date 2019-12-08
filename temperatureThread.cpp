#include "mbed.h"
#include "temperatureThread.h"
#include "displayThread.h"

#ifdef TARGET_CY8CPROTO_062_4343W
#define THERMISTOR
#endif

#ifdef TARGET_CY8CKIT_062_WIFI_BT
#define TMP36
#endif


static float temperatureF;
static float setPoint = 75.0;

static void readTemp();

typedef enum {
    CMD_setPointDelta,
    CMD_setPoint,

} command_t;


typedef struct {
    command_t cmd;
    float    value;   /* AD result of measured voltage */
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

void temperatureThread()
{

    char buffer[128];
    displaySendUpdateTemp(temperatureF);
    displaySendUpdateSetPoint(setPoint);
    
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
                break;
                case CMD_setPoint:
                    setPoint = message->value;
                    displaySendUpdateSetPoint(setPoint);
                break;

            }
            mpool.free(message);

        }
        else
        {
            readTemp();

            // Control the HVAC system with +- 0.5 degree of Hystersis
            if(temperatureF < setPoint - 0.5)
                displaySendUpdateMode(-1.0);
            else if (temperatureF > setPoint + 0.5)
                displaySendUpdateMode(1.0);
            else
                displaySendUpdateMode(0.0);

            displaySendUpdateTemp(temperatureF); 
        }
    }

}


#ifdef THERMISTOR
static DigitalOut thermVDD(P10_3,1);
static DigitalOut thermGND(P10_0,0);
static AnalogIn thermOut(P10_1);

static void readTemp()
{
    float refVoltage = thermOut.read() * 2.4; // Range of ADC 0->2*Vref
    float refCurrent = refVoltage  / 10000.0; // 10k Reference Resistor
    float thermVoltage = 3.3 - refVoltage;    // Assume supply voltage is 3.3v
    float thermResistance = thermVoltage / refCurrent; 
    float logrT = (float32_t)log((float64_t)thermResistance);

    /* Calculate temperature from the resistance of thermistor using Steinhart-Hart Equation */
    float stEqn = (float32_t)((0.0009032679) + ((0.000248772) * logrT) + 
                             ((2.041094E-07) * pow((float64)logrT, (float32)3)));

    float temperatureC = (float32_t)(((1.0 / stEqn) - 273.15)  + 0.5);
    temperatureF = (temperatureC * 9.0/5.0) + 32;
}
#endif

#ifdef TMP36
static AnalogIn tmp36(A5);
static void readTemp()
{
    float volts;
    
    volts = tmp36.read() * 2.4;
    float temperatureC = 1/.01 * volts - 50.0;
    temperatureF = (temperatureC * 9.0/5.0) + 32;
}
#endif
