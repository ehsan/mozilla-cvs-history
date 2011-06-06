#!/usr/bin/python

import os
import sys
import errno
import signal
import time

import unittest

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

verbose = False
info = False
password = 'db_passwd'
use_ssl = True
client_cert_action = NO_CLIENT_CERT
certdir = os.path.join(os.path.dirname(sys.argv[0]), 'pki')
hostname = os.uname()[1]
server_nickname = 'test_server'
client_nickname = 'test_user'
port = 1234
timeout_secs = 3
family = io.PR_AF_INET
sleep_time = 1


# -----------------------------------------------------------------------------
# Callback Functions
# -----------------------------------------------------------------------------

def password_callback(slot, retry, password):
    if password: return password
    return getpass.getpass("Enter password: ");

def handshake_callback(sock):
    if verbose: print "handshake complete, peer = %s" % (sock.get_peer_name())

def auth_certificate_callback(sock, check_sig, is_server, certdb):
    if verbose: print "auth_certificate_callback: check_sig=%s is_server=%s" % (check_sig, is_server)
    cert_is_valid = False

    cert = sock.get_peer_certificate()
    pin_args = sock.get_pkcs11_pin_arg()
    if pin_args is None:
        pin_args = ()

    #if verbose: print "cert:\n%s" % cert

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
        print >>sys.stderr, "auth_certificate_callback: %s" % e
        cert_is_valid = False
        if verbose: print "Returning cert_is_valid = %s" % cert_is_valid
        return cert_is_valid

    if verbose: print "approved_usage = %s" % ', '.join(nss.cert_usage_flags(approved_usage))

    # Is the intended usage a proper subset of the approved usage
    if approved_usage & intended_usage:
        cert_is_valid = True
    else:
        cert_is_valid = False

    # If this is a server, we're finished
    if is_server or not cert_is_valid:
        if verbose: print "Returning cert_is_valid = %s" % cert_is_valid
        return cert_is_valid

    # Certificate is OK.  Since this is the client side of an SSL
    # connection, we need to verify that the name field in the cert
    # matches the desired hostname.  This is our defense against
    # man-in-the-middle attacks.

    hostname = sock.get_hostname()
    if verbose: print "verifying socket hostname (%s) matches cert subject (%s)" % (hostname, cert.subject)
    try:
        # If the cert fails validation it will raise an exception
        cert_is_valid = cert.verify_hostname(hostname)
    except Exception, e:
        print >>sys.stderr, "auth_certificate_callback: %s" % e
        cert_is_valid = False
        if verbose: print "Returning cert_is_valid = %s" % cert_is_valid
        return cert_is_valid

    if verbose: print "Returning cert_is_valid = %s" % cert_is_valid
    return cert_is_valid

def client_auth_data_callback(ca_names, chosen_nickname, password, certdb):
    cert = None
    if chosen_nickname:
        try:
            cert = nss.find_cert_from_nickname(chosen_nickname, password)
            priv_key = nss.find_key_by_any_cert(cert, password)
            if verbose: print "client cert:\n%s" % cert
            return cert, priv_key
        except NSPRError, e:
            print >>sys.stderr, "client_auth_data_callback: %s" % e
            return False
    else:
        nicknames = nss.get_cert_nicknames(certdb, cert.SEC_CERT_NICKNAMES_USER)
        for nickname in nicknames:
            try:
                cert = nss.find_cert_from_nickname(nickname, password)
                if verbose: print "client cert:\n%s" % cert
                if cert.check_valid_times():
                    if cert.has_signer_in_ca_names(ca_names):
                        priv_key = nss.find_key_by_any_cert(cert, password)
                        return cert, priv_key
            except NSPRError, e:
                print >>sys.stderr, "client_auth_data_callback: %s" % e
        return False

# -----------------------------------------------------------------------------
# Client Implementation
# -----------------------------------------------------------------------------

def client(request):
    if use_ssl:
        if info: print "client: using SSL"
        ssl.set_domestic_policy()

    valid_addr = False
    # Get the IP Address of our server
    try:
        addr_info = io.AddrInfo(hostname)
    except Exception, e:
        print >>sys.stderr, "client: could not resolve host address \"%s\"" % hostname
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
            if verbose: print "client trying connection to: %s" % (net_addr)
            sock.connect(net_addr, timeout=io.seconds_to_interval(timeout_secs))
            if verbose: print "client connected to: %s" % (net_addr)
            valid_addr = True
            break
        except Exception, e:
            sock.close()
            print >>sys.stderr, "client: connection to: %s failed (%s)" % (net_addr, e)

    if not valid_addr:
        print >>sys.stderr, "Could not establish valid address for \"%s\" in family %s" % \
        (hostname, io.addr_family_name(family))
        return

    # Talk to the server
    try:
        if info: print "client: sending \"%s\"" % (request)
        sock.send(request)
        buf = sock.recv(1024)
        if not buf:
            print >>sys.stderr, "client: lost connection"
            sock.close()
            return
        if info: print "client: received \"%s\"" % (buf)
    except Exception, e:
        print >>sys.stderr, "client: %s" % e
        try:
            sock.close()
        except:
            pass
        return

    try:
        sock.shutdown()
    except Exception, e:
        print >>sys.stderr, "client: %s" % e

    try:
        sock.close()
        if use_ssl:
            ssl.clear_session_cache()
    except Exception, e:
        print >>sys.stderr, "client: %s" % e

    return buf

# -----------------------------------------------------------------------------
# Server Implementation
# -----------------------------------------------------------------------------

def server():
    global family

    if verbose: print "starting server:"

    # Initialize
    # Setup an IP Address to listen on any of our interfaces
    if family == io.PR_AF_UNSPEC:
        family = io.PR_AF_INET
    net_addr = io.NetworkAddress(io.PR_IpAddrAny, port, family)

    if use_ssl:
        if info: print "server: using SSL"
        ssl.set_domestic_policy()
        nss.set_password_callback(password_callback)

        # Perform basic SSL server configuration
        ssl.set_default_cipher_pref(ssl.SSL_RSA_WITH_NULL_MD5, True)
        ssl.config_server_session_id_cache()

        # Get our certificate and private key
        server_cert = nss.find_cert_from_nickname(server_nickname, password)
        priv_key = nss.find_key_by_any_cert(server_cert, password)
        server_cert_kea = server_cert.find_kea_type();

        #if verbose: print "server cert:\n%s" % server_cert

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

    else:
        sock = io.Socket(net_addr.family)

    # Bind to our network address and listen for clients
    sock.bind(net_addr)
    if verbose: print "listening on: %s" % (net_addr)
    sock.listen()

    while True:
        # Accept a connection from a client
        client_sock, client_addr = sock.accept()
        if use_ssl:
            client_sock.set_handshake_callback(handshake_callback)

        if verbose: print "client connect from: %s" % (client_addr)

        while True:
            try:
                # Handle the client connection
                buf = client_sock.recv(1024)
                if not buf:
                    print >>sys.stderr, "server: lost lost connection to %s" % (client_addr)
                    break

                if info: print "server: received \"%s\"" % (buf)
                reply = "{%s}" % buf # echo
                if info: print "server: sending \"%s\"" % (reply)
                client_sock.send(reply) # echo

                time.sleep(sleep_time)
                client_sock.shutdown()
                client_sock.close()
                break
            except Exception, e:
                print >>sys.stderr, "server: %s" % e
                break
        break

    # Clean up
    sock.shutdown()
    sock.close()
    if use_ssl:
        ssl.shutdown_server_session_id_cache()

# -----------------------------------------------------------------------------

def run_server():
    pid = os.fork()
    if pid == 0:
        nss.nss_init(certdir)
        server()
        nss.nss_shutdown()
    time.sleep(sleep_time)
    return pid

def cleanup_server(pid):
    try:
        wait_pid, wait_status = os.waitpid(pid, os.WNOHANG)
        if wait_pid == 0:
            os.kill(pid, signal.SIGKILL)
    except OSError, e:
        if e.errno == errno.ECHILD:
            pass                # child already exited
        else:
            print >>sys.stderr, "cleanup_server: %s" % e
    
class TestSSL(unittest.TestCase):

    def setUp(self):
        print
        self.server_pid = run_server()

    def tearDown(self):
        cleanup_server(self.server_pid)

    def test_ssl(self):
        request = "foo"
        nss.nss_init(certdir)
        reply = client(request)
        nss.nss_shutdown()
        self.assertEqual("{%s}" % request, reply)
        

if __name__ == '__main__':
    unittest.main()
