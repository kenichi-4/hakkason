#include "c.h"
#include "b.h"
#include <math.h>

float getMoveAcceleration()
{
  float x, y, z;
  readMMA8452Q(x, y, z);

  // 3軸合成加速度を計算する
  float totalAccel = sqrt(x * x + y * y + z * z);

  // 重力分の1.0gを引く
  float moveAccel = totalAccel - 1.0;

  return moveAccel;
}