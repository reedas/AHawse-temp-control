#include "aws_client.h"
#include "aws_config.h"
#include "mbed.h"
#include "temperatureThread.h"

extern WiFiInterface *wifi;

typedef enum {
  CMD_sendTemperature,
} command_t;
typedef struct {
  command_t cmd;
  float value; /* AD result of measured voltage */
} msg_t;
static Queue<msg_t, 32> queue;
static MemoryPool<msg_t, 16> mpool;
void awsSendUpdateTemperature(float temperature) {
  msg_t *message = mpool.alloc();
  if (message) {
    message->cmd = CMD_sendTemperature;
    message->value = temperature;
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
  float currentTemp;
  while (1) {
    AWSClient.yield(1000);
    while (!queue.empty()) {
      osEvent evt = queue.get(0);
      if (evt.status == osEventMessage) {
        msg_t *message = (msg_t *)evt.value.p;
        switch (message->cmd) {
        case CMD_sendTemperature:
          doPublish = true;
          currentTemp = message->value;
          break;
        }
        mpool.free(message);
      }
    }
    if (doPublish) {
      char buffer[128];
      sprintf(buffer, "{\"state\":{\"reported\":{\"currentTemp\":\"%2.1f\"}}}",
              currentTemp);
      AWSClient.publish(ep, AWSIOT_TOPIC_UPDATE, buffer, strlen((char *)buffer),
                        publish_params);
    }
    doPublish = false;
  }
}