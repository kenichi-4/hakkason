#include "IMUReader.h"
#include "MotionFilter.h"

const unsigned long SAMPLE_INTERVAL = 10;
unsigned long lastSampleTime = 0;

void setup() {
  Serial.begin(115200);

  setupIMU();

  Serial.println("time,ax,ay,az,gx,gy,gz,filterAx,filterAy,filterAz,totalAccel,smoothAccel");
}

void loop() {
  unsigned long now = millis();

  if (now - lastSampleTime >= SAMPLE_INTERVAL) {
    lastSampleTime = now;

    // IMUから加速度・角速度を取得
    readIMU();

    // 加速度データから重力成分を除去
    removeGravity(ax, ay, az);

    // 3軸加速度から合成加速度を算出
    calcMotion();

    // 合成加速度を平滑化
    smoothMotion();

    Serial.print(now);
    Serial.print(",");

    Serial.print(ax);
    Serial.print(",");
    Serial.print(ay);
    Serial.print(",");
    Serial.print(az);
    Serial.print(",");

    Serial.print(gx);
    Serial.print(",");
    Serial.print(gy);
    Serial.print(",");
    Serial.print(gz);
    Serial.print(",");

    Serial.print(filterAx);
    Serial.print(",");
    Serial.print(filterAy);
    Serial.print(",");
    Serial.print(filterAz);
    Serial.print(",");

    Serial.print(totalAccel);
    Serial.print(",");
    Serial.println(smoothAccel);
  }
}
