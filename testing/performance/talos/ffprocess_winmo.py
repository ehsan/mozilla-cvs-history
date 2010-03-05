# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1/GPL 2.0/LGPL 2.1
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Original Code is standalone Firefox Windows Mobile performance test.
#
# Contributor(s):
#   Joel Maher <joel.maher@gmail.com> (original author)
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 2 or later (the "GPL"), or
# the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
# in which case the provisions of the GPL or the LGPL are applicable instead
# of those above. If you wish to allow use of your version of this file only
# under the terms of either the GPL or the LGPL, and not to allow others to
# use your version of this file under the terms of the MPL, indicate your
# decision by deleting the provisions above and replace them with the notice
# and other provisions required by the GPL or the LGPL. If you do not delete
# the provisions above, a recipient may use your version of this file under
# the terms of any one of the MPL, the GPL or the LGPL.
#
# ***** END LICENSE BLOCK *****
from ffprocess import FFProcess
import devicemanager
import os
import time
import tempfile
import re

DEFAULT_PORT = 20701

class WinmoProcess(FFProcess):
    testAgent = None
    rootdir = ''
    dirSlash = ''
    host = ''
    port = ''
  
    def __init__(self, host, port, rootdir):
        if (port == 0):
            port = DEFAULT_PORT
        if (port == ''):
            port = DEFAULT_PORT
        if  (port == None):
            port = DEFAULT_PORT

        self.port = port
        self.host = host
        self.setupRemote(host, port)
        self.rootdir = rootdir
        parts = self.rootdir.split("\\")
        if (len(parts) > 1):
            self.dirSlash = "\\"
        else:
            self.dirSlash = "/"

    def setupRemote(self, host = '', port = DEFAULT_PORT):
        self.testAgent = devicemanager.DeviceManager(host, port)

    def GetRunningProcesses(self):
        current_procs = []
        return self.testAgent.getProcessList()


    def GenerateBrowserCommandLine(self, browser_path, extra_args, profile_dir, url):
        """Generates the command line for a process to run Browser

        Args:
        browser_path: String containing the path to the browser exe to use
        profile_dir: String containing the directory of the profile to run Browser in
        url: String containing url to start with.
        """

        profile_arg = ''
        if profile_dir:
            profile_arg = '-profile %s' % profile_dir

        cmd = '%s %s %s %s' % (browser_path,
                                 extra_args,
                                 profile_arg,
                                 url)
        return cmd
  

    def ProcessesWithNameExist(self, *process_names):
        """Returns true if there are any processes running with the
           given name.  Useful to check whether a Browser process is still running

        Args:
          process_name: String or strings containing the process name, i.e. "firefox"

        Returns:
          True if any processes with that name are running, False otherwise.
        """

        # refresh list of processes
        data = self.GetRunningProcesses()
        if (data == None):
            return False

        for process_name in process_names: 
            try:
                procre = re.compile(".*" + process_name + ".*")
                for line in data:
                    if (procre.match(line[1])):
                        return True
            except:
                # Might get an exception if there are no instances of the process running.
                continue
        return False
  

    def TerminateAllProcesses(self, *process_names):
        """Helper function to terminate all processes with the given process name

        Args:
          process_name: String or strings containing the process name, i.e. "firefox"
        """
        for process_name in process_names:
            try:
                self.testAgent.killProcess(process_name)
            except:
                # Might get an exception if there are no instances of the process running.
                continue


    def NonBlockingReadProcessOutput(self, handle):
        """Does a non-blocking read from the output of the process
           with the given handle.

        Args:
          handle: The process handle returned from os.popen()

        Returns:
          A tuple (bytes, output) containing the number of output
          bytes read, and the actual output.
        """

        output = ""
        try:
            output = self.getFile(handle)
            return (len(output), output)
        except:
            return (0, output)

    def getFile(self, handle, localFile = ""):
        temp = False
        if (localFile == ""):
            if (os.path.exists(handle)):
                #TODO
                return ""
            localFile = os.path.join(tempfile.mkdtemp(), "temp.txt")
            temp = True

        re_nofile = re.compile("error:.*")
        data = self.testAgent.getFile(handle, localFile)
        if (temp == True):
          os.remove(localFile)
        if (re_nofile.match(data)):
            fileData = ''
            if (os.path.isfile(handle)):
                results_file = open(handle, "r")
                fileData = results_file.read()
                results_file.close()
            return fileData
        return data

    def launchProcess(self, cmd, outputFile = "process.txt", timeout = -1):
        if (outputFile == "process.txt"):
            outputFile = self.rootdir + self.dirSlash + "process.txt"
        self.testAgent.fireProcess(cmd + " > " + outputFile)
        handle = outputFile
  
        timed_out = True
        if (timeout > 0):
            total_time = 0
            while total_time < timeout:
                time.sleep(1)
                if (not self.poll(handle)):
                    timed_out = False
                    break
                total_time += 1

            if (timed_out == True):
                return None
      
        return handle
  
    def poll(self, process):
        try:
            if (not self.testAgent.process.isAlive()):
                return None
            return 1
        except:
            return None
        return 1
  
    def copyDirToDevice(self, localDir):
        head, tail = os.path.split(localDir)

        remoteDir = self.rootdir + self.dirSlash + tail
        self.testAgent.pushDir(localDir, remoteDir)
        return remoteDir
  
    def removeDirectory(self, dir):
        self.testAgent.removeDir(dir)

    def MakeDirectoryContentsWritable(self, dir):
        pass

    def copyFile(self, fromfile, toDir):
        toDir = toDir.replace("/", self.dirSlash)
        self.testAgent.pushFile(fromfile, toDir + self.dirSlash + os.path.basename(fromfile))

    def getCurrentTime(self):
        return self.testAgent.getCurrentTime()

    def addRemoteServerPref(self, profile_dir, server):
        return self.testAgent.addRemoteServerPref(profile_dir, server)
