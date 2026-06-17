#include "BeatDetector.h"

BeatDetector::BeatDetector() {
  motionThreshold = 0.15;
  gyroThreshold = 30.0;
  downbeatInterval = 500;

  lastDownbeatTime = 0;

  peakValue = 0.0;
  outputPeakValue = 0.0;

  firstDownbeat = true;
}

bool BeatDetector::update(IMUData data, float smoothMotion, unsigned long now) {
  // ダウンビート間の最大値を保存
  if (smoothMotion > peakValue) {
    peakValue = smoothMotion;
  }

  // ダウンビートを検出したか確認
  if (detectDownbeat(data, smoothMotion, now)) {

    // 最初のダウンビートは区間開始なので出力しない
    if (firstDownbeat) {
      firstDownbeat = false;
      peakValue = 0.0;
      return false;
    }

    // 前のダウンビートから今回のダウンビートまでの最大値を出力用に保存
    outputPeakValue = peakValue;

    // 次の区間のためにリセット
    peakValue = 0.0;

    return true;
  }

  return false;
}

float BeatDetector::getPeakValue() {
  return outputPeakValue;
}

void BeatDetector::setMotionThreshold(float value) {
  motionThreshold = value;
}

void BeatDetector::setGyroThreshold(float value) {
  gyroThreshold = value;
}

void BeatDetector::setDownbeatInterval(unsigned long value) {
  downbeatInterval = value;
}

bool BeatDetector::detectDownbeat(IMUData data, float smoothMotion, unsigned long now) {
  // 短時間に連続して検出しない
  if (now - lastDownbeatTime < downbeatInterval) {
    return false;
  }

  float maxGyro = getMaxGyro(data);

  // 動きが大きく、角速度も大きいときにダウンビートとする
  if (smoothMotion > motionThreshold && maxGyro > gyroThreshold) {
    lastDownbeatTime = now;
    return true;
  }

  return false;
}

float BeatDetector::getMaxGyro(IMUData data) {
  float absGx = abs(data.gx);
  float absGy = abs(data.gy);
  float absGz = abs(data.gz);

  float maxGyro = absGx;

  if (absGy > maxGyro) {
    maxGyro = absGy;
  }

  if (absGz > maxGyro) {
    maxGyro = absGz;
  }

  return maxGyro;
}