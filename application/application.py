from flask import Flask
from routes import main_routes  # Import the Blueprint

def create_app():
    app = Flask(__name__)

    # Register the Blueprint
    app.register_blueprint(main_routes)

    return app

application = create_app()  # Set 'application' to the WSGI callable

if __name__ == '__main__':
    application.run(debug=True, host='0.0.0.0', port=8080)
