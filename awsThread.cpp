#include "aws_client.h"
#include "aws_config.h"
#include "awsThread.h"
#include "mbed.h"
#include "temperatureThread.h"

extern WiFiInterface *wifi;
extern int currLightLevel;
extern int currentRelHumid;

typedef enum {
  CMD_sendTemperature,
  CMD_sendIPAddress,
  CMD_sendSetPoint,
  CMD_sendLightLevel,
  CMD_sendRelativeHumidity,
  CMD_sendMode,
  CMD_sendDelta,
  CMD_sendAnnounce1,
  CMD_sendAnnounce2,
} command_t;
typedef struct {
  command_t cmd;
  float value; /* eg ADC result of measured voltage */
} msg_t;
static Queue<msg_t, 16> queue;
static MemoryPool<msg_t, 16> mpool;

void awsSendUpdateTemperature(float temperature) {
  msg_t *message = mpool.alloc();
  if (message) {
    message->cmd = CMD_sendTemperature;
    message->value = temperature;
    queue.put(message);
  }

}
void awsSendAnnounce1(void) {
  msg_t *message = mpool.alloc();
  if (message) {
    message->cmd = CMD_sendAnnounce1;
    message->value = 0;
    queue.put(message);
  }
}
void awsSendAnnounce2(void) {
  msg_t *message = mpool.alloc();
  if (message) {
    message->cmd = CMD_sendAnnounce2;
    message->value = 0;
    queue.put(message);
  }
}
void awsSendIPAddress(void) {
  msg_t *message = mpool.alloc();
  if (message) {
    message->cmd = CMD_sendIPAddress;
    message->value = 0;
    queue.put(message);
  }
}
void awsSendUpdateSetPoint(float setPoint) {
  msg_t *message = mpool.alloc();
  if (message) {
    message->cmd = CMD_sendSetPoint;
    message->value = setPoint;
    queue.put(message);
  }
}
void awsSendUpdateDelta(float delta) {
  msg_t *message = mpool.alloc();
  if (message) {
    message->cmd = CMD_sendDelta;
    message->value = delta;
    queue.put(message);
  }
}
void awsSendUpdateMode(float controlMode) {
  msg_t *message = mpool.alloc();
  if (message) {
    message->cmd = CMD_sendMode;
    message->value = controlMode;
    queue.put(message);
  }
}
void awsSendUpdateLight(int lighLlevel) {
  msg_t *message = mpool.alloc();
  if (message) {
    message->cmd = CMD_sendLightLevel;
    message->value = lighLlevel;
    queue.put(message);
  }
}
void awsSendUpdateHumid(int relHumidity) {
  msg_t *message = mpool.alloc();
  if (message) {
    message->cmd = CMD_sendRelativeHumidity;
    message->value = relHumidity;
    queue.put(message);
  }
}
static void messageArrived(aws_iot_message_t &md) {
  float setPoint;
  aws_message_t &message = md.message;
  sscanf((char *)message.payload, "%f", &setPoint);
  tempSendUpdateSetpointF(setPoint);
}
void awsThread(void) {
  AWSIoTClient client;
  AWSIoTEndpoint *ep = NULL;
  /* Initialize AWS Client library */
  AWSIoTClient AWSClient(wifi, AWSIOT_THING_NAME, SSL_CLIENTKEY_PEM,
                         strlen(SSL_CLIENTKEY_PEM), SSL_CLIENTCERT_PEM,
                         strlen(SSL_CLIENTCERT_PEM));
  aws_connect_params conn_params = {0, 0, NULL, NULL, NULL, NULL, NULL};
  ep = AWSClient.create_endpoint(AWS_TRANSPORT_MQTT_NATIVE,
                                 AWSIOT_ENDPOINT_ADDRESS, 8883, SSL_CA_PEM,
                                 strlen(SSL_CA_PEM));
  /* set MQTT connection parameters */
  conn_params.username = NULL;
  conn_params.password = NULL;
  conn_params.keep_alive = 60;
  conn_params.peer_cn = (uint8_t *)AWSIOT_ENDPOINT_ADDRESS;
  conn_params.client_id = (uint8_t *)AWSIOT_THING_NAME;
  /* connect to an AWS endpoint */
  AWSClient.connect(ep, conn_params);
  AWSClient.subscribe(ep, "setPoint", AWS_QOS_ATMOST_ONCE, messageArrived);
  aws_publish_params publish_params = {AWS_QOS_ATMOST_ONCE};

  bool doPublish = false;
  char buffer[256];
  //  float currentTemp, currentSetPt, currentLightLevel, currentRelHumid;
  while (1) {
    AWSClient.yield(1000);
    while (!queue.empty()) {
      ThisThread::sleep_for(50);  // Messages can be rejected if sent too close

      osEvent evt = queue.get(0);

      if (evt.status == osEventMessage) {
        msg_t *message = (msg_t *)evt.value.p;
        //        printf("%d", message->cmd);
        switch (message->cmd) {
        case CMD_sendTemperature:
          doPublish = true;
          sprintf(buffer,
                  "{\"state\":{\"reported\":{\"currentTemp\":\"%2.1f\"}}}",
                  message->value);
          break;
        case CMD_sendIPAddress:
          doPublish = true;
            sprintf(buffer, "{\"state\":{\"reported\":{\"IPAddress\":\"%s\"}}}",
                     wifi->get_ip_address());
            break;
        case CMD_sendAnnounce1:
          doPublish = true;
            sprintf(buffer, "{\"state\":{\"reported\":{\"I_Am\":\"%s\"}}}", 
                    "The AR Thang");
            break;
        case CMD_sendAnnounce2:
          doPublish = true;
            sprintf(buffer, "{\"state\":{\"desired\":{\"Are_You\": \"%s\"}}}",
                     "AR Thing?");
            break;
        case CMD_sendSetPoint:
          doPublish = true;
          sprintf(buffer, "{\"state\":{\"desired\":{\"setPoint\":\"%2.1f\"}}}",
                  message->value);
          break;
        case CMD_sendDelta:
          doPublish = true;
          sprintf(buffer, "{\"state\":{\"reported\":{\"Delta\":\"%2.1f\"}}}",
                  message->value);
          break;
        case CMD_sendLightLevel:
          doPublish = true;
          sprintf(buffer,
                  "{\"state\":{\"reported\":{\"LightLevel\":\"%d%c\"}}}",
                   (int)message->value, 0x25 );
          break;
        case CMD_sendRelativeHumidity:
          doPublish = true;
          sprintf(buffer,
                  "{\"state\":{\"reported\":{\"RelHumidity\":\"%d%c\"}}}",
                  (int)message->value, 0x25 );
          break;
        case CMD_sendMode:
          doPublish = true;
          char mode[5];
          if ((message->value) >= 1)
            sprintf(mode, "Cool");
          else if ((message->value) <= -1)
            sprintf(mode, "Heat");
          else
            sprintf(mode, "Off");
          sprintf(buffer, "{\"state\":{\"reported\":{\"controlMode\":\"%s\"}}}",
                  mode);
          break;
        }
        mpool.free(message);
      }
      if (doPublish) {

        AWSClient.publish(ep, AWSIOT_TOPIC_UPDATE, buffer,
                          strlen((char *)buffer), publish_params);
      }
      doPublish = false;
    }
  }
}