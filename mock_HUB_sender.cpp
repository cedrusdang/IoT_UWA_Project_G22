#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";
const char*   
 api_gateway_url = "https://your-api-id.execute-api.your-region.amazonaws.com/your-stage/gps";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");   

}

void loop() {
  // Get GPS data (replace with your actual GPS reading logic)
  float latitude = 47.6062; 
  float longitude = -122.3321;

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(api_gateway_url);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument doc;
    doc["latitude"] = latitude;
    doc["longitude"] = longitude;
    String json_data;
    serializeJson(doc, json_data);

    int httpResponseCode = http.POST(json_data);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }

  delay(5000);   
 // Send data every 5 seconds (adjust as needed)
}
