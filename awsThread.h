#ifndef AWS_THREAD_H
#define AWS_THREAD_H
void awsThread(void);
void awsSendUpdateTemperature(float temperature);
void awsSendUpdateSetPoint(float setPoint);
void awsSendUpdateLightLevel(float lightLevel);
void awsSendUpdateRelativeHumidity(float relHumidity);
void awsSendUpdateMode(float controlMode);
#endif