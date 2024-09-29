from flask import Flask, jsonify, request
import boto3

app = Flask(__name__)

# 使用AWS SDK Boto3连接AWS服务
iot_client = boto3.client('iot-data', region_name='ap-southeast-2')  # 根据你的region调整
sns_client = boto3.client('sns')

@app.route('/geo-fence', methods=['POST'])
def set_geofence():
    data = request.json
    # 设置新的地理围栏
    latitude_min = data.get('latitude_min')
    latitude_max = data.get('latitude_max')
    longitude_min = data.get('longitude_min')
    longitude_max = data.get('longitude_max')
    
    # 假设将这个设置存储到数据库或在内存中保存（如Redis等）
    geofence = {
        'latitude_min': latitude_min,
        'latitude_max': latitude_max,
        'longitude_min': longitude_min,
        'longitude_max': longitude_max
    }
    return jsonify({"message": "Geofence updated!", "geofence": geofence}), 200

@app.route('/get-device-data', methods=['GET'])
def get_device_data():
    # 模拟从AWS IoT获取设备数据
    response = iot_client.get_thing_shadow(
        thingName='livestock_device_001'
    )
    
    shadow_state = json.loads(response['payload'].read())
    device_data = shadow_state['state']['reported']
    
    return jsonify(device_data), 200

@app.route('/notify', methods=['POST'])
def notify():
    data = request.json
    device_id = data.get('device_id')
    latitude = data.get('latitude')
    longitude = data.get('longitude')
    
    # 使用SNS发送通知
    sns_client.publish(
        PhoneNumber='+614XXXXXXXX',  # 用户电话号码
        Message=f"Device {device_id} is out of bounds! Location: ({latitude}, {longitude})"
    )
    
    return jsonify({"message": "Notification sent!"}), 200

if __name__ == '__main__':
    app.run(debug=True)
