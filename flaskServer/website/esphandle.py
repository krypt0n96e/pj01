from flask import Blueprint, render_template, request, flash, jsonify
from .models import data1,status1
from . import db
import json

esphandle = Blueprint('esphandle', __name__)

@esphandle.route('/esp', methods=['POST'])
def esp_handle():
    if request.method == 'POST':
        try:
            post_data = json.loads(request.data)
            data=post_data['data']
            print("Received data:", data)
            data = data1(data=data)
            db.session.add(data)
            db.session.commit()
            
            # post_data = json.loads(request.data)
            # print("Received post_data:", post_data)
            # # Convert the dictionary to a JSON string before storing in the database
            # data = data1(data=json.dumps(post_data))
            # db.session.add(data)
            # db.session.commit()

            return "ok"
        except Exception as e:
            print("Error:", str(e))
            return "error"
