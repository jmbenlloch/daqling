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

import sys
import signal
from os import environ as env
import json
from jsonschema import validate
from functools import partial
from daqcontrol import daqcontrol
import concurrent.futures
import threading
import zmq
from time import sleep

def spawnJoin(list, func):
  futures = []
  rvs = []
  with concurrent.futures.ThreadPoolExecutor() as executor:
    for p in list:
      f = executor.submit(func, p)
      futures.append(f)
    for f in futures:
      try:
        rvs.append(f.result())
      except Exception as e:
        print(e)
        rvs.append(e)
  return rvs


def print_help():
  print("First argument must be a .json configuration file.\n"
        "Available second arguments: 'remove' 'add' 'configure' 'complete'.")

def signal_handler(sig, frame):
  print("Ctrl+C: shutting down")
  sc.stop_check_threads()
  spawnJoin(data['components'], dc.shutdownProcess)
  if arg != 'configure':
    try:
      dc.removeProcesses(data['components'])
    except Exception as e:
      print(e)
    if add_scripts:
      try:
        dc.removeProcesses(data['components'])
      except Exception as e:
        print(e)
  quit()


class statusChecker():
  def __init__(self, dc):
    self.stop_check = False
    self.status_check_threads = []
    self.dc = dc

  def statusCheck(self, p):
    status = ""
    while(not self.stop_check):
      sleep(0.5)
      new_status = self.dc.getStatus(p)
      if new_status != status:
        print(p['name'], "in status", new_status)
        status = new_status

  def startChecker(self, components):
    for p in components:
      t = threading.Thread(target=self.statusCheck, args=(p,))
      t.start()
      self.status_check_threads.append(t)


  def stop_check_threads(self):
    self.stop_check = True
    for t in self.status_check_threads:
      t.join()

## Main

debug = False

arg = "complete"
if len(sys.argv) <= 1:
  print_help()
  quit()
elif len(sys.argv) == 3:
  arg = sys.argv[2]

for o in sys.argv:
  if o == '-h':
    print_help()
    quit()

with open(sys.argv[1]) as f:
  data = json.load(f)
f.close()

add_scripts = False
if "scripts" in data:
  add_scripts = True

# open schema and validate the configuration
with open(env['DAQ_CONFIG_DIR']+'schemas/config-schema.json') as f:
  schema = json.load(f)
f.close()
print("Configuration Version:", data['version'])
validate(instance=data, schema=schema)

# define required parameters from configuration
group = data['group']
if 'path' in data.keys():
  dir = data['path']
else:
  dir = env['DAQ_BUILD_DIR']
print("Using path "+dir)
scripts_dir = env['DAQ_SCRIPT_DIR']
exe = "/bin/daqling"
lib_path = 'LD_LIBRARY_PATH='+env['LD_LIBRARY_PATH']+':'+dir+'/lib/'

# instanciate a daqcontrol object
if arg == "configure":
  dc = daqcontrol(group, False)
else:
  dc = daqcontrol(group)

if arg == "remove":
  try:
    dc.removeProcesses(data['components'])
  except Exception as e:
    print(e)
  if add_scripts:
    try:
      dc.removeProcesses(data['scripts'])
    except Exception as e:
      print(e)
  quit()

if arg == 'add' or arg == 'complete':
  log_files = []
  try:
    log_files = dc.addComponents(data['components'], exe, dir, lib_path)
  except Exception as e:
    print(e)
  if add_scripts:
    try:
      log_files = log_files + dc.addScripts(data['scripts'], scripts_dir)
    except Exception as e:
      print(e)
  for l in log_files:
    print("Host:", l[0], "\n", "tail -f", l[1])
  if arg == 'add':
    quit()

# add scripts
if add_scripts:
  # get the set of hosts
  component_hosts = {c['host'] for c in data['components']}
  script_hosts = {s['host'] for s in data['scripts']}
  context = zmq.Context()
  # loop on hosts, get the Name+PID dictionary and send it to psutil-manager
  for host in component_hosts.union(script_hosts):
    try:
      name_pids = dc.getAllNameProcessID(host)
      socket = context.socket(zmq.PAIR)
      socket.connect("tcp://"+host+":6100")
      socket.send_pyobj(name_pids)
    except Exception as e:
      print(e)

sleep(0.5) # allow time for daqling executables to boot

# spawn status check threads
sc = statusChecker(dc)
sc.startChecker(data['components'])

if arg == 'configure' or arg == 'complete':
  print(spawnJoin(data['components'], dc.configureProcess))

signal.signal(signal.SIGINT, signal_handler)

# user input loop
while(not sc.stop_check):
  cmd, *cmd_args = input("(config) | start | stop | down | command <cmd>\n").split(' ')
  print("Executing", cmd, ' '.join(cmd_args))
  command_threads = []
  if cmd == "config":
    print(spawnJoin(data['components'], dc.configureProcess))
  if cmd == "unconfig":
    print(spawnJoin(data['components'], dc.unconfigureProcess))
  elif cmd == "start":
    try:
      sp = partial(dc.startProcess, run_num=cmd_args[0])
      print(spawnJoin(data['components'], sp))
    except IndexError:
      print("Run number not specified. Default to 0")
      print(spawnJoin(data['components'], dc.startProcess))
    except Exception as e:
      print(e)
  elif cmd == "stop":
    print(spawnJoin(data['components'], dc.stopProcess))
  elif cmd == "down":
    sc.stop_check_threads()
    print(spawnJoin(data['components'], dc.shutdownProcess))
    if add_scripts:
      try:
        dc.removeProcesses(data['scripts'])
      except Exception as e:
        print(e)
    if arg != 'configure':
      sleep(2) # allow time for daqling executables to exit
      try:
        dc.removeProcesses(data['components'])
      except Exception as e:
        print(e)
  elif cmd == "command":
    try:
      ccp = partial(dc.customCommandProcess, command=cmd_args[0], arg=' '.join(cmd_args[1:]))
      print(spawnJoin(data['components'], ccp))
    except IndexError:
      print("Missing required command argument")


quit()
