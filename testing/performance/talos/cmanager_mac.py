#!/usr/bin/env python
#
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
#   Ben Hearsum    <bhearsum@wittydomain.com> (ported to linux)
#   Zach Lipton    <zach@zachlipton.com>  (Mac port)
#   Alice Nodelman    <anodelman@mozilla.com>  (removed threading)
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

__author__ = 'annie.sullivan@gmail.com (Annie Sullivan)'


import os
import time
import subprocess

def GetProcessData(pid):
  """Runs a ps on the process identified by pid and returns the output line
    as a list (pid, vsz, rss)
  """
  command = ['ps -o pid,vsize,rss -p'+str(pid)]
  handle = subprocess.Popen(command, stdout=subprocess.PIPE, universal_newlines=True, shell=True)
  handle.wait()
  data = handle.stdout.readlines()
  
  # First line is header output should look like:
  # PID      VSZ    RSS
  # 3210    75964    920 
  line = data[1]
  line = line.split()
  if (line[0] == str(pid)):
      return line

def GetPrivateBytes(pid):
  """Calculate the amount of private, writeable memory allocated to a process.
  """
  psData = GetProcessData(pid)
  return int(psData[1]) * 1024 #convert to bytes


def GetResidentSize(pid):
  """Retrieve the current resident memory for a given process"""
  psData = GetProcessData(pid)
  return int(psData[2]) * 1024 #convert to bytes

def GetCpuTime(pid):
  # return all zeros for now on this platform as per 7/18/07 perf meeting
  return 0

counterDict = {}
counterDict["Private Bytes"] = GetPrivateBytes
counterDict["RSS"] = GetResidentSize
counterDict["% Processor Time"] = GetCpuTime

class CounterManager():
  """This class manages the monitoring of a process with any number of
     counters.

     A counter can be any function that takes an argument of one pid and
     returns a piece of data about that process.
     Some examples are: CalcCPUTime, GetResidentSize, and GetPrivateBytes
  """
  
  def __init__(self, ffprocess, process, counters=None):
    """Args:
         counters: A list of counters to monitor. Any counters whose name does
         not match a key in 'counterDict' will be ignored.
    """
    self.allCounters = {}
    self.registeredCounters = {}
    self.ffprocess = ffprocess
    # the last process is the useful one
    self.pid = self.ffprocess.GetPidsByName(process)[-1]

    self._loadCounters()
    self.registerCounters(counters)

  def _loadCounters(self):
    """Loads all of the counters defined in the counterDict"""
    for counter in counterDict.keys():
      self.allCounters[counter] = counterDict[counter]

  def registerCounters(self, counters):
    """Registers a list of counters that will be monitoring.
       Only counters whose names are found in allCounters will be added
    """
    for counter in counters:
      if counter in self.allCounters:
        self.registeredCounters[counter] = \
          [self.allCounters[counter], []]

  def unregisterCounters(self, counters):
    """Unregister a list of counters.
       Only counters whose names are found in registeredCounters will be
       paid attention to
    """
    for counter in counters:
      if counter in self.registeredCounters:
        del self.registeredCounters[counter]

  def getRegisteredCounters(self):
    """Returns a list of the registered counters."""
    return keys(self.registeredCounters)

  def getCounterValue(self, counterName):
    """Returns the last value of the counter 'counterName'"""
    try:
      return self.registeredCounters[counterName][0](self.pid)
    except:
      print "Error in collecting counter: " + counterName
      return None

  def stopMonitor(self):
     """any final cleanup"""
     return
