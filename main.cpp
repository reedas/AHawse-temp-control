#include "mbed.h"
#include "blinkThread.h"
#include "displayThread.h"

Thread blinkThreadHandle;
Thread displayThreadHandle;

int main()
{

    printf("Started System\n");

    blinkThreadHandle.start(blinkThread);
    displayThreadHandle.start(displayThread);

}
