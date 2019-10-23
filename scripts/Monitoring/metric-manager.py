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

__version__ = "0.0.1"

import sys
import zmq
import json
import redis
from influxdb import InfluxDBClient


def print_help():
  print("First argument must be a .json configuration file.")


########## main ########

influxDB = True
redis = True


if len(sys.argv) <= 1:
  print_help()
  quit()

for o in sys.argv:
  if o == '-h':
    print_help()
    quit()

with open(sys.argv[1]) as f:
  data = json.load(f)
f.close()


#read ZMQ settings
try:
  data_zmq = data["zmq_settings"]
except Exception as e: 
  print(repr(e))
  print("ZMQ settings have to be provided!")
  quit()

#setup ZMQ socket
context = zmq.Context()
socket = context.socket(zmq.SUB)

socket.connect ("tcp://"+data_zmq["host"]+":"+data_zmq["port"])

#read influxDB settings
try:
  data_influxDB = data["influxDB_settings"]
except Exception as e:
  influxDB = False 
  print(repr(e))
  print("influxDB settings not provided! Running without influxDB publishing.")

#configure influxDb client:
if influxDB == True:
	client = InfluxDBClient(data_influxDB["host"], data_influxDB["port"], database=data_influxDB["name"])


#read redis settings
try:
  data_redis = data["redis_settings"]
except Exception as e:
  redis = False 
  print(repr(e))
  print("Redis settings not provided! Running without redis publishing.")


#read metrics configuration
try:
  data_metrics = data["metrics"]
  for metric in data_metrics:
    socket.setsockopt_string(zmq.SUBSCRIBE, metric)
except Exception as e:
  socket.setsockopt_string(zmq.SUBSCRIBE, "")
  print("Metrics configuration not provided - subscribing all possible metrices!")


while 1:
  string = socket.recv()
  print(string)
  name = string.split(b':')[0].decode() 
  json_body = [
		{
			"measurement": name,
			"fields": {
				"value": float(string.split()[1].decode())
			}
		}
  ]
  if influxDB == True:
    client.write_points(json_body)  
