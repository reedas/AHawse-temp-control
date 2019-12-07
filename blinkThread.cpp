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
        ThisThread::sleep_for(500);
    }
}
