from urlparse import urljoin

# This file contains misc. helper function that don't make sense to put in
# other files. For example, functions that are called in a master.cfg

def get_l10n_repositories(file, l10nCentral, relbranch):
    """Reads in a list of locale names and revisions for their associated
       repository from 'file'.
    """
    repositories = {}
    for localeLine in open(file).readlines():
        locale, revision = localeLine.rstrip().split(' ')
        if revision == 'FIXME':
            raise Exception('Found FIXME in %s for locale "%s"' % \
                           (file, locale))
        locale = urljoin(l10nCentral, locale)
        repositories[locale] = {
            'revision': revision,
            'relbranchOverride': relbranch,
            'bumpFiles': []
        }

    return repositories


# This function is used as fileIsImportant parameter for Buildbots that do both
# dep/nightlies and release builds. Because they build the same "branch" this
# allows us to have the release builder ignore HgPoller triggered changse
# and the dep builders only obey HgPoller/Force Build triggered ones.

def isHgPollerTriggered(change, hgUrl):
    if change.comments.find(hgUrl) > -1:
        return True
    return False
