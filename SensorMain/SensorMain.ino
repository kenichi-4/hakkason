#include "IMUReader.h"
#include "MotionFilter.h"
#include "BeatDetector.h"
#include "DownbeatDetector.h"

IMUReader imu;
MotionFilter filter;
BeatDetector beatDetector;
DownbeatDetector downbeatDetector;

// 10msごとにセンサ値を取得する
const unsigned long SAMPLE_INTERVAL = 10;
unsigned long lastSampleTime = 0;

void setup() {
  Serial.begin(9600);
  delay(1000);

  if (!imu.begin()) {
    Serial.println("MPU6050 init failed");
    while (1) {
      delay(10);
    }
  }

  // 調整用しきい値
  beatDetector.setThreshold(0.20);       // 加速度ピークのしきい値
  beatDetector.setMinInterval(250);      // 250ms以内の連続ピークを無効化

  downbeatDetector.setAxis(1);           // 0:gx, 1:gy, 2:gz
  downbeatDetector.setDirection(-1);     // 反応しない場合は1に変更
  downbeatDetector.setGyroThreshold(80); // ジャイロしきい値
  downbeatDetector.setMinInterval(500);  // 500ms以内の連続ダウンビートを無効化

  Serial.println("time,ax,ay,az,gx,gy,gz,filterAx,filterAy,filterAz,motion,smoothMotion,peak,peakValue,peakTime,downbeat,downbeatTime");
}

void loop() {
  unsigned long now = millis();

  if (now - lastSampleTime < SAMPLE_INTERVAL) {
    return;
  }

  lastSampleTime = now;

  // 1. IMUデータ取得
  IMUData data = imu.readIMU();

  if (!data.valid) {
    return;
  }

  // 2. 加速度・ジャイロデータの前処理
  IMUData filterData = filter.removeGravity(data);

  // 3. 合成加速度の算出
  float motion = filter.calcMotion(filterData);

  // 4. 移動平均による平滑化
  float smoothMotion = filter.smooth(motion);

  // 5. 加速度ピーク検出
  bool peakDetected = beatDetector.detectPeak(smoothMotion, now);

  // 6. ジャイロデータを用いたダウンビート検出
  bool downbeatDetected = downbeatDetector.detectDownbeat(data, peakDetected, now);

  Serial.print(now);
  Serial.print(",");

  Serial.print(data.ax);
  Serial.print(",");
  Serial.print(data.ay);
  Serial.print(",");
  Serial.print(data.az);
  Serial.print(",");

  Serial.print(data.gx);
  Serial.print(",");
  Serial.print(data.gy);
  Serial.print(",");
  Serial.print(data.gz);
  Serial.print(",");

  Serial.print(filterData.ax);
  Serial.print(",");
  Serial.print(filterData.ay);
  Serial.print(",");
  Serial.print(filterData.az);
  Serial.print(",");

  Serial.print(motion);
  Serial.print(",");
  Serial.print(smoothMotion);
  Serial.print(",");

  Serial.print(peakDetected ? 1 : 0);
  Serial.print(",");
  Serial.print(beatDetector.getPeakValue());
  Serial.print(",");
  Serial.print(beatDetector.getPeakTime());
  Serial.print(",");

  Serial.print(downbeatDetected ? 1 : 0);
  Serial.print(",");
  Serial.println(downbeatDetector.getLastDownbeatTime());
}
