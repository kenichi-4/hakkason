#ifndef PEAK_DETECTION_H
#define PEAK_DETECTION_H

#include <Arduino.h>

void setupPeakDetection();
void updatePeakAcceleration(float moveAccel);
float getPeakAcceleration();

#endif