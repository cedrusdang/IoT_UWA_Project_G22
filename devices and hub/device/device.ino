#include <TinyGPS++.h> 
#include <SoftwareSerial.h> 
#include <LoRa.h> 
#include <SPI.h> 
#include <TimeLib.h> 
#include <esp_sleep.h> 

// GPS Configuration 
static const int RXPin = 34, TXPin = 12; 
static const uint32_t GPSBaud = 9600; 

// LoRa Configuration 
#define SCK     5     // Serial Clock pin for LoRa (SPI) 
#define MISO    19    // Master In Slave Out pin for LoRa (SPI) 
#define MOSI    27    // Master Out Slave In pin for LoRa (SPI) 
#define SS      18    // Slave Select pin for LoRa (SPI) 
#define RST     14    // Reset pin for LoRa 
#define DI0     26    // Digital I/O 0 pin for LoRa 
#define BAND    915E6 // Set LoRa frequency to 915 MHz (Australia's frequency band) 

// Other Variables 
int security_key = 13;  // Default security key for the device 
int id = 1;  // Unique identifier for the device 
String type = "Cow";  // Type of device or data 
String data_to_send;  // Data that will be sent to the hub 
int adjust_hour = 8;  // Adjust to Perth time (UTC+8) 
int sleep_duration = 5 * 1000;  // Sleep duration in milliseconds 
int max_retries = 10;  // Set a reasonable retry count for sending data 
bool gps_time_received = false;  // Flag to indicate if GPS time is received 

int deep_sleep_start_hour = 6;  // Start deep sleep at 6 AM 
int deep_sleep_end_hour = 18;   // End deep sleep at 6 PM

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
  // GPS Power Mode Configuration - Full Power for initial fix 
  ss.println("$PMTK161,0*28");  // Disable standby mode (GPS stays on continuously) 
  ss.println("$PMTK220,30000*1B");  // Set GPS update rate to 0.2Hz (update every 30 seconds to save energy) 
  Serial.println("Setup complete. Waiting for GPS fix..."); 
} 

String prepareDataToSend(TinyGPSPlus &gps) { 
  // Adjust the hour to Perth time (UTC + 8) 
  int adjusted_hour = (gps.time.hour() + adjust_hour) % 24;  
  // Prepare the data to send 
  String utcDateTime = String(gps.date.year()) + "-" + String(gps.date.month()) + "-" + String(gps.date.day()) +
                       " " + String(adjusted_hour) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second()); 
  String data = String(security_key) + "," +  
                String(id) + "," +  
                utcDateTime + "," + 
                String(gps.speed.kmph()) + "," +  
                String(gps.location.lng(), 6) + "," + 
                String(gps.location.lat(), 6) + "," +  
                String(gps.course.deg()) + "," + 
                String(type); 
  return data; 
} 

bool sendDataToHub(String data) { 
  int retries = 0; 
  while (retries < max_retries) { 
    // Check LoRa status and reconnect if needed 
    if (!LoRa.begin(BAND)) { 
      Serial.println("Reconnecting LoRa..."); 
      LoRa.end(); 
      if (!LoRa.begin(BAND)) { 
        Serial.println("LoRa reconnection failed!"); 
      } else { 
        Serial.println("LoRa reconnected successfully."); 
      } 
    } 

    if (LoRa.beginPacket()) { 
      LoRa.print(data); 
      LoRa.endPacket(); 

      // Print the data to the console 
      Serial.println("Data sent: "); 
      Serial.println(data); 

      return true;  // Assume success if packet was sent 
    } else { 
      Serial.println("Failed to start LoRa packet. Retrying..."); 
      retries++; 
      delay(1000);  // Wait before retrying 
    } 
  } 
  return false;  // Failed to send after max retries 
} 

void loop() {
  // Get the current hour
  int current_hour = hour();

  // Check if it is within deep sleep hours
  if (current_hour >= deep_sleep_start_hour && current_hour < deep_sleep_end_hour) {
    Serial.println("Within deep sleep hours. Going to sleep...");
    esp_sleep_enable_timer_wakeup(12 * 60 * 60 * 1000000LL); // Sleep for 12 hours (or until external wake-up)
    esp_deep_sleep_start();
  }

  // Read GPS data
  while (ss.available() > 0) {
    if (gps.encode(ss.read())) {
      // Check if GPS location is valid (has a fix)
      if (gps.location.isValid() && gps.location.isUpdated() && gps.time.isValid()) {
        Serial.print("GPS fix acquired!");
        gps_time_received = true;

        // Set time directly from GPS (AWST is +0800)
        int adjusted_hour = (gps.time.hour() + adjust_hour) % 24;
        setTime(adjusted_hour, gps.time.minute(), gps.time.second(),
                gps.date.day(), gps.date.month(), gps.date.year());
        data_to_send = prepareDataToSend(gps);
        
        // Send data to the hub
        if (sendDataToHub(data_to_send)) {
          Serial.println("Data sent successfully!");
          ss.println("$PMTK225,2*2E");  // Enable power-saving mode
        } else {
          Serial.print("Failed to send data");
        }

        delay(sleep_duration);
      } else {
        Serial.print("GPS not fixed yet");
      }
    }
  }
}
