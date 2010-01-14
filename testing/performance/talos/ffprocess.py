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

"""A set of functions for process management on Windows.
"""

__author__ = 'annie.sullivan@gmail.com (Annie Sullivan)'



import platform
import os
import re
import time
import subprocess
from utils import talosError

class FFProcess(object):
    
    def __init__(self):
        pass

    def RunProcessAndWaitForOutput(self, command, process_name, browser_wait, output_regex, timeout):
        """Runs the given process and waits for the output that matches the given
        regular expression.  Stops if the process exits early or times out.

        Args:
            command: String containing command to run
            process_name: Name of the process to run, in case it has to be killed
            browser_wait: Amount of time allowed for the browser to cleanly close
            output_regex: Regular expression to check against each output line.
                            If the output matches, the process is terminated and 
                            the function returns.
            timeout: Time to wait before terminating the process and returning

        Returns:
            A tuple (match, timedout) where match is the match of the regular 
            expression, and timed out is true if the process timed out and 
            false otherwise.
        """

        # Start the process
        process = subprocess.Popen(command, 
                                stdout=subprocess.PIPE, 
                                universal_newlines=True, 
                                shell=True,
                                env=os.environ)
        handle = process.stdout

        # Wait for it to print output, terminate, or time out.
        time_elapsed = 0
        output = ''
        interval = 2 # Wait 2 seconds in between checks

        while time_elapsed < timeout:
            time.sleep(interval)
            time_elapsed += interval

            (bytes, current_output) = self.NonBlockingReadProcessOutput(handle)
            output += current_output
    
            result = output_regex.search(output)
            if result:
                try:
                    return_val = result.group(1)
                    timer=0
                    while ((process.poll() is None) and timer < browser_wait):
                        time.sleep(1)
                        timer+=1
                    self.TerminateAllProcesses(browser_wait, process_name)
                    return (return_val, False)
                except IndexError:
                    # Didn't really match
                    pass

        # Timed out.
        self.TerminateAllProcesses(browser_wait, process_name)
        return (None, True)

    def checkBrowserAlive(self, process_name):
        #is the browser actually up?
        return (self.ProcessesWithNameExist(process_name) and 
                not self.ProcessesWithNameExist("crashreporter", "talkback", "dwwin"))

    def checkAllProcesses(self, process_name, child_process):
        #is anything browser related active?
        return self.ProcessesWithNameExist(process_name, child_process, "crashreporter", "talkback", "dwwin")

    def cleanupProcesses(self, process_name, child_process, browser_wait):
        #kill any remaining browser processes
        self.TerminateAllProcesses(browser_wait, process_name, "crashreporter", "dwwin", "talkback")
        #check if anything is left behind
        if self.checkAllProcesses(process_name, child_process):
            #this is for windows machines.  when attempting to send kill messages to win processes the OS
            # always gives the process a chance to close cleanly before terminating it, this takes longer
            # and we need to give it a little extra time to complete
            time.sleep(browser_wait)
            if self.checkAllProcesses(process_name, child_process):
                raise talosError("failed to cleanup")
