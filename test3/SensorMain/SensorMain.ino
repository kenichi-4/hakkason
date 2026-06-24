#include "IMUReader.h"
#include "MotionFilter.h"
#include "BeatDetector.h"
#include "DownbeatDetector.h"
#include "TempoCalculator.h"

IMUReader imu;
MotionFilter filter;
BeatDetector beatDetector;
DownbeatDetector downbeatDetector;
TempoCalculator tempo;

const unsigned long SAMPLE_INTERVAL = 10;
unsigned long lastSampleTime = 0;

bool isStarted = false;

// 現在のテンポレベルとBPMを保存する
const char* currentLevelName = "NONE";
float currentBPM = 0.0;

void setup() {
  Serial.begin(9600);

  imu.begin();

  // ダウンビート検知設定
  downbeatDetector.setAxis(0);                  // 0:gx, 1:gy, 2:gz
  downbeatDetector.setDirection(-1);            // 下げた時にgxがマイナスなら-1。逆なら1
  downbeatDetector.setDownGyroThreshold(120.0); // 下げ方向のしきい値
  downbeatDetector.setSideRatio(1.2);           // 横振り除外の強さ

  downbeatDetector.setMinInterval(500);         // 連続検知防止時間 ms

  // 2拍子としてBPM計算
  tempo.setBeatsPerMeasure(2);

  // テンポレベルのしきい値
  // 130未満:SLOW, 130以上160未満:NORMAL, 160以上:FAST
  tempo.setThresholds(140.0, 170.0);

  Serial.println("downbeat,level,bpm");
}

void loop() {
  unsigned long now = millis();

  if (now - lastSampleTime >= SAMPLE_INTERVAL) {
    lastSampleTime = now;

    IMUData data = imu.readIMU();

    // 加速度の前処理
    IMUData filterData = filter.removeGravity(data);
    float motion = filter.calcMotion(filterData);
    float smoothMotion = filter.smooth(motion);

    // ダウンビート検知
    bool downbeatDetected = downbeatDetector.detectDownbeat(data, now);

    // ピーク値は内部で管理するだけ
    beatDetector.update(smoothMotion, downbeatDetected);

    if (downbeatDetected) {
      unsigned long interval = downbeatDetector.getInterval();

      // 最初のダウンビートはSTART扱い
      if (!isStarted) {
        isStarted = true;
        currentLevelName = "START";
        currentBPM = 0.0;
      } else {
        // 2回目以降はBPMからテンポレベルを更新
        if (tempo.update(interval)) {
          currentBPM = tempo.getBPM();

          TempoLevel lv = tempo.getLevel();

          if (lv == TEMPO_SLOW) {
            currentLevelName = "SLOW";
          } else if (lv == TEMPO_NORMAL) {
            currentLevelName = "NORMAL";
          } else {
            currentLevelName = "FAST";
          }
        }
      }
        // 出力は「ダウンビート検知」「現在のレベル」「BPM」だけ
    Serial.print(downbeatDetected ? 1 : 0);
    Serial.print(",");
    Serial.print(currentLevelName);
    Serial.print(",");
    Serial.println(currentBPM);
    }

  }
}