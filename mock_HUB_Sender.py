#Use this to test the connection, the data it will accept is GPS long and lat.
import requests

def send_gps_data(longitude, latitude, api_gateway_url):
    data = {
        'longitude': longitude,
        'latitude': latitude
    }
    headers = {'Content-Type': 'application/json'}

    try:
        response = requests.post(api_gateway_url, json=data, headers=headers)
        response.raise_for_status()  # Raise an exception for bad status codes

        print("GPS data sent successfully!")
        if response.content:  # Check if there's a response body
            print("API Gateway response:", response.json()) 

    except requests.exceptions.RequestException as e:
        print(f"Failed to send GPS data. Error: {e}")

# Example usage
api_gateway_url = 'https://fnh2s8vbnl.execute-api.ap-southeast-2.amazonaws.com/receiver'  # Replace with your actual API Gateway URL
send_gps_data(-122.3321, 47.6062, api_gateway_url)
