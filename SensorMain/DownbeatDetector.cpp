#include "DownbeatDetector.h"

DownbeatDetector::DownbeatDetector() {
  gyroThreshold = 80.0;
  minInterval = 500;
  lastDownbeatTime = 0;

  axis = 1;       // 初期設定ではgyを見る
  direction = -1; // 反応しない場合は1に変更する

  lastGyroValue = 0.0;
}

bool DownbeatDetector::detectDownbeat(IMUData data, bool peakDetected, unsigned long now) {


  if (now - lastDownbeatTime < minInterval) {
    return false;
  }

  float gyroValue;

  if (axis == 0) {
    gyroValue = data.gx;
  } else if (axis == 1) {
    gyroValue = data.gy;
  } else {
    gyroValue = data.gz;
  }

  lastGyroValue = gyroValue;

  float downGyro = gyroValue * direction;

  if (downGyro > gyroThreshold) {
    lastDownbeatTime = now;
    return true;
  }

  return false;
}

void DownbeatDetector::setGyroThreshold(float value) {
  gyroThreshold = value;
}

void DownbeatDetector::setMinInterval(unsigned long value) {
  minInterval = value;
}

void DownbeatDetector::setAxis(int value) {
  if (value >= 0 && value <= 2) {
    axis = value;
  }
}

void DownbeatDetector::setDirection(int value) {
  if (value >= 0) {
    direction = 1;
  } else {
    direction = -1;
  }
}

float DownbeatDetector::getLastGyroValue() {
  return lastGyroValue;
}

unsigned long DownbeatDetector::getLastDownbeatTime() {
  return lastDownbeatTime;
}
