#ifndef MOTION_FILTER_H
#define MOTION_FILTER_H

#include <Arduino.h>

// 重力除去後の加速度
extern float filterAx;
extern float filterAy;
extern float filterAz;

// 合成加速度
extern float totalAccel;

// 平滑化後の合成加速度
extern float smoothAccel;

// 重力成分を除去する関数
void removeGravity(float ax, float ay, float az);

// 3軸加速度から合成加速度を計算する関数
void calcMotion();

// 合成加速度を平滑化する関数
void smoothMotion();

#endif
