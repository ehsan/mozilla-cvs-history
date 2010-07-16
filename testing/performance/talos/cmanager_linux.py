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
#   Alice Nodelman    <anodelman@mozilla.com> (removed threading)
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

import subprocess
import sys
import os
import time

def GetPrivateBytes(pids):
  """Calculate the amount of private, writeable memory allocated to a process.
     This code was adapted from 'pmap.c', part of the procps project.
  """
  privateBytes = 0
  for pid in pids:
    mapfile = '/proc/%s/maps' % pid
    maps = open(mapfile)

    private = 0

    for line in maps:
      # split up
      (range,line) = line.split(" ", 1)

      (start,end) = range.split("-")
      flags = line.split(" ", 1)[0]

      size = int(end, 16) - int(start, 16)

      if flags.find("p") >= 0:
        if flags.find("w") >= 0:
          private += size

    privateBytes += private
    maps.close()

  return privateBytes


def GetResidentSize(pids):
  """Retrieve the current resident memory for a given process"""
  # for some reason /proc/PID/stat doesn't give accurate information
  # so we use status instead

  RSS = 0  
  for pid in pids:
    file = '/proc/%s/status' % pid

    status = open(file)

    for line in status:
      if line.find("VmRSS") >= 0:
        RSS += int(line.split()[1]) * 1024

    status.close()

  return RSS

def GetCpuTime(pid, sampleTime=1):
  # return all zeros on this platform as per the 7/18/07 perf meeting
  return 0

def GetXRes(pids):
  """Returns the total bytes used by X or raises an error if total bytes is not available"""
  XRes = 0
  for pid in pids:
    try: 
      cmdline = "xrestop -m 1 -b | grep -A 15 " + str(pid) + " | tr -d \"\n\" | sed \"s/.*total bytes.*: ~//g\""
      pipe = subprocess.Popen(cmdline, shell=True, stdout=-1).stdout
      data = pipe.read()
      pipe.close()
    except:
      print "Unexpected error:", sys.exc_info()
      raise
    try:
      data = float(data)
      XRes += data
    except:
      print "Invalid data, not a float"
      raise

  return XRes

counterDict = {}
counterDict["Private Bytes"] = GetPrivateBytes
counterDict["RSS"] = GetResidentSize
counterDict["% Processor Time"] = GetCpuTime
counterDict["XRes"] = GetXRes

class CounterManager:
  """This class manages the monitoring of a process with any number of
     counters.

     A counter can be any function that takes an argument of one pid and
     returns a piece of data about that process.
     Some examples are: CalcCPUTime, GetResidentSize, and GetPrivateBytes
  """
  
  pollInterval = .25

  def __init__(self, ffprocess, process, counters=None, childProcess="plugin-container"):
    """Args:
         counters: A list of counters to monitor. Any counters whose name does
         not match a key in 'counterDict' will be ignored.
    """
    self.allCounters = {}
    self.registeredCounters = {}
    self.childProcess = childProcess
    self.runThread = False
    self.pidList = []
    self.ffprocess = ffprocess
    self.primaryPid = self.ffprocess.GetPidsByName(process)[-1]
    os.stat('/proc/%s' % self.primaryPid)

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
      self.updatePidList()
      return self.registeredCounters[counterName][0](self.pidList)
    except:
      return None

  def updatePidList(self):
    """Updates the list of PIDs we're interested in"""
    try:
      self.pidList = [self.primaryPid]
      childPids = self.ffprocess.GetPidsByName(self.childProcess)
      for pid in childPids:
        os.stat('/proc/%s' % pid)
        self.pidList.append(pid)
    except:
      print "WARNING: problem updating child PID's"

  def stopMonitor(self):
    """any final cleanup"""
    # TODO: should probably wait until we know run() is completely stopped
    # before setting self.pid to None. Use a lock?
    return
