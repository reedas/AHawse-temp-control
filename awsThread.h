#ifndef AWS_THREAD_H
#define AWS_THREAD_H
void awsThread(void);
void awsSendUpdateTemperature(float temperature);
void awsSendUpdateSetPoint(float temperature);
void awsSendUpdateLightLevel(float temperature);
void awsSendUpdateRelativeHumidity(float temperature);
#endif