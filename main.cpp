#include "mbed.h"
#include "blinkThread.h"
#include "displayThread.h"
#include "temperatureThread.h"

Thread blinkThreadHandle;
Thread displayThreadHandle;
Thread temperatureThreadHandle;

int main()
{

    printf("Started System\n");

    blinkThreadHandle.start(blinkThread);
    displayThreadHandle.start(displayThread);
    temperatureThreadHandle.start(temperatureThread);

}
