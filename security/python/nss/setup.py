#!/usr/bin/python
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
# The Original Code is a Python binding for Network Security Services (NSS).
#
# The Initial Developer of the Original Code is Red Hat, Inc.
#   (Author: John Dennis <jdennis@redhat.com>)
#
# Portions created by the Initial Developer are Copyright (C) 2008,2009
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 2 or later (the "GPL"), or
# the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
# in which case the provisions of the GPL or the LGPL are applicable instead
# of those above.  If you wish to allow use of your version of this file only
# under the terms of either the GPL or the LGPL, and not to allow others to
# use your version of this file under the terms of the MPL, indicate your
# decision by deleting the provisions above and replace them with the notice
# and other provisions required by the GPL or the LGPL. If you do not delete
# the provisions above, a recipient may use your version of this file under
# the terms of any one of the MPL, the GPL or the LGPL.
#
# ***** END LICENSE BLOCK *****

import sys
import os
import re
import subprocess
import re

from distutils.core import setup, Extension, Command
from distutils.spawn import find_executable, spawn
from distutils import log
from distutils.filelist import FileList
from distutils.util import subst_vars, change_root
from distutils.command.build_py import build_py as _build_py
from distutils.command.sdist import sdist as _sdist

version = "0.9"

def update_version():
    """If the version string in __init__.py doesn't match the current
    version then edit the file replacing the version string
    with the current version."""

    version_file = 'src/__init__.py'
    tmp_file = 'src/__init__.tmp'
    version_re = re.compile("^\s*__version__\s*=\s*['\"]([^'\"]*)['\"]")
    need_to_update = False
    version_found = False
    f = open(tmp_file, "w")
    for line in open(version_file):
        match = version_re.search(line)
        if match:
            version_found = True
            file_version = match.group(1)
            if file_version != version:
                need_to_update = True
                f.write("__version__ = '%s'\n" % version)
        else:
            f.write(line)
    if not version_found:
        need_to_update = True
        f.write("__version__ = '%s'\n" % version)

    if need_to_update:
        print "Updating version in \"%s\" to \"%s\"" % (version_file, version)
        os.rename(tmp_file, version_file)
    else:
        os.unlink(tmp_file)

class BuildPy(_build_py):
    """Specialized Python source builder."""

    def run(self):
        update_version()
        _build_py.run(self)


class SDist(_sdist):
    """Specialized Python source builder."""

    def run(self):
        update_version()
        _sdist.run(self)


class BuildDoc(Command):
    description = 'generate documentation'
    user_options = [('docdir=', 'd', "directory root for documentation"),
                   ]

    def has_epydoc (self):
        if find_executable('epydoc'):
            return True
        else:
            return False

    sub_commands = [('build_api_doc', has_epydoc),
                   ]

    def initialize_options(self):
        self.build_base = None
        self.build_lib = None
        self.docdir = None

    def finalize_options(self):
        self.set_undefined_options('build',
                                   ('build_base', 'build_base'),
                                   ('build_lib', 'build_lib'))
        if self.docdir is None:
            self.docdir = change_root(self.build_base, 'doc')

    def run(self):
        self.run_command('build')
        for cmd_name in self.get_sub_commands():
            self.run_command(cmd_name)


class BuildApiDoc(Command):
    description = 'generate the API documentation'
    user_options = [('docdir=',  'd', "directory root for documentation"),
                    ('action=',  'a', "epydoc action (html, latex, dvi, ps, pdf, check, pickle"),
                    ('htmldir',  'H', "directory to locate the API HTML files under"),
                   ]

    def initialize_options(self):
        self.build_base = None
        self.build_lib = None
        self.docdir = None
        self.action = None
        self.htmldir = None

    def finalize_options(self):
        self.set_undefined_options('build',
                                   ('build_base', 'build_base'),
                                   ('build_lib', 'build_lib'))

        if self.action is None:
            self.action = 'html'

        if self.docdir is None:
            if self.action == 'html':
                self.docdir = change_root(self.get_finalized_command('build_doc').docdir, 'html')
            else:
                self.docdir = self.get_finalized_command('build_doc').docdir

    def run(self):
        prog = find_executable('epydoc')
        pkg_dirs = [change_root(self.build_lib, pkg) for pkg in self.distribution.packages]
        cmd = [prog, '-v', '--%s' % self.action, '--docformat', 'restructuredtext', '-o', self.docdir]
        #if self.verbose: cmd.append('-v')
        cmd.extend(pkg_dirs)
        self.mkpath(self.docdir)
        spawn(cmd)

class InstallDoc(Command):
    description = 'install documentation'
    user_options = [('docdir=', 'd', "directory root for documentation"),
                    ('root=', None,
                     "install everything relative to this alternate root directory"),
                    ('skip-build', None,
                     "skip rebuilding everything (for testing/debugging)"),
                   ]

    def initialize_options(self):
        self.root = None
        self.build_base = None
        self.docdir = None
        self.skip_build = False

    def finalize_options(self):
        self.set_undefined_options('install',
                                   ('root', 'root'))
        self.set_undefined_options('build',
                                   ('build_base', 'build_base'))

        if self.docdir is None:
            self.docdir = change_root(self.build_base, 'doc')

    def run(self):
        if not self.skip_build:
            self.run_command('build_doc')


        dst_root = change_root(self.root, self.docdir)
        self.copy_transformed_tree(doc_manifest,
                                   dst_root=dst_root,
                                   substitutions={'docdir' : self.docdir})

    def copy_transformed_tree(self, install_specs, dst_root=None, src_root=None, substitutions={}):
        """
        Copy parts of a source tree to a destination tree with a
        different tree structure and/or names.

        The basic idea: given a set of source files, copy them to a
        destination directory, let's call this operation an
        install_spec. A sequence of install_spec's allows one to build
        up the destrination tree in any structure desired.

        Each install_spec consists of 3 components
        (manifest_template, dst_xforms, dst_dir):

        The manifest_template is a sequence where each item is identical
        to a line in the MANIFEST.in template described in distutils. This
        gives you ability to easily specify a set of source files in a
        compact abstract manner (with recursion, exclusion, etc.) The
        manifest_template yields a sequence of source paths.

        dst_xforms is a sequence of regular expression substitutions
        applied to the each source path to yield a rewritten destination
        path. Each transform is expressed as a two-valued sequence
        (pattern, replacement)

        dst_dir is a destination directory where the destinations paths
        are written to. dst_dir is always relative to the dst_root.

        All input may be parametrized via variable substitutions
        supplied by a substitution dict. Any use of $name will cause
        name to be looked up first in the substitution dict and then
        if its not found there in the enviorment. If found it will be
        replaced with it's value.

        The pseudo code algorithm for processing an install_spec is:

        substitute all variables in manifest template
        src_list = evaluate manifest template
        for each src_path in src_list:
            dst_path = src_path
            for each xform in dst_xform:
                apply xform to dst_path
            copy src_root+src_path to dst_root+dest_dir+dest_path

        This process is repeated for each install spec. The src_root and
        dst_root are also subject to variable substitution.


        Examples:

        Copy all text files in build/doc to doc:

            copy_transformed_tree([[["include build/doc *.txt"], None, 'doc']])

        Copy all html files found under build to doc/html and change the extension from
        .html to .htm

            copy_transformed_tree([[["include build *.html"], [('\.html$','.htm')], 'doc']])

    """

        if src_root is not None: src_root = subst_vars(src_root, substitutions)
        if dst_root is not None: dst_root = subst_vars(dst_root, substitutions)

        filelist = FileList()
        if src_root is None:
            filelist.findall()
        else:
            filelist.findall(src_root)

        for manifest_template, dst_xforms, dst_dir in install_specs:
            if dst_dir is not None: dst_dir = subst_vars(dst_dir, substitutions)

            filelist.files = [] # reinitialize to empty

            for line in manifest_template:
                filelist.process_template_line(subst_vars(line, substitutions))

            for src_path in filelist.files:
                dst_path = src_path
                if dst_xforms:
                    for dst_xform in dst_xforms:
                        dst_path = re.sub(dst_xform[0], dst_xform[1], dst_path)
                if dst_dir is not None:
                    dst_path = change_root(dst_dir, dst_path)
                if dst_root is None:
                    full_dst_path = dst_path
                else:
                    full_dst_path = change_root(dst_root, dst_path)
                full_dst_dir = os.path.dirname(full_dst_path)
                self.mkpath(full_dst_dir)
                self.copy_file(src_path, full_dst_path)



#------------------------------------------------------------------------------

long_description='''
Python bindings for Network Security Services (NSS) and Netscape Portable Runtime (NSPR).
'''

debug_compile_args = ['-O0', '-g']
# force debug build until package matures
# extra_compile_args = debug_compile_args
extra_compile_args = []

nss_error_extension = \
    Extension('nss.error',
              sources            = ['src/py_nspr_error.c'],
              include_dirs       = ['/usr/include/nss3', '/usr/include/nspr4'],
              libraries          = ['nspr4'],
              extra_compile_args = extra_compile_args,
              )

nss_io_extension = \
    Extension('nss.io',
              sources            = ['src/py_nspr_io.c'],
              include_dirs       = ['/usr/include/nss3', '/usr/include/nspr4'],
              libraries          = ['nspr4'],
              extra_compile_args = extra_compile_args,
              )

nss_nss_extension = \
    Extension('nss.nss',
              sources            = ['src/py_nss.c'],
              include_dirs       = ['src', '/usr/include/nss3', '/usr/include/nspr4'],
              libraries          = ['nspr4', 'ssl3', 'nss3'],
              extra_compile_args = extra_compile_args,
              )

nss_ssl_extension = \
    Extension('nss.ssl',
              sources            = ['src/py_ssl.c'],
              include_dirs       = ['src', '/usr/include/nss3', '/usr/include/nspr4'],
              libraries          = ['nspr4', 'ssl3'],
              extra_compile_args = extra_compile_args,
              )

doc_manifest = [
    [['include README LICENSE*',
      'recursive-include doc *.py *.txt',],
     [('^doc/', '')], None],
    [['recursive-include build/doc/html *'],
     [('^build/doc/', 'api/')], None],
]

setup(name             = 'python-nss',
      version          = version,
      description      = 'Python bindings for Network Security Services (NSS) and Netscape Portable Runtime (NSPR)',
      long_description = long_description,
      author           = 'John Dennis',
      author_email     = 'jdennis@redhat.com',
      maintainer       = 'John Dennis',
      maintainer_email = 'jdennis@redhat.com',
      license          = 'MPLv1.1 or GPLv2+ or LGPLv2+',
      platforms        = 'posix',
      url              = ':pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot mozilla/security/python/nss',
      download_url     = '',
      ext_modules      = [nss_error_extension,
                          nss_io_extension,
                          nss_nss_extension,
                          nss_ssl_extension,
                         ],
      package_dir      = {'nss':'src'},
      packages         = ['nss'],
      cmdclass         = {'build_doc'     : BuildDoc,
                          'build_api_doc' : BuildApiDoc,
                          'install_doc'   : InstallDoc,
                          'build_py'      : BuildPy,
                          'sdist'         : SDist,
                         },

)

