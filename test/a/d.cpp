#include "d.h"

// ピーク値を調べる区間
const unsigned long PEAK_INTERVAL = 4000;

// 区間内の最大加速度
float peakAccel = 0.0;

// 時間管理
unsigned long previousPeakTime = 0;

void setupPeakDetection()
{
  peakAccel = 0.0;
  previousPeakTime = millis();
}

void updatePeakAcceleration(float moveAccel)
{
  // 最大値を更新する
  if (moveAccel > peakAccel)
  {
    peakAccel = moveAccel;
  }

  printPeakAcceleration();
}

void printPeakAcceleration()
{
  unsigned long currentTime = millis();

  // 一定時間ごとにピーク値を表示する
  if (currentTime - previousPeakTime >= PEAK_INTERVAL)
  {
    Serial.print("Peak Accel: ");
    Serial.print(peakAccel, 3);
    Serial.println(" g");

    // 次の区間に向けてリセットする
    peakAccel = 0.0;
    previousPeakTime = currentTime;
  }
}
