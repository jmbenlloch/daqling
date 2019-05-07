__author__ = "Wojciech Brylinski"
__credits__ = [""]
__version__ = "0.0.1"
__email__ = "wobrylin@cern.ch"

import ctypes
from multiprocessing.pool import ThreadPool
from collections import deque
import threading
from datetime import datetime, timedelta
import time
import json

import flask
from flask import Flask
from flask import Response
from flask import render_template
from flask_restful import Api, Resource, reqparse

#import queries
#import backend as db


global dataList
dataList = {}


'''
Main app
'''
app = Flask(__name__,
			static_url_path='',
			static_folder='.',
			template_folder='templates')
api = Api(app)
parser = reqparse.RequestParser()
parser.add_argument('value')

class Add(Resource):
	def post(self, metric):
		args = parser.parse_args()
		print(args)
		value = str(args['value'])
		for s in value.split():
			if s.isdigit():
				if not metric in dataList:
					dataList[metric] = []
				dataList[metric].append([time.time()*1000 ,int(s)])


@app.route("/graph")
def graph():
	return render_template('graph2.html', metrics=dataList.keys())


@app.route("/data/<string:metric>")
def data(metric):
	return json.dumps(dataList[metric])


@app.route("/metrics")
def metric():
	return json.dumps(dataList.keys())


@app.route("/lastMeas/<string:metric>")
def lastMeas(metric):
	if not dataList[metric]:
		return '0'
	else:
		return json.dumps(dataList[metric][-1])



api.add_resource(Add, "/add/<string:metric>", methods=['POST'])
#api.add_resource(Metrics, "/metrics", methods=['GET'])

# As a testserver.
app.run(host= '0.0.0.0', port=5000, debug=True)
# Normally spawned by gunicorn


