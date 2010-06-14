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
import logging

from nss.error import NSPRError
import nss.io as io
import nss.nss as nss
import nss.ssl as ssl

#------------------------------------------------------------------------------

httplib_debug_level = 0
logging_debug_level = logging.INFO
certdir = 'pki'
password = ''
nickname = ''
url = 'https://sourceforge.net/projects/python'
use_ssl = True
use_connection_class = True

#------------------------------------------------------------------------------

def auth_certificate_callback(sock, check_sig, is_server, certdb):
    cert_is_valid = False

    cert = sock.get_peer_certificate()

    logging.debug("auth_certificate_callback: check_sig=%s is_server=%s\n%s",
                  check_sig, is_server, str(cert))

    pin_args = sock.get_pkcs11_pin_arg()
    if pin_args is None:
        pin_args = ()

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
        logging.error('cert validation failed for "%s" (%s)', cert.subject, e.strerror)
        cert_is_valid = False
        return cert_is_valid

    logging.debug("approved_usage = %s intended_usage = %s",
                  ', '.join(nss.cert_usage_flags(approved_usage)),
                  ', '.join(nss.cert_usage_flags(intended_usage)))

    # Is the intended usage a proper subset of the approved usage
    if approved_usage & intended_usage:
        cert_is_valid = True
    else:
        cert_is_valid = False

    # If this is a server, we're finished
    if is_server or not cert_is_valid:
        logging.debug('cert valid %s for "%s"', cert_is_valid,  cert.subject)
        return cert_is_valid

    # Certificate is OK.  Since this is the client side of an SSL
    # connection, we need to verify that the name field in the cert
    # matches the desired hostname.  This is our defense against
    # man-in-the-middle attacks.

    hostname = sock.get_hostname()
    try:
        # If the cert fails validation it will raise an exception
        cert_is_valid = cert.verify_hostname(hostname)
    except Exception, e:
        logging.error('failed verifying socket hostname "%s" matches cert subject "%s" (%s)',
                      hostname, cert.subject, e.strerror)
        cert_is_valid = False
        return cert_is_valid

    logging.debug('cert valid %s for "%s"', cert_is_valid,  cert.subject)
    return cert_is_valid

def client_auth_data_callback(ca_names, chosen_nickname, password, certdb):
    cert = None
    if chosen_nickname:
        try:
            cert = nss.find_cert_from_nickname(chosen_nickname, password)
            priv_key = nss.find_key_by_any_cert(cert, password)
            return cert, priv_key
        except NSPRError:
            return False
    else:
        nicknames = nss.get_cert_nicknames(certdb, nss.SEC_CERT_NICKNAMES_USER)
        for nickname in nicknames:
            try:
                cert = nss.find_cert_from_nickname(nickname, password)
                if cert.check_valid_times():
                    if cert.has_signer_in_ca_names(ca_names):
                        priv_key = nss.find_key_by_any_cert(cert, password)
                        return cert, priv_key
            except NSPRError:
                return False
        return False

def password_callback(slot, retry, password):
    if not retry and password: return password
    return getpass.getpass("Enter password for %s: " % slot.token_name);

def handshake_callback(sock):
    """
    Verify callback. If we get here then the certificate is ok.
    """
    logging.debug("handshake complete, peer = %s", sock.get_peer_name())
    pass

class NSSConnection(httplib.HTTPConnection):
    default_port = httplib.HTTPSConnection.default_port

    def __init__(self, host, port=None, strict=None, dbdir=None):
        httplib.HTTPConnection.__init__(self, host, port, strict)

        if not dbdir:
            raise RuntimeError("dbdir is required")

        logging.debug('%s init %s', self.__class__.__name__, host)
        nss.nss_init(dbdir)
        ssl.set_domestic_policy()
        nss.set_password_callback(password_callback)

        # Create the socket here so we can do things like let the caller
        # override the NSS callbacks
        self.sock = ssl.SSLSocket()
        self.sock.set_ssl_option(ssl.SSL_SECURITY, True)
        self.sock.set_ssl_option(ssl.SSL_HANDSHAKE_AS_CLIENT, True)

        # Provide a callback which notifies us when the SSL handshake is complete
        self.sock.set_handshake_callback(handshake_callback)

        # Provide a callback to verify the servers certificate
        self.sock.set_auth_certificate_callback(auth_certificate_callback,
                                           nss.get_default_certdb())

    def connect(self):
        logging.debug("connect: host=%s port=%s", self.host, self.port)
        self.sock.set_hostname(self.host)
        net_addr = io.NetworkAddress(self.host, self.port)
        logging.debug("connect: %s", net_addr)
        self.sock.connect(net_addr)

class NSPRConnection(httplib.HTTPConnection):
    default_port = httplib.HTTPConnection.default_port

    def __init__(self, host, port=None, strict=None):
        httplib.HTTPConnection.__init__(self, host, port, strict)

        logging.debug('%s init %s', self.__class__.__name__, host)
        nss.nss_init_nodb()

        self.sock = io.Socket()
    def connect(self):
        logging.debug("connect: host=%s port=%s", self.host, self.port)
        net_addr = io.NetworkAddress(self.host, self.port)
        logging.debug("connect: %s", net_addr)
        self.sock.connect(net_addr)

class NSSHTTPS(httplib.HTTP):
    _http_vsn = 11
    _http_vsn_str = 'HTTP/1.1'

    _connection_class = NSSConnection

    def __init__(self, host='', port=None, strict=None, dbdir=None):
        # provide a default host, pass the X509 cert info

        # urf. compensate for bad input.
        if port == 0:
            port = None
        self._setup(self._connection_class(host, port, strict, dbdir=dbdir))

class NSPRHTTP(httplib.HTTP):
    _http_vsn = 11
    _http_vsn_str = 'HTTP/1.1'

    _connection_class = NSPRConnection

#------------------------------------------------------------------------------


opts, args = getopt.getopt(sys.argv[1:],
                           'Dd:n:w:sScC',
                           ['debuglevel','certdir=','nickname=','password=',
                            'use-ssl', 'no-ssl', 'use-connection-class', 'no-connection-class'])
for o, a in opts:
    if o in('-D', '--httplib_debug_level'):
        httplib_debug_level = httplib_debug_level + 1
    elif o in ("-d", "--certdir"):
        certdir = a
    elif o in ("-n", "--nickname"):
        nickname = a
    elif o in ("-w", "--password"):
        password = a
    elif o in ("-s", "--use-ssl"):
        use_ssl = True
    elif o in ("-S", "--no-ssl"):
        use_ssl = False
    elif o in ("-c", "--use-connection-class"):
        use_connection_class = True
    elif o in ("-C", "--no-connection-class"):
        use_connection_class = False

if len(args) > 0:
    url = args[0]


if httplib_debug_level > 0:
    logging_debug_level = logging.DEBUG
else:
    logging_debug_level = logging.INFO

logging.basicConfig(level=logging_debug_level,
                    format='%(asctime)s %(levelname)-8s %(message)s',
                    datefmt='%m-%d %H:%M')

# Perform basic configuration and setup

url_components = urlparse.urlsplit(url)
if use_ssl:
    url_components.schema = 'https'
else:
    url_components.schema = 'http'

url = urlparse.urlunsplit(url_components)
url_components = urlparse.urlsplit(url)

if use_connection_class:
    if use_ssl:
        logging.info("Start (using NSSConnection class) %s", url)
        conn = NSSConnection(url_components.netloc, 443, dbdir="/etc/pki/nssdb")
    else:
        logging.info("Start (using NSPRConnection class) %s", url)
        conn = NSPRConnection(url_components.netloc, 80)
    conn.set_debuglevel(httplib_debug_level)
    conn.connect()
    conn.request("GET", "/")
    response = conn.getresponse()
    print "status = %s %s" % (response.status, response.reason)
    headers = response.getheaders()
    print "headers:"
    for header in headers:
        print "%s: %s" % (header[0], header[1])
    content_length = int(response.getheader('content-length'))
    data = response.read()
    assert(content_length == len(data))
    print data
    conn.close()
else:
    if use_ssl:
        logging.info("Start (using NSSHTTPS class) %s", url)
        h = NSSHTTPS(url_components.netloc, 443, dbdir="/etc/pki/nssdb")
    else:
        logging.info("Start (using NSPRHTTP class) %s", url)
        h = NSPRHTTP(url_components.netloc, 80)
    h.set_debuglevel(httplib_debug_level)
    h.connect()
    h.putrequest('GET', '/')
    h.endheaders()
    http_status, http_reason, headers = h.getreply()
    print "status = %s %s" % (http_status, http_reason)
    print "headers:\n%s" % headers
    content_length = int(headers['content-length'])
    f = h.getfile()
    data = f.read() # Get the raw HTML
    assert(content_length == len(data))
    f.close()
    print data
