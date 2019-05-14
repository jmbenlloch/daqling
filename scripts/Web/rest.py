"""
 Copyright (C) 2019 CERN
 
 DAQling is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 DAQling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public License
 along with DAQling. If not, see <http://www.gnu.org/licenses/>.
"""

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

def setupMetrics():
  dataList['datarate'] = [] 
  dataList['cpuutil'] = []

setupMetrics()

'''
Main app
'''
app = Flask(__name__,
            static_url_path='',
            static_folder='web/static',
            template_folder='web/templates')
api = Api(app)
parser = reqparse.RequestParser()
parser.add_argument('value')

class Add(Resource):
    def post(self, metric):
        args = parser.parse_args()
        print(args)
        dataList[metric].append(args['value'])

class Metrics(Resource):
    def get(self):
        resp = flask.make_response(flask.jsonify(dataList))
        return resp

api.add_resource(Add, "/add/<string:metric>", methods=['POST'])
api.add_resource(Metrics, "/metrics", methods=['GET'])

# As a testserver.
app.run(host= '0.0.0.0', port=5000, debug=True)
# Normally spawned by gunicorn


