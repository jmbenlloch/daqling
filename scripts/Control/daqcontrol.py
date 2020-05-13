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
  def __init__(self, group, use_supervisor=True):
    self.group = group
    self.context = zmq.Context()
    self.stop_check = False
    self.use_supervisor = use_supervisor

  ## returns a map of process names and process ids
  def getAllNameProcessID(self, host):
    sw = supervisor_wrapper.supervisor_wrapper(host, self.group)
    try:
      return {info['name']: info['pid'] for info in sw.getAllProcessInfoInGroup()}
    except Exception as e:
      print("Exception", str(e))

  def removeProcess(self, host, name):
    sw = supervisor_wrapper.supervisor_wrapper(host, self.group)
    try:
      if sw.getProcessState(name)['statename'] == 'RUNNING':
        try:
          print('Stop', sw.stopProcess(name))
        except Exception as e:
          print("Exception",str(e),": cannot stop process",
                name, "(probably already stopped)")
      print('Remove', sw.removeProcessFromGroup(name))
    except Exception as e:
      print("Exception",str(e),": Couldn't get process state")

  def removeProcesses(self, components):
    for p in components:
      self.removeProcess(p['host'], p['name'])

  def addProcess(self, host, name, exe, dir, lib_path="", command=""):
    sw = supervisor_wrapper.supervisor_wrapper(host, self.group)
    try:
      rv, log_file = sw.addProgramToGroup(name, exe, dir, lib_path, command)
      print("Add", rv)
      return log_file
    except Exception as e:
      print("Exception",str(e),": cannot add program", name, "(probably already added)")


  def addComponents(self, components, exe, dir, lib_path):
    log_files = []
    for p in components:
      name = p['name']
      port = p['port']
      loglvl_core = p['loglevel']['core']
      loglvl_module = p['loglevel']['module']
      full_exe = exe+" "+name+" "+str(port)+" "+loglvl_core+" "+loglvl_module
      log_files.append(self.addProcess(p['host'], name, full_exe, dir, lib_path=lib_path))
    return log_files

  def addScripts(self, scripts, script_dir):
    log_files = []
    for s in scripts:
      name = s['name']
      command = s['command']
      exe = s['executable']
      dir = script_dir+s['directory']
      log_files.append(self.addProcess(s['host'], name, exe, dir, command=command))
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
      # print("Exception: handleRequest timeout", request)
      return b'', True

  def configureProcess(self, p):
    req = json.dumps({'command': 'configure'})
    config = json.dumps(p)
    rv, rv1 = self.handleRequest(p['host'], p['port'], req, config)
    if rv != b'Success':
      print("configureProcess", p['name'], "rv:", rv, "timeout:", rv1)

  def unconfigureProcess(self, p):
    req = json.dumps({'command': 'unconfigure'})
    rv, rv1 = self.handleRequest(p['host'], p['port'], req)
    if rv != b'Success':
      print("unconfigure", p['name'], "rv:", rv, "timeout:", rv1)

  def startProcess(self, p, arg="0"):
    req = json.dumps({'command': 'start'})
    rv, rv1 = self.handleRequest(p['host'], p['port'], req, arg)
    if rv != b'Success':
      print("startProcess", p['name'], "rv:", rv, "timeout:", rv1)

  def stopProcess(self, p):
    req = json.dumps({'command': 'stop'})
    rv, rv1 = self.handleRequest(p['host'], p['port'], req)
    if rv != b'Success':
      print("stopProcess", p['name'], "rv:", rv, "timeout:", rv1)

  def shutdownProcess(self, p):
    req = json.dumps({'command': 'shutdown'})
    rv, rv1 = self.handleRequest(p['host'], p['port'], req)
    if rv != b'Success':
      print("shutdown", p['name'], "rv:", rv, "timeout:", rv1)

  def customCommandProcess(self, p, command, args=None):
    req = json.dumps({'command': command})
    rv, rv1 = self.handleRequest(p['host'], p['port'], req, args)
    if rv != b'Success':
      print(command, p['name'], "rv:", rv, "timeout:", rv1)

  def getStatus(self, p):
      sw = supervisor_wrapper.supervisor_wrapper(p['host'], self.group)
      req = json.dumps({'command': 'status'})
      state = "RUNNING"
      timeout = False
      if self.use_supervisor:
        try:
          state = sw.getProcessState(p['name'])['statename']
        except:
          state = 'NOT_ADDED'
          status = b'not_added'
      if state == 'RUNNING':
        status, timeout = self.handleRequest(p['host'], p['port'], req)
        if status == b'':
          print("status", p['name'], "rv:", status, "timeout:", timeout)
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
