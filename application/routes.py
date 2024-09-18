from flask import Blueprint, render_template

# Create a blueprint for main routes
main_routes = Blueprint('main_routes', __name__)

# Home page route
@main_routes.route('/')
def home():
    return render_template('index.html')

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
