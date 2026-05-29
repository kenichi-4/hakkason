#ifndef ACCEL_PROCESSING_H
#define ACCEL_PROCESSING_H

#include <Arduino.h>

float getTotalAcceleration(float ax, float ay, float az);
float removeGravity(float totalAccel);
float getMoveAcceleration(float ax, float ay, float az);

#endif