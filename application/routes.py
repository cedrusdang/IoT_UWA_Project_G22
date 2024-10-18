from flask import Blueprint, render_template, request, redirect, url_for, session, flash, jsonify, make_response
from models import db, User, Animal, Geofence
import json

# Create a blueprint for main routes
main_routes = Blueprint('main_routes', __name__)

@main_routes.route('/index')
def index():
    """
    Render the homepage.

    Returns:
        Rendered template for the index page.
    """
    return render_template('index.html', active_page='home')

@main_routes.route('/register', methods=['GET', 'POST'])
def register():
    """
    Handle user registration.

    GET: Render the registration form.
    POST: Process registration data, create a new user, and redirect to login.

    Returns:
        Rendered template for the registration page or redirects to the login page.
    """
    if request.method == 'POST':
        username = request.form['username']
        password = request.form['password']
     
        # Check if username is already taken
        user = User.query.filter_by(username=username).first()
        if user:
            error_message = 'Username already taken. Please choose another one.'
            session['error_message'] = error_message
            return redirect(url_for('main_routes.register'))
        
        # Create a new user
        new_user = User(username=username)
        new_user.set_password(password)  
        db.session.add(new_user)
        db.session.commit()
      
        session['success_message'] = 'Registration successful. You can now login.'
        return redirect(url_for('main_routes.login'))
    
    # Pop error and success messages from session
    error_message = session.pop('error_message', None)
    success_message = session.pop('success_message', None)
    return render_template('register.html', error_message=error_message, success_message=success_message)

@main_routes.route('/', methods=['GET', 'POST'])
def login():
    """
    Handle user login.

    GET: Render the login form.
    POST: Process login data, authenticate user, and redirect to the homepage.

    Returns:
        Rendered template for the login page or redirects to the index page.
    """
    if request.method == 'POST':
        username = request.form['username']
        password = request.form['password']
        
        user = User.query.filter_by(username=username).first()
        
        # Check credentials
        if user and user.check_password(password):  
            session['user_id'] = user.id
            session['username'] = user.username
            return redirect(url_for('main_routes.index'))
        else:
            error_message = 'Wrong username or password.'  
            session['error_message'] = error_message
            return redirect(url_for('main_routes.login'))
    
    error_message = session.pop('error_message', None)
    return render_template('login.html', error_message=error_message)

@main_routes.route('/logout')
def logout():
    """
    Handle user logout.

    Clears the user session and redirects to the login page.

    Returns:
        Redirect response to the login page.
    """
    # Clear session
    session.pop('user_id', None)
    session.pop('username', None)
    
    # Prepare the response for redirection
    response = make_response(redirect(url_for('main_routes.login')))
    
    # Delete cookies by setting them with an expiration time in the past
    response.set_cookie('livestock', '', expires=0)
    
    return response

@main_routes.route('/save-geofence', methods=['POST'])
def save_geofence():
    """
    Save or update the user's geofence.

    Receives coordinates via a POST request, either creates a new geofence
    or updates an existing one.

    Returns:
        JSON response indicating success or failure.
    """
    data = request.get_json()
    user_id = session['user_id'] 
    coordinates = data.get('coordinates', [])
    
    if coordinates:
        # Convert list of coordinates to string format for storage
        coordinates_str = json.dumps(coordinates)
        
        # Check if the user already has a geofence entry
        geofence = Geofence.query.filter_by(owner_id=user_id).first()
        
        if geofence:
            # Update existing geofence
            geofence.coordinates = coordinates_str
        else:
            # Create a new geofence
            geofence = Geofence(owner_id=user_id, coordinates=coordinates_str)
            db.session.add(geofence)
        
        db.session.commit()
        return jsonify({'status': 'success', 'message': 'Geofence saved!'}), 200
    
    return jsonify({'status': 'error', 'message': 'No coordinates provided!'}), 400

@main_routes.route('/get-geofence', methods=['GET'])
def get_geofence():
    """
    Retrieve the user's geofence.

    Returns:
        JSON response with the geofence coordinates or None if not found.
    """
    user_id = session['user_id']  # Retrieve the user_id from the session
    
    # Retrieve geofence from the database using user_id
    geofence = Geofence.query.filter_by(owner_id=user_id).first()  
    
    if geofence:
        coordinates = geofence.coordinates
        return jsonify({'coordinates': coordinates}), 200
    
    return jsonify({'coordinates': None}), 200

@main_routes.route('/reset-geofence', methods=['POST'])
def reset_geofence():
    """
    Reset (delete) the user's geofence.

    Returns:
        JSON response indicating the success or failure of the operation.
    """
    try:
        user_id = session.get('user_id')  # Ensure you safely get user_id
        if not user_id:
            return jsonify({'error': 'User not logged in'}), 401

        # Find the existing geofence for the user
        geofence = Geofence.query.filter_by(owner_id=user_id).first()
        
        if geofence:
            # Remove the geofence from the database
            db.session.delete(geofence)
            db.session.commit()
            return jsonify({'message': 'Geofence reset successfully!'}), 200
        else:
            return jsonify({'message': 'No geofence found for this user.'}), 404

    except Exception as e:
        # Log the error for debugging
        print(f"Error occurred: {e}")
        return jsonify({'error': 'An error occurred while resetting geofence.'}), 500

@main_routes.route('/add_animal', methods=['GET', 'POST'])
def add_animal():
    """
    Add a new animal for the logged-in user.

    GET: Render the add animal form.
    POST: Process the animal data and save it to the database.

    Returns:
        Rendered template for the add animal page or redirects to the animal list.
    """
    if request.method == 'POST':
        name = request.form['name']
        code = request.form['code']
        remarks = request.form.get('remarks', '')
        
        if 'user_id' not in session:
            flash('You must be logged in to add an animal.', 'danger')
            return redirect(url_for('main_routes.login'))
        
        owner_id = session['user_id'] 

        new_animal = Animal(name=name, code=code, remarks=remarks, owner_id=owner_id)
        db.session.add(new_animal)
        db.session.commit()

        flash('Animal added successfully!', 'success')
        return redirect(url_for('main_routes.animal_list'))  

    return render_template('add_animal.html', active_page='add_animal')

@main_routes.route('/search_animal', methods=['GET'])
def search_animal():
    """
    Search for animals based on user input.

    Returns:
        Rendered template showing the search results.
    """
    query = request.args.get('query', '')
    if query:
        animals = Animal.query.filter(
            (Animal.name.like(f'%{query}%')) | 
            (Animal.code.like(f'%{query}%'))
        ).all()
    else:
        animals = []
    
    return render_template('search_animal.html', animals=animals, active_page='search_animal')

@main_routes.route('/profile', methods=['GET', 'POST'])
def profile():
    """
    Display and manage user profile.

    GET: Render the user's profile with their associated animals.
    POST: Update the user's password if provided.

    Returns:
        Rendered template for the user's profile page.
    """
    if 'user_id' not in session:
        return redirect(url_for('main_routes.login'))

    user_id = session['user_id']
    user = User.query.get(user_id)
    
    animals = Animal.query.filter_by(owner_id=user_id).all()

    if request.method == 'POST':
        if 'password' in request.form:
            new_password = request.form['password']
            user.set_password(new_password)
            db.session.commit()
            flash('Password updated successfully.')

        return redirect(url_for('main_routes.profile'))

    return render_template('profile.html', user=user, animals=animals)

@main_routes.route('/heatmap')
def heatmap():
    """
    Render the heatmap page.

    Returns:
        Rendered template for the heatmap.
    """
    return render_template('heatmap.html', active_page='heatmap')

@main_routes.route('/geofence')
def geofence():
    """
    Render the geofence page.

    Returns:
        Rendered template for the geofence.
    """
    return render_template('geofence.html', active_page='geofence')

@main_routes.route('/analytics')
def analytics():
    """
    Render the analytics page.

    Returns:
        Rendered template for analytics.
    """
    return render_template('analytics.html', active_page='analytics')

@main_routes.route('/animal_list')
def animal_list():
    """
    List all animals in the system.

    Returns:
        Rendered template for the animal list page.
    """
    animals = Animal.query.all()  
    return render_template('animal_list.html', animals=animals, active_page='animal_list')

@main_routes.route('/animal_detail/<int:animal_id>', methods=['GET'])
def animal_detail(animal_id):
    """
    Retrieve details of a specific animal.

    Returns:
        JSON response containing the animal's details.
    """
    # Query the animal by ID
    animal = Animal.query.get_or_404(animal_id)

    # Return animal details as JSON
    return jsonify({
        'name': animal.name,
        'code': animal.code,
        'remarks': animal.remarks,
        'owner': {
            'username': animal.owner.username
        }
    })
