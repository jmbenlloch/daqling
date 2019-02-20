from xmlrpc.client import ServerProxy

class supervisord:
    def __init__(self, host):
        print("host",host, group)
        self.group = group
        self.server = ServerProxy('http://'+host+':9001/RPC2')

    def getAllProcessInfo(self):
        return self.server.supervisor.getAllProcessInfo()

    def startProcess(self, name):
        