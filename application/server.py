# Flask server code (idcheck.py)
from flask import Flask, render_template, request, jsonify, redirect, url_for, session, flash, get_flashed_messages
import secrets
from user import admin_ls, user_ls

app = Flask(__name__)
app.secret_key = secrets.token_hex(16) 


@app.route('/signin', methods=['POST'])
def signin_logic():
    username = request.form['username']
    password = request.form['password']

    if username in admin_ls and password in user_ls:
        session['logged_in'] = True 
        return redirect(url_for('home'))
    else:
        flash('Incorrect username or password', 'error') 
        return redirect(url_for('signin_page'))

@app.route('/')
def signin_page():
    return render_template('signin.html', messages=get_flashed_messages(with_categories=True)) 

@app.route('/false')
def signin_false():
    return render_template('signinfalse.html')

@app.route('/home')
def home():
    if 'logged_in' in session and session['logged_in']:
        return render_template('home.html')
    else:
        return redirect(url_for('signin_page')) 

@app.route('/analysis')
def analysis():
    if 'logged_in' in session and session['logged_in']:
        return render_template('analysis.html')
    else:
        return redirect(url_for('signin_page'))

@app.route('/animal_list')
def animal_list():
    if 'logged_in' in session and session['logged_in']:
        return render_template('animal_list.html')
    else:
        return redirect(url_for('signin_page'))

if __name__ == '__main__':
    app.run(port=8080)