#ifndef IMU_READER_H
#define IMU_READER_H

#include <Arduino.h>
#include <Wire.h>

// IMUから取得したデータをまとめる構造体
struct IMUData {
  float ax;
  float ay;
  float az;

  float gx;
  float gy;
  float gz;
};

class IMUReader {
public:
  IMUReader();

  // MPU6050の初期化
  bool begin();

  // 加速度・角速度データを取得
  IMUData readIMU();

private:
  const uint8_t MPU_ADDR = 0x68;

  // MPU6050のレジスタへ値を書き込む
  void writeRegister(uint8_t reg, uint8_t value);
};

#endif
