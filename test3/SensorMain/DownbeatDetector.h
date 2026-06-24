#ifndef DOWNBEAT_DETECTOR_H
#define DOWNBEAT_DETECTOR_H

#include <Arduino.h>
#include "IMUReader.h"

class DownbeatDetector {
public:
  DownbeatDetector();

  bool detectDownbeat(IMUData data, unsigned long now);

  void setDownGyroThreshold(float value);
  void setSideRatio(float value);
  void setMinInterval(unsigned long value);
  void setAxis(int value);
  void setDirection(int value);

  float getLastGyroValue();
  unsigned long getInterval();

private:
  float downGyroThreshold;
  float sideRatio;

  unsigned long minInterval;
  unsigned long lastDownbeatTime;
  unsigned long interval;

  int axis;
  int direction;

  float lastGyroValue;
};

#endif