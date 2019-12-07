#include "mbed.h"
#include "blinkThread.h"

Thread blinkThreadHandle;

int main()
{

    printf("Started System\n");

    blinkThreadHandle.start(blinkThread);
}
