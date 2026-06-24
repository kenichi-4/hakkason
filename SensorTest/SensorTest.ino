// =============================================================
//  SensorTest.ino  ―  センサ単体テスト用（同期なし）
//
//  目的：
//    同期コントローラ（WiFi）に繋がず、センサ単体で
//    「開始ジェスチャ・テンポ検出・停止」がちゃんと動くかを
//    シリアルモニタで確認する。
//
//  操作：
//    鋭く振り上げる   → ">> START"（演奏開始）
//    振り下ろす       → テンポ表示（振り続ける限り続く）
//    振るのをやめる   → 3秒たつと ">> STOP"（途中で振り直せば継続）
//
//  停止の考え方（重要）：
//    横払いジェスチャでの停止はやめた。理由は、速く振るとジャイロが
//    全軸とも振り切れて(±250)、横払いと激しい指揮を区別できないため。
//    かわりに「振るのをやめる＝全軸が静かになる」を停止の合図にした。
//    止まれば全軸が一気に小さくなる(数値が一桁)ので、確実に検出できる。
//
//  調整用：DEBUG_RAW を true にすると 0.5秒ごとに各軸のピークが出る。
// =============================================================

#include "IMUReader.h"
#include "MotionFilter.h"
#include "BeatDetector.h"
#include "DownbeatDetector.h"
#include "TempoCalculator.h"
#include "StartDetector.h"

const long BAUD = 115200;            // シリアルの速さ（モニタも115200に合わせる）
const bool DEBUG_RAW = false;        // true で 0.5秒ごとに各軸のピーク表示（調整用）
const unsigned long STOP_TIMEOUT = 4000;  // この時間ずっと動きが無ければ停止[ms]
const float MOTION_THRESHOLD = 40.0;      // これより動いていれば「まだ振っている」とみなす
                                          // （静止時は全軸20以下なので、その上に設定）

IMUReader        imu;
MotionFilter     filter;
BeatDetector     beatDetector;
DownbeatDetector downbeatDetector;
TempoCalculator  tempo;
StartDetector    startDetector;

const unsigned long SAMPLE_INTERVAL = 10;
unsigned long lastSampleTime = 0;

// DEBUG_RAW用：0.5秒ごとに各軸のピーク（絶対値の最大）を表示するための変数
float peakGx = 0, peakGy = 0, peakGz = 0;
unsigned long lastPeakPrint = 0;

// 演奏の状態
//   IDLE    : 待機中。振り上げ（START）を待つ。
//   PLAYING : 演奏中。振り下ろしでテンポ。振りが止まって3秒で停止。
enum PerformState { IDLE, PLAYING };
PerformState state = IDLE;

unsigned long lastMotionTime = 0;    // 最後に動きがあった時刻（停止タイマーの起点）

void setup() {
  Serial.begin(BAUD);

  imu.begin();

  // ----- 各検出の設定（SensorMainと同じ値。ここで調整する）-----
  downbeatDetector.setAxis(0);
  downbeatDetector.setDirection(-1);
  downbeatDetector.setDownGyroThreshold(80.0);
  downbeatDetector.setSideRatio(1.2);
  downbeatDetector.setMinInterval(500);

  tempo.setBeatsPerMeasure(2);
  tempo.setThresholds(140.0, 170.0);

  startDetector.setAxis(0);
  startDetector.setDirection(1);
  startDetector.setThreshold(200.0);   // 振り上げの強さ（甘ければ上げる）
  startDetector.setMinInterval(500);

  Serial.println("=== SensorTest 開始（同期なし）===");
  Serial.println("振り上げ=START / 振り下ろし=テンポ / 振るのをやめる=3秒で停止");
}

void loop() {
  unsigned long now = millis();
  if (now - lastSampleTime < SAMPLE_INTERVAL) {
    return;
  }
  lastSampleTime = now;

  // --- センサ読み取り＋前処理 ---
  IMUData data = imu.readIMU();
  IMUData filterData  = filter.removeGravity(data);
  float   motion      = filter.calcMotion(filterData);
  float   smoothMotion = filter.smooth(motion);

  // 調整用：0.5秒ごとに各軸のピーク（絶対値の最大）だけ表示
  if (DEBUG_RAW) {
    if (abs(data.gx) > peakGx) peakGx = abs(data.gx);
    if (abs(data.gy) > peakGy) peakGy = abs(data.gy);
    if (abs(data.gz) > peakGz) peakGz = abs(data.gz);
    if (now - lastPeakPrint >= 500) {
      Serial.print("ピーク gx="); Serial.print(peakGx, 0);
      Serial.print(" gy=");        Serial.print(peakGy, 0);
      Serial.print(" gz=");        Serial.println(peakGz, 0);
      peakGx = peakGy = peakGz = 0;
      lastPeakPrint = now;
    }
  }

  // --- 振り下ろし検出 ---
  bool downbeatDetected = downbeatDetector.detectDownbeat(data, now);
  beatDetector.update(smoothMotion, downbeatDetected);

  // ===== IDLE：振り上げ（START）を待つ =====
  if (state == IDLE) {
    if (startDetector.detectRaise(data, now)) {
      state = PLAYING;
      tempo.reset();
      lastMotionTime = now;     // 停止タイマーの起点をセット
      Serial.println(">> START (振り上げ検出)");
    }
    return;
  }

  // ===== PLAYING =====

  // 「振り下ろし」ではなく「何か動いているか」で停止タイマーをリセットする。
  // ゆっくり指揮でも、振り上げ・戻しで指揮棒は動き続けるのでタイマーが進まず止まらない。
  // 本当に手を止めた時だけ全軸が静かになり、タイマーが進んで停止する。
  float maxGyro = max(abs(data.gx), max(abs(data.gy), abs(data.gz)));
  if (maxGyro > MOTION_THRESHOLD) {
    lastMotionTime = now;
  }

  // 振り下ろし → テンポ
  if (downbeatDetected) {
    unsigned long itv = downbeatDetector.getInterval();
    if (tempo.update(itv)) {
      TempoLevel lv = tempo.getLevel();
      const char* name = (lv == TEMPO_SLOW)   ? "SLOW" :
                         (lv == TEMPO_NORMAL) ? "NORMAL" : "FAST";
      Serial.print("   テンポ: BPM=");
      Serial.print(tempo.getBPM());
      Serial.print(" level=");
      Serial.println(name);
    }
  }

  // 動きが STOP_TIMEOUT の間ずっと無ければ停止（静止＝止めた合図。途中で動かせば継続）
  if (now - lastMotionTime > STOP_TIMEOUT) {
    state = IDLE;
    tempo.reset();
    Serial.println(">> STOP（静止）");
  }
}
