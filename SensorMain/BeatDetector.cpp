#include "BeatDetector.h"

BeatDetector::BeatDetector() {
  peakThreshold = 0.20;
  releaseThreshold = 0.10;

  detecting = false;
  candidatePeakValue = 0.0;
  candidatePeakTime = 0;

  lastPeakValue = 0.0;
  lastPeakTime = 0;
}

bool BeatDetector::detectPeak(float motion, unsigned long now) {
  bool peakDetected = false;

  // しきい値を超えたらピーク候補の追跡を開始
  if (!detecting && motion >= peakThreshold) {
    detecting = true;
    candidatePeakValue = motion;
    candidatePeakTime = now;
  }

  // ピーク候補中は最大値を更新
  if (detecting) {
    if (motion > candidatePeakValue) {
      candidatePeakValue = motion;
      candidatePeakTime = now;
    }

    // 値が十分下がったら、直前の最大値をピークとして確定
    if (motion <= releaseThreshold) {
      if (candidatePeakTime - lastPeakTime >= DEBOUNCE_TIME) {
        lastPeakValue = candidatePeakValue;
        lastPeakTime = candidatePeakTime;
        peakDetected = true;
      }

      detecting = false;
      candidatePeakValue = 0.0;
      candidatePeakTime = 0;
    }
  }

  return peakDetected;
}

float BeatDetector::getPeakValue() {
  return lastPeakValue;
}

unsigned long BeatDetector::getPeakTime() {
  return lastPeakTime;
}

void BeatDetector::setThreshold(float threshold) {
  peakThreshold = threshold;
  releaseThreshold = threshold * 0.5;
}
