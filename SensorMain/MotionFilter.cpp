#include "MotionFilter.h"
#include <math.h>

MotionFilter::MotionFilter() {
  gravityX = 0.0;
  gravityY = 0.0;
  gravityZ = 0.0;

  bufferIndex = 0;
  bufferFilled = false;

  for (int i = 0; i < SMOOTH_SIZE; i++) {
    motionBuffer[i] = 0.0;
  }
}

// 加速度データから重力成分を除去
IMUData MotionFilter::removeGravity(IMUData data) {
  IMUData filterData = data;

  gravityX = GRAVITY_ALPHA * gravityX + (1.0 - GRAVITY_ALPHA) * data.ax;
  gravityY = GRAVITY_ALPHA * gravityY + (1.0 - GRAVITY_ALPHA) * data.ay;
  gravityZ = GRAVITY_ALPHA * gravityZ + (1.0 - GRAVITY_ALPHA) * data.az;

  filterData.ax = data.ax - gravityX;
  filterData.ay = data.ay - gravityY;
  filterData.az = data.az - gravityZ;

  return filterData;
}

// 3軸加速度から合成加速度を算出
float MotionFilter::calcMotion(IMUData data) {
  float motion = sqrt(
    data.ax * data.ax +
    data.ay * data.ay +
    data.az * data.az
  );

  return motion;
}

// 合成加速度を移動平均で平滑化
float MotionFilter::smooth(float motion) {
  motionBuffer[bufferIndex] = motion;
  bufferIndex++;

  if (bufferIndex >= SMOOTH_SIZE) {
    bufferIndex = 0;
    bufferFilled = true;
  }

  int count;

  if (bufferFilled) {
    count = SMOOTH_SIZE;
  } else {
    count = bufferIndex;
  }

  float sum = 0.0;

  for (int i = 0; i < count; i++) {
    sum += motionBuffer[i];
  }

  if (count == 0) {
    return motion;
  }

  return sum / count;
}
