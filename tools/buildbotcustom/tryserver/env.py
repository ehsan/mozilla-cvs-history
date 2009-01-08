from buildbotcustom.env import MozillaEnvironments

MozillaEnvironments['tryserver'] = {
    'CVS_RSH': 'ssh',
    'MOZ_OBJDIR': "obj-firefox",
    'TINDERBOX_OUTPUT': '1',
    'MOZ_CRASHREPORTER_NO_REPORT': '1',
    'SYMBOL_SERVER_HOST': 'build.mozilla.org',
    'SYMBOL_SERVER_USER': 'trybld',
    'SYMBOL_SERVER_PATH': '/symbols/windows',
    'SYMBOL_SERVER_SSH_KEY': "$ENV{HOME}/.ssh/id_dsa"
}
