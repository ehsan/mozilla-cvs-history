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


import win32pdh
import win32pdhutil


class CounterManager:

  def __init__(self, ffprocess, process, counters=None, childProcess="plugin-container"):
    self.ffprocess = ffprocess
    self.childProcess = childProcess
    self.registeredCounters = {}
    self.registerCounters(counters)
    # PDH might need to be "refreshed" if it has been queried while the browser
    # is closed
    win32pdh.EnumObjects(None, None, 0, 1)

    # Add the counter path for the default process.
    for counter in self.registeredCounters:
      path = win32pdh.MakeCounterPath((None, 'process', process,
                                       None, -1, counter))
      hq = win32pdh.OpenQuery()
      try:
        hc = win32pdh.AddCounter(hq, path)
      except:
        win32pdh.CloseQuery(hq)
        #assume that this is a memory counter for the system, not a process counter
        path = win32pdh.MakeCounterPath((None, 'Memory', None, None, -1 , counter))
        hq = win32pdh.OpenQuery()  
        try:                                                                       
          hc = win32pdh.AddCounter(hq, path)                                       
        except:                                                                    
          win32pdh.CloseQuery(hq)    

      self.registeredCounters[counter] = [hq, [(hc, path)]]
      self.updateCounterPathsForChildProcesses(counter)


  def registerCounters(self, counters):
    # self.registeredCounters[counter][0] is a counter query handle
    # self.registeredCounters[counter][1] is a list of tuples, the first 
    # member of which is a counter handle, the second a counter path
    for counter in counters:
      self.registeredCounters[counter] = []
            
  def unregisterCounters(self, counters):
    for counter in counters:
      if counter in self.registeredCounters:
        del self.registeredCounters[counter]

  def getRegisteredCounters(self):
    return keys(self.registeredCounters)

  def updateCounterPathsForChildProcesses(self, counter):
    # Create a counter path for each instance of the child process that
    # is running.  If any of these paths are not in our counter list,
    # add them to our counter query and append them to the counter list, 
    # so that we'll begin tracking their statistics.  We don't need to
    # worry about removing invalid paths from the list, as getCounterValue()
    # will generate a value of 0 for those.
    hq = self.registeredCounters[counter][0]
    win32pdh.EnumObjects(None, None, 0, 1)
    counterListLength = len(self.registeredCounters[counter][1])
    try:
      expandedCounterPaths = \
        win32pdh.ExpandCounterPath('\\process(%s*)\\%s' % (self.childProcess, counter))
    except:
      return
    for expandedPath in expandedCounterPaths:
      alreadyInCounterList = False
      for singleCounter in self.registeredCounters[counter][1]:
        if expandedPath == singleCounter[1]:
          alreadyInCounterList = True
      if not alreadyInCounterList:
        try:
          counterHandle = win32pdh.AddCounter(hq, expandedPath)
          self.registeredCounters[counter][1].append((counterHandle, expandedPath))
        except:
          continue
    if counterListLength != len(self.registeredCounters[counter][1]):
      try:
        win32pdh.CollectQueryData(hq)
      except:
        return

  def getCounterValue(self, counter):
    # Update counter paths, to catch any new child processes that might
    # have been launched since last call.  Then iterate through all
    # counter paths for this counter, and return a combined value.
    aggregateValue = 0
    self.updateCounterPathsForChildProcesses(counter)
    hq = self.registeredCounters[counter][0]

    # This call can throw an exception in the case where all counter paths
    # are invalid (i.e., all the processes have terminated).
    try:
      win32pdh.CollectQueryData(hq)
    except:
      return None

    for singleCounter in self.registeredCounters[counter][1]: 
      hc = singleCounter[0]
      try:
        type, val = win32pdh.GetFormattedCounterValue(hc, win32pdh.PDH_FMT_LONG)
      except:
        val = 0
      aggregateValue += val

    return aggregateValue

  def getProcess(self):
    return self.process

  def stopMonitor(self):
    try:
      for counter in self.registeredCounters:
        for singleCounter in self.registeredCounters[counter][1]: 
          win32pdh.RemoveCounter(singleCounter[0])
        win32pdh.CloseQuery(self.registeredCounters[counter][0])
      self.registeredCounters.clear()
    except:
      print 'failed to stopMonitor'
