import pymysql
import os

# Get RDS connection information from environment variables
RDS_HOST = os.environ['RDS_HOST']
RDS_USER = os.environ['RDS_USER']
RDS_PASSWORD = os.environ['RDS_PASSWORD']
RDS_DB = os.environ['RDS_DB']

# Function to establish a connection to the RDS MySQL database
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

# Function to insert GPS data into the database
def insert_into_rds(latitude, longitude):
    try:
        # Connect to the RDS MySQL database
        conn = connect_to_rds()
        with conn.cursor() as cursor:
            # SQL query to insert the GPS data into the table
            sql = "INSERT INTO gps_records (latitude, longitude) VALUES (%s, %s)"
            cursor.execute(sql, (latitude, longitude))

        # Commit the transaction
        conn.commit()
        print("GPS data inserted successfully!")
    except Exception as e:
        print(f"Error inserting data into RDS: {e}")
        raise e
    finally:
        # Ensure the connection is closed after use
        if 'conn' in locals():
            conn.close()