const awsIot = require('aws-iot-device-sdk');

  // Use your AWS IoT credentials
  const clientId = 'mqttjs_' + Math.random().toString(36).substring(7);
  const mqttClient = awsIot.device({
    region: 'ap-southeast-2',
    host: 'a2hoy94astii3x-ats.iot.ap-southeast-2.amazonaws.com',  // Get this from the AWS IoT console
    clientId: clientId,
    protocol: 'wss',  // WebSocket Secure protocol
    accessKeyId: 'AKIA5V6I66FGB7YZAFAD',
    secretKey: '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',
    keepalive: 30
    
  });

  let map;
const markers = {};
let isMqttInitialized = false;
let livestock;
let topic_name = "hub/pub";

// Function to update cookies
function setCookie(name, value, days) {
  const d = new Date();
  d.setTime(d.getTime() + (days * 24 * 60 * 60 * 1000));
  const expires = "expires=" + d.toUTCString();
  document.cookie = name + "=" + JSON.stringify(value) + ";" + expires + ";path=/";
}

// Function to get cookies
function getCookie(name) {
  const nameEQ = name + "=";
  const ca = document.cookie.split(';');
  for (let i = 0; i < ca.length; i++) {
    let c = ca[i];
    while (c.charAt(0) === ' ') c = c.substring(1);
    if (c.indexOf(nameEQ) === 0) {
      return JSON.parse(c.substring(nameEQ.length, c.length));
    }
  }
  return null;
}

// Function to get icon URL based on animal type
function getAnimalIcon(type) {
  switch (type.toLowerCase()) {
    case 'cow':
      return "https://maps.google.com/mapfiles/ms/icons/blue-dot.png"; // Replace with a custom cow icon URL
    case 'sheep':
      return "https://maps.google.com/mapfiles/ms/icons/yellow-dot.png"; // Replace with a custom sheep icon URL
    case 'horse':
      return "https://maps.google.com/mapfiles/ms/icons/red-dot.png"; // Replace with a custom horse icon URL
    case 'goat':
      return "https://maps.google.com/mapfiles/ms/icons/purple-dot.png"; // Replace with a custom goat icon URL
    default:
      return "https://maps.google.com/mapfiles/ms/icons/green-dot.png"; // Default icon for others
  }
}

// Function to merge new livestock data with the existing one in the cookie
function mergeLivestockData(existingData, newData) {
  const dataMap = {};

  // Add existing data to the map by ID
  existingData.forEach(animal => {
    dataMap[animal.ID] = animal;
  });

  // Add/Update new data into the map
  newData.forEach(animal => {
    dataMap[animal.ID] = animal;
  });

  // Return the merged data as an array
  return Object.values(dataMap);
}

// Function to update map markers
function updateMarkers(livestock) {
  livestock.forEach(animal => {
    if (!markers[animal.ID]) {
      const marker = new google.maps.Marker({
        position: { lat: animal.Lat, lng: animal.Lon },
        map: map,
        title: animal.name,
        icon: {
          url: getAnimalIcon(animal.Type), // Assign icon based on the animal type
        },
      });

      const infoWindow = new google.maps.InfoWindow({
        content: `<h3>ID: ${animal.ID}</h3><p>Type: ${animal.Type}</p><p>Location: (${animal.Lat.toFixed(4)}, ${animal.Lon.toFixed(4)})</p><p>Speed: ${animal.speed.toFixed(2)}</p>`,
      });

      marker.addListener("click", () => {
        infoWindow.open(map, marker);
      });

      markers[animal.ID] = { marker, infoWindow };
    } else {
      markers[animal.ID].marker.setPosition(new google.maps.LatLng(animal.Lat, animal.Lon));
      markers[animal.ID].infoWindow.setContent(
        `<h3>${animal.ID}</h3><p>Type: ${animal.Type}</p><p>Location: (${animal.Lat.toFixed(4)}, ${animal.Lon.toFixed(4)})</p><p>Speed: ${animal.speed.toFixed(2)}</p>`
      );
    }
  });

  // Merge new livestock data with the existing data in the cookie
  let existingLivestock = getCookie('livestock') || [];
  const mergedLivestock = mergeLivestockData(existingLivestock, livestock);

  // Update the cookie with the merged livestock data
  setCookie('livestock', mergedLivestock, 7);
}

// Function to initialize Google Maps
function initMap() {
  map = new google.maps.Map(document.getElementById("map"), {
    center: { lat: -31.97, lng: 115.81 },
    zoom: 13,
    mapTypeId: google.maps.MapTypeId.HYBRID
  });

  // Load livestock positions from cookie if available
  const savedLivestock = getCookie('livestock');
  if (savedLivestock) {
    updateMarkers(savedLivestock);
  }
}

// MQTT message handling
mqttClient.on('connect', () => {
  console.log('Connected to AWS IoT');
  mqttClient.subscribe(topic_name, { qos: 1 }, (err, granted) => {
    if (err) {
      console.error('Subscription error:', err);
    } else {
      console.log('Subscribed to topic:', granted);
    }
  });
});

mqttClient.on('message', (topic, payload) => {
  console.log("Message received on topic:", topic);
  let livestock;
  
  try {
    livestock = JSON.parse(payload.toString());
  } catch (e) {
    console.error("Failed to parse JSON:", e);
  }

  // Ensure livestock is wrapped as an array for compatibility with updateMarkers
  if (!Array.isArray(livestock)) {
    livestock = [livestock];  // Convert to an array if it's not
  }

  updateMarkers(livestock);
});

// Ensure initMap is globally accessible for the Google Maps API callback
window.onload = initMap;
