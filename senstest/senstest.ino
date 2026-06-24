#include "Sensfunc.h"
#include "config.h"
#include <WiFiUdp.h>

/*
 * SensClient 合計遅延テスト用スケッチ (Loopback方式)
 */

SensClass sens;
WiFiUDP udp;
unsigned long sendTime = 0;
bool waitingForLoopback = false;

void setup() {
    sens.Starts();
    udp.begin(UDP_PORT); // 楽器と同じポートを監視
    Serial.println("================================");
    Serial.println("SensClient: Total Delay Test (Loopback)");
    Serial.println("Input 's' or '1'-'3' to measure delay");
    Serial.println("================================");
}

void loop() {
    // 1. 同期機からのUDPループバックを監視
    int packetSize = udp.parsePacket();
    if (packetSize) {
        unsigned long recvTime = millis();
        char buf[255];
        int len = udp.read(buf, 255);
        if (len > 0) buf[len] = 0;
        
        if (waitingForLoopback) {
            Serial.print(">>> TOTAL DELAY (Sens -> Sync -> Sens): ");
            Serial.print(recvTime - sendTime);
            Serial.print(" ms (Message: ");
            Serial.print(buf);
            Serial.println(")");
            waitingForLoopback = false;
        }
    }

    if (sens.ready == 0){
        sens.connection();
    } 
    else {
        // コマンド送信と送信時刻の記録
        if (Serial.available() > 0) {
            char cmd = Serial.read();
            if (cmd == 's' || cmd == '1' || cmd == '2' || cmd == '3') {
                sendTime = millis();
                waitingForLoopback = true;
                
                if (cmd == 's') {
                    sens.sendStart();
                } else {
                    sens.sendSpeed(cmd - '0');
                }
            }
        }
    }
}
