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

import threading
import concurrent.futures
import time
from anytree import NodeMixin
from json import dumps


## Node Tree class
#  Uses anytree NodeMixin.
class NodeTree(NodeMixin):
  ## Constructor
  #  @param type The type of node to instanciate
  def __init__(self, name, type=None, parent=None, children=None):
    self.name = name
    if type:
      self.type = type
    self.parent = parent
    self.state = "not_added"
    self.included = True
    self.inconsistent = False

  ## Configure node
  #  @param order_rules The json with rules for transition order
  #  @param state_action  The json with rules for state and allowed actions
  #  @param dc The the daqcontrol instance to use
  #  @param pconf The json with the process configuration
  #  @param exe The executable to spawn, with relative path from dir
  #  @param dir The absolute path of the directory from which exe is evaluated
  #  @param lib_path The LD_LIBRARY_PATH to pass to the executable
  def configure(self, order_rules, state_action, dc=None, pconf=None, exe=None, dir=None, lib_path=None):
    self.order_rules = order_rules
    self.state_action = state_action
    if pconf != None:
      self.dc = dc
      self.pconf = pconf
      self.dir = dir
      self.host = self.pconf['host']
      self.port = self.pconf['port']
      loglvl_core = self.pconf['loglevel']['core']
      loglvl_module = self.pconf['loglevel']['module']
      self.full_exe = exe+" "+self.name+" "+str(self.port)+" "+loglvl_core+" "+loglvl_module
      self.lib_path = lib_path

  ## Start node state checkers
  #  Spawns both self state checking threads and children state checking threads.
  def startStateCheckers(self):
    self.check = True
    self.sc = threading.Thread(target=self.checker)
    self.sc.start()
    if self.children:
      self.csc = threading.Thread(target=self.childrenStateChecker)
      self.csc.start()

  ## Stop node state checkers
  #  Stops and joins both self state checking threads and children state checking threads.
  #  To be called before exiting the script.
  def stopStateCheckers(self):
    self.check = False
    self.sc.join()
    if self.children:
      self.csc.join()

  def getState(self):
    return self.state

  ## Include the node in the tree
  #  When included, a node partecipates to the parent's state and receives actions from parent.
  def include(self):
    self.included = True

  ## Exclude the node from the tree
  #  When excluded, a node doesn't partecipate to the parent's state and doesn't receive actions from parent.
  def exclude(self):
    self.included = False

  def getIncluded(self):
    return self.included


  ## Execute an action
  #  The method is transparent to the node being a parent or children and acts accordingly.
  #  In the case of a parent it forwards the action to children, according to the order_rules.
  #  In the case of a children it executes the daqcontrol correspondent method if the action is allowed by state_action rules.
  def executeAction(self, action):
    if self.included == True:
      if not self.is_leaf:  # parent/controller
        with concurrent.futures.ThreadPoolExecutor() as executor:
          if action in self.order_rules:
            children_with_type = [x for x in self.children if hasattr(x, "type")]
            rvs = []
            for t in self.order_rules[action]:  # loop on the types in the order rule
              children_matching = [x for x in children_with_type if x.type == t]
              futures = []
              for c in children_matching:
                f = executor.submit(c.executeAction, action)
                futures.append(f)
              for f in futures:
                rvs.append(f.result())
            children_without_type = [x for x in self.children if not hasattr(x, "type")]
            futures = []
            for c in children_without_type:
              f = executor.submit(c.executeAction, action)
              futures.append(f)
            for f in futures:
              rvs.append(f.result())
            return rvs
          else:  # if the action is not in the order_rules just run the command in parallel on all children
            rvs = []
            futures = []
            for c in self.children:
              f = executor.submit(c.executeAction, action)
              futures.append(f)
            for f in futures:
              rvs.append(f.result())
            return rvs
      else:  # children/device
        if action in self.state_action[self.state]:
          # allow time for nodes to refresh their own state before printing
          if action == "add":
            return self.dc.addProcess(self.host, self.name, self.full_exe, self.dir, lib_path=self.lib_path)
          elif action == "boot":
            return self.dc.bootProcess(self.host, self.name)
          elif action == "remove":
            return self.dc.removeProcess(self.host, self.name)
          elif action == "configure":
            return self.dc.handleRequest(self.host, self.port, action, dumps(self.pconf))
          elif action == "unconfigure":
            return self.dc.handleRequest(self.host, self.port, action)
          elif action == "start":
            return self.dc.handleRequest(self.host, self.port, action, 0)
          elif action == "stop":
            return self.dc.handleRequest(self.host, self.port, action)
          elif action == "shutdown":
            return self.dc.handleRequest(self.host, self.port, "down")
          else:
            return self.dc.handleRequest(self.host, self.port, "custom", action)
        else:
          raise Exception("Action %s not allowed on %s" % (action, self.name))
    else:
      raise Exception("Node %s is excluded" % (self.name))

  def checker(self):
    prev = None
    while (self.check):
      if self.is_leaf:  # child
        self.state = self.dc.getStatus(self.pconf)
      if prev != self.state:
        prev = self.state
        # print((self.name, self.state))
      time.sleep(0.100)

  def childrenStateChecker(self):
    while (self.check):
      states = []
      for c in self.children:
        if c.getIncluded() == True:
          states.append(c.getState())
      max_state = max(states, key=lambda state: list(self.state_action.keys()).index(state))
      min_state = min(states, key=lambda state: list(self.state_action.keys()).index(state))
      max_count = 0
      min_count = 0
      for s in states:
        if s == max_state:
          max_count = max_count + 1
        elif s == min_state:
          min_count = min_count + 1
      if max_count == len(states):
        self.state = max_state
        self.inconsistent = False
      elif min_count == len(states):
        self.state = min_state
        self.inconsistent = False
      else:
        self.state = min_state
        self.inconsistent = True
      time.sleep(0.1)
