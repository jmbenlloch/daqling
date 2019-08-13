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
import daqcontrol
import threading


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
        "Available second arguments: 'remove' 'add' 'configure' 'complete'.\n"
        "Add 'dev' in order to suppress production feature.")

def signal_handler(sig, frame):
  print("Ctrl+C: shutting down")
  stop_check_threads()
  spawnJoin(data['components'], dc.shutdownProcess)
  if arg != 'configure':
    dc.removeProcesses(data['components'])
  quit()
  
def stop_check_threads():
  dc.stop_check = True
  for t in threads:
    t.join()

########## main ########

validation = True
debug = False

arg = "complete"
if len(sys.argv) <= 2:
  print_help()
  quit()
elif sys.argv[2] != 'dev':
  arg = sys.argv[2]

for o in sys.argv:
  if o == '-h':
    print_help()
    quit()
  elif o == 'dev':
    print("Developer mode")
    validation = False
    debug = True

with open(sys.argv[1]) as f:
  data = json.load(f)
f.close()

with open(env['DAQ_CONFIG_DIR']+'json-config.schema') as f:
  schema = json.load(f)
f.close()

if validation:
  print("Configuration Version:", data['version'])
  validate(instance=data, schema=schema)

group = data['group']
dir = env['DAQ_BUILD_DIR']
exe = "bin/daqling"
lib_path = 'LD_LIBRARY_PATH='+env['LD_LIBRARY_PATH']

if arg == "configure":
  dc = daqcontrol.daqcontrol(group, lib_path, dir, exe, False)
else:
  dc = daqcontrol.daqcontrol(group, lib_path, dir, exe)


if arg == "remove":
  dc.removeProcesses(data['components'])
  quit()

if arg == 'add' or arg == 'complete':
  log_files = dc.addProcesses(data['components'], debug)
  # print(log_files)
  if arg == 'add':
    quit()

if arg == 'configure' or arg == 'complete':
  threads = []
  for p in data['components']:
    t = threading.Thread(target=dc.configureProcess, args=(p,))
    t.start()
    threads.append(t)
  for t in threads:
    t.join()

# spawn status check threads
threads = []
for p in data['components']:
  t = threading.Thread(target=dc.statusCheck, args=(p,))
  t.start()
  threads.append(t)

signal.signal(signal.SIGINT, signal_handler)

while(not dc.stop_check):
  text = input("(config) | start | stop | down\n")
  print("Executing", text)
  command_threads = []
  if text == "config":
    spawnJoin(data['components'], dc.configureProcess)
  elif text == "start":
    spawnJoin(data['components'], dc.startProcess)
  elif text == "stop":
    spawnJoin(data['components'], dc.stopProcess)
  elif text == "down":
    stop_check_threads()
    spawnJoin(data['components'], dc.shutdownProcess)
    if arg != 'configure':
      dc.removeProcesses(data['components'])

quit()
