#include "DownbeatDetector.h"

DownbeatDetector::DownbeatDetector() {
  downGyroThreshold = 80.0;

  // 下方向の値が横方向より何倍大きいか
  // 1.2なら「下方向が横方向の1.2倍以上」の時だけ検知
  sideRatio = 1.2;

  minInterval = 300;
  lastDownbeatTime = 0;
  interval = 0;

  axis = 1;        // 0:gx, 1:gy, 2:gz
  direction = -1;  // 下げた時にgyがマイナスなら-1

  lastGyroValue = 0.0;
}

bool DownbeatDetector::detectDownbeat(IMUData data, unsigned long now) {
  float gyroValue;
  float sideGyro1;
  float sideGyro2;

  // 下方向として使う軸と、それ以外の横方向成分を分ける
  if (axis == 0) {
    gyroValue = data.gx;
    sideGyro1 = abs(data.gy);
    sideGyro2 = abs(data.gz);
  } else if (axis == 1) {
    gyroValue = data.gy;
    sideGyro1 = abs(data.gx);
    sideGyro2 = abs(data.gz);
  } else {
    gyroValue = data.gz;
    sideGyro1 = abs(data.gx);
    sideGyro2 = abs(data.gy);
  }

  // 下方向の値を正の値として扱う
  float downGyro = gyroValue * direction;
  lastGyroValue = downGyro;

  // 横方向で一番大きい値
  float maxSideGyro = max(sideGyro1, sideGyro2);

  // 連続検知を防ぐ
  if (now - lastDownbeatTime < minInterval) {
    return false;
  }

  // 下方向の値がしきい値を超えているか
  if (downGyro <= downGyroThreshold) {
    return false;
  }

  // 横振りが強すぎる場合は無視
  // 下方向の値が横方向より十分大きい時だけ検知
  if (downGyro < maxSideGyro * sideRatio) {
    return false;
  }

  // 前回検知との時間差を保存
  if (lastDownbeatTime == 0) {
    interval = 0;
  } else {
    interval = now - lastDownbeatTime;
  }

  lastDownbeatTime = now;
  return true;
}

void DownbeatDetector::setDownGyroThreshold(float value) {
  downGyroThreshold = value;
}

void DownbeatDetector::setSideRatio(float value) {
  sideRatio = value;
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

unsigned long DownbeatDetector::getInterval() {
  return interval;
}