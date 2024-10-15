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
int sleep_duration = 1 * 1000;  // Convert seconds to milliseconds  // Convert minutes to milliseconds (TEST as 1s)
int max_retries = 10;  // Set a reasonable retry count for sending data
bool gps_time_received = false;  // Flag to indicate if GPS time is received

// Create objects
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

// Function to get the number of days in a month
int daysInMonth(int month, int year) {
  if (month == 2) { // February
    // Leap year check: if divisible by 4 and either not divisible by 100 or divisible by 400
    return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 29 : 28;
  }
  // Months with 30 days: April, June, September, November
  else if (month == 4 || month == 6 || month == 9 || month == 11) {
    return 30;
  }
  return 31;  // All other months have 31 days
}

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
  // Normal Mode (to acquire GPS fix faster)
  ss.println("$PMTK161,0*28");  // Disable standby mode (GPS stays on continuously)
  ss.println("$PMTK220,1000*1F");  // Set GPS update rate to 1Hz (update every second for fast GPS fix)
  ss.println("$PMTK300,1000,0,0,0,0*1C"); // Improve fix acquisition time
  ss.println("$PMTK313,1*2E");  // Enable high sensitivity tracking

  Serial.println("Setup complete. Waiting for GPS fix...");
}

String prepareDataToSend(TinyGPSPlus &gps) {
  // Adjust the hour to Perth time (UTC + 8)
  int adjusted_hour = (gps.time.hour() + adjust_hour) % 24;  // Adding +8 hours and handling overflow

  // Handle date change when hour exceeds 24
  int adjusted_day = gps.date.day();
  int adjusted_month = gps.date.month();
  int adjusted_year = gps.date.year();

  // If adjusting the hour moves to the next day
  if (gps.time.hour() + adjust_hour >= 24) {
    adjusted_day += 1;

    // Handle month overflow
    if (adjusted_day > daysInMonth(adjusted_month, adjusted_year)) {
      adjusted_day = 1;
      adjusted_month += 1;

      // Handle year overflow
      if (adjusted_month > 12) {
        adjusted_month = 1;
        adjusted_year += 1;
      }
    }
  }

  // Prepare the data to send
  String utcDateTime = String(adjusted_year) + "-" + String(adjusted_month) + "-" + String(adjusted_day) +
                       " " + String(adjusted_hour) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second());
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
  int retries = 0;
  while (retries < max_retries) {
    if (LoRa.beginPacket()) {
      LoRa.print(data);
      LoRa.endPacket();

      // Print the data to the console
      Serial.print("Data sent: ");
      Serial.println(data);

      return true;  // Assume success if packet was sent
    } else {
      Serial.println("Failed to start LoRa packet. Retrying...");
      retries++;
      delay(1000);  // Wait before retrying

      // Reconnect LoRa if disconnected
      if (!LoRa.begin(BAND)) {
        Serial.println("Reconnecting LoRa...");
        LoRa.end();
        if (!LoRa.begin(BAND)) {
          Serial.println("LoRa reconnection failed!");
        } else {
          Serial.println("LoRa reconnected successfully.");
        }
      }
    }
  }
  return false;  // Failed to send after max retries
}

void loop() {
  // Read GPS data
  while (ss.available() > 0) {
    if (gps.encode(ss.read())) {
      // Show progress while waiting for GPS fix
      if (!gps.location.isValid()) {
        Serial.print(".");
        
        delay(500);  // Show progress dot every 500 ms
      }

      // Check if GPS location is valid (has a fix)
      if (gps.location.isValid() && gps.location.isUpdated() && gps.time.isValid()) {
        // GPS has a valid fix
        Serial.println("\nGPS fix acquired!");
        
        gps_time_received = true;

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

        delay(sleep_duration); // Sleep for 5 seconds before next transmission
      } else {
        // GPS does not have a fix yet
        Serial.println("GPS not fixed yet.");

        // Delay for the sleep period before trying again
        delay(5000);
      }
    }
  }
}
