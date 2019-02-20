import zmq
import msgpack
import supervisord

sd = supervisord.supervisord('rd51')

print(sd.getAllProcessInfo())

exit(0)

context = zmq.Context()
socket = context.socket(zmq.REQ)
socket.connect("tcp://localhost:5555")

msg = msgpack.packb(["configuration", "configure", "BoardReader"])
socket.send(msg)

# socket = context.socket(zmq.REP)
# socket.bind("tcp://localhost:5555")
reply = socket.recv()
unpacked = msgpack.unpackb(reply, raw=False)
print(unpacked)
