#include "mbed.h"
#include "ntp-client/NTPClient.h"

extern WiFiInterface *wifi;


void ntpThread()
{
    NTPClient ntpclient(wifi);
 
    uint32_t sleepTime = 1000 * 60 * 5 ; // 5 minutes
    while(1)
    {

        if(wifi->get_connection_status() == NSAPI_STATUS_GLOBAL_UP)
        {
            time_t timestamp = ntpclient.get_timestamp();
            if (timestamp < 0) {
                sleepTime = 1000 * 10 ; // 10 seconds
            } 
            else 
            {
                set_time(timestamp);
                sleepTime = 1000 * 60 * 5 ; // 5 minutes

            }
        }
        ThisThread::sleep_for(sleepTime); // Goto the NTP server every 5 minutes
    }
}