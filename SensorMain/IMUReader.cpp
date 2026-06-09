#include "IMUReader.h"

IMUReader::IMUReader() {
}

// MPU6050の初期化
bool IMUReader::begin() {
  Wire.begin();

  // スリープ解除
  writeRegister(0x6B, 0x00);

  // 加速度レンジ ±2g
  writeRegister(0x1C, 0x00);

  // ジャイロレンジ ±250 deg/s
  writeRegister(0x1B, 0x00);

  return true;
}

// 加速度・角速度データを取得
IMUData IMUReader::readIMU() {
  IMUData data;

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);  // ACCEL_XOUT_H から読み取り開始
  Wire.endTransmission(false);

  // 加速度6バイト + 温度2バイト + 角速度6バイト = 14バイト
  Wire.requestFrom(MPU_ADDR, (uint8_t)14, (uint8_t)true);

  int16_t rawAx = Wire.read() << 8 | Wire.read();
  int16_t rawAy = Wire.read() << 8 | Wire.read();
  int16_t rawAz = Wire.read() << 8 | Wire.read();

  // 温度データは使用しないため読み飛ばす
  Wire.read();
  Wire.read();

  int16_t rawGx = Wire.read() << 8 | Wire.read();
  int16_t rawGy = Wire.read() << 8 | Wire.read();
  int16_t rawGz = Wire.read() << 8 | Wire.read();

  // ±2g設定時：16384 LSB/g
  data.ax = rawAx / 16384.0;
  data.ay = rawAy / 16384.0;
  data.az = rawAz / 16384.0;

  // ±250 deg/s設定時：131 LSB/(deg/s)
  data.gx = rawGx / 131.0;
  data.gy = rawGy / 131.0;
  data.gz = rawGz / 131.0;

  return data;
}

// レジスタへ値を書き込む
void IMUReader::writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission(true);
}
