#include "mbed.h"
#include "blinkThread.h"
#include "displayThread.h"
#include "temperatureThread.h"
#include "capsenseThread.h"
#include "ntpThread.h"

Thread blinkThreadHandle;
Thread displayThreadHandle;
Thread temperatureThreadHandle;
Thread capsenseThreadHandle;
Thread ntpThreadHandle;

WiFiInterface *wifi;

int main()
{

    printf("Started System\n");

    int ret;
    wifi = WiFiInterface::get_default_instance();

    do {
            ret = wifi->connect("CYFI_IOT_EXT", "cypresswicedwifi101", NSAPI_SECURITY_WPA_WPA2);
            if (ret != 0) {
            ThisThread::sleep_for(2000); // If for some reason it doesnt work wait 2s and try again
            }
    } while(ret !=0);


    ntpThreadHandle.start(ntpThread);
    blinkThreadHandle.start(blinkThread);
    displayThreadHandle.start(displayThread);
    temperatureThreadHandle.start(temperatureThread);
    capsenseThreadHandle.start(capsenseThread);
    
}
