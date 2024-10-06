import pymysql
import os

RDS_HOST = os.environ['RDS_HOST']
RDS_USER = os.environ['RDS_USER']
RDS_PASSWORD = os.environ['RDS_PASSWORD']
RDS_DB = os.environ['RDS_DB']

# establish a connection to the RDS MySQL database
def connect_to_rds():
    try:
        # Establish the connection to the RDS database
        conn = pymysql.connect(
            host=RDS_HOST,
            user=RDS_USER,
            password=RDS_PASSWORD,
            database=RDS_DB,
            cursorclass=pymysql.cursors.DictCursor
        )
        return conn
    except Exception as e:
        print(f"Error connecting to RDS: {e}")
        raise e

# insert GPS data into the RDS
def insert_gps_data(device_id, utc_datetime, speed, longitude, latitude, course, data_type, rssi):
    conn = None
    try:
        # Connect to RDS MySQL database
        conn = connect_to_rds()
        with conn.cursor() as cursor:
            # SQL query to insert GPS data into the table
            sql = """
                INSERT INTO gps_data (id, utc_datetime, speed, longitude, latitude, course, type, rssi)
                VALUES (%s, %s, %s, %s, %s, %s, %s, %s)
            """
            cursor.execute(sql, (device_id, utc_datetime, speed, longitude, latitude, course, data_type, rssi))

        conn.commit()
        print("GPS data inserted successfully!")
    
    except Exception as e:
        print(f"Error inserting data into RDS: {e}")
        raise e
    
    finally:
        # closed after use
        if conn:
            conn.close()
            print("RDS connection closed.")