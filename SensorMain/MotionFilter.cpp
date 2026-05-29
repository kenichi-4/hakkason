#include "AccelProcessing.h"
#include <math.h>

float getTotalAcceleration(float ax, float ay, float az)
{
  // 3軸の合成加速度を計算
  float totalAccel = sqrt(ax * ax + ay * ay + az * az);
  return totalAccel;
}

float removeGravity(float totalAccel)
{
  // 静止時に含まれる重力加速度 1.0g を引く
  float moveAccel = totalAccel - 1.0;
  return moveAccel;
}

float getMoveAcceleration(float ax, float ay, float az)
{
  float totalAccel = getTotalAcceleration(ax, ay, az);
  float moveAccel = removeGravity(totalAccel);
  return moveAccel;
}