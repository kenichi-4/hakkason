#include "MPU6050Data.h"
#include <Wire.h>


const int MPU_ADDR = 0x68;


const float ACCEL_SCALE = 16384.0;

void setupMPU6050()
{
  Serial.begin(9600);
  Serial.println("MPU6050 test");

  Wire.begin();

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);    
  Wire.write(0x00);     
  Wire.endTransmission(true);


  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1C);     
  Wire.write(0x00);      
  Wire.endTransmission(true);
}

void readMPU6050Accel(float &ax, float &ay, float &az)
{
  int16_t axRaw, ayRaw, azRaw;


  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);     
  Wire.endTransmission(false);

  Wire.requestFrom(MPU_ADDR, 6, true);

  axRaw = Wire.read() << 8 | Wire.read();
  ayRaw = Wire.read() << 8 | Wire.read();
  azRaw = Wire.read() << 8 | Wire.read();


  ax = axRaw / ACCEL_SCALE;
  ay = ayRaw / ACCEL_SCALE;
  az = azRaw / ACCEL_SCALE;
}
