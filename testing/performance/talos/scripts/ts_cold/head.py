#!/usr/bin/env python

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
# The Original Code is the Talos ts_cold test.
#
# The Initial Developer of the Original Code is
# Mozilla Corporation.
# Portions created by the Initial Developer are Copyright (C) 2009
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#   Drew Willcoxon <adw@mozilla.com> (Original Author)
#   Alice Nodelman <alice@mozilla.com>
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

import os
import platform
import subprocess

def call(args):
    ret_code = subprocess.call(args)
    if ret_code != 0:
        raise StandardError(args)

if platform.system() == "Darwin":
    subprocess.call('sync')
    subprocess.call('purge')
elif platform.system() == "Linux":
    subprocess.call('sync')
    p1 = subprocess.Popen(['echo', '3'], stdout=subprocess.PIPE)
    p2 = subprocess.Popen(['sudo', 'tee', '/proc/sys/vm/drop_caches'], stdin=p1.stdout)
    p1.wait()
    p2.wait()

# We don't have a good way yet to simulate cold startup on Windows.  See
# https://wiki.mozilla.org/Firefox/Projects/Startup_Time_Improvements_Notes
# elif platform.system() in ("Windows", "Microsoft"):
                                                                         
