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

import supervisor_wrapper
import zmq
import json
from time import sleep


class daqcontrol:
  def __init__(self, group, lib_path, dir, exe, use_supervisor = True):
    self.group = group
    self.lib_path = lib_path
    self.dir = dir
    self.exe = exe
    self.context = zmq.Context()
    self.stop_check = False
    self.use_supervisor = use_supervisor

  def removeProcesses(self, components):
    for p in components:
      sd = supervisor_wrapper.supervisor_wrapper(p['host'], self.group)
      try:
        if sd.getProcessState(p['name'])['statename'] == 'RUNNING':
          try:
            print('Stop', sd.stopProcess(p['name']))
          except:
            print("Exception: cannot stop process",
                  p['name'], "(probably already stopped)")
        print('Remove', sd.removeProcessFromGroup(p['name']))
      except:
        print("Exception: Couldn't get process state")

  def addProcesses(self, components):
    log_files = []
    for p in components:
      sd = supervisor_wrapper.supervisor_wrapper(p['host'], self.group)
      try:
        rv, log_file = sd.addProgramToGroup(
            p['name'], self.exe+" "+str(p['port'])+" "+p['loglevel']['core']+" "+p['loglevel']['module'], self.dir, self.lib_path)
        print("Add", rv)
        log_files.append(log_file)
      except:
        print("Exception:\n  cannot add program", p['name'], "(probably already added)")
    return log_files

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
      return reply, False
    except:
      print("Timeout occurred")
      return b'', True

  def configureProcess(self, p):
    req = json.dumps({'command': 'configure'})
    config = json.dumps(p)
    rv, rv1 = self.handleRequest(p['host'], p['port'], req, config)
    if rv != b'Success':
      print("Error", p['name'], rv, rv1)

  def startProcess(self, p):
    req = json.dumps({'command': 'start'})
    rv, rv1 = self.handleRequest(p['host'], p['port'], req)
    if rv != b'Success':
      print("Error", p['name'], rv, rv1)

  def stopProcess(self, p):
    req = json.dumps({'command': 'stop'})
    rv, rv1 = self.handleRequest(p['host'], p['port'], req)
    if rv != b'Success':
      print("Error", p['name'], rv, rv1)

  def shutdownProcess(self, p):
    req = json.dumps({'command': 'shutdown'})
    rv, rv1 = self.handleRequest(p['host'], p['port'], req)
    if rv != b'Success':
      print("Error", p['name'], rv, rv1)

  def getStatus(self, p):
      sd = supervisor_wrapper.supervisor_wrapper(p['host'], self.group)
      req = json.dumps({'command': 'status'})
      state = "RUNNING"
      timeout = False
      if self.use_supervisor:
        try:
          state = sd.getProcessState(p['name'])['statename']
        except:
          state = 'NOT_ADDED'
          status = b'not_added'
      if state == 'RUNNING':
        status, timeout = self.handleRequest(p['host'], p['port'], req)
      elif state != 'NOT_ADDED':
        status = b'added'
      return status, timeout
  
  def statusCheck(self, p):
    status = ""
    while(not self.stop_check):
      sleep(0.5)
      new_status = self.getStatus(p)
      if new_status != status:
        print(p['name'], "in status", new_status)
        status = new_status
        # if status == b'booted':
        #   print("Automatically configure booted process")
        #   self.configureProcess(p)
