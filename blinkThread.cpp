#include "mbed.h"
#include "blinkThread.h"
#include "displayThread.h"

static DigitalOut led1(LED1);

void blinkThread()
{
 
    while (true) 
    {
        led1 = !led1;
        displaySendUpdateTime();
        displaySendUpdateTemp(69.2);
        displaySendUpdateSetPoint(23.5);
        displaySendUpdateMode(-1.0);
        ThisThread::sleep_for(500);
    }
}
