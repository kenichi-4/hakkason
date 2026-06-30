#ifndef SENSOR_SENDER_H
#define SENSOR_SENDER_H

#include <Arduino.h>
#include <WiFiS3.h>
#include "config.h"

class SensorSender {
public:
  SensorSender();

  void begin();
  void updateConnection();
  bool isReady();

  void sendStart();
  void sendTempo(float bpm, int level);
  void sendStop();

private:
  WiFiClient client;
  IPAddress serverIP;

  bool registered;
  bool ready;

  void startWiFi();
  bool connectToServer();
  void sendIdentification();
  String receiveLine(unsigned long timeoutMs);
};

#endif
