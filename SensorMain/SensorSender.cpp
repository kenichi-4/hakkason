#include "SensorSender.h"

SensorSender::SensorSender() {
  serverIP.fromString(SERVER_IP);
  registered = false;
  ready = false;
}

void SensorSender::begin() {
  startWiFi();
}

void SensorSender::startWiFi() {
  Serial.print("Connecting to sync AP: ");
  Serial.println(secret_ssid);

  WiFi.begin(secret_ssid, secret_pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  serverIP.fromString(SERVER_IP);
}

bool SensorSender::connectToServer() {
  if (client.connect(serverIP, TCP_PORT)) {
    Serial.println("Connected to sync controller");
    return true;
  }
  return false;
}

void SensorSender::sendIdentification() {
  client.println(myname);
  Serial.print("Sent identification: ");
  Serial.println(myname);
}

void SensorSender::updateConnection() {
  if (!client.connected()) {
    ready = false;
    registered = false;

    if (connectToServer()) {
      sendIdentification();
    } else {
      Serial.println("Connection failed. Retrying...");
      delay(5000);
      return;
    }
  }

  if (!registered) {
    String response = receiveLine(2000);
    if (response == "WELCOME") {
      registered = true;
      Serial.println("Registered by sync controller (WELCOME)");
    }
    return;
  }

  if (!ready) {
    String response = receiveLine(2000);
    if (response == "READY") {
      ready = true;
      Serial.println("All devices ready. Sensor control can start.");
    }
  }
}

bool SensorSender::isReady() {
  return ready && client.connected();
}

String SensorSender::receiveLine(unsigned long timeoutMs) {
  unsigned long start = millis();
  String line = "";

  while (millis() - start < timeoutMs) {
    if (client.available()) {
      char ch = client.read();
      if (ch == '\n') {
        line.trim();
        if (line.length() > 0) {
          return line;
        }
      } else if (ch != '\r') {
        line += ch;
      }
    }
    delay(1);
  }

  return "";
}

void SensorSender::sendStart() {
  if (!isReady()) {
    return;
  }

  Serial.println("Send START");
  client.print("START\n");
}

void SensorSender::sendTempo(float bpm, int level) {
  if (!isReady()) {
    return;
  }

  Serial.print("Send tempo level ");
  Serial.print(level);
  Serial.print(" (BPM=");
  Serial.print(bpm);
  Serial.println(")");

  // Current sync controller protocol receives 1, 2, or 3 as one line.
  client.print(String(level) + "\n");
}

void SensorSender::sendStop() {
  if (!isReady()) {
    return;
  }

  Serial.println("Send STOP");

  // ★同期コントローラ側が "STOP" をどう受け取るかは要確認。
  //   もし別の文字列（例 "0" や "END"）を期待するなら、ここを合わせる。
  client.print("STOP\n");
}
