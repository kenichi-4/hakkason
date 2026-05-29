#ifndef MPU6050_DATA_H
#define MPU6050_DATA_H

#include <Arduino.h>

void setupMPU6050();
void readMPU6050Accel(float &ax, float &ay, float &az);

#endif