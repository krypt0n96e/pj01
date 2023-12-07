from flask import Blueprint, render_template, request, flash, jsonify
from sqlalchemy import and_
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
    elif request.method == 'GET': #HOST/esp?start=value&end=value
        start_id = request.args.get('start', type=int)
        end_id = request.args.get('end', type=int)

        # Use the provided values or set defaults if not present
        start_id = start_id if start_id is not None else 0
        end_id = end_id if end_id is not None else float('inf')

        datas = data1.query.filter(and_(data1.id >= start_id, data1.id <= end_id)).all()

        if datas:
            data_list = [{"id": entry.id, "data": entry.data, "date": entry.date, "device_id": entry.device_id} for entry in datas]
            return jsonify(data_list), 200
        else:
            return jsonify({'error': 'Invalid range or no data found'}), 404
        # else:
        #     datas = data1.query.all()
        #     data_list = [{"id": entry.id, "data": entry.data,"date": entry.date, "device_id": entry.device_id} for entry in datas]
        #     return jsonify(data_list), 200
        
@esphandle.route('/device', methods=['GET'])
def device_logs():
    if request.method == 'GET':#HOST/device?id=value
        device_id = request.args.get('id', type=int)

        if device_id is not None:
            # Tìm device theo ID
            device = device1.query.filter_by(id=device_id).first()

            if device:
                # Trả về log của device nếu nó tồn tại
                return jsonify({'id': device.id, 'logs': device.logs})
            else:
                return jsonify({'error': 'Device not found'}), 404
        else:
            devices = device1.query.all()
            device_list = [{"id": entry.id, "logs": entry.logs} for entry in devices]
            return jsonify(device_list), 200