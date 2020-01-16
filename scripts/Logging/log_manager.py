import sys
import zmq

context = zmq.Context()
socket = context.socket(zmq.SUB)
# socket.setsockopt(zmq.SUBSCRIBE, (5561).to_bytes(4, byteorder='little'))
socket.setsockopt(zmq.SUBSCRIBE, b'')
socket.bind("tcp://*:6542")

while (1):
  msgs = socket.recv_multipart()
  print(int.from_bytes(msgs[0], byteorder='little'), str(msgs[1], 'utf-8'))