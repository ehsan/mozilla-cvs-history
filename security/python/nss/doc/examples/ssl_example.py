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

import warnings
warnings.simplefilter( "always", DeprecationWarning)

import os
import sys
import getopt
import getpass

from nss.error import NSPRError
import nss.io as io
import nss.nss as nss
import nss.ssl as ssl

# -----------------------------------------------------------------------------
NO_CLIENT_CERT             = 0
REQUEST_CLIENT_CERT_ONCE   = 1
REQUIRE_CLIENT_CERT_ONCE   = 2
REQUEST_CLIENT_CERT_ALWAYS = 3
REQUIRE_CLIENT_CERT_ALWAYS = 4

# command line parameters, default them to something reasonable
client = False
server = False
password = 'passwd'
use_ssl = True
client_cert_action = NO_CLIENT_CERT
certdir = 'pki'
hostname = os.uname()[1]
server_nickname = 'test_server'
client_nickname = 'test_user'
port = 1234
timeout_secs = 3
family = io.PR_AF_UNSPEC

# -----------------------------------------------------------------------------
# Utility Functions
# -----------------------------------------------------------------------------


# -----------------------------------------------------------------------------
# Callback Functions
# -----------------------------------------------------------------------------

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

    print "approved_usage = %s" % ', '.join(nss.cert_usage_flags(approved_usage))

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

def Client():
    valid_addr = False
    # Get the IP Address of our server
    try:
        addr_info = io.AddrInfo(hostname)
    except Exception, e:
        print "could not resolve host address \"%s\"" % hostname
        return

    for net_addr in addr_info:
        if family != io.PR_AF_UNSPEC:
            if net_addr.family != family: continue
        net_addr.port = port

        if use_ssl:
            sock = ssl.SSLSocket(net_addr.family)

            # Set client SSL socket options
            sock.set_ssl_option(ssl.SSL_SECURITY, True)
            sock.set_ssl_option(ssl.SSL_HANDSHAKE_AS_CLIENT, True)
            sock.set_hostname(hostname)
            sock.reset_handshake(False) # FIXME: is this needed

            # Provide a callback which notifies us when the SSL handshake is complete
            sock.set_handshake_callback(handshake_callback)

            # Provide a callback to supply our client certificate info
            sock.set_client_auth_data_callback(client_auth_data_callback, client_nickname,
                                               password, nss.get_default_certdb())

            # Provide a callback to verify the servers certificate
            sock.set_auth_certificate_callback(auth_certificate_callback,
                                               nss.get_default_certdb())
        else:
            sock = io.Socket(net_addr.family)

        try:
            print "client trying connection to: %s" % (net_addr)
            sock.connect(net_addr, timeout=io.seconds_to_interval(timeout_secs))
            print "client connected to: %s" % (net_addr)
            valid_addr = True
            break
        except Exception, e:
            print "client connection to: %s failed (%s)" % (net_addr, e)

    if not valid_addr:
        print "Could not establish valid address for \"%s\" in family %s" % \
        (hostname, io.addr_family_name(family))
        return
    try:
        print "client connected to: %s" % sock.get_peer_name()
    except Exception, e:
        print e.strerror
        try:
            sock.shutdown()
        except:
            pass
        return
        

    # Talk to the server
    try:
        sock.send("Hello")
        buf = sock.recv(1024)
        if not buf:
            print "client lost connection"
            return
        print "client received: %s" % (buf)
    except Exception, e:
        print e.strerror
        try:
            sock.shutdown()
        except:
            pass
        return

    # End of (simple) protocol session?
    if buf == 'Goodbye':
        try:
            sock.shutdown()
        except:
            pass
        return

# -----------------------------------------------------------------------------
# Server Implementation
# -----------------------------------------------------------------------------

def Server():
    global family

    # Perform basic SSL server configuration
    ssl.set_default_cipher_pref(ssl.SSL_RSA_WITH_NULL_MD5, True)
    ssl.config_server_session_id_cache()

    # Get our certificate and private key
    server_cert = nss.find_cert_from_nickname(server_nickname, password)
    priv_key = nss.find_key_by_any_cert(server_cert, password)
    server_cert_kea = server_cert.find_kea_type();

    print "server cert:\n%s" % server_cert

    # Setup an IP Address to listen on any of our interfaces
    if family == io.PR_AF_UNSPEC:
        family = io.PR_AF_INET
    net_addr = io.NetworkAddress(io.PR_IpAddrAny, port, family)

    if use_ssl:
        sock = ssl.SSLSocket(net_addr.family)

        # Set server SSL socket options
        sock.set_pkcs11_pin_arg(password)
        sock.set_ssl_option(ssl.SSL_SECURITY, True)
        sock.set_ssl_option(ssl.SSL_HANDSHAKE_AS_SERVER, True)

        # If we're doing client authentication then set it up
        if client_cert_action >= REQUEST_CLIENT_CERT_ONCE:
            sock.set_ssl_option(ssl.SSL_REQUEST_CERTIFICATE, True)
        if client_cert_action == REQUIRE_CLIENT_CERT_ONCE:
            sock.set_ssl_option(ssl.SSL_REQUIRE_CERTIFICATE, True)
        sock.set_auth_certificate_callback(auth_certificate_callback, nss.get_default_certdb())

        # Configure the server SSL socket
        sock.config_secure_server(server_cert, priv_key, server_cert_kea)
        sock.reset_handshake(True) # FIXME: is this needed?

    else:
        sock = io.Socket(net_addr.family)

    # Bind to our network address and listen for clients
    sock.bind(net_addr)
    print "listening on: %s" % (net_addr)
    sock.listen()

    while True:
        # Accept a connection from a client
        client_sock, client_addr = sock.accept()
        if use_ssl:
            client_sock.set_handshake_callback(handshake_callback)

        print "client connect from: %s" % (client_addr)

        while True:
            try:
                # Handle the client connection
                buf = client_sock.recv(1024)
                if not buf:
                    print "server lost lost connection to %s" % (client_addr)
                    break

                print "server received: %s" % (buf)

                client_sock.send("Goodbye")
                try:
                    client_sock.shutdown(io.PR_SHUTDOWN_RCV)
                except:
                    pass
                break
            except Exception, e:
                print e.strerror
                break
        break

    try:
        sock.shutdown()
    except:
        pass

# -----------------------------------------------------------------------------

usage_str = '''
-C --client     run as the client (default: %(client)s)
-S --server     run as the server (default: %(server)s)
-d --certdir    certificate directory (default: %(certdir)s)
-h --hostname   host to connect to (default: %(hostname)s)
-f --family     may be inet|inet6|unspec (default: %(family)s)
                if unspec client tries all addresses returned by AddrInfo
                          server binds to IPv4 "any" wildcard address
                if inet   client tries IPv4 addresses returned by AddrInfo
                          server binds to IPv4 "any" wildcard address
                if inet6  client tries IPv6 addresses returned by AddrInfo
                          server binds to IPv6 "any" wildcard address
-4 --inet       set family to inet (see family)
-6 --inet6      set family to inet6 (see family)
-n --server_nickname server certificate nickname (default: %(server_nickname)s)
-N --client_nickname client certificate nickname (default: %(client_nickname)s)
-w --password   certificate database password (default: %(password)s)
-p --port       host port (default: %(port)s)
-e --encrypt    use SSL (default) (default: %(encrypt)s)
-E --noencrypt  don't use SSL (default: %(noencrypt)s)
-f --require_cert_once (default: %(require_cert_once)s)
-F --require_cert_always (default: %(require_cert_always)s)
-r --request_cert_once (default: %(request_cert_once)s)
-R --request_cert_always (default: %(request_cert_always)s)
-H --help
''' % {
       'client'              : client,
       'server'              : server,
       'certdir'             : certdir,
       'hostname'            : hostname,
       'family'              : io.addr_family_name(family),
       'server_nickname'     : server_nickname,
       'client_nickname'     : client_nickname,
       'password'            : password,
       'port'                : port,
       'encrypt'             : use_ssl is True,
       'noencrypt'           : use_ssl is False,
       'require_cert_once'   : client_cert_action == REQUIRE_CLIENT_CERT_ONCE,
       'require_cert_always' : client_cert_action == REQUIRE_CLIENT_CERT_ALWAYS,
       'request_cert_once'   : client_cert_action == REQUEST_CLIENT_CERT_ONCE,
       'request_cert_always' : client_cert_action == REQUEST_CLIENT_CERT_ALWAYS,
       }

def usage():
    print usage_str

try:
    opts, args = getopt.getopt(sys.argv[1:], "Hd:h:f:46n:N:w:p:CSeE",
                               ["help", "certdir=", "hostname=",
                                "family", "inet", "inet6",
                                "server_nickname=", "client_nickname=",
                                "password=", "port=",
                                "client", "server", "encrypt", "noencrypt",
                                "require_cert_once", "require_cert_always",
                                "request_cert_once", "request_cert_always",
                                ])
except getopt.GetoptError:
    # print help information and exit:
    usage()
    sys.exit(2)


for o, a in opts:
    if o in ("-d", "--certdir"):
        certdir = a
    elif o in ("-h", "--hostname"):
        hostname = a
    elif o in ("-f", "--family"):
        if a == "inet":
            family = io.PR_AF_INET
        elif a == "inet6":
            family = io.PR_AF_INET6
        elif a == "unspec":
            family = io.PR_AF_UNSPEC
        else:
            print "unknown address family (%s)" % (a)
            usage()
            sys.exit()
    elif o in ("-4", "--inet"):
        family = io.PR_AF_INET
    elif o in ("-6", "--inet6"):
        family = io.PR_AF_INET6
    elif o in ("-n", "--server_nickname"):
        server_nickname = a
    elif o in ("-N", "--client_nickname"):
        client_nickname = a
    elif o in ("-w", "--password"):
        password = a
    elif o in ("-p", "--port"):
        port = int(a)
    elif o in ("-C", "--client"):
        client = True
    elif o in ("-S", "--server"):
        server = True
    elif o in ("-e", "--encrypt"):
        use_ssl = True
    elif o in ("-E", "--noencrypt"):
        use_ssl = False
    elif o in ("--require_cert_once"):
        client_cert_action = REQUIRE_CLIENT_CERT_ONCE
    elif o in ("--require_cert_always"):
        client_cert_action = REQUIRE_CLIENT_CERT_ALWAYS
    elif o in ("--request_cert_once"):
        client_cert_action = REQUEST_CLIENT_CERT_ONCE
    elif o in ("--request_cert_always"):
        client_cert_action = REQUEST_CLIENT_CERT_ALWAYS
    elif o in ("-H", "--help"):
        usage()
        sys.exit()
    else:
        usage()
        sys.exit()

if client and server:
    print "can't be both client and server"
    sys.exit(1)
if not (client or server):
    print "must be one of client or server"
    sys.exit(1)

# Perform basic configuration and setup
if certdir is None:
    nss.nss_init_nodb()
else:
    nss.nss_init(certdir)

ssl.set_domestic_policy()
nss.set_password_callback(password_callback)

# Run as a client or as a server
if client:
    print "starting as client"
    Client()

if server:
    print "starting as server"
    Server()

