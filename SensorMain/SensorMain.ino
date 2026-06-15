#include "IMUReader.h"
#include "MotionFilter.h"

IMUReader imu;
MotionFilter filter;

// センサ取得間隔 10ms
const unsigned long SAMPLE_INTERVAL = 10;
unsigned long lastSampleTime = 0;

// ピーク値を出す区間 1000ms = 1秒
const unsigned long PEAK_INTERVAL = 1000;
unsigned long lastPeakOutputTime = 0;

// 一定区間内の最大値
float peakValue = 0.0;

void setup() {
  Serial.begin(9600);

  imu.begin();

  Serial.println("time,peakValue");
}

void loop() {
  unsigned long now = millis();

  if (now - lastSampleTime >= SAMPLE_INTERVAL) {
    lastSampleTime = now;

    // 1. IMUデータ取得
    IMUData data = imu.readIMU();

    // 2. 重力成分を除去
    IMUData filterData = filter.removeGravity(data);

    // 3. 合成加速度を算出
    float motion = filter.calcMotion(filterData);

    // 4. 平滑化
    float smoothMotion = filter.smooth(motion);

    // 5. 一定区間内の最大値を記録
    if (smoothMotion > peakValue) {
      peakValue = smoothMotion;
    }
  }

  // 1秒ごとにピーク値を出力してリセット
  if (now - lastPeakOutputTime >= PEAK_INTERVAL) {
    lastPeakOutputTime = now;

    Serial.print(now);
    Serial.print(",");
    Serial.println(peakValue);

    // 次の区間のためにリセット
    peakValue = 0.0;
  }
}