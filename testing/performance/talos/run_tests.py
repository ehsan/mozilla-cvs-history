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
import tempfile
import os
import string
import socket
socket.setdefaulttimeout(480)
import getopt
import re

import utils
from utils import talosError
import post_file
import ttest

def shortNames(name):
  if name == "tp_loadtime":
    return "tp"
  elif name == "tp_js_loadtime":
    return "tp_js_l"
  elif name == "tp_Percent Processor Time":
    return "tp_%cpu"
  elif name == "tp_Working Set":
    return "tp_memset"
  elif name == "tp_Private Bytes":
    return "tp_pbytes"
  else:
    return name

def process_Request(post):
  str = ""
  lines = post.split('\n')
  for line in lines:
    if line.find("RETURN:") > -1:
        str += line.split(":")[3] + ":" + shortNames(line.split(":")[1]) + ":" + line.split(":")[2] + '\n'
    utils.debug("process_Request line: " + line.replace("RETURN", ""))
  return str

def send_to_csv(csv_dir, results):
  import csv
  for res in results:
    browser_dump, counter_dump = results[res]
    writer = csv.writer(open(os.path.join(csv_dir, res + '.csv'), "wb"))
    if res in ('ts', 'twinopen'):
      i = 0
      writer.writerow(['i', 'val'])
      for val in browser_dump:
        val_list = val.split('|')
        for v in val_list:
          writer.writerow([i, v])
          i += 1
    else:
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
    for cd in counter_dump:
      for count_type in cd:
        writer = csv.writer(open(os.path.join(csv_dir, res + '_' + count_type + '.csv'), "wb"))
        writer.writerow(['i', 'value'])
        i = 0
        for val in cd[count_type]:
          writer.writerow([i, val])
          i += 1

def post_chunk(results_server, results_link, id, filename):
  tmpf = open(filename, "r")
  file_data = tmpf.read()
  try:
    ret = post_file.post_multipart(results_server, results_link, [("key", "value")], [("filename", filename, file_data)])
  except:
    print "FAIL: error in post data"
    sys.exit(0)
  links = process_Request(ret)
  utils.debug(id + ": sent results")
  return links

def chunk_list(val_list):
  """ 
    divide up a list into manageable chunks
    currently set at length 500 
    this is for a failure on mac os x with python 2.4.4 
  """
  chunks = []
  end = 500
  while (val_list != []):
    chunks.append(val_list[0:end])
    val_list = val_list[end:len(val_list)]
  return chunks

def send_to_graph(results_server, results_link, title, date, browser_config, results):
  tbox = title
  url_format = "http://%s/%s"
  link_format= "<a href=\"%s\">%s</a>"
  #value, testname, tbox, timeval, date, branch, buildid, type, data
  result_format = "%.2f,%s,%s,%d,%d,%s,%s,%s,%s,\n"
  result_format2 = "%.2f,%s,%s,%d,%d,%s,%s,%s,\n"
  links = ''

  for res in results:
    browser_dump, counter_dump = results[res]
    utils.debug("Working with test: " + res)
    utils.debug("Sending results: " + " ".join(browser_dump))
    utils.stamped_msg("Transmitting test: " + res, "Started")
    filename = tempfile.mktemp()
    tmpf = open(filename, "w")
    if res in ('ts', 'twinopen'):
       i = 0
       for val in browser_dump:
        val_list = val.split('|')
        for v in val_list:
          tmpf.write(result_format % (float(v), res, tbox, i, date, browser_config['branch'], browser_config['buildid'], "discrete", "ms"))
          i += 1
    else:
      # each line of the string is of the format i;page_name;median;mean;min;max;time vals\n
      name = ''
      if ((res == 'tp') or (res == 'tp_js')):
          name = '_loadtime'
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
          try:
            val = float(r[2])
          except ValueError:
            print 'WARNING: value error for median in tp'
            val = 0
          tmpf.write(result_format % (val, res + name, tbox, i, date, browser_config['branch'], browser_config['buildid'], "discrete", page))
          i += 1
    tmpf.flush()
    tmpf.close()
    links += post_chunk(results_server, results_link, res, filename)
    os.remove(filename)
    for cd in counter_dump:
      for count_type in cd:
        val_list = cd[count_type]
        chunks = chunk_list(val_list)
        chunk_link = ''
        i = 0
        for chunk in chunks:
          filename = tempfile.mktemp()
          tmpf = open(filename, "w")
          for val in chunk:
              tmpf.write(result_format2 % (float(val), res + "_" + count_type.replace("%", "Percent"), tbox, i, date, browser_config['branch'], browser_config['buildid'], "discrete"))
              i += 1
          tmpf.flush()
          tmpf.close()
          chunk_link = post_chunk(results_server, results_link, '%s_%s (%d values)' % (res, count_type, len(chunk)), filename)
          os.remove(filename)
        links += chunk_link
    utils.stamped_msg("Transmitting test: " + res, "Stopped")
 
  first_results = 'RETURN:<br>'
  last_results = '' 
  full_results = '\nRETURN:<p style="font-size:smaller;">Details:<br>'  
  lines = links.split('\n')
  for line in lines:
    if line == "":
      continue
    values = line.split(":")
    linkName = values[1]
    if linkName in ('tp_pbytes', 'tp_%cpu'):
      continue
    if float(values[2]) > 0:
      linkName += ": " + str(values[2])
      url = url_format % (results_server, values[0])
      link = link_format % (url, linkName)
      first_results = first_results + "\nRETURN:" + link + "<br>"
    else:
      url = url_format % (results_server, values[0])
      link = link_format % (url, linkName)
      last_results = last_results + '| ' + link + ' '
  full_results = first_results + full_results + last_results + '|</p>'
  print full_results

def browserInfo(browser_config):
  """Get the buildid and changeset from the application.ini (if it exists)
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
  return browser_config

def test_file(filename):
  """Runs the Ts and Tp tests on the given config file and generates a report.
  
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
                    'browser_wait' : yaml_config['browser_wait'],
                    'process'      : yaml_config['process'],
                    'extra_args'   : yaml_config['extra_args'],
                    'branch'       : yaml_config['branch'],
                    'buildid'      : yaml_config['buildid'],
                    'profile_path' : yaml_config['profile_path'],
                    'env'          : yaml_config['env'],
                    'dirs'         : yaml_config['dirs'],
                    'init_url'     : yaml_config['init_url']}
  #normalize paths to work accross platforms
  browser_config['browser_path'] = os.path.normpath(browser_config['browser_path'])
  if browser_config['profile_path'] != {}:
    browser_config['profile_path'] = os.path.normpath(browser_config['profile_path'])
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
  #pull buildid & sourcestamp from browser
  browser_config = browserInfo(browser_config)

  utils.stamped_msg(title, "Started")
  for test in tests:
    testname = test['name']
    utils.stamped_msg("Running test " + testname, "Started")
    try:
      browser_dump, counter_dump = ttest.runTest(browser_config, test)
    except talosError, e:
      utils.stamped_msg("Failed " + testname, "Stopped")
      print 'FAIL: Busted: ' + testname
      print 'FAIL: ' + e.msg
      sys.exit(0)
    utils.debug("Received test results: " + " ".join(browser_dump))
    results[testname] = [browser_dump, counter_dump]
    utils.stamped_msg("Completed test " + testname, "Stopped")
  utils.stamped_msg(title, "Stopped")

  #process the results
  if (results_server != '') and (results_link != ''):
    #send results to the graph server
    utils.stamped_msg("Sending results", "Started")
    send_to_graph(results_server, results_link, title, date, browser_config, results)
    utils.stamped_msg("Completed sending results", "Stopped")
  if csv_dir != '':
    send_to_csv(csv_dir, results)
  
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

