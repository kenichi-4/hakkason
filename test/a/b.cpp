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

void printMMA8452Q()
{
  accel.read();

  Serial.print(accel.cx, 3);
  Serial.print("\t");
  Serial.print(accel.cy, 3);
  Serial.print("\t");
  Serial.print(accel.cz, 3);
  Serial.println();

  delay(100);
}
