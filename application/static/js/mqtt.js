const awsIot = require("aws-iot-device-sdk");

// AWS IoT credentials
const clientId = "mqttjs_" + Math.random().toString(36).substring(7);
const mqttClient = awsIot.device({
  region: "ap-southeast-2",
  host: "a2hoy94astii3x-ats.iot.ap-southeast-2.amazonaws.com", 
  clientId: clientId,
  protocol: "wss", 
  accessKeyId: "AKIA5V6I66FGB7YZAFAD",
  secretKey: "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",
  keepalive: 30,
});

let map;  
const markers = {};  
let geofencePolygon = null; 
let topic_name = "hub/pub";  // MQTT topic to subscribe to for livestock data

/**
 * Function to set a cookie with a specified name, value, and expiration days.
 * 
 * @param {string} name - The name of the cookie.
 * @param {any} value - The value to store in the cookie.
 * @param {number} days - The number of days until the cookie expires.
 */
function setCookie(name, value, days) {
  const d = new Date();
  d.setTime(d.getTime() + days * 24 * 60 * 60 * 1000);
  const expires = "expires=" + d.toUTCString();
  document.cookie = name + "=" + JSON.stringify(value) + ";" + expires + ";path=/";
}

/**
 * Function to get the value of a cookie by its name.
 * 
 * @param {string} name - The name of the cookie to retrieve.
 * @returns {any|null} - The value of the cookie or null if not found.
 */
function getCookie(name) {
  const nameEQ = name + "=";
  const ca = document.cookie.split(";");
  for (let i = 0; i < ca.length; i++) {
    let c = ca[i];
    while (c.charAt(0) === " ") c = c.substring(1);
    if (c.indexOf(nameEQ) === 0) {
      return JSON.parse(c.substring(nameEQ.length, c.length));
    }
  }
  return null;
}

/**
 * Function to get the URL of an icon based on the type of animal.
 * 
 * @param {string} type - The type of the animal (e.g., cow, sheep).
 * @returns {string} - The URL of the corresponding animal icon.
 */
function getAnimalIcon(type) {
  switch (type.toLowerCase()) {
    case "cow":
      return "https://maps.google.com/mapfiles/ms/icons/blue-dot.png";  // Icon for cows
    case "sheep":
      return "https://maps.google.com/mapfiles/ms/icons/yellow-dot.png";  // Icon for sheep
    case "horse":
      return "https://maps.google.com/mapfiles/ms/icons/red-dot.png";  // Icon for horses
    case "goat":
      return "https://maps.google.com/mapfiles/ms/icons/purple-dot.png";  // Icon for goats
    default:
      return "https://maps.google.com/mapfiles/ms/icons/green-dot.png";  // Default icon for other animals
  }
}

/**
 * Function to merge new livestock data with existing data stored in cookies.
 * 
 * @param {Array} existingData - The existing livestock data from cookies.
 * @param {Array} newData - The new livestock data received from the MQTT topic.
 * @returns {Array} - The merged array of livestock data.
 */
function mergeLivestockData(existingData, newData) {
  const dataMap = {};

  // Add existing data to the map by ID
  existingData.forEach((animal) => {
    dataMap[animal.ID] = animal;
  });

  // Add/Update new data into the map
  newData.forEach((animal) => {
    dataMap[animal.ID] = animal;
  });

  // Return the merged data as an array
  return Object.values(dataMap);
}

/**
 * Function to check if a given point (latitude and longitude) is inside the geofence.
 * 
 * @param {number} lat - Latitude of the point.
 * @param {number} lng - Longitude of the point.
 * @returns {boolean} - True if the point is inside the geofence, false otherwise.
 */
function isPointInGeofence(lat, lng) {
  if (!geofencePolygon) {
    console.log("No geofence polygon defined.");
    return true; // Assume inside geofence if no geofence is set
  }
  const point = new google.maps.LatLng(lat, lng);
  return google.maps.geometry.poly.containsLocation(point, geofencePolygon);
}

/**
 * Function to update map markers based on livestock data and check geofence status.
 * 
 * @param {Array} livestock - The array of livestock data.
 */
function updateMarkers(livestock) {
  let outsideGeofence = false;

  livestock.forEach((animal) => {
    const position = new google.maps.LatLng(animal.Lat, animal.Lon);

    // Check if the animal's position is inside the geofence
    if (!isPointInGeofence(animal.Lat, animal.Lon)) {
      outsideGeofence = true;
      alert(`Animal ${animal.ID} is outside the geofence!`);
    }

    if (!markers[animal.ID]) {
      const marker = new google.maps.Marker({
        position: position,
        map: map,
        title: animal.name,
        icon: {
          url: getAnimalIcon(animal.Type), // Assign icon based on the animal type
        },
      });

      const infoWindow = new google.maps.InfoWindow({
        content: `<h3>ID: ${animal.ID}</h3><p>Type: ${animal.Type}</p><p>Location: (${animal.Lat.toFixed(4)}, ${animal.Lon.toFixed(4)})</p><p>Speed: ${animal.speed.toFixed(2)}</p>`,
      });

      // Add click listener to show info window
      marker.addListener("click", () => {
        infoWindow.open(map, marker);
      });

      markers[animal.ID] = { marker, infoWindow };  // Store the marker and info window
    } else {
      // Update existing marker position and info window content
      markers[animal.ID].marker.setPosition(position);
      markers[animal.ID].infoWindow.setContent(
        `<h3>${animal.ID}</h3><p>Type: ${animal.Type}</p><p>Location: (${animal.Lat.toFixed(4)}, ${animal.Lon.toFixed(4)})</p><p>Speed: ${animal.speed.toFixed(2)}</p>`
      );
    }
  });

  // Merge new livestock data with existing data in the cookie
  let existingLivestock = getCookie("livestock") || [];
  const mergedLivestock = mergeLivestockData(existingLivestock, livestock);

  // Update the cookie with the merged livestock data
  setCookie("livestock", mergedLivestock, 7);
}

/**
 * Function to initialize the geofence polygon from the server.
 */
function loadGeofence() {
  fetch("/get-geofence", {
    method: "GET",
    headers: {
      "Content-Type": "application/json",
    },
  })
    .then((response) => {
      if (!response.ok) {
        throw new Error("Network response was not ok");
      }
      return response.json();
    })
    .then((data) => {
      if (data.coordinates) {
        let coordinates = JSON.parse(data.coordinates);
        
        coordinates = coordinates.map(coord => {
          const [lat, lng] = coord.split(",");
          return { lat: parseFloat(lat), lng: parseFloat(lng) };
        });

        // Create a geofence polygon on the map
        geofencePolygon = new google.maps.Polygon({
          paths: coordinates,
          editable: false,
          draggable: false,
          fillColor: "#FF0000",
          fillOpacity: 0.35,
          strokeColor: "#FF0000",
          strokeOpacity: 0.8,
          strokeWeight: 2,
        });

        geofencePolygon.setMap(map);  // Set the polygon on the map
      } else {
        console.log("No geofence found for this user.");
      }
    })
    .catch((error) => {
      console.error("Error loading geofence:", error);
    });
}

/**
 * Function to initialize Google Maps and load existing geofence and livestock data.
 */
function initMap() {
  // Initialize the map centered at specified coordinates
  map = new google.maps.Map(document.getElementById("map"), {
    center: { lat: -31.97, lng: 115.81 },
    zoom: 13,
    mapTypeId: google.maps.MapTypeId.HYBRID,
  });

  // Load the geofence polygon if it exists
  loadGeofence();

  // Load livestock positions from the cookie if available
  const savedLivestock = getCookie("livestock");
  if (savedLivestock) {
    updateMarkers(savedLivestock);
  }
}

// MQTT message handling
mqttClient.on("connect", () => {
  console.log("Connected to AWS IoT");
  mqttClient.subscribe(topic_name, { qos: 1 }, (err, granted) => {
    if (err) {
      console.error("Subscription error:", err);
    } else {
      console.log("Subscribed to topic:", granted);
    }
  });
});

// Handle incoming MQTT messages
mqttClient.on("message", (topic, payload) => {
  console.log("Message received on topic:", topic);
  let livestock;

  try {
    livestock = JSON.parse(payload.toString());
  } catch (e) {
    console.error("Failed to parse JSON:", e);
  }

  // Ensure livestock is wrapped as an array for compatibility with updateMarkers
  if (!Array.isArray(livestock)) {
    livestock = [livestock]; // Convert to an array if it's not
  }

  updateMarkers(livestock);  // Update markers based on received livestock data
});

// Ensure initMap is globally accessible for the Google Maps API callback
window.onload = initMap;
