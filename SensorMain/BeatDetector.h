#ifndef BEAT_DETECTOR_H
#define BEAT_DETECTOR_H

#include <Arduino.h>

class BeatDetector {
public:
  BeatDetector();

  // 平滑化後の合成加速度からピークを検出する
  bool detectPeak(float motion, unsigned long now);

  // 最後に検出したピーク値を返す
  float getPeakValue();

  // 最後に検出したピーク時刻を返す
  unsigned long getPeakTime();

  // 必要に応じてしきい値を変更する
  void setThreshold(float threshold);

private:
  float peakThreshold;       // ピーク検出のしきい値
  float releaseThreshold;    // 次のピーク検出を許可するしきい値

  bool detecting;            // 現在ピーク候補を追跡中か
  float candidatePeakValue;  // 追跡中の最大値
  unsigned long candidatePeakTime;

  float lastPeakValue;
  unsigned long lastPeakTime;

  const unsigned long DEBOUNCE_TIME = 250; // 連続誤検出防止時間[ms]
};

#endif
