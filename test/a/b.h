#ifndef B_H
#define B_H

#include <Arduino.h>

void setupMMA8452Q();
float getTotalAcceleration();
void printPeakAcceleration();

#endif