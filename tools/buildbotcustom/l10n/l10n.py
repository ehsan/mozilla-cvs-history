# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1
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
# The Original Code is Mozilla-specific Buildbot steps.
#
# The Initial Developer of the Original Code is
# Mozilla Corporation.
# Portions created by the Initial Developer are Copyright (C) 2007
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#   Axel Hecht <axel@mozilla.com>
#   Armen Zambrano Gasparnian <armenzg@mozilla.com>
# ***** END LICENSE BLOCK *****

from buildbot.steps.shell import ShellCommand
from buildbot import process

class BuildL10n(process.base.Build):
  """
  I subclass process.Build just to set some properties I get from
  the scheduler in setupBuild when I call "getNextLocale".
  """
  # this is the scheduler, needs to be set on the class in master.cfg
  buildQueue = None

  def setupBuild(self, expectations):
    #The L10n schedulers have a queue for each builder
    bd = self.buildQueue.getNextLocale(self.builder.name)
    if not bd:
      raise Exception("No build found for %s on %s, bad mojo" % \
                      (self.builder.name, self.slavename))
    process.base.Build.setupBuild(self, expectations)
    self.setProperty('locale', bd.locale)
