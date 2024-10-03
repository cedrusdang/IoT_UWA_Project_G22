from flask import Blueprint, render_template, request, redirect, url_for, session, flash, jsonify
from models import db, User, Animal
import os

# Create a blueprint for main routes
main_routes = Blueprint('main_routes', __name__)


# @main_routes.route('/')
# def home():
#     return render_template('homepage.html')

@main_routes.route('/index')
def index():
    return render_template('index.html', active_page='home')

@main_routes.route('/register', methods=['GET', 'POST'])
def register():
    if request.method == 'POST':
        username = request.form['username']
        password = request.form['password']
     
        user = User.query.filter_by(username=username).first()
        if user:
            error_message = 'Username already taken. Please choose another one.'
            session['error_message'] = error_message
            return redirect(url_for('main_routes.register'))
        
        
        new_user = User(username=username)
        new_user.set_password(password)  
        db.session.add(new_user)
        db.session.commit()
        
      
        session['success_message'] = 'Registration successful. You can now login.'
        return redirect(url_for('main_routes.login'))
    
    error_message = session.pop('error_message', None)
    success_message = session.pop('success_message', None)
    return render_template('register.html', error_message=error_message, success_message=success_message)


@main_routes.route('/', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        username = request.form['username']
        password = request.form['password']
        
        
        user = User.query.filter_by(username=username).first()
        
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
    session.pop('user_id', None)
    session.pop('username', None)
   # flash('Logged out successfully')
    return redirect(url_for('main_routes.login'))

@main_routes.route('/save-geofence', methods=['POST'])
def save_geofence():
    data = request.get_json()  # Get JSON data from the POST request
    coordinates = data.get('coordinates')
    if coordinates:
        # Save the coordinates as per your needs (e.g., database)
        print("Received geofence coordinates:", coordinates)
        # Return a JSON response
        return jsonify({'message': 'Geofence saved successfully!'}), 200
    else:
        return jsonify({'error': 'No coordinates provided'}), 400


@main_routes.route('/add_animal', methods=['GET', 'POST'])
def add_animal():
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


# About page route
@main_routes.route('/about')
def about():
    return render_template('about.html')

@main_routes.route('/heatmap')
def heatmap():
    return render_template('heatmap.html', active_page='heatmap')

@main_routes.route('/geofence')
def geofence():
    return render_template('geofence.html', active_page='geofence')

@main_routes.route('/analytics')
def analytics():
    return render_template('analytics.html', active_page='analytics')

@main_routes.route('/animal_list')
def animal_list():
    animals = Animal.query.all()  
    return render_template('animal_list.html', animals=animals, active_page='animal_list')

from flask import jsonify

@main_routes.route('/animal_detail/<int:animal_id>', methods=['GET'])
def animal_detail(animal_id):
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
