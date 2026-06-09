#ifndef E_H
#define E_H

#include <Arduino.h>

void setupBPMCalculation();
void updateBPMCalculation(float moveAccel);
float getCurrentBPM();
int getTempoLevel();

#endif