#include "b.h"
#include <Wire.h>
#include <SparkFun_MMA8452Q.h>

MMA8452Q accel;

void setupMMA8452Q()
{
  Serial.begin(9600);
  Serial.println("MMA8452Q test");

  accel.init();
}

void readMMA8452Q(float &x, float &y, float &z)
{
  accel.read();

  x = accel.cx;
  y = accel.cy;
  z = accel.cz;
}
