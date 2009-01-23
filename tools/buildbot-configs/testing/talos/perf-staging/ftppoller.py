import time
import re

from urllib2 import urlopen, unquote
from twisted.python import log, failure
from twisted.internet import defer, reactor
from twisted.internet.task import LoopingCall

from buildbot.changes import base, changes

class InvalidResultError(Exception):
    def __init__(self, value="InvalidResultError"):
        self.value = value
    def __str__(self):
        return repr(self.value)

class EmptyResult(Exception):
    pass

class FtpParser:
    """I parse the web page for possible builds to test"""
    findBuildDirs = re.compile('^.*"(\d{10})\/".*$')
    
    def __init__(self, query, searchString):
        url = query.geturl()
        pageContents = query.read()
        query.close()
        self.dirs = []
        self.dates = []
        lines = pageContents.split('\n')
        #for parsing lists of directories
        for line in lines:
            if line == "": continue
            match = re.match(self.findBuildDirs, line)
            if match:
              self.dirs.append(match.group(1))
        #for parsing lists of files
        findLastDate = re.compile('^.*"([^"]*' + searchString + ')".*(\d\d-[a-zA-Z]{3}-\d\d\d\d \d\d:\d\d).*$')
        for line in lines:
            match = re.match(findLastDate, line)
            if match:
              self.dates.append([match.group(1), url + match.group(1), time.mktime(time.strptime(match.group(2), "%d-%b-%Y %H:%M"))])
              
    def getDirs(self):
        return self.dirs

    def getDates(self):
        return self.dates
    

class FtpPoller(base.ChangeSource):
    """This source will poll an ftp directory for changes and submit
    them to the change master."""
    
    compare_attrs = ["ftpURLs", "pollInterval", "tree", "branch"]
    
    parent = None # filled in when we're added
    loop = None
    volatile = ['loop']
    working = 0
    
    def __init__(self, branch="", tree="Firefox", pollInterval=30, ftpURLs=[], searchString=""):
        """
        @type   ftpURLs:            list of strings
        @param  ftpURLs:            The ftp directories to monitor

        @type   tree:               string
        @param  tree:               The tree to look for changes in. 
                                    For example, Firefox trunk is 'Firefox'
        @type   branch:             string
        @param  branch:             The branch to look for changes in. This must
                                    match the 'branch' option for the Scheduler.
        @type   pollInterval:       int
        @param  pollInterval:       The time (in seconds) between queries for 
                                    changes
        @type   searchString:       string
        @param  searchString:       file type of the build we are looking for
        """
        
        self.ftpURLs = ftpURLs
        self.tree = tree
        self.branch = branch
        self.pollInterval = pollInterval
        self.lastChanges = {}
        for url in self.ftpURLs:
          self.lastChanges[url] = time.time() - 3600
        self.searchString = searchString
    
    def startService(self):
        self.loop = LoopingCall(self.poll)
        base.ChangeSource.startService(self)
        
        reactor.callLater(0, self.loop.start, self.pollInterval)
    
    def stopService(self):
        self.loop.stop()
        return base.ChangeSource.stopService(self)
    
    def describe(self):
        str = ""
        str += "Getting changes from ftp directory %s " \
                % str(self.ftpURLs)
        str += "<br>Using tree: %s, branch %s" % (self.tree, self.branch)
        return str
    
    def poll(self):
        if self.working > 0:
            log.msg("Not polling Tinderbox because last poll is still working (%s)" % (str(self.working)))
        else:
            for url in self.ftpURLs:
              self.working = self.working + 1
              d = self._get_changes(url)
              d.addCallback(self._process_changes, 0)
              d.addBoth(self._finished)
        return

    def _finished(self, res):
        self.working = self.working - 1

    def _get_changes(self, url):
        log.msg("Polling ftp dir %s" % url)
        return defer.maybeDeferred(urlopen, url)
    
    def _process_changes(self, query, forceDate):
    
        try:
            url = query.geturl()
            parser = FtpParser(query, self.searchString)
            dirList = parser.getDirs()
            dateList = parser.getDates()
        except InvalidResultError, e:
            log.msg("Could not process Tinderbox query: " + e.value)
            return
        except EmptyResult:
            return


        #figure out if there is a new directory that needs searching
        for dir in dirList:
            buildDate = int(dir)
            if self.lastChanges[url] >= buildDate:
                # change too old
                continue
            self.lastChanges[url] = buildDate
            self.working = self.working + 1
            d = self._get_changes(url + dir + '/')
            d.addBoth(self._process_changes, buildDate)
            d.addBoth(self._finished)

        #if we have a new browser to test, test it
        for buildname, fullpath, buildDate in dateList:
            if (url in self.lastChanges):
                if (self.lastChanges[url] >= buildDate):
                    # change too old
                     continue
            if forceDate > 0:
                buildDate = forceDate
            else:
                self.lastChanges[url] = buildDate
            c = changes.Change(who = url,
                               comments = "success",
                               files = [buildname,],
                               branch = self.branch,
                               when = buildDate,
                               links = fullpath)
            self.parent.addChange(c)
            log.msg("found a browser to test (%s)" % (fullpath))
