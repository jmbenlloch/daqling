import sys
import zmq
import requests
import time
from random import randint
import re

# defining the api-endpoint  
API_ENDPOINT = "http://127.0.0.1:5000/add/numberOfPackages"


port = "5555"

# Socket to talk to server
context = zmq.Context()
socket = context.socket(zmq.SUB)

socket.connect ("tcp://localhost:5556")

with open("metrics.conf", "r") as metric_conf:
	for line in metric_conf:
		socket.setsockopt_string(zmq.SUBSCRIBE, str(line).rstrip("\n\r"))


while 1:
	print("start")
	string = socket.recv()
	print("stop")
	print(string)
	data = {}
	endpoint = string.split(b':')[0].decode() 
	API_ENDPOINT = "http://127.0.0.1:5000/add/"+endpoint
	for s in string.split():
		print(s)
		if s.isdigit():
			data = {"value":s}
	print(data)
	r = requests.post(url = API_ENDPOINT, data = data)
	time.sleep(0.5)
