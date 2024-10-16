from flask import Flask
from models import db  # Import the db instance and models
from routes import main_routes 

def create_app():
    app = Flask(__name__)

    app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:////tmp/your_database.db' # Use for live  
    #app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///your_database.db'
    app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
    app.config['SECRET_KEY'] = 'SMART_TRACKING'  # Set your secret key


    # Initialize the db instance
    db.init_app(app)

    # Import and register routes
    app.register_blueprint(main_routes)

    # Create the database tables
    with app.app_context():
        db.create_all()

    return app

application = create_app()  # Set 'application' to the WSGI callable

if __name__ == '__main__':
    application.run(debug=True, host='0.0.0.0', port=8080)
