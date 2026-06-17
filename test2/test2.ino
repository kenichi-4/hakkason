#include <Wire.h>

void setup() {
  Serial.begin(9600);
  delay(1000);

  Wire.begin();
  Wire.setClock(50000);
  Wire.setWireTimeout(3000, true);

  Serial.println("I2C Scanner start");
}

void loop() {
  byte error;
  int count = 0;

  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");

      if (address < 16) {
        Serial.print("0");
      }

      Serial.println(address, HEX);
      count++;
    }
  }

  if (count == 0) {
    Serial.println("No I2C devices found");
  }

  Serial.println("Scan done");
  delay(2000);
}