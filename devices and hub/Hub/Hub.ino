//T-BEAM TTGO HUB For LORAWAN. Connect to AWS
#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <LoRa.h>
#include <SPI.h>

#define AWS_IOT_PUBLISH_TOPIC "hub/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "hub/sub"

// LoRa Configuration
#define SCK     5     
#define MISO    19    
#define MOSI    27    
#define SS      18    
#define RST     14    
#define DI0     26    
#define BAND    915E6 

// Security Key
int expected_security_key = 13;

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  client.setServer(AWS_IOT_ENDPOINT, 8883);
  client.setCallback(messageHandler);

  Serial.println("Connecting to AWS IOT");

  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(1);
  }

  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }

  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");

  // Publish "Hub connected" message
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["message"] = "Hub connected";
  char jsonBuffer[512];
  serializeJson(jsonDoc, jsonBuffer);
  publishMessage(jsonBuffer);
}

// Function to publish JSON message
void publishMessage(const char* jsonPayload) {
  if (client.publish(AWS_IOT_PUBLISH_TOPIC, jsonPayload)) {
    Serial.print("Published JSON Payload: ");
  } else {
    Serial.print("Failed to publish JSON Payload: ");
  }
  Serial.println(jsonPayload);
}

void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Incoming message: ");
  Serial.println(topic);

  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // LoRa Setup
  Serial.println("LoRa Receiver");
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  // AWS IoT Setup
  connectAWS(); 
}

void loop() {
  // Check for LoRa packets
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String receivedData = "";
    while (LoRa.available()) {
      receivedData += (char)LoRa.read();
    }
    processReceivedData(receivedData); 
  }

  client.loop();
  delay(1); 
}

void processReceivedData(String data) {
  // Split the data into its components
  int commaIndex = data.indexOf(',');
  int received_security_key = data.substring(0, commaIndex).toInt();

  // Check security key
  if (received_security_key == expected_security_key) {
    Serial.println("Security key matched, processing data...");
    String remainingData = data.substring(commaIndex + 1);
    int idCommaIndex = remainingData.indexOf(',');
    String id = remainingData.substring(0, idCommaIndex);
    String message = remainingData.substring(idCommaIndex + 1);

    // Extract individual fields
    String dataList[7]; 
    int lastCommaIndex = idCommaIndex;
    int currentCommaIndex;

    for (int i = 0; i < 6; i++) { 
      currentCommaIndex = message.indexOf(',', lastCommaIndex + 1);
      dataList[i] = message.substring(lastCommaIndex + 1, currentCommaIndex);
      lastCommaIndex = currentCommaIndex;
    }

    dataList[6] = message.substring(lastCommaIndex + 1);

    // Get RSSI
    int rssi = LoRa.packetRssi();

    // Create JSON object
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["ID"] = id;                      // Device ID
    jsonDoc["UTC DateTime"] = dataList[0];   // UTC Date and Time
    jsonDoc["speed"] = dataList[1].toFloat(); // Speed in km/h
    jsonDoc["Lon"] = dataList[2].toFloat();   // Longitude
    jsonDoc["Lat"] = dataList[3].toFloat();   // Latitude
    jsonDoc["Course"] = dataList[4].toFloat();// Course in degrees
    jsonDoc["Type"] = dataList[5];            // Type of data (e.g., "Cow")
    jsonDoc["RSSI"] = rssi;                   // RSSI from LoRa packet

    // Serialize JSON to a string
    char jsonBuffer[512];
    serializeJson(jsonDoc, jsonBuffer);

    // Print and publish the JSON payload
    Serial.print("Received Data (JSON): ");
    Serial.println(jsonBuffer);
    publishMessage(jsonBuffer);

    // Send an acknowledgment back to the sender
    sendAcknowledgment();

    // Confirmation print
    Serial.println("Data successfully processed.");
  } else {
    Serial.println("Invalid security key. Ignoring data.");
  }
}

void sendAcknowledgment() {
  LoRa.beginPacket();
  LoRa.print("ACK");
  LoRa.endPacket();
  Serial.println("Acknowledgment sent to the sender.");
}
