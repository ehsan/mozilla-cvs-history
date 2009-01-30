# -*- Python -*-

from twisted.python import log
from buildbot import buildset
from buildbot.scheduler import Scheduler
from buildbot.process.buildstep import BuildStep
from buildbot.process.factory import BuildFactory
from buildbot.buildset import BuildSet
from buildbot.sourcestamp import SourceStamp
from buildbot.steps.shell import ShellCommand
from buildbot.steps.transfer import FileDownload
from buildbot.status.builder import SUCCESS, WARNINGS, FAILURE, SKIPPED, EXCEPTION
import re, urllib, sys, os
from time import strptime, strftime, localtime
from datetime import datetime
from os import path
import copy

MozillaEnvironments = { }

# platform SDK location.  we can build both from one generic template.
# modified from vc8 environment
MozillaEnvironments['vc8perf'] = {
    "MOZ_CRASHREPORTER_NO_REPORT": '1',
    "MOZ_NO_REMOTE": '1',
    "NO_EM_RESTART": '1',
    "XPCOM_DEBUG_BREAK": 'warn',
    "CYGWINBASE": 'C:\\cygwin',
    "PATH": 'C:\\Python24;' + \
            'C:\\Python24\\Scripts;' + \
            'C:\\cygwin\\bin;' + \
            'C:\\WINDOWS\\System32;' + \
            'C:\\program files\\gnuwin32\\bin;' + \
            'C:\\WINDOWS;'
}

MozillaEnvironments['linux'] = {
    "MOZ_CRASHREPORTER_NO_REPORT": '1',
    "MOZ_NO_REMOTE": '1',
    "NO_EM_RESTART": '1',
    "XPCOM_DEBUG_BREAK": 'warn',
    "DISPLAY": ":0",
}

MozillaEnvironments['mac'] = {
    "MOZ_NO_REMOTE": '1',
    "NO_EM_RESTART": '1',
    "XPCOM_DEBUG_BREAK": 'warn',
    "MOZ_CRASHREPORTER_NO_REPORT": '1',
    # for extracting dmg's
    "PAGER": '/bin/cat',
}

class MultiBuildScheduler(Scheduler):
    """Trigger N (default three) build requests based upon the same change request"""
    def __init__(self, numberOfBuildsToTrigger=3, **kwargs):
        self.numberOfBuildsToTrigger = numberOfBuildsToTrigger
        Scheduler.__init__(self, **kwargs)

    def fireTimer(self):
        self.timer = None
        self.nextBuildTime = None
        changes = self.importantChanges + self.unimportantChanges
        self.importantChanges = []
        self.unimportantChanges = []

        # submit
        for i in range(0, self.numberOfBuildsToTrigger):
            bs = buildset.BuildSet(self.builderNames, SourceStamp(changes=changes))
            self.submit(bs)

class MozillaWget(ShellCommand):
    """Download built Firefox client from dated staging directory."""
    haltOnFailure = True
    
    def __init__(self, **kwargs):
        self.branch = "HEAD"
        if 'branch' in kwargs:
            self.branch = kwargs['branch']
        if not 'command' in kwargs:
            kwargs['command'] = ["wget"]
        ShellCommand.__init__(self, **kwargs)

    def setBuild(self, build):
        ShellCommand.setBuild(self, build)
        self.changes = build.source.changes
        #a full path is always provided by the poller 
        self.fileURL = self.changes[-1].links
        self.filename = self.changes[-1].files[0]
    
    def getFilename(self):
        return self.filename
    
    def describe(self, done=False):
        return ["Wget Download"]
    
    def start(self):
        if self.branch:
            self.setProperty("fileURL", self.fileURL)
            self.setProperty("filename", self.filename)
        self.setCommand(["wget", "-nv", "-N", self.fileURL])
        ShellCommand.start(self)
    
    def evaluateCommand(self, cmd):
        superResult = ShellCommand.evaluateCommand(self, cmd)
        if SUCCESS != superResult:
            return FAILURE
        if None != re.search('ERROR', cmd.logs['stdio'].getText()):
            return FAILURE
        return SUCCESS
    

class MozillaInstallZip(ShellCommand):
    """Install given file, unzipping to executablePath"""
    
    def __init__(self, **kwargs):
        self.filename = ""
        self.branch = ""
        if 'branch' in kwargs:
            self.branch = kwargs['branch']
        if 'filename' in kwargs:
            self.filename = kwargs['filename']
        if not 'command' in kwargs:
            kwargs['command'] = ["unzip", "-o"]
        ShellCommand.__init__(self, **kwargs)
    
    def describe(self, done=False):
        return ["Install zip"]
    
    def start(self):
        # removed the mkdir because this happens on the master, not the slave
        if not self.filename:
            if self.branch:
                self.filename = self.getProperty("filename")
            else:
                return FAILURE
        if self.filename:
            self.command = self.command[:] + [self.filename]
        ShellCommand.start(self)
    
    def evaluateCommand(self, cmd):
        superResult = ShellCommand.evaluateCommand(self, cmd)
        if SUCCESS != superResult:
            return FAILURE
        if None != re.search('ERROR', cmd.logs['stdio'].getText()):
            return FAILURE
        if None != re.search('Usage:', cmd.logs['stdio'].getText()):
            return FAILURE
        return SUCCESS
    

class MozillaUpdateConfig(ShellCommand):
    """Configure YAML file for run_tests.py"""
   
    def __init__(self, **kwargs):
        self.addOptions = []
        assert 'executablePath' in kwargs
        assert 'branch' in kwargs
        self.branch = kwargs['branch']
        self.exePath = kwargs['executablePath']
        if 'addOptions' in kwargs:
            self.addOptions = kwargs['addOptions']
        ShellCommand.__init__(self, **kwargs)

    def setBuild(self, build):
        ShellCommand.setBuild(self, build)
        self.title = build.slavename
        self.changes = build.source.changes
        self.buildid = strftime("%Y%m%d%H%M", localtime(self.changes[-1].when))
        if not self.command:
            self.setCommand(["python", "PerfConfigurator.py", "-v", "-e", self.exePath, "-t", self.title, "-b", self.branch, "-d", self.buildid, "-i", self.buildid] + self.addOptions)

    def describe(self, done=False):
        return ["Update config"]
    
    def evaluateCommand(self, cmd):
        superResult = ShellCommand.evaluateCommand(self, cmd)
        if SUCCESS != superResult:
            return FAILURE
        stdioText = cmd.logs['stdio'].getText()
        if None != re.search('ERROR', stdioText):
            return FAILURE
        if None != re.search('USAGE:', stdioText):
            return FAILURE
        configFileMatch = re.search('outputName\s*=\s*(\w*?.yml)', stdioText)
        if not configFileMatch:
            return FAILURE
        else:
            self.setProperty("configFile", configFileMatch.group(1))
        return SUCCESS
    

class MozillaRunPerfTests(ShellCommand):
    """Run the performance tests"""
    
    def __init__(self, **kwargs):
        if 'branch' in kwargs:
            self.branch = kwargs['branch']
        if not 'command' in kwargs:
            kwargs['command'] = ["python", "run_tests.py"]
        ShellCommand.__init__(self, **kwargs)
    
    def describe(self, done=False):
        return ["Run performance tests"]
    
    def createSummary(self, log):
        summary = []
        for line in log.readlines():
            if "RETURN:" in line:
                summary.append(line.replace("RETURN:", "TinderboxPrint:"))
            if "FAIL:" in line:
                summary.append(line.replace("FAIL:", "TinderboxPrint:FAIL:"))
        self.addCompleteLog('summary', "\n".join(summary))
    
    def start(self):
        """docstring for start"""
        self.command = copy.copy(self.command)
        self.command.append(self.getProperty("configFile"))
        ShellCommand.start(self)
    
    def evaluateCommand(self, cmd):
        superResult = ShellCommand.evaluateCommand(self, cmd)
        stdioText = cmd.logs['stdio'].getText()
        if SUCCESS != superResult:
            return FAILURE
        if None != re.search('ERROR', stdioText):
            return FAILURE
        if None != re.search('USAGE:', stdioText):
            return FAILURE
        if None != re.search('FAIL:', stdioText):
            return WARNINGS
        return SUCCESS

class MozillaInstallTarBz2(ShellCommand):
    """Install given file, unzipping to executablePath"""
    
    def __init__(self, **kwargs):
        self.filename = ""
        self.branch = ""
        if 'branch' in kwargs:
            self.branch = kwargs['branch']
        if 'filename' in kwargs:
            self.filename = kwargs['filename']
        if not 'command' in kwargs:
            kwargs['command'] = ["tar", "-jvxf"]
        ShellCommand.__init__(self, **kwargs)
    
    def describe(self, done=False):
        return ["Install tar.bz2"]
    
    def start(self):
        if not self.filename:
            if self.branch:
                self.filename = self.getProperty("filename")
            else:
                return FAILURE
        if self.filename:
            self.command = self.command[:] + [self.filename]
        ShellCommand.start(self)
    
    def evaluateCommand(self, cmd):
        superResult = ShellCommand.evaluateCommand(self, cmd)
        if SUCCESS != superResult:
            return FAILURE
        return SUCCESS

class MozillaInstallTarGz(ShellCommand):
    """Install given file, unzipping to executablePath"""
    
    def __init__(self, **kwargs):
        self.filename = ""
        self.branch = ""
        if 'branch' in kwargs:
            self.branch = kwargs['branch']
        if 'filename' in kwargs:
            self.filename = kwargs['filename']
        if not 'command' in kwargs:
            kwargs['command'] = ["tar", "-zvxf"]
        ShellCommand.__init__(self, **kwargs)
    
    def describe(self, done=False):
        return ["Install tar.gz"]
    
    def start(self):
        if not self.filename:
            if self.branch:
                self.filename = self.getProperty("filename")
            else:
                return FAILURE
        if self.filename:
            self.command = self.command[:] + [self.filename]
        ShellCommand.start(self)
    
    def evaluateCommand(self, cmd):
        superResult = ShellCommand.evaluateCommand(self, cmd)
        if SUCCESS != superResult:
            return FAILURE
        return SUCCESS

class MozillaInstallDmg(ShellCommand):
    """Install given file, copying to workdir"""
    
    def __init__(self, **kwargs):
        self.filename = ""
        self.branch = ""
        if 'branch' in kwargs:
            self.branch = kwargs['branch']
        if 'filename' in kwargs:
            self.filename = kwargs['filename']
        if not 'command' in kwargs:
            kwargs['command'] = ["bash", "installdmg.sh", "$FILENAME"]
        ShellCommand.__init__(self, **kwargs)
    
    def describe(self, done=False):
        return ["Install dmg"]
    
    def start(self):
        if not self.filename:
            if self.branch:
                self.filename = self.getProperty("filename")
            else:
                return FAILURE

        self.command = self.command[:]
        for i in range(len(self.command)):
            if self.command[i] == "$FILENAME":
                self.command[i] = self.filename
        ShellCommand.start(self)
    
    def evaluateCommand(self, cmd):
        superResult = ShellCommand.evaluateCommand(self, cmd)
        if SUCCESS != superResult:
            return FAILURE
        return SUCCESS

class MozillaInstallDmgEx(ShellCommand):
    """Install given file, copying to workdir"""
    #This is a temporary class to test the new InstallDmg script without affecting the production mac machines
    # if this works everything should be switched over the using it
    
    def __init__(self, **kwargs):
        self.filename = ""
        self.branch = ""
        if 'branch' in kwargs:
            self.branch = kwargs['branch']
        if 'filename' in kwargs:
            self.filename = kwargs['filename']
        if not 'command' in kwargs:
            kwargs['command'] = ["expect", "installdmg.ex", "$FILENAME"]
        ShellCommand.__init__(self, **kwargs)
    
    def describe(self, done=False):
        return ["Install dmg"]
    
    def start(self):
        if not self.filename:
            if self.branch:
                self.filename = self.getProperty("filename")
            else:
                return FAILURE

        self.command = self.command[:]
        for i in range(len(self.command)):
            if self.command[i] == "$FILENAME":
                self.command[i] = self.filename
        ShellCommand.start(self)
    
    def evaluateCommand(self, cmd):
        superResult = ShellCommand.evaluateCommand(self, cmd)
        if SUCCESS != superResult:
            return FAILURE
        return SUCCESS

class TalosFactory(BuildFactory):
    """Create working talos build factory"""

    winClean   = ["touch temp.zip &", "rm", "-rf", "*.zip", "talos/", "firefox/"]
    macClean   = "rm -vrf *"
    linuxClean = "rm -vrf *"

    def __init__(self, OS, envName, buildBranch, configOptions, buildPath, talosCmd, customManifest='', cvsRoot=":pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot"):
        BuildFactory.__init__(self)
        if OS in ('linux', 'linuxbranch',):
            cleanCmd = self.linuxClean
        elif OS in ('win',):
            cleanCmd = self.winClean
        else:
            cleanCmd = self.macClean
        self.addStep(ShellCommand(
                           workdir=".",
                           description="Cleanup",
                           command=cleanCmd,
                           env=MozillaEnvironments[envName]))
        self.addStep(FileDownload(
                           mastersrc="scripts/count_and_reboot.py",
                           slavedest="count_and_reboot.py",
                           workdir="."))
        self.addStep(ShellCommand(
                           command=["cvs", "-d", cvsRoot, "co", "-d", "talos",
                                    "mozilla/testing/performance/talos"],
                           workdir=".",
                           description="checking out talos",
                           haltOnFailure=True,
                           flunkOnFailure=True,
                           env=MozillaEnvironments[envName]))
        self.addStep(FileDownload(
                           mastersrc="scripts/generate-tpcomponent.py",
                           slavedest="generate-tpcomponent.py",
                           workdir="talos/page_load_test"))
        if customManifest != '':
            self.addStep(FileDownload(
                           mastersrc=customManifest,
                           slavedest="manifest.txt",
                           workdir="talos/page_load_test"))
        self.addStep(ShellCommand(
                           command=["python", "generate-tpcomponent.py"],
                           workdir="talos/page_load_test",
                           description="setting up pageloader",
                           haltOnFailure=True,
                           flunkOnFailure=True,
                           env=MozillaEnvironments[envName]))
        self.addStep(MozillaWget(
                           workdir=".",
                           branch=buildBranch,
                           env=MozillaEnvironments[envName]))
        #install the browser, differs based upon platform
        if OS == 'linux':
            self.addStep(MozillaInstallTarBz2(
                               workdir=".",
                               branch=buildBranch,
                               haltOnFailure=True,
                               env=MozillaEnvironments[envName]))
        elif OS == 'linuxbranch': #special case for old linux builds
            self.addStep(MozillaInstallTarGz(
                           workdir=".",
                           branch=buildBranch,
                           haltOnFailure=True,
                           env=MozillaEnvironments[envName]))
        elif OS == 'win':
            self.addStep(MozillaInstallZip(
                               workdir=".",
                               branch=buildBranch,
                               haltOnFailure=True,
                               env=MozillaEnvironments[envName]))
            self.addStep(ShellCommand(
                               workdir="firefox/",
                               flunkOnFailure=False,
                               warnOnFailure=False,
                               description="chmod files (see msys bug)",
                               command=["chmod", "-v", "-R", "a+x", "."],
                               env=MozillaEnvironments[envName]))
        elif OS == 'tiger':
            self.addStep(FileDownload(
                           mastersrc="scripts/installdmg.sh",
                           slavedest="installdmg.sh",
                           workdir="."))
            self.addStep(MozillaInstallDmg(
                               workdir=".",
                               branch=buildBranch,
                               haltOnFailure=True,
                               env=MozillaEnvironments[envName]))
        else: #leopard
            self.addStep(FileDownload(
                           mastersrc="scripts/installdmg.ex",
                           slavedest="installdmg.ex",
                           workdir="."))
            self.addStep(MozillaInstallDmgEx(
                               workdir=".",
                               branch=buildBranch,
                               haltOnFailure=True,
                               env=MozillaEnvironments[envName]))
        self.addStep(MozillaUpdateConfig(
                           workdir="talos/",
                           branch=buildBranch,
                           haltOnFailure=True,
                           executablePath=buildPath,
                           addOptions=configOptions,
                           env=MozillaEnvironments[envName]))
        self.addStep(MozillaRunPerfTests(
                           warnOnWarnings=True,
                           workdir="talos/",
                           branch=buildBranch,
                           timeout=21600,
                           haltOnFailure=False,
                           command=talosCmd,
                           env=MozillaEnvironments[envName]))
        self.addStep(ShellCommand(
                           flunkOnFailure=False,
                           warnOnFailure=False,
                           alwaysRun=True,
                           workdir='.',
                           description="reboot after 1 test run",
                           command=["python", "count_and_reboot.py", "-f", "../talos_count.txt", "-n", "1", "-z"],
                           env=MozillaEnvironments[envName]))


def main(argv=None):
    if argv is None:
        argv = sys.argv
    #BUG - need to come up with a new test case now that we aren't doing url scraping in the perfrunner code
    #tester = LatestFileURL('http://stage.mozilla.org/pub/mozilla.org/firefox/tinderbox-builds/fx-win32-tbox-trunk/', "en-US.win32.zip")
   # tester.testrun()
    return 0

if __name__ == '__main__':
    main()
