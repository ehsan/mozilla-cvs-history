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
# The Original Code is standalone Firefox Windows performance test.
#
# The Initial Developer of the Original Code is Google Inc.
# Portions created by the Initial Developer are Copyright (C) 2006
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#   Annie Sullivan <annie.sullivan@gmail.com> (original author)
#   Ben Hearsum    <bhearsum@wittydomain.com> (OS independence)
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

import subprocess
import signal
import os
from select import select
import time
from ffprocess import FFProcess
import shutil
import utils


class LinuxProcess(FFProcess):

    def __init__(self):
        pass

    def GenerateBrowserCommandLine(self, browser_path, extra_args, profile_dir, url):
        """Generates the command line for a process to run Browser

        Args:
            browser_path: String containing the path to the browser to use
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


    def GetPidsByName(self, process_name):
        """Searches for processes containing a given string.
            This function is UNIX specific.

        Args:
            process_name: The string to be searched for

        Returns:
            A list of PIDs containing the string. An empty list is returned if none are
            found.
        """

        matchingPids = []
  
        # A list of process names which should not be in the PID list returned
        # by this function.  This is needed so that we can reliably build a list
        # of browser child processes without including any Talos python
        # processes, which can pass the name of the child process as a parameter.
        processExclusionList = ['bcontroller.py']
  
        command = ['ps', 'ax']
        handle = subprocess.Popen(command, stdout=subprocess.PIPE)

        # wait for the process to terminate
        handle.wait()
        data = handle.stdout.read()
  
        # find all matching processes and add them to the list
        for line in data.splitlines():
            if line.find('defunct') != -1:
                continue
            if line.find(process_name) >= 0:
                shouldExclude = False
                # skip this process if it's in the processExclusionList
                for excludedProcess in processExclusionList:
                    if line.find(excludedProcess) >= 0:
                        shouldExclude = True
                if not shouldExclude:
                    # splits by whitespace, the first one should be the pid
                    pid = int(line.split()[0])
                    matchingPids.append(pid)

        return matchingPids


    def ProcessesWithNameExist(self, *process_names):
        """Returns true if there are any processes running with the
            given name.  Useful to check whether a Browser process is still running

        Args:
            process_names: String or strings containing the process name, i.e. "firefox"

        Returns:
            True if any processes with that name are running, False otherwise.
        """

        for process_name in process_names:
            pids = self.GetPidsByName(process_name)
            if len(pids) > 0:
                return True
        return False


    def TerminateProcess(self, pid, timeout):
        """Helper function to terminate a process, given the pid

        Args:
            pid: integer process id of the process to terminate.
        """
        try:
            if self.ProcessesWithNameExist(str(pid)):
                os.kill(pid, signal.SIGABRT)
                time.sleep(timeout)
                if self.ProcessesWithNameExist(str(pid)):
                    os.kill(pid, signal.SIGTERM)
                    time.sleep(timeout)
                    if self.ProcessesWithNameExist(str(pid)):
                        os.kill(pid, signal.SIGKILL)
        except OSError, (errno, strerror):
            print 'WARNING: failed os.kill: %s : %s' % (errno, strerror)

    def TerminateAllProcesses(self, timeout, *process_names):
        """Helper function to terminate all processes with the given process name

        Args:
            process_names: String or strings containing the process name, i.e. "firefox"
        """

        # Get all the process ids of running instances of this process,
        # and terminate them
        for process_name in process_names:
            pids = self.GetPidsByName(process_name)
            for pid in pids:
                self.TerminateProcess(pid, timeout)


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
        num_avail = 0

        # check for data
        # select() does not seem to work well with pipes.
        # after data is available once it *always* thinks there is data available
        # readline() will continue to return an empty string however
        # so we can use this behavior to work around the problem
        while select([handle], [], [], 0)[0]:
            line = handle.readline()
            if line:
                output += line
            else:
                break
            # this statement is true for encodings that have 1byte/char
            num_avail = len(output)
            
        return (num_avail, output)

    def MakeDirectoryContentsWritable(self, dirname):
        """Recursively makes all the contents of a directory writable.
            Uses os.chmod(filename, 0755).

        Args:
            dirname: Name of the directory to make contents writable.
        """
        try:
            for (root, dirs, files) in os.walk(dirname):
                os.chmod(root, 0755)
                for filename in files:
                    try:
                        os.chmod(os.path.join(root, filename), 0755)
                    except OSError, (errno, strerror):
                        print 'WARNING: failed to os.chmod(%s): %s : %s' % (os.path.join(root, filename), errno, strerror)
        except OSError, (errno, strerror):
            print 'WARNING: failed to MakeDirectoryContentsWritable: %s : %s' % (errno, strerror)
            
    def copyFile(self, fromfile, toDir):
        if not os.path.isfile(os.path.join(toDir, os.path.basename(fromfile))):
            shutil.copy(fromfile, toDir)
            utils.debug("insetalled" + fromfile)
        else:
            utils.debug("WARNING: file already insetalled (" + fromfile + ")")

    def removeDirectory(self, dir):
        self.MakeDirectoryContentsWritable(dir)
        shutil.rmtree(dir)

    def launchProcess(self, cmd, outputFile = "process.txt", timeout = -1):
        process = subprocess.Popen(cmd, stdout=subprocess.PIPE, universal_newlines=True, shell=True, env=os.environ)
        handle = process.stdout

        timed_out = True
        if (timeout > 0):
            total_time = 0
            while total_time < timeout:
                time.sleep(1)
                if (not self.poll(process)):
                    timed_out = False
                    break
                total_time += 1
      
        if (timed_out == True):
            return None
      
        return handle
  
    def poll(self, process):
        return process.poll()

    def getFile(self, handle, localFile = ""):
        fileData = ''
        if os.path.isfile(handle):
            results_file = open(handle, "r")
            fileData = results_file.read()
            results_file.close()
        return fileData

