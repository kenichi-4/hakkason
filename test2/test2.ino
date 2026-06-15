#include <Wire.h>

const int MPU_addr = 0x68;

int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

void setup() {
  Serial.begin(9600);
  delay(1000);

  Wire.begin();
  Wire.setClock(50000);  // 通信を少し安定させるため遅めにする

  initMPU6050();

  Serial.println("AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ");
}

void loop() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);

  byte error = Wire.endTransmission(false);

  if (error != 0) {
    Serial.print("register set error = ");
    Serial.println(error);

    // 通信が失敗したら再初期化
    initMPU6050();

    delay(100);
    return;
  }

  int received = Wire.requestFrom(MPU_addr, 14, true);

  if (received != 14) {
    Serial.print("read failed received = ");
    Serial.println(received);

    // 失敗したら今回は読まない
    initMPU6050();

    delay(100);
    return;
  }

  AcX = (Wire.read() << 8) | Wire.read();
  AcY = (Wire.read() << 8) | Wire.read();
  AcZ = (Wire.read() << 8) | Wire.read();
  Tmp = (Wire.read() << 8) | Wire.read();
  GyX = (Wire.read() << 8) | Wire.read();
  GyY = (Wire.read() << 8) | Wire.read();
  GyZ = (Wire.read() << 8) | Wire.read();

  Serial.print("AcX = "); Serial.print(AcX);
  Serial.print(" | AcY = "); Serial.print(AcY);
  Serial.print(" | AcZ = "); Serial.print(AcZ);
  Serial.print(" | Tmp = "); Serial.print(Tmp / 340.00 + 36.53);
  Serial.print(" | GyX = "); Serial.print(GyX);
  Serial.print(" | GyY = "); Serial.print(GyY);
  Serial.print(" | GyZ = "); Serial.println(GyZ);

  delay(100);
}

void initMPU6050() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0x00);
  byte error = Wire.endTransmission(true);

  if (error == 0) {
    Serial.println("MPU6050 init OK");
  } else {
    Serial.print("MPU6050 init error = ");
    Serial.println(error);
  }

  delay(50);
}