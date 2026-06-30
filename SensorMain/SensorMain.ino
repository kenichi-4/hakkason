// =============================================================
//  SensorMain.ino  ―  本番版（同期コントローラへWiFi送信あり）
//
//  フロー：
//    IMU取得 → 前処理 → 振り上げ(START) / 振り下ろし(テンポ) / 静止(STOP)
//    を判定し、同期コントローラへ START / テンポ段階 / STOP を送る。
//
//  停止の考え方（SensorTestで確定した方針）：
//    横払いジェスチャでの停止はやめた。速く振るとジャイロが全軸とも
//    振り切れて(±250)、横払いと激しい指揮を区別できないため。
//    かわりに「振るのをやめる＝全軸が静かになる」を停止の合図にした。
//    止めるジェスチャ自体は人間が"演技"でやり、検出は静止で確実に行う。
// =============================================================

#include "IMUReader.h"
#include "MotionFilter.h"
#include "BeatDetector.h"
#include "DownbeatDetector.h"
#include "TempoCalculator.h"
#include "StartDetector.h"
#include "SensorSender.h"

const unsigned long STOP_TIMEOUT = 4000;  // この時間ずっと動きが無ければ停止[ms]
const float MOTION_THRESHOLD = 40.0;      // これより動いていれば「まだ振っている」とみなす
                                          // （静止時は全軸20以下なので、その上に設定）

IMUReader        imu;             // センサ読み取り
MotionFilter     filter;          // 重力除去・平滑化
BeatDetector     beatDetector;    // 1拍区間の「強さ」管理（強弱演出用・テンポには未使用）
DownbeatDetector downbeatDetector;// 振り下ろし検出＋間隔測定
TempoCalculator  tempo;           // 間隔→BPM→3段階
StartDetector    startDetector;   // 振り上げ（構え）で開始判定
SensorSender     sender;          // START/テンポ/STOP 送信

const unsigned long SAMPLE_INTERVAL = 10;  // 10msごとにセンサを読む（100Hz）
unsigned long lastSampleTime = 0;

// 演奏の状態。
//   IDLE    : 待機中。指揮棒を持っても何も起きない。開始ジェスチャ(振り上げ)を待つ。
//   PLAYING : 演奏中。振り下ろしでテンポ。手を止めて一定時間で停止。
enum PerformState { IDLE, PLAYING };
PerformState state = IDLE;

unsigned long lastMotionTime = 0;  // 最後に動きがあった時刻（停止タイマーの起点）

void setup() {
  Serial.begin(BAUD);

  imu.begin();
  sender.begin();

  // ----- ダウンビート検知の設定（★実機で振って調整する値）-----
  downbeatDetector.setAxis(0);                  // gx（実測で確認した軸に合わせる）
  downbeatDetector.setDirection(-1);            // 振り下ろしでgxがマイナスなら-1
  downbeatDetector.setDownGyroThreshold(80.0);  // 振り下ろしの強さの下限
  downbeatDetector.setSideRatio(1.2);           // 横振りを除外する強さ
  downbeatDetector.setMinInterval(600);         // 連続検知を防ぐ最小間隔[ms]（揺り戻しの二度検出を防ぐ）

  // ----- テンポ計算の設定（★必要なら実測で調整）-----
  tempo.setBeatsPerMeasure(2);                  // 2拍子
  tempo.setThresholds(100.0, 130.0);            // 100未満SLOW / 130以上FAST

  // ----- 開始判定の設定（★実機で振って調整する値）-----
  startDetector.setAxis(0);                     // 振り上げを見る軸（振り下ろしと同じgx）
  startDetector.setDirection(1);                // 振り上げ方向の符号（振り下ろしの-1の逆＝+1）
  startDetector.setThreshold(200.0);            // 振り上げと判定する強さ（持ち上げ程度で反応しない高めの値）
  startDetector.setMinInterval(500);            // 連続検出防止

  Serial.println("=== SensorMain ===");
}

void loop() {
  // 同期コントローラと繋がるまで待つ（WELCOME/READYが返るまで何も送らない）
  if (!sender.isReady()) {
    sender.updateConnection();
    return;
  }

  unsigned long now = millis();

  // 10msごとにだけ処理する
  if (now - lastSampleTime < SAMPLE_INTERVAL) {
    return;
  }
  lastSampleTime = now;

  // --- ① センサから読む ---
  IMUData data = imu.readIMU();

  // --- ② 前処理（重力除去 → 合成 → 平滑化）---
  IMUData filterData  = filter.removeGravity(data);
  float   motion      = filter.calcMotion(filterData);
  float   smoothMotion = filter.smooth(motion);

  // --- ③ ダウンビート（振り下ろし）を検出 ---
  bool downbeatDetected = downbeatDetector.detectDownbeat(data, now);

  // --- ④ 強さ(peak)を管理（テンポには使わないが残しておく）---
  beatDetector.update(smoothMotion, downbeatDetected);

  // ===== IDLE：待機中。開始ジェスチャ（鋭い振り上げ）だけを待つ =====
  if (state == IDLE) {
    // 振り下ろしを無視するので、指揮棒を持っただけ・構えただけでは始まらない。
    if (startDetector.detectRaise(data, now)) {
      state = PLAYING;
      tempo.reset();            // テンポ履歴をまっさらに
      lastMotionTime = now;     // 停止タイマーの起点
      sender.sendStart();
      Serial.println(">> START (振り上げ検出)");
    }
    return;
  }

  // ===== PLAYING =====

  // 「振り下ろし」ではなく「何か動いているか」で停止タイマーをリセットする。
  // ゆっくり指揮でも、振り上げ・戻しで指揮棒は動き続けるので止まらない。
  // 本当に手を止めた時だけ全軸が静かになり、タイマーが進んで停止する。
  float maxGyro = max(abs(data.gx), max(abs(data.gy), abs(data.gz)));
  if (maxGyro > MOTION_THRESHOLD) {
    lastMotionTime = now;
  }

  // --- ⑤ 振り下ろしが来たら、テンポを計算して送る ---
  if (downbeatDetected) {
    unsigned long itv = downbeatDetector.getInterval();

    // 演奏開始直後の最初の振り下ろしは間隔が大きすぎる/0なので update は false を返す。
    if (tempo.update(itv)) {
      TempoLevel lv = tempo.getLevel();
      const char* name = (lv == TEMPO_SLOW)   ? "SLOW" :
                         (lv == TEMPO_NORMAL) ? "NORMAL" : "FAST";
      int sendLevel = (lv == TEMPO_SLOW)   ? 1 :
                      (lv == TEMPO_NORMAL) ? 2 : 3;

      sender.sendTempo(tempo.getBPM(), sendLevel);

      // CSV形式：時刻,間隔,BPM,段階,送信値
      Serial.print(now);          Serial.print(",");
      Serial.print(itv);          Serial.print(",");
      Serial.print(tempo.getBPM()); Serial.print(",");
      Serial.print(name);         Serial.print(",");
      Serial.println(sendLevel);
    }
  }

  // --- ⑥ 動きが STOP_TIMEOUT の間ずっと無ければ停止 ---
  // 静止＝止めた合図。途中で動かせばタイマーがリセットされて継続。
  if (now - lastMotionTime > STOP_TIMEOUT) {
    sender.sendStop();
    state = IDLE;
    tempo.reset();
    Serial.println(">> STOP（静止）");
  }
}
