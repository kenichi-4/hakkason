#ifndef IMU_READER_H
#define IMU_READER_H

#include <Arduino.h>
#include <Wire.h>

// MPU6050のI2Cアドレス
#define MPU_ADDR 0x68

// 加速度データ
extern float ax;
extern float ay;
extern float az;

// 角速度データ
extern float gx;
extern float gy;
extern float gz;

// IMUセンサの初期化
void setupIMU();

// IMUセンサから加速度・角速度を取得
void readIMU();

#endif
