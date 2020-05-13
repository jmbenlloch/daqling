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
import daqcontrol
import threading
import zmq


def spawnJoin(list, func):
  threads = []
  for p in list:
    t = threading.Thread(target=func, args=(p,))
    t.start()
    threads.append(t)
  for t in threads:
    t.join()


def print_help():
  print("First argument must be a .json configuration file.\n"
        "Available second arguments: 'remove' 'add' 'configure' 'complete'.")

def signal_handler(sig, frame):
  print("Ctrl+C: shutting down")
  stop_check_threads()
  spawnJoin(data['components'], dc.shutdownProcess)
  if arg != 'configure':
    dc.removeProcesses(data['components'])
    if add_scripts:
      dc.removeProcesses(data['scripts'])
  quit()
  
def stop_check_threads():
  dc.stop_check = True
  for t in threads:
    t.join()

########## main ########

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

with open(env['DAQ_CONFIG_DIR']+'json-config.schema') as f:
  schema = json.load(f)
f.close()

add_scripts = False
if "scripts" in data:
  add_scripts = True

print("Configuration Version:", data['version'])
validate(instance=data, schema=schema)

group = data['group']
if 'path' in data.keys():
  dir = data['path']
else:
  dir = env['DAQ_BUILD_DIR']
print("Using path "+dir)
scripts_dir = env['DAQ_SCRIPT_DIR']
exe = "/bin/daqling"
lib_path = 'LD_LIBRARY_PATH='+env['LD_LIBRARY_PATH']+':'+dir+'/lib/'

if arg == "configure":
  dc = daqcontrol.daqcontrol(group, False)
else:
  dc = daqcontrol.daqcontrol(group)

if arg == "remove":
  dc.removeProcesses(data['components'])
  if add_scripts:
    dc.removeProcesses(data['scripts'])
  quit()

if arg == 'add' or arg == 'complete':
  log_files = dc.addComponents(data['components'], exe, dir, lib_path)
  # print(log_files)
  if add_scripts:
    dc.addScripts(data['scripts'], scripts_dir)
  if arg == 'add':
    quit()

# get the set of hosts
component_hosts = {c['host'] for c in data['components']}
script_hosts = {c['host'] for c in data['scripts']}
context = zmq.Context()
# loop on hosts, get the Name+PID dictionary and send it to psutil-manager
for host in component_hosts.union(script_hosts):
  name_pids = dc.getAllNameProcessID(host)
  socket = context.socket(zmq.PAIR)
  socket.connect("tcp://"+host+":6100")
  socket.send_pyobj(name_pids)

# spawn status check threads
threads = []
for p in data['components']:
  t = threading.Thread(target=dc.statusCheck, args=(p,))
  t.start()
  threads.append(t)

if arg == 'configure' or arg == 'complete':
  threads = []
  for p in data['components']:
    t = threading.Thread(target=dc.configureProcess, args=(p,))
    t.start()
    threads.append(t)
  for t in threads:
    t.join()

signal.signal(signal.SIGINT, signal_handler)

while(not dc.stop_check):
  cmd, *cmd_args = input("(config) | start | stop | down | command <cmd>\n").split(' ')
  print("Executing", cmd, ' '.join(cmd_args))
  command_threads = []
  if cmd == "config":
    spawnJoin(data['components'], dc.configureProcess)
  if cmd == "unconfig":
    spawnJoin(data['components'], dc.unconfigureProcess)
  elif cmd == "start":
    try:
      sp = partial(dc.startProcess, arg=cmd_args[0])
      spawnJoin(data['components'], sp)
    except IndexError:
      print("Run number not specified. Default to 0")
      spawnJoin(data['components'], dc.startProcess)
  elif cmd == "stop":
    spawnJoin(data['components'], dc.stopProcess)
  elif cmd == "down":
    stop_check_threads()
    spawnJoin(data['components'], dc.shutdownProcess)
    if add_scripts:
      dc.removeProcesses(data['scripts'])
    if arg != 'configure':
      dc.removeProcesses(data['components'])
  elif cmd == "command":
    try:
      ccp = partial(dc.customCommandProcess, command=cmd_args[0], args=' '.join(cmd_args[1:]))
      spawnJoin(data['components'], ccp)
    except IndexError:
      print("Missing required command argument")


quit()
