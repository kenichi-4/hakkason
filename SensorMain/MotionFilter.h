#ifndef MOTION_FILTER_H
#define MOTION_FILTER_H

#include <Arduino.h>
#include "IMUReader.h"

class MotionFilter {
public:
  MotionFilter();

  // 加速度データから重力成分を除去
  IMUData removeGravity(IMUData data);

  // 3軸加速度から合成加速度を算出
  float calcMotion(IMUData data);

  // 合成加速度を平滑化
  float smooth(float motion);

private:
  float gravityX;
  float gravityY;
  float gravityZ;

  static const int SMOOTH_SIZE = 5;
  float motionBuffer[SMOOTH_SIZE];

  int bufferIndex;
  bool bufferFilled;

  const float GRAVITY_ALPHA = 0.9;
};

#endif
