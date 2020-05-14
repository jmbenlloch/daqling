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
import time
from influxdb import InfluxDBClient


def print_help():
  print("First argument must be a .json configuration file.", flush=True)


########## main ##########

use_influxDB = True
use_redis = True
subscribe_all = False


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
  print(repr(e), flush=True)
  print("ZMQ settings have to be provided!", flush=True)
  quit()

#setup ZMQ socket
context = zmq.Context()
socket = context.socket(zmq.SUB)

socket.bind("tcp://"+data_zmq["host"]+":"+data_zmq["port"])


#read settings and configure influxDB
try:
  data_influxDB = data["influxDB_settings"]
  client = InfluxDBClient(data_influxDB["host"], data_influxDB["port"], database=data_influxDB["name"])
except Exception as e:
  use_influxDB = False 
  print(repr(e), flush=True)
  print("influxDB settings not provided! Running without influxDB publishing.", flush=True)


#read settings and configure redis
try:
  data_redis = data["redis_settings"]
  r = redis.Redis(host=data_redis["host"], port=data_redis["port"])
except Exception as e:
  use_redis = False 
  print(repr(e), flush=True)
  print("Redis settings not provided! Running without redis publishing.", flush=True)


#read metrics configuration
#####################################
# Metrics configuration
#  "metrics": [
#    "<metric_name>,<destination>",
#    ...
#  ]
#  Possible destinations:
#  - i - influxDB
#  - r - Redis
#  - b - both (influxDB and Redis)
#  If <destination> not provided, 
#  "i" option is default
#####################################

metric_dest = {}

try:
  data_metrics = data["metrics"]
  for metric in data_metrics:
    metric_split = metric.split(',',2)
    if len(metric_split) == 1:
      metric_dest[metric_split[0]] = "i"    
    elif metric_split[1] in "irb":
      metric_dest[metric_split[0]] = metric_split[1]
    else:
      print(metric_split[0]+" <destination> option not valid - setting destination to influxDB", flush=True)
      metric_dest[metric_split[0]] = "i"    
    socket.setsockopt_string(zmq.SUBSCRIBE, metric_split[0])
except Exception as e:
  socket.setsockopt_string(zmq.SUBSCRIBE, "")
  subscribe_all = True
  print("Metrics configuration not provided - subscribing all possible metrics!", flush=True)


while 1:
  string = socket.recv()
  name = string.split(b':')[0].decode()
  json_body = [
		{
			"measurement": name,
			"fields": {
				"value": float(string.split()[1].decode())
			}
		}
  ]
  if use_influxDB == True and (subscribe_all == True or metric_dest[name] == "i" or metric_dest[name] == "b"):
    client.write_points(json_body)  
  
  if use_redis == True and (subscribe_all == True or metric_dest[name] == "r" or metric_dest[name] == "b"):
    r.rpush(name, str(time.time())+':'+string.split()[1].decode())
