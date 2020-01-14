import sys
import zmq

context = zmq.Context()
socket = context.socket(zmq.SUB)
socket.setsockopt_string(zmq.SUBSCRIBE, '')
socket.bind("tcp://*:6542")

while (1):
  string = socket.recv()
  print(string)
