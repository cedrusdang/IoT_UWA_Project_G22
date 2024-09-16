import json
from db_operations import insert_into_rds  # Import the function from db_operations.py

# Function to extract GPS data from the event
def get_gps_data(event):
    try:

        
        body = event['body'] if 'body' in event else ''

        # assuming the data json like {"body": "[time,ID,lon,lat,RSSI,battery]"}
        # Remove brackets and split by comma
        data = body.strip('[]').split(',')

        if len(data) != 6:
            raise ValueError("Invalid data format. Expected 6 elements (time, ID, long, lat, RSSI, battery).")

        # Parse the values from the split data
        time = data[0].strip()          # Time
        device_id = int(data[1].strip())  # Device ID
        longitude = float(data[2].strip())  # Longitude
        latitude = float(data[3].strip())   # Latitude
        rssi = float(data[4].strip())     # RSSI
        battery = int(data[5].strip())    # Battery

        # Print
        #print(f"Parsed data - Time: {time}, Device ID: {device_id}, Longitude: {longitude}, Latitude: {latitude}, RSSI: {rssi}, Battery: {battery}")

        # Return parsed data as a dictionary
        return {
            'time': time,
            'device_id': device_id,
            'longitude': longitude,
            'latitude': latitude,
            'rssi': rssi,
            'battery': battery
        }
    except Exception as e:
        print(f"Error getting GPS data: {e}")
        raise e

# Main Lambda handler function
def lambda_handler(event, context):
    try:
        # Step 1: Get GPS data
        gps_data = get_gps_data(event)

        # Step 2: Insert GPS data into the RDS database
        insert_into_rds(latitude, longitude)

        return {
            'statusCode': 200,
            'body': json.dumps('GPS data processed and inserted successfully!')
        }

    except Exception as e:
        print(f"Error processing GPS data: {e}")
        return {
            'statusCode': 500,
            'body': json.dumps('Failed to process GPS data')
        }