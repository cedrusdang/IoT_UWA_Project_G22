#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <math.h>

#define AWS_IOT_PUBLISH_TOPIC "hub/pub"

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

// GPS Center Coordinates (latitude, longitude)
float centerLat = -31.980275;
float centerLon = 115.817848;

// Animal data
struct Animal {
  int id;            // Integer ID instead of string
  String type;       // Type of animal
  float lat;
  float lon;
  float speed;       // km/h
  float distanceMoved; // km
};

// Initialize a single animal with an integer ID and specific type
Animal animal = {1, "Cow", centerLat, centerLon, 10.0, 0.0};

unsigned long lastSendTime = 0;
const unsigned long delayBetweenSends = 35000; // 35 seconds

void connectAWS() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  client.setServer(AWS_IOT_ENDPOINT, 8883);

  Serial.println("Connecting to AWS IoT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }

  Serial.println("AWS IoT Connected!");
}

// Function to publish JSON message
void publishMessage(const char* jsonPayload) {
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonPayload);
  Serial.print("Published JSON Payload: ");
  Serial.println(jsonPayload);
}

// Function to move the animal
void moveAnimal(Animal &animal) {
  // Move animal by 100 meters (10 km/h)
  float distanceStep = animal.speed / 3600; // Convert km/h to km/sec
  animal.lat += distanceStep * 0.0009; // Rough conversion for latitude
  animal.lon += distanceStep * 0.0011; // Rough conversion for longitude
  animal.distanceMoved += distanceStep;

  // Stop moving after 10 km
  if (animal.distanceMoved >= 10.0) {
    animal.speed = 0; // Stop the animal
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // AWS IoT Setup
  connectAWS();
}

void loop() {
  // Check if enough time has passed since the last message
  if (millis() - lastSendTime >= delayBetweenSends) {
    // Move the animal
    moveAnimal(animal);

    // Create JSON object for the animal
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["ID"] = animal.id;  // Use integer ID
    jsonDoc["UTC DateTime"] = "Simulation";          // Simulation date and time
    jsonDoc["speed"] = animal.speed;  // Speed in km/h
    jsonDoc["Lon"] = animal.lon;      // Longitude
    jsonDoc["Lat"] = animal.lat;      // Latitude
    jsonDoc["Course"] = "Simulation";               // Course set to "Simulation"
    jsonDoc["Type"] = animal.type;    // Animal type (Cow)

    // Serialize JSON to a string
    char jsonBuffer[512];
    serializeJson(jsonDoc, jsonBuffer);

    // Publish the JSON payload to AWS IoT Core
    publishMessage(jsonBuffer);

    // Update the last send time
    lastSendTime = millis();
  }

  client.loop();
}