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

#undef HAVE_LONG_LONG           /* FIXME: both Python.h and nspr.h define HAVE_LONG_LONG  */
#include "nss.h"
#include "ssl.h"

typedef struct {
    SOCKET_HEAD;
    PyObject *py_auth_certificate_callback;
    PyObject *py_auth_certificate_callback_data;
    PyObject *py_pk11_pin_args;
    PyObject *py_handshake_callback;
    PyObject *py_handshake_callback_data;
    PyObject *py_client_auth_data_callback;
    PyObject *py_client_auth_data_callback_data;
} SSLSocket;

#define PySSLSocket_Check(op) PyObject_TypeCheck(op, &SSLSocketType)

typedef struct {
    PyTypeObject *sslsocket_type;
} PyNSS_SSL_C_API_Type;

#ifdef NSS_SSL_MODULE

static PyTypeObject SSLSocketType;

#else /* not NSS_SSL_MODULE */

static PyNSS_SSL_C_API_Type nss_ssl_c_api;

#define SSLSocketType (*nss_ssl_c_api.sslsocket_type)

static int
import_nss_ssl_c_api(void)
{
    PyObject *module = NULL;
    PyObject *c_api_object = NULL;
    void *api = NULL;

    if ((module = PyImport_ImportModule("nss.ss;")) == NULL)
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

    memcpy(&nss_ssl_c_api, api, sizeof(nss_ssl_c_api));
    Py_DECREF(c_api_object);
    Py_DECREF(module);
    return 0;
}

#endif /* NSS_SSL_MODULE */

