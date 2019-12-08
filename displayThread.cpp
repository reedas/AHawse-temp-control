#include "mbed.h"
#include "displayThread.h"

#ifdef TARGET_CY8CKIT_062_WIFI_BT
#include "GUI.h"
#endif

typedef enum {
    CMD_temperature,
    CMD_setPoint,
    CMD_time,
    CMD_mode,
} command_t;


typedef struct {
    command_t cmd;
    float    value;
} msg_t;


static Queue<msg_t, 32> queue;
static MemoryPool<msg_t, 16> mpool;


void displaySendUpdateTemp(float temperature)
{
    msg_t *message = mpool.alloc();
    if(message)
    {
        message->cmd = CMD_temperature;
        message->value = temperature;
        queue.put(message);
    }
}

void displaySendUpdateTime()
{
    msg_t *message = mpool.alloc();
    if(message)
    {
        message->cmd = CMD_time;
        message->value = 0;
        queue.put(message);
    }
}

void displaySendUpdateSetPoint(float setPoint)
{
    msg_t *message = mpool.alloc();
    if(message)
    {
        message->cmd = CMD_setPoint;
        message->value = setPoint;
        queue.put(message);
    }
}


void displaySendUpdateMode(float mode)
{
    msg_t *message = mpool.alloc();
    if(message)
    {
        message->cmd = CMD_mode;
        message->value = mode;
        queue.put(message);
    }
}

static void displayAtXY(int x, int y,char *buffer)
{
    #ifdef TARGET_CY8CKIT_062_WIFI_BT
    GUI_SetTextAlign(GUI_TA_LEFT);
    GUI_DispStringAt(buffer,(x-1)*8,(y-1)*16);
    #endif
    // row column
    printf("\033[%d;%dH%s",y,x,buffer);
    fflush(stdout);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////

void displayThread()
{
    char buffer[128];

    printf("\033[2J\033[H"); // Clear Screen and go Home
    printf("\033[?25l"); // Turn the cursor off
    fflush(stdout);

    #ifdef TARGET_CY8CKIT_062_WIFI_BT
        GUI_Init();
        GUI_SetColor(GUI_WHITE);
        GUI_SetBkColor(GUI_BLACK);
        GUI_SetFont(GUI_FONT_8X16_1);
    #endif

    while(1)
    {
        osEvent evt = queue.get();
        if (evt.status == osEventMessage) {
            msg_t *message = (msg_t*)evt.value.p;
            switch(message->cmd)
            {
                case CMD_temperature:
                    sprintf(buffer,"Temperature = %2.1fF",message->value);
                    displayAtXY(1, 1, buffer);
                break;
                case CMD_setPoint:
                    sprintf(buffer,"Set Point = %2.1fF",message->value);
                    displayAtXY(1, 2, buffer);
                break;
                case CMD_time:
                    time_t rawtime;
                    struct tm * timeinfo;
                    time (&rawtime);
                    rawtime = rawtime - (5*60*60); // UTC - 4hours ... serious hack which only works in winter
                    timeinfo = localtime (&rawtime);
                    strftime (buffer,sizeof(buffer),"%r",timeinfo);
                    displayAtXY(1,3, buffer);
                break;
                case CMD_mode:
                    if(message->value == 0.0)
                        sprintf(buffer,"Mode = Off ");
                    else if (message->value < 0.0)
                        sprintf(buffer,"Mode = Heat");
                    else
                        sprintf(buffer,"Mode = Cool");
                    displayAtXY(1, 4, buffer);
                break;

            }
            mpool.free(message);

        }
    }
}
