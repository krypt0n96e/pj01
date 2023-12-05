from flask import Blueprint, render_template, request, flash, jsonify
from .models import data1,device1
from . import db
import json

views = Blueprint('views', __name__)


@views.route('/', methods=['GET', 'POST'])
def home():
    log1=device1.query.filter_by(id=1).first()
    if not log1:
        db.session.add(device1(logs=0)) #adding the data to the database 
    db.session.commit()
    if request.method == 'POST': 
        rawdata = request.form.get('data')#Gets the data from the HTML
        rawid = request.form.get('device_id')#Gets the id from the HTML 
        data = data1(data=rawdata,device_id=rawid)  #providing the schema for the data
        db.session.add(data) #adding the data to the database 
        db.session.commit()
        flash('Data added!', category='success')
    datas = data1.query.all()  # Retrieve all datas from the database
    return render_template("home.html", datas=datas, log=log1)


@views.route('/delete-data', methods=['POST'])
def delete_data():  
    data = json.loads(request.data)
    dataId = data['dataId']
    data = data1.query.get(dataId)
    if data:
        db.session.delete(data)
        db.session.commit()
    flash('Data deleted!', category='success')
    return jsonify({})

@views.route('/delete-all', methods=['POST'])
def delete_all():  
    try:
        data_objects = data1.query.all()
        for data in data_objects:
            db.session.delete(data)
        db.session.commit()
        flash('All data deleted!', category='success')
    except Exception as e:
        return jsonify({"error": str(e)}), 500
    return jsonify({})

@views.route('/status-change', methods=['POST'])
def status_change():
    log = json.loads(request.data)
    req=device1.query.filter_by(id=1).first()
    req.logs=log['log']
    db.session.commit()
    return jsonify({})