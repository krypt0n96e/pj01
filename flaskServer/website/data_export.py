from flask import Blueprint, render_template, request, flash, jsonify
from .models import data1, device1
from . import db
from datetime import datetime
import csv
import requests

# Tạo một Blueprint trong Flask
data_export = Blueprint('data_export', __name__)

@data_export.route('/export', methods=['GET'])
def export_csv():
    # Lấy tất cả dữ liệu từ bảng data1
    datas = data1.query.all()

    # Kiểm tra nếu có ít nhất 1000 dòng dữ liệu
    if len(datas) >= 10:
        # Tạo một danh sách để chứa dữ liệu cho CSV
        csv_data = []

        # Lặp qua các dòng dữ liệu và thêm chúng vào danh sách
        for data in datas:
            # Xử lý mã hóa và tách thành cặp (x, y)
            processed_data = process_and_split_data(data.data)

            # Thêm vào danh sách
            csv_data.extend([(time, value, data.device_id) for time, value in processed_data])

        # Generate a timestamp for the filename
        timestamp = datetime.now().strftime('%Y%m%d%H%M%S')
        csv_filename = f'exported_data_{timestamp}.csv'

        # Mở file CSV để ghi
        with open(csv_filename, 'w', newline='') as csvfile:
            # Sử dụng CSV writer để ghi dữ liệu vào file
            csv_writer = csv.writer(csvfile)

            # Ghi header nếu cần thiết
            csv_writer.writerow(['TIME', 'VALUE', 'Device ID'])

            # Ghi dữ liệu vào file
            csv_writer.writerows(csv_data)
        
        # Make a POST request to /delete_all
        delete_all_response = requests.post('http://127.0.0.1:8888/delete-all')

        # Check if the delete_all request was successful
        if delete_all_response.status_code == 200:
            # If successful, return a JSON response with success and filename
            flash('Data exported!', category='success')
            return jsonify({'success': True, 'filename': csv_filename})
        else:
            # If not successful, return a JSON response with an error message
            flash('Failed to delete all data after export!', category='error')
            return jsonify({'success': False, 'message': 'Failed to delete all data after export'})
    else:
        # Trả về thông báo nếu không đủ dữ liệu
        flash('Not enough data to export CSV!', category='error')
        return jsonify({'success': False, 'message': 'Not enough data to export CSV'})

def process_and_split_data(data_str):
    # Giả sử dữ liệu có định dạng "?x1&y1?x2&y2"
    parts = data_str.split('?')
    
    # Lọc bỏ các phần rỗng
    parts = list(filter(None, parts))

    # Xử lý mã hóa và tách thành cặp (x, y)
    processed_data = [tuple(part.split('&')) for part in parts]

    return processed_data


# # Thêm Blueprint vào ứng dụng Flask
# app.register_blueprint(data_export)
