#include <LoRa.h>
#include <SPI.h>

// LoRa Configuration
#define SCK     5     
#define MISO    19    
#define MOSI    27    
#define SS      18    
#define RST     14    
#define DI0     26    
#define BAND    915E6 

// Other Variables
int security_key = 13;  // Default security key for the device
int id = 1;  // Unique identifier for the device (can be changed for testing other IDs)
String type = "Cow";  // Type of device or data
String data_to_send;  // Data that will be sent to the hub

// Fake GPS Data Configuration
float current_lat = -31.980150;  // Starting latitude for testing
float current_lng = 115.817765;  // Starting longitude for testing
float speed_kmph = 10.0;  // Simulate moving at 10 km/h
float course = 90.0;  // Moving in a straight line east (90 degrees)

void setup() {
  Serial.begin(115200);
  
  // LoRa Setup
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("Setup complete. Sending test GPS data...");
}

// Function to simulate GPS movement
void simulateMovement() {
  float distance_km = speed_kmph / 3600.0;  // Distance traveled in km per second
  float lat_change = distance_km / 111.32;  // Latitude change per km (111.32 km per degree of latitude)
  float lng_change = distance_km / (111.32 * cos(current_lat * PI / 180.0));  // Longitude change per km, adjusted for latitude
  
  // Adjust position based on course (moving east in this case)
  current_lat += lat_change * cos(course * PI / 180.0);
  current_lng += lng_change * sin(course * PI / 180.0);
}

// Function to prepare test data for sending, with placeholders for missing data
String prepareDataToSend() {
  // Prepare data string with placeholders (1) for date, time, speed, and course
  String data = String(security_key) + "," +  // SecKey
                String(id) + "," +  // ID
                "1,1," + // Date and time (placeholder value 1)
                "1," +  // Speed (placeholder value 1)
                String(current_lng, 6) + "," + // Longitude (6 decimal places)
                String(current_lat, 6) + "," + // Latitude (6 decimal places)
                "1," +  // Course (placeholder value 1)
                String(type); // Type (e.g., "Cow")
  return data;
}

bool sendDataToHub(String data) {
  LoRa.beginPacket();
  LoRa.print(data);
  LoRa.endPacket();
  return true;  // Assume success (simplified)
}

void loop() {
  // Simulate movement for the next GPS update
  simulateMovement();

  // Prepare the data to send
  data_to_send = prepareDataToSend();

  // Send data to the hub
  if (sendDataToHub(data_to_send)) {
    Serial.print("Test Data sent: ");
    Serial.println(data_to_send);
  } else {
    Serial.println("Failed to send data.");
  }

  delay(10000);  // Wait 10 seconds between transmissions for testing
}
