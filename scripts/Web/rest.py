__author__ = "Roland Sipos"
__credits__ = [""]
__version__ = "0.0.1"
__maintainer__ = "Roland Sipos"
__email__ = "roland.sipos@cern.ch"

import ctypes
from multiprocessing.pool import ThreadPool
from collections import deque
import threading
from datetime import datetime, timedelta
import time
import json
import redis

import flask
from flask import Flask
from flask import Response
from flask import render_template
from flask_restful import Api, Resource, reqparse

#import queries
#import backend as db



'''
Main app
'''
app = Flask(__name__,
			static_url_path='',
			static_folder='templates',
			template_folder='templates')
api = Api(app)
parser = reqparse.RequestParser()
parser.add_argument('value')


r = redis.Redis(host='localhost', port=6379, db=0)

#r.delete('datarate', 'size')

class Add(Resource):
	def post(self, metric):
		args = parser.parse_args()
		print(args)
		value = str(args['value'])
		r.rpush(metric, str(time.time()*1000)+':'+value)
#		r.expire(metric, 5);


@app.route("/graph")
def graph():
	metrics = [x.decode() for x in r.keys()]
	print(metrics)
	return render_template('graph2.html', metrics=metrics)


@app.route("/data/<string:metric>")
def data(metric):
	metric_array = []
	for x in r.lrange(metric, 0, -1):
		value = x.decode().split(':')
		metric_array.append([float(value[0]), float(value[1])])
	return json.dumps(metric_array[-1000:])


@app.route("/metrics")
def metric():
	return json.dumps(r.keys())


@app.route("/lastMeas/<string:metric>")
def lastMeas(metric):
	value = r.lrange(metric, -1, -1)[0].decode().split(':')
	return json.dumps([float(value[0]), float(value[1])])



api.add_resource(Add, "/add/<string:metric>", methods=['POST'])
#api.add_resource(Metrics, "/metrics", methods=['GET'])

# As a testserver.
app.run(host= '0.0.0.0', port=5000, debug=True)
# Normally spawned by gunicorn


