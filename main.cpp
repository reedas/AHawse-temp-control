#include "mbed.h"
#include "blinkThread.h"
#include "displayThread.h"
#include "temperatureThread.h"
#include "capsenseThread.h"

Thread blinkThreadHandle;
Thread displayThreadHandle;
Thread temperatureThreadHandle;
Thread capsenseThreadHandle;

int main()
{

    printf("Started System\n");

    blinkThreadHandle.start(blinkThread);
    displayThreadHandle.start(displayThread);
    temperatureThreadHandle.start(temperatureThread);
    capsenseThreadHandle.start(capsenseThread);

}
