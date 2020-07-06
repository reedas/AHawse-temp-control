#ifndef DISPLAY_THREAD_H
#define DISPLAY_THREAD_H

void displayThread();


void displaySendUpdateTemp(float temperature);
void displaySendUpdateTime();
void displaySendUpdateSetPoint(float setPoint);
void displaySendUpdateMode(float mode);
void displaySendDebug(float code);

#endif
