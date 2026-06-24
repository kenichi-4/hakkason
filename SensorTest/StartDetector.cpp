#include "StartDetector.h"

StartDetector::StartDetector() {
  axis        = 0;       // gx（振り下ろしと同じ軸）
  direction   = 1;       // 振り上げが正になる符号（振り下ろしが-1なら振り上げは+1）
  threshold   = 120.0;   // 振り上げと判定する強さ。誤発進を防ぐため振り下ろしより高め（実測で調整）
  minInterval = 500;     // 連続検出を防ぐ最小間隔[ms]
  lastTime    = 0;
}

bool StartDetector::detectRaise(IMUData data, unsigned long now) {
  // 見る軸の角速度を取り出す
  float value;
  if (axis == 0) {
    value = data.gx;
  } else if (axis == 1) {
    value = data.gy;
  } else {
    value = data.gz;
  }

  // direction を掛けて「振り上げ方向」を正の値にする。
  // （振り下ろし方向は負になるので、振り下ろしでは誤反応しない）
  float raise = value * direction;

  // 連続検出を防ぐ
  if (now - lastTime < minInterval) {
    return false;
  }

  // 振り上げが閾値を超えたら開始ジェスチャとみなす
  if (raise > threshold) {
    lastTime = now;
    return true;
  }

  return false;
}

void StartDetector::setAxis(int value) {
  if (value >= 0 && value <= 2) {
    axis = value;
  }
}

void StartDetector::setDirection(int value) {
  if (value >= 0) {
    direction = 1;
  } else {
    direction = -1;
  }
}

void StartDetector::setThreshold(float value) {
  threshold = value;
}

void StartDetector::setMinInterval(unsigned long value) {
  minInterval = value;
}
