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

__author__ = 'annie.sullivan@gmail.com (Annie Sullivan)'


import time
import yaml
import sys
import urllib 
import os
import string
import socket
socket.setdefaulttimeout(480)
import getopt
import re

import utils
from utils import talosError
import post_file
from ttest import TTest

def shortName(name):
  if name == "Working Set":
    return "memset"
  elif name == "% Processor Time":
    return "%cpu"
  elif name == "Private Bytes":
    return "pbytes"
  elif name == "RSS":
    return "rss"
  elif name == "XRes":
    return "xres"
  else: 
    return name

def process_tpformat(line):
  # each line of the string is of the format i;page_name;median;mean;min;max;time vals\n
  r = line.split(';')
  #skip this line if it isn't the correct format
  if len(r) == 1:
      return -1, ''
  r[1] = r[1].rstrip('/')
  if r[1].find('/') > -1 :
     page = r[1].split('/')[0]
  else:
     page = r[1]
  try:
    val = float(r[2])
  except ValueError:
    print 'WARNING: value error for median in tp'
    val = 0
  return val, page

def process_Request(post):
  links = ""
  lines = post.split('\n')
  for line in lines:
    if line.find("RETURN\t") > -1:
        links += line.replace("RETURN\t", "") + '\n'
    utils.debug("process_Request line: " + line.replace("RETURN\t", ""))
  if not links:
    raise talosError("send failed, graph server says:\n" + post)
  return links

def send_to_csv(csv_dir, results):
  import csv
  for res in results:
    browser_dump, counter_dump, print_format = results[res]
    writer = csv.writer(open(os.path.join(csv_dir, res + '.csv'), "wb"))
    if print_format == 'tsformat':
      i = 0
      writer.writerow(['i', 'val'])
      for val in browser_dump:
        val_list = val.split('|')
        for v in val_list:
          writer.writerow([i, v])
          i += 1
    elif print_format == 'tpformat':
      writer.writerow(['i', 'page', 'median', 'mean', 'min' , 'max', 'runs'])
      for bd in browser_dump:
        bd.rstrip('\n')
        page_results = bd.splitlines()
        i = 0
        for mypage in page_results:
          r = mypage.split(';')
          #skip this line if it isn't the correct format
          if len(r) == 1:
              continue
          r[1] = r[1].rstrip('/')
          if r[1].find('/') > -1 :
             page = r[1].split('/')[1]
          else:
             page = r[1]
          writer.writerow([i, page, r[2], r[3], r[4], r[5], '|'.join(r[6:])])
          i += 1
    else:
      raise talosError("Unknown print format in send_to_csv")
    for cd in counter_dump:
      for count_type in cd:
        writer = csv.writer(open(os.path.join(csv_dir, res + '_' + count_type + '.csv'), "wb"))
        writer.writerow(['i', 'value'])
        i = 0
        for val in cd[count_type]:
          writer.writerow([i, val])
          i += 1

def filesizeformat(bytes):
    """
    Format the value like a 'human-readable' file size (i.e. 13 KB, 4.1 MB, 102
    bytes, etc).
    """
    bytes = float(bytes)
    if bytes < 1024:
        return "%dB" % (bytes)
    if bytes < 1024 * 1024:
        return "%.1fKB" % (bytes / 1024)
    if bytes < 1024 * 1024 * 1024:
        return "%.1fMB" % (bytes / (1024 * 1024))
    return "%.1fGB" % (bytes / (1024 * 1024 * 1024))

def construct_results (machine, testname, branch, sourcestamp, buildid, date, vals):
  """ 
  Creates string formated for the collector script of the graph server
  Returns the completed string
  """
  #machine_name,test_name,branch_name,sourcestamp,buildid,date_run
  info_format = "%s,%s,%s,%s,%s,%s\n"
  data_string = ""
  data_string += "START\n"
  data_string += "VALUES\n"
  data_string += info_format % (machine, testname, branch, sourcestamp, buildid, date)
  i = 0
  for val, page in vals:
    data_string += "%d,%.2f,%s\n" % (i,float(val), page)
    i += 1
  data_string += "END"
  return data_string

def send_to_graph(results_server, results_link, machine, date, browser_config, results):
  links = ''
  result_strings = []

  #construct all the strings of data, one string per test and one string  per counter
  for testname in results:
    vals = []
    fullname = testname
    browser_dump, counter_dump, print_format = results[testname]
    utils.debug("Working with test: " + testname)
    utils.debug("Sending results: " + " ".join(browser_dump))
    utils.stamped_msg("Generating results file: " + testname, "Started")
    if print_format == 'tsformat':
      #non-tpformat results
      for bd in browser_dump:
        vals.extend([[x, 'NULL'] for x in bd.split('|')])
    elif print_format == 'tpformat':
      #tpformat results
      fullname += browser_config['test_name_extension']
      for bd in browser_dump:
        bd.rstrip('\n')
        page_results = bd.splitlines()
        for line in page_results:
          val, page = process_tpformat(line)
          if val > -1 :
            vals.append([val, page])
    else:
      raise talosError("Unknown print format in send_to_graph")
    result_strings.append(construct_results(machine, fullname, browser_config['branch_name'], browser_config['sourcestamp'], browser_config['buildid'], date, vals))
    utils.stamped_msg("Generating results file: " + testname, "Stopped")
    #counters collected for this test
    for cd in counter_dump:
      for count_type in cd:
        vals = [[x, 'NULL'] for x in cd[count_type]]
        counterName = testname + '_' + shortName(count_type)
        utils.stamped_msg("Generating results file: " + counterName, "Started")
        if print_format == "tpformat":
          counterName += browser_config['test_name_extension']
        utils.stamped_msg("Generating results file: " + counterName, "Started")
        result_strings.append(construct_results(machine, counterName, browser_config['branch_name'], browser_config['sourcestamp'], browser_config['buildid'], date, vals))
        utils.stamped_msg("Generating results file: " + counterName, "Stopped")
    
  #send all the strings along to the graph server
  for data_string in result_strings:
    RETRIES = 5
    wait_time = 5
    times = 0
    msg = ""
    while (times < RETRIES):
      try:
        utils.stamped_msg("Transmitting test: " + testname, "Started")
        links += process_Request(post_file.post_multipart(results_server, results_link, [("key", "value")], [("filename", "data_string", data_string)]))
        break
      except talosError, e:
        times += 1
        msg = e.msg
        time.sleep(wait_time)
        wait_time += wait_time
    if times == RETRIES:
        raise talosError("Failed to send data %d times... quitting\n%s" % (RETRIES, msg))
    utils.stamped_msg("Transmitting test: " + testname, "Stopped")

  return links

def results_from_graph(links, results_server):
  #take the results from the graph server collection script and put it into a pretty format for the waterfall
  url_format = "http://%s/%s"
  link_format= "<a href=\'%s\'>%s</a>"
  first_results = 'RETURN:<br>'
  last_results = '' 
  full_results = '\nRETURN:<p style="font-size:smaller;">Details:<br>'  
  memory_metric = ['memset', 'rss', 'pbytes', 'xres']
  lines = links.split('\n')
  for line in lines:
    if line == "":
      continue
    linkvalue = -1
    linkdetail = ""
    values = line.split("\t")
    linkName = values[0]
    if len(values) == 2:
      linkdetail = values[1]
    else:
      linkvalue = float(values[1])
      linkdetail = values[2]
    if linkvalue > -1:
      if filter(lambda x: x in linkName, memory_metric): #measured in bytes
        linkName += ": " + filesizeformat(linkvalue)
      else:
        linkName += ": " + str(linkvalue)
      url = url_format % (results_server, linkdetail)
      link = link_format % (url, linkName)
      first_results = first_results + "\nRETURN:" + link + "<br>"
    else:
      url = url_format % (results_server, linkdetail)
      link = link_format % (url, linkName)
      last_results = last_results + '| ' + link + ' '
  full_results = first_results + full_results + last_results + '|</p>'
  print 'RETURN: new graph links'
  print full_results

def browserInfo(browser_config):
  """Get the buildid and sourcestamp from the application.ini (if it exists)
  """
  appIniFileName = "application.ini"
  appIniPath = os.path.join(os.path.dirname(browser_config['browser_path']), appIniFileName)
  if os.path.isfile(appIniPath):
    appIni = open(appIniPath)
    appIniContents = appIni.readlines()
    appIni.close()
    reSourceStamp = re.compile('SourceStamp\s*=\s*(.*)$')
    reRepository = re.compile('SourceRepository\s*=\s*(.*)$')
    reBuildID = re.compile('BuildID\s*=\s*(.*)$')
    for line in appIniContents:
      match = re.match(reBuildID, line)
      if match:
        browser_config['buildid'] = match.group(1)
        print 'RETURN:id:' + browser_config['buildid']
      match = re.match(reRepository, line)
      if match:
          browser_config['repository'] = match.group(1)
      match = re.match(reSourceStamp, line)
      if match:
          browser_config['sourcestamp'] = match.group(1)
    if ('repository' in browser_config) and ('sourcestamp' in browser_config):
      print 'RETURN:<a href = "' + browser_config['repository'] + '/rev/' + browser_config['sourcestamp'] + '">rev:' + browser_config['sourcestamp'] + '</a>'
    else:
        browser_config['repository'] = 'NULL'
        browser_config['sourcestamp'] = 'NULL'
  return browser_config

def test_file(filename):
  """Runs the talos tests on the given config file and generates a report.
  
  Args:
    filename: the name of the file to run the tests on
  """
  
  browser_config = []
  tests = []
  title = ''
  testdate = ''
  csv_dir = ''
  results_server = ''
  results_link = ''
  results = {}
  
  # Read in the profile info from the YAML config file
  config_file = open(filename, 'r')
  yaml_config = yaml.load(config_file)
  config_file.close()
  for item in yaml_config:
    if item == 'title':
      title = yaml_config[item]
    elif item == 'testdate':
      testdate = yaml_config[item]
    elif item == 'csv_dir':
       csv_dir = os.path.normpath(yaml_config[item])
       if not os.path.exists(csv_dir):
         print "FAIL: path \"" + csv_dir + "\" does not exist"
         sys.exit(0)
    elif item == 'results_server':
       results_server = yaml_config[item]
    elif item == 'results_link' :
       results_link = yaml_config[item]
  if (results_link != results_server != ''):
    if not post_file.link_exists(results_server, results_link):
      sys.exit(0)
  browser_config = {'preferences'  : yaml_config['preferences'],
                    'extensions'   : yaml_config['extensions'],
                    'browser_path' : yaml_config['browser_path'],
                    'browser_log'  : yaml_config['browser_log'],
                    'symbols_path' : yaml_config.get('symbols_path', None),
                    'browser_wait' : yaml_config['browser_wait'],
                    'process'      : yaml_config['process'],
                    'extra_args'   : yaml_config['extra_args'],
                    'branch'       : yaml_config['branch'],
                    'buildid'      : yaml_config['buildid'],
                    'env'          : yaml_config['env'],
                    'dirs'         : yaml_config['dirs'],
                    'init_url'     : yaml_config['init_url']}
  if 'child_process' in yaml_config:
      browser_config['child_process'] = yaml_config['child_process']
  else:
      browser_config['child_process'] = 'mozilla-runtime'
  if 'branch_name' in yaml_config:
      browser_config['branch_name'] = yaml_config['branch_name']
  if 'test_name_extension' in yaml_config:
      browser_config['test_name_extension'] = yaml_config['test_name_extension']
  else:
      browser_config['test_name_extension'] = ''
  #normalize paths to work accross platforms
  browser_config['browser_path'] = os.path.normpath(browser_config['browser_path'])
  for dir in browser_config['dirs']:
    browser_config['dirs'][dir] = os.path.normpath(browser_config['dirs'][dir])
  tests = yaml_config['tests']
  config_file.close()
  if (testdate != ''):
    date = int(time.mktime(time.strptime(testdate, '%a, %d %b %Y %H:%M:%S GMT')))
  else:
    date = int(time.time()) #TODO get this into own file
  utils.debug("using testdate: %d" % date)
  utils.debug("actual date: %d" % int(time.time()))
  print 'RETURN:s: %s' % title
  #pull buildid & sourcestamp from browser
  browser_config = browserInfo(browser_config)

  utils.startTimer()
  utils.stamped_msg(title, "Started")
  for test in tests:
    testname = test['name']
    utils.stamped_msg("Running test " + testname, "Started")
    try:
      browser_dump, counter_dump, print_format = TTest().runTest(browser_config, test)
      utils.debug("Received test results: " + " ".join(browser_dump))
      results[testname] = [browser_dump, counter_dump, print_format]
      # If we're doing CSV, write this test immediately (bug 419367)
      if csv_dir != '':
        send_to_csv(csv_dir, {testname : results[testname]})
    except talosError, e:
      utils.stamped_msg("Failed " + testname, "Stopped")
      print 'FAIL: Busted: ' + testname
      print 'FAIL: ' + e.msg.replace('\n','\nRETURN:')
    utils.stamped_msg("Completed test " + testname, "Stopped")
  elapsed = utils.stopTimer()
  print "RETURN: cycle time: " + elapsed + "<br>"
  utils.stamped_msg(title, "Stopped")

  #process the results
  if (results_server != '') and (results_link != ''):
    #send results to the graph server
    try:
      utils.stamped_msg("Sending results", "Started")
      links = send_to_graph(results_server, results_link, title, date, browser_config, results)
      results_from_graph(links, results_server)
      utils.stamped_msg("Completed sending results", "Stopped")
    except talosError, e:
      utils.stamped_msg("Failed sending results", "Stopped")
      print 'FAIL: ' + e.msg.replace('\n', '\nRETURN:')
  
if __name__=='__main__':
  optlist, args = getopt.getopt(sys.argv[1:], 'dn', ['debug', 'noisy'])
  for o, a in optlist:
    if o in ('-d', "--debug"):
      print 'setting debug'
      utils.setdebug(1)
    if o in ('-n', "--noisy"):
      utils.setnoisy(1)
  # Read in each config file and run the tests on it.
  for arg in args:
    utils.debug("running test file " + arg)
    test_file(arg)

