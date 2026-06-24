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

const float SIDE_SHAKE_GYRO_THRESHOLD = 100.0;
const float SIDE_SHAKE_RATIO = 1.2;
const float STOP_MOTION_THRESHOLD = 0.12;
const float STOP_GYRO_THRESHOLD = 20.0;
const unsigned long STOP_STILL_TIME = 500;
const unsigned long STOP_ARM_TIME = 2000;

bool stopArmed = false;
unsigned long stopArmTime = 0;
unsigned long stillStartTime = 0;

// 現在のテンポレベルとBPMを保存する
const char* currentLevelName = "NONE";
float currentBPM = 0.0;

bool detectSideShake(IMUData data) {
  float downGyro = abs(data.gx);
  float sideGyro = max(abs(data.gy), abs(data.gz));

  if (sideGyro < SIDE_SHAKE_GYRO_THRESHOLD) {
    return false;
  }

  return sideGyro > downGyro * SIDE_SHAKE_RATIO;
}

bool detectStill(IMUData data, float smoothMotion) {
  float maxGyro = max(abs(data.gx), max(abs(data.gy), abs(data.gz)));
  return smoothMotion < STOP_MOTION_THRESHOLD && maxGyro < STOP_GYRO_THRESHOLD;
}

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

    bool sideShakeDetected = detectSideShake(data);
    bool stillDetected = detectStill(data, smoothMotion);

    if (sideShakeDetected) {
      stopArmed = true;
      stopArmTime = now;
      stillStartTime = 0;
    }

    if (stopArmed && !sideShakeDetected) {
      if (stillDetected) {
        if (stillStartTime == 0) {
          stillStartTime = now;
        }

        if (isStarted && now - stillStartTime >= STOP_STILL_TIME) {
          isStarted = false;
          stopArmed = false;
          stillStartTime = 0;
          currentLevelName = "STOP";
          currentBPM = 0.0;
          tempo.reset();

          Serial.print(0);
          Serial.print(",");
          Serial.print(currentLevelName);
          Serial.print(",");
          Serial.println(currentBPM);
        }
      } else {
        stillStartTime = 0;

        if (now - stopArmTime > STOP_ARM_TIME) {
          stopArmed = false;
        }
      }
    }

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
