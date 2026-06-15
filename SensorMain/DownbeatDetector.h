#ifndef DOWNBEAT_DETECTOR_H
#define DOWNBEAT_DETECTOR_H

#include <Arduino.h>
#include "IMUReader.h"

class DownbeatDetector {
public:
  DownbeatDetector();

  // ジャイロデータから下方向の振りを検出し、ダウンビートであるかを判定する
  bool detectDownbeat(IMUData data, bool peakDetected, unsigned long now);

  void setGyroThreshold(float value);
  void setMinInterval(unsigned long value);
  void setAxis(int value);
  void setDirection(int value);

  float getLastGyroValue();
  unsigned long getLastDownbeatTime();

private:
  float gyroThreshold;
  unsigned long minInterval;
  unsigned long lastDownbeatTime;

  // 0:gx, 1:gy, 2:gz
  int axis;

  // 1:正方向を下方向として扱う、-1:負方向を下方向として扱う
  int direction;

  float lastGyroValue;
};

#endif
