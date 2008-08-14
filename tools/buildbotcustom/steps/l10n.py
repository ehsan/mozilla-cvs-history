import buildbot
from buildbot.process.base import Build
from buildbot.process.buildstep import BuildStep
from buildbot.status.builder import SKIPPED
from buildbot.steps.shell import ShellCommand, Compile
from buildbot.steps.source import Mercurial

class SetLocalesStep(BuildStep):
    """Dummy step to set the "locales" build property"""

    def __init__(self, locales):
        BuildStep.__init__(self)
        self.locales = locales
        self.addFactoryArguments(locales=locales)

    def setBuild(self, build):
        BuildStep.setBuild(self, build)
        build.setProperty('locales', self.locales)

    def start(self):
        return SKIPPED

class LocaleCompile(Compile):
    """Subclass of Compile step for localized builds."""
    flunkOnFailure = False
    haltOnFailure = False
    warnOnWarnings = False

    def __init__(self, locale, **kwargs):
        Compile.__init__(self, **kwargs)
        self.locale = locale
        self.addFactoryArguments(locale=locale)

    def describe(self, done=False):
        d = Compile.describe(self, done)
        if isinstance(d, (str, unicode)):
            return "%s: %s" % (self.locale, d)

        d = list(d)
        d.insert(0, "%s:" % self.locale)
        return d

    def commandComplete(self, cmd):
        self.step_status.locale = self.locale

class NonLocaleMercurial(Mercurial):
    """Subclass of Mercurial pull step for the main tree of a l10n build."""

    def __init__(self, repourl, mainBranch, **kwargs):
        Mercurial.__init__(self, repourl=repourl, **kwargs)
        self.mainBranch = mainBranch
        self.addFactoryArguments(mainBranch=mainBranch)

    def startVC(self, branch, revision, patch):
        # strip out the "branch" which is fake... we always use a repourl
        Mercurial.startVC(self, None, revision, patch)

    def computeSourceRevision(self, changes):
        """Walk backwards through the list of changes: find the revision of
        the last change associated with our 'branch'."""

        if not changes:
            return None

        for change in changes[::-1]:
            if change.branch == self.mainBranch:
                return change.revision

        return None

class LocaleMercurial(Mercurial):
    """Subclass of Mercurial pull step for localized pulls."""

    flunkOnFailure = False
    haltOnFailure = False
    warnOnWarnings = False

    def __init__(self, locale, repourl, localesBranch, baseURL=None, **kwargs):
        if baseURL is not None:
            raise ValueError("baseURL must not be used with MercurialLocale")
        Mercurial.__init__(self, repourl=repourl, **kwargs)
        self.locale = locale
        self.localesBranch = localesBranch
        self.addFactoryArguments(locale=locale,
                                 localesBranch=localesBranch)

    def startVC(self, branch, revision, patch):
        # if we're running a main tree and locales in the same tree,
        # we get a "branch" parameter here which doesn't have the locale
        # information which is encoded in repoURL. Strip it.
        Mercurial.startVC(self, None, revision, patch)

    def computeSourceRevision(self, changes):
        """Walk backwards through the list of changes: find the revision of
        the last change associated with this locale."""

        if not changes:
            return None

        for change in changes[::-1]:
            if change.branch == self.localesBranch and change.locale == self.locale:
                return change.revision

        return None

    def describe(self, done=False):
        return [done and "updated" or "updating", self.locale]

    def commandComplete(self, cmd):
        self.step_status.locale = self.locale
        Mercurial.commandComplete(self, cmd)

def getLocalesForRequests(requests):
    """Return a list of locales to build from a list of build requests.

    The list of locales is obtained from various sources:
    * TriggerLocalesStep (from the main Mozilla build) sets .allLocales on the 
      source stamp from the all-locales file in the main build.
    * When triggered from L10nPeriodic the locales to build are taken from the
      changes (for one more more locales) themselves.
    """

    found = {}
    for r in requests:
        if hasattr(r.source, 'allLocales'):
            return r.source.allLocales
        elif not r.source.changes:
            raise ValueError("Passed a build request with no changes and no allLocales attribute to a localization RepackFactory.")

        for c in r.source.changes:
            if not hasattr(c, 'locale'):
                raise ValueError('allLocales was not set, and a change has no locale attribute.')
            locale = c.locale
            if locale not in found:
                found[locale] = 1

    return found.keys()

class RepackFactory(buildbot.util.ComparableMixin):
    buildClass = Build
    compare_attrs = ['mainRepoURL',
                     'localesRepoURL',
                     'repackLocation',
                     'mainBranch',
                     'localesBranch']

    # This dummy attribute exists so that buildbot configuration can succeed
    steps = ()

    repackFile = 'firefox-3.1a2pre.en-US.linux-i686.tar.bz2'

    def __init__(self, mainRepoURL, localesRepoURL, repackLocation,
                 mainBranch, localesBranch):
        """
        @param mainRepoURL: the repoURL to check out the main codebase
        @param localesRepoURL: the repoURL pattern to check out localized
                               files. %(locale)s will be substituted
        @param repackLocation: the directory from which the main binaries
                               should be downloaded
        @param mainBranch:     the branch name used for Changes from the
                               main codebase
        @param localesBranch:  the branch name used for Changes to
                               localizations. These Changes must have a .locale
                               property.
        """
        
        self.mainRepoURL = mainRepoURL
        self.localesRepoURL = localesRepoURL
        self.repackLocation = repackLocation
        self.mainBranch = mainBranch
        self.localesBranch = localesBranch

    def newBuild(self, requests):
        """Create a list of steps to build these possibly coalesced requests.
        After initial steps, loop over the locales which need building
        and insert steps for each locale."""

        locales = getLocalesForRequests(requests)

        steps = []
        if len(locales) > 0:
            steps.append(SetLocalesStep(locales=locales))
            steps.append(NonLocaleMercurial(repourl=self.mainRepoURL,
                                            mainBranch=self.mainBranch))
            steps.append(ShellCommand(command=['autoconf-2.13'],
                                      haltOnFailure=True))
            steps.append(ShellCommand(command=['rm', '-rf', 'obj'],
                                      flunkOnFailure=False))
            steps.append(ShellCommand(command=['mkdir', '-p', 'l10n', 'obj/dist'],
                                      flunkOnFailure=False))
            steps.append(Compile(command=['../configure', '--enable-application=browser',
                                          '--disable-compile-environment'],
                                 workdir='build/obj',
                                 haltOnFailure=True))
            steps.append(ShellCommand(command=['wget', '-O',
                                               'obj/dist/%s' % self.repackFile,
                                               '%s/%s' % (self.repackLocation, self.repackFile)],
                                      haltOnFailure=True))

            for locale in locales:
                steps.append(LocaleMercurial(locale=locale,
                                             localesBranch=self.localesBranch,
                                             workdir='build/l10n/%s' % locale,
                                             repourl=self.localesRepoURL % {'locale': locale}))
                steps.append(LocaleCompile(locale=locale,
                                           command=['make', '-C', 'obj/browser/locales', 'installers-%s' % locale]))

        b = self.buildClass(requests)
        b.useProgress = False
        b.setStepFactories([step.getStepFactory() for step in steps])
        return b
