import zmq
import msgpack
from xmlrpc.client import ServerProxy

s = ServerProxy('http://rd51:9001/RPC2')

print(s.supervisor.getAllProcessInfo())

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
