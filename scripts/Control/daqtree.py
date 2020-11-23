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
from os import environ as env
import time
import json
import jsonref
from jsonschema import validate
from anytree import RenderTree
from anytree.search import find_by_attr
from anytree.importer import DictImporter
from pathlib import Path
from copy import deepcopy

from nodetree import NodeTree
from daqcontrol import daqcontrol as daqctrl, jsonref_to_json

def print_help():
  print("Missing JSON configuration dictionary file.\n"
        "Usage: python3 daqtree.py <config-dict>")

## Main

if len(sys.argv) <= 1:
  print_help()
  quit()

for o in sys.argv:
  if o == '-h':
    print_help()
    quit()

# load all required configuration files
config_dict_path = sys.argv[1]

with open(config_dict_path) as f:
  config_dict = json.load(f)
f.close()

config_dir_path = env['DAQ_CONFIG_DIR']

with open(config_dir_path+config_dict["tree"]) as f:
  tree = json.load(f)
f.close()

with open(config_dir_path+config_dict["fsm_rules"]) as f:
   fsm_rules = json.load(f)
f.close()

state_action = fsm_rules["fsm"]
order_rules = fsm_rules["order"]

with open(config_dir_path+config_dict["config"]) as f:
  base_dir_uri = Path(env['DAQ_CONFIG_DIR']).as_uri() + '/'
  jsonref_obj = jsonref.load(f, base_uri=base_dir_uri, loader=jsonref.JsonLoader())
f.close()

if "configuration" in jsonref_obj:
  # schema with references (version >= 10)
  configuration = deepcopy(jsonref_obj)["configuration"]
else:
  # old-style schema (version < 10)
  configuration = jsonref_obj

# open schema and validate the configuration
with open(config_dir_path+'schemas/validation-schema.json') as f:
  schema = json.load(f)
f.close()
print("Configuration Version:", configuration['version'])
validate(instance=configuration, schema=schema)

# define required parameters from configuration
group = configuration['group']
if 'path' in configuration.keys():
  dir = configuration['path']
else:
  dir = env['DAQ_BUILD_DIR']
exe = "/bin/daqling"
lib_path = 'LD_LIBRARY_PATH='+env['LD_LIBRARY_PATH']+':'+dir+'/lib/'
components = configuration["components"]

# instanciate a daqcontrol object
dc = daqctrl(group)

# anytree importer from dictionary, using DAQling NodeTree class
importer = DictImporter(nodecls=NodeTree)

root = importer.import_(tree)

# configure the nodes of the imported tree and start checkers
for pre, _, node in RenderTree(root):
  for c in components:
    if node.name ==  c['name']:
      node.configure(order_rules, state_action, pconf=c, exe=exe, dc=dc, dir=dir, lib_path=lib_path)
    else:
      node.configure(order_rules, state_action)
  
  node.startStateCheckers()
  print("%s%s" % (pre, node.name))

# user input loop
while (True):
  try:
    ctrl, *args = input("Enter input:\n"+
      "  <node> <action> <args> or \"render\" or \"exit\"\n"+
      "    with actions: add, boot, configure, start [run num], stop, unconfigure,\n"+
      "                  shutdown, remove, <custom-command>, exclude, include\n").split(' ')
    if ctrl == "exit":
      for _, _, node in RenderTree(root):
        node.stopStateCheckers()
      sys.exit()
    elif ctrl == "render":
      for pre, _, node in RenderTree(root):
        print("%s%s [%s] {included %s} {inconsistent %s}" % (pre, node.name, node.getState(), node.included, node.inconsistent))
    elif args[0] == "exclude":
      find_by_attr(root, ctrl).exclude()
    elif args[0] == "include":
      find_by_attr(root, ctrl).include()
    else:
      rv = find_by_attr(root, ctrl).executeAction(*args)
      print(rv)
  except Exception as e:
    print("Exception:", e)
  time.sleep(1)
