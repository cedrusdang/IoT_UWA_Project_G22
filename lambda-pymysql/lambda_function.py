import json
from db_operations import insert_into_rds  # Import the function from db_operations.py

# Function to extract GPS data from the event
def get_gps_data(event):
    try:
        # Parse the event body to get latitude and longitude
        body = json.loads(event['body'])
        latitude = body.get('latitude')
        longitude = body.get('longitude')

        # Validate that latitude and longitude exist
        if latitude is None or longitude is None:
            raise ValueError("Missing GPS data (latitude or longitude)")
        
        print(f"Received GPS data: Latitude={latitude}, Longitude={longitude}")
        return latitude, longitude
    except Exception as e:
        print(f"Error getting GPS data: {e}")
        raise e

# Main Lambda handler function
def lambda_handler(event, context):
    try:
        # Step 1: Get GPS data from the event
        latitude, longitude = get_gps_data(event)

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