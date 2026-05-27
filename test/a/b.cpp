#include "b.h"
#include <Wire.h>
#include <SparkFun_MMA8452Q.h>
#include <math.h>

MMA8452Q accel;

// ピーク値を調べる区間
const unsigned long PEAK_INTERVAL = 4000; 

// 区間内の最大加速度
float peakAccel = 0.0;

// 時間管理
unsigned long previousPeakTime = 0;

void setupMMA8452Q()
{
  Serial.begin(9600);
  Serial.println("MMA8452Q test");

  accel.init();

  // ピーク検出用の初期設定
  peakAccel = 0.0;
  previousPeakTime = millis();
}

float getTotalAcceleration()
{
  accel.read();

  float x = accel.cx;
  float y = accel.cy;
  float z = accel.cz;

  // 3軸合成加速度を計算
  float totalAccel = sqrt(x * x + y * y + z * z);

  // 重力分の 1.0g を最後に引く
  float moveAccel = totalAccel - 1.0;

  return moveAccel;
}

void printPeakAcceleration()
{
  float moveAccel = getTotalAcceleration();

  // 最大値を更新
  if (moveAccel > peakAccel)
  {
    peakAccel = moveAccel;
  }

  unsigned long currentTime = millis();

  // 一定時間ごとにピーク値を表示
  if (currentTime - previousPeakTime >= PEAK_INTERVAL)
  {
    Serial.print("Peak Accel: ");
    Serial.print(peakAccel, 3);
    Serial.println(" g");

    // 次の区間に向けてリセット
    peakAccel = 0.0;
    previousPeakTime = currentTime;
  }

  delay(50);
}