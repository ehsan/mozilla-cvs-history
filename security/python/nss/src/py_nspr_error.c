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

#define PY_SSIZE_T_CLEAN
#include "Python.h"
#include "structmember.h"

#define NSS_ERROR_MODULE
#include "py_nspr_error.h"


static PyObject *NSPR_Exception = NULL;

typedef struct {
    PRErrorCode	 num;
    const char *name;
    const char *string;
} NSPRErrorDesc;

#include "nspr.h"
#include "seccomon.h"

#define ER2(a,b)   {a, #a, b},
#define ER3(a,b,c) {a, #a, c},

#include "secerr.h"
#include "sslerr.h"

NSPRErrorDesc nspr_errors[] = {
#include "SSLerrs.h"
#include "SECerrs.h"
#include "NSPRerrs.h"
};

static int
cmp_error(const void *p1, const void *p2)
{
    NSPRErrorDesc *e1 = (NSPRErrorDesc *) p1;
    NSPRErrorDesc *e2 = (NSPRErrorDesc *) p2;

    if (e1->num < e2->num) return -1;
    if (e1->num > e2->num) return  1;
    return 0;
}

const int nspr_error_count = sizeof(nspr_errors) / sizeof(NSPRErrorDesc);

static PRStatus
init_nspr_errors(void) {
    int low  = 0;
    int high = nspr_error_count - 1;
    int i;
    PRErrorCode err_num;
    int result = SECSuccess;

    /* Make sure table is in ascending order. binary search depends on it. */

    qsort((void*)nspr_errors, nspr_error_count, sizeof(NSPRErrorDesc), cmp_error);

    PRErrorCode last_num = ((PRInt32)0x80000000);
    for (i = low; i <= high; ++i) {
        err_num = nspr_errors[i].num;
        if (err_num <= last_num) {
            result = SECFailure;
            fprintf(stderr,
"sequence error in error strings at item %d\n"
"error %d (%s)\n"
"should come after \n"
"error %d (%s)\n",
                    i, last_num, nspr_errors[i-1].string,
                    err_num, nspr_errors[i].string);
        }
        last_num = err_num;
    }
    return result;
}

static const NSPRErrorDesc *
lookup_nspr_error(PRErrorCode num) {
    int low  = 0;
    int high = nspr_error_count - 1;
    int i;
    PRErrorCode err_num;

    /* Do binary search of table. */
    while (low + 1 < high) {
    	i = (low + high) / 2;
	err_num = nspr_errors[i].num;
	if (num == err_num)
	    return &nspr_errors[i];
        if (num < err_num)
	    high = i;
	else
	    low = i;
    }
    if (num == nspr_errors[low].num)
    	return &nspr_errors[low];
    if (num == nspr_errors[high].num)
    	return &nspr_errors[high];
    return NULL;
}

static PyObject *
set_nspr_error(const char *format, ...)
{
    va_list vargs;
    PyObject *v;
    const NSPRErrorDesc *error_desc;
    char *errstr=NULL;
    PRErrorCode err;
    PyObject *detail = NULL;
    char buf[1024];

    if (format) {
#ifdef HAVE_STDARG_PROTOTYPES
	va_start(vargs, format);
#else
	va_start(vargs);
#endif
	detail = PyString_FromFormatV(format, vargs);
	va_end(vargs);
    }

    err = PR_GetError();
    PR_GetErrorText(errstr);
    if (errstr == NULL) {
        if ((error_desc = lookup_nspr_error(err)) != NULL) {
            snprintf(buf, sizeof(buf), "(%s) %s", error_desc->name, error_desc->string);
            errstr = buf;
        } else {
            errstr = NULL;
        }

    }

    if (detail) {
        v = Py_BuildValue("(isS)", err, errstr, detail);
	Py_DECREF(detail);
    } else {
        v = Py_BuildValue("(is)", err, errstr);
    }
    if (v != NULL) {
        PyErr_SetObject(NSPR_Exception, v);
        Py_DECREF(v);
    }
    return NULL;
}

PyDoc_STRVAR(io_get_nspr_error_string_doc,
"get_nspr_error_string(number) -> string\n\
\n\
Given an NSPR error number, returns it's string description\n\
");

static PyObject *
io_get_nspr_error_string(PyObject *self, PyObject *args)
{
    int err_num;
    NSPRErrorDesc const *error_desc = NULL;

    if (!PyArg_ParseTuple(args, "i:get_nspr_error_string", &err_num)) {
        return NULL;
    }

    if ((error_desc = lookup_nspr_error(err_num)) == NULL)
        Py_RETURN_NONE;

    return PyString_FromString(error_desc->string);
}

/* List of functions exported by this module. */
static PyMethodDef
module_methods[] = {
    {"get_nspr_error_string", io_get_nspr_error_string, METH_VARARGS, io_get_nspr_error_string_doc},
    {NULL, NULL}            /* Sentinel */
};

static PyObject *
init_py_nspr_errors(PyObject *module)
{
    NSPRErrorDesc *error_desc = NULL;
    PyObject *py_error_doc = NULL;
    PyObject *error_str = NULL;
    int i;

    /* Load and intialize NSPR error descriptions */
    if (init_nspr_errors() != PR_SUCCESS)
        return NULL;

    /* Create a python string to hold the modules error documentation */
    if ((py_error_doc = PyString_FromString("NSPR Error Constants:\n\n")) == NULL)
        return NULL;

    /*
     * Iterate over all the NSPR errors, for each:
     * add it's doc string to the module doc
     * add it's numeric value as as a module constant
     */
    for (i = 0, error_desc = &nspr_errors[0]; i < nspr_error_count; i++, error_desc++) {

        if ((error_str = PyString_FromFormat("%s: %s\n\n", error_desc->name, error_desc->string)) == NULL) {
            Py_DECREF(py_error_doc);
            return NULL;
        }
        PyString_ConcatAndDel(&py_error_doc, error_str);

        if (PyModule_AddIntConstant(module, error_desc->name, error_desc->num) < 0) {
            Py_DECREF(py_error_doc);
            return NULL;
        }
    }
    return py_error_doc;
}

/* ================= Utilities shared with other modules  ================= */


/*
 * Format a tuple into a string by calling the str() method on
 * each member of the tuple.
 * 
 * Tuples do not implement a str method only a repr with the 
 * unfortunate result repr() is invoked on each of its members.
 */
static PyObject *tuple_str(PyObject *tuple)
{
    PyObject *separator = NULL;
    PyObject *obj = NULL;
    PyObject *tmp_obj = NULL;
    PyObject *text = NULL;
    Py_ssize_t i, len;
        
    if (!PyTuple_Check(tuple)) return NULL;

    len = PyTuple_GET_SIZE(tuple);
    
    if (len == 0) {
        return PyString_FromString("()");
    }

    if ((text = PyString_FromString("(")) == NULL) {
        goto exit;
    }

    if (len > 1) {
        if ((separator = PyString_FromString(", ")) == NULL) {
            goto exit;
        }
    }

    for (i = 0; i < len; i++) {
        obj = PyTuple_GET_ITEM(tuple, i);
        tmp_obj = PyObject_Str(obj);
        PyString_ConcatAndDel(&text, tmp_obj);
        if (text == NULL) {
            goto exit;
        }
        if (i < len-1) {
            PyString_Concat(&text, separator);
            if (text == NULL) {
                goto exit;
            }
        }
    }

    if ((tmp_obj = PyString_FromString(")")) == NULL) {
        Py_CLEAR(text);
        goto exit;
    }

    PyString_ConcatAndDel(&text, tmp_obj);
    if (text == NULL) {
        goto exit;
    }

 exit:
    Py_XDECREF(separator);
    return text;
}

/* ============================== Module Exports ============================= */

static PyNSPR_ERROR_C_API_Type nspr_error_c_api =
{
    NULL,                       /* nspr_exception */
    set_nspr_error,             /* set_nspr_error */
    tuple_str
};

/* ============================== Module Construction ============================= */

PyDoc_STRVAR(module_doc,
"This module defines the NSPR errors and provides functions to\n\
manipulate them.\n\
");

PyMODINIT_FUNC
initerror(void)
{
    PyObject *m;
    PyObject *py_error_doc = NULL;
    PyObject *py_module_doc = NULL;

    if ((m = Py_InitModule3("error", module_methods, module_doc)) == NULL)
        return;

    if ((py_error_doc = init_py_nspr_errors(m)) == NULL)
        return;

    if ((py_module_doc = PyString_FromString(module_doc)) == NULL)
        return;

    PyString_ConcatAndDel(&py_module_doc, py_error_doc);
    PyModule_AddObject(m, "__doc__", py_module_doc);

    /* exceptions */
    if ((NSPR_Exception = PyErr_NewException("nss.error.NSPRError", PyExc_EnvironmentError, NULL)) == NULL)
        return;
    Py_INCREF(NSPR_Exception);
    if (PyModule_AddObject(m, "NSPRError", NSPR_Exception) < 0)
        return;

    /* Export C API */
    nspr_error_c_api.nspr_exception = NSPR_Exception;
    if (PyModule_AddObject(m, "_C_API", PyCObject_FromVoidPtr((void *)&nspr_error_c_api, NULL)) != 0)
        return;

}
