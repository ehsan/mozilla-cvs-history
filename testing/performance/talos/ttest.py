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
#   Alice Nodelman <anodelman@mozilla.com>
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

"""A generic means of running an URL based browser test
   follows the following steps
     - creates a profile
     - tests the profile
     - gets metrics for the current test environment
     - loads the url
     - collects info on any counters while test runs
     - waits for a 'dump' from the browser
"""

__author__ = 'annie.sullivan@gmail.com (Annie Sullivan)'


import platform
import os
import os.path
import re
import shutil
import time
import sys
import subprocess
import utils
import glob
from utils import talosError

import ffprocess
import ffsetup


if platform.system() == "Linux":
    from cmanager_linux import *
    platform_type = 'linux_'
elif platform.system() in ("Windows", "Microsoft"):
    from cmanager_win32 import *
    platform_type = 'win_'
elif platform.system() == "Darwin":
    from cmanager_mac import *
    platform_type = 'mac_'


# Regular expression for getting results from most tests
RESULTS_REGEX = re.compile('__start_report(.*?)__end_report.*?__startTimestamp(.*?)__endTimestamp.*?__startSecondTimestamp(.*?)__endSecondTimestamp',
                      re.DOTALL | re.MULTILINE)
# Regular expression to get stats for page load test (Tp) - should go away once data passing is standardized
RESULTS_TP_REGEX = re.compile('__start_tp_report(.*?)__end_tp_report.*?__startTimestamp(.*?)__endTimestamp.*?__startSecondTimestamp(.*?)__endSecondTimestamp',
                      re.DOTALL | re.MULTILINE)
RESULTS_REGEX_FAIL = re.compile('__FAIL(.*?)__FAIL', re.DOTALL|re.MULTILINE)

def createProfile(profile_path, browser_config):
  # Create the new profile
  temp_dir, profile_dir = ffsetup.CreateTempProfileDir(profile_path,
                                             browser_config['preferences'],
                                             browser_config['extensions'])
  utils.debug("created profile") 
  return profile_dir, temp_dir

def initializeProfile(profile_dir, browser_config):
  if not (ffsetup.InitializeNewProfile(browser_config['browser_path'], browser_config['process'], browser_config['browser_wait'], browser_config['extra_args'], profile_dir, browser_config['init_url'], browser_config['browser_log'])):
     raise talosError("failed to initialize browser")
  time.sleep(browser_config['browser_wait'])
  if ffprocess.checkAllProcesses(browser_config['process'], browser_config['child_process']):
     raise talosError("browser failed to close after being initialized") 

def cleanupProfile(dir):
  # Delete the temp profile directory  Make it writeable first,
  # because every once in a while browser seems to drop a read-only
  # file into it.
  ffsetup.MakeDirectoryContentsWritable(dir)
  shutil.rmtree(dir)

def checkForCrashes(browser_config, profile_dir):
    if platform.system() in ('Windows', 'Microsoft'):
        stackwalkpaths = ['win32', 'minidump_stackwalk.exe']
    elif platform.system() == 'Linux':
        if platform.machine() == 'armv6l':
            stackwalkpaths = ['maemo', 'minidump_stackwalk']
        else:
            stackwalkpaths = ['linux', 'minidump_stackwalk']
    elif platform.system() == 'Darwin':
        stackwalkpaths = ['osx', 'minidump_stackwalk']
    else:
        return
    stackwalkbin = os.path.join(os.path.dirname(__file__), 'breakpad', *stackwalkpaths)

    found = False
    for dump in glob.glob(os.path.join(profile_dir, 'minidumps', '*.dmp')):
        utils.noisy("Found crashdump: " + dump)
        if browser_config['symbols_path']:
            nullfd = open(os.devnull, 'w')
            subprocess.call([stackwalkbin, dump, browser_config['symbols_path']], stderr=nullfd)
            nullfd.close()
        os.remove(dump)
        found = True

    if found:
        raise talosError("crash during run (stack found)")

def runTest(browser_config, test_config):
  """
  Runs an url based test on the browser as specified in the browser_config dictionary
  
  Args:
    browser_config:  Dictionary of configuration options for the browser (paths, prefs, etc)
    test_config   :  Dictionary of configuration for the given test (url, cycles, counters, etc)
  
  """
 
  utils.debug("operating with platform_type : " + platform_type)
  counters = test_config[platform_type + 'counters']
  resolution = test_config['resolution']
  all_browser_results = []
  all_counter_results = []
  format = ""
  utils.setEnvironmentVars(browser_config['env'])
  utils.setEnvironmentVars({'MOZ_CRASHREPORTER_NO_REPORT': '1'})

  if browser_config['symbols_path']:
      utils.setEnvironmentVars({'MOZ_CRASHREPORTER': '1'})
  else:
      utils.setEnvironmentVars({'MOZ_CRASHREPORTER_DISABLE': '1'})

  utils.setEnvironmentVars({"LD_LIBRARY_PATH" : os.path.dirname(browser_config['browser_path'])})

  profile_dir = None

  try:
    if ffprocess.checkAllProcesses(browser_config['process'], browser_config['child_process']):
      utils.debug(browser_config['process'] + " already running before testing started (unclean system)")
      raise talosError("system not clean")
  
    # add any provided directories to the installed browser
    for dir in browser_config['dirs']:
      ffsetup.InstallInBrowser(browser_config['browser_path'], browser_config['dirs'][dir])
  
    # make profile path work cross-platform
    test_config['profile_path'] = os.path.normpath(test_config['profile_path'])
    profile_dir, temp_dir = createProfile(test_config['profile_path'], browser_config)
    if os.path.isfile(browser_config['browser_log']):
      os.chmod(browser_config['browser_log'], 0777)
      os.remove(browser_config['browser_log'])
    initializeProfile(profile_dir, browser_config)
    
    utils.debug("initialized " + browser_config['process'])
    if test_config['shutdown']:
      shutdown = []
  
    for i in range(test_config['cycles']):
      if os.path.isfile(browser_config['browser_log']):
        os.chmod(browser_config['browser_log'], 0777)
        os.remove(browser_config['browser_log'])
      time.sleep(browser_config['browser_wait']) #wait out the browser closing
      # check to see if the previous cycle is still hanging around 
      if (i > 0) and ffprocess.checkAllProcesses(browser_config['process'], browser_config['child_process']):
        raise talosError("previous cycle still running")

      # Execute the test's head script if there is one
      if 'head' in test_config:
        try:
          subprocess.call(['python', test_config['head']])
        except:
          raise talosError("error executing head script: %s" % sys.exc_info()[0])

      # Run the test 
      browser_results = ""
      if 'timeout' in test_config:
        timeout = test_config['timeout']
      else:
        timeout = 7200 # 2 hours
      total_time = 0
      output = ''
      url = test_config['url']
      command_line = ffprocess.GenerateBrowserCommandLine(browser_config['browser_path'], browser_config['extra_args'], profile_dir, url)
  
      utils.debug("command line: " + command_line)
 
      if 'url_mod' in test_config:
        process = subprocess.Popen('python bcontroller.py --command "%s" --mod "%s" --name %s --child_process %s --timeout %d --log %s' % (command_line, test_config['url_mod'], browser_config['process'], browser_config['child_process'], browser_config['browser_wait'], browser_config['browser_log']), universal_newlines=True, shell=True, bufsize=0, env=os.environ)
      else:
        process = subprocess.Popen('python bcontroller.py --command "%s" --name %s --child_process %s --timeout %d --log %s' % (command_line, browser_config['process'], browser_config['child_process'], browser_config['browser_wait'], browser_config['browser_log']), universal_newlines=True, shell=True, bufsize=0, env=os.environ)
  
      #give browser a chance to open
      # this could mean that we are losing the first couple of data points as the tests starts, but if we don't provide
      # some time for the browser to start we have trouble connecting the CounterManager to it
      time.sleep(browser_config['browser_wait'])
      #set up the counters for this test
      if counters:
        cm = CounterManager(browser_config['process'], counters)
        cm.startMonitor()
      counter_results = {}
      for counter in counters:
        counter_results[counter] = []
     
      startTime = -1
      dumpResult = ""
      while total_time < timeout: #the main test loop, monitors counters and checks for browser ouptut
        # Sleep for [resolution] seconds
        time.sleep(resolution)
        total_time += resolution
        if os.path.isfile(browser_config['browser_log']):
          results_file = open(browser_config['browser_log'], "r")
          fileData = results_file.read()
          results_file.close()
          utils.noisy(fileData.replace(dumpResult, ''))
          dumpResult = fileData
        
        # Get the output from all the possible counters
        for count_type in counters:
          val = cm.getCounterValue(count_type)
          if (val):
            counter_results[count_type].append(val)
        if process.poll() != None: #browser_controller completed, file now full
          #stop the counter manager since this test is complete
          if counters:
            cm.stopMonitor()
          if not os.path.isfile(browser_config['browser_log']):
            raise talosError("no output from browser")
          results_file = open(browser_config['browser_log'], "r")
          results_raw = results_file.read()
          results_file.close()
  
          match = RESULTS_REGEX.search(results_raw)
          if match:
            browser_results += match.group(1)
            startTime = int(match.group(2))
            endTime = int(match.group(3))
            format = "tsformat"
            break
          #TODO: this a stop gap until all of the tests start outputting the same format
          match = RESULTS_TP_REGEX.search(results_raw)
          if match:
            browser_results += match.group(1)
            startTime = int(match.group(2))
            endTime = int(match.group(3))
            format = "tpformat"
            break
          match = RESULTS_REGEX_FAIL.search(results_raw)
          if match:
            browser_results += match.group(1)
            raise talosError(match.group(1))
          raise talosError("unrecognized output format")
  
      if total_time >= timeout:
        raise talosError("timeout exceeded")
  
      time.sleep(browser_config['browser_wait']) 
      #clean up the process
      timer = 0
      while ((process.poll() is None) and timer < browser_config['browser_wait']):
        time.sleep(1)
        timer+=1
 
      if test_config['shutdown']:
          shutdown.append(endTime - startTime)

      checkForCrashes(browser_config, profile_dir)

      all_browser_results.append(browser_results)
      all_counter_results.append(counter_results)

      # Execute the test's tail script if there is one
      if 'tail' in test_config:
        try:
          subprocess.call(['python', test_config['tail']])
        except:
          raise talosError("error executing tail script: %s" % sys.exc_info()[0])

     
    ffprocess.cleanupProcesses(browser_config['process'], browser_config['child_process'], browser_config['browser_wait']) 
    cleanupProfile(temp_dir)

    utils.restoreEnvironmentVars()
    if test_config['shutdown']:
      all_counter_results.append({'shutdown' : shutdown})      
    return (all_browser_results, all_counter_results, format)
  except:
    try:
      if 'cm' in vars():
        cm.stopMonitor()

      if os.path.isfile(browser_config['browser_log']):
        results_file = open(browser_config['browser_log'], "r")
        results_raw = results_file.read()
        results_file.close()
        utils.noisy(results_raw)

      ffprocess.cleanupProcesses(browser_config['process'], browser_config['child_process'], browser_config['browser_wait'])

      if profile_dir:
          try:
              checkForCrashes(browser_config, profile_dir)
          except talosError:
              pass

      if vars().has_key('temp_dir'):
        cleanupProfile(temp_dir)
    except talosError, te:
      utils.debug("cleanup error: " + te.msg)
    except:
      utils.debug("unknown error during cleanup")
    raise
