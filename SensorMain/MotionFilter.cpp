#include "MotionFilter.h"
#include <math.h>

// 重力除去後の加速度
float filterAx = 0.0;
float filterAy = 0.0;
float filterAz = 0.0;

// 合成加速度
float totalAccel = 0.0;

// 平滑化後の合成加速度
float smoothAccel = 0.0;

// ローパスフィルタで推定する重力成分
float gravityX = 0.0;
float gravityY = 0.0;
float gravityZ = 0.0;

// 重力推定用の係数
// 0.9に近いほどゆっくり重力を追いかける
const float GRAVITY_ALPHA = 0.9;

// 移動平均用
const int SMOOTH_SIZE = 5;
float accelBuffer[SMOOTH_SIZE] = {0};
int bufferIndex = 0;
bool bufferFilled = false;

// 加速度データから重力成分を除去する関数
void removeGravity(float ax, float ay, float az) {
  // ローパスフィルタで重力成分を推定
  gravityX = GRAVITY_ALPHA * gravityX + (1.0 - GRAVITY_ALPHA) * ax;
  gravityY = GRAVITY_ALPHA * gravityY + (1.0 - GRAVITY_ALPHA) * ay;
  gravityZ = GRAVITY_ALPHA * gravityZ + (1.0 - GRAVITY_ALPHA) * az;

  // 元の加速度から重力成分を引く
  filterAx = ax - gravityX;
  filterAy = ay - gravityY;
  filterAz = az - gravityZ;
}

// 3軸加速度から合成加速度を算出する関数
void calcMotion() {
  totalAccel = sqrt(
    filterAx * filterAx +
    filterAy * filterAy +
    filterAz * filterAz
  );
}

// 合成加速度を移動平均で平滑化する関数
void smoothMotion() {
  accelBuffer[bufferIndex] = totalAccel;
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
    sum += accelBuffer[i];
  }

  if (count > 0) {
    smoothAccel = sum / count;
  } else {
    smoothAccel = totalAccel;
  }
}
