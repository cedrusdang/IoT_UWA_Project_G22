import json
from db_operations import insert_gps_data 

def get_gps_data(event):
    try:
        # Assuming the body contains a JSON string, convert it to a Python dictionary
        body = json.loads(event['body']) if 'body' in event else {}

        # Extract values from the payload
         device_id = body.get('ID', 0)
        utc_datetime = body.get('UTC DateTime', '')
        speed = body.get('speed', 0.0)
        longitude = body.get('Lon', 0.0)
        latitude = body.get('Lat', 0.0)
        course = body.get('Course', 0.0)
        data_type = body.get('Type', '')
        rssi = body.get('RSSI', 0)

        # Print data
        print(f"Parsed data - ID: {device_id}, UTC DateTime: {utc_datetime}, Speed: {speed}, "
              f"Longitude: {longitude}, Latitude: {latitude}, Course: {course}, Type: {data_type}, RSSI: {rssi}")

        # Return parsed data as a dictionary
        return {
           'device_id': device_id,
            'utc_datetime': utc_datetime,
            'speed': speed,
            'longitude': longitude,
            'latitude': latitude,
            'course': course,
            'data_type': data_type,
            'rssi': rssi
        }

    except Exception as e:
        print(f"Error parsing GPS data: {e}")
        raise e

# Main Lambda handler function
def lambda_handler(event, context):
    try:
        #Get GPS data from the event
        gps_data = get_gps_data(event)

        #Insert GPS data into the RDS database
        insert_gps_data(
            gps_data['device_id'],
            gps_data['utc_datetime'],
            gps_data['speed'],
            gps_data['longitude'],
            gps_data['latitude'],
            gps_data['course'],
            gps_data['data_type'],
            gps_data['rssi']
        )

        return {
            'body': json.dumps('GPS data parsed and inserted successfully!')
        }

    except Exception as e:
        print(f"Error processing GPS data: {e}")
        return {
            'body': json.dumps('Failed to process GPS data')
        }