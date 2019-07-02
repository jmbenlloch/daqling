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

import supervisord
import zmq
import json
from time import sleep

class daqcontrol:
  def __init__(self, group, lib_path, dir, exe):
    self.group = group
    self.lib_path = lib_path
    self.dir = dir
    self.exe = exe
    self.context = zmq.Context()
    self.stop_check = False

  def removeProcesses(self, components):
    for p in components:
      sd = supervisord.supervisord(p['host'], self.group)
      info = sd.getAllProcessInfo()
      for i in info:
        print(i)
        if i['statename'] == 'RUNNING':
          try:
            print('Stop', sd.stopProcess(i['name']))
          except:
            print("Exception: cannot stop process",
                  i['name'], "(probably already stopped)")
          print('State', sd.getProcessState(i['name'])['statename'])
        try:
          print('Remove', sd.removeProcessFromGroup(i['name']))
        except:
          print("Exception: cannot remove process", i['name'])


  def addProcesses(self, components, debug):
    for p in components:
      sd = supervisord.supervisord(p['host'], self.group)
      if debug is True:
        print("Add", sd.addProgramToGroup(
            p['name'], self.exe+" "+str(p['port'])+" debug", self.dir, self.lib_path))
      else:
        print("Add", sd.addProgramToGroup(
            p['name'], self.exe+" "+str(p['port']), self.dir, self.lib_path))


  def handleRequest(self, host, port, request, config=None):
    socket = self.context.socket(zmq.REQ)
    socket.setsockopt(zmq.LINGER, 0)
    socket.RCVTIMEO = 2000
    socket.connect("tcp://"+host+":"+str(port))
    if config == None:
      socket.send_string(request)
    else:
      socket.send_string(request, zmq.SNDMORE)
      socket.send_string(config)
    try:
      reply = socket.recv()
      # print(reply)
      return reply
    except:
      print("Timeout occurred")
      return ""


  def configureProcess(self, p):
    req = json.dumps({'command': 'configure'})
    config = json.dumps(p)
    rv = self.handleRequest(p['host'], p['port'], req, config)
    if rv != b'Success':
      print("Error", p['name'], rv)


  def startProcess(self, p):
    req = json.dumps({'command': 'start'})
    rv = self.handleRequest(p['host'], p['port'], req)
    if rv != b'Success':
      print("Error", p['name'], rv)


  def stopProcess(self, p):
    req = json.dumps({'command': 'stop'})
    rv = self.handleRequest(p['host'], p['port'], req)
    if rv != b'Success':
      print("Error", p['name'], rv)


  def shutdownProcess(self, p):
    req = json.dumps({'command': 'shutdown'})
    rv = self.handleRequest(p['host'], p['port'], req)
    if rv != b'Success':
      print("Error", p['name'], rv)


  def statusCheck(self, p):
    status = ""
    while(not self.stop_check):
      sleep(0.5)
      req = json.dumps({'command': 'status'})
      new_status = self.handleRequest(p['host'], p['port'], req)
      if new_status != status and new_status != "":
        print(p['name'], "in status", new_status)
        status = new_status
      elif new_status == "":
        print("Error", p['name'])
  