from datetime import datetime
import os.path

from twisted.python import log

from buildbot.process.factory import BuildFactory
from buildbot.steps.shell import Compile, ShellCommand, WithProperties#, \
#  SetProperty
from buildbot.steps.source import Mercurial
from buildbot.steps.transfer import FileDownload

import buildbotcustom.steps.misc
import buildbotcustom.steps.test
import buildbotcustom.steps.transfer
import buildbotcustom.steps.updates
reload(buildbotcustom.steps.misc)
reload(buildbotcustom.steps.test)
reload(buildbotcustom.steps.transfer)
reload(buildbotcustom.steps.updates)

from buildbotcustom.steps.misc import SetMozillaBuildProperties
from buildbotcustom.steps.test import AliveTest, CompareBloatLogs, \
  CompareLeakLogs, Codesighs, GraphServerPost
from buildbotcustom.steps.transfer import MozillaStageUpload
from buildbotcustom.steps.updates import CreateCompleteUpdateSnippet


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
         command=['rm', '-rfv', 'build'],
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



class MercurialBuildFactory(BuildFactory):
    def __init__(self, env, objdir, platform, branch, sourceRepo, configRepo,
                 configSubDir, profiledBuild, stageServer=None,
                 stageUsername=None, stageGroup=None, stageSshKey=None,
                 stageBasePath=None, ausBaseUploadDir=None,
                 updatePlatform=None, downloadBaseURL=None, ausUser=None,
                 ausHost=None, nightly=False, leakTest=False, codesighs=True,
                 graphServer=None, graphSelector=None, graphBranch=None,
                 baseName=None, uploadPackages=True, uploadSymbols=True,
                 dependToDated=True, createSnippet=False, **kwargs):
        BuildFactory.__init__(self, **kwargs)
        self.env = env
        self.objdir = objdir
        self.platform = platform
        self.branch = branch
        self.sourceRepo = sourceRepo
        self.configRepo = configRepo
        self.configSubDir = configSubDir
        self.profiledBuild = profiledBuild
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

        if self.uploadPackages:
            assert stageServer and stageUsername and stageSshKey
            assert stageBasePath
        if self.nightly:
            assert ausBaseUploadDir and updatePlatform and downloadBaseURL
            assert ausUser and ausHost

        # platform actually contains more than just the platform...
        # we need that to figure out which mozconfig to use
        # but for other purposes we only need to know linux/win32/macosx
        # platform can be things like: linux, win32-debug, macosx-release, etc.
        self.mozconfig = 'configs/%s/%s/mozconfig' % (self.configSubDir,
                                                      platform)
        log.msg("\n\n" + self.mozconfig + "\n\n")
        # we don't need the extra cruft in 'platform' anymore
        self.platform = platform.split('-')[0].replace('64', '')
        assert self.platform in ('linux', 'win32', 'macosx')

        self.logUploadDir = 'tinderbox-builds/%s-%s/' % (self.branch,
                                                         self.platform)
        # this is a tad ugly because we need to python interpolation
        # as well as WithProperties
        # here's an example of what it translates to:
        # /opt/aus2/build/0/Firefox/mozilla2/WINNT_x86-msvc/2008010103/en-US
        self.ausFullUploadDir = '%s/%s/%%(buildid)s/en-US' % \
          (self.ausBaseUploadDir, self.updatePlatform)

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
        #  for everything:
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
        self.addCleanupSteps()

    def addBuildSteps(self):
        if self.nightly:
            self.addStep(ShellCommand,
             command=['rm', '-rfv', 'build'],
             env=self.env,
             workdir='.'
            )
        self.addStep(ShellCommand,
         command=['echo', WithProperties('Building on: %(slavename)s')],
         env=self.env
        )
        self.addStep(ShellCommand,
         command="rm -rfv %s/dist/firefox-* %s/dist/install/sea/*.exe " %
                  (self.objdir, self.objdir),
         env=self.env,
         description=['deleting', 'old', 'package'],
         descriptionDone=['delete', 'old', 'package']
        )
        if self.nightly:
            self.addStep(ShellCommand,
             command="find 20* -maxdepth 2 -mtime +7 -exec rm -rfv {} \;",
             env=self.env,
             workdir='.',
             description=['cleanup', 'old', 'symbols'],
             flunkOnFailure=False
            )
        self.addStep(Mercurial,
         mode='update',
         baseURL=self.sourceRepo,
         defaultBranch=self.branch
        )
        changesetLink = '<a href=%s/%s/index.cgi/rev' % (self.sourceRepo,
                                                         self.branch)
        changesetLink += '/%(got_revision)s title="Built from revision %(got_revision)s">rev:%(got_revision)s</a>'
        self.addStep(ShellCommand,
         command=['echo', 'TinderboxPrint:', WithProperties(changesetLink)]
        )
        self.addStep(ShellCommand,
         command=['rm', '-rfv', 'configs'],
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
         # cp configs/mozilla2/$platform/mozconfig .mozconfig
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
         timeout=3600 # 1 hour, because windows PGO builds take a long time
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
        if self.nightly:
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
         milestone=self.branch,
         remoteHost=self.stageServer,
         remoteBasePath=self.stageBasePath,
         platform=self.platform,
         group=self.stageGroup,
         sshKey=self.stageSshKey,
         releaseToLatest=releaseToLatest,
         releaseToDated=releaseToDated,
         releaseToTinderboxBuilds=True,
         tinderboxBuildsDir='%s-%s' % (self.branch, self.platform),
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
         milestone=self.branch,
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
             command=['rm', '-rfv', 'build'],
             env=self.env,
             workdir='.'
            )
            # no need to clean-up temp files if we clobber the whole directory
            return

        # OS X builds eat up a ton of space with -save-temps enabled
        # until we have dwarf support we need to clean this up so we don't
        # fill up the disk
        if self.platform.startswith("macosx"):
            self.addStep(ShellCommand,
             command=['find', '-E', '.', '-iregex',
                      '.*\.(i|s|mii|ii)$', '-exec', 'rm', '{}', ';'],
             workdir='build/%s' % self.objdir
            )



class ReleaseFactory(BuildFactory):
    def getPushRepo(repo):
        for proto in ('https', 'http'):
             if repo.startswith(proto):
                 return repo.replace(proto, 'ssh')
        return repo

    def getRepoName(repo):
        return repo.rstrip('/').split('/')[-1]



class ReleaseTaggingFactory(ReleaseFactory):
    def __init__(self, repositories, buildToolsRepo, productName, appName,
                 appVersion, milestone, baseTag, buildNumber):
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
        """
        BuildFactory.__init__(self)

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
             haltOnFailure=True
            )
            self.addStep(ShellCommand,
             command=['hg', 'up', '-r',
                      WithProperties('%s', '%s-revision' % repoName)],
             workdir=repoName,
             haltOnFailure=True
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
                 command=['hg', 'commit', '-m',
                          'Automated checkin: version bump remove "pre" ' + \
                          ' from version number for ' + productName + ' ' + \
                          appVersion + ' release on ' + relbranchName],
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
                 command=['hg', 'tag', '-f', '-r',
                          WithProperties('%s', '%s-revision' % repoName),
                          tag],
                 workdir=repoName,
                 description=['tag %s' % repoName],
                 haltOnFailure=True
                )
            self.addStep(ShellCommand,
             command=['hg', 'out', pushRepo],
             workdir=repoName,
            )
            self.addStep(ShellCommand,
             command=['hg', 'push', '-f', pushRepo],
             workdir=repoName,
             description=['push %s' % repoName],
             haltOnFailure=True
            )



class SingleSourceFactory(ReleaseFactory):
    def __init__(self, repository, productName, appVersion, baseTag):
        BuildFactory.__init__(self)
        repoName = self.getRepoName(repository)
        pushRepo = self.getPushRepo(repository)
        releaseTag = '%s_RELEASE' % (baseTag)
        bundleFile = '../%s-%s.bundle' % (productName, appVersion)
        sourceTarball = '%s-%s-source.tar.bz2' % (productName, appVersion)

        self.addStep(ShellCommand,
         command=['hg', 'clone', repository, repoName],
         workdir='.',
         description=['clone %s' % repoName],
         haltOnFailure=True
        )
        self.addStep(ShellCommand,
         command=['hg', 'up', '-r', releaseTag],
         workdir=repoName,
         description=['update to', releaseTag],
         haltOnFailure=True
        )
        self.addStep(ShellCommand,
         command=['hg', 'bundle', '--base', '0', '-r', releaseTag, bundleFile],
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
        self.addStep(ShellCommand,
         command=['tar', '-cjf', sourceTarball, repoName],
         workdir='.',
         description=['create tarball'],
         haltOnFailure=True
        )
        # TODO: upload files
