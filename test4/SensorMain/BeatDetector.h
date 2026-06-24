#ifndef BEAT_DETECTOR_H
#define BEAT_DETECTOR_H

#include <Arduino.h>

class BeatDetector {
public:
  BeatDetector();

  // motionValue の最大値を保存し、downbeatDetected が true の時に1拍分のピーク値を確定する
  bool update(float motionValue, bool downbeatDetected);

  // 前回の1拍区間での最大ピーク値を返す
  float getPeakValue();

  // ピーク値をリセットする
  void reset();

private:
  float peakValue;
  float outputPeakValue;
  bool firstDownbeat;
};

#endif