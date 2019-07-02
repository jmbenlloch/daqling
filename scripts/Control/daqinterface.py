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

# enrico.gamberini@cern.ch

import sys
from os import environ as env
import json
from jsonschema import validate
import daqcontrol

def spawnJoin(list, func):
  threads = []
  for p in list:
    t = func(dq, p)
    t.start()
    threads.append(t)
  for t in threads:
    t.join()


def print_help():
  print("First argument must be a .json configuration file.\n"
        "Available second arguments: 'remove' 'add' 'configure' 'complete'.\n"
        "Add 'dev' in order to suppress production feature.")

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
exe = "bin/main_core"
lib_path = 'LD_LIBRARY_PATH='+env['LD_LIBRARY_PATH']

dq = daqcontrol.daqcontrol(group, lib_path, dir, exe)

if arg == "remove":
  dq.removeProcesses(data['components'])
  quit()

if arg == 'add' or arg == 'complete':
  dq.addProcesses(data['components'], debug)
  if arg == 'add':
    quit()

if arg == 'configure' or arg == 'complete':
  threads = []
  for p in data['components']:
    t = daqcontrol.configureProcess(dq, p)
    t.start()
    threads.append(t)
  for t in threads:
    t.join()

# spawn status check threads
threads = []
for p in data['components']:
  t = daqcontrol.statusCheck(dq, p)
  t.start()
  threads.append(t)

while(not daqcontrol.exit):
  text = input("(config) | start | stop | down\n")
  print("Executing", text)
  command_threads = []
  if text == "config":
    spawnJoin(data['components'], daqcontrol.configureProcess)
  elif text == "start":
    spawnJoin(data['components'], daqcontrol.startProcess)
  elif text == "stop":
    spawnJoin(data['components'], daqcontrol.stopProcess)
  elif text == "down":
    daqcontrol.exit = True
    for t in threads:
      t.join()
    spawnJoin(data['components'], daqcontrol.shutdownProcess)
    if arg != 'configure':
      dq.removeProcesses(data['components'])

quit()
