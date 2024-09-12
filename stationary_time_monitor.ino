#include <math.h>  // Required for distance calculation
#include <TinyGPS++.h>  // GPS library

TinyGPSPlus gps;

// Variables to store the last recorded GPS coordinates
float lastLat = 0.0;
float lastLon = 0.0;

// Last time the cow moved
unsigned long lastMoveTime = 0;

// Threshold for stationary time in milliseconds (e.g., 1 hour)
const unsigned long stationaryThreshold = 3600000;  // 1 hour = 60 minutes * 60 seconds * 1000 ms

// Threshold for movement in meters (to ignore small movements due to GPS inaccuracy)
const float movementThreshold = 5.0;  // Ignore movement under 5 meters

// Function to calculate the distance between two points using the Haversine formula (returns meters)
float calculateDistance(float lat1, float lon1, float lat2, float lon2) {
    const float R = 6371000;  // Earth's radius in meters
    float dLat = radians(lat2 - lat1);
    float dLon = radians(lon2 - lon1);

    lat1 = radians(lat1);
    lat2 = radians(lat2);

    float a = sin(dLat / 2) * sin(dLat / 2) +
              cos(lat1) * cos(lat2) *
              sin(dLon / 2) * sin(dLon / 2);
    float c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return R * c;  // Distance in meters
}

// Function to monitor the cow's movement
void monitorMovement(float currentLat, float currentLon) {
    // Calculate the distance between the current and last recorded position
    float distance = calculateDistance(lastLat, lastLon, currentLat, currentLon);

    // If the cow hasn't moved beyond the threshold, check how long it's been stationary
    if (distance < movementThreshold) {
        unsigned long currentTime = millis();  // Get the current time in milliseconds

        // If the cow has been stationary for longer than the threshold, trigger a warning
        if (currentTime - lastMoveTime > stationaryThreshold) {
            Serial.println("WARNING: Cow has been stationary for over 1 hour.");
            // Trigger a notification here, e.g., SMS, push notification, etc.
        }
    } else {
        // If the cow has moved, reset the timer and update the last recorded position
        lastMoveTime = millis();  // Update the time the cow last moved
        lastLat = currentLat;  // Update last latitude
        lastLon = currentLon;  // Update last longitude
        Serial.println("Cow has moved. Resetting stationary timer.");
    }
}

void setup() {
    Serial.begin(115200);
    // Initialize the GPS module (using Serial1 for GPS)
    Serial1.begin(9600);

    // Set the lastMoveTime to the current time at startup
    lastMoveTime = millis();
}

void loop() {
    // Check if there is new GPS data available
    while (Serial1.available() > 0) {
        gps.encode(Serial1.read());

        // When a new GPS location is available, process the current coordinates
        if (gps.location.isUpdated()) {
            float currentLat = gps.location.lat();
            float currentLon = gps.location.lng();

            // Output the current coordinates for debugging
            Serial.print("Current Latitude: ");
            Serial.println(currentLat, 6);
            Serial.print("Current Longitude: ");
            Serial.println(currentLon, 6);

            // Call the movement monitoring function
            monitorMovement(currentLat, currentLon);
        }
    }

    // Adjust the delay according to your requirements (e.g., check every 10 seconds)
    delay(10000);  // Check every 10 seconds
}

