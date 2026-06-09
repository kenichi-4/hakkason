#include "IMUReader.h"

// 加速度データ
float ax = 0.0;
float ay = 0.0;
float az = 0.0;

// 角速度データ
float gx = 0.0;
float gy = 0.0;
float gz = 0.0;

// IMUセンサの初期化
void setupIMU() {
  Wire.begin();

  // MPU6050のスリープ解除
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);      // PWR_MGMT_1 レジスタ
  Wire.write(0x00);      // スリープ解除
  Wire.endTransmission(true);

  // 加速度レンジを ±2g に設定
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1C);      // ACCEL_CONFIG レジスタ
  Wire.write(0x00);      // ±2g
  Wire.endTransmission(true);

  // ジャイロレンジを ±250 deg/s に設定
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1B);      // GYRO_CONFIG レジスタ
  Wire.write(0x00);      // ±250 deg/s
  Wire.endTransmission(true);
}

// IMUセンサから加速度・角速度を取得
void readIMU() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);      // ACCEL_XOUT_H から読み取り開始
  Wire.endTransmission(false);

  // 加速度6バイト + 温度2バイト + 角速度6バイト = 14バイト
  Wire.requestFrom(MPU_ADDR, 14, true);

  int16_t rawAx = Wire.read() << 8 | Wire.read();
  int16_t rawAy = Wire.read() << 8 | Wire.read();
  int16_t rawAz = Wire.read() << 8 | Wire.read();

  // 温度データは使わないので読み飛ばす
  Wire.read();
  Wire.read();

  int16_t rawGx = Wire.read() << 8 | Wire.read();
  int16_t rawGy = Wire.read() << 8 | Wire.read();
  int16_t rawGz = Wire.read() << 8 | Wire.read();

  // ±2g設定時：16384 LSB/g
  ax = rawAx / 16384.0;
  ay = rawAy / 16384.0;
  az = rawAz / 16384.0;

  // ±250deg/s設定時：131 LSB/(deg/s)
  gx = rawGx / 131.0;
  gy = rawGy / 131.0;
  gz = rawGz / 131.0;
}
