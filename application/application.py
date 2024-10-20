from flask import Flask
from models import db  # Import the db instance and models
from routes import main_routes 

def create_app():
    """
    Create and configure the Flask application.

    Returns:
        Flask application instance.
    """
    app = Flask(__name__)

    # Configure the database URI and other app settings
    # app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:////tmp/your_database.db'  # Use for live  
    app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///database.db'  # Use a local SQLite database
    app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False  # Disable modification tracking
    app.config['SECRET_KEY'] = 'SMART_TRACKING'  # Set your secret key for session management and CSRF protection

    # Initialize the db instance with the app
    db.init_app(app)

    # Import and register the main routes blueprint
    app.register_blueprint(main_routes)

    # Create the database tables if they don't already exist
    with app.app_context():
        db.create_all()

    return app

# Create the Flask application instance
application = create_app()  # Set 'application' to the WSGI callable

if __name__ == '__main__':
    # Run the application on all interfaces at port 8080
    application.run(host='0.0.0.0', port=8080)
