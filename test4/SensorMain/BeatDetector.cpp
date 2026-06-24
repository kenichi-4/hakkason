#include "BeatDetector.h"

BeatDetector::BeatDetector() {
  peakValue = 0.0;
  outputPeakValue = 0.0;
  firstDownbeat = true;
}

bool BeatDetector::update(float motionValue, bool downbeatDetected) {
  // 1拍区間内の最大値を保存する
  if (motionValue > peakValue) {
    peakValue = motionValue;
  }

  // ダウンビートが来ていないなら、まだピーク値を確定しない
  if (!downbeatDetected) {
    return false;
  }

  // 最初のダウンビートは区間開始なので出力しない
  if (firstDownbeat) {
    firstDownbeat = false;
    peakValue = 0.0;
    return false;
  }

  // 前回のダウンビートから今回のダウンビートまでの最大値を確定する
  outputPeakValue = peakValue;

  // 次の1拍区間のためにリセットする
  peakValue = 0.0;

  return true;
}

float BeatDetector::getPeakValue() {
  return outputPeakValue;
}

void BeatDetector::reset() {
  peakValue = 0.0;
  outputPeakValue = 0.0;
  firstDownbeat = true;
}