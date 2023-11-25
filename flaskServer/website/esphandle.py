from flask import Blueprint, render_template, request, flash, jsonify
from .models import data1,device1
from . import db
import json

esphandle = Blueprint('esphandle', __name__)

@esphandle.route('/esp', methods=['GET','POST'])
def esp_handle():
    if request.method == 'POST':
        try:
            post_data = request.data.decode('utf-8')
            post_data = json.loads(request.data)
            data=post_data['data']
            device_id=post_data['device_id']
            print("Received data: ", data, "--- Device id: ",device_id)
            data = data1(data=data,device_id=device_id)
            db.session.add(data)
            db.session.commit()
            # post_data = json.loads(request.data)
            # print("Received post_data:", post_data)
            # # Convert the dictionary to a JSON string before storing in the database
            # data = data1(data=json.dumps(post_data),device_id=1)
            # db.session.add(data)
            # db.session.commit()

            return "ok"
        except Exception as e:
            print("Error:", str(e))
            return "error"
    if request.method == 'GET':
        datas = data1.query.all()
        data_list = [{"id": entry.id, "data": entry.data,"date": entry.date, "device_id": entry.device_id} for entry in datas]
        return jsonify(data_list), 200
        
