# Testing LoRa - receiver
#include <LoRa.h>

void setup() {
    Serial.begin(115200);
    LoRa.begin(915E6); // Set frequency to 915 MHz for Australia
    Serial.println("LoRa receiver setup done");
}

void loop() {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        Serial.print("Received packet: ");
        while (LoRa.available()) {
            Serial.print((char)LoRa.read());
        }
        Serial.println();
    }
}

