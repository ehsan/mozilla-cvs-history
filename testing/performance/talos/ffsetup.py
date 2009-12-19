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

"""A set of functions to set up a browser with the correct
   preferences and extensions in the given directory.

"""

__author__ = 'annie.sullivan@gmail.com (Annie Sullivan)'


import platform
import os
import os.path
import re
import shutil
import tempfile
import time
import glob

import utils
from utils import talosError
import subprocess
import ffprocess

if platform.system() == "Linux":
    from ffprofile_unix import *
elif platform.system() in ("Windows", "Microsoft"):
    from ffprofile_win32 import *
elif platform.system() == "Darwin":
    from ffprofile_unix import *

def PrefString(name, value, newline):
  """Helper function to create a pref string for profile prefs.js
     in the form 'user_pref("name", value);<newline>'

  Args:
    name: String containing name of pref
    value: String containing value of pref
    newline: Line ending to use, i.e. '\n' or '\r\n'

  Returns:
    String containing 'user_pref("name", value);<newline>'
  """

  out_value = str(value)
  if type(value) == bool:
    # Write bools as "true"/"false", not "True"/"False".
    out_value = out_value.lower()
  if type(value) == str:
    # Write strings with quotes around them.
    out_value = '"%s"' % value
  return 'user_pref("%s", %s);%s' % (name, out_value, newline)


def CreateTempProfileDir(source_profile, prefs, extensions):
  """Creates a temporary profile directory from the source profile directory
     and adds the given prefs and links to extensions.

  Args:
    source_profile: String containing the absolute path of the source profile
                    directory to copy from.
    prefs: Preferences to set in the prefs.js file of the new profile.  Format:
           {"PrefName1" : "PrefValue1", "PrefName2" : "PrefValue2"}
    extensions: Guids and paths of extensions to link to.  Format:
                {"{GUID1}" : "c:\\Path\\to\\ext1", "{GUID2}", "c:\\Path\\to\\ext2"}

  Returns:
    String containing the absolute path of the profile directory.
  """

  # Create a temporary directory for the profile, and copy the
  # source profile to it.
  temp_dir = tempfile.mkdtemp()
  profile_dir = os.path.join(temp_dir, 'profile')
  shutil.copytree(source_profile, profile_dir)
  MakeDirectoryContentsWritable(profile_dir)

  # Copy the user-set prefs to user.js
  user_js_filename = os.path.join(profile_dir, 'user.js')
  user_js_file = open(user_js_filename, 'w')
  for pref in prefs:
    user_js_file.write(PrefString(pref, prefs[pref], '\n'))
  user_js_file.close()

  # Add links to all the extensions.
  extension_dir = os.path.join(profile_dir, "extensions")
  if not os.path.exists(extension_dir):
    os.makedirs(extension_dir)
  for extension in extensions:
    link_file = open(os.path.join(extension_dir, extension), 'w')
    link_file.write(extensions[extension])
    link_file.close()

  return temp_dir, profile_dir

def InstallInBrowser(browser_path, dir_path):
  """
    Take the given directory and copies it to appropriate location in the given
    browser install
  """
  # add the provided directory to the given browser install
  fromfiles = glob.glob(os.path.join(dir_path, '*'))
  todir = os.path.join(os.path.dirname(browser_path), os.path.basename(os.path.normpath(dir_path)))
  for fromfile in fromfiles:
      if not os.path.isfile(os.path.join(todir, os.path.basename(fromfile))):
          shutil.copy(fromfile, todir)
          utils.debug("installed " + fromfile)
      else:
          utils.debug("WARNING: file already installed (" + fromfile + ")")

def InitializeNewProfile(browser_path, process, browser_wait, extra_args, profile_dir, init_url, log):
  """Runs browser with the new profile directory, to negate any performance
     hit that could occur as a result of starting up with a new profile.  
     Also kills the "extra" browser that gets spawned the first time browser
     is run with a new profile.

  Args:
    browser_path: String containing the path to the browser exe
    profile_dir: The full path to the profile directory to load
  """
  PROFILE_REGEX = re.compile('__metrics(.*)__metrics', re.DOTALL|re.MULTILINE)
  command_line = ffprocess.GenerateBrowserCommandLine(browser_path, extra_args, profile_dir, init_url)
  process = subprocess.Popen('python bcontroller.py --command "%s" --name %s --timeout %d --log %s' % (command_line, process, browser_wait, log), universal_newlines=True, shell=True, bufsize=0, env=os.environ)
  res = 0
  total_time = 0
  while total_time < 600: #10 minutes
    time.sleep(1)
    if process.poll() != None: #browser_controller completed, file now full
      if not os.path.isfile(log):
        raise talosError("no output from browser")
      results_file = open(log, "r")
      results_raw = results_file.read()
      results_file.close()
      match = PROFILE_REGEX.search(results_raw)
      if match:
        res = 1
        print match.group(1)
        break
    total_time += 1

  return res
