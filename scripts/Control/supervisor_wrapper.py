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

from xmlrpc.client import ServerProxy
import datetime
import getpass

class supervisor_wrapper:
  def __init__(self, host, group):
    self.group = group
    self.host = host
    self.server = ServerProxy('http://'+host+':9001/RPC2')

  def getAllProcessInfo(self):
    return self.server.supervisor.getAllProcessInfo()

  def startProcess(self, name):
    return self.server.supervisor.startProcess(self.group+":"+name)

  def stopProcess(self, name):
    return self.server.supervisor.stopProcess(self.group+":"+name)

  def getProcessState(self, name):
    return self.server.supervisor.getProcessInfo(self.group+":"+name)

  def addProgramToGroup(self, name, exe, dir, env):
    now = datetime.datetime.now().strftime("%Y%m%d%H%M%S")
    user = getpass.getuser()
    log_file = "/log/"+name+"-"+user+"-"+now+".log"
    print("tail -f "+log_file)
    settings = {
        'command': dir+exe,
        'directory': dir,
        'autorestart': 'false',
        'environment': env,
        'user': user,
        'stdout_logfile': log_file,
        'stdout_logfile_maxbytes': '0',
        'stdout_logfile_backups': '0',
        'stderr_logfile': log_file
        # 'redirect_stderr': 'true' # TODO apparently supervisor 3.1.4 ignores this.
    }
    return (self.server.twiddler.addProgramToGroup(self.group, name, settings), (self.host, log_file))

  def removeProcessFromGroup(self, name):
    return self.server.twiddler.removeProcessFromGroup(self.group, name)
