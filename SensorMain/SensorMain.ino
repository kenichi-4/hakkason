#include "MPU6050Data.h"
#include "AccelProcessing.h"
#include "PeakDetection.h"

void setup()
{
  setupMPU6050();
  setupPeakDetection();
}

void loop()
{
  float ax, ay, az;

  //MPU6050からデータ取得
  readMPU6050Accel(ax, ay, az);

  // 合成加速度を計算し，重力分を除去
  float moveAccel = getMoveAcceleration(ax, ay, az);

  //一定区間のピーク値を表示
  updatePeakAcceleration(moveAccel);

  delay(50);
}