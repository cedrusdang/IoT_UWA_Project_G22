const awsIot = require('aws-iot-device-sdk');

  // Use your AWS IoT credentials
  const clientId = 'mqttjs_' + Math.random().toString(36).substring(7);
  const mqttClient = awsIot.device({
    region: 'ap-southeast-2',
    host: 'a2hoy94astii3x-ats.iot.ap-southeast-2.amazonaws.com',  // Get this from the AWS IoT console
    clientId: "hub2",
    protocol: 'wss',  // WebSocket Secure protocol
    accessKeyId: 'AKIA5V6I66FGFEWPKXSJ',
    secretKey: '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',
   // keepalive: '30'
    
  });

let map;
const markers = {};
let isMqttInitialized = false;
let livestock;

// Function to update map markers
function updateMarkers(livestock) {
  livestock.forEach(animal => {
    if (!markers[animal.id]) {
      const marker = new google.maps.Marker({
        position: { lat: animal.lat, lng: animal.lng },
        map: map,
        title: animal.name,
        icon: {
          url: animal.type === "Cow"
            ? "https://maps.google.com/mapfiles/ms/icons/blue-dot.png"
            : "https://maps.google.com/mapfiles/ms/icons/green-dot.png",
        },
      });

      const infoWindow = new google.maps.InfoWindow({
        content: `<h3>${animal.name}</h3><p>Type: ${animal.type}</p><p>Location: (${animal.lat.toFixed(4)}, ${animal.lng.toFixed(4)})</p>`,
      });

      marker.addListener("click", () => {
        infoWindow.open(map, marker);
      });

      markers[animal.id] = { marker, infoWindow };
    } else {
      markers[animal.id].marker.setPosition(new google.maps.LatLng(animal.lat, animal.lng));
      markers[animal.id].infoWindow.setContent(
        `<h3>${animal.name}</h3><p>Type: ${animal.type}</p><p>Location: (${animal.lat.toFixed(4)}, ${animal.lng.toFixed(4)})</p>`
      );
    }
  });
}

// Function to initialize Google Maps
function initMap() {
  map = new google.maps.Map(document.getElementById("map"), {
    center: { lat: -31.97, lng: 115.81 },
    zoom: 13,
    mapTypeId: google.maps.MapTypeId.HYBRID
  });
  
 // initMqtt(updateMarkers); 
}
mqttClient.on('connect', () => {
    console.log('Connected to AWS IoT');
    mqttClient.subscribe('livestock', { qos: 1 }, (err, granted) => {
        if (err) {
            console.error('Subscription error:', err);
        } else {
            console.log('Subscribed to topic:', granted); // Check if granted array is non-empty
        }
    });
   // mqttClient.publish('livestock', JSON.stringify({ test_data: 1}));
});
mqttClient.on('message', (topic, payload) => {
    console.log("Message received on topic:", topic);
    livestock = JSON.parse(payload.toString());
    //console.log('Message payload:', payload.toString());
    updateMarkers(livestock);
  //  onMessageReceived(livestock); 
});/*
mqttClient.on('receive', (topic, payload) => {
    console.log("Message received on topic:", topic);
    livestock = JSON.parse(payload.toString());
    console.log('Message payload:', payload.toString());
    updateMarkers(livestock);
  //  onMessageReceived(livestock); 
});*/
// // Function to initialize MQTT only once
// function initMqtt(onMessageReceived) {
//     if (isMqttInitialized) {
//         console.log('MQTT client is already initialized.');
//         return;
//     }

    

    

//     mqttClient.on('error', (error) => {
//         console.error('MQTT connection error:', error);
//     });

//     isMqttInitialized = true;
// }

// Ensure initMap is globally accessible for the Google Maps API callback
window.initMap = initMap;