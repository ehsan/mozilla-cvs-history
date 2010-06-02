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

import os
import sys
import getopt
import getpass

from nss.error import NSPRError
import nss.io as io
import nss.nss as nss

# -----------------------------------------------------------------------------
def print_extension(level, extension):
    print nss.indented_format([(level, 'Name: %s' % extension.name),
                               (level, 'Critical: %s' % extension.critical)])

    oid_tag = extension.oid_tag

    if   oid_tag == nss.SEC_OID_PKCS12_KEY_USAGE:
        print nss.indented_format([(level, 'Usages:')])
        print nss.indented_format(nss.make_line_pairs(level+1, nss.x509_key_usage(extension.value)))

    elif oid_tag == nss.SEC_OID_X509_SUBJECT_KEY_ID:
        print nss.indented_format([(level, 'Data:')])
        print nss.indented_format(nss.make_line_pairs(level+1,
              extension.value.der_to_hex(nss.OCTETS_PER_LINE_DEFAULT)))

    elif oid_tag == nss.SEC_OID_X509_CRL_DIST_POINTS:
        pts = nss.CRLDistributionPts(extension.value)
        i = 1
        print nss.indented_format([(level, 'CRL Distribution Points: [%d total]' % len(pts))])
        for pt in pts:
            print nss.indented_format([(level+1, 'Point[%d]:' % i)])
            names = pt.get_general_names()
            print nss.indented_format([(level+2, 'General Names: [%d total]' % len(names))])
            for name in names:
                print nss.indented_format([(level+3, '%s:' % name)])
            print nss.indented_format([(level+2, 'Reasons: %s' % (pt.get_reasons(),))])
            print nss.indented_format([(level+2, 'Issuer: %s' % pt.issuer)])

    elif oid_tag == nss.SEC_OID_X509_AUTH_KEY_ID:
        auth_key_id = nss.AuthKeyID(extension.value)
        print nss.indented_format([(level+1, 'Key ID:')])
        print nss.indented_format(nss.make_line_pairs(level+1,
              auth_key_id.key_id.to_hex(nss.OCTETS_PER_LINE_DEFAULT)))
        print nss.indented_format([(level+1, 'Serial Number: %s' % (auth_key_id.serial_number))])
        print nss.indented_format([(level+1, 'Issuer:' % auth_key_id.get_general_names())])

    elif oid_tag == nss.SEC_OID_X509_BASIC_CONSTRAINTS:
        bc = nss.BasicConstraints(extension.value)
        print nss.indented_format([(level, '%s' % str(bc))])

    elif oid_tag == nss.SEC_OID_X509_EXT_KEY_USAGE:
        print nss.indented_format([(level, 'Usages:')])
        print nss.indented_format(nss.make_line_pairs(level+1, nss.x509_ext_key_usage(extension.value)))

    elif oid_tag in (nss.SEC_OID_X509_SUBJECT_ALT_NAME, nss.SEC_OID_X509_ISSUER_ALT_NAME):
        names = nss.x509_alt_name(extension.value)
        print nss.indented_format([(level+2, 'Alternate Names: [%d total]' % len(names))])
        for name in names:
            print nss.indented_format([(level+3, '%s:' % name)])
       
    print

# -----------------------------------------------------------------------------

usage_str = '''
'''

def usage():
    print usage_str

try:
    opts, args = getopt.getopt(sys.argv[1:], "h",
                               ["help", ])
except getopt.GetoptError:
    # print help information and exit:
    usage()
    sys.exit(2)


filename = 'cert.der'

for o, a in opts:
    if o in ("-H", "--help"):
        usage()
        sys.exit()

filename = sys.argv[1]

# Perform basic configuration and setup
nss.nss_init_nodb()

if False:
    l = nss.temp_test()
    print type(l)
    print l
    sys.exit(0)

if len(args):
    filename = args[0]

print "certificate filename=%s" % (filename)

# Read the certificate as DER encoded data
si = nss.read_der_from_file(filename)
# Parse the DER encoded data returning a Certificate object
cert = nss.Certificate(si)

# Get the extension list from the certificate
extensions = cert.extensions

print nss.indented_format([(0, 'Certificate:'),
                           (1, 'Data:')])
print nss.indented_format([(2, 'Version: %d (%#x)' % (cert.version+1, cert.version))])
print nss.indented_format([(2, 'Serial Number: %d (%#x)' % (cert.serial_number, cert.serial_number))])
print nss.indented_format([(2, 'Signature Algorithm: %s' % cert.signature_algorithm)])
print nss.indented_format([(2, 'Issuer: "%s"' % cert.issuer)])
print nss.indented_format([(2, 'Validity:'),
                           (3, 'Not Before: %s' % cert.valid_not_before_str),
                           (3, 'Not After:  %s' % cert.valid_not_after_str)])
print nss.indented_format([(2, 'Subject: "%s"' % cert.subject)])
print nss.indented_format([(2, 'Subject Public Key Info:')])
print nss.indented_format(cert.subject_public_key_info.format_lines(3))

if len(extensions) > 0:
    print nss.indented_format([(1, 'Signed Extensions: (%d)' % len(extensions))])
    for extension in extensions:
        print_extension(2, extension)

print nss.indented_format(cert.signed_data.format_lines(1))

print nss.indented_format([(1, 'Fingerprint (MD5):')])
print nss.indented_format(nss.make_line_pairs(2,
                                              nss.data_to_hex(nss.md5_digest(cert.der_data),
                                              nss.OCTETS_PER_LINE_DEFAULT)))

print nss.indented_format([(1, 'Fingerprint (SHA1):')])
print nss.indented_format(nss.make_line_pairs(2,
                                              nss.data_to_hex(nss.sha1_digest(cert.der_data),
                                              nss.OCTETS_PER_LINE_DEFAULT)))

