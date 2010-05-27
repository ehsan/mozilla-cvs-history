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

//#define DEBUG

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#define NSS_THREAD_LOCAL_KEY "nss"

#define ASSIGN_REF(dst, obj)                    \
do {                                            \
    PyObject *tmp;                              \
                                                \
    tmp = (PyObject *)dst;                      \
    Py_INCREF(obj);                             \
    dst = obj;                                  \
    Py_CLEAR(tmp);                              \
} while (0)

#define ASSIGN_NEW_REF(dst, obj)                \
do {                                            \
    PyObject *tmp;                              \
                                                \
    tmp = (PyObject *)dst;                      \
    dst = obj;                                  \
    Py_CLEAR(tmp);                              \
} while (0)


// Gettext
#ifndef _
#define _(s) s
#endif

#if (PY_VERSION_HEX < 0x02050000)
typedef int Py_ssize_t;
#define PY_SSIZE_T_MAX INT_MAX
#define PY_SSIZE_T_MIN INT_MIN
#define PyInt_FromSsize_t PyInt_FromLong
#define PyNumber_AsSsize_t(ob, exc) PyInt_AsLong(ob)
#define PyIndex_Check(ob) PyInt_Check(ob)
typedef Py_ssize_t (*readbufferproc)(PyObject *, Py_ssize_t, void **);
typedef Py_ssize_t (*writebufferproc)(PyObject *, Py_ssize_t, void **);
typedef Py_ssize_t (*segcountproc)(PyObject *, Py_ssize_t *);
typedef Py_ssize_t (*charbufferproc)(PyObject *, Py_ssize_t, char **);
typedef Py_ssize_t (*lenfunc)(PyObject *);
typedef PyObject *(*ssizeargfunc)(PyObject *, Py_ssize_t);
typedef PyObject *(*ssizessizeargfunc)(PyObject *, Py_ssize_t, Py_ssize_t);
#endif

#if (PY_VERSION_HEX < 0x02060000)
#define Py_TYPE(ob) (((PyObject*)(ob))->ob_type)
#define PyVarObject_HEAD_INIT(type, size) \
	PyObject_HEAD_INIT(type) size,
#define PyImport_ImportModuleNoBlock PyImport_ImportModule
#define PyLong_FromSsize_t PyInt_FromLong
#define Py_TPFLAGS_HAVE_NEWBUFFER 0
#endif

#define AddIntConstant(c) if (PyModule_AddIntConstant(m, #c, c) < 0) return;

#ifdef DEBUG

#define DumpRefCount(_obj)                                              \
{                                                                       \
    printf("<%s object at %p refcnt=%d>\n", Py_TYPE(_obj)->tp_name, _obj, _obj->ob_refcnt); \
}


#define TraceMessage(_msg)                      \
{                                               \
    printf("%s\n", _msg);                       \
}

#define TraceMethodEnter(_obj)                                          \
{                                                                       \
    const char *name = NULL;                                            \
                                                                        \
    if (_obj) {                                                         \
        name = Py_TYPE(_obj)->tp_name;                                  \
    }                                                                   \
    printf("%s (Enter): <%s object at %p refcnt=%d>\n",                 \
           __FUNCTION__, name, _obj, _obj ? _obj->ob_refcnt : -9999);   \
}

#define TraceMethodLeave(_obj)                                          \
{                                                                       \
    const char *name = NULL;                                            \
                                                                        \
    if (_obj) {                                                         \
        name = Py_TYPE(_obj)->tp_name;                                  \
    }                                                                   \
    printf("%s (Leave): <%s object at %p refcnt=%d>\n",                 \
           __FUNCTION__, name, _obj, _obj ? _obj->ob_refcnt : -9999);   \
}

#define TraceObjNewEnter(_tp)                                   \
{                                                               \
    PyTypeObject *tp = _tp;                                     \
    if (tp != NULL)                                             \
        printf("%s (Enter) %s\n", __FUNCTION__, tp->tp_name);   \
    else                                                        \
        printf("%s (Enter)\n", __FUNCTION__);                   \
}


#define TraceObjNewLeave(_obj)                                          \
{                                                                       \
    const char *name = NULL;                                            \
                                                                        \
    if (_obj) {                                                         \
        name = Py_TYPE(_obj)->tp_name;                                  \
    }                                                                   \
    printf("%s: returns <%s object at %p refcnt=%d>\n",                 \
           __FUNCTION__, name, _obj, _obj ? _obj->ob_refcnt : -9999);   \
}

#else
#define DumpRefCount(_obj)
#define TraceMessage(_msg)
#define TraceMethodEnter(_obj)
#define TraceMethodLeave(_obj)
#define TraceObjNewEnter(_tp)
#define TraceObjNewLeave(_obj)
#endif

