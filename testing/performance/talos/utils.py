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
# The Original Code is standalone Firefox performance tests.
#
# The Initial Developer of the Original Code is The Mozilla Corporation.
# Portions created by the Initial Developer are Copyright (C) 2007
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#   Zach Lipton <zach@zachlipton.com>
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

"""Utility functions"""

import os
import time
import sys
DEBUG = 0
NOISY = 0
START_TIME = 0
saved_environment = {}

def startTimer():
  global START_TIME
  START_TIME = time.time()

def stopTimer():
  stop_time = time.time()
  return time.strftime("%H:%M:%S", time.gmtime(stop_time-START_TIME))

def setdebug(val):
  global DEBUG
  DEBUG = val

def setnoisy(val):
  global NOISY
  NOISY = val

def noisy(message):
  """Prints messages from the browser/application that are generated, otherwise
     these are ignored.  Controlled through command line switch (-n or --noisy)
  """
  if NOISY == 1:
    lines = message.splitlines()
    counter = 1
    for line in lines:
      print "NOISE: " + line
      #really silly throttling
      if counter % 100 == 0:
        time.sleep(1) #twisted.spread.banana.BananaError: string is too long to send (803255)
      sys.stdout.flush()
      counter += 1

def debug(message):
  """Prints a debug message to the console if the DEBUG switch is turned on 
     debug switch is controlled through command line switch (-d or --debug)
     Args:
       message: string containing a debugging statement
  """
  if DEBUG == 1:
    lines = message.splitlines()
    counter = 1
    for line in lines:
      print "DEBUG: " + line
      #really silly throttling
      if counter % 100 == 0:
        time.sleep(1) #twisted.spread.banana.BananaError: string is too long to send (803255)
      sys.stdout.flush()
      counter += 1

def stamped_msg(msg_title, msg_action):
  """Prints a message to the console with a time stamp
  """
  time_format = "%a, %d %b %Y %H:%M:%S"
  msg_format = "%s: \n\t\t%s %s"
  print msg_format % (msg_title, msg_action, time.strftime(time_format, time.localtime()))
  sys.stdout.flush()

def setEnvironmentVars(newVars): 
   """Sets environment variables as specified by env, an array of variables 
   from sample.config"""
   global saved_environment
   env = os.environ
   for var in newVars:
     # save the old values so they can be restored later:
     try:
       saved_environment[var] = str(env[var])
     except :
       saved_environment[var] = ""
     env[var] = str(newVars[var])

def restoreEnvironmentVars():
  """Restores environment variables to the state they were in before 
  setEnvironmentVars() was last called"""
  global saved_environment
  for var in saved_environment:
    os.environ[var] = saved_environment[var]

class talosError(Exception):
  def __init__(self, msg):
    self.msg = msg
  def __str__(self):
    return repr(self.msg)
