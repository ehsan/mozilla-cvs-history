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

/* NSPR header files */
#undef HAVE_LONG_LONG           /* FIXME: both Python.h and nspr.h define HAVE_LONG_LONG  */
#include "nspr.h"
#include "private/pprio.h"
#include "prnetdb.h"

/* ========================================================================== */
/* ============================= HostEntry Class ============================ */
/* ========================================================================== */

typedef struct {
    PyObject_HEAD
    PRHostEnt entry;
    char buffer[PR_NETDB_BUF_SIZE]; /* this is where data pointed to in PRHostEnt is stored */
} HostEntry;

#define PyHostEntry_Check(op) PyObject_TypeCheck(op, &HostEntryType)

/* ========================================================================== */
/* =========================== NetworkAddress Class ========================= */
/* ========================================================================== */

typedef struct {
    PyObject_HEAD
    PRNetAddr addr;
    PyObject *py_hostname;
    HostEntry *py_hostentry;
} NetworkAddress;

#define PyNetworkAddress_Check(op) PyObject_TypeCheck(op, &NetworkAddressType)

/* ========================================================================== */
/* ============================== Socket Class ============================== */
/* ========================================================================== */

#define ALLOC_INCREMENT 1024
typedef struct {
    char *buf;
    long len;
    long alloc_len;
} ReadAhead;


#define INIT_READAHEAD(readahead)               \
{                                               \
    (readahead)->buf = NULL;                    \
    (readahead)->len = 0;                       \
    (readahead)->alloc_len = 0;                 \
}

#define FREE_READAHEAD(readahead)               \
{                                               \
    if ((readahead)->buf)                       \
        PyMem_FREE((readahead)->buf);           \
    INIT_READAHEAD(readahead);                  \
}

#define SOCKET_HEAD                             \
    PyObject_HEAD;                              \
    PRFileDesc *pr_socket;                      \
    int family;                                 \
    int makefile_refs;                          \
    int open_for_read;                          \
    NetworkAddress *py_netaddr;                 \
    ReadAhead readahead;


typedef struct {
    SOCKET_HEAD
} Socket;

#define PySocket_Check(op) PyObject_TypeCheck(op, &SocketType)

typedef struct {
    PyTypeObject *network_address_type;
    PyTypeObject *host_entry_type;
    PyTypeObject *socket_type;
    void         (*Socket_init_from_PRFileDesc)(Socket *py_socket, PRFileDesc *pr_socket, int family);
} PyNSPR_IO_C_API_Type;

#ifdef NSS_IO_MODULE

static PyObject *
HostEntry_new_from_PRNetAddr(PRNetAddr *pr_netaddr);

#else  /* not NSS_IO_MODULE */

static PyNSPR_IO_C_API_Type nspr_io_c_api;

#define NetworkAddressType (*nspr_io_c_api.network_address_type)
#define HostEntryType (*nspr_io_c_api.host_entry_type)
#define SocketType (*nspr_io_c_api.socket_type)

#define Socket_init_from_PRFileDesc (*nspr_io_c_api.Socket_init_from_PRFileDesc)

static int
import_nspr_io_c_api(void)
{
    PyObject *module = NULL;
    PyObject *c_api_object = NULL;
    void *api = NULL;

    if ((module = PyImport_ImportModule("nss.io")) == NULL)
        return -1;

    if ((c_api_object = PyObject_GetAttrString(module, "_C_API")) == NULL) {
        Py_DECREF(module);
        return -1;
    }

    if (!(PyCObject_Check(c_api_object))) {
        Py_DECREF(c_api_object);
        Py_DECREF(module);
        return -1;
    }

    if ((api = PyCObject_AsVoidPtr(c_api_object)) == NULL) {
        Py_DECREF(c_api_object);
        Py_DECREF(module);
        return -1;
    }

    memcpy(&nspr_io_c_api, api, sizeof(nspr_io_c_api));
    Py_DECREF(c_api_object);
    Py_DECREF(module);
    return 0;
}

#endif /* NSS_IO_MODULE */
