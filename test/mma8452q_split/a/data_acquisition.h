#ifndef DATA_ACQUISITION_H
#define DATA_ACQUISITION_H

#include <Arduino.h>

void setupMMA8452Q();
void readMMA8452Q(float &x, float &y, float &z);

#endif
