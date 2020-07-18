#ifndef AWS_THREAD_H
#define AWS_THREAD_H
void awsThread(void);
void awsSendUpdateTemperature(float temperature);
void awsSendUpdateSetPoint(float setPoint);
void awsSendUpdateLight(int lightLevel);

void awsSendUpdateHumid(int relHumidity);

void awsSendUpdateMode(float controlMode);
void awsSendIPAddress(void);
void awsSendUpdateDelta(float delta);
void awsSendAnnounce1(void);
void awsSendAnnounce2(void);
#endif