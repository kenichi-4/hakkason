#include "IMUReader.h"
#include "MotionFilter.h"

IMUReader imu;
MotionFilter filter;

const unsigned long SAMPLE_INTERVAL = 10;
unsigned long lastSampleTime = 0;

void setup() {
  Serial.begin(115200);

  imu.begin();

  Serial.println("time,ax,ay,az,gx,gy,gz,filterAx,filterAy,filterAz,motion,smoothMotion");
}

void loop() {
  unsigned long now = millis();

  if (now - lastSampleTime >= SAMPLE_INTERVAL) {
    lastSampleTime = now;

    // IMUデータ取得
    IMUData data = imu.readIMU();

    // 重力除去
    IMUData filterData = filter.removeGravity(data);

    // 合成加速度の算出
    float motion = filter.calcMotion(filterData);

    // 平滑化処理
    float smoothMotion = filter.smooth(motion);

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
    Serial.println(smoothMotion);
  }
}
