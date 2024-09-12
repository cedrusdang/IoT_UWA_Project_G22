# tesing LoRa Communication - Sender
#include <LoRa.h>

void setup() {
    Serial.begin(115200);
    LoRa.begin(915E6); // Set frequency to 915 MHz for Australia
    Serial.println("LoRa sender setup done");
}

void loop() {
    Serial.println("Sending packet...");
    LoRa.beginPacket();
    LoRa.print("Hello, LoRa!");
    LoRa.endPacket();
    delay(1000);
}

