#ifndef TEMPERATURE_H
#define TEMPERATURE_H

void temperatureThread();

void tempSendUpdateSetpointF(float setPoint);
void tempSendDeltaSetpointF(float delta);
void modeControlSetMode(float controlMode);

#endif
