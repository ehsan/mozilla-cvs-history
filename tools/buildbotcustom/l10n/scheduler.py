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

from twisted.python import log
from twisted.web.client import getPage
from twisted.internet import defer, reactor
from buildbot.steps.shell import ShellCommand
from buildbot.scheduler import Nightly, Periodic, Dependent 
from buildbot.sourcestamp import SourceStamp
from buildbot.process import properties
from buildbot import buildset
import subprocess

class L10nMixin(object):
  """
  This class helps any of the L10n custom made schedulers
  to generate build objects as specified per list of locales
  """

  def __init__(self, scheduler,
               repoPath    = None,
               repoType    = 'cvs', baseTag = 'tip',
               localesFile = None,
               cvsRoot     = ':pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot',
               locales     = None):
      self.scheduler = scheduler
      self.repoType = repoType
      self.cvsRoot = cvsRoot
      # set a default localesURL accordingly to the repoType if none has been set 
      if repoType.find('hg') >= 0:
        if not localesFile:
          localesFile = "browser/locales/all-locales"          
        self.localesURL = "http://hg.mozilla.org/%s/raw-file/%s/%s" \
                          % (repoPath, baseTag, localesFile)
      elif repoType.find('cvs') >= 0:
        if not localesFile:
          self.localesURL = "mozilla/browser/locales/all-locales"
        else:
          self.localesURL = localesFile
          
      # we are going to have a queue per builder attached to the scheduler
      self.queue = {} 
      for builderName in scheduler.builderNames:
          self.queue[builderName] = []
      # if the user wants to use something different than all locales
      if locales:
          self.locales = locales[:]
      else:
          self.locales = None

  class NoMergeStamp(SourceStamp):
      """
      This source stamp implementation keeps them from being merged by
      the build master.
      """
      def canBeMergedWith(self, other):
        return False
    
  class BuildDesc(object):
      """
      All it does is to associate the self.locale property
      """
      def __init__(self, locale):
        self.locale = locale
      
      def __eq__(self, other):
        return self.locale == other.locale
      
      def __repr__(self):
        return "Build: %s" % (self.locale)
  
  def _cbLoadedLocales(self, locales):
      """
      This is the callback function that gets called once the list
      of locales are ready to be processed
      Let's fill the queues per builder and submit the BuildSets per each locale
      """
      log.msg("L10nMixin:: loaded locales' list")
      for locale in locales:
        props = properties.Properties()
        props.updateFromProperties(self.scheduler.properties)
        #I do not know exactly what to pass as the source parameter
        props.update(dict(locale=locale),'DependentL10n')
        log.msg('Submitted '+locale+' locale')
        # let's submit the BuildSet for this locale
        self.scheduler.submitBuildSet(
            buildset.BuildSet(self.scheduler.builderNames,
                              self.NoMergeStamp(branch=self.scheduler.branch),
                              self.scheduler.reason,
                              properties = props))

  def getLocales(self): 
      """
      It returns a list of locales if the user has set a list of locales
      in the scheduler OR it returns a defered
      """
      if self.locales:
        log.msg('L10nMixin.getLocales():: The user has set a list of locales')
        return self.locales
      else:
        log.msg("L10nMixin:: Getting locales from: "+self.localesURL)
        if self.repoType.find('cvs') >= 0:
           args = ['cvs', '-q', '-d', self.cvsRoot, 'co', '-p', self.localesURL]
           # communicate() returns a tuple - stdio, stderr
           # the output of cvs has a '\n' element at the end
           # a last '' string is generated that we get rid of
           return (lambda lines: [line.split(' ')[0] for line in lines]) \
                    (subprocess.Popen(args, stdout=subprocess.PIPE).communicate()[0].split('\n')[0:-1])
        else: # the repoType is 'hg'
           # getPage returns a defered that will return a string
           d = getPage(self.localesURL, timeout = 5 * 60)
           # we expect that getPage will return the output of "all-locales"
           # or "shipped-locales" or any file that contains a locale per line
           # in the begining of the line e.g. "en-GB" or "ja linux win32"
           # this code will only care about the first argument appearing in the line
           d.addCallback(lambda data: [line.split(' ')[0] \
                         for line in data.split("\n") if line])
           return d

  def createL10nBuilds(self):
      """
      We request to get the locales that we have to process and which
      method to call once they are ready
      """
      log.msg('L10nMixin:: A list of locales is going to be requested')
      d = defer.maybeDeferred(self.getLocales)
      d.addCallback(self._cbLoadedLocales)

  def getLocale(self, builderName):
      """
      This is the method that the schedulers call to request 
      the next locale to build by the builder that is doing the request
      """
      buildDescription = self.queue[builderName].pop()
      log.msg('%s requests next locale and %s was given' % (builderName, buildDescription.locale))
      return buildDescription 

class NightlyL10n(Nightly):
  """
  NightlyL10n is used to paralellize the generation of l10n builds.

  NightlyL10n is designed to be used with its special Build class,
  which actually pops the build items and moves the relevant information
  onto build properties for steps to use.
  """

  compare_attrs = ('name', 'builderNames', 
                   'minute', 'hour', 'dayOfMonth', 'month',
                   'dayOfWeek', 'branch')
  
  def __init__(self, name,  builderNames, minute=0, hour='*', dayOfMonth='*', month='*', dayOfWeek='*', 
               repoType=None, repoPath=None, baseTag=None, localesFile=None,
               branch=None, cvsRoot=None, locales=None):
    
    Nightly.__init__(self, name, builderNames, minute, hour, dayOfMonth, month, dayOfWeek, branch)
    # To avoid breakage in 1.9.0 that uses this scheduler
    # TODO: change repoType=None to repoType when fixed
    if not repoType:
        repoType = 'cvs'
    self.helper = L10nMixin(self,
                            repoType = repoType, repoPath = repoPath,
                            baseTag = baseTag,
                            localesFile = localesFile,
                            cvsRoot = cvsRoot,
                            locales = locales)
  
  def doPeriodicBuild(self):
    # Schedule the next run (as in Nightly's doPeriodicBuild)
    self.setTimer()
    self.helper.createL10nBuilds()

class DependentL10n(Dependent):
  """
  This scheduler runs some set of 'downstream' builds when the
  'upstream' scheduler has completed successfully.
  """

  compare_attrs = ('name', 'upstream', 'builders')

  def __init__(self, name, upstream, builderNames,
               repoType, repoPath=None,
               baseTag='tip', localesFile=None,
               cvsRoot=None, locales=None):
      Dependent.__init__(self, name, upstream, builderNames)
      # The next two lines has been added because of:
      # _cbLoadedLocales's BuildSet submit needs them
      self.branch = None 
      self.reason = None
      self.helper = L10nMixin(self,
                              repoType = repoType, repoPath = repoPath,
                              baseTag = baseTag,
                              localesFile = localesFile,
                              cvsRoot = cvsRoot,
                              locales = locales)

  # ss is the source stamp that we don't use currently
  def upstreamBuilt(self, ss):
      self.helper.createL10nBuilds()
