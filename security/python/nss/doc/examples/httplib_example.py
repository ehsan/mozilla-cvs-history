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
import getopt
import urlparse
import httplib
import getpass

from nss.error import NSPRError
import nss.io as io
import nss.nss as nss
import nss.ssl as ssl

#------------------------------------------------------------------------------

debug_level = 0
certdir = 'pki'
password = ''
nickname = ''
url = 'https://sourceforge.net/projects/python'
use_ssl = False

#------------------------------------------------------------------------------
def password_callback(slot, retry, password):
    if password: return password
    return getpass.getpass("Enter password: ");

def handshake_callback(sock):
    print "handshake complete, peer = %s" % (sock.get_peer_name())

def client_auth_data_callback(ca_names, chosen_nickname, password, certdb):
    if debug_level: print "client_auth_data_callback: nickname=%s password=%s" % (chosen_nickname, password)
    cert = None
    if chosen_nickname:
        try:
            cert = nss.find_cert_from_nickname(chosen_nickname, password)
            priv_key = nss.find_key_by_any_cert(cert, password)
            if debug_level: print "client cert:\n%s" % cert
            return cert, priv_key
        except NSPRError, e:
            print e
            return False
    else:
        nicknames = nss.get_cert_nicknames(certdb, nss.SEC_CERT_NICKNAMES_USER)
        for nickname in nicknames:
            try:
                cert = nss.find_cert_from_nickname(nickname, password)
                print "client cert:\n%s" % cert
                if cert.check_valid_times():
                    if cert.has_signer_in_ca_names(ca_names):
                        priv_key = nss.find_key_by_any_cert(cert, password)
                        return cert, priv_key
            except NSPRError, e:
                print e
        return False
#------------------------------------------------------------------------------

opts, args = getopt.getopt(sys.argv[1:],
                           'Dd:n:w:',
                           ['debuglevel','certdir=','nickname=','password='])
for o, a in opts:
    if o in('-D', '--debug_level'):
        debug_level = debug_level + 1
    if o in ("-d", "--certdir"):
        certdir = a
    if o in ("-n", "--nickname"):
        nickname = a
    if o in ("-w", "--password"):
        password = a

if len(args) > 0:
    url = args[0]


# Perform basic configuration and setup

url_components = urlparse.urlsplit(url)
if url_components.scheme == 'https':
    use_ssl = True


if use_ssl:
    ssl.nssinit(certdir)
    ssl.set_domestic_policy()
    nss.set_password_callback(password_callback)
    h = httplib.HTTPS()
    h._conn.sock.set_handshake_callback(handshake_callback)

    # Provide a callback to supply our client certificate info
    h._conn.sock.set_client_auth_data_callback(client_auth_data_callback, nickname, password, nss.get_default_certdb())
else:
    h = httplib.HTTP()

h.set_debuglevel(debug_level)
h.connect(url_components.netloc)
h.putrequest('GET', url_components.path)
h.endheaders()
status, reason, headers = h.getreply()
print 'status =', status
print 'reason =', reason
# FIXME HTTPConnection.will_close invoked by getreply closes the connection, thus one can't read, why is this?
# print "read", len(h.read())
print
if headers:
    for header in headers.headers: print header.strip()
print
