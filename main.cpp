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

Semaphore WiFiSemaphore;

WiFiInterface *wifi;

// wifiStatusCallback
// Changes the display when the wifi status is changed
void wifiStatusCallback(nsapi_event_t status, intptr_t param)
{
    switch(param) {
        case NSAPI_STATUS_LOCAL_UP:
        break;
        case NSAPI_STATUS_GLOBAL_UP:
        break;
        case NSAPI_STATUS_DISCONNECTED:
        WiFiSemaphore.release();
        break;
        case NSAPI_STATUS_CONNECTING:
        break;
        default:
        break;
    }
}

int main()
{

    printf("Started System\n");

    int ret;
    wifi = WiFiInterface::get_default_instance();
    wifi->attach(&wifiStatusCallback);
    while(1)
    {
        do {
            ret = wifi->connect("CYFI_IOT_EXT", "cypresswicedwifi101", NSAPI_SECURITY_WPA_WPA2);
            if (ret != 0) {
            ThisThread::sleep_for(2000); // If for some reason it doesnt work wait 2s and try again
            }
        } while(ret !=0);

        if(blinkThreadHandle.get_state() == Thread::Deleted)
        {

            ntpThreadHandle.start(ntpThread);
            blinkThreadHandle.start(blinkThread);
            displayThreadHandle.start(displayThread);
            temperatureThreadHandle.start(temperatureThread);
            capsenseThreadHandle.start(capsenseThread);
        }
        WiFiSemaphore.acquire();
    }
}
