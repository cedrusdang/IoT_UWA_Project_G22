import json

# define the geofence
GEOFENCE = {
    'latitude_min': -31.9505,   # example: Perth, WA
    'latitude_max': -31.9405,
    'longitude_min': 115.8605,
    'longitude_max': 115.8705
}

def lambda_handler(event, context):
    # get the GPS data from the event
    gps_data = event['gps_data']
    device_id = gps_data['device_id']
    latitude = gps_data['latitude']
    longitude = gps_data['longitude']
    
    # Check if the cow is inside the geofence
    in_geofence = (
        GEOFENCE['latitude_min'] <= latitude <= GEOFENCE['latitude_max'] and
        GEOFENCE['longitude_min'] <= longitude <= GEOFENCE['longitude_max']
    )

    if not in_geofence:
        # action: notify the user
        notify_user(device_id, latitude, longitude)

    return {
        'statusCode': 200,
        'body': json.dumps(f'Device {device_id} is {"inside" if in_geofence else "outside"} the geofence.')
    }

def notify_user(device_id, latitude, longitude):
    # integrate with a notification service
    print(f"Device {device_id} is out of bounds! Location: ({latitude}, {longitude})")
