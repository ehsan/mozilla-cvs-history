from datetime import datetime
import os.path, re
from time import strftime

from twisted.python import log

from buildbot.process.factory import BuildFactory
from buildbot.steps.shell import Compile, ShellCommand, WithProperties, \
  SetProperty
from buildbot.steps.source import CVS, Mercurial
from buildbot.steps.transfer import FileDownload

import buildbotcustom.steps.misc
import buildbotcustom.steps.release
import buildbotcustom.steps.test
import buildbotcustom.steps.transfer
import buildbotcustom.steps.updates
import buildbotcustom.unittest.steps
import buildbotcustom.env
reload(buildbotcustom.steps.misc)
reload(buildbotcustom.steps.release)
reload(buildbotcustom.steps.test)
reload(buildbotcustom.steps.transfer)
reload(buildbotcustom.steps.updates)
reload(buildbotcustom.unittest.steps)
reload(buildbotcustom.env)

from buildbotcustom.steps.misc import SetMozillaBuildProperties, TinderboxShellCommand
from buildbotcustom.steps.release import UpdateVerify, L10nVerifyMetaDiff
from buildbotcustom.steps.test import AliveTest, CompareBloatLogs, \
  CompareLeakLogs, Codesighs, GraphServerPost
from buildbotcustom.steps.transfer import MozillaStageUpload
from buildbotcustom.steps.updates import CreateCompleteUpdateSnippet
from buildbotcustom.env import MozillaEnvironments

import buildbotcustom.unittest.steps as unittest_steps


class BootstrapFactory(BuildFactory):
    def __init__(self, automation_tag, logdir, bootstrap_config, 
                 cvsroot="pserver:anonymous@cvs-mirror.mozilla.org", 
                 cvsmodule="mozilla"):
        """
    @type  cvsroot: string
    @param cvsroot: The CVSROOT to use for checking out Bootstrap.

    @type  cvsmodule: string
    @param cvsmodule: The CVS module to use for checking out Bootstrap.

    @type  automation_tag: string
    @param automation_tag: The CVS Tag to use for checking out Bootstrap.

    @type  logdir: string
    @param logdir: The log directory for Bootstrap to use. 
                   Note - will be created if it does not already exist.

    @type  bootstrap_config: string
    @param bootstrap_config: The location of the bootstrap.cfg file on the 
                             slave. This will be copied to "bootstrap.cfg"
                             in the builddir on the slave.
        """
        BuildFactory.__init__(self)
        self.addStep(ShellCommand, 
         description='clean checkout',
         workdir='.', 
         command=['rm', '-rf', 'build'],
         haltOnFailure=1)
        self.addStep(ShellCommand, 
         description='checkout', 
         workdir='.',
         command=['cvs', '-d', cvsroot, 'co', '-r', automation_tag,
                  '-d', 'build', cvsmodule],
         haltOnFailure=1,
        )
        self.addStep(ShellCommand, 
         description='copy bootstrap.cfg',
         command=['cp', bootstrap_config, 'bootstrap.cfg'],
         haltOnFailure=1,
        )
        self.addStep(ShellCommand, 
         description='echo bootstrap.cfg',
         command=['cat', 'bootstrap.cfg'],
         haltOnFailure=1,
        )
        self.addStep(ShellCommand, 
         description='(re)create logs area',
         command=['bash', '-c', 'mkdir -p ' + logdir], 
         haltOnFailure=1,
        )

        self.addStep(ShellCommand, 
         description='clean logs area',
         command=['bash', '-c', 'rm -rf ' + logdir + '/*.log'], 
         haltOnFailure=1,
        )
        self.addStep(ShellCommand, 
         description='unit tests',
         command=['make', 'test'], 
         haltOnFailure=1,
        )


class MozillaBuildFactory(BuildFactory):
    ignore_dirs = [
            'info',
            'repo_setup',
            'tag',
            'source',
            'updates',
            'final_verification',
            'l10n_verification',
            'mac_update_verify',
            'mac_build'
            'win32_update_verify',
            'win32_build',
            'linux_update_verify',
            'linux_build',
            ]

    def __init__(self, buildToolsRepo, buildSpace=0, **kwargs):
        BuildFactory.__init__(self, **kwargs)

        self.buildToolsRepo = buildToolsRepo
        self.buildSpace = buildSpace

        self.addPreBuildCleanupSteps()

    def addPreBuildCleanupSteps(self):
        if self.buildSpace > 0:
            self.addStep(ShellCommand,
             command=['bash', '-c',
              'if [ ! -d tools ]; then hg clone %s; fi' % self.buildToolsRepo],
             description=['clone', 'build tools'],
             workdir='.'
            )
            self.addStep(ShellCommand,
             command=['hg', 'pull', '-u'],
             description=['update', 'build tools'],
             workdir='tools',
            )

            command = ['python', 'tools/buildfarm/maintenance/purge_builds.py',
                 '-s', str(self.buildSpace)]

            for i in self.ignore_dirs:
                command.extend(["-n", i])
            command.append("..")

            self.addStep(ShellCommand,
             command=command,
             description=['cleaning', 'old', 'builds'],
             descriptionDone=['clean', 'old', 'builds'],
             warnOnFailure=True,
             flunkOnFailure=False,
             workdir='.',
             timeout=3600, # One hour, because Windows is slow
            )

class MercurialBuildFactory(MozillaBuildFactory):
    def __init__(self, env, objdir, platform, branch, sourceRepo, buildToolsRepo,
                 configRepo, configSubDir, profiledBuild, mozconfig,
                 buildRevision=None, stageServer=None, stageUsername=None,
                 stageGroup=None, stageSshKey=None, stageBasePath=None,
                 ausBaseUploadDir=None, updatePlatform=None,
                 downloadBaseURL=None, ausUser=None, ausHost=None,
                 nightly=False, leakTest=False, codesighs=True,
                 graphServer=None, graphSelector=None, graphBranch=None,
                 baseName=None, uploadPackages=True, uploadSymbols=True,
                 dependToDated=True, createSnippet=False, doCleanup=True,
                 buildSpace=0, **kwargs):
        MozillaBuildFactory.__init__(self, buildToolsRepo, buildSpace, **kwargs)
        self.env = env
        self.objdir = objdir
        self.platform = platform
        self.branch = branch
        self.sourceRepo = sourceRepo
        self.configRepo = configRepo
        self.configSubDir = configSubDir
        self.profiledBuild = profiledBuild
        self.buildRevision = buildRevision
        self.stageServer = stageServer
        self.stageUsername = stageUsername
        self.stageGroup = stageGroup
        self.stageSshKey = stageSshKey
        self.stageBasePath = stageBasePath
        self.ausBaseUploadDir = ausBaseUploadDir
        self.updatePlatform = updatePlatform
        self.downloadBaseURL = downloadBaseURL
        self.ausUser = ausUser
        self.ausHost = ausHost
        self.nightly = nightly
        self.leakTest = leakTest
        self.codesighs = codesighs
        self.graphServer = graphServer
        self.graphSelector = graphSelector
        self.graphBranch = graphBranch
        self.baseName = baseName
        self.uploadPackages = uploadPackages
        self.uploadSymbols = uploadSymbols
        self.dependToDated = dependToDated
        self.createSnippet = createSnippet
        self.doCleanup = doCleanup

        # short name can be inferred from the full branch name
        self.shortName = self.branch.split('/')[-1]

        if self.uploadPackages:
            assert stageServer and stageUsername and stageSshKey
            assert stageBasePath
        if self.createSnippet:
            assert ausBaseUploadDir and updatePlatform and downloadBaseURL
            assert ausUser and ausHost

            # this is a tad ugly because we need to python interpolation
            # as well as WithProperties
            # here's an example of what it translates to:
            # /opt/aus2/build/0/Firefox/mozilla2/WINNT_x86-msvc/2008010103/en-US
            self.ausFullUploadDir = '%s/%s/%%(buildid)s/en-US' % \
              (self.ausBaseUploadDir, self.updatePlatform)

        self.mozconfig = 'configs/%s/%s/mozconfig' % (self.configSubDir,
                                                      mozconfig)

        # we don't need the extra cruft in 'platform' anymore
        self.platform = platform.split('-')[0].replace('64', '')
        assert self.platform in ('linux', 'win32', 'macosx')

        self.logUploadDir = 'tinderbox-builds/%s-%s/' % (self.shortName,
                                                         self.platform)
        # now, generate the steps
        #  regular dep builds (no clobber, no leaktest):
        #   addBuildSteps()
        #   addUploadSteps()
        #   addCodesighsSteps()
        #  leak test builds (no clobber, leaktest):
        #   addBuildSteps()
        #   addLeakTestSteps()
        #  nightly builds (clobber)
        #   addBuildSteps()
        #   addSymbolSteps()
        #   addUploadSteps()
        #   addUpdateSteps()
        #  for all dep and nightly builds (but not release builds):
        #   addCleanupSteps()
        self.addBuildSteps()
        if self.leakTest:
            self.addLeakTestSteps()
        if self.codesighs:
            self.addCodesighsSteps()
        if self.uploadSymbols:
            self.addSymbolsSteps()
        if self.uploadPackages:
            self.addUploadSteps()
        if self.createSnippet:
            self.addUpdateSteps()
        if self.doCleanup:
            self.addCleanupSteps()

    def addBuildSteps(self):
        if self.nightly:
            self.addStep(ShellCommand,
             command=['rm', '-rf', 'build'],
             env=self.env,
             workdir='.',
             timeout=60*60 # 1 hour
            )
        self.addStep(ShellCommand,
         command=['echo', WithProperties('Building on: %(slavename)s')],
         env=self.env
        )
        self.addStep(ShellCommand,
         command="rm -rf %s/dist/firefox-* %s/dist/install/sea/*.exe " %
                  (self.objdir, self.objdir),
         env=self.env,
         description=['deleting', 'old', 'package'],
         descriptionDone=['delete', 'old', 'package']
        )
        if self.nightly:
            self.addStep(ShellCommand,
             command="find 20* -maxdepth 2 -mtime +7 -exec rm -rf {} \;",
             env=self.env,
             workdir='.',
             description=['cleanup', 'old', 'symbols'],
             flunkOnFailure=False
            )
        self.addStep(Mercurial,
         mode='update',
         baseURL=self.sourceRepo,
         defaultBranch=self.branch,
         timeout=60*60, # 1 hour
        )
        if self.buildRevision:
            self.addStep(ShellCommand,
             command=['hg', 'up', '-C', '-r', self.buildRevision],
             haltOnFailure=True
            )
            self.addStep(SetProperty,
             command=['hg', 'identify', '-i'],
             property='got_revision'
            )
        changesetLink = '<a href=%s/%s/index.cgi/rev' % (self.sourceRepo,
                                                         self.branch)
        changesetLink += '/%(got_revision)s title="Built from revision %(got_revision)s">rev:%(got_revision)s</a>'
        self.addStep(ShellCommand,
         command=['echo', 'TinderboxPrint:', WithProperties(changesetLink)]
        )
        self.addStep(ShellCommand,
         command=['rm', '-rf', 'configs'],
         description=['removing', 'configs'],
         descriptionDone=['remove', 'configs'],
         haltOnFailure=True
        )
        self.addStep(ShellCommand,
         command=['hg', 'clone', self.configRepo, 'configs'],
         description=['checking', 'out', 'configs'],
         descriptionDone=['checkout', 'configs'],
         haltOnFailure=True
        )
        self.addStep(ShellCommand,
         # cp configs/mozilla2/$platform/$repo/$type/mozconfig .mozconfig
         command=['cp', self.mozconfig, '.mozconfig'],
         description=['copying', 'mozconfig'],
         descriptionDone=['copy', 'mozconfig'],
         haltOnFailure=True
        )
        self.addStep(ShellCommand,
         command=['cat', '.mozconfig'],
        )

        buildcmd = 'build'
        if self.profiledBuild:
            buildcmd = 'profiledbuild'
        self.addStep(Compile,
         command=['make', '-f', 'client.mk', buildcmd],
         env=self.env,
         haltOnFailure=True,
         timeout=5400 # 90 minutes, because windows PGO builds take a long time
        )

    def addLeakTestSteps(self):
        # we want the same thing run a few times here, with different
        # extraArgs
        for args in [['-register'], ['-CreateProfile', 'default'],
                     ['-P', 'default']]:
            self.addStep(AliveTest,
                env=self.env,
                workdir='build/%s/_leaktest' % self.objdir,
                extraArgs=args
            )
        # we only want this variable for this test - this sucks
        bloatEnv = self.env.copy()
        bloatEnv['XPCOM_MEM_BLOAT_LOG'] = '1' 
        self.addStep(AliveTest,
         env=bloatEnv,
         workdir='build/%s/_leaktest' % self.objdir,
         logfile='bloat.log',
        )
        self.addStep(ShellCommand,
         env=self.env,
         workdir='.',
         command=['wget', '-O', 'bloat.log.old',
                  'http://%s/pub/mozilla.org/firefox/%s/bloat.log' % \
                    (self.stageServer, self.logUploadDir)]
        )
        self.addStep(ShellCommand,
         env=self.env,
         command=['mv', '%s/_leaktest/bloat.log' % self.objdir,
                  '../bloat.log'],
        )
        self.addStep(ShellCommand,
         env=self.env,
         command=['scp', '-o', 'User=%s' % self.stageUsername,
                  '-o', 'IdentityFile=~/.ssh/%s' % self.stageSshKey,
                  '../bloat.log',
                  '%s:%s/%s' % (self.stageServer, self.stageBasePath,
                                self.logUploadDir)]
        )
        self.addStep(CompareBloatLogs,
         bloatLog='../bloat.log',
         env=self.env,
        )
        self.addStep(GraphServerPost,
         server=self.graphServer,
         selector=self.graphSelector,
         branch=self.graphBranch,
         resultsname=self.baseName
        )
        self.addStep(AliveTest,
         env=self.env,
         workdir='build/%s/_leaktest' % self.objdir,
         extraArgs=['--trace-malloc', 'malloc.log',
                    '--shutdown-leaks=sdleak.log'],
         timeout=3600 # 1 hour, because this takes a long time on win32
        )
        self.addStep(ShellCommand,
         env=self.env,
         workdir='.',
         command=['wget', '-O', 'malloc.log.old',
                  'http://%s/pub/mozilla.org/firefox/%s/malloc.log' % \
                    (self.stageServer, self.logUploadDir)]
        )
        self.addStep(ShellCommand,
         env=self.env,
         workdir='.',
         command=['wget', '-O', 'sdleak.tree.old',
                  'http://%s/pub/mozilla.org/firefox/%s/sdleak.tree' % \
                    (self.stageServer, self.logUploadDir)]
        )
        self.addStep(ShellCommand,
         env=self.env,
         command=['mv',
                  '%s/_leaktest/malloc.log' % self.objdir,
                  '../malloc.log'],
        )
        self.addStep(ShellCommand,
         env=self.env,
         command=['mv',
                  '%s/_leaktest/sdleak.log' % self.objdir,
                  '../sdleak.log'],
        )
        self.addStep(CompareLeakLogs,
         mallocLog='../malloc.log',
         platform=self.platform,
         env=self.env,
         testname='current'
        )
        self.addStep(GraphServerPost,
         server=self.graphServer,
         selector=self.graphSelector,
         branch=self.graphBranch,
         resultsname=self.baseName
        )
        self.addStep(CompareLeakLogs,
         mallocLog='../malloc.log.old',
         platform=self.platform,
         env=self.env,
         testname='previous'
        )
        self.addStep(ShellCommand,
         env=self.env,
         workdir='.',
         command=['bash', '-c',
                  'perl build/tools/trace-malloc/diffbloatdump.pl '
                  '--depth=15 --use-address /dev/null sdleak.log '
                  '> sdleak.tree']
        )
        if self.platform in ('macosx', 'linux'):
            self.addStep(ShellCommand,
             env=self.env,
             workdir='.',
             command=['mv', 'sdleak.tree', 'sdleak.tree.raw']
            )
            self.addStep(ShellCommand,
             env=self.env,
             workdir='.',
             command=['/bin/bash', '-c', 
                      'perl '
                      'build/tools/rb/fix-%s-stack.pl '
                      'sdleak.tree.raw '
                      '> sdleak.tree' % self.platform]
            )
        self.addStep(ShellCommand,
         env=self.env,
         command=['scp', '-o', 'User=%s' % self.stageUsername,
                  '-o', 'IdentityFile=~/.ssh/%s' % self.stageSshKey,
                  '../malloc.log', '../sdleak.tree',
                  '%s:%s/%s' % (self.stageServer, self.stageBasePath,
                                self.logUploadDir)]
        )
        self.addStep(ShellCommand,
         env=self.env,
         command=['perl', 'tools/trace-malloc/diffbloatdump.pl',
                  '--depth=15', '../sdleak.tree.old', '../sdleak.tree']
        )

    def addUploadSteps(self):
        self.addStep(ShellCommand,
         command=['make', 'package'],
         workdir='build/%s' % self.objdir,
         haltOnFailure=True
        )
        if self.platform.startswith("win32"):
         self.addStep(ShellCommand,
             command=['make', 'installer'],
             workdir='build/%s' % self.objdir,
             haltOnFailure=True
         )
        if self.createSnippet:
         self.addStep(ShellCommand,
             command=['make', '-C',
                      '%s/tools/update-packaging' % self.objdir],
             haltOnFailure=True
         )
        self.addStep(SetMozillaBuildProperties,
         objdir='build/%s' % self.objdir
        )
        releaseToLatest = False
        releaseToDated = False
        if self.nightly:
            releaseToLatest=True
            releaseToDated=True

        self.addStep(MozillaStageUpload,
         objdir=self.objdir,
         username=self.stageUsername,
         milestone=self.shortName,
         remoteHost=self.stageServer,
         remoteBasePath=self.stageBasePath,
         platform=self.platform,
         group=self.stageGroup,
         sshKey=self.stageSshKey,
         uploadCompleteMar=self.createSnippet,
         releaseToLatest=releaseToLatest,
         releaseToDated=releaseToDated,
         releaseToTinderboxBuilds=True,
         tinderboxBuildsDir='%s-%s' % (self.shortName, self.platform),
         dependToDated=self.dependToDated
        )
        
    def addCodesighsSteps(self):
        self.addStep(ShellCommand,
         command=['make'],
         workdir='build/%s/tools/codesighs' % self.objdir
        )
        self.addStep(ShellCommand,
         command=['wget', '-O', 'codesize-auto-old.log',
          'http://%s/pub/mozilla.org/firefox/%s/codesize-auto.log' % \
           (self.stageServer, self.logUploadDir)],
         workdir='.',
         env=self.env
        )
        self.addStep(Codesighs,
         objdir=self.objdir,
         platform=self.platform,
         env=self.env
        )
        self.addStep(GraphServerPost,
         server=self.graphServer,
         selector=self.graphSelector,
         branch=self.graphBranch,
         resultsname=self.baseName
        )
        self.addStep(ShellCommand,
         command=['cat', '../codesize-auto-diff.log']
        )
        self.addStep(ShellCommand,
         command=['scp', '-o', 'User=%s' % self.stageUsername,
          '-o', 'IdentityFile=~/.ssh/%s' % self.stageSshKey,
          '../codesize-auto.log',
          '%s:%s/%s' % (self.stageServer, self.stageBasePath,
                        self.logUploadDir)]
        )
        self.addStep(ShellCommand,
         command=['wget', '-O', 'codesize-base-old.log',
          'http://%s/pub/mozilla.org/firefox/%s/codesize-base.log' %\
           (self.stageServer, self.logUploadDir)],
         workdir='.',
         env=self.env
        )
        self.addStep(Codesighs,
         objdir=self.objdir,
         platform=self.platform,
         type='base',
         env=self.env
        )
        self.addStep(GraphServerPost,
         server=self.graphServer,
         selector=self.graphSelector,
         branch=self.graphBranch,
         resultsname=self.baseName
        )
        self.addStep(ShellCommand,
         command=['cat', '../codesize-base-diff.log']
        )
        self.addStep(ShellCommand,
         command=['scp', '-o', 'User=%s' % self.stageUsername,
          '-o', 'IdentityFile=~/.ssh/%s' % self.stageSshKey,
          '../codesize-base.log',
          '%s:%s/%s' % (self.stageServer, self.stageBasePath,
                        self.logUploadDir)]
        )

    def addUpdateSteps(self):
        self.addStep(CreateCompleteUpdateSnippet,
         objdir='build/%s' % self.objdir,
         milestone=self.shortName,
         baseurl='%s/nightly' % self.downloadBaseURL
        )
        self.addStep(ShellCommand,
         command=['ssh', '-l', self.ausUser, self.ausHost,
                  WithProperties('mkdir -p %s' % self.ausFullUploadDir)],
         description=['create', 'aus', 'upload', 'dir'],
         haltOnFailure=True
        )
        self.addStep(ShellCommand,
         command=['scp', '-o', 'User=%s' % self.ausUser,
                  'dist/update/complete.update.snippet',
                  WithProperties('%s:%s/complete.txt' % \
                    (self.ausHost, self.ausFullUploadDir))],
         workdir='build/%s' % self.objdir,
         description=['upload', 'complete', 'snippet'],
         haltOnFailure=True
        )

    def addSymbolsSteps(self):
        self.addStep(ShellCommand,
         command=['make', 'buildsymbols'],
         env=self.env,
         workdir='build/%s' % self.objdir,
         haltOnFailure=True
        )
        self.addStep(ShellCommand,
         command=['make', 'uploadsymbols'],
         env=self.env,
         workdir='build/%s' % self.objdir,
         haltOnFailure=True
        )

    def addCleanupSteps(self):
        if self.nightly:
            self.addStep(ShellCommand,
             command=['rm', '-rf', 'build'],
             env=self.env,
             workdir='.',
             timeout=60*60 # 1 hour
            )
            # no need to clean-up temp files if we clobber the whole directory
            return

        # OS X builds eat up a ton of space with -save-temps enabled
        # until we have dwarf support we need to clean this up so we don't
        # fill up the disk
        if self.platform.startswith("macosx"):
            # For regular OS X builds the "objdir" passed in is objdir/ppc
            # For leak test builds the "objdir" passed in is objdir.
            # To properly cleanup we need to make sure we're in 'objdir',
            # otherwise we miss the entire i386 dir in the normal case
            # We can't just run this in the srcdir because there are other files
            # most notably hg metadata which have the same extensions
            baseObjdir = self.objdir.split('/')[0]
            self.addStep(ShellCommand,
             command=['find', '-d', '-E', '.', '-iregex',
                      '.*\.(mi|i|s|mii|ii)$',
                      '-exec', 'rm', '-rf', '{}', ';'],
             workdir='build/%s' % baseObjdir
            )



class RepackFactory(MozillaBuildFactory):
    # Override ignore_dirs so that we don't delete l10n nightly builds
    # before running a l10n nightly build
    ignore_dirs = MozillaBuildFactory.ignore_dirs + [
            'mozilla-central-macosx-l10n-nightly',
            'mozilla-central-linux-l10n-nightly',
            'mozilla-central-win32-l10n-nightly',
            'mozilla-1.9.1-macosx-l10n-nightly',
            'mozilla-1.9.1-linux-l10n-nightly',
            'mozilla-1.9.1-win32-l10n-nightly',
    ]

    def __init__(self, branch, project, enUSBinaryURL, stageServer,
                 stageUsername, uploadPath, repoPath, l10nRepoPath,
                 buildToolsRepo, buildSpace):
        MozillaBuildFactory.__init__(self, buildToolsRepo, buildSpace)

        self.addStep(ShellCommand,
         command=['sh', '-c',
                  'if [ -d '+branch+'/dist/upload ]; then ' +
                  'rm -rf '+branch+'/dist/upload; ' +
                  'fi'],
         workdir='build',
         haltOnFailure=True
        )

        self.addStep(ShellCommand,
         command=['sh', '-c', 'mkdir -p %s' % l10nRepoPath],
         descriptionDone='mkdir '+ l10nRepoPath,
         workdir='build'
        )
        self.addStep(ShellCommand,
         command=['sh', '-c', 'if [ -d '+branch+' ]; then ' +
                  'hg -R '+branch+' pull -r tip ; ' +
                  'else ' +
                  'hg clone http://hg.mozilla.org/'+repoPath+'/ ; ' +
                  'fi '
                  '&& hg -R '+branch+' update'],
         descriptionDone=branch+"'s source",
         workdir='build',
         haltOnFailure=True
        )
        self.addStep(ShellCommand,
         command=['sh', '-c',
          WithProperties('if [ -d %(locale)s ]; then ' +
                         'hg -R %(locale)s pull -r tip ; ' +
                         'else ' +
                         'hg clone ' +
                         'http://hg.mozilla.org/'+l10nRepoPath+'/%(locale)s/ ; ' +
                         'fi ' +
                         '&& hg -R %(locale)s update')],
         descriptionDone="locale's source",
         workdir='build/l10n-central'
        )
        self.addStep(ShellCommand,
         command=['make', '-f', 'client.mk', 'configure'],
         env={'CONFIGURE_ARGS': '--enable-application=browser'},
         haltOnFailure=True,
         descriptionDone='autoconf',
         workdir='build/'+branch
        )
        self.addStep(Compile,
         command=['sh', '--',
                  './configure', '--enable-application=browser',
                  '--disable-compile-environment',
                  '--with-l10n-base=../l10n-central'],
         description='configure',
         descriptionDone='configure done',
         haltOnFailure=True,
         workdir='build/'+branch
        )
        self.addStep(ShellCommand,
         command=['make', 'wget-en-US'],
         descriptionDone='wget en-US',
         env={'EN_US_BINARY_URL': enUSBinaryURL},
         haltOnFailure=True,
         workdir='build/'+branch+'/browser/locales'
        )
        self.addStep(ShellCommand,
         command=['make', WithProperties('installers-%(locale)s')],
         env={'PKG_DMG_SOURCE': project},
         haltOnFailure=True,
         workdir='build/'+branch+'/browser/locales'
        )
        self.addStep(ShellCommand,
         command=['make', WithProperties('prepare-upload-latest-%(locale)s')],
         haltOnFailure=True,
         workdir='build/'+branch+'/browser/locales'
        )
        self.addStep(ShellCommand,
         name='upload locale',
         command=['sh', '-c',
                  'scp -i ~/.ssh/ffxbld_dsa -r * '+stageUsername+'@'+\
                   stageServer+":"+uploadPath],
         description='uploading packages',
         descriptionDone='uploaded packages',
         workdir='build/'+branch+'/dist/upload/latest'
        )



class ReleaseFactory(BuildFactory):
    def getPushRepo(self, repo):
        for proto in ('https', 'http'):
             if repo.startswith(proto):
                 return repo.replace(proto, 'ssh')
        return repo

    def getRepoName(self, repo):
        return repo.rstrip('/').split('/')[-1]

    def getRepoPath(self, repo):
        repo = repo.rstrip('/')
        repo = repo.split('/')[-1]
        if repo.find('central') == -1:
            repo = 'l10n-central/%s' % repo
        return repo

    def getCandidatesDir(self, product, version, buildNumber):
        return '/home/ftp/pub/' + product + '/nightly/' + str(version) + \
               '-candidates/build' + str(buildNumber) + '/'

    def getShippedLocales(self, mozillaCentral, baseTag, appName):
        return '%s/raw-file/%s_RELEASE/%s/locales/shipped-locales' % \
                 (mozillaCentral, baseTag, appName)

    def getSshKeyOption(self, hgSshKey):
        if hgSshKey:
            return '-i %s' % hgSshKey
        return hgSshKey

    def makeLongVersion(self, version):
        version = re.sub('a([0-9]+)$', ' Alpha \\1', version)
        version = re.sub('b([0-9]+)$', ' Beta \\1', version)
        version = re.sub('rc([0-9]+)$', ' RC \\1', version)
        return version



class StagingRepositorySetupFactory(ReleaseFactory):
    def __init__(self, hgHost, username, sshKey, repositories):
        ReleaseFactory.__init__(self)
        for repo in sorted(repositories.keys()):
            repoName = self.getRepoName(repo)
            pushRepo = self.getPushRepo(repo)
            sourceRepoPath = self.getRepoPath(repo)

            # test for existence
            command = 'wget -O- %s >/dev/null' % repo
            command += ' && '
            # if it exists, delete it
            command += 'ssh -l %s -i %s %s edit %s delete YES' % \
              (username, sshKey, hgHost, repoName)
            command += '; '
            # either way, try to create it again
            # this kindof sucks, but if we '&&' we can't create repositories
            # that don't already exist, which is a huge pain when adding new
            # locales or repositories.
            command += 'ssh -l %s -i %s %s clone %s %s' % \
              (username, sshKey, hgHost, repoName, sourceRepoPath)

            self.addStep(ShellCommand,
             command=['bash', '-c', command],
             description=['recreate', repoName],
             timeout=30*60 # 30 minutes
            )



class ReleaseTaggingFactory(ReleaseFactory):
    def __init__(self, repositories, buildToolsRepo, productName, appName,
                 appVersion, milestone, baseTag, buildNumber, hgUsername,
                 hgSshKey=None):
        """Repositories looks like this:
            repositories[name]['revision']: changeset# or tag
            repositories[name]['relbranchOverride']: branch name
            repositories[name]['bumpFiles']: [filesToBump]
           eg:
            repositories['http://hg.mozilla.org/mozilla-central']['revision']:
              d6a0a4fca081
            repositories['http://hg.mozilla.org/mozilla-central']['relbranchOverride']:
              GECKO191_20080828_RELBRANCH
            repositories['http://hg.mozilla.org/mozilla-central']['bumpFiles']:
              ['client.mk', 'browser/config/version.txt',
               'browser/app/module.ver', 'config/milestone.txt']
            relbranchOverride is typically used in two situations:
             1) During a respin (buildNumber > 1) when the "release" branch has
                already been created (during build1). In cases like this all
                repositories should have the relbranch specified
             2) During non-Firefox builds. Because Seamonkey, Thunderbird, etc.
                are releases off of the same platform code as Firefox, the
                "release branch" will already exist in mozilla-central but not
                comm-central, mobile-browser, domi, etc. In cases like this,
                mozilla-central and l10n should be specified with a
                relbranchOverride and the other source repositories should NOT
                specify one.
           buildToolsRepo: This is the repository containing the version-bump.pl
                           script. Typically, this is:
                            http://hg.mozilla.org/build/tools
           productName: The name of the actual *product* being shipped.
                        Examples include: firefox, thunderbird, seamonkey.
                        This is only used for the automated check-in message
                        the version bump generates.
           appName: The "application" name (NOT product name). Examples:
                    browser, suite, mailnews. It is used in version bumping
                    code and assumed to be a subdirectory of the source
                    repository being bumped. Eg, for Firefox, appName should be
                    'browser', which is a subdirectory of 'mozilla-central'.
                    For Thunderbird, it would be 'mailnews', a subdirectory
                    of 'comm-central'.
           appVersion: The current version number of the application being
                       built. Eg, 3.0.2 for Firefox, 2.0 for Seamonkey, etc.
                       This is different than the platform version. See below.
           milestone: The current version of *Gecko*. This is generally
                      along the lines of: 1.8.1.14, 1.9.0.2, etc.
           baseTag: The prefix to use for BUILD/RELEASE tags. It will be 
                    post-fixed with _BUILD$buildNumber and _RELEASE. Generally,
                    this is something like: FIREFOX_3_0_2.
           buildNumber: The current build number. If this is the first time
                        attempting a release this is 1. Other times it may be
                        higher. It is used for post-fixing tags and some
                        internal logic.
           hgUsername: The username to use when pushing changes to the remote
                       repository.
           hgSshKey: The full path to the ssh key to use (if necessary) when
                     pushing changes to the remote repository.

        """
        ReleaseFactory.__init__(self)

        # extremely basic validation, to catch really dumb configurations
        assert len(repositories) > 0, \
          'You must provide at least one repository.'
        assert buildToolsRepo, 'You must provide a build tools repository ' \
          '(eg. http://hg.mozilla.org/build/tools).'
        assert productName, 'You must provide a product name (eg. firefox).'
        assert appName, 'You must provide an application name (eg. browser).'
        assert appVersion, \
          'You must provide an application version (eg. 3.0.2).'
        assert milestone, 'You must provide a milestone (eg. 1.9.0.2).'
        assert baseTag, 'You must provide a baseTag (eg. FIREFOX_3_0_2).'
        assert buildNumber, 'You must provide a buildNumber.'

        # if we're doing a respin we already have a relbranch created
        if buildNumber > 1:
            for repo in repositories:
                assert repositories[repo]['relbranchOverride'], \
                  'No relbranchOverride specified for ' + repo + \
                  '. You must provide a relbranchOverride when buildNumber > 2'

        # now, down to work
        buildTag = '%s_BUILD%s' % (baseTag, str(buildNumber))
        releaseTag = '%s_RELEASE' % baseTag

        # the only tool we use here is the version bump script. we don't do
        # any bumping when buildNumber > 1, so we don't need them in that case
        if buildNumber == 1:
            self.addStep(ShellCommand,
             command=['hg', 'clone', buildToolsRepo, 'tools'],
             workdir='.',
             description=['clone', 'build tools'],
             haltOnFailure=1
            )

        for repo in sorted(repositories.keys()):
            # we need to handle http(s):// and ssh:// URLs here.
            repoName = self.getRepoName(repo)
            pushRepo = self.getPushRepo(repo)

            sshKeyOption = self.getSshKeyOption(hgSshKey)

            repoRevision = repositories[repo]['revision']
            bumpFiles = repositories[repo]['bumpFiles']

            relbranchName = ''
            relbranchOverride = repositories[repo]['relbranchOverride']
            # generally, this is specified for Firefox respins and non-Firefox
            # builds (where mozilla-central has been bumped and branched
            # but not, eg, comm-central).
            if relbranchOverride:
                relbranchName = relbranchOverride
            else:
                # generate the release branch name, which is based on the
                # version and the current date.
                # looks like: GECKO191_20080728_RELBRANCH
                relbranchName = 'GECKO%s_%s_RELBRANCH' % (
                  milestone.replace('.', ''), datetime.now().strftime('%Y%m%d'))
                

            # For l10n we never bump any files, so this will never get
            # overridden. For source repos, we will do a version bump in build1
            # which we commit, and set this property again, so we tag
            # the right revision. For build2, we don't version bump, and this
            # will not get overridden
            self.addStep(SetProperty,
             command=['echo', repoRevision],
             property='%s-revision' % repoName,
             workdir='.',
             haltOnFailure=True
            )
            # 'hg clone -r' breaks in the respin case because the cloned
            # repository will not have ANY changesets from the release branch
            # and 'hg up -C' will fail
            self.addStep(ShellCommand,
             command=['hg', 'clone', repo, repoName],
             workdir='.',
             description=['clone %s' % repoName],
             haltOnFailure=True,
             timeout=30*60 # 30 minutes
            )
            # for build1 we need to create a branch
            if buildNumber == 1 and not relbranchOverride:
                # remember:
                # 'branch' in Mercurial does not actually create a new branch,
                # it switches the "current" branch to the one of the given name.
                # when buildNumber == 1 this will end up creating a new branch
                # when we commit version bumps and tags.
                # note: we don't actually have to switch to the release branch
                # to create tags, but it seems like a more sensible place to
                # have those commits
                self.addStep(ShellCommand,
                 command=['hg', 'up', '-r',
                          WithProperties('%s', '%s-revision' % repoName)],
                 workdir=repoName,
                 description=['update', repoName],
                 haltOnFailure=True
                )
                self.addStep(ShellCommand,
                 command=['hg', 'branch', relbranchName],
                 workdir=repoName,
                 description=['branch %s' % repoName],
                 haltOnFailure=True
                )
            # if buildNumber > 1 we need to switch to it with 'hg up -C'
            else:
                self.addStep(ShellCommand,
                 command=['hg', 'up', '-C', relbranchName],
                 workdir=repoName,
                 description=['switch to', relbranchName],
                 haltOnFailure=True
                )
            # we don't need to do any version bumping if this is a respin
            if buildNumber == 1 and len(bumpFiles) > 0:
                command = ['perl', 'tools/release/version-bump.pl',
                           '-w', repoName, '-t', releaseTag, '-a', appName,
                           '-v', appVersion, '-m', milestone]
                command.extend(bumpFiles)
                self.addStep(ShellCommand,
                 command=command,
                 workdir='.',
                 description=['bump %s' % repoName],
                 haltOnFailure=True
                )
                self.addStep(ShellCommand,
                 command=['hg', 'diff'],
                 workdir=repoName
                )
                self.addStep(ShellCommand,
                 # mozilla-central and other developer repositories have a
                 # 'CLOSED TREE' hook on them which rejects commits when the
                 # tree is declared closed. It is very common for us to tag
                 # and branch when the tree is in this state. Adding the 
                 # 'CLOSED TREE' string at the end will force the hook to
                 # let us commit regardless of the tree state.
                 command=['hg', 'commit', '-u', hgUsername, '-m',
                          'Automated checkin: version bump remove "pre" ' + \
                          ' from version number for ' + productName + ' ' + \
                          appVersion + ' release on ' + relbranchName + ' ' + \
                          'CLOSED TREE'],
                 workdir=repoName,
                 description=['commit %s' % repoName],
                 haltOnFailure=True
                )
                self.addStep(SetProperty,
                 command=['hg', 'identify', '-i'],
                 property='%s-revision' % repoName,
                 workdir=repoName,
                 haltOnFailure=True
                )
            for tag in (buildTag, releaseTag):
                self.addStep(ShellCommand,
                 command=['hg', 'tag', '-u', hgUsername, '-f', '-r',
                          WithProperties('%s', '%s-revision' % repoName),
                          tag],
                 workdir=repoName,
                 description=['tag %s' % repoName],
                 haltOnFailure=True
                )
            self.addStep(ShellCommand,
             command=['hg', 'out', '-e',
                      'ssh -l %s %s' % (hgUsername, sshKeyOption),
                      pushRepo],
             workdir=repoName,
             description=['hg out', repoName]
            )
            self.addStep(ShellCommand,
             command=['hg', 'push', '-e',
                      'ssh -l %s %s' % (hgUsername, sshKeyOption),
                      '-f', pushRepo],
             workdir=repoName,
             description=['push %s' % repoName],
             haltOnFailure=True
            )



class SingleSourceFactory(ReleaseFactory):
    def __init__(self, repository, productName, appVersion, baseTag,
                 autoconfDirs=['.']):
        ReleaseFactory.__init__(self)
        repoName = self.getRepoName(repository)
        pushRepo = self.getPushRepo(repository)
        releaseTag = '%s_RELEASE' % (baseTag)
        bundleFile = '../%s-%s.bundle' % (productName, appVersion)
        sourceTarball = '%s-%s-source.tar.bz2' % (productName, appVersion)

        self.addStep(ShellCommand,
         command=['hg', 'clone', repository, repoName],
         workdir='.',
         description=['clone %s' % repoName],
         haltOnFailure=True,
         timeout=30*60 # 30 minutes
        )
        # This will get us to the version we're building the release with
        self.addStep(ShellCommand,
         command=['hg', 'up', '-C', '-r', releaseTag],
         workdir=repoName,
         description=['update to', releaseTag],
         haltOnFailure=True
        )
        # ...And this will get us the tags so people can do things like
        # 'hg up -r FIREFOX_3_1b1_RELEASE' with the bundle
        self.addStep(ShellCommand,
         command=['hg', 'up'],
         workdir=repoName,
         description=['update to', 'include tag revs'],
         haltOnFailure=True
        )
        self.addStep(SetProperty,
         command=['hg', 'identify', '-i'],
         property='revision',
         workdir=repoName,
         haltOnFailure=True
        )
        self.addStep(ShellCommand,
         command=['hg', 'bundle', '--base', 'null',
                  '-r', WithProperties('%(revision)s'),
                  bundleFile],
         workdir=repoName,
         description=['create bundle'],
         haltOnFailure=True
        )
        self.addStep(ShellCommand,
         command=['rm', '-rf', '.hg'],
         workdir=repoName,
         description=['delete metadata'],
         haltOnFailure=True
        )
        for dir in autoconfDirs:
            self.addStep(ShellCommand,
             command=['autoconf-2.13'],
             workdir='%s/%s' % (repoName, dir),
             haltOnFailure=True
            )
        self.addStep(ShellCommand,
         command=['tar', '-cjf', sourceTarball, repoName],
         workdir='.',
         description=['create tarball'],
         haltOnFailure=True
        )
        # TODO: upload files



class ReleaseUpdatesFactory(ReleaseFactory):
    def __init__(self, cvsroot, patcherToolsTag, mozillaCentral, buildTools,
                 patcherConfig, baseTag, appName, productName, appVersion,
                 oldVersion, buildNumber, ftpServer, bouncerServer,
                 stagingServer, useBetaChannel, stageUsername, stageSshKey,
                 ausUser, ausHost, commitPatcherConfig=True):
        """cvsroot: The CVSROOT to use when pulling patcher, patcher-configs,
                    Bootstrap/Util.pm, and MozBuild. It is also used when
                    commiting the version-bumped patcher config so it must have
                    write permission to the repository if commitPatcherConfig
                    is True.
           patcherToolsTag: A tag that has been applied to all of:
                              mozillaCentral, buildTools, patcher,
                              MozBuild, Bootstrap.
                            This version of all of the above tools will be
                            used - NOT tip.
           mozillaCentral: Generally is http://hg.mozilla.org/mozilla-central.
                           This repository is passed to patcher which then
                           builds the mar tools out of it.
           buildTools: Generally is http://hg.mozilla.org/build/tools.
                       This repository must contain the patcher-config-bump.pl
                       script.
           patcherConfig: The filename of the patcher config file to bump,
                          and pass to patcher.
           commitPatcherConfig: This flag simply controls whether or not
                                the bumped patcher config file will be
                                commited to the CVS repository.
        """
        ReleaseFactory.__init__(self)

        patcherConfigFile = 'patcher-configs/%s' % patcherConfig
        shippedLocales = self.getShippedLocales(mozillaCentral, baseTag,
                                                appName)
        candidatesDir = self.getCandidatesDir(productName, appVersion,
                                              buildNumber)
        updateDir = 'build/temp/%s/%s-%s' % \
          (productName, oldVersion, appVersion)
        marDir = '%s/ftp/%s/nightly/%s-candidates/build%s' % \
          (updateDir, productName, appVersion, buildNumber)

        # If useBetaChannel is False the unnamed snippet type will be
        # 'beta' channel snippets (and 'release' if we're into stable releases).
        # If useBetaChannel is True the unnamed type will be 'release'
        # channel snippets
        snippetTypes = ['', 'test']
        if useBetaChannel:
            snippetTypes.append('beta')

        self.addStep(CVS,
         cvsroot=cvsroot,
         branch=patcherToolsTag,
         cvsmodule='mozilla/tools/patcher'
        )
        self.addStep(ShellCommand,
         command=['cvs', '-d', cvsroot, 'co', '-r', patcherToolsTag,
                  '-d', 'MozBuild',
                  'mozilla/tools/release/MozBuild'],
         description=['checkout', 'MozBuild'],
         haltOnFailure=True
        )
        self.addStep(ShellCommand,
         command=['cvs', '-d', cvsroot, 'co', '-r', patcherToolsTag,
                  '-d' 'Bootstrap',
                  'mozilla/tools/release/Bootstrap/Util.pm'],
         description=['checkout', 'Bootstrap/Util.pm'],
         haltOnFailure=True
        )
        self.addStep(ShellCommand,
         command=['cvs', '-d', cvsroot, 'co', '-d' 'patcher-configs',
                  'mozilla/tools/patcher-configs'],
         description=['checkout', 'patcher-configs'],
         haltonFailure=True
        )
        self.addStep(ShellCommand,
         command=['hg', 'clone', '--rev', patcherToolsTag, buildTools],
         description=['clone', 'build tools'],
         haltOnFailure=True
        )
        self.addStep(ShellCommand,
         command=['wget', '-O', 'shipped-locales', shippedLocales],
         description=['get', 'shipped-locales'],
         haltOnFailure=True
        )

        bumpCommand = ['perl', 'tools/release/patcher-config-bump.pl',
                       '-p', productName, '-v', appVersion, '-a', appVersion,
                       '-o', oldVersion, '-b', str(buildNumber),
                       '-c', patcherConfigFile, '-t', stagingServer,
                       '-f', ftpServer, '-d', bouncerServer,
                       '-l', 'shipped-locales']
        if useBetaChannel:
            bumpCommand.append('-u')
        self.addStep(ShellCommand,
         command=bumpCommand,
         description=['bump', patcherConfig],
         haltOnFailure=True
        )
        self.addStep(TinderboxShellCommand,
         command=['cvs', 'diff', '-u', patcherConfigFile],
         description=['diff', patcherConfig],
         ignoreCodes=[1]
        )
        if commitPatcherConfig:
            self.addStep(ShellCommand,
             command=['cvs', 'commit', '-m',
                      'Automated configuration bump: ' + \
                      '%s, from %s to %s build %s' % \
                        (patcherConfig, oldVersion, appVersion, buildNumber)
                     ],
             workdir='build/patcher-configs',
             description=['commit', patcherConfig],
             haltOnFailure=True
            )
        self.addStep(ShellCommand,
         command=['perl', 'patcher2.pl', '--build-tools-hg', 
                  '--tools-revision=%s' % patcherToolsTag,
                  '--app=%s' % productName,
                  '--config=%s' % patcherConfigFile],
         description=['patcher:', 'build tools'],
         haltOnFailure=True
        )
        self.addStep(ShellCommand,
         command=['perl', 'patcher2.pl', '--download',
                  '--app=%s' % productName,
                  '--config=%s' % patcherConfigFile],
         description=['patcher:', 'download builds'],
         haltOnFailure=True
        )
        self.addStep(ShellCommand,
         command=['perl', 'patcher2.pl', '--create-patches',
                  '--partial-patchlist-file=patchlist.cfg',
                  '--app=%s' % productName,
                  '--config=%s' % patcherConfigFile],
         description=['patcher:', 'create patches'],
         haltOnFailure=True
        )
        self.addStep(ShellCommand,
         command=['rsync', '-av',
                  '-e', 'ssh -oIdentityFile=~/.ssh/%s' % stageSshKey,
                  '--exclude=*complete.mar',
                  'update',
                  '%s@%s:%s' % (stageUsername, stagingServer, candidatesDir)],
         workdir=marDir,
         description=['upload', 'partial mars'],
         haltOnFailure=True
        )
        # It gets a little hairy down here
        date = strftime('%Y%m%d')
        for type in snippetTypes:
            # Patcher generates an 'aus2' directory and 'aus2.snippetType'
            # directories for each snippetType. Typically, there is 'aus2',
            # 'aus2.test', and (when we're out of beta) 'aus2.beta'.
            localDir = 'aus2'
            # On the AUS server we store each type of snippet in a directory
            # named thusly, with the snippet type appended to it
            remoteDir = '%s-%s-%s' % (date, productName.title(), appVersion)
            if type != '':
                localDir = localDir + '.%s' % type
                remoteDir = remoteDir + '-%s' % type
            snippetDir = '/opt/aus2/snippets/staging/%s' % remoteDir

            self.addStep(ShellCommand,
             command=['rsync', '-av', localDir + '/',
                      '%s@%s:%s' % (ausUser, ausHost, snippetDir)],
             workdir=updateDir,
             description=['upload', '%s snippets' % type],
             haltOnFailure=True
            )

            # We only push test channel snippets from automation.
            if type == 'test':
                self.addStep(ShellCommand,
                 command=['ssh', '-l', ausUser, ausHost,
                          '~/bin/backupsnip %s' % remoteDir],
                 timeout=7200, # 2 hours
                 description=['backupsnip'],
                 haltOnFailure=True
                )
                self.addStep(ShellCommand,
                 command=['ssh', '-l', ausUser, ausHost,
                          '~/bin/pushsnip %s' % remoteDir],
                 timeout=3600, # 1 hour
                 description=['pushsnip'],
                 haltOnFailure=True
                )
                # Wait for timeout on AUS's NFS caching to expire before
                # attempting to test newly-pushed snippets
                self.addStep(ShellCommand,
                 command=['sleep','360'],
                 description=['wait for live snippets']
                )



class UpdateVerifyFactory(ReleaseFactory):
    def __init__(self, mozillaCentral, cvsroot, buildTools, patcherToolsTag,
                 hgUsername, baseTag, appName, platform, productName,
                 oldVersion, oldBuildNumber, version, buildNumber, ausServerUrl,
                 stagingServer, verifyConfig, oldAppVersion=None,
                 appVersion=None, hgSshKey=None):
        ReleaseFactory.__init__(self)

        if not oldAppVersion:
            oldAppVersion = oldVersion
        if not appVersion:
            appVersion = version

        oldLongVersion = self.makeLongVersion(oldVersion)
        longVersion = self.makeLongVersion(version)
        # Unfortunately we can't use the getCandidatesDir() function here
        # because that returns it as a file path on the server and we need
        # an http:// compatible path
        oldCandidatesDir = \
          '/pub/mozilla.org/%s/nightly/%s-candidates/build%s' % \
            (productName, oldVersion, oldBuildNumber)

        verifyConfigPath = 'release/updates/%s' % verifyConfig
        shippedLocales = self.getShippedLocales(mozillaCentral, baseTag,
                                                appName)
        pushRepo = self.getPushRepo(buildTools)
        sshKeyOption = self.getSshKeyOption(hgSshKey)

        self.addStep(Mercurial,
         mode='clobber',
         repourl=buildTools
        )
        self.addStep(ShellCommand,
         command=['cvs', '-d', cvsroot, 'co', '-r', patcherToolsTag,
                  '-d', 'MozBuild',
                  'mozilla/tools/release/MozBuild'],
         description=['checkout', 'MozBuild'],
         haltOnFailure=True
        )
        self.addStep(ShellCommand,
         command=['cvs', '-d', cvsroot, 'co', '-r', patcherToolsTag,
                  '-d', 'Bootstrap',
                  'mozilla/tools/release/Bootstrap/Util.pm'],
         description=['checkout', 'Bootstrap/Util.pm'],
         haltOnFailure=True
        )
        self.addStep(ShellCommand,
         command=['wget', '-O', 'shipped-locales', shippedLocales],
         description=['get', 'shipped-locales'],
         haltOnFailure=True
        )
        self.addStep(ShellCommand,
         command=['perl', 'release/update-verify-bump.pl',
                  '-o', platform, '-p', productName,
                  '--old-version=%s' % oldVersion,
                  '--old-app-version=%s' % oldAppVersion,
                  '--old-long-version=%s' % oldLongVersion,
                  '-v', version, '--app-version=%s' % appVersion,
                  '--long-version=%s' % longVersion,
                  '-n', str(buildNumber), '-a', ausServerUrl,
                  '-s', stagingServer, '-c', verifyConfigPath,
                  '-d', oldCandidatesDir, '-l', 'shipped-locales',
                  '--pretty-candidates-dir'],
         description=['bump', verifyConfig]
        )
        self.addStep(ShellCommand,
         command=['hg', 'commit', '-m',
                  'Automated configuration bump: ' + \
                  '%s, from %s to %s build %s' % \
                    (verifyConfig, oldVersion, version, buildNumber)],
         description=['commit', verifyConfig],
         haltOnFailure=True
        )
        self.addStep(ShellCommand,
         command=['hg', 'push', '-e',
                  'ssh -l %s %s' % (hgUsername, sshKeyOption),
                  '-f', pushRepo],
         description=['push updated', 'config'],
         haltOnFailure=True
        )
        self.addStep(UpdateVerify,
         command=['bash', 'verify.sh', '-c', verifyConfig],
         workdir='build/release/updates',
         description=['./verify.sh', verifyConfig]
        )



class ReleaseFinalVerification(ReleaseFactory):
    def __init__(self, buildTools, linuxConfig, macConfig, win32Config):
        ReleaseFactory.__init__(self)
        self.addStep(Mercurial,
         mode='clobber',
         repourl=buildTools
        )
        self.addStep(ShellCommand,
         command=['bash', 'final-verification.sh',
                  linuxConfig, macConfig, win32Config],
         description=['final-verification.sh'],
         workdir='build/release'
        )

class UnittestBuildFactory(MozillaBuildFactory):
    def __init__(self, platform, config_repo_url, config_dir, branch,
                 repoPath, buildToolsRepo, buildSpace, **kwargs):
        self.config_repo_url = config_repo_url
        self.config_dir = config_dir
        self.branch = branch
        self.repoPath = repoPath

        env_map = {
                'linux': 'linux-centos-unittest',
                'macosx': 'mac-osx-unittest',
                'win32': 'win32-vc8-mozbuild-unittest',
                }

        config_dir_map = {
                'linux': 'linux/%s/unittest' % branch,
                'macosx': 'macosx/%s/unittest' % branch,
                'win32': 'win32/%s/unittest' % branch,
                }

        self.platform = platform.split('-')[0].replace('64', '')
        assert self.platform in ('linux', 'win32', 'macosx')

        self.env = MozillaEnvironments[env_map[self.platform]]

        MozillaBuildFactory.__init__(self, buildToolsRepo, buildSpace, **kwargs)

        if self.platform == 'win32':
            self.addStep(TinderboxShellCommand, name="kill sh",
             description='kill sh',
             descriptionDone="killed sh",
             command="pskill -t sh.exe",
             workdir="D:\\Utilities"
            )
            self.addStep(TinderboxShellCommand, name="kill make",
             description='kill make',
             descriptionDone="killed make",
             command="pskill -t make.exe",
             workdir="D:\\Utilities"
            )
            self.addStep(TinderboxShellCommand, name="kill firefox",
             description='kill firefox',
             descriptionDone="killed firefox",
             command="pskill -t firefox.exe",
             workdir="D:\\Utilities"
            )

        self.addStepNoEnv(Mercurial, mode='update',
         baseURL='http://hg.mozilla.org/',
         defaultBranch=self.repoPath
        )

        self.addPrintChangesetStep()

        self.addStep(ShellCommand,
         name="clean configs",
         command=['rm', '-rf', 'mozconfigs'],
         workdir='.'
        )

        self.addStep(ShellCommand,
         name="buildbot configs",
         command=['hg', 'clone', self.config_repo_url, 'mozconfigs'],
         workdir='.'
        )

        self.addStep(ShellCommand, name="copy mozconfig",
         command=['cp',
                  'mozconfigs/%s/%s/mozconfig' % \
                    (self.config_dir, config_dir_map[self.platform]),
                  'build/.mozconfig'],
         description=['copy mozconfig'],
         workdir='.'
        )

        # TODO: Do we need this special windows rule?
        if self.platform == 'win32':
            self.addStep(ShellCommand, name="mozconfig contents",
             command=["type", ".mozconfig"]
            )
        else:
            self.addStep(ShellCommand, name='mozconfig contents',
             command=['cat', '.mozconfig']
            )

        # TODO: Do we need this special windows rule?
        if self.platform == 'win32':
            self.addStep(Compile,
             command=["make", "-f", "client.mk", "build"],
             timeout=60*20,
             warningPattern=''
            )
        else:
            self.addStep(Compile,
             warningPattern='',
             command=['make', '-f', 'client.mk', 'build']
            )

        # TODO: Do we need this special windows rule?
        if self.platform == 'win32':
            self.addStep(unittest_steps.MozillaCheck, warnOnWarnings=True,
             workdir="build\\objdir",
             timeout=60*5
            )
        else:
            self.addStep(unittest_steps.MozillaCheck,
             warnOnWarnings=True,
             timeout=60*5,
             workdir="build/objdir"
            )

        if self.platform == 'win32':
            self.addStep(unittest_steps.CreateProfileWin,
             warnOnWarnings=True,
             workdir="build",
             command = r'python testing\tools\profiles\createTestingProfile.py --clobber --binary objdir\dist\bin\firefox.exe',
             clobber=True
            )
        else:
            self.addStep(unittest_steps.CreateProfile,
             warnOnWarnings=True,
             workdir="build",
             command = r'python testing/tools/profiles/createTestingProfile.py --clobber --binary objdir/dist/bin/firefox',
             clobber=True
            )

        if self.platform == 'linux':
            self.addStep(unittest_steps.MozillaUnixReftest, warnOnWarnings=True,
             workdir="build/layout/reftests",
             timeout=60*5
            )
            self.addStep(unittest_steps.MozillaUnixCrashtest, warnOnWarnings=True,
             workdir="build/testing/crashtest"
            )
            self.addStep(unittest_steps.MozillaMochitest, warnOnWarnings=True,
             workdir="build/objdir/_tests/testing/mochitest",
             timeout=60*5
            )
            self.addStep(unittest_steps.MozillaMochichrome, warnOnWarnings=True,
             workdir="build/objdir/_tests/testing/mochitest"
            )
            self.addStep(unittest_steps.MozillaBrowserChromeTest, warnOnWarnings=True,
             workdir="build/objdir/_tests/testing/mochitest"
            )
            self.addStep(unittest_steps.MozillaA11YTest, warnOnWarnings=True,
             workdir="build/objdir/_tests/testing/mochitest"
            )
        elif self.platform == 'macosx':
            self.addStep(unittest_steps.MozillaOSXReftest, warnOnWarnings=True,
             workdir="build/layout/reftests",
             timeout=60*5
            )
            self.addStep(unittest_steps.MozillaOSXCrashtest, warnOnWarnings=True,
             workdir="build/testing/crashtest"
            )
            self.addStep(unittest_steps.MozillaMochitest, warnOnWarnings=True,
             workdir="build/objdir/_tests/testing/mochitest",
             timeout=60*5
            )
            self.addStep(unittest_steps.MozillaMochichrome, warnOnWarnings=True,
             workdir="build/objdir/_tests/testing/mochitest",
             leakThreshold="8"
            )
            self.addStep(unittest_steps.MozillaBrowserChromeTest, warnOnWarnings=True,
             workdir="build/objdir/_tests/testing/mochitest"
            )
        elif self.platform == 'win32':
            self.addStep(unittest_steps.MozillaWin32Reftest, warnOnWarnings=True,
             workdir="build\\layout\\reftests",
             timeout=60*5
            )
            self.addStep(unittest_steps.MozillaWin32Crashtest, warnOnWarnings=True,
             workdir="build\\testing\\crashtest"
            )
            self.addStep(unittest_steps.MozillaMochitest, warnOnWarnings=True,
             workdir="build\\objdir\\_tests\\testing\\mochitest",
             timeout=60*5
            )
            # Can use the regular build step here. Perl likes the PATHs that way anyway.
            self.addStep(unittest_steps.MozillaMochichrome, warnOnWarnings=True,
             workdir="build\\objdir\\_tests\\testing\\mochitest"
            )
            self.addStep(unittest_steps.MozillaBrowserChromeTest, warnOnWarnings=True,
             workdir="build\\objdir\\_tests\\testing\\mochitest"
            )
            self.addStep(unittest_steps.MozillaA11YTest, warnOnWarnings=True,
             workdir="build\\objdir\\_tests\\testing\\mochitest"
            )

    def addPrintChangesetStep(self):
        changesetLink = ''.join(['<a href=http://hg.mozilla.org/',
            self.branch,
            '/index.cgi/rev/%(got_revision)s title="Built from revision %(got_revision)s">rev:%(got_revision)s</a>'])
        self.addStep(ShellCommand,
         command=['echo', 'TinderboxPrint:', WithProperties(changesetLink)],
        )

    def addStep(self, *args, **kw):
        kw.setdefault('env', self.env)
        return BuildFactory.addStep(self, *args, **kw)

    def addStepNoEnv(self, *args, **kw):
        return BuildFactory.addStep(self, *args, **kw)


class L10nVerifyFactory(ReleaseFactory):
    def __init__(self, cvsroot, buildTools,
                stagingServer, productName,
                 appVersion, buildNumber,
                 oldAppVersion, oldBuildNumber,
                 verifyDir='verify',
                 linuxExtension='bz2'):

        ReleaseFactory.__init__(self)

        productDir = 'build/%s/%s-%s' % (verifyDir, 
                                         productName,
                                         appVersion)
        verifyDirVersion = '%s/tools/release/l10n' % productDir

        # Remove existing verify dir 
        self.addStep(ShellCommand,
         description=['remove', 'verify', 'dir'],
         descriptionDone=['removed', 'verify', 'dir'],
         command=['rm', '-rf', verifyDir],
         haltOnFailure=True,
        )

        self.addStep(ShellCommand,
         description=['(re)create', 'verify', 'dir'],
         descriptionDone=['(re)created', 'verify', 'dir'],
         command=['bash', '-c', 'mkdir -p ' + verifyDirVersion], 
         haltOnFailure=True,
        )
        
        # check out l10n verification scripts
        self.addStep(ShellCommand,
         command=['hg', 'clone', buildTools, 'tools'],
         workdir=productDir,
         description=['clone', 'build tools'],
         haltOnFailure=True
        )
        
        # Download current release
        self.addStep(ShellCommand,
         description=['download', 'current', 'release'],
         descriptionDone=['downloaded', 'current', 'release'],
         command=['rsync',
                  '-Lav', 
                  '-e', 'ssh', 
                  '--exclude=*.asc',
                  '--exclude=source',
                  '--exclude=xpi',
                  '--exclude=unsigned',
                  '--exclude=update',
                  '%s:/home/ftp/pub/%s/nightly/%s-candidates/build%s/*' %
                   (stagingServer, productName, appVersion, str(buildNumber)),
                  '%s-%s-build%s/' % (productName, 
                                      appVersion, 
                                      str(buildNumber))
                  ],
         workdir=verifyDirVersion,
         haltOnFailure=True,
         timeout=60*60
        )

        # Download previous release
        self.addStep(ShellCommand,
         description=['download', 'previous', 'release'],
         descriptionDone =['downloaded', 'previous', 'release'],
         command=['rsync',
                  '-Lav', 
                  '-e', 'ssh', 
                  '--exclude=*.asc',
                  '--exclude=source',
                  '--exclude=xpi',
                  '--exclude=unsigned',
                  '--exclude=update',
                  '%s:/home/ftp/pub/%s/nightly/%s-candidates/build%s/*' %
                   (stagingServer, 
                    productName, 
                    oldAppVersion,
                    str(oldBuildNumber)),
                  '%s-%s-build%s/' % (productName, 
                                      oldAppVersion,
                                      str(oldBuildNumber))
                  ],
         workdir=verifyDirVersion,
         haltOnFailure=True,
         timeout=60*60
        )

        currentProduct = '%s-%s-build%s' % (productName, 
                                            appVersion,
                                            str(buildNumber))
        previousProduct = '%s-%s-build%s' % (productName, 
                                             oldAppVersion,
                                             str(oldBuildNumber))

        for product in [currentProduct, previousProduct]:
            self.addStep(ShellCommand,
                         description=['(re)create', 'product', 'dir'],
                         descriptionDone=['(re)created', 'product', 'dir'],
                         command=['bash', '-c', 'mkdir -p %s/%s' % (verifyDirVersion, product)], 
                         workdir='.',
                         haltOnFailure=True,
                        )
            self.addStep(ShellCommand,
                         description=['verify', 'l10n', product],
                         descriptionDone=['verified', 'l10n', product],
                         command=["bash", "-c", 
                                  "./verify_l10n.sh " + product],
                         workdir=verifyDirVersion,
                         haltOnFailure=True,
                        )

        self.addStep(L10nVerifyMetaDiff,
                     currentProduct=currentProduct,
                     previousProduct=previousProduct,
                     workdir=verifyDirVersion,
                     )
