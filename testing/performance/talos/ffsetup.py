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
import zipfile
from xml.dom import minidom
import shutil

import utils
from utils import talosError
import subprocess

class FFSetup(object):

    ffprocess = None
    _remoteWebServer = 'localhost'
    _deviceroot = ''
    _host = ''
    _port = ''

    def __init__(self, procmgr, options = None):
        self.ffprocess = procmgr
        if options <> None:
            self.intializeRemoteDevice(options)

    def initializeRemoteDevice(self, options):
        self._remoteWebServer = options['webserver']
        self._deviceroot = options['deviceroot']
        self._host = options['host']
        self._port = options['port']
        
    def PrefString(self, name, value, newline):
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

    def install_addon(self, profile_path, addon):
        """Installs the given addon in the profile.
           most of this borrowed from mozrunner, except downgraded to work on python 2.4
           # Contributor(s) for mozrunner:
           # Mikeal Rogers <mikeal.rogers@gmail.com>
           # Clint Talbert <ctalbert@mozilla.com>
           # Henrik Skupin <hskupin@mozilla.com>
        """
        def find_id(desc):
            addon_id = None
            for elem in desc:
                apps = elem.getElementsByTagName('em:targetApplication')
                if apps:
                    for app in apps:
                        #remove targetApplication nodes, they contain id's we aren't interested in
                        elem.removeChild(app)
                    if elem.getElementsByTagName('em:id'):
                        addon_id = str(elem.getElementsByTagName('em:id')[0].firstChild.data)
                    elif elem.hasAttribute('em:id'):
                        addon_id = str(elem.getAttribute('em:id'))
            return addon_id

        tmpdir = None
        addon_id = None
        tmpdir = tempfile.mkdtemp(suffix = "." + os.path.split(addon)[-1])
        compressed_file = zipfile.ZipFile(addon, "r")
        #in python2.6 can use extractall, currently limited to python2.4
        for name in compressed_file.namelist():
            if name.endswith('/'):
                os.makedirs(os.path.join(tmpdir, name))
            else:
                if not os.path.isdir(os.path.dirname(os.path.join(tmpdir, name))):
                    os.makedirs(os.path.dirname(os.path.join(tmpdir, name)))
                data = compressed_file.read(name)
                f = open(os.path.join(tmpdir, name), 'w')
                f.write(data) ; f.close()
        addon = tmpdir

        doc = minidom.parse(os.path.join(addon, 'install.rdf')) 
        # description_element =
        # tree.find('.//{http://www.w3.org/1999/02/22-rdf-syntax-ns#}Description/')

        desc = doc.getElementsByTagName('Description')
        addon_id = find_id(desc)
        if not addon_id:
          desc = doc.getElementsByTagName('RDF:Description')
          addon_id = find_id(desc)
        
        if not addon_id: #bail out, we don't have an addon id
            raise talosError("no addon_id found for extension")
                 
        addon_path = os.path.join(profile_path, 'extensions', addon_id)
        #if an old copy is already installed, remove it 
        if os.path.isdir(addon_path): 
            shutil.rmtree(addon_path, ignore_errors=True) 
        shutil.move(addon, addon_path) 

    def CreateTempProfileDir(self, source_profile, prefs, extensions):
        """Creates a temporary profile directory from the source profile directory
            and adds the given prefs and links to extensions.

        Args:
            source_profile: String containing the absolute path of the source profile
                            directory to copy from.
            prefs: Preferences to set in the prefs.js file of the new profile.  Format:
                    {"PrefName1" : "PrefValue1", "PrefName2" : "PrefValue2"}
            extensions: list of paths to .xpi files to be installed

        Returns:
            String containing the absolute path of the profile directory.
        """

        # Create a temporary directory for the profile, and copy the
        # source profile to it.
        temp_dir = tempfile.mkdtemp()
        profile_dir = os.path.join(temp_dir, 'profile')
        shutil.copytree(source_profile, profile_dir)
        self.ffprocess.MakeDirectoryContentsWritable(profile_dir)

        # Copy the user-set prefs to user.js
        user_js_filename = os.path.join(profile_dir, 'user.js')
        user_js_file = open(user_js_filename, 'w')
        for pref in prefs:
            user_js_file.write(self.PrefString(pref, prefs[pref], '\n'))

        user_js_file.close()

        if (self._remoteWebServer <> 'localhost'):
             self.ffprocess.addRemoteServerPref(profile_dir, self._remoteWebServer)

        # Add links to all the extensions.
        extension_dir = os.path.join(profile_dir, "extensions")
        if not os.path.exists(extension_dir):
            os.makedirs(extension_dir)
        for addon in extensions:
            self.install_addon(profile_dir, addon)

        if (self._remoteWebServer <> 'localhost'):
            remote_dir = self.ffprocess.copyDirToDevice(profile_dir)
            profile_dir = remote_dir
        return temp_dir, profile_dir
        
    def InstallInBrowser(self, browser_path, dir_path):
        """
            Take the given directory and copies it to appropriate location in the given
            browser install
        """
        # add the provided directory to the given browser install
        fromfiles = glob.glob(os.path.join(dir_path, '*'))
        todir = os.path.join(os.path.dirname(browser_path), os.path.basename(os.path.normpath(dir_path)))
        for fromfile in fromfiles:
            self.ffprocess.copyFile(fromfile, todir)

    def InitializeNewProfile(self, browser_path, process, child_process, browser_wait, extra_args, profile_dir, init_url, log):
        """Runs browser with the new profile directory, to negate any performance
            hit that could occur as a result of starting up with a new profile.  
            Also kills the "extra" browser that gets spawned the first time browser
            is run with a new profile.

        Args:
            browser_path: String containing the path to the browser exe
            profile_dir: The full path to the profile directory to load
        """
        PROFILE_REGEX = re.compile('__metrics(.*)__metrics', re.DOTALL|re.MULTILINE)
        command_line = self.ffprocess.GenerateBrowserCommandLine(browser_path, extra_args, profile_dir, init_url)

        b_cmd = 'python bcontroller.py --command "%s"' % (command_line)
        b_cmd += " --name %s" % (process)
        b_cmd += " --child_process %s" % (child_process)
        b_cmd += " --timeout %d" % (browser_wait)
        b_cmd += " --log %s" % (log)

        if (self._remoteWebServer <> 'localhost'):
            b_cmd += ' --host %s' % (self._host)
            b_cmd += ' --port %s' % (self._port)
            b_cmd += ' --deviceRoot %s' % (self._deviceroot)

        process = subprocess.Popen(b_cmd, universal_newlines=True, shell=True, bufsize=0, env=os.environ)

        timeout = True
        total_time = 0
        while total_time < 1200: #20 minutes
            time.sleep(1)
            if process.poll() != None: #browser_controller completed, file now full
                timeout = False
                break
            total_time += 1

        res = 0
        if (timeout == False):
            if not os.path.isfile(log):
                raise talosError("initalization has no output from browser")
            results_file = open(log, "r")
            results_raw = results_file.read()
            results_file.close()
            match = PROFILE_REGEX.search(results_raw)
            if match:
                res = 1
                print match.group(1)
        else:
            raise talosError("initialization timed out")

        return res

