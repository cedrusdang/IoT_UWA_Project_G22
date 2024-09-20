from flask import Blueprint, render_template, request, redirect, url_for, session, flash
from models import db, User, Animal

# Create a blueprint for main routes
main_routes = Blueprint('main_routes', __name__)


@main_routes.route('/')
def home():
    return render_template('homepage.html')

@main_routes.route('/index')
def index():
    return render_template('index.html')

@main_routes.route('/register', methods=['GET', 'POST'])
def register():
    if request.method == 'POST':
        username = request.form['username']
        password = request.form['password']
        
        if User.query.filter_by(username=username).first():
            flash('Username already exists')
            return redirect(url_for('main_routes.register'))
        
        new_user = User(username=username)
        new_user.set_password(password)
        db.session.add(new_user)
        db.session.commit()
        flash('Registration successful')
        return redirect(url_for('main_routes.login'))
    
    return render_template('register.html')

@main_routes.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        username = request.form['username']
        password = request.form['password']
        user = User.query.filter_by(username=username).first()
        
        if user is None or not user.check_password(password):
            flash('Invalid username or password')
            return redirect(url_for('main_routes.login'))
        
        session['user_id'] = user.id
        session['username'] = user.username
        flash('Login successful')
        return redirect(url_for('main_routes.index'))
    
    return render_template('login.html')

@main_routes.route('/logout')
def logout():
    session.pop('user_id', None)
    session.pop('username', None)
    flash('Logged out successfully')
    return redirect(url_for('main_routes.home'))


@main_routes.route('/add_animal', methods=['GET', 'POST'])
def add_animal():
    if 'user_id' not in session:
        return redirect(url_for('main_routes.login'))

    if request.method == 'POST':
        name = request.form['name']
        code = request.form['code']
        remarks = request.form['remarks']

        if Animal.query.filter_by(code=code).first():
            flash('Animal with this code already exists')
            return redirect(url_for('main_routes.add_animal'))

        new_animal = Animal(name=name, code=code, remarks=remarks)
        db.session.add(new_animal)
        db.session.commit()
        flash('Animal added successfully')
        return redirect(url_for('main_routes.index'))

    return render_template('add_animal.html')

@main_routes.route('/search_animal', methods=['GET'])
def search_animal():
    query = request.args.get('query', '')
    if query:
        animals = Animal.query.filter(Animal.name.like(f'%{query}%') | Animal.code.like(f'%{query}%')).all()
    else:
        animals = []
    
    return render_template('search_animal.html', animals=animals)





# About page route
@main_routes.route('/about')
def about():
    return render_template('about.html')

@main_routes.route('/heatmap')
def heatmap():
    return render_template('heatmap.html')

@main_routes.route('/geofence')
def geofence():
    return render_template('geofence.html')

@main_routes.route('/analytics')
def analytics():
    return render_template('analytics.html')

@main_routes.route('/animal_list')
def animal_list():
    return render_template('animal_list.html')
