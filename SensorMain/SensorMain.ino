#include "IMUReader.h"
#include "MotionFilter.h"
#include "BeatDetector.h"

IMUReader imu;
MotionFilter filter;
BeatDetector beatDetector;

const unsigned long SAMPLE_INTERVAL = 10;
unsigned long lastSampleTime = 0;

void setup() {
  Serial.begin(9600);

  imu.begin();

  beatDetector.setMotionThreshold(0.10);
  beatDetector.setGyroThreshold(15.0);
  beatDetector.setDownbeatInterval(500);

  Serial.println("time,motion,smoothMotion,gx,gy,gz,output,downbeatPeakValue");
}


 void loop() {
  unsigned long now = millis();

  IMUData data = imu.readIMU();
  IMUData filterData = filter.removeGravity(data);
  float motion = filter.calcMotion(filterData);
  float smoothMotion = filter.smooth(motion);

  bool output = beatDetector.update(data, smoothMotion, now);

  Serial.print(now);
  Serial.print(",");
  Serial.print(smoothMotion);
  Serial.print(",");
  Serial.println(output);

  if (output) {
    Serial.println(beatDetector.getPeakValue());
  } 

  delay(50);
}
    

