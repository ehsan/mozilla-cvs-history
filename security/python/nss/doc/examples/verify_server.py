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
import nss.ssl as ssl

# -----------------------------------------------------------------------------

# command line parameters, default them to something reasonable
password = ''
certdir = '/etc/httpd/alias'
#certdir = '/etc/pki/nssdb'
hostname = os.uname()[1]
nickname = hostname.split('.')[0]
port = 443

request = '''\
GET /index.html HTTP/1.0

'''
# -----------------------------------------------------------------------------
# Callback Functions
# -----------------------------------------------------------------------------

def cert_usage_str(cert_usage):
    usages = []

    if cert_usage & nss.certificateUsageSSLClient:
        cert_usage &= ~nss.certificateUsageSSLClient
        usages.append('SSLClient')

    if cert_usage & nss.certificateUsageSSLServer:
        cert_usage &= ~nss.certificateUsageSSLServer
        usages.append('SSLServer')

    if cert_usage & nss.certificateUsageSSLServerWithStepUp:
        cert_usage &= ~nss.certificateUsageSSLServerWithStepUp
        usages.append('SSLServerWithStepUp')

    if cert_usage & nss.certificateUsageSSLCA:
        cert_usage &= ~nss.certificateUsageSSLCA
        usages.append('SSLCA')

    if cert_usage & nss.certificateUsageEmailSigner:
        cert_usage &= ~nss.certificateUsageEmailSigner
        usages.append('EmailSigner')

    if cert_usage & nss.certificateUsageEmailRecipient:
        cert_usage &= ~nss.certificateUsageEmailRecipient
        usages.append('EmailRecipient')

    if cert_usage & nss.certificateUsageObjectSigner:
        cert_usage &= ~nss.certificateUsageObjectSigner
        usages.append('ObjectSigner')

    if cert_usage & nss.certificateUsageUserCertImport:
        cert_usage &= ~nss.certificateUsageUserCertImport
        usages.append('UserCertImport')

    if cert_usage & nss.certificateUsageVerifyCA:
        cert_usage &= ~nss.certificateUsageVerifyCA
        usages.append('VerifyCA')

    if cert_usage & nss.certificateUsageProtectedObjectSigner:
        cert_usage &= ~nss.certificateUsageProtectedObjectSigner
        usages.append('ProtectedObjectSigner')

    if cert_usage & nss.certificateUsageStatusResponder:
        cert_usage &= ~nss.certificateUsageStatusResponder
        usages.append('StatusResponder')

    if cert_usage & nss.certificateUsageAnyCA:
        cert_usage &= ~nss.certificateUsageAnyCA
        usages.append('AnyCA')


    usages.sort()
    usage_str = ','.join(usages)

    if cert_usage:
        usage_str += ' (plus unknown flags %#x)' % cert_usage

    return usage_str

def password_callback(slot, retry, password):
    if password: return password
    return getpass.getpass("Enter password: ");

def handshake_callback(sock):
    print "handshake complete, peer = %s" % (sock.get_peer_name())

def auth_certificate_callback(sock, check_sig, is_server, certdb):
    print "auth_certificate_callback: check_sig=%s is_server=%s" % (check_sig, is_server)
    cert_is_valid = False

    cert = sock.get_peer_certificate()
    pin_args = sock.get_pkcs11_pin_arg()
    if pin_args is None:
        pin_args = ()

    print "cert:\n%s" % cert

    # Define how the cert is being used based upon the is_server flag.  This may
    # seem backwards, but isn't. If we're a server we're trying to validate a
    # client cert. If we're a client we're trying to validate a server cert.
    if is_server:
        intended_usage = nss.certificateUsageSSLClient
    else:
        intended_usage = nss.certificateUsageSSLServer

    try:
        # If the cert fails validation it will raise an exception, the errno attribute
        # will be set to the error code matching the reason why the validation failed
        # and the strerror attribute will contain a string describing the reason.
        approved_usage = cert.verify_now(certdb, check_sig, intended_usage, *pin_args)
    except Exception, e:
        print e.strerror
        cert_is_valid = False
        print "Returning cert_is_valid = %s" % cert_is_valid
        return cert_is_valid

    print "approved_usage = %s" % cert_usage_str(approved_usage)

    # Is the intended usage a proper subset of the approved usage
    if approved_usage & intended_usage:
        cert_is_valid = True
    else:
        cert_is_valid = False

    # If this is a server, we're finished
    if is_server or not cert_is_valid:
        print "Returning cert_is_valid = %s" % cert_is_valid
        return cert_is_valid

    # Certificate is OK.  Since this is the client side of an SSL
    # connection, we need to verify that the name field in the cert
    # matches the desired hostname.  This is our defense against
    # man-in-the-middle attacks.

    hostname = sock.get_hostname()
    print "verifying socket hostname (%s) matches cert subject (%s)" % (hostname, cert.subject)
    try:
        # If the cert fails validation it will raise an exception
        cert_is_valid = cert.verify_hostname(hostname)
    except Exception, e:
        print e.strerror
        cert_is_valid = False
        print "Returning cert_is_valid = %s" % cert_is_valid
        return cert_is_valid

    print "Returning cert_is_valid = %s" % cert_is_valid
    return cert_is_valid

def client_auth_data_callback(ca_names, chosen_nickname, password, certdb):
    print "client_auth_data_callback"
    cert = None
    if chosen_nickname:
        try:
            cert = nss.find_cert_from_nickname(chosen_nickname, password)
            priv_key = nss.find_key_by_any_cert(cert, password)
            print "client cert:\n%s" % cert
            return cert, priv_key
        except NSPRError, e:
            print e
            return False
    else:
        nicknames = nss.get_cert_nicknames(certdb, cert.SEC_CERT_NICKNAMES_USER)
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

# -----------------------------------------------------------------------------
# Client Implementation
# -----------------------------------------------------------------------------

def client():
    # Get the IP Address of our server
    net_addr = io.NetworkAddress(hostname, port)
    sock = ssl.SSLSocket()
    # Set client SSL socket options
    sock.set_ssl_option(ssl.SSL_SECURITY, True)
    sock.set_ssl_option(ssl.SSL_HANDSHAKE_AS_CLIENT, True)
    sock.set_hostname(hostname)

    # Provide a callback which notifies us when the SSL handshake is
    # complete
    sock.set_handshake_callback(handshake_callback)

    # Provide a callback to supply our client certificate info
    sock.set_client_auth_data_callback(client_auth_data_callback, nickname,
                                       password, nss.get_default_certdb())

    # Provide a callback to verify the servers certificate
    sock.set_auth_certificate_callback(auth_certificate_callback,
                                       nss.get_default_certdb())

    print "client connecting to: %s" % (net_addr)
    sock.connect(net_addr)

    sock.reset_handshake(False) # FIXME: is this needed

    try:
        # Talk to the server
        n_received = 0
        sock.send(request)
        while True:
            buf = sock.recv(1024)
            n_received += len(buf)
            if not buf:
                print "\nclient lost connection, received %d bytes" % (n_received)
                break
    except Exception, e:
        print e.strerror
        sock.shutdown()
        return

    sock.shutdown()
    return

# -----------------------------------------------------------------------------

usage_str = '''
-d --certdir    certificate directory (default: %(certdir)s)
-h --hostname   host to connect to (default: %(hostname)s)
-n --nickname   certificate nickname (default: %(nickname)s)
-w --password   certificate database password (default: %(password)s)
-p --port       host port (default: %(port)s)
''' % {
       'certdir'             : certdir,
       'hostname'            : hostname,
       'nickname'            : nickname,
       'password'            : password,
       'port'                : port,
       }

def usage():
    print usage_str

try:
    opts, args = getopt.getopt(sys.argv[1:], "Hd:h:n:w:p:",
                               ["help", "certdir=", "hostname=",
                                "nickname=", "password=", "port=",
                                ])
except getopt.GetoptError:
    # print help information and exit:
    usage()
    sys.exit(2)


for o, a in opts:
    if o in ("-d", "--certdir"):
        certdir = a
    if o in ("-h", "--hostname"):
        hostname = a
    if o in ("-n", "--nickname"):
        nickname = a
    if o in ("-w", "--password"):
        password = a
    if o in ("-p", "--port"):
        port = int(a)
    if o in ("-H", "--help"):
        usage()
        sys.exit()

# Perform basic configuration and setup
try:
    nss.nss_init(certdir)
    ssl.set_domestic_policy()
    nss.set_password_callback(password_callback)
except Exception, e:
    print >>sys.stderr, e.strerror
    sys.exit(1)

client()

