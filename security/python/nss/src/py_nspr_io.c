/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is a Python binding for Network Security Services (NSS).
 *
 * The Initial Developer of the Original Code is Red Hat, Inc.
 *   (Author: John Dennis <jdennis@redhat.com>)
 *
 * Portions created by the Initial Developer are Copyright (C) 2008,2009
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above.  If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

// FIXME: add detail to each set_nspr_error()
// FIXME: should nspr exception be derived from IOError? Note our detail is IOError's filename
// FIXME: add factory functions for TCPSocket, UDPSocket
// FIXME: change where class initializers appear in the file, should be first so class methods can use them
//        or just add prototypes for everything (maybe better solution).

#define PY_SSIZE_T_CLEAN
#include "Python.h"
#include "structmember.h"

#include "py_nspr_common.h"
#define NSS_IO_MODULE
#include "py_nspr_io.h"
#include "py_nspr_error.h"

static char *unset_string = "<unset>";

#define Py_RETURN_BOOL(condition) {if (condition) Py_RETURN_TRUE; else Py_RETURN_FALSE;}

/* ========================================================================== */
/* =========================== Forward Declarations ========================= */
/* ========================================================================== */

static PyTypeObject NetworkAddressType;
static PyTypeObject HostEntryType;
static PyTypeObject SocketType;

/* ========================================================================== */
/* =============================== Prototypes =============================== */
/* ========================================================================== */

static PyObject *
HostEntry_get_hostname(HostEntry *self, void *closure);

static PyObject *
_readline(Socket *self, long size);

static PyObject *
_recv(Socket *self, long requested_amount, unsigned int timeout);

/* ========================================================================== */
/* ================================ Utilities =============================== */
/* ========================================================================== */

#ifdef MS_WINDOWS
    typedef SOCKET SOCKET_T;
    #ifdef MS_WIN64
        #define SIZEOF_SOCKET_T 8
    #else
        #define SIZEOF_SOCKET_T 4
    #endif
#else
    typedef int SOCKET_T;
    #define SIZEOF_SOCKET_T SIZEOF_INT
#endif

PROsfd PR_FileDesc2NativeHandle(PRFileDesc *);

#if defined(MS_WINDOWS) || defined(__BEOS__)
#define SOCKETCLOSE closesocket
#define NO_DUP /* Actually it exists on NT 3.5, but what the heck... */
#endif

#ifndef SOCKETCLOSE
#define SOCKETCLOSE close
#endif

static PyObject *
err_closed(void)
{
    PyErr_SetString(PyExc_ValueError, "I/O operation on closed socket");
    return NULL;
}

static const char*
pr_family_str(value)
{
    switch(value) {
    case PR_AF_INET:   return "PR_AF_INET";
    case PR_AF_INET6:  return "PR_AF_INET6";
    case PR_AF_LOCAL:  return "PR_AF_LOCAL";
    case PR_AF_UNSPEC: return "PR_AF_UNSPEC";
    default:           return "unknown";
    }
}

static const char*
pr_file_desc_type_str(value)
{
    switch(value) {
    case PR_DESC_FILE:       return "PR_DESC_FILE";
    case PR_DESC_SOCKET_TCP: return "PR_DESC_SOCKET_TCP";
    case PR_DESC_SOCKET_UDP: return "PR_DESC_SOCKET_UDP";
    case PR_DESC_LAYERED:    return "PR_DESC_LAYERED";
    case PR_DESC_PIPE:       return "PR_DESC_PIPE";
    default:                 return "unknown";
    }
}

#if 0
static const char*
pr_sock_option_str(value)
{
    switch(value) {
    case PR_SockOpt_Nonblocking:     return "PR_SockOpt_Nonblocking";
    case PR_SockOpt_Linger:          return "PR_SockOpt_Linger";
    case PR_SockOpt_Reuseaddr:       return "PR_SockOpt_Reuseaddr";
    case PR_SockOpt_Keepalive:       return "PR_SockOpt_Keepalive";
    case PR_SockOpt_RecvBufferSize:  return "PR_SockOpt_RecvBufferSize";
    case PR_SockOpt_SendBufferSize:  return "PR_SockOpt_SendBufferSize";
    case PR_SockOpt_IpTimeToLive:    return "PR_SockOpt_IpTimeToLive";
    case PR_SockOpt_IpTypeOfService: return "PR_SockOpt_IpTypeOfService";
    case PR_SockOpt_AddMember:       return "PR_SockOpt_AddMember";
    case PR_SockOpt_DropMember:      return "PR_SockOpt_DropMember";
    case PR_SockOpt_McastInterface:  return "PR_SockOpt_McastInterface";
    case PR_SockOpt_McastTimeToLive: return "PR_SockOpt_McastTimeToLive";
    case PR_SockOpt_McastLoopback:   return "PR_SockOpt_McastLoopback";
    case PR_SockOpt_NoDelay:         return "PR_SockOpt_NoDelay";
    case PR_SockOpt_MaxSegment:      return "PR_SockOpt_MaxSegment";
    case PR_SockOpt_Broadcast:       return "PR_SockOpt_Broadcast";
    default:                         return "unknown";
    }
}
#endif


#if 0
static const char*
pr_initialize_netaddr_str(value)
{
    switch(value) {
    case PR_IpAddrNull:     return "PR_IpAddrNull";
    case PR_IpAddrAny:      return "PR_IpAddrAny";
    case PR_IpAddrLoopback: return "PR_IpAddrLoopback";
    default:                return "unknown";
    }
}

static const char*
pr_socket_shutdown_str(value)
{
    switch(value) {
    case PR_SHUTDOWN_RCV:  return "PR_SHUTDOWN_RCV";
    case PR_SHUTDOWN_SEND: return "PR_SHUTDOWN_SEND";
    case PR_SHUTDOWN_BOTH: return "PR_SHUTDOWN_BOTH";
    default:               return "unknown";
    }
}


#endif

/* ========================================================================== */
/* =========================== NetworkAddress Class ========================= */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static PyObject *
NetworkAddress_get_hostentry(NetworkAddress *self, void *closure)
{
    if (self->py_hostentry == NULL) {
        self->py_hostentry = (HostEntry *)HostEntry_new_from_PRNetAddr(&self->addr);
    }
    return (PyObject *)self->py_hostentry;
}

static PyObject *
NetworkAddress_get_hostname(NetworkAddress *self, void *closure)
{
    if (self->py_hostname) {
        Py_INCREF(self->py_hostname);
        return self->py_hostname;
    }
    if (self->py_hostentry == NULL) {
        if ((self->py_hostentry = (HostEntry *)HostEntry_new_from_PRNetAddr(&self->addr)) == NULL)
            return NULL;
    }
    if ((self->py_hostname = HostEntry_get_hostname(self->py_hostentry, NULL)) == NULL)
        return NULL;

    Py_INCREF(self->py_hostname);
    return self->py_hostname;
}

static PyObject *
NetworkAddress_get_port(NetworkAddress *self, void *closure)
{
    return PyInt_FromLong(PR_ntohs(self->addr.inet.port));
}

static int
NetworkAddress_set_port(NetworkAddress *self, PyObject *value, void *closure)
{
    int port;

    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "Cannot delete the port attribute");
        return -1;
    }

    if (!PyInt_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "The port attribute value must be an integer");
        return -1;
    }

    port = PyInt_AsLong(value);
    if (PR_InitializeNetAddr(PR_IpAddrNull, port, &self->addr) != PR_SUCCESS) {
        set_nspr_error(NULL);
        return -1;
    }

  return 0;
}

static PyGetSetDef
NetworkAddress_getseters[] = {
    {"hostentry", (getter)NetworkAddress_get_hostentry, NULL, "HostEntry object representing this NetworkAddress", NULL},
    {"hostname",  (getter)NetworkAddress_get_hostname, NULL,
     "If a hostname was used to construct this NetworkAddress then return that name (e.g. NetworkAddress(hostname), else return the reverse lookup hostname (equivalent to self.hostentry.hostname)", NULL},
    {"port",      (getter)NetworkAddress_get_port,      (setter)NetworkAddress_set_port, "network address port", NULL},
    {NULL}  /* Sentinel */
};

static PyMemberDef
NetworkAddress_members[] = {
    {"family", T_INT, offsetof(NetworkAddress, addr.raw.family), 0, "network address family"},
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

PyDoc_STRVAR(NetworkAddress_set_from_string_doc,
"set_from_string(addr)\n\
\n\
:Parameters:\n\
    addr : string\n\
        the address string to convert\n\
\n\
Reinitializes the NetworkAddress object given a string.\n\
Identical to constructing nss.io.NetworkAddress() with a\n\
string value (see constructor for documentation).\n\
");
static PyObject *
NetworkAddress_set_from_string(NetworkAddress *self, PyObject *args)
{
    PyObject *addr = NULL;
    char *addr_str = NULL;

    if (!PyArg_ParseTuple(args, "O!:set_from_string", &PyString_Type, &addr))
        return NULL;
    addr_str = PyString_AsString(addr);

    /* First try to parse as a dotted decimal, if that fails try a DNS lookup */
    if (PR_StringToNetAddr(addr_str, &self->addr) != PR_SUCCESS) {
        /* Not an address string, try DNS */
        PRHostEnt entry;
        char buffer[PR_NETDB_BUF_SIZE];

        Py_INCREF(addr);
        self->py_hostname = addr;

        Py_BEGIN_ALLOW_THREADS
        if (PR_GetHostByName(addr_str, buffer, sizeof(buffer), &entry) != PR_SUCCESS) {
            Py_BLOCK_THREADS
            return set_nspr_error("cannot resolve address (%s)", addr_str);
        }
        Py_END_ALLOW_THREADS

        if (PR_EnumerateHostEnt(0, &entry, 0, &self->addr) < 0) {
            return set_nspr_error(NULL);
        }
    }
        Py_RETURN_NONE;
}

static PyMethodDef
NetworkAddress_methods[] = {
    {"set_from_string", (PyCFunction)NetworkAddress_set_from_string, METH_VARARGS, NetworkAddress_set_from_string_doc},
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
NetworkAddress_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    NetworkAddress *self = NULL;

    TraceObjNewEnter(type);

    if ((self = (NetworkAddress *)type->tp_alloc(type, 0)) == NULL) return NULL;
    memset(&self->addr, 0, sizeof(self->addr));
    self->py_hostname = NULL;
    self->py_hostentry = NULL;

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static PyObject *
NetworkAddress_new_from_PRNetAddr(PRNetAddr *pr_netaddr)
{
    NetworkAddress *self = NULL;

    TraceObjNewEnter(NULL);

    if ((self = (NetworkAddress *) NetworkAddressType.tp_new(&NetworkAddressType, NULL, NULL)) == NULL)
        return NULL;

    self->addr = *pr_netaddr;

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

static void
NetworkAddress_dealloc(NetworkAddress* self)
{
    TraceMethodEnter(self);

    Py_XDECREF(self->py_hostname);
    Py_XDECREF(self->py_hostentry);
    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(NetworkAddress_doc,
"NetworkAddress(addr, port=0)\n\
\n\
:Parameters:\n\
    addr : string or integer\n\
        may be an int or a string.\n\
    port : integer\n\
        port number\n\
\n\
If addr argument is a string it may be either a numeric address or a DNS host\n\
name. First the addr string is tested to see if it can be parsed as a IPv4\n\
Dotted Decimal Notation or IPv6 Hexadecimal Notation, Otherwise the addr string\n\
is passed to PR_GetHostByName to resolve the name. If the name is resolved the\n\
first host entry returned by PR_EnumerateHostEnt is used to initialize the\n\
NetworkAddress. If you need more fine grained control over which address is\n\
selected from the HostEntry then utilize HostEntry.get_network_addresses()\n\
instead.\n\
\n\
If the addr argument is an integer it may be one of the following constants:\n\
\n\
PR_IpAddrNull\n\
    Do not set the IP address, only set the port.\n\
    NetworkAddress(PR_IpAddrNull, 123) is equivalent to NetworkAddress(port=123)\n\
\n\
PR_IpAddrAny\n\
    Assign logical PR_INADDR_ANY to IP address. This wildcard value is typically\n\
    used to establish a socket on which to listen for incoming connection requests.\n\
\n\
PR_IpAddrLoopback\n\
    Assign logical PR_INADDR_LOOPBACK. A client can use this value to connect to\n\
    itself without knowing the host's network address.\n\
\n\
The optional port argument sets the port number in the NetworkAddress object.\n\
The port number may be modfied later by assigning to the port attribute.\n\
\n\
Example::\n\
    \n\
    netaddr = nss.io.NetworkAddress('www.python.org')\n\
    print '%s %s' % (netaddr, netaddr.hostname)\n\
    netaddr = nss.io.NetworkAddress('82.94.237.218')\n\
    print '%s %s' % (netaddr, netaddr.hostname)\n\
    \n\
    Output:\n\
    82.94.237.218:0 www.python.org\n\
    82.94.237.218:0 dinsdale.python.org\n\
");

static int
NetworkAddress_init(NetworkAddress *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"addr", "port",  NULL};
    PyObject *addr = NULL;
    int port = 0;
    int addr_int = PR_IpAddrNull;
    char *addr_str = NULL;


    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|Oi", kwlist, &addr, &port))
        return -1;

    if (addr && !(PyInt_Check(addr) || PyString_Check(addr) || PyUnicode_Check(addr))) {
        PyErr_SetString(PyExc_ValueError, "addr must be an int or a string");
        return -1;
    }

    if (addr && PyInt_Check(addr)) {
        addr_int = PyInt_AsLong(addr);
        switch(addr_int) {
        case PR_IpAddrNull:
        case PR_IpAddrAny:
        case PR_IpAddrLoopback:
            break;
        default:
            PyErr_SetString(PyExc_ValueError, "addr is an int, must be PR_IpAddrNull, PR_IpAddrAny or PR_IpAddrLoopback");
            return -1;
        }
    }

    if (PR_InitializeNetAddr(addr_int, port, &self->addr) != PR_SUCCESS) {
        set_nspr_error(NULL);
        return -1;
    }

    if (addr && (PyString_Check(addr) || PyUnicode_Check(addr))) {
        addr_str = PyString_AsString(addr);

        /* First try to parse as a dotted decimal, if that fails try a DNS lookup */
        if (PR_StringToNetAddr(addr_str, &self->addr) != PR_SUCCESS) {
            /* Not an address string, try DNS */
            PRHostEnt entry;
            char buffer[PR_NETDB_BUF_SIZE];

            Py_INCREF(addr);
            self->py_hostname = addr;

            Py_BEGIN_ALLOW_THREADS
            if (PR_GetHostByName(addr_str, buffer, sizeof(buffer), &entry) != PR_SUCCESS) {
                Py_BLOCK_THREADS
                set_nspr_error("cannot resolve address (%s)", addr_str);
                return -1;
            }
            Py_END_ALLOW_THREADS

            if (PR_EnumerateHostEnt(0, &entry, port, &self->addr) < 0) {
                set_nspr_error(NULL);
                return -1;
            }
        }
    }
    return 0;
}

static PyObject *
NetworkAddress_str(NetworkAddress *self)
{
    char buf[1024];

    if (self->addr.raw.family == PR_AF_UNSPEC) {
        return PyString_FromString(unset_string);
    }

    if (PR_NetAddrToString(&self->addr, buf, sizeof(buf)) != PR_SUCCESS) {
        return set_nspr_error(NULL);
    }

    switch(self->addr.raw.family) {
    case PR_AF_INET:
        return PyString_FromFormat("%s:%d", buf, PR_ntohs(self->addr.inet.port));
    case PR_AF_INET6:
        return PyString_FromFormat("[%s]:%d", buf, PR_ntohs(self->addr.ipv6.port));
    default:
        return PyString_FromString(buf);
    }
}

static PyTypeObject
NetworkAddressType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.io.NetworkAddress",			/* tp_name */
    sizeof(NetworkAddress),			/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)NetworkAddress_dealloc,		/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    0,						/* tp_compare */
    0,						/* tp_repr */
    0,						/* tp_as_number */
    0,						/* tp_as_sequence */
    0,						/* tp_as_mapping */
    0,						/* tp_hash */
    0,						/* tp_call */
    (reprfunc)NetworkAddress_str,		/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
    NetworkAddress_doc,				/* tp_doc */
    0,						/* tp_traverse */
    0,						/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    NetworkAddress_methods,			/* tp_methods */
    NetworkAddress_members,			/* tp_members */
    NetworkAddress_getseters,			/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)NetworkAddress_init,		/* tp_init */
    0,						/* tp_alloc */
    NetworkAddress_new,				/* tp_new */
};

/* ========================================================================== */
/* ============================= HostEntry Class ============================ */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static PyObject *
HostEntry_get_hostname(HostEntry *self, void *closure)
{
    return PyString_FromString(self->entry.h_name);
}

static PyObject *
HostEntry_get_aliases(HostEntry *self, void *closure)
{
    int len, i;
    PyObject *alias_tuple = NULL;
    PyObject *alias = NULL;

    if (self->entry.h_aliases)
        for (len = 0; self->entry.h_aliases[len]; len++);
    else
        len = 0;

    if ((alias_tuple = PyTuple_New(len)) == NULL)
        return NULL;

    for (i = 0; i < len; i++) {
        if ((alias = PyString_FromString(self->entry.h_aliases[i])) == NULL) {
            Py_DECREF(alias_tuple);
            return NULL;
        }
        PyTuple_SetItem(alias_tuple, i, alias);
    }

    return alias_tuple;
}

static PyGetSetDef
HostEntry_getseters[] = {
    {"hostname", (getter)HostEntry_get_hostname, (setter)NULL, "official name of host", NULL},
    {"aliases",  (getter)HostEntry_get_aliases, (setter)NULL, "tuple of aliases for host", NULL},
    {NULL}  /* Sentinel */
};

static PyMemberDef
HostEntry_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

PyDoc_STRVAR(HostEntry_get_network_addresses_doc,
"get_network_addresses(port=0)\n\
\n\
Return a tuple of all possible network address associated with this\n\
HostEntry. Each item in the returned tuple is a NetworkAddress object.\n\
");
static PyObject *
HostEntry_get_network_addresses(HostEntry *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"port", NULL};
    int len, i;
    PyObject *addr_tuple = NULL;
    PyObject *py_netaddr = NULL;
    PRNetAddr pr_netaddr;

    int port = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:get_network_addresses", kwlist, &port))
        return NULL;

    if (self->entry.h_addr_list)
        for (len = 0; self->entry.h_addr_list[len]; len++);
    else
        len = 0;

    if ((addr_tuple = PyTuple_New(len)) == NULL)
        return NULL;

    for (i = 0; i < len; i++) {
        if (PR_EnumerateHostEnt(i, &self->entry, port, &pr_netaddr) < 0) {
            Py_DECREF(addr_tuple);
            return set_nspr_error(NULL);
        }
        if ((py_netaddr = NetworkAddress_new_from_PRNetAddr(&pr_netaddr)) == NULL) {
            Py_DECREF(addr_tuple);
            return NULL;
        }
        PyTuple_SetItem(addr_tuple, i, py_netaddr);
    }

    return addr_tuple;
}

PyDoc_STRVAR(HostEntry_get_network_address_doc,
"get_network_address(port=0)\n\
\n\
:Parameters:\n\
    port : integer\n\
        optional port value specifying the port to associate with the NetworkAddress.\n\
\n\
Returns the first network address associated with this HostEntry as a\n\
NetworkAddress object. Equivalent to get_network_addresses()[0]. Note,\n\
may return None if the HostEntry does not have address associated with\n\
it.\n\
");
static PyObject *
HostEntry_get_network_address(HostEntry *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"port", NULL};
    NetworkAddress *netaddr = NULL;
    int port = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:get_network_address", kwlist, &port))
        return NULL;

    if (!self->entry.h_addr_list)
        Py_RETURN_NONE;

    if (!self->entry.h_addr_list[0])
        Py_RETURN_NONE;

    if ((netaddr = (NetworkAddress *) NetworkAddressType.tp_new(&NetworkAddressType, NULL, NULL)) == NULL)
        return NULL;

    if (PR_EnumerateHostEnt(0, &self->entry, port, &netaddr->addr) < 0) {
        Py_DECREF(netaddr);
        return set_nspr_error(NULL);
    }

    return (PyObject *)netaddr;
}

static PyMethodDef
HostEntry_methods[] = {
    {"get_network_addresses", (PyCFunction)HostEntry_get_network_addresses, METH_VARARGS|METH_KEYWORDS, HostEntry_get_network_addresses_doc},
    {"get_network_address",   (PyCFunction)HostEntry_get_network_address,   METH_VARARGS|METH_KEYWORDS, HostEntry_get_network_address_doc},
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
HostEntry_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    HostEntry *self = NULL;

    TraceObjNewEnter(type);

    if ((self = (HostEntry *)type->tp_alloc(type, 0)) == NULL) return NULL;
    memset(&self->entry,  0, sizeof(self->entry));
    memset(&self->buffer, 0, sizeof(self->buffer));

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static PyObject *
HostEntry_new_from_PRNetAddr(PRNetAddr *pr_netaddr)
{
    HostEntry *self = NULL;

    TraceObjNewEnter(NULL);

    if ((self = (HostEntry *) HostEntryType.tp_new(&HostEntryType, NULL, NULL)) == NULL)
        return NULL;

    Py_BEGIN_ALLOW_THREADS
    if ((PR_GetHostByAddr(pr_netaddr, self->buffer,
                          sizeof(self->buffer), &self->entry)) != PR_SUCCESS) {
        Py_BLOCK_THREADS
        return set_nspr_error(NULL);
    }
    Py_END_ALLOW_THREADS

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

static void
HostEntry_dealloc(HostEntry* self)
{
    TraceMethodEnter(self);

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(HostEntry_doc,
"HostEntry(addr)\n\
\n\
:Parameters:\n\
    addr : string or NetworkAddr object\n\
        May be either a string or a NetworkAddr object.\n\
            - If addr is string it is equivalent to GetHostByName.\n\
            - If addr is a NetworkAddress object it is equivalent to GetHostByAddr.\n\
\n\
An object used to encapsulate network address information for a\n\
specific host.\n\
");

static int
HostEntry_init(HostEntry *self, PyObject *args)
{
    PyObject *addr = NULL;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "O", &addr))
        return -1;

    if (PyString_Check(addr) || PyUnicode_Check(addr)) {

        Py_BEGIN_ALLOW_THREADS
        if (PR_GetHostByName(PyString_AsString(addr), self->buffer,
                             sizeof(self->buffer), &self->entry) != PR_SUCCESS) {
            Py_BLOCK_THREADS
            set_nspr_error(NULL);
            return -1;
        }
        Py_END_ALLOW_THREADS

    }
    else if (PyNetworkAddress_Check(addr)) {

        Py_BEGIN_ALLOW_THREADS
        if (PR_GetHostByAddr(&((NetworkAddress *)addr)->addr, self->buffer,
                             sizeof(self->buffer), &self->entry) != PR_SUCCESS) {
            Py_BLOCK_THREADS
            set_nspr_error(NULL);
            return -1;
        }
        Py_END_ALLOW_THREADS
    }

    return 0;
}

static PyObject *
HostEntry_str(HostEntry *self)
{
    PyObject *addrs = NULL;
    PyObject *addr_list = NULL;
    PyObject *addr_iter = NULL;
    PyObject *addr = NULL;
    PyObject *str = NULL;
    PyObject *aliases = NULL;
    PyObject *args = NULL;
    PyObject *text = NULL;
    int i;

    addrs = PyObject_CallMethod((PyObject *)self, "get_network_addresses", NULL);
    if ((addr_list = PyTuple_New(PyTuple_Size(addrs))) == NULL) {
        goto exit;
    }

    if ((addr_iter = PyObject_GetIter(addrs)) == NULL) {
        goto exit;
    }

    for (i = 0; (addr = PyIter_Next(addr_iter)); i++) {
        if ((str = PyObject_Str(addr)) == NULL) {
            Py_DECREF(addr);
            goto exit;
        }
        if (PyTuple_SetItem(addr_list, i, str) == -1) {
            Py_DECREF(str);
            goto exit;
        }
    }

    if ((aliases = PyObject_GetAttrString((PyObject *)self, "aliases")) == NULL) {
        goto exit;
    }

    args = Py_BuildValue("(sSS)", self->entry.h_name ? self->entry.h_name : "None",
                         aliases, addr_list);
    text = PyString_Format(PyString_FromString("name=%s aliases=%s addresses=%s"), args);


 exit:
    Py_XDECREF(addrs);
    Py_XDECREF(addr_list);
    Py_XDECREF(addr_iter);
    Py_XDECREF(aliases);
    Py_XDECREF(args);
    return text;

}

static PyTypeObject
HostEntryType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.io.HostEntry",				/* tp_name */
    sizeof(HostEntry),				/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)HostEntry_dealloc,		/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    0,						/* tp_compare */
    0,						/* tp_repr */
    0,						/* tp_as_number */
    0,						/* tp_as_sequence */
    0,						/* tp_as_mapping */
    0,						/* tp_hash */
    0,						/* tp_call */
    (reprfunc)HostEntry_str,			/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
    HostEntry_doc,				/* tp_doc */
    0,						/* tp_traverse */
    0,						/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    HostEntry_methods,				/* tp_methods */
    HostEntry_members,				/* tp_members */
    HostEntry_getseters,			/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)HostEntry_init,			/* tp_init */
    0,						/* tp_alloc */
    HostEntry_new,				/* tp_new */
};

/* ========================================================================== */
/* ============================== Socket Class ============================== */
/* ========================================================================== */

static void
Socket_init_from_PRFileDesc(Socket *self, PRFileDesc *pr_socket, int family)
{
    TraceMethodEnter(self);

    self->pr_socket = pr_socket;
    self->family = family;
}


static PyObject *
Socket_new_from_PRFileDesc(PRFileDesc *pr_socket, int family)
{
    Socket *self = NULL;

    TraceObjNewEnter(NULL);

    if ((self = (Socket *) SocketType.tp_new(&SocketType, NULL, NULL)) == NULL)
        return NULL;

    Socket_init_from_PRFileDesc(self, pr_socket, family);

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

/* ============================ Attribute Access ============================ */

static PyObject *
Socket_get_netaddr(Socket *self, void *closure)
{
    if (!self->py_netaddr) {
        Py_RETURN_NONE;
    }
    Py_INCREF(self->py_netaddr);
    return (PyObject *)self->py_netaddr;
}

// FIXME: should we just store this in our struct?
static PyObject *
Socket_get_desc_type(Socket *self, void *closure)
{
    int desc_type;

    if (!self->pr_socket) {
        PyErr_SetString(PyExc_ValueError, "socket not intialized");
        return NULL;
    }

    desc_type = PR_GetDescType(self->pr_socket);
    return PyInt_FromLong(desc_type);
}

static PyGetSetDef
Socket_getseters[] = {
    {"netaddr", (getter)Socket_get_netaddr,     (setter)NULL, "NetworkAddress object bound to this socket", NULL},
    {"desc_type", (getter)Socket_get_desc_type, (setter)NULL, "socket description: PR_DESC_FILE, PR_DESC_SOCKET_TCP, PR_DESC_SOCKET_UDP, PR_DESC_LAYERED, PR_DESC_PIPE", NULL},
    {NULL}  /* Sentinel */
};

static PyMemberDef
Socket_members[] = {
    {"family", T_INT, offsetof(Socket, family), 0, "socket family: PR_AF_INET, PR_AF_INET6, PR_AF_LOCAL, PR_AF_UNSPEC"},
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

PyDoc_STRVAR(Socket_set_socket_option_doc,
"set_socket_option(option, ...)\n\
\n\
The method signature varies depending on the option, see below:\n\
\n\
Set socket to non-blocking IO\n\
    ::\n\
        \n\
        set_socket_option(PR_SockOpt_Nonblocking, bool)\n\
\n\
Time to linger on close if data is present in socket send buffer. \n\
    ::\n\
        \n\
        set_socket_option(PR_SockOpt_Linger, polarity, interval)\n\
\n\
Allow local address reuse\n\
    ::\n\
        \n\
        set_socket_option(PR_SockOpt_Reuseaddr, bool)\n\
\n\
Keep connections alive\n\
    ::\n\
        \n\
        set_socket_option(PR_SockOpt_Keepalive, bool)\n\
\n\
Allow IP multicast loopback\n\
    ::\n\
        \n\
        set_socket_option(PR_SockOpt_McastLoopback, bool)\n\
\n\
Disable Nagle algorithm. Don't delay send to coalesce packets. \n\
    ::\n\
        \n\
        set_socket_option(PR_SockOpt_NoDelay, bool)\n\
\n\
Enable broadcast\n\
    ::\n\
        \n\
        set_socket_option(PR_SockOpt_Broadcast, bool)\n\
\n\
Receive buffer size. \n\
    ::\n\
        \n\
        set_socket_option(PR_SockOpt_RecvBufferSize, size)\n\
\n\
Send buffer size. \n\
    ::\n\
        \n\
        set_socket_option(PR_SockOpt_SendBufferSize, size)\n\
\n\
Maximum segment size\n\
    ::\n\
        \n\
        set_socket_option(PR_SockOpt_MaxSegment, size)\n\
\n\
IP Time to Live\n\
    ::\n\
        \n\
        set_socket_option(PR_SockOpt_IpTimeToLive, interval)\n\
\n\
IP type of service and precedence\n\
    ::\n\
        \n\
        set_socket_option(PR_SockOpt_IpTypeOfService, tos)\n\
\n\
Add an IP group membership\n\
    ::\n\
        \n\
        set_socket_option(PR_SockOpt_AddMember, mcaddr, ifaddr)\n\
\n\
- mcaddr is a NetworkAddress object representing the IP multicast address of group\n\
- ifaddr is a NetworkAddress object representing the local IP address of the interface\n\
\n\
Drop an IP group membership\n\
    ::\n\
        \n\
        set_socket_option(PR_SockOpt_DropMember, mcaddr, ifaddr)\n\
\n\
- mcaddr is a NetworkAddress object representing the IP multicast address of group\n\
- ifaddr is a NetworkAddress object representing the local IP address of the interface\n\
\n\
Multicast Time to Live\n\
    ::\n\
        \n\
        set_socket_option(PR_SockOpt_McastTimeToLive, interval)\n\
\n\
Multicast interface address\n\
    ::\n\
        \n\
        set_socket_option(PR_SockOpt_McastInterface, ifaddr)\n\
\n\
- ifaddr is a NetworkAddress object representing the multicast interface address\n\
"
);
static PyObject *
Socket_set_socket_option(Socket *self, PyObject *args)
{
    PyObject *py_option = NULL;
    int option;
    int bool;
    unsigned int uint;
    NetworkAddress *mcaddr = NULL;
    NetworkAddress *ifaddr = NULL;
    PRSocketOptionData data;

    TraceMethodEnter(self);

    if ((py_option = PyTuple_GetItem(args, 0)) == NULL) {
        PyErr_SetString(PyExc_TypeError, "set_socket_option: missing option argument");
        return NULL;
    }

    if (!PyInt_Check(py_option)) {
        PyErr_SetString(PyExc_TypeError, "set_socket_option: option must be an int");
        return NULL;
    }

    option = PyInt_AsLong(py_option);
    data.option = option;

    switch(option) {
    case PR_SockOpt_Nonblocking:
        if (!PyArg_ParseTuple(args, "ii:set_socket_option", &option, &bool))
            return NULL;
        data.value.non_blocking = bool;
        break;
    case PR_SockOpt_Linger:
        if (!PyArg_ParseTuple(args, "iiI:set_socket_option", &option, &bool, &uint))
            return NULL;
        data.value.linger.polarity = bool;
        data.value.linger.linger = uint;
        break;
    case PR_SockOpt_Reuseaddr:
        if (!PyArg_ParseTuple(args, "ii:set_socket_option", &option, &bool))
            return NULL;
        data.value.reuse_addr = bool;
        break;
    case PR_SockOpt_Keepalive:
        if (!PyArg_ParseTuple(args, "ii:set_socket_option", &option, &bool))
            return NULL;
        data.value.keep_alive = bool;
        break;
    case PR_SockOpt_RecvBufferSize:
        if (!PyArg_ParseTuple(args, "iI:set_socket_option", &option, &uint))
            return NULL;
        data.value.recv_buffer_size = uint;
        break;
    case PR_SockOpt_SendBufferSize:
        if (!PyArg_ParseTuple(args, "iI:set_socket_option", &option, &uint))
            return NULL;
        data.value.send_buffer_size = uint;
        break;
    case PR_SockOpt_IpTimeToLive:
        if (!PyArg_ParseTuple(args, "iI:set_socket_option", &option, &uint))
            return NULL;
        data.value.ip_ttl = uint;
        break;
    case PR_SockOpt_IpTypeOfService:
        if (!PyArg_ParseTuple(args, "iI:set_socket_option", &option, &uint))
            return NULL;
        data.value.tos = uint;
        break;
    case PR_SockOpt_AddMember:
        if (!PyArg_ParseTuple(args, "iO!O!:set_socket_option", &option,
                              &NetworkAddressType, &mcaddr,
                              &NetworkAddressType, &ifaddr))
            return NULL;
        data.value.add_member.mcaddr = mcaddr->addr;
        data.value.add_member.ifaddr = ifaddr->addr;
        break;
    case PR_SockOpt_DropMember:
        if (!PyArg_ParseTuple(args, "iO!O!:set_socket_option", &option,
                              &NetworkAddressType, &mcaddr,
                              &NetworkAddressType, &ifaddr))
            return NULL;
        data.value.drop_member.mcaddr = mcaddr->addr;
        data.value.drop_member.ifaddr = ifaddr->addr;
        break;
    case PR_SockOpt_McastInterface:
        if (!PyArg_ParseTuple(args, "iO!:set_socket_option", &option,
                              &NetworkAddressType, &ifaddr))
            return NULL;
        data.value.mcast_if = ifaddr->addr;
        break;
    case PR_SockOpt_McastTimeToLive:
        if (!PyArg_ParseTuple(args, "iI:set_socket_option", &option, &uint))
            return NULL;
        data.value.mcast_ttl = uint;
        break;
    case PR_SockOpt_McastLoopback:
        if (!PyArg_ParseTuple(args, "ii:set_socket_option", &option, &bool))
            return NULL;
        data.value.mcast_loopback = bool;
        break;
    case PR_SockOpt_NoDelay:
        if (!PyArg_ParseTuple(args, "ii:set_socket_option", &option, &bool))
            return NULL;
        data.value.no_delay = bool;
        break;
    case PR_SockOpt_MaxSegment:
        if (!PyArg_ParseTuple(args, "iI:set_socket_option", &option, &uint))
            return NULL;
        data.value.max_segment = uint;
        break;
    case PR_SockOpt_Broadcast:
        if (!PyArg_ParseTuple(args, "ii:set_socket_option", &option, &bool))
            return NULL;
        data.value.broadcast = bool;
        break;
    default:
        PyErr_SetString(PyExc_ValueError, "set_socket_option: unknown option");
        return NULL;
    }

    if (PR_SetSocketOption(self->pr_socket, &data) != PR_SUCCESS)
        return set_nspr_error(NULL);

    Py_RETURN_NONE;
}

PyDoc_STRVAR(Socket_get_socket_option_doc,
"get_socket_option(option)\n\
\n\
The method return values varies depending on the option, see below:\n\
\n\
Set socket to non-blocking IO\n\
    ::\n\
        \n\
        get_socket_option(PR_SockOpt_Nonblocking) -> bool\n\
\n\
Time to linger on close if data is present in socket send buffer. \n\
    ::\n\
        \n\
        get_socket_option(PR_SockOpt_Linger) -> (polarity, interval)\n\
\n\
Allow local address reuse\n\
    ::\n\
        \n\
        get_socket_option(PR_SockOpt_Reuseaddr) -> bool\n\
\n\
Keep connections alive\n\
    ::\n\
        \n\
        get_socket_option(PR_SockOpt_Keepalive) -> bool\n\
\n\
Allow IP multicast loopback\n\
    ::\n\
        \n\
        get_socket_option(PR_SockOpt_McastLoopback) -> bool\n\
\n\
Disable Nagle algorithm. Don't delay send to coalesce packets. \n\
    ::\n\
        \n\
        get_socket_option(PR_SockOpt_NoDelay) -> bool\n\
\n\
Enable broadcast\n\
    ::\n\
        \n\
        get_socket_option(PR_SockOpt_Broadcast) -> bool\n\
\n\
Receive buffer size. \n\
    ::\n\
        \n\
        get_socket_option(PR_SockOpt_RecvBufferSize) -> size\n\
\n\
Send buffer size. \n\
    ::\n\
        \n\
        get_socket_option(PR_SockOpt_SendBufferSize) -> size\n\
\n\
Maximum segment size\n\
    ::\n\
        \n\
        get_socket_option(PR_SockOpt_MaxSegment) -> size\n\
\n\
IP Time to Live\n\
    ::\n\
        \n\
        get_socket_option(PR_SockOpt_IpTimeToLive) -> interval\n\
\n\
IP type of service and precedence\n\
    ::\n\
        \n\
        get_socket_option(PR_SockOpt_IpTypeOfService) -> tos\n\
\n\
Add an IP group membership\n\
    ::\n\
        \n\
        get_socket_option(PR_SockOpt_AddMember) -> (mcaddr, ifaddr)\n\
\n\
- mcaddr is a NetworkAddress object representing the IP multicast address of group\n\
- ifaddr is a NetworkAddress object representing the local IP address of the interface\n\
\n\
Drop an IP group membership\n\
    ::\n\
        \n\
        get_socket_option(PR_SockOpt_DropMember) -> (mcaddr, ifaddr)\n\
\n\
- mcaddr is a NetworkAddress object representing the IP multicast address of group\n\
- ifaddr is a NetworkAddress object representing the local IP address of the interface\n\
\n\
Multicast Time to Live\n\
    ::\n\
        \n\
        get_socket_option(PR_SockOpt_McastTimeToLive) -> interval\n\
\n\
Multicast interface address\n\
    ::\n\
        \n\
        get_socket_option(PR_SockOpt_McastInterface) -> ifaddr\n\
\n\
- ifaddr is a NetworkAddress object representing the multicast interface address\n\
"
);
static PyObject *
Socket_get_socket_option(Socket *self, PyObject *args)
{
    int option;
    PyObject *mcaddr = NULL;
    PyObject *ifaddr = NULL;
    PRSocketOptionData data;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "i:get_socket_option", &option))
        return NULL;

    data.option = option;
    if (PR_GetSocketOption(self->pr_socket, &data) != PR_SUCCESS)
        return set_nspr_error(NULL);

    switch(option) {
    case PR_SockOpt_Nonblocking:
        Py_RETURN_BOOL(data.value.non_blocking);
        break;
    case PR_SockOpt_Linger:
        return Py_BuildValue("OI", data.value.linger.polarity ? Py_True : Py_False, data.value.linger.linger);
        break;
    case PR_SockOpt_Reuseaddr:
        Py_RETURN_BOOL(data.value.reuse_addr);
        break;
    case PR_SockOpt_Keepalive:
        Py_RETURN_BOOL(data.value.keep_alive);
        break;
    case PR_SockOpt_RecvBufferSize:
        return Py_BuildValue("I", data.value.recv_buffer_size);
        break;
    case PR_SockOpt_SendBufferSize:
        return Py_BuildValue("I", data.value.send_buffer_size);
        break;
    case PR_SockOpt_IpTimeToLive:
        return Py_BuildValue("I", data.value.ip_ttl);
        break;
    case PR_SockOpt_IpTypeOfService:
        return Py_BuildValue("I", data.value.tos);
        break;
    case PR_SockOpt_AddMember:
        if ((mcaddr = NetworkAddress_new_from_PRNetAddr(&data.value.add_member.mcaddr)) == NULL) {
            return NULL;
        }
        if ((ifaddr = NetworkAddress_new_from_PRNetAddr(&data.value.add_member.ifaddr)) == NULL) {
            Py_DECREF(mcaddr);
            return NULL;
        }
        return Py_BuildValue("OO", mcaddr, ifaddr);
        break;
    case PR_SockOpt_DropMember:
        if ((mcaddr = NetworkAddress_new_from_PRNetAddr(&data.value.drop_member.mcaddr)) == NULL) {
            return NULL;
        }
        if ((ifaddr = NetworkAddress_new_from_PRNetAddr(&data.value.drop_member.ifaddr)) == NULL) {
            Py_DECREF(mcaddr);
            return NULL;
        }
        return Py_BuildValue("OO", mcaddr, ifaddr);
        break;
    case PR_SockOpt_McastInterface:
        if ((ifaddr = NetworkAddress_new_from_PRNetAddr(&data.value.mcast_if)) == NULL) {
            return NULL;
        }
        return ifaddr;
        break;
    case PR_SockOpt_McastTimeToLive:
        return Py_BuildValue("I", data.value.mcast_ttl);
        break;
    case PR_SockOpt_McastLoopback:
        Py_RETURN_BOOL(data.value.mcast_loopback);
        break;
    case PR_SockOpt_NoDelay:
        Py_RETURN_BOOL(data.value.no_delay);
        break;
    case PR_SockOpt_MaxSegment:
        return Py_BuildValue("I", data.value.max_segment);
        break;
    case PR_SockOpt_Broadcast:
        Py_RETURN_BOOL(data.value.broadcast);
        break;
    default:
        PyErr_SetString(PyExc_ValueError, "get_socket_option: unknown option");
        return NULL;
    }
    return NULL;
}

PyDoc_STRVAR(Socket_connect_doc,
"connect(addr, timeout=PR_INTERVAL_NO_TIMEOUT)\n\
\n\
:Parameters:\n\
    addr : NetworkAddress object\n\
        address to connect to\n\
    timeout : integer\n\
        optional timeout value expressed as a NSPR interval\n\
\n\
Socket.connect() is usually invoked on a TCP socket, but it may also\n\
be invoked on a UDP socket. Both cases are discussed here.\n\
\n\
If the socket is a TCP socket, Socket.connect() establishes a TCP\n\
connection to the peer. If the socket is not bound, it will be bound\n\
to an arbitrary local address.\n\
\n\
Socket.connect() blocks until either the connection is successfully\n\
established or an error occurs. If the timeout parameter is not\n\
PR_INTERVAL_NO_TIMEOUT and the connection setup cannot complete before\n\
the time limit, Socket.connect() fails with the error code\n\
PR_IO_TIMEOUT_ERROR.\n\
\n\
If the socket is a UDP socket, there is no connection setup to speak\n\
of, since UDP is connectionless. If Socket.connect() is invoked on a\n\
UDP socket, it has an overloaded meaning: Socket.connect() merely\n\
saves the specified address as the default peer address for the\n\
socket, so that subsequently one can send and receive datagrams from\n\
the socket using Socket.send() and Socket.recv() instead of the usual\n\
Socket.send_to() and Socket.recv_from().\n\
");

static PyObject *
Socket_connect(Socket *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"addr", "timeout", NULL};
    NetworkAddress *py_netaddr = NULL;
    unsigned int timeout = PR_INTERVAL_NO_TIMEOUT;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!|I:connect", kwlist,
                                     &NetworkAddressType, &py_netaddr, &timeout))
        return NULL;

    Py_XDECREF(self->py_netaddr);
    Py_INCREF(py_netaddr);
    self->py_netaddr = py_netaddr;

    Py_BEGIN_ALLOW_THREADS
    if (PR_Connect(self->pr_socket, &py_netaddr->addr, timeout) != PR_SUCCESS) {
        Py_BLOCK_THREADS
        Py_DECREF(self->py_netaddr);
        self->py_netaddr = NULL;
        return set_nspr_error(NULL);
    }
    Py_END_ALLOW_THREADS

    self->open_for_read = 1;
    Py_RETURN_NONE;
}

PyDoc_STRVAR(Socket_accept_doc,
"accept(timeout=PR_INTERVAL_NO_TIMEOUT) -> (Socket, NetworkAddress)\n\
\n\
:Parameters:\n\
    timeout : integer\n\
        optional timeout value expressed as a NSPR interval\n\
\n\
The socket is a rendezvous socket that has been bound to an address\n\
with Socket.bind() and is listening for connections after a call to\n\
Socket.listen(). Socket.accept() accepts the first connection from the\n\
queue of pending connections and creates a new socket for the newly\n\
accepted connection. The rendezvous socket can still be used to accept\n\
more connections.\n\
\n\
Socket.accept() blocks the calling thread until either a new\n\
connection is successfully accepted or an error occurs. If the timeout\n\
parameter is not PR_INTERVAL_NO_TIMEOUT and no pending connection can\n\
be accepted before the time limit, Socket.accept() raises a\n\
nss.error.NSPRError exception with the error code PR_IO_TIMEOUT_ERROR.\n\
\n\
Socket.accept() returns a tuple containing a new Socket object and\n\
Networkaddress object for the peer.\n\
");

static PyObject *
Socket_accept(Socket *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"timeout", NULL};
    unsigned int timeout = PR_INTERVAL_NO_TIMEOUT;
    PRNetAddr pr_netaddr;
    PyObject *py_socket = NULL;
    PyObject *py_netaddr = NULL;
    PRFileDesc *pr_socket = NULL;
    PyObject *return_values = NULL;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|I:accept", kwlist,
                                     &timeout))
        return NULL;

    Py_BEGIN_ALLOW_THREADS
    if ((pr_socket = PR_Accept(self->pr_socket, &pr_netaddr, timeout)) == NULL) {
        Py_BLOCK_THREADS
        return set_nspr_error(NULL);
    }
    Py_END_ALLOW_THREADS

    if ((py_netaddr = NetworkAddress_new_from_PRNetAddr(&pr_netaddr)) == NULL) {
        goto error;
    }

    if ((py_socket = Socket_new_from_PRFileDesc(pr_socket, self->family)) == NULL) {
        goto error;
    }

    if ((return_values = Py_BuildValue("OO", py_socket, py_netaddr)) == NULL) {
        goto error;
    }

    return return_values;

 error:
    Py_XDECREF(py_socket);
    Py_XDECREF(py_netaddr);
    Py_XDECREF(return_values);
    return NULL;
}

PyDoc_STRVAR(Socket_accept_read_doc,
"accept_read(amount, timeout=PR_INTERVAL_NO_TIMEOUT) -> (Socket, NetworkAddress, buf)\n\
\n\
:Parameters:\n\
    amount : integer\n\
        the maximum number of bytes to receive\n\
    timeout : integer\n\
        optional timeout value expressed as a NSPR interval\n\
\n\
Socket.accept_read() combines the behavior of Socket.accept() and\n\
Socket.recv(). It accepts a new connection and after it performs an\n\
initial read on the new socket as Socket.recv() would it returns the\n\
newly created Socket and NetworkAddress objects for the peer as well\n\
as a buffer of data.\n\
\n\
Socket.accept_read() returns a tuple containing a new Socket object, a\n\
new Networkaddress object for the peer, and a bufer containing data\n\
from the first read on the Socket object.\n\
");

static PyObject *
Socket_accept_read(Socket *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"amount", "timeout", NULL};
    int requested_amount = 0;
    unsigned int timeout = PR_INTERVAL_NO_TIMEOUT;
    int amount_read;
    PyObject *buf = NULL;
    PRNetAddr *pr_netaddr;
    PyObject *py_socket = NULL;
    PyObject *py_netaddr = NULL;
    PRFileDesc *pr_socket = NULL;
    PyObject *return_values = NULL;

    /* FIXME: for consistency should use readahead buffering, but since this is the first read
     * the readahead would be empty anyway */

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i|I:accept_read", kwlist,
                                     &requested_amount, &timeout))
        return NULL;

    if ((buf = PyString_FromStringAndSize(NULL, requested_amount)) == NULL)
        return NULL;

    Py_BEGIN_ALLOW_THREADS
    amount_read = PR_AcceptRead(self->pr_socket, &pr_socket, &pr_netaddr,
                                PyString_AS_STRING(buf), requested_amount,
                                timeout);
    Py_END_ALLOW_THREADS

    if (amount_read < 0) {
        set_nspr_error(NULL);
        goto error;
    }

    if (amount_read != requested_amount) {
        if (_PyString_Resize(&buf, amount_read) < 0) {
            goto error;
	}
    }

    if ((py_netaddr = NetworkAddress_new_from_PRNetAddr(pr_netaddr)) == NULL)
        goto error;

    if ((py_socket = Socket_new_from_PRFileDesc(pr_socket, self->family)) == NULL)
        goto error;

    if ((return_values = Py_BuildValue("OOO", py_socket, py_netaddr, buf)) == NULL)
        goto error;

    return return_values;

 error:
    Py_XDECREF(buf);
    Py_XDECREF(py_socket);
    Py_XDECREF(py_netaddr);
    Py_XDECREF(return_values);
    return NULL;
}

PyDoc_STRVAR(Socket_bind_doc,
"bind(addr)\n\
\n\
:Parameters:\n\
    addr : NetworkAddress object\n\
        address to bind to\n\
\n\
When a new socket is created, it has no address bound to\n\
it. Socket.bind() assigns the specified network address to the\n\
socket. If you do not care about the exact IP address assigned to the\n\
socket, create a NetworkAddress object using PR_INADDR_ANY. If you do\n\
not care about the TCP/UDP port assigned to the socket, set the port\n\
value of the NetworkAddress object to 0.\n\
\n\
Note that if Socket.connect() is invoked on a socket that is not\n\
bound, it implicitly binds an arbitrary address to the socket.\n\
\n\
Call Socket.get_sock_name to obtain the address (name) bound to a\n\
socket.\n\
");

static PyObject *
Socket_bind(Socket *self, PyObject *args)
{
    NetworkAddress *py_netaddr = NULL;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "O!:bind", &NetworkAddressType, &py_netaddr))
        return NULL;

    Py_XDECREF(self->py_netaddr);
    Py_INCREF(py_netaddr);
    self->py_netaddr = py_netaddr;

    Py_BEGIN_ALLOW_THREADS
    if (PR_Bind(self->pr_socket, &py_netaddr->addr) != PR_SUCCESS) {
        Py_BLOCK_THREADS
        Py_DECREF(self->py_netaddr);
        self->py_netaddr = NULL;
        return set_nspr_error(NULL);
    }
    Py_END_ALLOW_THREADS

    Py_RETURN_NONE;
}

PyDoc_STRVAR(Socket_listen_doc,
"listen(backlog=5)\n\
\n\
:Parameters:\n\
    backlog : integer\n\
        The maximum length of the queue of pending connections.\n\
\n\
Socket.listen() turns the specified socket into a rendezvous\n\
socket. It creates a queue for pending connections and starts to\n\
listen for connection requests on the socket. The maximum size of the\n\
queue for pending connections is specified by the backlog\n\
parameter. Pending connections may be accepted by calling\n\
Socket.accept().\n\
");

static PyObject *
Socket_listen(Socket *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"backlog", NULL};
    int backlog = 5;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|I:listen", kwlist, &backlog))
        return NULL;

    Py_BEGIN_ALLOW_THREADS
    if (PR_Listen(self->pr_socket, backlog) != PR_SUCCESS) {
        Py_BLOCK_THREADS
        return set_nspr_error(NULL);
    }
    Py_END_ALLOW_THREADS

    Py_RETURN_NONE;
}

PyDoc_STRVAR(Socket_shutdown_doc,
"shutdown(how=PR_SHUTDOWN_BOTH)\n\
\n\
:Parameters:\n\
    how : integer\n\
        The kind of disallowed operations on the socket.\n\
\n\
        May be one of the following the following:\n\
        \n\
        PR_SHUTDOWN_RCV\n\
            Further receives will be disallowed.\n\
        PR_SHUTDOWN_SEND\n\
            Further sends will be disallowed.\n\
        PR_SHUTDOWN_BOTH\n\
            Further sends and receives will be disallowed. \n\
");

static PyObject *
Socket_shutdown(Socket *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"how", NULL};
    int how = PR_SHUTDOWN_BOTH;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|I:shutdown", kwlist, &how))
        return NULL;

    Py_BEGIN_ALLOW_THREADS
    if (PR_Shutdown(self->pr_socket, how) != PR_SUCCESS) {
        Py_BLOCK_THREADS
        return set_nspr_error(NULL);
    }
    Py_END_ALLOW_THREADS

    if (how == PR_SHUTDOWN_RCV || how == PR_SHUTDOWN_BOTH) {
        self->open_for_read = 0;
    }

    Py_RETURN_NONE;
}

PyDoc_STRVAR(Socket_close_doc,
"close()\n\
\n\
Close the socket.\n\
");

static PyObject *
Socket_close(Socket *self, PyObject *args)
{
    TraceMethodEnter(self);

    if (self->makefile_refs > 0) {
        self->makefile_refs--;
        Py_RETURN_NONE;
    }

    self->makefile_refs = 0;

    Py_BEGIN_ALLOW_THREADS
    if (PR_Close(self->pr_socket) != PR_SUCCESS) {
        Py_BLOCK_THREADS
        return set_nspr_error(NULL);
    }
    Py_END_ALLOW_THREADS

    self->open_for_read = 0;
    self->pr_socket = NULL;
    Py_RETURN_NONE;
}

PyDoc_STRVAR(Socket_readline_doc,
"readline([size]) -> buf\n\
\n\
:Parameters:\n\
    size : integer\n\
        optional, read at most size bytes\n\
\n\
Read one entire line from the socket. If the size argument is present\n\
and non-negative, it is a maximum byte count (including the trailing\n\
newline) and an incomplete line may be returned. An empty string is\n\
returned on EOF (connection close). Note: Unlike stdio's fgets(), the\n\
returned string may contain null characters ('\0') if they occurred in\n\
the input.\n\
\n\
The trailing line ending character(s) are preserved in the string (but\n\
may be absent when a socket stream ends with an incomplete line). No\n\
line ending conversions are performed. This is because some network\n\
protocols require <CR><LF> sequences in some parts of the protocol\n\
stream but permit <LF> (e.g. newline) endings in encapsulated portions\n\
of the protocol. It is up to the caller to make line endings canonical\n\
or to strip them altogether if necessary for their application. Both\n\
operations are trival and not considered a burden in light of the need\n\
to read exact protocol sequences.\n\
");

static PyObject *
Socket_readline(Socket *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"size", NULL};
    long size = 0;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|l:readline", kwlist, &size))
        return NULL;

    return _readline(self, size);
}

static PyObject *
_readline(Socket *self, long size)
{
    unsigned int timeout = PR_INTERVAL_NO_TIMEOUT;
    long read_len, space_available, amount_read, line_len;
    PyObject *line = NULL;

    while (1) {
        /* Is there a complete line already buffered */
        if (self->readahead.len) {
            char *p, *beg, *end;
            /* Set the beginning and ending pointers which defines the
             * region inside of which a newline will be searched for. */
            beg = self->readahead.buf;
            if (size > 0) {
                end = beg + MIN(size, self->readahead.len);
            } else {
                end = beg + self->readahead.len;
            }
            /* Scan for a newline, stop at a newline or the limit of the scan, whichever comes first */
            for (p = beg; p < end && *p != '\n'; p++);
            line_len = p - beg;
            /* Where did we stop?
             * At the size limit? Then return size bytes.
             * At the end of the readahead buffer? No newline found, get more data.
             * At a newline? Then return from the buffer begininning up to and including the newline */
            if ((size > 0) && (line_len == size)) goto return_line;
            if (line_len == self->readahead.len)  goto more_data;
            assert(*p == '\n');			/* 1st two conditions don't apply, must have found newline */
            line_len++;				/* always include line ending chars */
            goto return_line;
        }
    more_data:
        /* Need more data, try to read at least an ALLOC_INCREMENT chunk */
        space_available = self->readahead.alloc_len - self->readahead.len;
        if (space_available < ALLOC_INCREMENT) {
            self->readahead.alloc_len = self->readahead.alloc_len + ALLOC_INCREMENT;
            if ((self->readahead.buf = realloc(self->readahead.buf, self->readahead.alloc_len)) == NULL) {
                /* ERROR */
                return PyErr_NoMemory();
            }
        }
        read_len = self->readahead.alloc_len - self->readahead.len;

        Py_BEGIN_ALLOW_THREADS
        amount_read = PR_Recv(self->pr_socket,
                              self->readahead.buf + self->readahead.len,
                              read_len, 0, timeout);
        Py_END_ALLOW_THREADS

        if (amount_read < 0) {
            return set_nspr_error(NULL);
        } else if (amount_read == 0) {				/* EOF, return what we've got */
            line_len = self->readahead.len;
            goto return_line;
        }
        self->readahead.len += amount_read;
        /* Got more data, go back and try again ... */
    }
    assert(0);                  /* should never reach here */
 return_line:
    if ((line = PyString_FromStringAndSize(self->readahead.buf, line_len)) == NULL) {
        return NULL;
    }
    memmove(PyString_AsString(line), self->readahead.buf, line_len);
    /* Subtract the data being returned from the cached readahead buffer */
    memmove(self->readahead.buf, self->readahead.buf + line_len, self->readahead.len - line_len);
    self->readahead.len -= line_len;
    return line;
}

PyDoc_STRVAR(Socket_readlines_doc,
"readlines([sizehint]) -> [buf]\n\
\n\
:Parameters:\n\
    sizehint : integer\n\
        optional, read approximately sizehint bytes before returning\n\
\n\
Read until EOF using Socket.readline() and return a list containing\n\
the lines thus read. If the optional sizehint argument is present and\n\
non-negative, instead of reading up to EOF, whole lines totalling\n\
approximately sizehint bytes are read.\n\
");

static PyObject *
Socket_readlines(Socket *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"sizehint", NULL};
    long sizehint = 0;
    PyObject *list;
    PyObject *line;
    Py_ssize_t line_len;
    Py_ssize_t amount_read = 0;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|l:readlines", kwlist, &sizehint))
        return NULL;

    if ((list = PyList_New(0)) == NULL)
        return NULL;

    while (1) {
        if ((line = _readline(self, 0)) == NULL) {
            Py_DECREF(list);
            return NULL;
        }

        if (PyList_Append(list, line) != 0) {
            Py_DECREF(line);
            Py_DECREF(list);
            return NULL;
        }
        Py_DECREF(line);
        line_len = PyString_Size(line);
        amount_read += line_len;
        if (sizehint > 0 && amount_read >= sizehint) break; /* read at least requested amount */
        if (line_len == 0) break; /* EOF */
    }

    return list;
}

static PyObject *
Socket_iter(Socket *self)
{
    if (!self->open_for_read) {
        return err_closed();
    }

    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *
Socket_iternext(Socket *self)
{
    PyObject *line;

    if ((line = _readline(self, 0)) == NULL) {
        return NULL;
    }

    if (PyString_Size(line) == 0) {
        Py_DECREF(line);
        return NULL;
    }
    return line;
}

PyDoc_STRVAR(Socket_recv_doc,
"recv(amount, timeout=PR_INTERVAL_NO_TIMEOUT) -> buf\n\
\n\
:Parameters:\n\
    amount : integer\n\
        the maximum number of bytes to receive\n\
    timeout : integer\n\
        optional timeout value expressed as a NSPR interval\n\
\n\
Socket.recv() blocks until some positive number of bytes are\n\
transferred, a timeout occurs, or an error occurs. No more than amount\n\
bytes will be transferred.\n\
\n\
If the length of the returned buffer is 0 this indicates the network\n\
connection is closed.\n\
");

static PyObject *
Socket_recv(Socket *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"amount", "timeout", NULL};
    long requested_amount = 0;
    unsigned int timeout = PR_INTERVAL_NO_TIMEOUT;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "l|I:recv", kwlist,
                                     &requested_amount, &timeout))
        return NULL;
    
    return _recv(self, requested_amount, timeout);
}

static PyObject *
_recv(Socket *self, long requested_amount, unsigned int timeout)
{
    PyObject *buf = NULL;
    long read_len, amount_read, result_len;
    char *dst = NULL;

    result_len = 0;
    read_len = requested_amount;
    if ((buf = PyString_FromStringAndSize(NULL, requested_amount)) == NULL)
        return NULL;

    dst = PyString_AsString(buf);

    /* Is the read request already buffered? */
    if (self->readahead.len) {
        if (self->readahead.len >= requested_amount) {
            result_len = requested_amount;
            memmove(dst, self->readahead.buf, result_len);
            if (self->readahead.len > result_len) {
                FREE_READAHEAD(&self->readahead);
            } else {
                memmove(self->readahead.buf, self->readahead.buf + result_len, self->readahead.len - result_len);
                self->readahead.len -= result_len;
            }
            return buf;
        }

        /* We'll completly empty the read ahead buffer satisfying this request so malloc
         * the result string now, copy the read ahead portion into it and then free the
         * read ahead. By eschewing the read ahead until it's needed again saves us an
         * extra buffer copy on each subsequent read until it's needed again. */

        memmove(dst, self->readahead.buf, self->readahead.len);
        dst += self->readahead.len;
        read_len = requested_amount - self->readahead.len;
        FREE_READAHEAD(&self->readahead);
    }

    /* Need more data */
    Py_BEGIN_ALLOW_THREADS
    amount_read = PR_Recv(self->pr_socket, dst, read_len, 0, timeout);
    Py_END_ALLOW_THREADS

    if (amount_read < 0) {
        Py_DECREF(buf);
        return set_nspr_error(NULL);
    }
    if (amount_read == 0) {
        /* EOF */
    }

    result_len += amount_read;

    if (result_len != requested_amount) {
        if (_PyString_Resize(&buf, result_len) < 0) {
            return NULL;
	}
    }
    return buf;
}

PyDoc_STRVAR(Socket_read_doc,
"read(size=-1)\n\
\n\
:Parameters:\n\
    size : integer\n\
        If specified and non-negative the maximum number of bytes to receive\n\
        otherwise read till EOF\n\
\n\
If the length of the returned buffer is 0 this indicates the network\n\
connection is closed.\n\
");

static PyObject *
Socket_read(Socket *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"size", NULL};
    long requested_amount = -1;
    unsigned int timeout = PR_INTERVAL_NO_TIMEOUT;
    PyObject *buf = NULL;
    long read_len, space_available, amount_read;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|l:read", kwlist, &requested_amount))
        return NULL;


    /* If read a specific amount then use recv */
    if (requested_amount >= 0) {
        return _recv(self, requested_amount, timeout);
    }

    /* Otherwise read until EOF */
    do {
        read_len = ALLOC_INCREMENT;
        space_available = self->readahead.alloc_len - self->readahead.len;
        if (space_available < read_len) {
            self->readahead.alloc_len = self->readahead.alloc_len + ALLOC_INCREMENT;
            if ((self->readahead.buf = PyMem_REALLOC(self->readahead.buf, self->readahead.alloc_len)) == NULL) {
                return PyErr_NoMemory();
            }
        }
        read_len = self->readahead.alloc_len - self->readahead.len;

        Py_BEGIN_ALLOW_THREADS
        amount_read = PR_Recv(self->pr_socket,
                              self->readahead.buf + self->readahead.len,
                              read_len, 0, timeout);
        Py_END_ALLOW_THREADS

        if (amount_read  < 0) {
            return set_nspr_error(NULL);
        }
        self->readahead.len += amount_read;

	if (self->readahead.len > PY_SSIZE_T_MAX) {
            PyErr_Format(PyExc_OverflowError, "have read %ld bytes, this is more than a Python string can hold", self->readahead.len);
            return NULL;
        }

    } while (amount_read != 0);

    if ((buf = PyString_FromStringAndSize(self->readahead.buf, self->readahead.len)) == NULL)
        return NULL;
    FREE_READAHEAD(&self->readahead);
    return buf;
}

PyDoc_STRVAR(Socket_recv_from_doc,
"recv_from(amount, addr, timeout=PR_INTERVAL_NO_TIMEOUT) -> buf\n\
\n\
:Parameters:\n\
    amount : integer\n\
        the maximum number of bytes to receive\n\
    addr : NetworkAddress object\n\
        a NetworkAddress object to receive from\n\
    timeout : integer\n\
        optional timeout value expressed as a NSPR interval\n\
\n\
Socket.recv_from() blocks until some positive number of bytes are\n\
transferred, a timeout occurs, or an error occurs. No more than amount\n\
bytes will be transferred.\n\
\n\
If the length of the returned buffer is 0 this indicates the network\n\
connection is closed.\n\
\n\
Note: Socket.recv_from() is usually used with a UDP socket.\n\
");

static PyObject *
Socket_recv_from(Socket *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"amount", "addr", "timeout", NULL};
    int requested_amount = 0;
    NetworkAddress *py_netaddr = NULL;
    unsigned int timeout = PR_INTERVAL_NO_TIMEOUT;
    int amount_read;
    PyObject *buf = NULL;

    /* FIXME: for consistency should use readahead buffering, but since this is the first read
     * the readahead would be empty anyway */

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "iO!|I:recv_from", kwlist,
                                     &requested_amount, &NetworkAddressType, &py_netaddr, &timeout))
        return NULL;

    Py_XDECREF(self->py_netaddr);
    Py_INCREF(py_netaddr);
    self->py_netaddr = py_netaddr;

    if ((buf = PyString_FromStringAndSize((char *) 0, requested_amount)) == NULL)
        return NULL;

    Py_BEGIN_ALLOW_THREADS
    amount_read = PR_RecvFrom(self->pr_socket, PyString_AS_STRING(buf),
                              requested_amount, 0, &py_netaddr->addr, timeout);
    Py_END_ALLOW_THREADS

    if (amount_read < 0) {
        Py_DECREF(self->py_netaddr);
        self->py_netaddr = NULL;
        Py_DECREF(buf);
        return set_nspr_error(NULL);
    }

    if (amount_read != requested_amount) {
        if (_PyString_Resize(&buf, amount_read) < 0) {
            return NULL;
	}
    }
    return buf;
}

PyDoc_STRVAR(Socket_send_doc,
"send(buf, timeout=PR_INTERVAL_MIN) -> amount\n\
\n\
:Parameters:\n\
    buf : buffer\n\
        a buffer of data to transmit\n\
    timeout : integer\n\
        optional timeout value expressed as a NSPR interval\n\
\n\
Socket.send() blocks until all bytes are sent (unless the socket is in\n\
non-blocking mode), a timeout occurs, or an error occurs. In the case\n\
of a timeout or an error then a nss.error.NSPRError will be raised.\n\
\n\
The function returns the number of bytes actually transmitted.\n\
");

static PyObject *
Socket_send(Socket *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"buf", "timeout", NULL};
    char *buf = NULL;
    Py_ssize_t len = 0;
    unsigned int timeout = PR_INTERVAL_MIN;
    int amount;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s#|I:send", kwlist,
                                     &buf, &len, &timeout))
        return NULL;

    Py_BEGIN_ALLOW_THREADS
    amount = PR_Send(self->pr_socket, buf, len, 0, timeout);
    Py_END_ALLOW_THREADS

    if (amount < 0) {
        return set_nspr_error(NULL);
    }

    return PyInt_FromLong(amount);
}

PyDoc_STRVAR(Socket_sendall_doc,
"sendall(buf, timeout=PR_INTERVAL_NO_TIMEOUT) -> amount\n\
\n\
:Parameters:\n\
    buf : buffer\n\
        a buffer of data to transmit\n\
    timeout : integer\n\
        optional timeout value expressed as a NSPR interval\n\
\n\
Socket.sendall() blocks until all bytes are sent (unless the socket is in\n\
non-blocking mode), a timeout occurs, or an error occurs. In the case\n\
of a timeout or an error then a nss.error.NSPRError will be raised.\n\
\n\
The function returns the number of bytes actually transmitted.\n\
");

static PyObject *
Socket_sendall(Socket *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"buf", "timeout", NULL};
    char *buf = NULL;
    Py_ssize_t len = 0;
    unsigned int timeout = PR_INTERVAL_NO_TIMEOUT;
    int amount;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s#|I:sendall", kwlist,
                                     &buf, &len, &timeout))
        return NULL;

    Py_BEGIN_ALLOW_THREADS
    amount = PR_Send(self->pr_socket, buf, len, 0, timeout);
    Py_END_ALLOW_THREADS

    if (amount < 0) {
        return set_nspr_error(NULL);
    }

    return PyInt_FromLong(amount);
}

PyDoc_STRVAR(Socket_send_to_doc,
"send_to(buf, addr, timeout=PR_INTERVAL_NO_TIMEOUT) -> amount\n\
\n\
:Parameters:\n\
    buf : buffer\n\
        a buffer of data to transmit\n\
    addr : NetworkAddress object\n\
        a NetworkAddress object to send to\n\
    timeout : integer\n\
        optional timeout value expressed as a NSPR interval\n\
\n\
Socket.send_to() blocks until all bytes are sent (unless the socket is in\n\
non-blocking mode), a timeout occurs, or an error occurs. In the case\n\
of a timeout or an error then a nss.error.NSPRError will be raised.\n\
\n\
The function returns the number of bytes actually transmitted.\n\
\n\
Note: Socket.send_to() is usually used with a UDP socket.\n\
");

static PyObject *
Socket_send_to(Socket *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"buf", "addr", "timeout", NULL};
    char *buf = NULL;
    Py_ssize_t len = 0;
    NetworkAddress *py_netaddr = NULL;
    unsigned int timeout = PR_INTERVAL_NO_TIMEOUT;
    int amount;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s#O!|I:send_to", kwlist,
                                     &buf, &len, &NetworkAddressType, &py_netaddr, &timeout))
        return NULL;

    Py_XDECREF(self->py_netaddr);
    Py_INCREF(py_netaddr);
    self->py_netaddr = py_netaddr;

    Py_BEGIN_ALLOW_THREADS
    amount = PR_SendTo(self->pr_socket, buf, len, 0, &py_netaddr->addr, timeout);
    Py_END_ALLOW_THREADS

    if (amount < 0) {
        Py_DECREF(self->py_netaddr);
        self->py_netaddr = NULL;
        return set_nspr_error(NULL);
    }

    return PyInt_FromLong(amount);
}

PyDoc_STRVAR(Socket_get_sock_name_doc,
"get_sock_name() -> NetworkAddress\n\
\n\
Return the network address for this socket.\n\
");

static PyObject *
Socket_get_sock_name(Socket *self, PyObject *args)
{
    PRNetAddr pr_netaddr;
    PyObject *py_netaddr = NULL;

    Py_BEGIN_ALLOW_THREADS
    if (PR_GetSockName(self->pr_socket, &pr_netaddr) != PR_SUCCESS) {
        Py_BLOCK_THREADS
        return set_nspr_error(NULL);
    }
    Py_END_ALLOW_THREADS

    if ((py_netaddr = NetworkAddress_new_from_PRNetAddr(&pr_netaddr)) == NULL)
        return NULL;

    return py_netaddr;
}

PyDoc_STRVAR(Socket_get_peer_name_doc,
"get_peer_name() -> NetworkAddress\n\
\n\
Return the network address for the connected peer socket.\n\
");

static PyObject *
Socket_get_peer_name(Socket *self, PyObject *args)
{
    PRNetAddr pr_netaddr;
    PyObject *py_netaddr = NULL;

    if (PR_GetPeerName(self->pr_socket, &pr_netaddr) != PR_SUCCESS)
        return set_nspr_error(NULL);
    if ((py_netaddr = NetworkAddress_new_from_PRNetAddr(&pr_netaddr)) == NULL)
        return NULL;

    return py_netaddr;
}

PyDoc_STRVAR(Socket_fileno_doc,
"fileno() -> integer\n\
\n\
Return the integer file descriptor of the socket.\n\
");

static PyObject *
Socket_fileno(Socket *self)
{
    PROsfd sock_fd = -1;

    if ((sock_fd = PR_FileDesc2NativeHandle(self->pr_socket)) == -1) {
        return set_nspr_error(NULL);
    }

#if SIZEOF_SOCKET_T <= SIZEOF_LONG
    return PyInt_FromLong((long)sock_fd);
#else
    return PyLong_FromLongLong((PY_LONG_LONG)sock_fd);
#endif
}

PyDoc_STRVAR(Socket_makefile_doc,
"makefile([mode[, buffersize]]) -> file object\n\
\n\
:Parameters:\n\
    mode : string\n\
        mode string identical to open(), e.g. 'r','w','rb', etc.\n\
    buffersize : integer\n\
        file buffer size\n\
\n\
Return a regular file object corresponding to the socket.\n\
The mode and buffersize arguments are as for the built-in open() function.");

static PyObject *
Socket_makefile(Socket *self, PyObject *args)
{
    char *mode = "r";
    int bufsize = -1;

    if (!PyArg_ParseTuple(args, "|si:makefile", &mode, &bufsize))
        return NULL;

    self->makefile_refs++;
    Py_INCREF(self);
    return (PyObject *)self;
}

PyDoc_STRVAR(Socket_new_tcp_pair_doc,
"new_tcp_pair() -> (Socket, Socket)\n\
\n\
Returns a pair of connected TCP sockets: data written to one can be read from\n\
the other and vice versa.\n\
");

static PyObject *
Socket_new_tcp_pair(Socket *self, PyObject *args)
{
    PRFileDesc *socks[2];
    PRNetAddr addr0, addr1;
    PyObject *py_sock0 = NULL, *py_sock1 = NULL;
    PyObject *return_value = NULL;

    TraceMethodEnter(self);

    if (PR_NewTCPSocketPair(socks) != PR_SUCCESS)
        return set_nspr_error(NULL);

    Py_BEGIN_ALLOW_THREADS
    if (PR_GetSockName(socks[0], &addr0) != PR_SUCCESS ||
        PR_GetSockName(socks[1], &addr1) != PR_SUCCESS) {
        Py_BLOCK_THREADS
	return_value = set_nspr_error(NULL);
	goto error_socks;
    }
    Py_END_ALLOW_THREADS

    if ((py_sock0 = Socket_new_from_PRFileDesc(socks[0],
					       PR_NetAddrFamily(&addr0)))
	== NULL)
	goto error_socks;
    if ((py_sock1 = Socket_new_from_PRFileDesc(socks[1],
					       PR_NetAddrFamily(&addr1)))
	== NULL)
	goto error_socks1;
    if ((return_value = Py_BuildValue("OO", py_sock0, py_sock1)) == NULL)
        goto error;

    return return_value;

 error_socks:
    PR_Close(socks[0]);
 error_socks1:
    PR_Close(socks[1]);
 error:
    Py_XDECREF(py_sock0);
    Py_XDECREF(py_sock1);
    return return_value;
}

PyDoc_STRVAR(Socket_poll_doc,
"poll(poll_descs, timeout) -> (flags, ...)\n\
\n\
:Parameters:\n\
    poll_descs : sequence of (Socket, flags) sequences\n\
        flags is a bitwise OR of PR_POLL_* flags\n\
    timeout : interval time\n\
        how long to block\n\
\n\
Wait until at least one of the Socket objects is ready for the action in\n\
flags.  Return a sequence of flags values, each representing the state of\n\
the corresponding Socket in poll_descs.\n\
");

static PyObject *
Socket_poll(Socket *unused_class, PyObject *args)
{
    PyObject *py_descs, *return_value = NULL, *py_desc = NULL, *o = NULL;
    Py_ssize_t num_descs;
    PRPollDesc *descs;
    unsigned int timeout;
    size_t i;

    if (!PyArg_ParseTuple(args, "OI:poll", &py_descs, &timeout))
	return NULL;

    if (!PySequence_Check(py_descs)
	|| (num_descs = PySequence_Size(py_descs)) == -1) {
	PyErr_SetString(PyExc_TypeError,
			"poll_descs is not a suitable sequence");
	return NULL;
    }

    descs = PyMem_New(PRPollDesc, num_descs);
    if (descs == NULL)
	return PyErr_NoMemory();

    for (i = 0; i < num_descs; i++) {
	PyObject *py_desc, *o;
	long flags;

	py_desc = PySequence_GetItem(py_descs, i);
	if (py_desc == NULL)
	    goto err_descs_TypeError;

	o = PySequence_GetItem(py_desc, 0);
	if (o == NULL)
	    goto err_descs;
	if (!PyObject_TypeCheck(o, &SocketType))
	    goto err_descs_TypeError;
	descs[i].fd = ((Socket *)o)->pr_socket;
	Py_CLEAR(o);

	o = PySequence_GetItem(py_desc, 1);
	if (o == NULL)
	    goto err_descs;
	flags = PyInt_AsLong(o);
	if (flags == -1 && PyErr_Occurred())
	    goto err_descs;
	descs[i].in_flags = flags;
	if (descs[i].in_flags != flags) /* Overflow */
	    goto err_descs_TypeError;

	Py_CLEAR(py_desc);
    }

    Py_BEGIN_ALLOW_THREADS
    if (PR_Poll(descs, num_descs, timeout) == -1) {
        Py_BLOCK_THREADS
	set_nspr_error(NULL);
	goto err_descs;
    }
    Py_END_ALLOW_THREADS

    return_value = PyTuple_New(num_descs);
    if (return_value == NULL)
	goto err_descs;
    for (i = 0; i < num_descs; i++)
	PyTuple_SET_ITEM(return_value, i, PyInt_FromLong(descs[i].out_flags));

    PyMem_Del(descs);
    return return_value;

 err_descs_TypeError:
    PyErr_SetString(PyExc_TypeError, "Invalid content of poll_descs");
 err_descs:
    PyMem_Del(descs);
    Py_XDECREF(py_desc);
    Py_XDECREF(o);
    Py_XDECREF(return_value);
    return NULL;
}

PyDoc_STRVAR(Socket_import_tcp_socket_doc,
"import_tcp_socket(osfd) -> Socket\n\
:Parameters:\n\
    osfd : integer\n\
        file descriptor of the SOCK_STREAM socket to import\n\
\n\
Returns a Socket object that uses the specified socket file descriptor for\n\
communication.\n\
");

static PyObject *
Socket_import_tcp_socket(Socket *unused_class, PyObject *args)
{
    int osfd;
    PRFileDesc *sock;
    PRNetAddr addr;
    PyObject *return_value = NULL;

    if (!PyArg_ParseTuple(args, "i:import_tcp_socket", &osfd))
	return NULL;

    sock = PR_ImportTCPSocket(osfd);
    if (sock == NULL)
	return set_nspr_error(NULL);

    Py_BEGIN_ALLOW_THREADS
    if (PR_GetSockName(sock, &addr) != PR_SUCCESS) {
        Py_BLOCK_THREADS
	return_value = set_nspr_error(NULL);
	goto error;
    }
    Py_END_ALLOW_THREADS

    if ((return_value = Socket_new_from_PRFileDesc(sock,
						   PR_NetAddrFamily(&addr)))
	== NULL)
	goto error;

    return return_value;

 error:
    PR_Close(sock);
    return NULL;
}

static PyMethodDef
Socket_methods[] = {
    {"set_socket_option", (PyCFunction)Socket_set_socket_option, METH_VARARGS,               Socket_set_socket_option_doc},
    {"get_socket_option", (PyCFunction)Socket_get_socket_option, METH_VARARGS,               Socket_get_socket_option_doc},
    {"connect",           (PyCFunction)Socket_connect,           METH_VARARGS|METH_KEYWORDS, Socket_connect_doc},
    {"accept",            (PyCFunction)Socket_accept,            METH_VARARGS|METH_KEYWORDS, Socket_accept_doc},
    {"accept_read",       (PyCFunction)Socket_accept_read,       METH_VARARGS|METH_KEYWORDS, Socket_accept_read_doc},
    {"bind",              (PyCFunction)Socket_bind,              METH_VARARGS,               Socket_bind_doc},
    {"listen",            (PyCFunction)Socket_listen,            METH_VARARGS|METH_KEYWORDS, Socket_listen_doc},
    {"shutdown",          (PyCFunction)Socket_shutdown,          METH_VARARGS|METH_KEYWORDS, Socket_shutdown_doc},
    {"close"   ,          (PyCFunction)Socket_close,             METH_NOARGS,                Socket_close_doc},
    {"recv",              (PyCFunction)Socket_recv,              METH_VARARGS|METH_KEYWORDS, Socket_recv_doc},
    {"read",              (PyCFunction)Socket_read,              METH_VARARGS|METH_KEYWORDS, Socket_read_doc},
    {"readline",          (PyCFunction)Socket_readline,          METH_VARARGS|METH_KEYWORDS, Socket_readline_doc},
    {"readlines",         (PyCFunction)Socket_readlines,         METH_VARARGS|METH_KEYWORDS, Socket_readlines_doc},
    {"recv_from",         (PyCFunction)Socket_recv_from,         METH_VARARGS|METH_KEYWORDS, Socket_recv_from_doc},
    {"send",              (PyCFunction)Socket_send,              METH_VARARGS|METH_KEYWORDS, Socket_send_doc},
    {"sendall",           (PyCFunction)Socket_sendall,           METH_VARARGS|METH_KEYWORDS, Socket_sendall_doc},
    {"send_to",           (PyCFunction)Socket_send_to,           METH_VARARGS|METH_KEYWORDS, Socket_send_to_doc},
    {"get_sock_name",     (PyCFunction)Socket_get_sock_name,     METH_NOARGS,                Socket_get_sock_name_doc},
    {"get_peer_name",     (PyCFunction)Socket_get_peer_name,     METH_NOARGS,                Socket_get_peer_name_doc},
    {"fileno",            (PyCFunction)Socket_fileno,            METH_NOARGS,                Socket_fileno_doc},
#ifndef NO_DUP
    {"makefile",          (PyCFunction)Socket_makefile,          METH_VARARGS,               Socket_makefile_doc},
#endif
    {"new_tcp_pair",      (PyCFunction)Socket_new_tcp_pair,      METH_NOARGS|METH_STATIC,    Socket_new_tcp_pair_doc},
    {"poll"        ,      (PyCFunction)Socket_poll,              METH_VARARGS|METH_STATIC,   Socket_poll_doc},
    {"import_tcp_socket", (PyCFunction)Socket_import_tcp_socket, METH_VARARGS|METH_STATIC,   Socket_import_tcp_socket_doc},
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
Socket_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Socket *self;

    TraceObjNewEnter(type);

    if ((self = (Socket *)type->tp_alloc(type, 0)) == NULL) return NULL;
    self->pr_socket = NULL;
    self->family = 0;
    self->py_netaddr = NULL;
    self->makefile_refs = 0;
    self->open_for_read = 0;
    INIT_READAHEAD(&self->readahead);

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static void
Socket_dealloc(Socket* self)
{
    TraceMethodEnter(self);

    Py_XDECREF(self->py_netaddr);
    FREE_READAHEAD(&self->readahead);
    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(Socket_doc,
"Socket(family=PR_AF_INET, type=PR_DESC_SOCKET_TCP)\n\
\n\
:Parameters:\n\
    family : integer\n\
        one of:\n\
            - PR_AF_INET\n\
            - PR_AF_INET6\n\
            - PR_AF_LOCAL\n\
    type : integer\n\
        one of:\n\
            - PR_DESC_SOCKET_TCP\n\
            - PR_DESC_SOCKET_UDP\n\
\n\
Create a new NSPR socket:\n\
\n\
");

static int
Socket_init(Socket *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"family", "type", NULL};
    int family = PR_AF_INET;
    int desc_type = PR_DESC_SOCKET_TCP;
    PRFileDesc *pr_socket = NULL;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ii", kwlist,
                                     &family, &desc_type))
        return -1;

    /* If reinitializing, first close down previous socket */
    if (self->pr_socket) {
        if (PR_Shutdown(self->pr_socket, PR_SHUTDOWN_BOTH) != PR_SUCCESS) {
            /* ignore error */
        }
        if (PR_Close(self->pr_socket) != PR_SUCCESS) {
            /* ignore error */
        }
        self->pr_socket = NULL;
    }


    switch (desc_type) {
    case PR_DESC_SOCKET_TCP:
        if ((pr_socket = PR_OpenTCPSocket(family)) == NULL) {
            set_nspr_error(NULL);
            return -1;
        }
        break;
    case PR_DESC_SOCKET_UDP:
        if ((pr_socket = PR_OpenUDPSocket(family)) == NULL) {
            set_nspr_error(NULL);
            return -1;
        }
        break;
    default:
        PyErr_SetString(PyExc_ValueError, "type must be PR_DESC_SOCKET_TCP or PR_DESC_SOCKET_UDP");
        return -1;
    }


    Socket_init_from_PRFileDesc(self, pr_socket, family);

    return 0;
}

static PyObject *
Socket_repr(Socket *self)
{
    return PyString_FromFormat("<%s object at %p PRFileDesc %p>",
                               self->ob_type->tp_name, self, self->pr_socket);
}

static PyObject *
Socket_str(Socket *self)
{
    PyObject *args = NULL;
    PyObject *text = NULL;

    args = Py_BuildValue("(ss)",
                         pr_family_str(self->family),
                         pr_file_desc_type_str(PR_GetDescType(self->pr_socket)));
    if (!args) goto error;
    text = PyString_Format(PyString_FromString("family=%s type=%s"), args);

    Py_DECREF(args);
    return text;

 error:
    Py_XDECREF(args);
    Py_XDECREF(text);
    return NULL;

}

static PyTypeObject
SocketType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.io.Socket",				/* tp_name */
    sizeof(Socket),				/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)Socket_dealloc,			/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    0,						/* tp_compare */
    (reprfunc)Socket_repr,			/* tp_repr */
    0,						/* tp_as_number */
    0,						/* tp_as_sequence */
    0,						/* tp_as_mapping */
    0,						/* tp_hash */
    0,						/* tp_call */
    (reprfunc)Socket_str,			/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
    Socket_doc,					/* tp_doc */
    0,						/* tp_traverse */
    0,						/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    (getiterfunc)Socket_iter,			/* tp_iter */
    (iternextfunc)Socket_iternext,		/* tp_iternext */
    Socket_methods,				/* tp_methods */
    Socket_members,				/* tp_members */
    Socket_getseters,				/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)Socket_init,			/* tp_init */
    0,						/* tp_alloc */
    Socket_new,					/* tp_new */
};

/* ========================================================================== */
/* ================================= Module ================================= */
/* ========================================================================== */

/* ============================== Module Methods ============================= */


PyDoc_STRVAR(io_ntohs_doc, "16 bit conversion from network to host");
static PyObject *
io_ntohs(PyObject *self, PyObject *args)
{
    int net, host;

    if (!PyArg_ParseTuple(args, "i:ntohs", &net)) {
        return NULL;
    }
    host = PR_ntohs(net);
    return PyInt_FromLong(host);
}

PyDoc_STRVAR(io_ntohl_doc, "32 bit conversion from network to host");
static PyObject *
io_ntohl(PyObject *self, PyObject *args)
{
    int net, host;

    if (!PyArg_ParseTuple(args, "i:ntohl", &net)) {
        return NULL;
    }
    host = PR_ntohl(net);
    return PyInt_FromLong(host);
}

PyDoc_STRVAR(io_htons_doc, "16 bit conversion from host to network");
static PyObject *
io_htons(PyObject *self, PyObject *args)
{
    int host, net ;

    if (!PyArg_ParseTuple(args, "i:htons", &host)) {
        return NULL;
    }
    net = PR_htons(host);
    return PyInt_FromLong(net);
}

PyDoc_STRVAR(io_htonl_doc, "32 bit conversion from host to network");
static PyObject *
io_htonl(PyObject *self, PyObject *args)
{
    int host, net ;

    if (!PyArg_ParseTuple(args, "i:htonl", &host)) {
        return NULL;
    }
    net = PR_htonl(host);
    return PyInt_FromLong(net);
}

// FIXME: the PR_GetProto* functions return success even if they fail

PyDoc_STRVAR(io_get_proto_by_name_doc,
"Returns the protocol number given the protocol's name. Raises exception if lookup fails.");
static PyObject *
io_get_proto_by_name(PyObject *self, PyObject *args)
{
    int rv;
    char *name;
    char buffer[PR_NETDB_BUF_SIZE]; /* this is where data pointed to in PRProtoEnt is stored */
    PRProtoEnt proto_ent;

    if (!PyArg_ParseTuple(args, "s:get_proto_by_name", &name)) {
        return NULL;
    }

    if ((rv = PR_GetProtoByName(name, buffer, sizeof(buffer), &proto_ent)) == PR_FAILURE) {
        return set_nspr_error(NULL);
    }

    return PyInt_FromLong(proto_ent.p_num);
}


PyDoc_STRVAR(io_get_proto_by_number_doc,
"Returns the protocol name and a tuple of aliases given the protocol's number.\n\
name, aliases = get_proto_by_number(number)\n\
Raises an exception if the lookup fails.");
static PyObject *
io_get_proto_by_number(PyObject *self, PyObject *args)
{
    int rv;
    int number;
    char buffer[PR_NETDB_BUF_SIZE]; /* this is where data pointed to in PRProtoEnt is stored */
    PRProtoEnt proto_ent;
    int len, i;
    PyObject *alias_tuple = NULL;
    PyObject *alias = NULL;
    PyObject *return_values;

    if (!PyArg_ParseTuple(args, "i:get_proto_by_number", &number)) {
        return NULL;
    }

    if ((rv = PR_GetProtoByNumber(number, buffer, sizeof(buffer), &proto_ent)) == PR_FAILURE) {
        return set_nspr_error(NULL);
    }

    for (len = 0; proto_ent.p_aliases[len]; len++);

    if ((alias_tuple = PyTuple_New(len)) == NULL)
        return NULL;

    for (i = 0; i < len; i++) {
        if ((alias = PyString_FromString(proto_ent.p_aliases[i])) == NULL) {
            Py_DECREF(alias_tuple);
            return NULL;
        }
        PyTuple_SetItem(alias_tuple, i, alias);
    }

    if ((return_values = Py_BuildValue("sO", proto_ent.p_name, alias_tuple)) == NULL)
        return NULL;

    Py_DECREF(alias_tuple);

    return return_values;
}

PyDoc_STRVAR(io_interval_now_doc,
"You can use the value returned by interval_now() to establish epochs\n\
and to determine intervals (that is, compute the difference\n\
between two times)");
static PyObject *
io_interval_now(PyObject *self, PyObject *args)
{
    PRIntervalTime interval;

    interval = PR_IntervalNow();
    return PyInt_FromLong(interval);
}

PyDoc_STRVAR(io_ticks_per_second_doc,
"An integer between 1000 and 100000 indicating the number of ticks per\n\
second counted by PRIntervalTime on the current platform.\n\
\n\
The value returned by ticks_per_second() lies between PR_INTERVAL_MIN\n\
and PR_INTERVAL_MAX.\n\
 ");
static PyObject *
io_ticks_per_second(PyObject *self, PyObject *args)
{
    PRUint32 ticks_per_second;

    ticks_per_second = PR_TicksPerSecond();
    return PyInt_FromLong(ticks_per_second);
}

PyDoc_STRVAR(io_seconds_to_interval_doc,
"Converts standard clock seconds to platform-dependent intervals.");
static PyObject *
io_seconds_to_interval(PyObject *self, PyObject *args)
{
    PRIntervalTime interval;
    unsigned int seconds;

    if (!PyArg_ParseTuple(args, "I:seconds_to_interval", &seconds)) {
        return NULL;
    }

    interval = PR_SecondsToInterval(seconds);
    return PyInt_FromLong(interval);
}

PyDoc_STRVAR(io_milliseconds_to_interval_doc,
"Converts standard clock milliseconds to platform-dependent intervals.");
static PyObject *
io_milliseconds_to_interval(PyObject *self, PyObject *args)
{
    PRIntervalTime interval;
    unsigned int milliseconds;

    if (!PyArg_ParseTuple(args, "I:milliseconds_to_interval", &milliseconds)) {
        return NULL;
    }

    interval = PR_MillisecondsToInterval(milliseconds);
    return PyInt_FromLong(interval);
}


PyDoc_STRVAR(io_microseconds_to_interval_doc,
"Converts standard clock microseconds to platform-dependent intervals.");
static PyObject *
io_microseconds_to_interval(PyObject *self, PyObject *args)
{
    PRIntervalTime interval;
    unsigned int microseconds;

    if (!PyArg_ParseTuple(args, "I:microseconds_to_interval", &microseconds)) {
        return NULL;
    }

    interval = PR_MicrosecondsToInterval(microseconds);
    return PyInt_FromLong(interval);
}

PyDoc_STRVAR(io_interval_to_seconds_doc,
"Converts platform-dependent intervals to standard clock seconds");
static PyObject *
io_interval_to_seconds(PyObject *self, PyObject *args)
{
    unsigned int interval;
    unsigned int seconds;

    if (!PyArg_ParseTuple(args, "I:interval_to_seconds", &interval)) {
        return NULL;
    }

    seconds = PR_IntervalToSeconds(interval);
    return PyInt_FromLong(seconds);
}

PyDoc_STRVAR(io_interval_to_milliseconds_doc,
"Converts platform-dependent intervals to standard clock milliseconds");
static PyObject *
io_interval_to_milliseconds(PyObject *self, PyObject *args)
{
    unsigned int interval;
    unsigned int milliseconds;

    if (!PyArg_ParseTuple(args, "I:interval_to_milliseconds", &interval)) {
        return NULL;
    }

    milliseconds = PR_IntervalToMilliseconds(interval);
    return PyInt_FromLong(milliseconds);
}

PyDoc_STRVAR(io_interval_to_microseconds_doc,
"Converts platform-dependent intervals to standard clock microseconds");
static PyObject *
io_interval_to_microseconds(PyObject *self, PyObject *args)
{
    unsigned int interval;
    unsigned int microseconds;

    if (!PyArg_ParseTuple(args, "I:interval_to_microseconds", &interval)) {
        return NULL;
    }

    microseconds = PR_IntervalToMicroseconds(interval);
    return PyInt_FromLong(microseconds);
}

/* List of functions exported by this module. */
static PyMethodDef
module_methods[] = {
    {"ntohs",                        io_ntohs,                    METH_VARARGS, io_ntohs_doc},
    {"ntohl",                        io_ntohl,                    METH_VARARGS, io_ntohl_doc},
    {"htons",                        io_htons,                    METH_VARARGS, io_htons_doc},
    {"htonl",                        io_htonl,                    METH_VARARGS, io_htonl_doc},
    {"get_proto_by_name",            io_get_proto_by_name,        METH_VARARGS, io_get_proto_by_name_doc},
    {"get_proto_by_number",          io_get_proto_by_number,      METH_VARARGS, io_get_proto_by_number_doc},
    {"interval_now",                 io_interval_now,             METH_NOARGS,  io_interval_now_doc},
    {"ticks_per_second",             io_ticks_per_second,         METH_NOARGS,  io_ticks_per_second_doc},
    {"seconds_to_interval",          io_seconds_to_interval,      METH_VARARGS, io_seconds_to_interval_doc},
    {"milliseconds_to_interval",     io_milliseconds_to_interval, METH_VARARGS, io_milliseconds_to_interval_doc},
    {"microseconds_to_interval",     io_microseconds_to_interval, METH_VARARGS, io_microseconds_to_interval_doc},
    {"interval_to_seconds",          io_interval_to_seconds,      METH_VARARGS, io_interval_to_seconds_doc},
    {"interval_to_milliseconds",     io_interval_to_milliseconds, METH_VARARGS, io_interval_to_milliseconds_doc},
    {"interval_to_microseconds",     io_interval_to_microseconds, METH_VARARGS, io_interval_to_microseconds_doc},
    {NULL,                  NULL}            /* Sentinel */
};

/* ============================== Module Exports ============================= */

static PyNSPR_IO_C_API_Type nspr_io_c_api =
{
    &NetworkAddressType,        /* network_address_type */
    &HostEntryType,             /* host_entry_type */
    &SocketType,                /* socket_type */
    Socket_init_from_PRFileDesc /* Socket_init_from_PRFileDesc */
};

/* ============================== Module Construction ============================= */

PyDoc_STRVAR(module_doc,
"This module implements the NSPR IO functions\n\
\n\
");

PyMODINIT_FUNC
initio(void)
{
    PyObject *m;

    if (import_nspr_error_c_api() < 0)
        return;

    if (PyType_Ready(&NetworkAddressType) < 0)
        return;

    if (PyType_Ready(&HostEntryType) < 0)
        return;

    if (PyType_Ready(&SocketType) < 0)
        return;

    if ((m = Py_InitModule3("io", module_methods, module_doc)) == NULL)
        return;

    Py_INCREF(&NetworkAddressType);
    PyModule_AddObject(m, "NetworkAddress", (PyObject *)&NetworkAddressType);

    Py_INCREF(&HostEntryType);
    PyModule_AddObject(m, "HostEntry", (PyObject *)&HostEntryType);

    Py_INCREF(&SocketType);
    PyModule_AddObject(m, "Socket", (PyObject *)&SocketType);

    /* Export C API */
    if (PyModule_AddObject(m, "_C_API", PyCObject_FromVoidPtr((void *)&nspr_io_c_api, NULL)) != 0)
        return;

    /* Socket types */
    AddIntConstant(PR_AF_INET);
    AddIntConstant(PR_AF_INET6);
    AddIntConstant(PR_AF_LOCAL);
    AddIntConstant(PR_AF_UNSPEC);

    /* PR_InitializeNetAddr */
    AddIntConstant(PR_IpAddrNull);
    AddIntConstant(PR_IpAddrAny);
    AddIntConstant(PR_IpAddrLoopback);

    /* PR_Shutdown */
    AddIntConstant(PR_SHUTDOWN_RCV);
    AddIntConstant(PR_SHUTDOWN_SEND);
    AddIntConstant(PR_SHUTDOWN_BOTH);


    /* PRDescType */
    AddIntConstant(PR_DESC_FILE);
    AddIntConstant(PR_DESC_SOCKET_TCP);
    AddIntConstant(PR_DESC_SOCKET_UDP);
    AddIntConstant(PR_DESC_LAYERED);
    AddIntConstant(PR_DESC_PIPE);

    /* PRSockOption */
    AddIntConstant(PR_SockOpt_Nonblocking);
    AddIntConstant(PR_SockOpt_Linger);
    AddIntConstant(PR_SockOpt_Reuseaddr);
    AddIntConstant(PR_SockOpt_Keepalive);
    AddIntConstant(PR_SockOpt_RecvBufferSize);
    AddIntConstant(PR_SockOpt_SendBufferSize);
    AddIntConstant(PR_SockOpt_IpTimeToLive);
    AddIntConstant(PR_SockOpt_IpTypeOfService);
    AddIntConstant(PR_SockOpt_AddMember);
    AddIntConstant(PR_SockOpt_DropMember);
    AddIntConstant(PR_SockOpt_McastInterface);
    AddIntConstant(PR_SockOpt_McastTimeToLive);
    AddIntConstant(PR_SockOpt_McastLoopback);
    AddIntConstant(PR_SockOpt_NoDelay);
    AddIntConstant(PR_SockOpt_MaxSegment);
    AddIntConstant(PR_SockOpt_Broadcast);

    /* Interval */
    AddIntConstant(PR_INTERVAL_MIN);
    AddIntConstant(PR_INTERVAL_MAX);
    AddIntConstant(PR_INTERVAL_NO_WAIT);
    AddIntConstant(PR_INTERVAL_NO_TIMEOUT);

    /* Interval */
    AddIntConstant(PR_POLL_READ);
    AddIntConstant(PR_POLL_WRITE);
    AddIntConstant(PR_POLL_EXCEPT);
    AddIntConstant(PR_POLL_ERR);
    AddIntConstant(PR_POLL_NVAL);
    AddIntConstant(PR_POLL_HUP);

}
