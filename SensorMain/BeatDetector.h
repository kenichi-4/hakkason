#ifndef BEAT_DETECTOR_H
#define BEAT_DETECTOR_H

#include <Arduino.h>
#include "IMUReader.h"

class BeatDetector {
public:
  BeatDetector();

  // ダウンビートが検出されたら true を返す
  bool update(IMUData data, float smoothMotion, unsigned long now);

  // 前回のダウンビート区間での最大ピーク値を返す
  float getPeakValue();

  // 調整用
  void setMotionThreshold(float value);
  void setGyroThreshold(float value);
  void setDownbeatInterval(unsigned long value);

private:
  float motionThreshold;
  float gyroThreshold;
  unsigned long downbeatInterval;

  unsigned long lastDownbeatTime;

  float peakValue;
  float outputPeakValue;

  bool firstDownbeat;

  bool detectDownbeat(IMUData data, float smoothMotion, unsigned long now);
  float getMaxGyro(IMUData data);
};

#endif