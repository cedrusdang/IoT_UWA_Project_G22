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
def insert_gps_data(device_id, date, time, speed, longitude, latitude, course, rssi):
    conn = None
    try:
        # Connect to RDS MySQL
        conn = connect_to_rds()
        with conn.cursor() as cursor:
            # SQL query to insert GPS data into the table
            sql = """
                INSERT INTO gps_data (id, date, time, speed, longitude, latitude, course, rssi)
                VALUES (%s, %s, %s, %s, %s, %s, %s, %s)
            """
            cursor.execute(sql, (device_id, date, time, speed, longitude, latitude, course, rssi))

        conn.commit()
        print("GPS data inserted successfully!")
    
    except Exception as e:
        print(f"Error inserting data into RDS: {e}")
        raise e
    
    finally:

        if conn:
            conn.close()
            print("RDS connection closed.")