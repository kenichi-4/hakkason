#ifndef START_DETECTOR_H
#define START_DETECTOR_H

#include <Arduino.h>
#include "IMUReader.h"

// =============================================================
//  StartDetector（開始ジェスチャ判定クラス）
//
//  役割：
//    演奏を「始める」合図＝指揮棒を鋭く振り上げる動き（構え／予備拍）を検出する。
//
//  なぜジェスチャにする？
//    「最初の振り下ろし＝開始」にすると、指揮棒を持ち上げただけ・構えただけの
//    動きが振り下ろしと誤検出され、勝手に演奏が始まってしまう。
//    そこで「鋭い振り上げ」という別の動きを開始合図にして、誤発進を防ぐ。
//
//  なぜ振り下ろしと混ざらない？
//    振り下ろしは下方向(gxがマイナス)、振り上げは上方向(gxがプラス)。
//    同じ軸でも符号が逆なので区別できる。さらに閾値を高くしておけば、
//    ゆっくり持ち上げる程度では反応せず、意図して鋭く振り上げた時だけ反応する。
// =============================================================

class StartDetector {
public:
  StartDetector();

  // 鋭い振り上げ（開始ジェスチャ）を検出したら true。
  bool detectRaise(IMUData data, unsigned long now);

  // --- 調整用 ---
  void setAxis(int value);                // 振り上げを見る軸（振り下ろしと同じ軸でOK）
  void setDirection(int value);           // 振り上げ方向の符号（振り下ろしの逆にする）
  void setThreshold(float value);         // 振り上げと判定する強さ（誤発進防止のため高め）
  void setMinInterval(unsigned long value);// 連続検出を防ぐ最小間隔

private:
  int           axis;          // 見る軸（0:gx 1:gy 2:gz）
  int           direction;     // 振り上げを正にする符号（+1 or -1）
  float         threshold;     // 振り上げの強さの下限
  unsigned long minInterval;   // 連続検出防止
  unsigned long lastTime;      // 前回検出した時刻
};

#endif
