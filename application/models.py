from flask_sqlalchemy import SQLAlchemy
from werkzeug.security import generate_password_hash, check_password_hash

db = SQLAlchemy()

class User(db.Model):
    """
    Represents a user in the system.

    Attributes:
        id (int): The primary key of the user.
        username (str): The unique username of the user.
        password_hash (str): The hashed password of the user.
        animals (relationship): A relationship to the Animal model indicating the user's animals.
        geofence (relationship): A one-to-one relationship with the Geofence model for the user.

    Methods:
        set_password(password): Hashes and sets the user's password.
        check_password(password): Checks if the provided password matches the stored hashed password.
    """
    
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80), unique=True, nullable=False)
    password_hash = db.Column(db.String(128))

    animals = db.relationship('Animal', backref='owner', lazy=True)
    geofence = db.relationship('Geofence', backref='owner', lazy=True, uselist=False)  # One-to-One relationship

    def set_password(self, password):
        """
        Hashes the password and sets it for the user.

        Args:
            password (str): The plaintext password to be hashed.
        """
        self.password_hash = generate_password_hash(password)

    def check_password(self, password):
        """
        Checks if the provided password matches the user's hashed password.

        Args:
            password (str): The plaintext password to check against the hash.

        Returns:
            bool: True if the password matches, False otherwise.
        """
        return check_password_hash(self.password_hash, password)

class Animal(db.Model):
    """
    Represents an animal owned by a user.

    Attributes:
        id (int): The primary key of the animal.
        name (str): The name of the animal.
        code (str): A unique code for the animal.
        remarks (str): Additional remarks about the animal.
        owner_id (int): The foreign key linking to the User model, indicating the owner of the animal.
    """
    
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(80), nullable=False)
    code = db.Column(db.String(50), nullable=False)
    remarks = db.Column(db.String(200), nullable=True)
    owner_id = db.Column(db.Integer, db.ForeignKey('user.id'), nullable=False)

class Geofence(db.Model):
    """
    Represents a geofence associated with a user.

    Attributes:
        id (int): The primary key of the geofence.
        coordinates (JSON): The geographic coordinates defining the geofence area.
        owner_id (int): The foreign key linking to the User model, indicating the owner of the geofence.

    Relationships:
        user (relationship): A relationship back to the User model.
    """
    
    id = db.Column(db.Integer, primary_key=True)
    coordinates = db.Column(db.JSON, nullable=False)
    owner_id = db.Column(db.Integer, db.ForeignKey('user.id'), nullable=False)  # ForeignKey to the User model

    user = db.relationship('User', backref=db.backref('geofences', lazy=True))
