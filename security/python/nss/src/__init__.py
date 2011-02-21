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
"""
============
Introduction
============

This package provides a binding for the Network Security Services
(NSS) library. Because NSS directly uses the Netscape Portable Runtime
(NSPR) the binding also provides support for NSPR. There is an
inherent conflict between NSPR and Python, please see the Issues
section for more detail.

General documentation on NSS can be found here:

http://www.mozilla.org/projects/security/pki/nss

General documentation on NSPR can be found here:

http://developer.mozilla.org/en/docs/NSPR_API_Reference

Please note, the documentation included with this package already
encapsultes most of the information at the above two URL's, but is
specific to the python binding of NSS/NSPR. It is suggested you refer
to the python-nss documentation.

Most of the names and symbols in the NSS/NSPR C API have been kept in
the nss-python binding and should be instantly familar or
recognizable. Python has different naming conventions and the
nss-python binding has adhered to the python naming convensions,
Classes are camel case, otherwise symbols are all lower case with
words seperated by underscores. The constants used by NSS/NSPR in C
API have been imported literally to add the programmer who might be
referring to the Mozilla NSS/NSPR documentation and/or header files or
who is porting an existing C application to python. Minor other
changes have been made in the interest of being "Pythonic".

========================
Deprecated Functionality
========================

Some elements of the binding have been deprecated because of lessons
learned along the way. The following emit deprecation warnings and
should not be used, they will be removed in a subsequent release.

`io.NetworkAddress()`
    `NetworkAddress` initialization from a string parameter only works
    for IPv4, use `AddrInfo` instead.

`io.NetworkAddress.set_from_string()`
    `NetworkAddress` initialization from a string parameter only works
    for IPv4, use `AddrInfo` instead.

`io.NetworkAddress.hostentry`
    `HostEntry` objects only support IPv4, this property will be
    removed, use `AddrInfo` instead.

`io.HostEntry.get_network_addresses()`
    Use iteration instead (e.g. for net_adder in hostentry), the port
    parameter is not respected, port will be value when `HostEntry`
    object was created.

`io.HostEntry.get_network_address()`
    Use indexing instead (e.g. hostentry[i]), the port parameter is
    not respected, port will be value when `HostEntry` object was
    created.

`ssl.nssinit()`
    nssinit has been moved to the nss module, use `nss.nss_init()`
    instead of ssl.nssinit

`ssl.nss_init()`
    nss_init has been moved to the nss module, use `nss.nss_init()`
    instead of ssl.nssinit

`ssl.nss_shutdown()`
    nss_shutdown() has been moved to the nss module, use
    `nss.nss_shutdown()` instead of ssl.nss_shutdown()

===============
Getting Started
===============

NSS stores it's certificates and private keys in a security database
unlike OpenSSL which references it's certificates and keys via file
pathnames. This means unless you already have an NSS Certificate
Database (CertDB) the first order of business will be to create
one. When a NSS application initializes itself it will need to specify
the path to the CertDB (see "Things All NSS programs must do").

The CertDB is created and manipulated by the command line utilities
certutil and modutil. Both of these programs are part of the nss-tools
RPM. Documentation for these tools can be found here:
http://www.mozilla.org/projects/security/pki/nss/tools

Here is an example of creating a CertDB and populating it. In the
example the CertDB will be created under the directory "./pki", the CA
will be called "myca", the database password will be "myca", and the
server's hostname will be "myhost.example.com".

1. Create the database::

     certutil -N -d ./pki

   This creates a new database under the directory ./pki

2. Create a root CA certificate::

     certutil -d ./pki -S -s "CN=myca" -n myca -x -t "CTu,C,C" -m 1

   This creates an individual certificate and adds it to the
   certificate database with a subject of "CN=myca", a nickname of
   "myca", trust flags indicating for SSL indicating it can issue
   server certificates (C), can issue client certificates (T), and the
   certificate can be used for authentication and signing (u). For
   email and object signing it's trusted to create server
   certificates. The certificate serial number is set to 1.


3. Create a server certificate and sign it. Our example server will
   use this::

     certutil -d pki -S -c myca -s "CN=myhost.example.com" -n myhost -t "u,u,u" -m 2

   This creates an individual certificate issued by the CA "myca" and
   adds it to the certificate database with a subject of
   "CN=myhost.example.com", a nickname of "myhost". The certificate
   serial number is set to 2.

4. Import public root CA's::

     modutil -add ca_certs -libfile /usr/lib/libnssckbi.so -dbdir ./pki

   This is necessary to verify certificates presented by a SSL server a
   NSS client might connect to. When verifying a certificate the NSS
   library will "walk the certificate chain" back to a root CA which
   must be trusted. This command imports the well known root CA's as a
   PKCS #11 module.


===============================
Things All NSS programs must do
===============================

- Import the NSS/NSPR modules::

    from nss.error import NSPRError
    import nss.io as io
    import nss.nss as nss
    import nss.ssl as ssl

  In the interest of code brevity we drop the leading "nss." from the
  module namespace.

- Initialize NSS and indicate the certficate database (CertDB)::

    certdir = './pki'
    ssl.nssinit(certdir)

- If you are implementing an SSL server call config_secure_server()
  (see ssl_example.py)::

    sock = ssl.SSLSocket()
    sock.config_secure_server(server_cert, priv_key, server_cert_kea)

  **WARNING** you must call config_secure_server() for SSL servers, if
  you do not call it the most likely result will be the NSS library
  will segfault (not pretty).

========
Examples
========

There are example programs in under "examples" in the documentation
directory. On Fedora/RHEL/CentOS systems this will be
/usr/share/doc/python-nss.

The ssl_example.py sample implements both a client and server in one
script. You tell it whether to run as a client (-C) or a server (-S)
when you invoke it. The sample shows many of the NSS/NSPR calls and
fully implements basic non-SSL client/server using NSPR, SSL
client/server using NSS, certificate validation, CertDB operations,
and client authentication using certificates.

To get a list of command line options::

  ssl_example.py --help

Using the above example certificate database server can be run like
this::

  ssl_example.py -S -c ./pki -n myhost

The client can be run like this::

  ssl_example.py -C -c ./pki

======
Issues
======

- The current partitioning of the NSS and NSPR API's into Python
  modules (i.e. the Python namespaces and their symbols) is a first
  cut and may not be ideal. One should be prepared for name changes as
  the binding matures.

- NSPR vs. Python

    An original design goal of NSS was to be portable, however NSS
    required access to many system level functions which can vary
    widely between platforms and OS's. Therefore NSPR was written to
    encapsulate system services such as IO, sockets, threads, timers,
    etc. into a common API to insulate NSS from the underlying
    platform.

    In many respects Python and its collection of packages and modules
    provides the same type of platform independence for applications
    and libraries and provides it's own implementation of IO, sockets,
    threads, timers, etc.

    Unfortunately NSPR's and Python's run time abstractions are not
    the same nor can either be configured to use a different
    underlying abstraction layer.

    Currently the NSS binding utilizes *only* the NSPR abstraction
    layer. One consequence of this is it is not possible to create a
    Python socket and use it as the foundation for any NSS functions
    expecting a socket, or visa versa.

    You **must** use the nss.io module to create and manipulate a
    socket used by NSS. You cannot pass this socket to any Python
    library function expecting a socket. The two are not compatible.

    Here are some reasons for this incompatibility, perhaps in the
    future we can find a solution but the immediate goal of the NSS
    Python binding was to expose NSS through Python, not necessarily
    to solve the larger integration issue of Python run-time and NSPR
    run-time. 

    - NSPR would like to hide the underlying platform socket (in the
      NSPR code this is called "osfd"). There are NSPR API's which
      will operate on osfd's

      - One can base a NSPR socket on an existing osfd via:

        - PR_ImportFile()
        - PR_ImportPipe()
        - PR_ImportTCPSocket()
        - PR_ImportUDPSocket()

      - One can obtain the osfd in use by NSPR, either when the
        osfd was imported or because NSPR created the osfd itself via:

	- PR_FileDesc2NativeHandle();

        But note this function is not meant to be public in the NSPR
        API and is documented as being deprecated and carries an
        explicit warning against it's use.

      Once NSPR gets a hold of an osfd it manipulates it in a manner
      as if it were the only owner of the osfd. Other native code
      (e.g. the CPython socket code) which operates on the fd may run
      afoul of NSPR belief it is the only code in the system operating
      on the fd. For example in CPython the non-blocking flag is
      directly set on the fd and non-blocking behavior is implemented
      by the OS. However, NSPR manages non-blocking behavior
      internally to the NSPR library eschewing direct OS support for
      non-blocking. Thus CPython and NSPR are in direct conflict over
      when and how non-blocking is set on an fd. Examples of this
      problem can be seen in the Python socket.makefile() operation
      which takes the fd belonging to a system socket, dups it, and
      calls fdopen() on the dup'ed fd to return a FILE stream (all
      Python file IO is based on file objects utilizing a FILE
      stream). However, the dup'ed fd does not share the same
      non-blocking flag, NSPR explicitly forces the flag off, Python
      wants to directly manipulate it. Dup'ed fd's share their flags
      thus if Python operates on the dup'ed fd returned by NSPR it's
      going to confuse NSPR. Likewise if one sets non-blocking via
      NSPR then Python won't honor the flag because Python is
      expecting the flag to be set on the fd, not in some other
      location (e.g. internal to NSPR).

    - Python's socket implementation is a very thin layer over the
      Berkely socket API. There is very little abstraction, thus
      Python and Python program expect to manipulate sockets directly
      via their fd's.

    - The error and exception model for Python sockets and SSL is an
      almost direct one-to-one mapping of the Posix and OpenSSL
      errors. But NSS uses NSPR errors, thus Python code which has
      exception handlers for sockets and SSL are expecting a complete
      different set of exceptions.

    - Python's SSL implementation is a very thin layer over the
      OpenSSL API, there is little abstraction. Thus there is a
      sizeable body of Python code which expects the OpenSSL model for
      IO ready and has exception handlers based on OpenSSL.

===
FAQ
===

To be added

"""
__version__ = '0.11'

