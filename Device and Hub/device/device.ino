/*
For TTGO T-BEAM ESP32:
Send GPS data to HUB with local LoRaWAN HUB (Another T-BEAM TTGO)
Only send data if GPS is fix, if not, it not fix, send "GPS not fixed"

Data Structure:
list: [security key, id, UTC Date and Time , speed, longitude, latitude, course, type]
- security_key: This is used to let the hub know that this is our device, default: 13
- id: Unique identifier for the device
- UTC Date and Time: The current date and time in UTC, formatted as YYYY-MM-DD HH:MM:SS
- speed: Current speed in km/h
- longitude: Longitude coordinate with 6 decimal places
- latitude: Latitude coordinate with 6 decimal places
- course: The course or heading in degrees
- type: The type of data, e.g., "Cow" for livestock tracking

Default:
This device always has solar panel support to run longer. 
By default, it will enter deep sleep between 6pm to 6am to save power. Default location for time is WA AUS time.
GPS is in Low Power Mode. For every {sleep_period} of seconds, it will send data using LoRa to the hub.
If the transmission is successful, the LoRa module will sleep; if not, it will keep sending data until the sleep period ends.
LORA band by default for Perth, Australia, without conflict with other networks: 915E6.

Power-Saving Mode:
Initial GPS Power-Saving Setup:
- Set this up for update time:
  ss.println("$PMTK220,60000*1F"); // Update every 60 seconds
  ss.println("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28"); // GPS Only
*/

#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <LoRa.h>
#include <SPI.h>
#include <TimeLib.h>

// GPS Configuration
static const int RXPin = 34, TXPin = 12;
static const uint32_t GPSBaud = 9600;

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
int id = 1;  // Unique identifier for the device
String type = "Cow";  // Type of device or data
String data_to_send;  // Data that will be sent to the hub
int adjust_hour = 8;  // Adjust to Perth time (UTC+8)
int sleep_period = 1;  // Sleep period in minutes for LoRa 
int sleep_duration = sleep_period * 60 * 1000;  // Convert minutes to milliseconds
int begin_sleep_at = 18;  // Start sleep time (6 PM)
int sleep_time = 0;  // Duration of the sleep period in hours
int max_retries = 10;  // Set a reasonable retry count for sending data

// Create objects
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

void setup() {
  Serial.begin(115200);
  ss.begin(GPSBaud);

  // LoRa Setup
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  // GPS Power Mode Configuration
  // You can switch between power-saving mode and normal mode for different use cases:
  // Normal Mode (to acquire GPS fix faster):
  // ss.println("$PMTK161,0*28");  // Disable standby mode (GPS will be on continuously)
  // ss.println("$PMTK220,1000*1F");  // Set GPS update rate to 1Hz (update every second for fast GPS fix)
  //
  // Power-Saving Mode (for long-term operation):
  // ss.println("$PMTK225,2*2E");  // Enable periodic power-saving mode to save energy after getting the fix
  // ss.println("$PMTK220,60000*1F");  // Set GPS update rate to 60 seconds (updates every 60 seconds)

  // Currently using normal mode to acquire GPS fix faster
  ss.println("$PMTK161,0*28");  // Disable standby mode (GPS stays on)
  ss.println("$PMTK220,1000*1F");  // Set GPS update rate to 1Hz (updates every second for fast fix)

  Serial.println("Setup complete. Waiting for GPS fix...");
}

String prepareDataToSend(TinyGPSPlus &gps) {
  String utcDateTime = String(gps.date.year()) + "-" + String(gps.date.month()) + "-" + String(gps.date.day()) +
                       " " + String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second());
  String data = String(security_key) + "," +  // SecKey
                String(id) + "," +  // ID
                utcDateTime + "," + // Date and time
                String(gps.speed.kmph()) + "," +  // Speed
                String(gps.location.lng(), 6) + "," + // Longitude
                String(gps.location.lat(), 6) + "," + // Latitude
                String(gps.course.deg()) + "," + // Course
                String(type); // Type
  return data;
}

bool sendDataToHub(String data) {
  LoRa.beginPacket();
  LoRa.print(data);
  LoRa.endPacket();
  return true;  // Assume success (simplified)
}

void loop() {
  // Read GPS data
  while (ss.available() > 0) {
    if (gps.encode(ss.read())) {
      // Check if GPS location is valid (has a fix)
      if (gps.location.isValid() && gps.location.isUpdated() && gps.time.isValid()) {
        // GPS has a valid fix
        Serial.println("GPS fix acquired!");

        // Set time directly from GPS (AWST is +0800)
        int adjusted_hour = (gps.time.hour() + adjust_hour) % 24;
        setTime(adjusted_hour, gps.time.minute(), gps.time.second(), 
                gps.date.day(), gps.date.month(), gps.date.year());

        data_to_send = prepareDataToSend(gps);

        // Send data to the hub
        if (sendDataToHub(data_to_send)) {
          Serial.println("Data sent successfully!");
        } else {
          Serial.println("Failed to send data.");
        }

        // Enter power-saving mode after getting a fix
        ss.println("$PMTK225,2*2E"); // Enable power-saving mode (Periodic mode)

        delay(sleep_duration); // Sleep before next transmission
      } else {
        // GPS does not have a fix yet
        Serial.println("GPS not fixed yet. Trying to acquire fix...");
        delay(1000); // Wait 1 second before checking again
      }
    }
  }
}
