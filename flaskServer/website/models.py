from . import db
# from sqlalchemy.sql import func
from datetime import datetime

class data1(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    data = db.Column(db.String(10000))
    date = db.Column(db.DateTime(timezone=True), default=datetime.utcnow)
    device_id = db.Column(db.Integer, db.ForeignKey('device1.id'))
    

class device1(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    logs = db.Column(db.Integer)
    data = db.relationship('data1', backref='logs', lazy=True)
    
