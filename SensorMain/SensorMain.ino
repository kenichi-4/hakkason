#include "IMUReader.h"
#include "MotionFilter.h"
#include "BeatDetector.h"
#include "DownbeatDetector.h"
#include "TempoCalculator.h"
#include "SensorSender.h"

IMUReader        imu;             // センサ読み取り
MotionFilter     filter;          // 重力除去・平滑化
BeatDetector     beatDetector;    // 1拍区間の「強さ」管理
DownbeatDetector downbeatDetector;// 振り下ろし検出＋間隔測定
TempoCalculator  tempo;           // 間隔→BPM→3段階
SensorSender     sender;          // START/テンポ情報送信

const unsigned long SAMPLE_INTERVAL = 10;  // 10msごとにセンサを読む（100Hz）
unsigned long lastSampleTime = 0;

bool isStarted = false;  // 最初の振り下ろしでSTARTを送ったか

void setup() {
  Serial.begin(BAUD);

  imu.begin();
  sender.begin();

  // ダウンビート検知設定
  downbeatDetector.setAxis(0);
  downbeatDetector.setDirection(-1);
  downbeatDetector.setDownGyroThreshold(120.0);
  downbeatDetector.setSideRatio(1.5);
  downbeatDetector.setMinInterval(700); 


  // ----- テンポ計算の設定（★必要なら実測で調整）-----
  tempo.setBeatsPerMeasure(2);                  // 2拍子
  tempo.setThresholds(130.0, 160.0);            // 130未満SLOW / 160以上FAST


  // シリアルモニタの見出し（CSVとして表計算に貼れる）
  Serial.println("time,interval,bpm,level,send");
}

void loop() {
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

  // センサから読む
  IMUData data = imu.readIMU();

  //  前処理
  IMUData filterData  = filter.removeGravity(data);
  float   motion      = filter.calcMotion(filterData);
  float   smoothMotion = filter.smooth(motion);

  // ダウンビート
  bool downbeatDetected = downbeatDetector.detectDownbeat(data, now);

  // peak
  beatDetector.update(smoothMotion, downbeatDetected);

  // 振り下ろしが来た時だけ、テンポを計算して表示 
  if (downbeatDetected) {
    unsigned long itv = downbeatDetector.getInterval();

    if (!isStarted) {
      // 最初の振り下ろし＝演奏開始の合図
      isStarted = true;
      sender.sendStart();
      Serial.println(">> START");
      return;
    }

    // 間隔をテンポ計算に渡す。
    // 最初の拍は itv==0 なので update は false を返す（BPMはまだ出ない）。
    if (tempo.update(itv)) {
      TempoLevel lv = tempo.getLevel();
      const char* name = (lv == TEMPO_SLOW)   ? "SLOW" :
                         (lv == TEMPO_NORMAL) ? "NORMAL" : "FAST";
      int sendLevel = (lv == TEMPO_SLOW)   ? 1 :
                      (lv == TEMPO_NORMAL) ? 2 : 3;

      sender.sendTempo(tempo.getBPM(), sendLevel);

      Serial.print(now);          Serial.print(",");
      Serial.print(itv);          Serial.print(",");
      Serial.print(tempo.getBPM()); Serial.print(",");
      Serial.print(name);         Serial.print(",");
      Serial.println(sendLevel);
    }
  }
}
