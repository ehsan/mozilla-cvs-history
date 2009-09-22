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

// FIXME: should we be calling these?
// SECKEY_DestroyEncryptedPrivateKeyInfo
// SECKEY_DestroyPrivateKey	   SECKEY_DestroyPrivateKeyInfo
// SECKEY_DestroyPrivateKeyList	   SECKEY_DestroyPublicKey
// SECKEY_DestroyPublicKeyList	   SECKEY_DestroySubjectPublicKeyInfo

#define PY_SSIZE_T_CLEAN
#include "Python.h"
#include "structmember.h"

#include "py_nspr_common.h"
#define NSS_NSS_MODULE
#include "py_nss.h"
#include "py_nspr_error.h"

#include "secder.h"
#include "sechash.h"
#include "certdb.h"
#include "hasht.h"

#define OCTETS_PER_LINE_DEFAULT 16
#define HEX_SEPARATOR_DEFAULT ":"

#define FMT_OBJ_AND_APPEND(level, label, obj, lines, fail)              \
{                                                                       \
    PyObject *pair = NULL;                                              \
                                                                        \
    Py_INCREF(obj);                                                     \
    if ((pair = fmt_pair(level, label, obj)) == NULL) goto fail;        \
    Py_DECREF(obj);                                                     \
    if (PyList_Append(lines, pair) != 0) goto fail;                     \
}


#define FMT_LABEL_AND_APPEND(level, label, lines, fail)                 \
{                                                                       \
    PyObject *pair = NULL;                                              \
                                                                        \
    if ((pair = fmt_label(level, label)) == NULL) goto fail;            \
    if (PyList_Append(lines, pair) != 0) goto fail;                     \
}


static char time_format[] = "%a %b %d %H:%M:%S %Y UTC";
static char hex_chars[] = "0123456789abcdef";
static PyObject *empty_tuple = NULL;
static PyTypeObject PK11SymKeyType;
static PyTypeObject PK11ContextType;
static PyObject *sec_oid_name_to_value = NULL;
static PyObject *sec_oid_value_to_name = NULL;
static PyObject *ckm_name_to_value = NULL;
static PyObject *ckm_value_to_name = NULL;
static PyObject *cka_name_to_value = NULL;
static PyObject *cka_value_to_name = NULL;

typedef PyObject *(*format_lines_func)(PyObject *self, PyObject *args, PyObject *kwds);

static PyTypeObject SecItemType;
static PyTypeObject PK11SymKeyType;

/* === Prototypes === */

static PyObject *
obj_to_hex(PyObject *obj, int octets_per_line, char *separator);

static PyObject *
raw_data_to_hex(unsigned char *data, int data_len, int octets_per_line, char *separator);

static SECStatus
sec_strip_tag_and_length(SECItem *item);

static PyObject *
sec_context_specific_str(SECItem *item);

static PyObject *
sec_any_str(SECItem *item);

static PyObject *
sec_set_str_list(SECItem *item);

static PyObject *
sec_boolean_str(SECItem *item);

static PyObject *
sec_encoded_boolean_str(SECItem *item);

static PyObject *
sec_integer_str(SECItem *item);

static PyObject *
sec_encoded_integer_str(SECItem *item);

static PyObject *
sec_string_str(SECItem *item);

static PyObject *
sec_oid_str_by_secitem(SECItem *oid);

static PyObject *
sec_encoded_oid_str(SECItem *item);

static PyObject *
sec_utc_time_str(SECItem *item);

static PyObject *
sec_generalized_time_str(SECItem *item);

PRTime
sec_time_choice(SECItem *item);

static PyObject *
sec_time_choice_str(SECItem *item);

static PyObject *
sec_octet_str(SECItem *item);

static PyObject *
sec_bit_str(SECItem *item);

static PyObject *
sec_bmp_str(SECItem *item);

static PyObject *
sec_universal_str(SECItem *item);

static PyObject *
sec_universal_item_str(SECItem *item);

static PyObject *
get_algorithm_id_str(SECAlgorithmID *a);

static PyObject *
cert_trust_flags_str(unsigned int flags);

static PyObject *
nss_indented_format(PyObject *self, PyObject *args, PyObject *kwds);

static PyObject *
pk11_md5_digest(PyObject *self, PyObject *args);

static PyObject *
pk11_sha1_digest(PyObject *self, PyObject *args);

static PyObject *
pk11_sha256_digest(PyObject *self, PyObject *args);

static PyObject *
pk11_sha512_digest(PyObject *self, PyObject *args);

static PyObject *
PyPK11Context_new_from_PK11Context(PK11Context *pk11_context);

static PyObject *
PyPK11SymKey_new_from_PK11SymKey(PK11SymKey *pk11_sym_key);

static const char *
key_mechanism_type_name(CK_MECHANISM_TYPE mechanism);

static const char *
pk11_attribute_type_name(CK_ATTRIBUTE_TYPE type);

/* ==================================== */

int
_AddIntConstantWithLookup(PyObject *module, const char *name, long value, const char *prefix,
                          PyObject *name_to_value, PyObject *value_to_name)
{
    PyObject *module_dict;
    PyObject *py_name;
    PyObject *py_value;

    if (!PyModule_Check(module)) {
        PyErr_SetString(PyExc_TypeError, "_AddIntConstantWithLookup() needs module as first arg");
        return -1;
    }

    if ((module_dict = PyModule_GetDict(module)) == NULL) {
        PyErr_Format(PyExc_SystemError, "module '%s' has no __dict__",
                     PyModule_GetName(module));
        return -1;
    }

    if ((py_name = PyString_FromString(name)) == NULL) {
        return -1;
    }

    if ((py_value = PyInt_FromLong(value)) == NULL) {
        return -1;
    }

    if (PyDict_SetItem(module_dict, py_name, py_value) != 0) {
        Py_DECREF(py_name);
        Py_DECREF(py_value);
        return -1;
    }

    if (PyDict_SetItem(value_to_name, py_value, py_name) != 0) {
        Py_DECREF(py_name);
        Py_DECREF(py_value);
        return -1;
    }

    if (PyDict_SetItem(name_to_value, py_name, py_value) != 0) {
        Py_DECREF(py_name);
        Py_DECREF(py_value);
        return -1;
    }

    if (prefix) {
        size_t prefix_len = strlen(prefix);

        if (strlen(name) > prefix_len &&
            strncmp(prefix, name, prefix_len) == 0) {

            if ((py_name = PyString_FromString(name + prefix_len)) == NULL) {
                Py_DECREF(py_name);
                Py_DECREF(py_value);
                return -1;
            }

            if (PyDict_SetItem(name_to_value, py_name, py_value) != 0) {
                Py_DECREF(py_name);
                Py_DECREF(py_value);
                return -1;
            }
        }
    }

    Py_DECREF(py_name);
    Py_DECREF(py_value);
    return 0;
}

/* FIXME: convert all equality tests to Py_None to PyNone_Check() */
#define PyNone_Check(x) ((x) == Py_None)

int SecItemOrNoneConvert(PyObject *obj, PyObject **param)
{
    if (PySecItem_Check(obj)) {
        *param = obj;
        return 1;
    }

    if (PyNone_Check(obj)) {
        *param = NULL;
        return 1;
    }

    PyErr_Format(PyExc_TypeError, "must be %.50s or None, not %.50s",
                 SecItemType.tp_name, Py_TYPE(obj)->tp_name);
    return 0;
}

int SymKeyOrNoneConvert(PyObject *obj, PyObject **param)
{
    if (PySymKey_Check(obj)) {
        *param = obj;
        return 1;
    }

    if (PyNone_Check(obj)) {
        *param = NULL;
        return 1;
    }

    PyErr_Format(PyExc_TypeError, "must be %.50s or None, not %.50s",
                 PK11SymKeyType.tp_name, Py_TYPE(obj)->tp_name);
    return 0;
}

static const char *
key_type_str(KeyType key_type)
{
    static char buf[80];

    switch(key_type) {
    case nullKey:     return "NULL";
    case rsaKey:      return "RSA";
    case dsaKey:      return "DSA";
    case fortezzaKey: return "Fortezza";
    case dhKey:       return "Diffie Helman";
    case keaKey:      return "Key Exchange Algorithm";
    case ecKey:       return "Elliptic Curve";
    default:
        snprintf(buf, sizeof(buf), "unknown(%#x)", key_type);
        return buf;
    }
}


const char *
sec_oid_tag_str(SECOidTag tag)
{
    static char buf[80];

    SECOidData *oiddata;

    if ((oiddata = SECOID_FindOIDByTag(tag)) != NULL) {
	return oiddata->desc;
    }
    snprintf(buf, sizeof(buf), "unknown(%#x)", tag);
    return buf;
}

PyDoc_STRVAR(generic_format_doc,
"format(level=0, indent='    ') -> string)\n\
\n\
:Parameters:\n\
    level : integer\n\
        Initial indentation level, all subsequent indents are relative\n\
        to this starting level.\n\
    indent : string\n\
        string replicated once for each indent level then prepended to output line\n\
\n\
This is equivalent to:\n\
nss.indented_format(cert.signed_data.format_lines())\n\
");

PyDoc_STRVAR(generic_format_lines_doc,
"format_lines(level=0) -> [(level, string),...]\n\
\n\
:Parameters:\n\
    level : integer\n\
        Initial indentation level, all subsequent indents are relative\n\
        to this starting level.\n\
\n\
Formats the object into a sequence of lines with indent level\n\
information.  The return value is a list where each list item is a 2\n\
valued tuple pair.  The first item in the pair is an integer\n\
representing the indentation level for that line and the second item\n\
in the pair is the string value for the line.\n\
\n\
The output of this function can be formatted into a single string\n\
by calling nss.indented_format(). The reason this function returns\n\
(level, string) pairs as opposed to an single indented string is to\n\
support other text formatting systems with indentation controls.\n\
");

static PyObject *
fmt_pair(int level, char *label, PyObject *obj)
{
    PyObject *pair = NULL;
    PyObject *obj_str = NULL;

    if (PyString_Check(obj)) {
        Py_INCREF(obj);
        obj_str = obj;
    } else {
        if ((obj_str = PyObject_Str(obj)) == NULL)
            return NULL;
    }

    if (label) {
        PyObject *labeled_str = NULL;

        if (obj_str) {
            if ((labeled_str = PyString_FromFormat("%s: %s", label, PyString_AsString(obj_str))) == NULL) {
                Py_DECREF(obj_str);
                return NULL;
            }
            Py_DECREF(obj_str);
            obj_str = labeled_str;
        } else {
            if ((obj_str = PyString_FromFormat("%s:", label)) == NULL) {
                return NULL;
            }
        }
    }

    if (!obj_str) {
        if ((obj_str = PyString_FromString("None")) == NULL) {
            return NULL;
        }
    }

    if ((pair = PyTuple_New(2)) == NULL) {
        return NULL;
    }

    PyTuple_SET_ITEM(pair, 0, PyInt_FromLong(level));
    PyTuple_SET_ITEM(pair, 1, obj_str);

    return pair;
}

static PyObject *
fmt_label(int level, char *label)
{
    PyObject *pair = NULL;
    PyObject *label_str = NULL;

    if ((label_str = PyString_FromFormat("%s:", label)) == NULL) {
        return NULL;
    }

    if ((pair = PyTuple_New(2)) == NULL) {
        return NULL;
    }

    PyTuple_SET_ITEM(pair, 0, PyInt_FromLong(level));
    PyTuple_SET_ITEM(pair, 1, label_str);

    return pair;
}


static PyObject *
format_from_lines(format_lines_func formatter, PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"level", "indent",  NULL};
    int level = 0;
    PyObject *py_indent = NULL;
    PyObject *py_lines = NULL;
    PyObject *py_formatted_result = NULL;
    PyObject *tmp_args = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|iS:format", kwlist, &level, &py_indent))
        return NULL;

    if (!py_indent) {
        if ((py_indent = PyString_FromString("    ")) == NULL)
            goto fail;
    } else {
        Py_INCREF(py_indent);
    }

    if ((tmp_args = Py_BuildValue("(i)", level)) == NULL)
        goto fail;
    if ((py_lines = formatter(self, tmp_args, NULL)) == NULL)
        goto fail;

    if ((tmp_args = Py_BuildValue("OO", py_lines, py_indent)) == NULL)
        goto fail;
    if ((py_formatted_result = nss_indented_format(NULL, tmp_args, NULL)) == NULL)
        goto fail;

    Py_DECREF(tmp_args);
    Py_DECREF(py_indent);
    return py_formatted_result;

 fail:
    Py_XDECREF(tmp_args);
    Py_XDECREF(py_indent);
    return NULL;
}

static PyObject *
obj_to_hex(PyObject *obj, int octets_per_line, char *separator)
{
    unsigned char *data = NULL;
    Py_ssize_t data_len;

    if (PyObject_AsReadBuffer(obj, (void *)&data, &data_len))
        return NULL;

    return raw_data_to_hex(data, data_len, octets_per_line, separator);

}

/* see cert_data_to_hex() for documentation */
static PyObject *
raw_data_to_hex(unsigned char *data, int data_len, int octets_per_line, char *separator)
{
    int separator_len = 0;
    char *separator_end = NULL;
    char *src=NULL, *dst=NULL;
    int line_size = 0;
    unsigned char octet = 0;
    int num_lines = 0;
    PyObject *lines = NULL;
    PyObject *line = NULL;
    int line_number, i, j;
    int num_octets = 0;


    if (octets_per_line < 0)
        octets_per_line = 0;

    if (!separator)
        separator = "";

    separator_len = strlen(separator);
    separator_end = separator + separator_len;

    if (octets_per_line == 0) {
        num_octets = data_len;
        line_size = (num_octets * 2) + ((num_octets-1) * separator_len);
        if (line_size < 0) line_size = 0;

        if ((line = PyString_FromStringAndSize(NULL, line_size)) == NULL) {
            return NULL;
        }
        dst = PyString_AS_STRING(line);
        for (i = 0; i < data_len; i++) {
            octet = data[i];
            *dst++ = hex_chars[(octet & 0xF0) >> 4];
            *dst++ = hex_chars[octet & 0xF];
            if (i < data_len-1)
                for (src = separator; src < separator_end; *dst++ = *src++);
        }
        return line;
    } else {
        num_lines = (data_len + octets_per_line - 1) / octets_per_line;
        if (num_lines < 0) num_lines = 0;

        if ((lines = PyList_New(num_lines)) == NULL) {
            return NULL;
        }

        for (i = line_number = 0; i < data_len;) {
            num_octets = data_len - i;
            if (num_octets > octets_per_line) {
                num_octets = octets_per_line;
                line_size = num_octets*(2+separator_len);
            } else {
                line_size = (num_octets * 2) + ((num_octets-1) * separator_len);
            }

            if (line_size < 0) line_size = 0;
            if ((line = PyString_FromStringAndSize(NULL, line_size)) == NULL) {
                Py_DECREF(lines);
                return NULL;
            }
            dst = PyString_AS_STRING(line);
            for (j = 0; j < num_octets && i < data_len; i++, j++) {
                octet = data[i];
                *dst++ = hex_chars[(octet & 0xF0) >> 4];
                *dst++ = hex_chars[octet & 0xF];
                if (i < data_len-1)
                    for (src = separator; src < separator_end; *dst++ = *src++);
            }
            PyList_SetItem(lines, line_number++, line);
        }
        return lines;
    }
}

PyDoc_STRVAR(cert_data_to_hex_doc,
"data_to_hex(data, octets_per_line=0, separator=None) -> string or list of strings\n\
\n\
:Parameters:\n\
    data : buffer\n\
        Binary data\n\
    octets_per_line : integer\n\
        Number of octets formatted on one line, if 0 then\n\
        return a single string instead of an array of lines\n\
    separator : string\n\
        String used to seperate each octet\n\
        If None it will be as if the empty string had been\n\
        passed and no separator will be used.\n\
\n\
Format the binary data as hex string(s).\n\
Either a list of strings is returned or a single string.\n\
\n\
If octets_per_line is greater than zero then a list of\n\
strings will be returned where each string contains\n\
octets_per_line number of octets (except for the last\n\
string in the list which will contain the remainder of the\n\
octets). Returning a list of \"lines\" makes it convenient\n\
for a caller to format a block of hexadecimal data with line\n\
wrapping. If octets_per_line is greater than zero indicating\n\
a list result is desired a list is always returned even if\n\
the number of octets would produce only a single line.\n\
\n\
If octets_per_line is zero then a single string is returned,\n\
(no line splitting is performed). This is the default.\n\
\n\
The separator string is used to separate each octet. If None\n\
it will be as if the empty string had been passed and no\n\
separator will be used. This is the default.\n\
");

static PyObject *
cert_data_to_hex(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"data", "octets_per_line", "separator", NULL};
    PyObject *obj = NULL;
    int octets_per_line = 0;
    char *separator = "";

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|iz:cert_data_to_hex", kwlist,
                                     &obj, &octets_per_line, &separator))
        return NULL;

    return obj_to_hex(obj, octets_per_line, separator);
}

PyDoc_STRVAR(read_hex_doc,
"read_hex(input, separators=\" ,:\\t\\n\") -> buffer\n\
\n\
:Parameters:\n\
    input : string\n\
        string containing hexadecimal data\n\
    separators : string or None\n\
        string containing set of separator characters\n\
        Any character encountered during parsing which is in\n\
        this string will be skipped and considered a separator\n\
        between pairs of hexadecimal characters.\n\
\n\
\n\
Parse a string containing hexadecimal data and return a buffer\n\
object containing the binary octets. Each octet in the string is\n\
represented as a pair of case insensitive hexadecimal characters\n\
(0123456789abcdef). Each octet must be a pair of\n\
characters. Octets may optionally be preceded by 0x or 0X. Octets\n\
may be separated by separator characters specified in the\n\
separators string. The separators string is a set of\n\
characters. Any character in the separators character set will be\n\
ignored when it occurs between octets. If no separators should be\n\
considered then pass an empty string.\n\
\n\
Using the default separators each of these strings is valid input\n\
representing the same 8 octet sequence:\n\
\n\
01, 23, 45, 67, 89, ab, cd, ef\n\
01, 23, 45, 67, 89, AB, CD, EF\n\
0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef\n\
01:23:45:67:89:ab:cd:ef\n\
0123456789abcdef\n\
01 23 45 67 89 ab cd ef\n\
0x010x230x450x670x890xab0xcd0xef\n\
");
static PyObject *
read_hex(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"input", "separators", NULL};
    const char *input;
    const char *separators = " ,:\t\n";
    size_t input_len, separators_len;
    Py_ssize_t n_octets;
    unsigned char octet, *data, *dst;
    const char *src, *input_end;
    const char *sep, *separators_end;
    PyObject *py_out_buf;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|s:read_hex", kwlist,
                                     &input, &separators))
        return NULL;

    input_len = strlen(input);
    src = input;
    input_end = input + input_len;
    separators_len = strlen(separators);
    n_octets = 0;

    /*
     * The maximum number of octets is half the string length
     * because they must occur in pairs. If there are separators
     * in the string then the number of octets will be less than
     * half. Thus len/2 is an upper bound.
     */
    if ((data = PyMem_Malloc(input_len/2)) == NULL) {
        return PyErr_NoMemory();
    }
    dst = data;

    separators_end = separators + separators_len;
    while (src < input_end) {
        for (; *src; src++) {
            for (sep = separators; sep < separators_end && *src != *sep; sep++);
            if (sep == separators_end) break;
        }
        if (!*src) break;
        if (src[0] == '0' && (tolower(src[1]) == 'x')) src +=2; /* skip 0x or 0X */
        octet = 0;
        switch (tolower(src[0])) {
        case '0': octet = 0x0 << 4; break;
        case '1': octet = 0x1 << 4; break;
        case '2': octet = 0x2 << 4; break;
        case '3': octet = 0x3 << 4; break;
        case '4': octet = 0x4 << 4; break;
        case '5': octet = 0x5 << 4; break;
        case '6': octet = 0x6 << 4; break;
        case '7': octet = 0x7 << 4; break;
        case '8': octet = 0x8 << 4; break;
        case '9': octet = 0x9 << 4; break;
        case 'a': octet = 0xa << 4; break;
        case 'b': octet = 0xb << 4; break;
        case 'c': octet = 0xc << 4; break;
        case 'd': octet = 0xd << 4; break;
        case 'e': octet = 0xe << 4; break;
        case 'f': octet = 0xf << 4; break;
        default:
            PyMem_Free(data);
            PyErr_Format(PyExc_ValueError, "invalid hexadecimal string beginning at offset %d \"%s\"",
                         src - input, src);
            return NULL;
        }
        switch (tolower(src[1])) {
        case '0': octet |= 0x0; break;
        case '1': octet |= 0x1; break;
        case '2': octet |= 0x2; break;
        case '3': octet |= 0x3; break;
        case '4': octet |= 0x4; break;
        case '5': octet |= 0x5; break;
        case '6': octet |= 0x6; break;
        case '7': octet |= 0x7; break;
        case '8': octet |= 0x8; break;
        case '9': octet |= 0x9; break;
        case 'a': octet |= 0xa; break;
        case 'b': octet |= 0xb; break;
        case 'c': octet |= 0xc; break;
        case 'd': octet |= 0xd; break;
        case 'e': octet |= 0xe; break;
        case 'f': octet |= 0xf; break;
        default:
            PyMem_Free(data);
            PyErr_Format(PyExc_ValueError, "invalid hexadecimal string beginning at offset %d \"%s\"",
                         src - input, src);
            return NULL;
        }
        src += 2;
        data[n_octets++] = octet;
    }

    if ((py_out_buf = PyString_FromStringAndSize((char *)data, n_octets)) == NULL) {
        PyMem_Free(data);
        return NULL;
    }
    PyMem_Free(data);

    return py_out_buf;
}

static SECStatus
sec_strip_tag_and_length(SECItem *item)
{
    unsigned int start;

    if (!item || !item->data || item->len < 2) { /* must be at least tag and length */
        return SECFailure;
    }
    start = ((item->data[1] & 0x80) ? (item->data[1] & 0x7f) + 2 : 2);
    if (item->len < start) {
        return SECFailure;
    }
    item->data += start;
    item->len  -= start;
    return SECSuccess;
}

static PyObject *
sec_context_specific_str(SECItem *item)
{
    PyObject *str = NULL;
    PyObject *hex_str = NULL;
    int type        = item->data[0] & SEC_ASN1_TAGNUM_MASK;
    int constructed = item->data[0] & SEC_ASN1_CONSTRUCTED;
    SECItem tmp;

    if (constructed) {
        str = PyString_FromFormat("[%d]", type);
    } else {
        tmp = *item;
        if (sec_strip_tag_and_length(&tmp) == SECSuccess) {
            if ((hex_str = raw_data_to_hex(tmp.data, tmp.len, 0, HEX_SEPARATOR_DEFAULT))) {
                str = PyString_FromFormat("[%d] %s", type, PyString_AsString(hex_str));
                Py_DECREF(hex_str);
            }
        }
        if (!str) {
            str = PyString_FromFormat("[%d]", type);
        }
    }

    return str;
}

static PyObject *
sec_any_str(SECItem *item)
{
    if (item && item->len && item->data) {
	switch (item->data[0] & SEC_ASN1_CLASS_MASK) {
	case SEC_ASN1_CONTEXT_SPECIFIC:
	    return sec_context_specific_str(item);
	    break;
	case SEC_ASN1_UNIVERSAL:
	    return sec_universal_item_str(item);
	    break;
	default:
	    return raw_data_to_hex(item->data, item->len, 0, HEX_SEPARATOR_DEFAULT);
	}
    }
    return PyString_FromString("(null)");
}


/* return a ASN1 SET or SEQUENCE as a list of strings */
static PyObject *
sec_set_str_list(SECItem *item)
{
    int type        = item->data[0] & SEC_ASN1_TAGNUM_MASK;
    int constructed = item->data[0] & SEC_ASN1_CONSTRUCTED;
    char *label = NULL;
    SECItem stripped_item = *item;
    PyObject *py_items = NULL;
    PyObject *py_item = NULL;

    if (!constructed) {
        return raw_data_to_hex(item->data, item->len, 0, HEX_SEPARATOR_DEFAULT);
    }

    if (sec_strip_tag_and_length(&stripped_item) != SECSuccess)
        Py_RETURN_NONE;

    if ((py_items = PyList_New(0)) == NULL)
        return NULL;

    if (type == SEC_ASN1_SET)
    	label = "Set ";
    else if (type == SEC_ASN1_SEQUENCE)
    	label = "Sequence ";
    else
    	label = "";

    while (stripped_item.len >= 2) {
	SECItem  tmp_item = stripped_item;

        if (tmp_item.data[1] & 0x80) {
	    unsigned int i;
	    unsigned int len = tmp_item.data[1] & 0x7f;
	    if (len > sizeof tmp_item.len)
	        break;
	    tmp_item.len = 0;
	    for (i = 0; i < len; i++) {
		tmp_item.len = (tmp_item.len << 8) | tmp_item.data[2+i];
	    }
	    tmp_item.len += len + 2;
	} else {
	    tmp_item.len = tmp_item.data[1] + 2;
	}
	if (tmp_item.len > stripped_item.len) {
	    tmp_item.len = stripped_item.len;
	}
	stripped_item.data += tmp_item.len;
	stripped_item.len  -= tmp_item.len;

        py_item = sec_any_str(&tmp_item);
        PyList_Append(py_items, py_item);
    }

    return py_items;
}

static PyObject *
sec_boolean_str(SECItem *item)
{
    int val = 0;

    if (item->data && item->len) {
	val = item->data[0];
    }

    if (val)
        return PyString_FromString("True");
    else
        return PyString_FromString("False");
}

static PyObject *
sec_encoded_boolean_str(SECItem *item)
{
    PyObject *str = NULL;
    SECItem stripped_item = *item;

    if (sec_strip_tag_and_length(&stripped_item) == SECSuccess)
	str = sec_boolean_str(&stripped_item);

    return str;
}

static PyObject *
sec_integer_str(SECItem *item)
{
    PyObject *str = NULL;
    int ival;

    if (!item || !item->len || !item->data) {
        str = PyString_FromFormat("(null)");
    } else if (item->len > 4) {
        str = raw_data_to_hex(item->data, item->len, 0, HEX_SEPARATOR_DEFAULT);
    } else {
	ival = DER_GetInteger(item);
        str = PyString_FromFormat("%d (0x%x)", ival, ival);
    }
    return str;
}

static PyObject *
sec_encoded_integer_str(SECItem *item)
{
    PyObject *str = NULL;
    SECItem stripped_item = *item;

    if (sec_strip_tag_and_length(&stripped_item) == SECSuccess)
	str = sec_integer_str(&stripped_item);

    return str;
}

static PyObject *
sec_string_str(SECItem *item)
{
    PyObject *str = NULL;
    SECItem stripped_item = *item;

    if (sec_strip_tag_and_length(&stripped_item) == SECSuccess)
	str = PyString_FromStringAndSize((char *)item->data, item->len);

    return str;
}

/* This function does NOT expect a DER type and length. */
static PyObject *
sec_oid_str_by_secitem(SECItem *oid)
{
    SECOidData *oiddata;
    char *oidString = NULL;
    PyObject *py_oid_str = NULL;

    if ((oiddata = SECOID_FindOID(oid)) != NULL) {
	return PyString_FromString(oiddata->desc);
    }
    if ((oidString = CERT_GetOidString(oid)) != NULL) {
        py_oid_str = PyString_FromString(oidString);
	PR_smprintf_free(oidString);
	return py_oid_str;
    }
    Py_RETURN_NONE;
}

static PyObject *
sec_encoded_oid_str(SECItem *item)
{
    PyObject *str = NULL;
    SECItem stripped_item = *item;

    if (sec_strip_tag_and_length(&stripped_item) == SECSuccess)
	str = sec_oid_str_by_secitem(&stripped_item);

    return str;
}

static PyObject *
sec_utc_time_str(SECItem *item)
{
    PRTime pr_time = 0;
    PRExplodedTime exploded_time;
    char time_str[100];

    if ((DER_UTCTimeToTime(&pr_time, item) != SECSuccess))
        Py_RETURN_NONE;
    PR_ExplodeTime(pr_time, PR_GMTParameters, &exploded_time);
    PR_FormatTime(time_str, sizeof(time_str), time_format, &exploded_time);

    return PyString_FromString(time_str);
}


static PyObject *
sec_generalized_time_str(SECItem *item)
{
    PRTime pr_time = 0;
    PRExplodedTime exploded_time;
    char time_str[100];

    if ((DER_GeneralizedTimeToTime(&pr_time, item) != SECSuccess))
        Py_RETURN_NONE;
    PR_ExplodeTime(pr_time, PR_GMTParameters, &exploded_time);
    PR_FormatTime(time_str, sizeof(time_str), time_format, &exploded_time);

    return PyString_FromString(time_str);
}


PRTime
sec_time_choice(SECItem *item)
{
    PRTime pr_time = 0;

    switch (item->type) {
    case siUTCTime:
        DER_UTCTimeToTime(&pr_time, item);
        break;
    case siGeneralizedTime:
        DER_GeneralizedTimeToTime(&pr_time, item);
        break;
    default:
        PyErr_SetString(PyExc_ValueError, "unknown sec ANS.1 time type");
    }
    return pr_time;
}

static PyObject *
sec_time_choice_str(SECItem *item)
{
    PRTime pr_time = 0;
    PRExplodedTime exploded_time;
    char time_str[100];

    pr_time = sec_time_choice(item);
    PR_ExplodeTime(pr_time, PR_GMTParameters, &exploded_time);
    PR_FormatTime(time_str, sizeof(time_str), time_format, &exploded_time);

    return PyString_FromString(time_str);
}

static PyObject *
sec_octet_str(SECItem *item)
{
    PyObject *str = NULL;
    SECItem stripped_item = *item;

    if (sec_strip_tag_and_length(&stripped_item) == SECSuccess)
        str = raw_data_to_hex(item->data, item->len, 0, HEX_SEPARATOR_DEFAULT);

    return str;
}

static PyObject *
sec_bit_str(SECItem *item)
{
    PyObject *str = NULL;
    SECItem stripped_item = *item;
    int unused_bits;

    if (sec_strip_tag_and_length(&stripped_item) != SECSuccess || stripped_item.len < 2)
        Py_RETURN_NONE;

    unused_bits = *stripped_item.data++;
    stripped_item.len--;

    str = raw_data_to_hex(stripped_item.data, stripped_item.len, 0, HEX_SEPARATOR_DEFAULT);

    if (unused_bits) {
	PyString_ConcatAndDel(&str, PyString_FromFormat("(%d least significant bits unused)", unused_bits));
    }

    return str;
}

static PyObject *
sec_bmp_str(SECItem *item)
{
    PyObject *str = NULL;
    SECItem stripped_item = *item;
    unsigned char * s;
    unsigned char * d;
    int      len;
    SECItem  tmp_item = {0, 0, 0};

    if (sec_strip_tag_and_length(&stripped_item) != SECSuccess)
	goto loser;

    if (stripped_item.len % 2)
    	goto loser;

    len = (int)(stripped_item.len / 2);
    tmp_item.data = (unsigned char *)PORT_Alloc(len);

    if (!tmp_item.data)
    	goto loser;

    tmp_item.len = len;

    for (s = stripped_item.data, d = tmp_item.data ; len > 0; len--) {
    	PRUint32 tmp_char = (s[0] << 8) | s[1]; s += 2;
	if (!isprint(tmp_char))
	    goto loser;
	*d++ = (unsigned char)tmp_char;
    }
    str = PyString_FromString((char *)tmp_item.data);
    PORT_Free(tmp_item.data);
    return str;

loser:
    str = raw_data_to_hex(stripped_item.data, stripped_item.len, 0, HEX_SEPARATOR_DEFAULT);
    if (tmp_item.data)
	PORT_Free(tmp_item.data);
    return str;
}

static PyObject *
sec_universal_str(SECItem *item)
{
    PyObject *str = NULL;
    SECItem stripped_item = *item;
    unsigned char * s;
    unsigned char * d;
    int      len;
    SECItem  tmp_item = {0, 0, 0};

    if (sec_strip_tag_and_length(&stripped_item) != SECSuccess)
	goto loser;

    if (stripped_item.len % 4)
    	goto loser;

    len = (int)(stripped_item.len / 4);
    tmp_item.data = (unsigned char *)PORT_Alloc(len);

    if (!tmp_item.data)
    	goto loser;

    tmp_item.len = len;

    for (s = stripped_item.data, d = tmp_item.data ; len > 0; len--) {
    	PRUint32 tmp_char = (s[0] << 24) | (s[1] << 16) | (s[2] << 8) | s[3];
	if (!isprint(tmp_char))
	    goto loser;
	*d++ = (unsigned char)tmp_char;
    }
    str = PyString_FromString((char *)tmp_item.data);
    PORT_Free(tmp_item.data);
    return str;

loser:
    str = raw_data_to_hex(stripped_item.data, stripped_item.len, 0, HEX_SEPARATOR_DEFAULT);
    if (tmp_item.data)
	PORT_Free(tmp_item.data);
    return str;
}

static PyObject *
sec_universal_item_str(SECItem *item)
{
    switch (item->data[0] & SEC_ASN1_TAGNUM_MASK) {
    case SEC_ASN1_ENUMERATED:
    case SEC_ASN1_INTEGER:
        return sec_encoded_integer_str(item);
    case SEC_ASN1_OBJECT_ID:
        return sec_encoded_oid_str(item);
    case SEC_ASN1_BOOLEAN:
        return sec_encoded_boolean_str(item);
    case SEC_ASN1_UTF8_STRING:
    case SEC_ASN1_PRINTABLE_STRING:
    case SEC_ASN1_VISIBLE_STRING:
    case SEC_ASN1_IA5_STRING:
    case SEC_ASN1_T61_STRING:
        return sec_string_str(item);
    case SEC_ASN1_GENERALIZED_TIME:
        return sec_generalized_time_str(item);
    case SEC_ASN1_UTC_TIME:
        return sec_utc_time_str(item);
    case SEC_ASN1_NULL:
        return PyString_FromString("(null)");
    case SEC_ASN1_SET:
    case SEC_ASN1_SEQUENCE:
        return sec_set_str_list(item);
    case SEC_ASN1_OCTET_STRING:
        return sec_octet_str(item);
    case SEC_ASN1_BIT_STRING:
        sec_bit_str(item);
        break;
    case SEC_ASN1_BMP_STRING:
        return sec_bmp_str(item);
    case SEC_ASN1_UNIVERSAL_STRING:
        return sec_universal_str(item);
    default:
        return raw_data_to_hex(item->data, item->len, 0, HEX_SEPARATOR_DEFAULT);
    }
    Py_RETURN_NONE;
}

static PyObject *
get_algorithm_id_str(SECAlgorithmID *a)
{
    PyObject *str = NULL;

    if ((str = sec_oid_str_by_secitem(&a->algorithm)) == NULL)
        Py_RETURN_NONE;

    if ((a->parameters.len == 0) ||
	(a->parameters.len == 2 && memcmp(a->parameters.data, "\005\000", 2) == 0)) {
	/* No arguments or NULL argument */
    } else {
	/* Print args to algorithm */
        PyObject *hex_args = NULL;

        if ((hex_args = raw_data_to_hex(a->parameters.data, a->parameters.len, 0, HEX_SEPARATOR_DEFAULT)) != NULL) {
            PyString_ConcatAndDel(&str, hex_args);
        }
    }
    return str;
}


static PyObject *
cert_trust_flags_str(unsigned int flags)
{
    PyObject *py_flags = NULL;
    PyObject *py_flag = NULL;

    if ((py_flags = PyList_New(0)) == NULL)
        return NULL;

    if (flags & CERTDB_VALID_PEER) {
	if ((py_flag = PyString_FromString("Valid Peer")) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
    }
    if (flags & CERTDB_TRUSTED) {
	if ((py_flag = PyString_FromString("Trusted")) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
    }
    if (flags & CERTDB_SEND_WARN) {
	if ((py_flag = PyString_FromString("Warn When Sending")) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
    }
    if (flags & CERTDB_VALID_CA) {
	if ((py_flag = PyString_FromString("Valid CA")) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
    }
    if (flags & CERTDB_TRUSTED_CA) {
	if ((py_flag = PyString_FromString("Trusted CA")) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
    }
    if (flags & CERTDB_NS_TRUSTED_CA) {
	if ((py_flag = PyString_FromString("Netscape Trusted CA")) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
    }
    if (flags & CERTDB_USER) {
	if ((py_flag = PyString_FromString("User")) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
    }
    if (flags & CERTDB_TRUSTED_CLIENT_CA) {
	if ((py_flag = PyString_FromString("Trusted Client CA")) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
    }
    if (flags & CERTDB_GOVT_APPROVED_CA) {
	if ((py_flag = PyString_FromString("Step-up")) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
    }
    return py_flags;
}


/* ========================================================================== */
/* =============================== SecItem Class ============================ */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static PyObject *
SecItem_get_type(SecItem *self, void *closure)
{
    return PyInt_FromLong(self->item.type);
}

static PyObject *
SecItem_get_len(SecItem *self, void *closure)
{
    return PyInt_FromLong(self->item.len);
}

static PyObject *
SecItem_get_data(SecItem *self, void *closure)
{
    return PyString_FromStringAndSize((const char *)self->item.data, self->item.len);
}

static
PyGetSetDef SecItem_getseters[] = {
    {"type", (getter)SecItem_get_type, (setter)NULL, "the SecItem type (si* constant)", NULL},
    {"len",  (getter)SecItem_get_len,  (setter)NULL, "number of octets in SecItem buffer", NULL},
    {"data", (getter)SecItem_get_data, (setter)NULL, "contents of SecItem buffer", NULL},
    {NULL}  /* Sentinel */
};

static PyMemberDef SecItem_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

static PyMethodDef SecItem_methods[] = {
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
SecItem_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    SecItem *self;

    TraceObjNewEnter(type);

    if ((self = (SecItem *)type->tp_alloc(type, 0)) == NULL) return NULL;
    self->item.type = 0;
    self->item.len = 0;
    self->item.data = NULL;
    self->kind = SECITEM_unknown;

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static void
SecItem_dealloc(SecItem* self)
{
    TraceMethodEnter(self);

    if (self->item.data) {
        PyMem_FREE(self->item.data);
    }

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(SecItem_doc,
"SecItem(data=None, type=siBuffer)\n\
\n\
:Parameters:\n\
    data : any read buffer compatible object (e.g. buffer or string)\n\
        raw data to initialize from\n\
    type : int\n\
        SECItemType constant (e.g. si*)\n\
\n\
Encoded data. Used internally by NSS\n\
");
static int
SecItem_init(SecItem *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"data", "type", NULL};
    const void *buffer = NULL;
    Py_ssize_t buffer_len;
    int type = siBuffer;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|z#i:SecItem", kwlist,
                                     &buffer, &buffer_len, &type))
        return -1;

    if (buffer) {
        self->kind = SECITEM_buffer;
        self->item.type = type;
        self->item.len = buffer_len;
        if ((self->item.data = PyMem_MALLOC(buffer_len)) == NULL) {
            PyErr_Format(PyExc_MemoryError, "not enough memory to copy buffer of size %d into SecItem",
                         buffer_len);
            return -1;
        }
        memmove(self->item.data, buffer, buffer_len);
    } else {                    /* empty buffer */
        self->kind = SECITEM_buffer;
        self->item.type = siBuffer;
        self->item.len = 0;
        self->item.data = NULL;
    }

    return 0;
}

static PyObject *
SecItem_repr(SecItem *self)
{
    return PyString_FromFormat("<%s object at %p>",
                               Py_TYPE(self)->tp_name, self);
}

static PyObject *
SecItem_str(SecItem *self)
{
    PyObject *return_value = NULL;

    switch(self->kind) {
    case SECITEM_dist_name:
        {
            char *name;

            if ((name = CERT_DerNameToAscii(&self->item)) == NULL) {
                return set_nspr_error(NULL);
            }
            return_value = PyString_FromString(name);
            PORT_Free(name);
        }
        break;
    case SECITEM_algorithm:
        return sec_oid_str_by_secitem(&self->item);
    default:
        return_value =  obj_to_hex((PyObject *)self, 0, HEX_SEPARATOR_DEFAULT);

        break;
    }
    return return_value;
}

/* =========================== Buffer Protocol ========================== */

static Py_ssize_t
SecItem_buffer_getbuf(PyObject *obj, Py_ssize_t index, void **ptr)
{
    SecItem *self = (SecItem *) obj;
    if (index != 0) {
        PyErr_SetString(PyExc_SystemError, "Accessing non-existent segment");
        return -1;
    }
    *ptr = self->item.data;
    return self->item.len;
}

static Py_ssize_t
SecItem_buffer_getsegcount(PyObject *obj, Py_ssize_t *lenp)
{
    if (lenp)
        *lenp = 1;
    return 1;
}

static PyBufferProcs SecItem_as_buffer = {
    SecItem_buffer_getbuf,			/* bf_getreadbuffer */
    SecItem_buffer_getbuf,			/* bf_getwritebuffer */
    SecItem_buffer_getsegcount,			/* bf_getsegcount */
    NULL,					/* bf_getcharbuffer */
};

static Py_ssize_t
SecItem_length(SecItem *self)
{
    return self->item.len;
}

static PyObject *
SecItem_item(SecItem *self, register Py_ssize_t i)
{
    char octet;

    if (i < 0 || i >= self->item.len) {
        PyErr_SetString(PyExc_IndexError, "SecItem index out of range");
        return NULL;
    }
    octet = self->item.data[i];
    return PyString_FromStringAndSize(&octet, 1);
}

/* slice a[i:j] consists of octets a[i] ... a[j-1], j -- may be negative! */
static PyObject *
SecItem_slice(SecItem *a, Py_ssize_t i, Py_ssize_t j)
{
    if (i < 0)
        i = 0;
    if (j < 0)
        j = 0;
    if (j > SecItem_GET_SIZE(a))
        j = SecItem_GET_SIZE(a);
    if (j < i)
        j = i;
    return PyString_FromStringAndSize((const char *)(a->item.data + i), j-i);
}

static PyObject*
SecItem_subscript(SecItem *self, PyObject* item)
{
    if (PyIndex_Check(item)) {
        Py_ssize_t i = PyNumber_AsSsize_t(item, PyExc_IndexError);
        if (i == -1 && PyErr_Occurred())
            return NULL;
        if (i < 0)
            i += SecItem_GET_SIZE(self);
        return SecItem_item(self, i);
    }
    else if (PySlice_Check(item)) {
        Py_ssize_t start, stop, step, slice_len, cur, i;
        unsigned char* src;
        unsigned char* dst;
        PyObject* result;

        if (PySlice_GetIndicesEx((PySliceObject*)item, SecItem_GET_SIZE(self),
				 &start, &stop, &step, &slice_len) < 0) {
            return NULL;
        }

        if (slice_len <= 0) {
            return PyString_FromStringAndSize("", 0);
        } else if (step == 1) {
            return PyString_FromStringAndSize((char *)self->item.data + start, slice_len);
        } else {
            src = self->item.data;
            if ((result = PyString_FromStringAndSize(NULL, slice_len)) == NULL) {
                return NULL;
            }
            dst = (unsigned char *)PyString_AsString(result);
            for (cur = start, i = 0; i < slice_len; cur += step, i++) {
                dst[i] = src[cur];
            }
            return result;
        }
    } else {
        PyErr_Format(PyExc_TypeError, "SecItem indices must be integers, not %.200s",
                     Py_TYPE(item)->tp_name);
        return NULL;
    }
}

static PySequenceMethods SecItem_as_sequence = {
    (lenfunc)SecItem_length,			/* sq_length */
    0,						/* sq_concat */
    0,						/* sq_repeat */
    (ssizeargfunc)SecItem_item,			/* sq_item */
    (ssizessizeargfunc)SecItem_slice,		/* sq_slice */
    0,						/* sq_ass_item */
    0,						/* sq_ass_slice */
    0,						/* sq_contains */
    0,						/* sq_inplace_concat */
    0,						/* sq_inplace_repeat */
};

static PyMappingMethods SecItem_as_mapping = {
    (lenfunc)SecItem_length,			/* mp_length */
    (binaryfunc)SecItem_subscript,		/* mp_subscript */
    0,						/* mp_ass_subscript */
};

static PyTypeObject SecItemType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.SecItem",				/* tp_name */
    sizeof(SecItem),				/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)SecItem_dealloc,		/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    0,						/* tp_compare */
    (reprfunc)SecItem_repr,			/* tp_repr */
    0,						/* tp_as_number */
    &SecItem_as_sequence,			/* tp_as_sequence */
    &SecItem_as_mapping,			/* tp_as_mapping */
    0,						/* tp_hash */
    0,						/* tp_call */
    (reprfunc)SecItem_str,			/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    &SecItem_as_buffer,				/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
    SecItem_doc,				/* tp_doc */
    0,						/* tp_traverse */
    0,						/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    SecItem_methods,				/* tp_methods */
    SecItem_members,				/* tp_members */
    SecItem_getseters,				/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)SecItem_init,			/* tp_init */
    0,						/* tp_alloc */
    SecItem_new,				/* tp_new */
};

PyObject *
SecItem_new_from_SECItem(SECItem *item, SECItemKind kind)
{
    SecItem *self = NULL;

    TraceObjNewEnter(NULL);

    if ((self = (SecItem *) SecItemType.tp_new(&SecItemType, NULL, NULL)) == NULL)
        return NULL;

    self->item.type = item->type;
    self->item.len = item->len;
    if ((self->item.data = PyMem_MALLOC(item->len)) == NULL) {
        return PyErr_NoMemory();
    }
    memmove(self->item.data, item->data, item->len);

    self->kind = kind;

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

/* ========================================================================== */
/* ======================== SignatureAlgorithm Class ======================== */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static
PyGetSetDef SignatureAlgorithm_getseters[] = {
    {NULL}  /* Sentinel */
};

static PyMemberDef SignatureAlgorithm_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

static PyMethodDef SignatureAlgorithm_methods[] = {
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
SignatureAlgorithm_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    SignatureAlgorithm *self;

    TraceObjNewEnter(type);

    if ((self = (SignatureAlgorithm *)type->tp_alloc(type, 0)) == NULL) return NULL;

    memset(&self->id, 0, sizeof(self->id));
    self->py_id = NULL;
    self->py_parameters = NULL;

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static void
SignatureAlgorithm_dealloc(SignatureAlgorithm* self)
{
    TraceMethodEnter(self);

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(SignatureAlgorithm_doc,
"An object representing a signature algorithm");

static int
SignatureAlgorithm_init(SignatureAlgorithm *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter(self);

    return 0;
}

static PyObject *
SignatureAlgorithm_repr(SignatureAlgorithm *self)
{
    return PyString_FromFormat("<%s object at %p>",
                               Py_TYPE(self)->tp_name, self);
}

static PyObject *
SignatureAlgorithm_str(SignatureAlgorithm *self)
{
    return get_algorithm_id_str(&self->id);
}

static PyTypeObject SignatureAlgorithmType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.SignatureAlgorithm",		/* tp_name */
    sizeof(SignatureAlgorithm),			/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)SignatureAlgorithm_dealloc,	/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    0,						/* tp_compare */
    (reprfunc)SignatureAlgorithm_repr,		/* tp_repr */
    0,						/* tp_as_number */
    0,						/* tp_as_sequence */
    0,						/* tp_as_mapping */
    0,						/* tp_hash */
    0,						/* tp_call */
    (reprfunc)SignatureAlgorithm_str,		/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
    SignatureAlgorithm_doc,			/* tp_doc */
    0,						/* tp_traverse */
    0,						/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    SignatureAlgorithm_methods,			/* tp_methods */
    SignatureAlgorithm_members,			/* tp_members */
    SignatureAlgorithm_getseters,		/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)SignatureAlgorithm_init,		/* tp_init */
    0,						/* tp_alloc */
    SignatureAlgorithm_new,			/* tp_new */
};

PyObject *
SignatureAlgorithm_new_from_SECAlgorithmID(SECAlgorithmID *id)
{
    SignatureAlgorithm *self = NULL;

    TraceObjNewEnter(NULL);

    if ((self = (SignatureAlgorithm *) SignatureAlgorithmType.tp_new(&SignatureAlgorithmType, NULL, NULL)) == NULL)
        return NULL;

    self->id = *id;
    if ((self->py_id = SecItem_new_from_SECItem(&id->algorithm, SECITEM_algorithm)) == NULL)
        return NULL;
    if ((self->py_parameters = SecItem_new_from_SECItem(&id->parameters, SECITEM_unknown)) == NULL)
        return NULL;

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

/* ========================================================================== */
/* ============================ KEYPQGParams Class ========================== */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static PyObject *
KEYPQGParams_get_prime(KEYPQGParams *self, void *closure)
{
    Py_INCREF(self->py_prime);
    return self->py_prime;
}

static PyObject *
KEYPQGParams_get_subprime(KEYPQGParams *self, void *closure)
{
    Py_INCREF(self->py_subprime);
    return self->py_subprime;
}

static PyObject *
KEYPQGParams_get_base(KEYPQGParams *self, void *closure)
{
    Py_INCREF(self->py_base);
    return self->py_base;
}

static
PyGetSetDef KEYPQGParams_getseters[] = {
    {"prime",    (getter)KEYPQGParams_get_prime,    (setter)NULL, "key prime value, also known as p", NULL},
    {"subprime", (getter)KEYPQGParams_get_subprime, (setter)NULL, "key subprime value, also known as q", NULL},
    {"base",     (getter)KEYPQGParams_get_base,     (setter)NULL, "key base value, also known as g", NULL},
    {NULL}  /* Sentinel */
};

static PyMemberDef KEYPQGParams_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

static PyMethodDef KEYPQGParams_methods[] = {
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
KEYPQGParams_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    KEYPQGParams *self;

    TraceObjNewEnter(type);

    if ((self = (KEYPQGParams *)type->tp_alloc(type, 0)) == NULL) return NULL;

    self->py_prime = NULL;
    self->py_subprime = NULL;
    self->py_base = NULL;

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static void
KEYPQGParams_dealloc(KEYPQGParams* self)
{
    TraceMethodEnter(self);

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(KEYPQGParams_doc,
"An object representing key parameters\n\
    - prime (also known as p)\n\
    - subprime (also known as q)\n\
    - base (also known as g)\n\
");

static int
KEYPQGParams_init(KEYPQGParams *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter(self);

    return 0;
}

static PyObject *
KEYPQGParams_repr(KEYPQGParams *self)
{
    return PyString_FromFormat("<%s object at %p>",
                               Py_TYPE(self)->tp_name, self);
}

static PyObject *
KEYPQGParams_str(KEYPQGParams *self)
{
    PyObject *fmt = NULL;
    PyObject *args = NULL;
    PyObject *str = NULL;

    TraceMethodEnter(self);

    if ((fmt = PyString_FromString("prime(p)=%s subprime(q)=%s base(g)=%s")) == NULL)
        return NULL;
    if ((args = PyTuple_New(3)) == NULL)
        return NULL;

    PyTuple_SET_ITEM(args, 0, PyObject_Str(self->py_prime));
    PyTuple_SET_ITEM(args, 1, PyObject_Str(self->py_subprime));
    PyTuple_SET_ITEM(args, 2, PyObject_Str(self->py_base));

    str = PyString_Format(fmt, args);

    Py_DECREF(fmt);
    Py_DECREF(args);
    return str;
}

static PyTypeObject KEYPQGParamsType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.KEYPQGParams",				/* tp_name */
    sizeof(KEYPQGParams),			/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)KEYPQGParams_dealloc,		/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    0,						/* tp_compare */
    (reprfunc)KEYPQGParams_repr,		/* tp_repr */
    0,						/* tp_as_number */
    0,						/* tp_as_sequence */
    0,						/* tp_as_mapping */
    0,						/* tp_hash */
    0,						/* tp_call */
    (reprfunc)KEYPQGParams_str,			/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
    KEYPQGParams_doc,				/* tp_doc */
    0,						/* tp_traverse */
    0,						/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    KEYPQGParams_methods,			/* tp_methods */
    KEYPQGParams_members,			/* tp_members */
    KEYPQGParams_getseters,			/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)KEYPQGParams_init,		/* tp_init */
    0,						/* tp_alloc */
    KEYPQGParams_new,				/* tp_new */
};

PyObject *
KEYPQGParams_new_from_SECKEYPQGParams(SECKEYPQGParams *params)
{
    KEYPQGParams *self = NULL;

    TraceObjNewEnter(NULL);

    if ((self = (KEYPQGParams *) KEYPQGParamsType.tp_new(&KEYPQGParamsType, NULL, NULL)) == NULL)
        return NULL;

    if ((self->py_prime = SecItem_new_from_SECItem(&params->prime, SECITEM_unknown)) == NULL)
        return NULL;

    if ((self->py_subprime = SecItem_new_from_SECItem(&params->subPrime, SECITEM_unknown)) == NULL)
        return NULL;

    if ((self->py_base = SecItem_new_from_SECItem(&params->base, SECITEM_unknown)) == NULL)
        return NULL;

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

/* ========================================================================== */
/* =========================== RSAPublicKey Class =========================== */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static PyObject *
RSAPublicKey_get_modulus(RSAPublicKey *self, void *closure)
{
    Py_INCREF(self->py_modulus);
    return self->py_modulus;
}

static PyObject *
RSAPublicKey_get_exponent(RSAPublicKey *self, void *closure)
{
    Py_INCREF(self->py_exponent);
    return self->py_exponent;
}

static
PyGetSetDef RSAPublicKey_getseters[] = {
    {"modulus",  (getter)RSAPublicKey_get_modulus,  (setter)NULL, "RSA modulus", NULL},
    {"exponent", (getter)RSAPublicKey_get_exponent, (setter)NULL, "RSA exponent", NULL},
    {NULL}  /* Sentinel */
};

static PyMemberDef RSAPublicKey_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

static PyObject *
RSAPublicKey_format_lines(RSAPublicKey *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"level", NULL};
    int level = 0;
    PyObject *lines = NULL;
    PyObject *obj = NULL;
    PyObject *obj_lines = NULL;
    int i;
    Py_ssize_t len;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:format_lines", kwlist, &level))
        return NULL;

    if ((lines = PyList_New(0)) == NULL) {
        return NULL;
    }

    FMT_LABEL_AND_APPEND(level, "Modulus", lines, fail);

    if ((obj = RSAPublicKey_get_modulus(self, NULL)) == NULL)
        goto fail;
    obj_lines = obj_to_hex(obj, OCTETS_PER_LINE_DEFAULT, HEX_SEPARATOR_DEFAULT);
    Py_DECREF(obj);

    level += 1;
    len = PyList_Size(obj_lines);
    for (i = 0; i < len; i++) {
        obj = PyList_GET_ITEM(obj_lines, i);
        FMT_OBJ_AND_APPEND(level, NULL, obj, lines, fail);
    }

    Py_DECREF(obj_lines);
    return lines;
 fail:
    Py_XDECREF(obj);
    Py_XDECREF(lines);
    Py_XDECREF(obj_lines);
    return NULL;
}

static PyObject *
RSAPublicKey_format(RSAPublicKey *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter(self);

    return format_from_lines((format_lines_func)RSAPublicKey_format_lines, (PyObject *)self, args, kwds);
}

static PyObject *
RSAPublicKey_str(RSAPublicKey *self)
{
    PyObject *py_formatted_result = NULL;

    TraceMethodEnter(self);

    py_formatted_result =  RSAPublicKey_format(self, empty_tuple, NULL);
    return py_formatted_result;

}

static PyMethodDef RSAPublicKey_methods[] = {
    {"format_lines", (PyCFunction)RSAPublicKey_format_lines,   METH_VARARGS|METH_KEYWORDS, generic_format_lines_doc},
    {"format",       (PyCFunction)RSAPublicKey_format,         METH_VARARGS|METH_KEYWORDS, generic_format_doc},
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
RSAPublicKey_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    RSAPublicKey *self;

    TraceObjNewEnter(type);

    if ((self = (RSAPublicKey *)type->tp_alloc(type, 0)) == NULL) return NULL;

    self->py_modulus = NULL;
    self->py_exponent = NULL;

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static void
RSAPublicKey_dealloc(RSAPublicKey* self)
{
    TraceMethodEnter(self);

    Py_XDECREF(self->py_modulus);
    Py_XDECREF(self->py_exponent);

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(RSAPublicKey_doc,
"An object representing an RSA Public Key");

static int
RSAPublicKey_init(RSAPublicKey *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter(self);

    return 0;
}

static PyObject *
RSAPublicKey_repr(RSAPublicKey *self)
{
    return PyString_FromFormat("<%s object at %p>",
                               Py_TYPE(self)->tp_name, self);
}

static PyTypeObject RSAPublicKeyType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.RSAPublicKey",				/* tp_name */
    sizeof(RSAPublicKey),			/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)RSAPublicKey_dealloc,		/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    0,						/* tp_compare */
    (reprfunc)RSAPublicKey_repr,		/* tp_repr */
    0,						/* tp_as_number */
    0,						/* tp_as_sequence */
    0,						/* tp_as_mapping */
    0,						/* tp_hash */
    0,						/* tp_call */
    (reprfunc)RSAPublicKey_str,			/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
    RSAPublicKey_doc,				/* tp_doc */
    0,						/* tp_traverse */
    0,						/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    RSAPublicKey_methods,			/* tp_methods */
    RSAPublicKey_members,			/* tp_members */
    RSAPublicKey_getseters,			/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)RSAPublicKey_init,		/* tp_init */
    0,						/* tp_alloc */
    RSAPublicKey_new,				/* tp_new */
};

PyObject *
RSAPublicKey_new_from_SECKEYRSAPublicKey(SECKEYRSAPublicKey *rsa)
{
    RSAPublicKey *self = NULL;

    TraceObjNewEnter(NULL);

    if ((self = (RSAPublicKey *) RSAPublicKeyType.tp_new(&RSAPublicKeyType, NULL, NULL)) == NULL)
        return NULL;

    if ((self->py_modulus = SecItem_new_from_SECItem(&rsa->modulus, SECITEM_unknown)) == NULL)
        return NULL;

    if ((self->py_exponent = SecItem_new_from_SECItem(&rsa->publicExponent, SECITEM_unknown)) == NULL)
        return NULL;

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

/* ========================================================================== */
/* =========================== DSAPublicKey Class =========================== */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static PyObject *
DSAPublicKey_get_pqg_params(DSAPublicKey *self, void *closure)
{
    Py_INCREF(self->py_pqg_params);
    return self->py_pqg_params;
}

static PyObject *
DSAPublicKey_get_public_value(DSAPublicKey *self, void *closure)
{
    Py_INCREF(self->py_public_value);
    return self->py_public_value;
}

static
PyGetSetDef DSAPublicKey_getseters[] = {
    {"pqg_params",   (getter)DSAPublicKey_get_pqg_params,   (setter)NULL, "DSA P,Q,G params as a KEYPQGParams object", NULL},
    {"public_value", (getter)DSAPublicKey_get_public_value, (setter)NULL, "DSA public_value", NULL},
    {NULL}  /* Sentinel */
};

static PyMemberDef DSAPublicKey_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

static PyMethodDef DSAPublicKey_methods[] = {
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
DSAPublicKey_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    DSAPublicKey *self;

    TraceObjNewEnter(type);

    if ((self = (DSAPublicKey *)type->tp_alloc(type, 0)) == NULL) return NULL;

    self->py_pqg_params = NULL;
    self->py_public_value = NULL;

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static void
DSAPublicKey_dealloc(DSAPublicKey* self)
{
    TraceMethodEnter(self);

    Py_XDECREF(self->py_pqg_params);
    Py_XDECREF(self->py_public_value);

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(DSAPublicKey_doc,
"A object representing a DSA Public Key");

static int
DSAPublicKey_init(DSAPublicKey *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter(self);

    return 0;
}

static PyObject *
DSAPublicKey_repr(DSAPublicKey *self)
{
    return PyString_FromFormat("<%s object at %p>",
                               Py_TYPE(self)->tp_name, self);
}

static PyObject *
DSAPublicKey_str(DSAPublicKey *self)
{
    PyObject *fmt = NULL;
    PyObject *args = NULL;
    PyObject *str = NULL;

    TraceMethodEnter(self);

    if ((fmt = PyString_FromString("pqg_params=[%s] public_value=%s")) == NULL)
        return NULL;
    if ((args = PyTuple_New(2)) == NULL)
        return NULL;

    PyTuple_SET_ITEM(args, 0, PyObject_Str(self->py_pqg_params));
    PyTuple_SET_ITEM(args, 1, PyObject_Str(self->py_public_value));

    str = PyString_Format(fmt, args);

    Py_DECREF(fmt);
    Py_DECREF(args);
    return str;
}

static PyTypeObject DSAPublicKeyType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.DSAPublicKey",				/* tp_name */
    sizeof(DSAPublicKey),			/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)DSAPublicKey_dealloc,		/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    0,						/* tp_compare */
    (reprfunc)DSAPublicKey_repr,		/* tp_repr */
    0,						/* tp_as_number */
    0,						/* tp_as_sequence */
    0,						/* tp_as_mapping */
    0,						/* tp_hash */
    0,						/* tp_call */
    (reprfunc)DSAPublicKey_str,			/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
    DSAPublicKey_doc,				/* tp_doc */
    0,						/* tp_traverse */
    0,						/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    DSAPublicKey_methods,			/* tp_methods */
    DSAPublicKey_members,			/* tp_members */
    DSAPublicKey_getseters,			/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)DSAPublicKey_init,		/* tp_init */
    0,						/* tp_alloc */
    DSAPublicKey_new,				/* tp_new */
};

PyObject *
DSAPublicKey_new_from_SECKEYDSAPublicKey(SECKEYDSAPublicKey *dsa)
{
    DSAPublicKey *self = NULL;

    TraceObjNewEnter(NULL);

    if ((self = (DSAPublicKey *) DSAPublicKeyType.tp_new(&DSAPublicKeyType, NULL, NULL)) == NULL)
        return NULL;

    if ((self->py_pqg_params = KEYPQGParams_new_from_SECKEYPQGParams(&dsa->params)) == NULL)
        return NULL;

    if ((self->py_public_value = SecItem_new_from_SECItem(&dsa->publicValue, SECITEM_unknown)) == NULL)
        return NULL;

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

/* ========================================================================== */
/* ============================= SignedData Class =========================== */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static PyObject *
SignedData_get_algorithm(SignedData *self, void *closure)
{
    Py_INCREF(self->py_algorithm);
    return self->py_algorithm;
}

static PyObject *
SignedData_get_signature(SignedData *self, void *closure)
{
    Py_INCREF(self->py_signature);
    return self->py_signature;
}

static
PyGetSetDef SignedData_getseters[] = {
    {"algorithm",  (getter)SignedData_get_algorithm,  (setter)NULL, "signature algorithm as a SignatureAlgorithm object", NULL},
    {"signature",  (getter)SignedData_get_signature,  (setter)NULL, "signature as a SecItem object", NULL},
    {NULL}  /* Sentinel */
};

static PyMemberDef SignedData_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

static PyObject *
SignedData_format_lines(SignedData *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"level", NULL};
    int level = 0;
    PyObject *lines = NULL;
    PyObject *obj = NULL;
    PyObject *obj_lines = NULL;
    int i;
    Py_ssize_t len;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:format_lines", kwlist, &level))
        return NULL;

    if ((lines = PyList_New(0)) == NULL) {
        return NULL;
    }

    if ((obj = SignedData_get_algorithm(self, NULL)) == NULL)
        goto fail;
    FMT_OBJ_AND_APPEND(level, "Signature Algorithm", obj, lines, fail);
    Py_DECREF(obj);

    FMT_LABEL_AND_APPEND(level, "Signature Data", lines, fail);

    if ((obj = SignedData_get_signature(self, NULL)) == NULL)
        goto fail;
    obj_lines = obj_to_hex(obj, OCTETS_PER_LINE_DEFAULT, HEX_SEPARATOR_DEFAULT);
    Py_DECREF(obj);

    level += 1;
    len = PyList_Size(obj_lines);
    for (i = 0; i < len; i++) {
        obj = PyList_GET_ITEM(obj_lines, i);
        FMT_OBJ_AND_APPEND(level, NULL, obj, lines, fail);
    }

    Py_DECREF(obj_lines);
    return lines;
 fail:
    Py_XDECREF(obj);
    Py_XDECREF(lines);
    Py_XDECREF(obj_lines);
    return NULL;
}

static PyObject *
SignedData_format(SignedData *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter(self);

    return format_from_lines((format_lines_func)SignedData_format_lines, (PyObject *)self, args, kwds);
}

static PyObject *
SignedData_str(SignedData *self)
{
    PyObject *py_formatted_result = NULL;

    py_formatted_result =  SignedData_format(self, empty_tuple, NULL);
    return py_formatted_result;

}

static PyMethodDef SignedData_methods[] = {
    {"format_lines", (PyCFunction)SignedData_format_lines,   METH_VARARGS|METH_KEYWORDS, generic_format_lines_doc},
    {"format",       (PyCFunction)SignedData_format,         METH_VARARGS|METH_KEYWORDS, generic_format_doc},
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
SignedData_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    SignedData *self;

    TraceObjNewEnter(type);

    if ((self = (SignedData *)type->tp_alloc(type, 0)) == NULL) return NULL;

    self->py_data = NULL;
    self->py_algorithm = NULL;
    self->py_signature = NULL;

    if ((self->arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE)) == NULL ) {
        return PyErr_NoMemory();
    }

    memset(&self->signed_data, 0, sizeof(self->signed_data));

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static void
SignedData_dealloc(SignedData* self)
{
    TraceMethodEnter(self);

    Py_XDECREF(self->py_data);
    Py_XDECREF(self->py_algorithm);
    Py_XDECREF(self->py_signature);

    PORT_FreeArena(self->arena, PR_FALSE);

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(SignedData_doc,
"A object representing a signature");

static int
SignedData_init(SignedData *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter(self);

    return 0;
}

static PyObject *
SignedData_repr(SignedData *self)
{
    return PyString_FromFormat("<%s object at %p>",
                               Py_TYPE(self)->tp_name, self);
}

static PyTypeObject SignedDataType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.SignedData",				/* tp_name */
    sizeof(SignedData),				/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)SignedData_dealloc,		/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    0,						/* tp_compare */
    (reprfunc)SignedData_repr,			/* tp_repr */
    0,						/* tp_as_number */
    0,						/* tp_as_sequence */
    0,						/* tp_as_mapping */
    0,						/* tp_hash */
    0,						/* tp_call */
    (reprfunc)SignedData_str,			/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
    SignedData_doc,				/* tp_doc */
    0,						/* tp_traverse */
    0,						/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    SignedData_methods,				/* tp_methods */
    SignedData_members,				/* tp_members */
    SignedData_getseters,			/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)SignedData_init,			/* tp_init */
    0,						/* tp_alloc */
    SignedData_new,				/* tp_new */
};

PyObject *
SignedData_new_from_SECItem(SECItem *item)
{
    SignedData *self = NULL;

    TraceObjNewEnter(NULL);

    if ((self = (SignedData *) SignedDataType.tp_new(&SignedDataType, NULL, NULL)) == NULL)
        return NULL;

    if (SEC_ASN1DecodeItem(self->arena, &self->signed_data,
                           SEC_ASN1_GET(CERT_SignedDataTemplate), item) != SECSuccess) {
        return NULL;
    }

    if ((self->py_data = SecItem_new_from_SECItem(&self->signed_data.data, SECITEM_unknown)) == NULL)
        return NULL;

    if ((self->py_algorithm = SignatureAlgorithm_new_from_SECAlgorithmID(&self->signed_data.signatureAlgorithm)) == NULL)
        return NULL;

    DER_ConvertBitString(&self->signed_data.signature);
    if ((self->py_signature = SecItem_new_from_SECItem(&self->signed_data.signature, SECITEM_signature)) == NULL)
        return NULL;

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

/* ========================================================================== */
/* ============================= PublicKey Class ============================ */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static PyObject *
PublicKey_get_key_type(PublicKey *self, void *closure)
{
    return PyInt_FromLong(self->pk->keyType);
}

static PyObject *
PublicKey_get_key_type_str(PublicKey *self, void *closure)
{
    return PyString_FromString(key_type_str(self->pk->keyType));
}

static PyObject *
PublicKey_get_rsa(PublicKey *self, void *closure)
{
    if (self->pk->keyType == rsaKey) {
        Py_INCREF(self->py_rsa_key);
        return self->py_rsa_key;
    } else {
        PyErr_Format(PyExc_AttributeError, "when '%.50s' object has key_type=%s there is no attribute 'rsa'",
                     Py_TYPE(self)->tp_name, key_type_str(self->pk->keyType));
        return NULL;
    }
}

static PyObject *
PublicKey_get_dsa(PublicKey *self, void *closure)
{
    if (self->pk->keyType == dsaKey) {
        Py_INCREF(self->py_dsa_key);
        return self->py_dsa_key;
    } else {
        PyErr_Format(PyExc_AttributeError, "when '%.50s' object has key_type=%s there is no attribute 'dsa'",
                     Py_TYPE(self)->tp_name, key_type_str(self->pk->keyType));
        return NULL;
    }
}

static
PyGetSetDef PublicKey_getseters[] = {
    {"key_type",     (getter)PublicKey_get_key_type,     (setter)NULL, "key type (e.g. rsaKey, dsaKey, etc.) as an int", NULL},
    {"key_type_str", (getter)PublicKey_get_key_type_str, (setter)NULL, "key type as a string", NULL},
    {"rsa",          (getter)PublicKey_get_rsa,          (setter)NULL, "RSA key as a RSAPublicKey object", NULL},
    {"dsa",          (getter)PublicKey_get_dsa,          (setter)NULL, "RSA key as a RSAPublicKey object", NULL},
    {NULL}  /* Sentinel */
};

static PyMemberDef PublicKey_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

static PyObject *
PublicKey_format_lines(PublicKey *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"level", NULL};
    int level = 0;
    PyObject *lines = NULL;
    PyObject *obj = NULL;
    PyObject *obj_lines = NULL;
    int i;
    Py_ssize_t len;
    PyObject *tmp_args = NULL;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:format_lines", kwlist, &level))
        return NULL;

    if ((lines = PyList_New(0)) == NULL)
        goto fail;

    if ((tmp_args = Py_BuildValue("(i)", level)) == NULL)
        goto fail;

    switch(self->pk->keyType) {       /* FIXME: handle the other cases */
    case rsaKey:
        FMT_LABEL_AND_APPEND(level, "RSA Public Key", lines, fail);
        if ((obj_lines = RSAPublicKey_format_lines((RSAPublicKey *)self->py_rsa_key, tmp_args, NULL)) == NULL)
            goto fail;
        break;
    case dsaKey:
        break;
    case fortezzaKey:
    case dhKey:
    case keaKey:
    case ecKey:
    case nullKey:
        break;
    }
    Py_DECREF(tmp_args);

    if (obj_lines) {
        len = PyList_Size(obj_lines);
        for (i = 0; i < len; i++) {
            obj = PyList_GET_ITEM(obj_lines, i);
            PyList_Append(lines, obj);
        }

        Py_DECREF(obj_lines);
    }
    return lines;
 fail:
    Py_XDECREF(obj);
    Py_XDECREF(lines);
    Py_XDECREF(obj_lines);
    Py_XDECREF(tmp_args);
    return NULL;
}

static PyObject *
PublicKey_format(PublicKey *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter(self);

    return format_from_lines((format_lines_func)PublicKey_format_lines, (PyObject *)self, args, kwds);
}

static PyObject *
PublicKey_str(PublicKey *self)
{
    PyObject *py_formatted_result = NULL;

    py_formatted_result = PublicKey_format(self, empty_tuple, NULL);
    return py_formatted_result;

}

static PyMethodDef PublicKey_methods[] = {
    {"format_lines", (PyCFunction)PublicKey_format_lines,   METH_VARARGS|METH_KEYWORDS, generic_format_lines_doc},
    {"format",       (PyCFunction)PublicKey_format,         METH_VARARGS|METH_KEYWORDS, generic_format_doc},
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
PublicKey_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PublicKey *self;

    TraceObjNewEnter(type);

    if ((self = (PublicKey *)type->tp_alloc(type, 0)) == NULL) return NULL;

    self->py_rsa_key = NULL;
    self->py_dsa_key = NULL;

    memset(&self->pk, 0, sizeof(self->pk));

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static void
PublicKey_dealloc(PublicKey* self)
{
    TraceMethodEnter(self);

    Py_XDECREF(self->py_rsa_key);
    Py_XDECREF(self->py_dsa_key);

    SECKEY_DestroyPublicKey(self->pk);

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(PublicKey_doc,
"An object representing a Public Key");

static int
PublicKey_init(PublicKey *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter(self);

    return 0;
}

static PyObject *
PublicKey_repr(PublicKey *self)
{
    return PyString_FromFormat("<%s object at %p>",
                               Py_TYPE(self)->tp_name, self);
}

static PyTypeObject PublicKeyType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.PublicKey",				/* tp_name */
    sizeof(PublicKey),				/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)PublicKey_dealloc,		/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    0,						/* tp_compare */
    (reprfunc)PublicKey_repr,			/* tp_repr */
    0,						/* tp_as_number */
    0,						/* tp_as_sequence */
    0,						/* tp_as_mapping */
    0,						/* tp_hash */
    0,						/* tp_call */
    (reprfunc)PublicKey_str,			/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
    PublicKey_doc,				/* tp_doc */
    0,						/* tp_traverse */
    0,						/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    PublicKey_methods,				/* tp_methods */
    PublicKey_members,				/* tp_members */
    PublicKey_getseters,			/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)PublicKey_init,			/* tp_init */
    0,						/* tp_alloc */
    PublicKey_new,				/* tp_new */
};

PyObject *
PublicKey_new_from_SECKEYPublicKey(SECKEYPublicKey *pk)
{
    PublicKey *self = NULL;

    TraceObjNewEnter(NULL);

    if ((self = (PublicKey *) PublicKeyType.tp_new(&PublicKeyType, NULL, NULL)) == NULL)
        return NULL;


    self->pk = pk;

    switch(pk->keyType) {       /* FIXME: handle the other cases */
    case rsaKey:
        self->py_rsa_key = RSAPublicKey_new_from_SECKEYRSAPublicKey(&pk->u.rsa);
        break;
    case dsaKey:
        self->py_dsa_key = DSAPublicKey_new_from_SECKEYDSAPublicKey(&pk->u.dsa);
        break;
    case fortezzaKey:
    case dhKey:
    case keaKey:
    case ecKey:
    case nullKey:
        break;
    }

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

/* ========================================================================== */
/* ======================= SubjectPublicKeyInfo Class ======================= */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static PyObject *
SubjectPublicKeyInfo_get_algorithm(SubjectPublicKeyInfo *self, void *closure)
{
    Py_INCREF(self->py_algorithm);
    return self->py_algorithm;
}

static PyObject *
SubjectPublicKeyInfo_get_public_key(SubjectPublicKeyInfo *self, void *closure)
{
    Py_INCREF(self->py_public_key);
    return self->py_public_key;
}

static
PyGetSetDef SubjectPublicKeyInfo_getseters[] = {
    {"algorithm",  (getter)SubjectPublicKeyInfo_get_algorithm,  (setter)NULL, "algorithm", NULL},
    {"public_key", (getter)SubjectPublicKeyInfo_get_public_key, (setter)NULL, "PublicKey object", NULL},
    {NULL}  /* Sentinel */
};

static PyMemberDef SubjectPublicKeyInfo_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

static PyObject *
SubjectPublicKeyInfo_format_lines(SubjectPublicKeyInfo *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"level", NULL};
    int level = 0;
    PyObject *lines = NULL;
    PyObject *obj = NULL;
    PyObject *obj_lines = NULL;
    int i;
    Py_ssize_t len;
    PublicKey *py_public_key = NULL;
    PyObject *tmp_args = NULL;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:format_lines", kwlist, &level))
        return NULL;

    if ((lines = PyList_New(0)) == NULL) {
        return NULL;
    }

    if ((obj = SubjectPublicKeyInfo_get_algorithm(self, NULL)) == NULL)
        goto fail;
    FMT_OBJ_AND_APPEND(level, "Public Key Algorithm", obj, lines, fail);

    if ((py_public_key = (PublicKey *)SubjectPublicKeyInfo_get_public_key(self, NULL)) == NULL)
        goto fail;

    if ((tmp_args = Py_BuildValue("(i)", level+1)) == NULL)
        goto fail;
    if ((obj_lines = PublicKey_format_lines((PublicKey *)py_public_key, tmp_args, NULL)) == NULL)
        goto fail;
    Py_DECREF(tmp_args);

    if (obj_lines) {
        level += 1;
        len = PyList_Size(obj_lines);
        for (i = 0; i < len; i++) {
            obj = PyList_GET_ITEM(obj_lines, i);
            PyList_Append(lines, obj);
        }

        Py_DECREF(obj_lines);
    }
    return lines;
 fail:
    Py_XDECREF(tmp_args);
    Py_XDECREF(lines);
    Py_XDECREF(obj_lines);
    return NULL;
}

static PyObject *
SubjectPublicKeyInfo_format(SubjectPublicKeyInfo *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter(self);

    return format_from_lines((format_lines_func)SubjectPublicKeyInfo_format_lines, (PyObject *)self, args, kwds);
}

static PyObject *
SubjectPublicKeyInfo_str(SubjectPublicKeyInfo *self)
{
    PyObject *py_formatted_result = NULL;

    py_formatted_result =  SubjectPublicKeyInfo_format(self, empty_tuple, NULL);
    return py_formatted_result;

}

static PyMethodDef SubjectPublicKeyInfo_methods[] = {
    {"format_lines", (PyCFunction)SubjectPublicKeyInfo_format_lines,   METH_VARARGS|METH_KEYWORDS, generic_format_lines_doc},
    {"format",       (PyCFunction)SubjectPublicKeyInfo_format,         METH_VARARGS|METH_KEYWORDS, generic_format_doc},
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
SubjectPublicKeyInfo_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    SubjectPublicKeyInfo *self;

    TraceObjNewEnter(type);

    if ((self = (SubjectPublicKeyInfo *)type->tp_alloc(type, 0)) == NULL) return NULL;

    self->py_algorithm = NULL;
    self->py_public_key = NULL;

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static void
SubjectPublicKeyInfo_dealloc(SubjectPublicKeyInfo* self)
{
    TraceMethodEnter(self);

    Py_XDECREF(self->py_algorithm);
    Py_XDECREF(self->py_public_key);

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(SubjectPublicKeyInfo_doc,
"An object representing a Subject Public Key");

static int
SubjectPublicKeyInfo_init(SubjectPublicKeyInfo *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter(self);

    return 0;
}

static PyObject *
SubjectPublicKeyInfo_repr(SubjectPublicKeyInfo *self)
{
    return PyString_FromFormat("<%s object at %p>",
                               Py_TYPE(self)->tp_name, self);
}

static PyTypeObject SubjectPublicKeyInfoType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.SubjectPublicKeyInfo",			/* tp_name */
    sizeof(SubjectPublicKeyInfo),		/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)SubjectPublicKeyInfo_dealloc,	/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    0,						/* tp_compare */
    (reprfunc)SubjectPublicKeyInfo_repr,	/* tp_repr */
    0,						/* tp_as_number */
    0,						/* tp_as_sequence */
    0,						/* tp_as_mapping */
    0,						/* tp_hash */
    0,						/* tp_call */
    (reprfunc)SubjectPublicKeyInfo_str,		/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
    SubjectPublicKeyInfo_doc,			/* tp_doc */
    0,						/* tp_traverse */
    0,						/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    SubjectPublicKeyInfo_methods,		/* tp_methods */
    SubjectPublicKeyInfo_members,		/* tp_members */
    SubjectPublicKeyInfo_getseters,		/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)SubjectPublicKeyInfo_init,	/* tp_init */
    0,						/* tp_alloc */
    SubjectPublicKeyInfo_new,			/* tp_new */
};

PyObject *
SubjectPublicKeyInfo_new_from_CERTSubjectPublicKeyInfo(CERTSubjectPublicKeyInfo *spki)
{
    SubjectPublicKeyInfo *self = NULL;
    SECKEYPublicKey *pk = NULL;

    TraceObjNewEnter(NULL);

    if ((self = (SubjectPublicKeyInfo *) SubjectPublicKeyInfoType.tp_new(&SubjectPublicKeyInfoType, NULL, NULL)) == NULL)
        return NULL;

    if ((self->py_algorithm = SignatureAlgorithm_new_from_SECAlgorithmID(&spki->algorithm)) == NULL)
        return NULL;

    if ((pk = SECKEY_ExtractPublicKey(spki)) == NULL) {
        Py_DECREF(self->py_algorithm);
        return set_nspr_error(NULL);
    }

    if ((self->py_public_key = PublicKey_new_from_SECKEYPublicKey(pk)) == NULL) {
        Py_DECREF(self->py_algorithm);
        return NULL;
    }

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

/* ============================== Utilities ============================= */

CERTDistNames *
cert_distnames_as_CERTDistNames(PyObject *py_distnames)
{
    PRArenaPool *arena = NULL;
    CERTDistNames *names = NULL;
    int i;
    SecItem *py_sec_item;

    if (!PySequence_Check(py_distnames)) {
        PyErr_SetString(PyExc_TypeError, "cert distnames must be a sequence");
        return NULL;
    }

    /* allocate an arena to use */
    if ((arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE)) == NULL ) {
        PyErr_NoMemory();
        return NULL;
    }

    /* allocate the header structure */
    if ((names = (CERTDistNames *)PORT_ArenaAlloc(arena, sizeof(CERTDistNames))) == NULL) {
        PORT_FreeArena(arena, PR_FALSE);
        PyErr_NoMemory();
        return NULL;
    }

    /* initialize the header struct */
    names->arena = arena;
    names->head = NULL;
    names->nnames = PySequence_Size(py_distnames);
    names->names = NULL;

    /* construct the array from the list */
    if (names->nnames) {
	names->names = (SECItem *)PORT_ArenaAlloc(arena, names->nnames * sizeof(SECItem));

	if (names->names == NULL) {
            PORT_FreeArena(arena, PR_FALSE);
            PyErr_NoMemory();
            return NULL;
	}

	for (i = 0; i < names->nnames; i++) {
            py_sec_item = (SecItem *)PySequence_GetItem(py_distnames, i);
            if ((!PySecItem_Check(py_sec_item)) || (py_sec_item->kind != SECITEM_dist_name)) {
                PyErr_Format(PyExc_TypeError, "item must be a %s containing a DistName", SecItemType.tp_name);
                PORT_FreeArena(arena, PR_FALSE);
                return NULL;
            }
	    names->names[i] = py_sec_item->item;
	}
    }
    return names;
}

/* ========================================================================== */
/* =============================== CertDB Class ============================= */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static
PyGetSetDef CertDB_getseters[] = {
    {NULL}  /* Sentinel */
};

static PyMemberDef CertDB_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

static PyMethodDef CertDB_methods[] = {
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
CertDB_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    CertDB *self;

    TraceObjNewEnter(type);

    if ((self = (CertDB *)type->tp_alloc(type, 0)) == NULL) return NULL;
    self->handle = NULL;

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static void
CertDB_dealloc(CertDB* self)
{
    TraceMethodEnter(self);

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(CertDB_doc,
"An object representing a Certificate Database");

static int
CertDB_init(CertDB *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter(self);
    return 0;
}

static PyTypeObject CertDBType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.CertDB",				/* tp_name */
    sizeof(CertDB),				/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)CertDB_dealloc,			/* tp_dealloc */
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
    0,						/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
    CertDB_doc,					/* tp_doc */
    0,						/* tp_traverse */
    0,						/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    CertDB_methods,				/* tp_methods */
    CertDB_members,				/* tp_members */
    CertDB_getseters,				/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)CertDB_init,			/* tp_init */
    0,						/* tp_alloc */
    CertDB_new,					/* tp_new */
};

PyObject *
CertDB_new_from_CERTCertDBHandle(CERTCertDBHandle *cert_handle)
{
    CertDB *self = NULL;

    TraceObjNewEnter(NULL);
    if ((self = (CertDB *) CertDBType.tp_new(&CertDBType, NULL, NULL)) == NULL)
        return NULL;

    self->handle = cert_handle;

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

PyObject *
cert_distnames_new_from_CERTDistNames(CERTDistNames *names)
{
    PyObject *py_distnames = NULL;
    PyObject *py_sec_item = NULL;
    int i, len;

    len = names->nnames;
    if ((py_distnames = PyTuple_New(len)) == NULL)
        return NULL;

    for (i = 0; i< names->nnames; i++) {
        if ((py_sec_item = SecItem_new_from_SECItem(&names->names[i], SECITEM_dist_name)) == NULL) {
            return NULL;
        }
        PyTuple_SetItem(py_distnames, i, py_sec_item);
    }

    return py_distnames;
}

/* ========================================================================== */
/* ============================ Certificate Class =========================== */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static PyObject *
Certificate_get_valid_not_before(Certificate *self, void *closure)
{
    PRTime pr_time = 0;
    double d_time;

    pr_time = sec_time_choice(&self->cert->validity.notBefore);
    LL_L2D(d_time, pr_time);

    return PyFloat_FromDouble(d_time);
}

static PyObject *
Certificate_get_valid_not_before_str(Certificate *self, void *closure)
{
    return sec_time_choice_str(&self->cert->validity.notBefore);
}

static PyObject *
Certificate_get_valid_not_after(Certificate *self, void *closure)
{
    PRTime pr_time = 0;
    double d_time;

    pr_time = sec_time_choice(&self->cert->validity.notAfter);
    LL_L2D(d_time, pr_time);

    return PyFloat_FromDouble(d_time);
}

static PyObject *
Certificate_get_valid_not_after_str(Certificate *self, void *closure)
{
    return sec_time_choice_str(&self->cert->validity.notAfter);
}

static PyObject *
Certificate_get_subject(Certificate *self, void *closure)
{
    char *subject;
    PyObject *py_subject = NULL;

    subject = CERT_NameToAscii(&self->cert->subject);
    py_subject = PyString_FromString(subject);
    PORT_Free(subject);

    return py_subject;
}

static PyObject *
Certificate_get_subject_common_name(Certificate *self, void *closure)
{
    char *cn;
    PyObject *py_cn = NULL;

    cn = CERT_GetCommonName(&self->cert->subject);
    if (cn == NULL) {
	Py_INCREF(Py_None);
	return Py_None;
    }
    py_cn = PyString_FromString(cn);
    PORT_Free(cn);

    return py_cn;
}

static PyObject *
Certificate_get_issuer(Certificate *self, void *closure)
{
    char *issuer;
    PyObject *py_issuer = NULL;

    issuer = CERT_NameToAscii(&self->cert->issuer);
    py_issuer = PyString_FromString(issuer);
    PORT_Free(issuer);

    return py_issuer;
}

static PyObject *
Certificate_get_version(Certificate *self, void *closure)
{
    int version = 0;

    if (self->cert->version.len) {
        version = DER_GetInteger(&self->cert->version);
    }

    return PyInt_FromLong(version);
}

static PyObject *
Certificate_get_serial_number(Certificate *self, void *closure)
{
    int serial_number = 0;

    if (self->cert->serialNumber.len) {
        serial_number = DER_GetInteger(&self->cert->serialNumber);
    }

    return PyInt_FromLong(serial_number);
}

// FIXME: should this come from SignedData?
static PyObject *
Certificate_get_signature_algorithm(Certificate *self, void *closure)
{
    return SignatureAlgorithm_new_from_SECAlgorithmID(&self->cert->signature);
}

static PyObject *
Certificate_get_signed_data(Certificate *self, void *closure)
{
    PyObject *py_signed_data = NULL;

    return py_signed_data = SignedData_new_from_SECItem(&self->cert->derCert);
}

static PyObject *
Certificate_get_der_data(Certificate *self, void *closure)
{
    SECItem der;

    der = self->cert->derCert;
    return PyString_FromStringAndSize((char *)der.data, der.len);
}

static PyObject *
Certificate_get_ssl_trust_str(Certificate *self, void *closure)
{
    if (self->cert->trust)
        return cert_trust_flags_str(self->cert->trust->sslFlags);
    else
        Py_RETURN_NONE;
}

static PyObject *
Certificate_get_email_trust_str(Certificate *self, void *closure)
{
    if (self->cert->trust)
        return cert_trust_flags_str(self->cert->trust->emailFlags);
    else
        Py_RETURN_NONE;
}

static PyObject *
Certificate_get_signing_trust_str(Certificate *self, void *closure)
{
    if (self->cert->trust)
        return cert_trust_flags_str(self->cert->trust->objectSigningFlags);
    else
        Py_RETURN_NONE;
}

static PyObject *
Certificate_get_subject_public_key_info(Certificate *self, void *closure)
{
    Py_INCREF(self->py_subject_public_key_info);
    return self->py_subject_public_key_info;
}

static
PyGetSetDef Certificate_getseters[] = {
    {"valid_not_before",        (getter)Certificate_get_valid_not_before,        NULL,
     "certificate not valid before this time (floating point value expressed as microseconds since the epoch, midnight January 1st 1970 UTC)", NULL},

    {"valid_not_before_str",    (getter)Certificate_get_valid_not_before_str,    NULL,
     "certificate not valid before this time (string value expressed, UTC)", NULL},

    {"valid_not_after",         (getter)Certificate_get_valid_not_after,         NULL,
     "certificate not valid after this time (floating point value expressed as microseconds since the epoch, midnight January 1st 1970, UTC)", NULL},

    {"valid_not_after_str",     (getter)Certificate_get_valid_not_after_str,     NULL,
     "certificate not valid after this time (string value expressed, UTC)", NULL},

    {"subject",                 (getter)Certificate_get_subject,                 NULL,
     "certificate subject", NULL},

    {"subject_common_name",     (getter)Certificate_get_subject_common_name,     NULL,
     "certificate subject", NULL},

    {"issuer",                  (getter)Certificate_get_issuer,                  NULL,
     "certificate issuer",  NULL},

    {"version",                 (getter)Certificate_get_version,                 NULL,
     "certificate version",  NULL},

    {"serial_number",           (getter)Certificate_get_serial_number,           NULL,
     "certificate serial number",  NULL},

    {"signature_algorithm",     (getter)Certificate_get_signature_algorithm,     NULL,
     "certificate signature algorithm",  NULL},

    {"signed_data",             (getter)Certificate_get_signed_data,             NULL,
     "certificate signature as SignedData object",  NULL},

    {"der_data",                (getter)Certificate_get_der_data,                NULL,
     "raw certificate DER data as data buffer",  NULL},

    {"ssl_trust_str",           (getter)Certificate_get_ssl_trust_str,           NULL,
     "certificate SSL trust flags as array of strings, or None if trust is not defined",  NULL},

    {"email_trust_str",         (getter)Certificate_get_email_trust_str,         NULL,
     "certificate email trust flags as array of strings, or None if trust is not defined",  NULL},

    {"signing_trust_str",       (getter)Certificate_get_signing_trust_str,       NULL,
     "certificate object signing trust flags as array of strings, or None if trust is not defined",  NULL},

    {"subject_public_key_info", (getter)Certificate_get_subject_public_key_info, NULL, "certificate public info as SubjectPublicKeyInfo object",  NULL},
    {NULL}  /* Sentinel */
};

static PyMemberDef Certificate_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

PyDoc_STRVAR(Certificate_find_kea_type_doc,
"find_kea_type() -> kea_type\n\
Returns key exchange type of the keys in an SSL server certificate.\n\
\n\
May be one of the following:\n\
    - ssl_kea_null\n\
    - ssl_kea_rsa\n\
    - ssl_kea_dh\n\
    - ssl_kea_fortezza (deprecated)\n\
    - ssl_kea_ecdh\n\
"
);
static PyObject *
Certificate_find_kea_type(Certificate *self, PyObject *args)
{
    TraceMethodEnter(self);

    return PyInt_FromLong(NSS_FindCertKEAType(self->cert));
}


PyDoc_STRVAR(Certificate_verify_hostname_doc,
"verify_hostname(hostname) -> bool\n\
\n\
A restricted regular expression syntax is used to test if the common\n\
name specified in the subject DN of the certificate is a match,\n\
returning True if so, False otherwise.\n\
\n\
The regular expression systax is:\n\
    \\*\n\
        matches anything\n\
    \\?\n\
        matches one character\n\
    \\\\ (backslash)\n\
        escapes a special character\n\
    \\$\n\
         matches the end of the string\n\
    [abc]\n\
        matches one occurrence of a, b, or c. The only character\n\
        that needs to be escaped in this is ], all others are not special.\n\
    [a-z]\n\
        matches any character between a and z\n\
    [^az]\n\
        matches any character except a or z\n\
    \\~\n\
        followed by another shell expression removes any pattern matching\n\
        the shell expression from the match list\n\
    (foo|bar)\n\
        matches either the substring foo or the substring bar.\n\
        These can be shell expressions as well.\n\
");

static PyObject *
Certificate_verify_hostname(Certificate *self, PyObject *args)
{
    char *hostname;
    SECStatus sec_status;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "s:verify_hostname", &hostname))
        return NULL;

    sec_status = CERT_VerifyCertName(self->cert, hostname);

    if (sec_status == SECSuccess)
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;;
}

PyDoc_STRVAR(Certificate_has_signer_in_ca_names_doc,
"has_signer_in_ca_names(ca_names) -> bool\n\
\n\
:Parameters:\n\
    ca_names : (SecItem, ...)\n\
        Sequence of CA distinguished names. Each item in the sequence must\n\
        be a SecItem object containing a distinguished name.\n\
\n\
Returns True if any of the signers in the certificate chain for a\n\
specified certificate are in the list of CA names, False\n\
otherwise.\n\
");

static PyObject *
Certificate_has_signer_in_ca_names(Certificate *self, PyObject *args)
{
    PyObject *py_ca_names = NULL;
    CERTDistNames *ca_names = NULL;
    SECStatus sec_status;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "s:has_signer_in_ca_names", py_ca_names))
        return NULL;

    if ((ca_names = cert_distnames_as_CERTDistNames(py_ca_names)) == NULL) {
    }

    sec_status = NSS_CmpCertChainWCANames(self->cert, ca_names);
    CERT_FreeDistNames(ca_names);

    if (sec_status == SECSuccess)
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;;
}

PyDoc_STRVAR(Certificate_check_valid_times_doc,
"check_valid_times(time=now, allow_override=False) --> validity\n\
\n\
:Parameters:\n\
    time : number\n\
        an optional point in time as number of microseconds\n\
        since the NSPR epoch, midnight (00:00:00) 1 January\n\
        1970 UTC, either as an integer or a float. If time \n\
        is not specified the current time is used.\n\
    allow_override : bool\n\
        If True then check to see if the invalidity has\n\
        been overridden by the user, defaults to False.\n\
\n\
Checks whether a specified time is within a certificate's validity\n\
period.\n\
\n\
Returns one of:\n\
\n\
- secCertTimeValid\n\
- secCertTimeExpired\n\
- secCertTimeNotValidYet\n\
");

static PyObject *
Certificate_check_valid_times(Certificate *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"time", NULL};
    PyObject *py_time = NULL;
    int allow_override = 0;
    PRTime time;
    SECCertTimeValidity validity;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|Oi:check_valid_times", kwlist, &py_time, &allow_override))
        return NULL;

    if (py_time) {
        if (PyFloat_Check(py_time)) {
            LL_D2L(time, PyFloat_AsDouble(py_time));
        } else if (PyInt_Check(py_time)) {
            LL_I2L(time, PyInt_AsLong(py_time)); /* FIXME: should be PyLong_AsLongLong? */
        } else {
            PyErr_SetString(PyExc_TypeError, "check_valid_times: time must be a float or an integer");
            return NULL;
        }
    } else {
        time = PR_Now();
    }

    validity = CERT_CheckCertValidTimes(self->cert, time, allow_override);

    return PyInt_FromLong(validity);
}

PyDoc_STRVAR(Certificate_verify_now_doc,
"verify_now(certdb, check_sig, required_usages) -> valid_usages\n\
\n\
:Parameters:\n\
    certdb : CertDB object\n\
        CertDB certificate database object\n\
    check_sig : bool\n\
        True if certificate signatures should be checked\n\
    required_usages : integer\n\
        A bitfield of all cert usages that are required for verification\n\
        to succeed. If zero return all possible valid usages.\n\
\n\
Verify a certificate by checking if it's valid and that we\n\
trust the issuer.\n\
\n\
Possible usage bitfield values are:\n\
    - certificateUsageCheckAllUsages\n\
    - certificateUsageSSLClient\n\
    - certificateUsageSSLServer\n\
    - certificateUsageSSLServerWithStepUp\n\
    - certificateUsageSSLCA\n\
    - certificateUsageEmailSigner\n\
    - certificateUsageEmailRecipient\n\
    - certificateUsageObjectSigner\n\
    - certificateUsageUserCertImport\n\
    - certificateUsageVerifyCA\n\
    - certificateUsageProtectedObjectSigner\n\
    - certificateUsageStatusResponder\n\
    - certificateUsageAnyCA\n\
\n\
Returns valid_usages, a bitfield of certificate usages.  If\n\
required_usages is non-zero, the returned bitmap is only for those\n\
required usages, otherwise it is for all possible usages.\n\
");

static PyObject *
Certificate_verify_now(Certificate *self, PyObject *args)
{
    Py_ssize_t n_base_args = 3;
    Py_ssize_t argc;
    PyObject *parse_args = NULL;
    PyObject *pin_args = NULL;
    CertDB *py_certdb = NULL;
    PyObject *py_check_sig = NULL;
    int check_sig = 0;
    long required_usages = 0;
    SECCertificateUsage returned_usages;

    TraceMethodEnter(self);

    argc = PyTuple_Size(args);
    if (argc == n_base_args) {
        parse_args = args;
    } else {
        parse_args = PyTuple_GetSlice(args, 0, n_base_args);
    }
    if (!PyArg_ParseTuple(parse_args, "O!O!l:verify_now",
                          &CertDBType, &py_certdb,
                          &PyBool_Type, &py_check_sig,
                          &required_usages)) {
        if (parse_args != args) {
            Py_DECREF(parse_args);
        }
        return NULL;
    }
    if (parse_args != args) {
        Py_DECREF(parse_args);
    }

    check_sig = PyInt_AsLong(py_check_sig);
    pin_args = PyTuple_GetSlice(args, n_base_args, argc);
    Py_INCREF(pin_args);

    if (CERT_VerifyCertificateNow(py_certdb->handle, self->cert, check_sig,
                                  required_usages, pin_args, &returned_usages) != SECSuccess) {
        Py_DECREF(pin_args);
        return set_nspr_error(NULL);
    }
    Py_DECREF(pin_args);

    return PyInt_FromLong(returned_usages);
}


static PyObject *
Certificate_format_lines(Certificate *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"level", NULL};
    int level = 0;
    PyObject *lines = NULL;
    PyObject *obj = NULL;
    PyObject *obj_lines = NULL;
    PyObject *ssl_trust_lines = NULL, *email_trust_lines = NULL, *signing_trust_lines = NULL;
    int i;
    Py_ssize_t len;
    PyObject *tmp_args = NULL;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:format_lines", kwlist, &level))
        return NULL;

    if ((lines = PyList_New(0)) == NULL)
        goto fail;

    //FMT_LABEL_AND_APPEND(level, "Certificate", lines, fail);
    FMT_LABEL_AND_APPEND(level+1, "Data", lines, fail);

    if ((obj = Certificate_get_version(self, NULL)) == NULL)
        goto fail;
    FMT_OBJ_AND_APPEND(level+2, "Version", obj, lines, fail);
    Py_DECREF(obj);

    if ((obj = Certificate_get_serial_number(self, NULL)) == NULL)
        goto fail;
    FMT_OBJ_AND_APPEND(level+2, "Serial Number", obj, lines, fail);
    Py_DECREF(obj);

    if ((obj = Certificate_get_signature_algorithm(self, NULL)) == NULL)
        goto fail;
    FMT_OBJ_AND_APPEND(level+2, "Signature Algorithm", obj, lines, fail);
    Py_DECREF(obj);

    if ((obj = Certificate_get_issuer(self, NULL)) == NULL)
        goto fail;
    FMT_OBJ_AND_APPEND(level+2, "Issuer", obj, lines, fail);
    Py_DECREF(obj);

    FMT_LABEL_AND_APPEND(level+2, "Validity", lines, fail);

    if ((obj = Certificate_get_valid_not_before_str(self, NULL)) == NULL)
        goto fail;
    FMT_OBJ_AND_APPEND(level+3, "Not Before", obj, lines, fail);
    Py_DECREF(obj);

    if ((obj = Certificate_get_valid_not_after_str(self, NULL)) == NULL)
        goto fail;
    FMT_OBJ_AND_APPEND(level+3, "Not After ", obj, lines, fail);
    Py_DECREF(obj);

    if ((obj = Certificate_get_subject(self, NULL)) == NULL)
        goto fail;
    FMT_OBJ_AND_APPEND(level+2, "Subject", obj, lines, fail);
    Py_DECREF(obj);

    FMT_LABEL_AND_APPEND(level+2, "Subject Public Key Info", lines, fail);

    if ((obj = Certificate_get_subject_public_key_info(self, NULL)) == NULL)
        goto fail;
    if ((tmp_args = Py_BuildValue("(i)", level+3)) == NULL)
        goto fail;
    if ((obj_lines = SubjectPublicKeyInfo_format_lines((SubjectPublicKeyInfo *)obj, tmp_args, NULL)) == NULL)
        goto fail;
    Py_DECREF(obj);
    Py_DECREF(tmp_args);
    len = PyList_Size(obj_lines);
    for (i = 0; i < len; i++) {
        obj = PyList_GET_ITEM(obj_lines, i);
        PyList_Append(lines, obj);
    }
    Py_DECREF(obj_lines);

    if ((ssl_trust_lines = Certificate_get_ssl_trust_str(self, NULL)) == NULL)
        goto fail;
    if ((email_trust_lines = Certificate_get_email_trust_str(self, NULL)) == NULL)
        goto fail;
    if ((signing_trust_lines = Certificate_get_signing_trust_str(self, NULL)) == NULL)
        goto fail;

    if ((ssl_trust_lines != Py_None) || (email_trust_lines != Py_None) || (signing_trust_lines != Py_None)) {
        FMT_LABEL_AND_APPEND(level+2, "Certificate Trust Flags", lines, fail);

        if (PyList_Check(ssl_trust_lines)) {
            FMT_LABEL_AND_APPEND(level+3, "SSL Flags", lines, fail);
            len = PyList_Size(ssl_trust_lines);
            for (i = 0; i < len; i++) {
                obj = PyList_GET_ITEM(ssl_trust_lines, i);
                FMT_OBJ_AND_APPEND(level+4, NULL, obj, lines, fail);
            }
        }

        if (PyList_Check(email_trust_lines)) {
            FMT_LABEL_AND_APPEND(level+3, "Email Flags", lines, fail);
            len = PyList_Size(email_trust_lines);
            for (i = 0; i < len; i++) {
                obj = PyList_GET_ITEM(email_trust_lines, i);
                FMT_OBJ_AND_APPEND(level+4, NULL, obj, lines, fail);
            }
        }

        if (PyList_Check(signing_trust_lines)) {
            FMT_LABEL_AND_APPEND(level+3, "Object Signing Flags", lines, fail);
            len = PyList_Size(signing_trust_lines);
            for (i = 0; i < len; i++) {
                obj = PyList_GET_ITEM(signing_trust_lines, i);
                FMT_OBJ_AND_APPEND(level+4, NULL, obj, lines, fail);
            }
        }
    }
    Py_DECREF(ssl_trust_lines);
    Py_DECREF(email_trust_lines);
    Py_DECREF(signing_trust_lines);

    FMT_LABEL_AND_APPEND(level+2, "Fingerprint (MD5)", lines, fail);

    if ((obj = Certificate_get_der_data(self, NULL)) == NULL)
        goto fail;

    if ((tmp_args = Py_BuildValue("(O)", obj)) == NULL)
        goto fail;
    Py_DECREF(obj);
    if ((obj = pk11_md5_digest(NULL, tmp_args)) == NULL)
        goto fail;
    Py_DECREF(tmp_args);

    if ((tmp_args = Py_BuildValue("(O)", obj)) == NULL)
        goto fail;
    Py_DECREF(obj);
    if ((obj_lines = cert_data_to_hex(NULL, tmp_args, NULL)) == NULL)
        goto fail;
    Py_DECREF(tmp_args);

    len = PyList_Size(obj_lines);
    for (i = 0; i < len; i++) {
        obj = PyList_GET_ITEM(obj_lines, i);
        FMT_OBJ_AND_APPEND(level+3, NULL, obj, lines, fail);
    }
    Py_DECREF(obj_lines);

    FMT_LABEL_AND_APPEND(level+2, "Fingerprint (SHA1)", lines, fail);

    if ((obj = Certificate_get_der_data(self, NULL)) == NULL)
        goto fail;

    if ((tmp_args = Py_BuildValue("(O)", obj)) == NULL)
        goto fail;
    Py_DECREF(obj);
    if ((obj = pk11_sha1_digest(NULL, tmp_args)) == NULL)
        goto fail;
    Py_DECREF(tmp_args);

    if ((tmp_args = Py_BuildValue("(O)", obj)) == NULL)
        goto fail;
    Py_DECREF(obj);
    if ((obj_lines = cert_data_to_hex(NULL, tmp_args, NULL)) == NULL)
        goto fail;
    Py_DECREF(tmp_args);

    len = PyList_Size(obj_lines);
    for (i = 0; i < len; i++) {
        obj = PyList_GET_ITEM(obj_lines, i);
        FMT_OBJ_AND_APPEND(level+3, NULL, obj, lines, fail);
    }
    Py_DECREF(obj_lines);

    FMT_LABEL_AND_APPEND(level+1, "Signature", lines, fail);

    if ((obj = Certificate_get_signed_data(self, NULL)) == NULL)
        goto fail;

    if ((tmp_args = Py_BuildValue("(i)", level+2)) == NULL)
        goto fail;
    if ((obj_lines = SignedData_format_lines((SignedData *)obj, tmp_args, NULL)) == NULL)
        goto fail;
    Py_DECREF(obj);
    Py_DECREF(tmp_args);
    len = PyList_Size(obj_lines);
    for (i = 0; i < len; i++) {
        obj = PyList_GET_ITEM(obj_lines, i);
        PyList_Append(lines, obj);
    }
    Py_DECREF(obj_lines);

    return lines;
 fail:
    Py_XDECREF(obj);
    Py_XDECREF(lines);
    Py_XDECREF(obj_lines);
    Py_XDECREF(tmp_args);
    Py_XDECREF(ssl_trust_lines);
    Py_XDECREF(email_trust_lines);
    Py_XDECREF(signing_trust_lines);
    return NULL;
}

static PyObject *
Certificate_format(Certificate *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter(self);

    return format_from_lines((format_lines_func)Certificate_format_lines, (PyObject *)self, args, kwds);
}

static PyObject *
Certificate_str(Certificate *self)
{
    PyObject *py_formatted_result = NULL;

    py_formatted_result = Certificate_format(self, empty_tuple, NULL);
    return py_formatted_result;

}

static PyMethodDef Certificate_methods[] = {
    {"find_kea_type",          (PyCFunction)Certificate_find_kea_type,          METH_NOARGS,                Certificate_find_kea_type_doc},
    {"has_signer_in_ca_names", (PyCFunction)Certificate_has_signer_in_ca_names, METH_VARARGS,               Certificate_has_signer_in_ca_names_doc},
    {"verify_hostname",        (PyCFunction)Certificate_verify_hostname,        METH_VARARGS,               Certificate_verify_hostname_doc},
    {"check_valid_times",      (PyCFunction)Certificate_check_valid_times,      METH_VARARGS,               Certificate_check_valid_times_doc},
    {"verify_now",             (PyCFunction)Certificate_verify_now,             METH_VARARGS,               Certificate_verify_now_doc},
    {"format_lines",           (PyCFunction)Certificate_format_lines,           METH_VARARGS|METH_KEYWORDS, generic_format_lines_doc},
    {"format",                 (PyCFunction)Certificate_format,                 METH_VARARGS|METH_KEYWORDS, generic_format_doc},
    //    {"", (PyCFunction)Certificate_, METH_RARGS, Certificate__doc},
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
Certificate_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Certificate *self;

    TraceObjNewEnter(type);

    if ((self = (Certificate *)type->tp_alloc(type, 0)) == NULL) return NULL;
    self->cert = NULL;
    self->py_subject_public_key_info = NULL;

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static void
Certificate_dealloc(Certificate* self)
{
    TraceMethodEnter(self);

    if (self->cert)
        CERT_DestroyCertificate(self->cert);

    Py_XDECREF(self->py_subject_public_key_info);

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(Certificate_doc,
"An object representing a Certificate");

static int
Certificate_init(Certificate *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter(self);

    return 0;
}

static PyObject *
Certificate_repr(Certificate *self)
{
    return PyString_FromFormat("<%s object at %p Certificate %p>",
                               Py_TYPE(self)->tp_name, self, self->cert);
}

static PyTypeObject CertificateType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.Certificate",				/* tp_name */
    sizeof(Certificate),			/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)Certificate_dealloc,		/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    0,						/* tp_compare */
    (reprfunc)Certificate_repr,			/* tp_repr */
    0,						/* tp_as_number */
    0,						/* tp_as_sequence */
    0,						/* tp_as_mapping */
    0,						/* tp_hash */
    0,						/* tp_call */
    (reprfunc)Certificate_str,			/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
    Certificate_doc,				/* tp_doc */
    0,						/* tp_traverse */
    0,						/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    Certificate_methods,			/* tp_methods */
    Certificate_members,			/* tp_members */
    Certificate_getseters,			/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)Certificate_init,			/* tp_init */
    0,						/* tp_alloc */
    Certificate_new,				/* tp_new */
};

PyObject *
Certificate_new_from_CERTCertificate(CERTCertificate *cert)
{
    Certificate *self = NULL;

    TraceObjNewEnter(NULL);

    if ((self = (Certificate *) CertificateType.tp_new(&CertificateType, NULL, NULL)) == NULL)
        return NULL;

    self->cert = cert;
    self->py_subject_public_key_info = SubjectPublicKeyInfo_new_from_CERTSubjectPublicKeyInfo(&cert->subjectPublicKeyInfo);

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

/* ========================================================================== */
/* ============================= PrivateKey Class =========================== */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static
PyGetSetDef PrivateKey_getseters[] = {
    {NULL}  /* Sentinel */
};

static PyMemberDef PrivateKey_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */


static PyMethodDef PrivateKey_methods[] = {
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
PrivateKey_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PrivateKey *self;

    TraceObjNewEnter(type);

    if ((self = (PrivateKey *)type->tp_alloc(type, 0)) == NULL) return NULL;
    self->private_key = NULL;

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static void
PrivateKey_dealloc(PrivateKey* self)
{
    TraceMethodEnter(self);

    if (self->private_key)
        SECKEY_DestroyPrivateKey(self->private_key);

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(PrivateKey_doc,
"An object representing a Private Key");

static int
PrivateKey_init(PrivateKey *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter(self);
    return 0;
}

static PyTypeObject PrivateKeyType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.PrivateKey",				/* tp_name */
    sizeof(PrivateKey),				/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)PrivateKey_dealloc,		/* tp_dealloc */
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
    0,						/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
    PrivateKey_doc,				/* tp_doc */
    0,						/* tp_traverse */
    0,						/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    PrivateKey_methods,				/* tp_methods */
    PrivateKey_members,				/* tp_members */
    PrivateKey_getseters,			/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)PrivateKey_init,			/* tp_init */
    0,						/* tp_alloc */
    PrivateKey_new,				/* tp_new */
};

PyObject *
PrivateKey_new_from_SECKEYPrivateKey(SECKEYPrivateKey *private_key)
{
    PrivateKey *self = NULL;

    TraceObjNewEnter(NULL);
    if ((self = (PrivateKey *) PrivateKeyType.tp_new(&PrivateKeyType, NULL, NULL)) == NULL)
        return NULL;

    self->private_key = private_key;

    TraceObjNewLeave(self);
    return (PyObject *) self;
}


/* ============================== Module Methods ============================= */

PyDoc_STRVAR(cert_get_default_certdb_doc,
"get_default_certdb()\n\
\n\
Returns the default certificate database as a CertDB object\n\
");
static PyObject *
cert_get_default_certdb(PyObject *self, PyObject *args)
{
    CERTCertDBHandle *cert_handle;

    TraceMethodEnter(self);

    if ((cert_handle = CERT_GetDefaultCertDB()) == NULL)
        Py_RETURN_NONE;

    return CertDB_new_from_CERTCertDBHandle(cert_handle);
}

PyDoc_STRVAR(cert_get_cert_nicknames_doc,
"get_cert_nicknames(certdb, what, [user_data1, ...]) -> name0, ...\n\
\n\
:Parameters:\n\
    certdb : CertDB object\n\
        CertDB certificate database object\n\
    what : integer\n\
        one of:\n\
            - SEC_CERT_NICKNAMES_ALL\n\
            - SEC_CERT_NICKNAMES_USER\n\
            - SEC_CERT_NICKNAMES_SERVER\n\
            - SEC_CERT_NICKNAMES_CA\n\
    user_dataN : object\n\
        zero or more caller supplied parameters which will\n\
        be passed to the password callback function\n\
\n\
Returns a tuple of the nicknames of the certificates in a specified\n\
certificate database.\n\
");

static PyObject *
cert_get_cert_nicknames(PyObject *self, PyObject *args)
{
    Py_ssize_t n_base_args = 2;
    Py_ssize_t argc;
    PyObject *parse_args = NULL;
    PyObject *pin_args = NULL;
    int what;
    CertDB *py_certdb = NULL;
    CERTCertNicknames *cert_nicknames = NULL;
    PyObject *py_nicknames = NULL;
    PyObject *py_nickname = NULL;
    int i, len;

    TraceMethodEnter(self);

    argc = PyTuple_Size(args);
    if (argc == n_base_args) {
        parse_args = args;
    } else {
        parse_args = PyTuple_GetSlice(args, 0, n_base_args);
    }
    if (!PyArg_ParseTuple(parse_args, "O!i:get_cert_nicknames",
                          &CertDBType, &py_certdb, &what)) {
        if (parse_args != args) {
            Py_DECREF(parse_args);
        }
        return NULL;
    }
    if (parse_args != args) {
        Py_DECREF(parse_args);
    }

    pin_args = PyTuple_GetSlice(args, n_base_args, argc);
    Py_INCREF(pin_args);

    if ((cert_nicknames = CERT_GetCertNicknames(py_certdb->handle, what, pin_args)) == NULL) {
        Py_DECREF(pin_args);
        return set_nspr_error(NULL);
    }

    Py_DECREF(pin_args);

    len = cert_nicknames->numnicknames;
    if ((py_nicknames = PyTuple_New(len)) == NULL) {
        CERT_FreeNicknames(cert_nicknames);
        return NULL;
    }

    for (i = 0; i < len; i++) {
        if ((py_nickname = PyString_FromString(cert_nicknames->nicknames[i])) == NULL) {
            CERT_FreeNicknames(cert_nicknames);
            return NULL;
        }
        PyTuple_SetItem(py_nicknames, i, py_nickname);
    }

    CERT_FreeNicknames(cert_nicknames);

    return py_nicknames;
}

PyDoc_STRVAR(pk11_hash_buf_doc,
"hash_buf(hash_alg, data) --> digest\n\
\n\
:Parameters:\n\
    hash_alg : int\n\
        hash algorithm enumeration (SEC_OID_*)\n\
        e.g.: SEC_OID_MD5, SEC_OID_SHA1, SEC_OID_SHA256, SEC_OID_SHA512, etc.\n\
    data : buffer or string\n\
        buffer the digest will be computed for\n\
\n\
Computes a digest according to the hash_alg type.\n\
Return the digest data as buffer object.\n\
\n\
Note, if a hexidecimal string representation is desired then pass\n\
result to data_to_hex()\n\
");
static PyObject *
pk11_hash_buf(PyObject *self, PyObject *args)
{
    unsigned long hash_alg;
    unsigned char *in_data = NULL;
    Py_ssize_t in_data_len = 0;
    unsigned int hash_len;
    PyObject *py_out_buf = NULL;
    void *out_buf = NULL;
    Py_ssize_t out_buf_len;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "kt#:hash_buf",
                          &hash_alg, &in_data, &in_data_len)) {
        return NULL;
    }

    if ((hash_len = HASH_ResultLenByOidTag(hash_alg)) == 0) {
        return set_nspr_error("unable to determine resulting hash length for hash_alg = %s",
                              sec_oid_tag_str(hash_alg));
    }

    out_buf_len = hash_len;

    if ((py_out_buf = PyString_FromStringAndSize(NULL, out_buf_len)) == NULL) {
        return NULL;
    }

    if ((out_buf = PyString_AsString(py_out_buf)) == NULL) {
        return NULL;
    }

    if (PK11_HashBuf(hash_alg, out_buf, in_data, in_data_len) != SECSuccess)
        return set_nspr_error(NULL);

    return py_out_buf;
}

PyDoc_STRVAR(pk11_md5_digest_doc,
"md5_digest(data) --> digest\n\
\n\
:Parameters:\n\
    data : buffer or string\n\
        buffer the digest will be computed for\n\
\n\
Returns 16 octet MD5 digest data as buffer object.\n\
\n\
Note, if a hexidecimal string representation is desired then pass\n\
result to data_to_hex()\n\
");
static PyObject *
pk11_md5_digest(PyObject *self, PyObject *args)
{
    unsigned char *in_data = NULL;
    Py_ssize_t in_data_len = 0;
    PyObject *py_out_buf = NULL;
    void *out_buf;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "t#:md5_digest", &in_data, &in_data_len)) {
        return NULL;
    }

    if ((py_out_buf = PyString_FromStringAndSize(NULL, MD5_LENGTH)) == NULL) {
        return NULL;
    }

    if ((out_buf = PyString_AsString(py_out_buf)) == NULL) {
        return NULL;
    }

    if (PK11_HashBuf(SEC_OID_MD5, out_buf, in_data, in_data_len) != SECSuccess) {
        return set_nspr_error(NULL);
    }

    return py_out_buf;
}

PyDoc_STRVAR(pk11_sha1_digest_doc,
"sha1_digest(data) --> digest\n\
\n\
:Parameters:\n\
    data : buffer or string\n\
        buffer the digest will be computed for\n\
\n\
Returns 20 octet SHA1 digest data as buffer object.\n\
\n\
Note, if a hexidecimal string representation is desired then pass\n\
result to data_to_hex()\n\
");
static PyObject *
pk11_sha1_digest(PyObject *self, PyObject *args)
{
    unsigned char *in_data = NULL;
    Py_ssize_t in_data_len = 0;
    PyObject *py_out_buf = NULL;
    void *out_buf;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "t#:sha1_digest", &in_data, &in_data_len)) {
        return NULL;
    }

    if ((py_out_buf = PyString_FromStringAndSize(NULL, SHA1_LENGTH)) == NULL) {
        return NULL;
    }

    if ((out_buf = PyString_AsString(py_out_buf)) == NULL) {
        return NULL;
    }

    if (PK11_HashBuf(SEC_OID_SHA1, out_buf, in_data, in_data_len) != SECSuccess) {
        return set_nspr_error(NULL);
    }

    return py_out_buf;
}

PyDoc_STRVAR(pk11_sha256_digest_doc,
"sha256_digest(data) --> digest\n\
\n\
:Parameters:\n\
    data : buffer or string\n\
        buffer the digest will be computed for\n\
\n\
Returns 32 octet SHA256 digest data as buffer object.\n\
\n\
Note, if a hexidecimal string representation is desired then pass\n\
result to data_to_hex()\n\
");

static PyObject *
pk11_sha256_digest(PyObject *self, PyObject *args)
{
    unsigned char *in_data = NULL;
    Py_ssize_t in_data_len = 0;
    PyObject *py_out_buf = NULL;
    void *out_buf;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "t#:sha256_digest", &in_data, &in_data_len)) {
        return NULL;
    }

    if ((py_out_buf = PyString_FromStringAndSize(NULL, SHA256_LENGTH)) == NULL) {
        return NULL;
    }

    if ((out_buf = PyString_AsString(py_out_buf)) == NULL) {
        return NULL;
    }

    if (PK11_HashBuf(SEC_OID_SHA256, out_buf, in_data, in_data_len) != SECSuccess) {
        return set_nspr_error(NULL);
    }

    return py_out_buf;
}

PyDoc_STRVAR(pk11_sha512_digest_doc,
"sha512_digest(data) --> digest\n\
\n\
:Parameters:\n\
    data : buffer or string\n\
        buffer the digest will be computed for\n\
\n\
Returns 64 octet SHA512 digest data as buffer object.\n\
\n\
Note, if a hexidecimal string representation is desired then pass\n\
result to data_to_hex()\n\
");
static PyObject *
pk11_sha512_digest(PyObject *self, PyObject *args)
{
    unsigned char *in_data = NULL;
    Py_ssize_t in_data_len = 0;
    PyObject *py_out_buf = NULL;
    void *out_buf;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "t#:sha512_digest", &in_data, &in_data_len)) {
        return NULL;
    }

    if ((py_out_buf = PyString_FromStringAndSize(NULL, SHA512_LENGTH)) == NULL) {
        return NULL;
    }

    if ((out_buf = PyString_AsString(py_out_buf)) == NULL) {
        return NULL;
    }

    if (PK11_HashBuf(SEC_OID_SHA512, out_buf, in_data, in_data_len) != SECSuccess) {
        return set_nspr_error(NULL);
    }

    return py_out_buf;
}

/* ========================================================================== */
/* ============================== PK11Slot Class ============================ */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static PyObject *
PK11_get_slot_name(PK11Slot *self, void *closure)
{
    char *slot_name = NULL;

    if ((slot_name = PK11_GetSlotName(self->slot)) == NULL)
        Py_RETURN_NONE;

    return PyString_FromString(slot_name);
}

static PyObject *
PK11_get_token_name(PK11Slot *self, void *closure)
{
    char *token_name = NULL;

    if ((token_name = PK11_GetTokenName(self->slot)) == NULL)
        Py_RETURN_NONE;

    return PyString_FromString(token_name);
}

static
PyGetSetDef PK11Slot_getseters[] = {
    {"slot_name",  (getter)PK11_get_slot_name,  (setter)NULL, "slot name", NULL},
    {"token_name", (getter)PK11_get_token_name, (setter)NULL, "token name", NULL},
    {NULL}  /* Sentinel */
};

static PyMemberDef PK11Slot_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

PyDoc_STRVAR(PK11Slot_is_hw_doc,
"is_hw() -> bool\n\
\n\
Finds out whether a slot is implemented in hardware or software.\n\
");
static PyObject *
PK11Slot_is_hw(PK11Slot *self, PyObject *args)
{
    TraceMethodEnter(self);

    if (PK11_IsHW(self->slot))
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;

}

PyDoc_STRVAR(PK11Slot_is_present_doc,
"is_present() -> bool\n\
\n\
Finds out whether the token for a slot is available.\n\
");
static PyObject *
PK11Slot_is_present(PK11Slot *self, PyObject *args)
{
    TraceMethodEnter(self);

    if (PK11_IsPresent(self->slot))
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;

}

PyDoc_STRVAR(PK11Slot_is_read_only_doc,
"is_read_only() -> bool\n\
\n\
Finds out whether a slot is read-only.\n\
");
static PyObject *
PK11Slot_is_read_only(PK11Slot *self, PyObject *args)
{
    TraceMethodEnter(self);

    if (PK11_IsReadOnly(self->slot))
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;

}

PyDoc_STRVAR(PK11Slot_get_best_wrap_mechanism_doc,
"get_best_wrap_mechanism() -> mechanism\n\
\n\
Find the best key wrap mechanism for this slot.\n\
");
static PyObject *
PK11Slot_get_best_wrap_mechanism(PK11Slot *self, PyObject *args)
{
    CK_MECHANISM_TYPE mechanism;

    TraceMethodEnter(self);

    mechanism = PK11_GetBestWrapMechanism(self->slot);
    return PyInt_FromLong(mechanism);
}


PyDoc_STRVAR(PK11Slot_get_best_key_length_doc,
"get_best_key_length(mechanism) -> length\n\
\n\
:Parameters:\n\
    mechanism : int\n\
        key mechanism enumeration constant (CKM_*)\n\
\n\
Return the best key length for this slot and mechanism.\n\
A zero result means that token knows how long the key should be,\n\
the result is typically used with key_gen(), token_key_gen(), or\n\
token_key_gen_with_flags()\n\
");
static PyObject *
PK11Slot_get_best_key_length(PK11Slot *self, PyObject *args)
{
    unsigned long mechanism;
    int length;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "k:get_best_key_length", &mechanism))
        return NULL;

    length = PK11_GetBestKeyLength(self->slot, mechanism);
    return PyInt_FromLong(length);
}

PyDoc_STRVAR(PK11Slot_key_gen_doc,
"key_gen(mechanism, sec_param, key_size, [user_data1, ...]) -> PK11SymKey object\n\
\n\
:Parameters:\n\
    mechanism : int\n\
        key mechanism enumeration constant (CKM_*)\n\
    key_param : SecItem object or None\n\
        SecItem key parameters. None is also valid.\n\
    key_size : int\n\
        key length (use get_best_key_length())\n\
    user_dataN : object ...\n\
        zero or more caller supplied parameters which will\n\
        be passed to the password callback function\n\
\n\
Generate a symmetric key.\n\
");
static PyObject *
PK11Slot_key_gen(PK11Slot *self, PyObject *args)
{
    Py_ssize_t n_base_args = 3;
    Py_ssize_t argc;
    PyObject *parse_args = NULL;
    PyObject *pin_args = NULL;
    unsigned long mechanism;
    int key_size;
    SecItem *py_sec_param;
    PK11SymKey *sym_key;

    TraceMethodEnter(self);

    argc = PyTuple_Size(args);
    if (argc == n_base_args) {
        parse_args = args;
    } else {
        parse_args = PyTuple_GetSlice(args, 0, n_base_args);
    }
    if (!PyArg_ParseTuple(parse_args, "kO&i:key_gen",
                          &mechanism, SecItemOrNoneConvert, &py_sec_param,
                          &key_size)) {
        if (parse_args != args) {
            Py_DECREF(parse_args);
        }
        return NULL;
    }
    if (parse_args != args) {
        Py_DECREF(parse_args);
    }

    pin_args = PyTuple_GetSlice(args, n_base_args, argc);
    Py_INCREF(pin_args);

    if ((sym_key = PK11_KeyGen(self->slot, mechanism, py_sec_param ? &py_sec_param->item : NULL,
                               key_size, pin_args)) == NULL) {
        Py_DECREF(pin_args);
        return set_nspr_error(NULL);
    }

    Py_DECREF(pin_args);

    return PyPK11SymKey_new_from_PK11SymKey(sym_key);
}

static PyMethodDef PK11Slot_methods[] = {
    {"is_hw",                   (PyCFunction)PK11Slot_is_hw,                   METH_NOARGS,  PK11Slot_is_hw_doc},
    {"is_present",              (PyCFunction)PK11Slot_is_present,              METH_NOARGS,  PK11Slot_is_present_doc},
    {"is_read_only",            (PyCFunction)PK11Slot_is_read_only,            METH_NOARGS,  PK11Slot_is_read_only_doc},
    {"get_best_wrap_mechanism", (PyCFunction)PK11Slot_get_best_wrap_mechanism, METH_NOARGS,  PK11Slot_get_best_wrap_mechanism_doc},
    {"get_best_key_length",     (PyCFunction)PK11Slot_get_best_key_length,     METH_VARARGS, PK11Slot_get_best_key_length_doc},
    {"key_gen",                 (PyCFunction)PK11Slot_key_gen,                 METH_VARARGS, PK11Slot_key_gen_doc},
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
PK11Slot_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PK11Slot *self;

    TraceObjNewEnter(type);

    if ((self = (PK11Slot *)type->tp_alloc(type, 0)) == NULL) return NULL;
    self->slot = NULL;

    TraceObjNewLeave(type);
    return (PyObject *)self;
}

static void
PK11Slot_dealloc(PK11Slot* self)
{
    TraceMethodEnter(self);

    PK11_FreeSlot(self->slot);
    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(PK11Slot_doc,
"An object representing a PKCS #11 Slot");

static int
PK11Slot_init(PK11Slot *self, PyObject *args, PyObject *kwds)
{
    PyObject *arg1 = NULL;
    static char *kwlist[] = {"arg1", NULL};

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O", kwlist,
                                     &arg1))
        return -1;

    return 0;
}

static PyTypeObject PK11SlotType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.PK11Slot",				/* tp_name */
    sizeof(PK11Slot),				/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)PK11Slot_dealloc,		/* tp_dealloc */
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
    0,						/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
    PK11Slot_doc,				/* tp_doc */
    0,						/* tp_traverse */
    0,						/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    PK11Slot_methods,				/* tp_methods */
    PK11Slot_members,				/* tp_members */
    PK11Slot_getseters,				/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)PK11Slot_init,			/* tp_init */
    0,						/* tp_alloc */
    PK11Slot_new,				/* tp_new */
};

PyObject *
PK11Slot_new_from_PK11SlotInfo(PK11SlotInfo *slot)
{
    PK11Slot *self = NULL;

    TraceObjNewEnter(NULL);

    if ((self = (PK11Slot *) PK11SlotType.tp_new(&PK11SlotType, NULL, NULL)) == NULL)
        return NULL;

    self->slot = slot;

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

/* ========================================================================== */
/* =========================== PK11SymKey Class =========================== */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static PyObject *
PK11SymKey_get_mechanism(PyPK11SymKey *self, void *closure)
{
    return PyInt_FromLong(PK11_GetMechanism(self->pk11_sym_key));
}

static PyObject *
PK11SymKey_get_key_data(PyPK11SymKey *self, void *closure)
{
    SECItem *sec_item;

    if (PK11_ExtractKeyValue(self->pk11_sym_key) != SECSuccess) {
        return set_nspr_error(NULL);
    }

    if ((sec_item = PK11_GetKeyData(self->pk11_sym_key)) == NULL) {
        return PyString_FromStringAndSize("", 0);
    }

    return PyString_FromStringAndSize((const char *)sec_item->data, sec_item->len);
}

static PyObject *
PK11SymKey_get_key_length(PyPK11SymKey *self, void *closure)
{
    return PyInt_FromLong(PK11_GetKeyLength(self->pk11_sym_key));
}

static PyObject *
PK11SymKey_get_slot(PyPK11SymKey *self, void *closure)
{
    PK11SlotInfo *slot = NULL;
    PyObject *py_slot = NULL;

    slot = PK11_GetSlotFromKey(self->pk11_sym_key);
    if ((py_slot = PK11Slot_new_from_PK11SlotInfo(slot)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "unable to create PK11Slot object");
        return NULL;
    }
    return py_slot;
}

static
PyGetSetDef PK11SymKey_getseters[] = {
    {"mechanism",  (getter)PK11SymKey_get_mechanism,  (setter)NULL, "CK_MECHANISM_TYPE mechanism", NULL},
    {"key_data",   (getter)PK11SymKey_get_key_data,   (setter)NULL, "key data", NULL},
    {"key_length", (getter)PK11SymKey_get_key_length, (setter)NULL, "key length", NULL},
    {"slot",       (getter)PK11SymKey_get_slot,       (setter)NULL, "slot", NULL},
    {NULL}  /* Sentinel */
};

static PyMemberDef PK11SymKey_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

PyDoc_STRVAR(PK11SymKey_derive_doc,
"derive(mechanism, sec_param, target, operation, key_size) -> PK11SymKey\n\
\n\
:Parameters:\n\
    mechanism : int\n\
        key mechanism enumeration constant (CKM_*)\n\
    sec_param : SecItem object or None\n\
        mechanism parameters or None.\n\
    target : int\n\
        key mechanism enumeration constant (CKM_*)\n\
    operation : int\n\
        type of operation. A (CKA_*) constant\n\
        (e.g. CKA_ENCRYPT, CKA_DECRYPT, CKA_SIGN, CKA_VERIFY, CKA_DIGEST)\n\
    key_size : int\n\
        key size.\n\
\n\
Derive a new key from this key.\n\
Return a key which can do exactly one operation, it is\n\
ephemeral (session key).\n\
");
static PyObject *
PK11SymKey_derive(PyPK11SymKey *self, PyObject *args)
{
    unsigned long mechanism;
    SecItem *py_sec_param;
    unsigned long target;
    unsigned long operation;
    int key_size;
    PK11SymKey *derived_key = NULL;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "kO&kki:derive",
                          &mechanism, SecItemOrNoneConvert, &py_sec_param,
                          &target, &operation, &key_size))
        return NULL;

    if ((derived_key = PK11_Derive(self->pk11_sym_key, mechanism,
                                   py_sec_param ? &py_sec_param->item : NULL,
                                   target, operation, key_size)) == NULL) {
        return set_nspr_error(NULL);
    }

    return PyPK11SymKey_new_from_PK11SymKey(derived_key);
}

PyDoc_STRVAR(PK11SymKey_wrap_sym_key_doc,
"wrap_sym_key(mechanism, sec_param, sym_key) -> SecItem\n\
\n\
:Parameters:\n\
    mechanism : int\n\
        key mechanism enumeration constant (CKM_*)\n\
    sec_param : SecItem object or None\n\
        mechanism parameters or None.\n\
    sym_key : PK11SymKey object\n\
        the symmetric key to wrap\n\
\n\
Wrap (encrypt) the supplied sym_key using the mechanism\n\
and parameter. Return the wrapped key as a SecItem.\n\
");
static PyObject *
PK11SymKey_wrap_sym_key(PyPK11SymKey *self, PyObject *args)
{
    unsigned long mechanism;
    SecItem *py_sec_param;
    PyPK11SymKey *py_sym_key = NULL;
    SECItem wrapped_key;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "kO&O!:wrap_sym_key",
                          &mechanism, SecItemOrNoneConvert, &py_sec_param,
                          &PK11SymKeyType, &py_sym_key))
        return NULL;

    if (PK11_WrapSymKey(mechanism, py_sec_param ? &py_sec_param->item : NULL,
                        self->pk11_sym_key, py_sym_key->pk11_sym_key,
                        &wrapped_key) != SECSuccess) {
        return set_nspr_error(NULL);
    }

    return SecItem_new_from_SECItem(&wrapped_key, SECITEM_wrapped_key);
}

PyDoc_STRVAR(PK11SymKey_unwrap_sym_key_doc,
"unwrap_sym_key(mechanism, sec_param, wrapped_key, target, operation, key_size) -> PK11SymKey\n\
\n\
:Parameters:\n\
    mechanism : int\n\
        key mechanism enumeration constant (CKM_*)\n\
    sec_param : SecItem object or None\n\
        mechanism parameters or None.\n\
    wrapped_key : SecItem object\n\
        the symmetric key to unwrap\n\
    target : int\n\
        key mechanism enumeration constant (CKM_*)\n\
    operation : int\n\
        type of operation. A (CKA_*) constant\n\
        (e.g. CKA_ENCRYPT, CKA_DECRYPT, CKA_SIGN, CKA_VERIFY, CKA_DIGEST)\n\
    key_size : int\n\
        key size.\n\
\n\
Unwrap (decrypt) the supplied wrapped key.\n\
Return the unwrapped key as a PK11SymKey.\n\
");
static PyObject *
PK11SymKey_unwrap_sym_key(PyPK11SymKey *self, PyObject *args)
{
    unsigned long mechanism;
    SecItem *py_sec_param;
    unsigned long target;
    unsigned long operation;
    int key_size;
    SecItem *py_wrapped_key = NULL;
    PK11SymKey *sym_key = NULL;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "kO&O!kki:unwrap_sym_key",
                          &mechanism, SecItemOrNoneConvert, &py_sec_param,
                          &SecItemType, &py_wrapped_key,
                          &target, &operation, &key_size))
        return NULL;

    if ((sym_key = PK11_UnwrapSymKey(self->pk11_sym_key, mechanism,
                                     py_sec_param ? &py_sec_param->item : NULL,
                                     &py_wrapped_key->item,
                                     target, operation, key_size)) == NULL) {
        return set_nspr_error(NULL);
    }

    return PyPK11SymKey_new_from_PK11SymKey(sym_key);
}


static PyObject *
PK11SymKey_repr(PyPK11SymKey *self)
{
    return PyString_FromFormat("<%s object at %p>",
                               Py_TYPE(self)->tp_name, self);
}

static PyObject *
PK11SymKey_str(PyPK11SymKey *self)
{
    return PK11SymKey_repr(self);
}

static PyMethodDef PK11SymKey_methods[] = {
    {"derive",         (PyCFunction)PK11SymKey_derive,           METH_VARARGS, PK11SymKey_derive_doc},
    {"wrap_sym_key",   (PyCFunction)PK11SymKey_wrap_sym_key,     METH_VARARGS, PK11SymKey_wrap_sym_key_doc},
    {"unwrap_sym_key", (PyCFunction)PK11SymKey_unwrap_sym_key,   METH_VARARGS, PK11SymKey_unwrap_sym_key_doc},
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static void
PK11SymKey_dealloc(PyPK11SymKey* self)
{
    TraceMethodEnter(self);

    if (self->pk11_sym_key)
        PK11_FreeSymKey(self->pk11_sym_key);

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(PK11SymKey_doc,
"Holds a hash, encryption or signing context for multi-part operations.\n\
");
static int
PK11SymKey_init(PyPK11SymKey *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter(self);

    return 0;
}

static PyTypeObject PK11SymKeyType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.PK11SymKey",			/* tp_name */
    sizeof(PyPK11SymKey),			/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)PK11SymKey_dealloc,		/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    0,						/* tp_compare */
    (reprfunc)PK11SymKey_repr,			/* tp_repr */
    0,						/* tp_as_number */
    0,						/* tp_as_sequence */
    0,						/* tp_as_mapping */
    0,						/* tp_hash */
    0,						/* tp_call */
    (reprfunc)PK11SymKey_str,			/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
    PK11SymKey_doc,				/* tp_doc */
    0,						/* tp_traverse */
    0,						/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    PK11SymKey_methods,				/* tp_methods */
    PK11SymKey_members,				/* tp_members */
    PK11SymKey_getseters,			/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)PK11SymKey_init,			/* tp_init */
    0,						/* tp_alloc */
    0,/* NULL cannot be directly created */	/* tp_new */
};

static PyObject *
PyPK11SymKey_new_from_PK11SymKey(PK11SymKey *pk11_sym_key)
{
    PyPK11SymKey *self = NULL;

    TraceObjNewEnter(NULL);

    if ((self = PyObject_NEW(PyPK11SymKey, &PK11SymKeyType)) == NULL)
        return NULL;

    self->pk11_sym_key = pk11_sym_key;

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

/* ========================================================================== */
/* ============================ PK11Context Class =========================== */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static
PyGetSetDef PK11Context_getseters[] = {
    {NULL}  /* Sentinel */
};

static PyMemberDef PK11Context_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

PyDoc_STRVAR(PK11Context_digest_key_doc,
"digest_key(sym_key)\n\
\n\
:Parameters:\n\
    sym_key : PK11SymKey object\n\
        symmetric key\n\
\n\
Continues a multiple-part message-digesting operation by digesting the\n\
value of a secret key.\n\
");
static PyObject *
PK11Context_digest_key(PyPK11Context *self, PyObject *args)
{
    PyPK11SymKey *py_sym_key;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "O!:digest_key", &PK11SymKeyType, &py_sym_key))
        return NULL;

    if (PK11_DigestKey(self->pk11_context, py_sym_key->pk11_sym_key) != SECSuccess) {
        return set_nspr_error(NULL);
    }
    Py_RETURN_NONE;
}

PyDoc_STRVAR(PK11Context_clone_context_doc,
"clone_context(context) -> PK11Context\n\
\n\
:Parameters:\n\
    context : PK11Context object\n\
        The PK11Context to be cloned\n\
\n\
Create a new PK11Context which is clone of the supplied context.\n\
");
static PyObject *
PK11Context_clone_context(PyPK11Context *self, PyObject *args)
{
    PK11Context *pk11_context;
    PyObject *py_pk11_context;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "O!:clone_context", &PK11ContextType, &py_pk11_context))
        return NULL;

    if ((pk11_context = PK11_CloneContext(self->pk11_context)) == NULL) {
        return set_nspr_error(NULL);
    }

    if ((py_pk11_context =
         PyPK11Context_new_from_PK11Context(pk11_context)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "unable to create PK11Context object");
        return NULL;
    }

    return py_pk11_context;
}

PyDoc_STRVAR(PK11Context_digest_begin_doc,
"digest_begin()\n\
\n\
Start a new digesting or Mac'ing operation on this context.\n\
");
static PyObject *
PK11Context_digest_begin(PyPK11Context *self, PyObject *args)
{
    TraceMethodEnter(self);

    if (PK11_DigestBegin(self->pk11_context) != SECSuccess) {
        return set_nspr_error(NULL);
    }

    Py_RETURN_NONE;
}

PyDoc_STRVAR(PK11Context_digest_op_doc,
"digest_op(data)\n\
:Parameters:\n\
    data : any read buffer compatible object (e.g. buffer or string)\n\
        raw data to compute digest from\n\
\n\
Execute a digest/signature operation.\n\
");
static PyObject *
PK11Context_digest_op(PyPK11Context *self, PyObject *args)
{
    const void *buffer = NULL;
    Py_ssize_t buffer_len;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "t#:digest_op", &buffer, &buffer_len))
        return NULL;

    if (PK11_DigestOp(self->pk11_context, buffer, buffer_len) != SECSuccess) {
        return set_nspr_error(NULL);
    }

    Py_RETURN_NONE;
}

PyDoc_STRVAR(PK11Context_cipher_op_doc,
"cipher_op(data) -> data\n\
:Parameters:\n\
    data : any read buffer compatible object (e.g. buffer or string)\n\
        raw data to compute digest from\n\
\n\
Execute a digest/signature operation.\n\
");
static PyObject *
PK11Context_cipher_op(PyPK11Context *self, PyObject *args)
{
    const void *in_buf = NULL;
    void *out_buf = NULL;
    PyObject *out_string;
    Py_ssize_t in_buf_len;
    Py_ssize_t out_buf_alloc_len;
    int suggested_out_len = 0, actual_out_len;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "t#:cipher_op", &in_buf, &in_buf_len))
        return NULL;

    /*
     * Create an output buffer to hold the result.
     */

    /*
     * We call the PK11 function with a NULL output buffer and it returns an
     * upper bound on the size of the output data buffer. We create a string to
     * hold the data using the upper bound as it's size. We then invoke the PK11
     * function again which performs the operation writing into string buffer.
     * It returns the exact number of bytes written. If the allocated size does
     * not equal the actual number of bytes written we resize the string before
     * returning it so the caller sees a string whose length exactly matches
     * the number of bytes written by the PK11 function.
     */
    if (PK11_CipherOp(self->pk11_context, NULL, &suggested_out_len, 0,
                      (unsigned char *)in_buf, in_buf_len) != SECSuccess) {
        return set_nspr_error(NULL);
    }

    out_buf_alloc_len = suggested_out_len;

    if ((out_string = PyString_FromStringAndSize(NULL, out_buf_alloc_len)) == NULL) {
        return NULL;
    }
    out_buf = PyString_AsString(out_string);

    /*
     * Now that we have both the input and output buffers perform the cipher operation.
     */
    if (PK11_CipherOp(self->pk11_context, out_buf, &actual_out_len, out_buf_alloc_len,
                      (unsigned char *)in_buf, in_buf_len) != SECSuccess) {
        Py_DECREF(out_string);
        return set_nspr_error(NULL);
    }

    if (actual_out_len != out_buf_alloc_len) {
        if (_PyString_Resize(&out_string, actual_out_len) < 0) {
        return NULL;
        }
    }

    return out_string;
}

PyDoc_STRVAR(PK11Context_finalize_doc,
"finalize()\n\
\n\
Clean up cipher operation so that any pending multi-part\n\
operations have been flushed. Any pending output which would\n\
have been available as a result of the flush is discarded.\n\
The context is left in a state available for reuse.\n\
\n\
WARNING: Currently context reuse only works for digest contexts\n\
not encryption/decryption contexts\n\
");
static PyObject *
PK11Context_finalize(PyPK11Context *self, PyObject *args)
{
    TraceMethodEnter(self);

    if (PK11_Finalize(self->pk11_context) != SECSuccess) {
        return set_nspr_error(NULL);
    }

    Py_RETURN_NONE;
}

PyDoc_STRVAR(PK11Context_digest_final_doc,
"digest_final() -> data\n\
\n\
Completes the multi-part cryptographic operation in progress\n\
on this context and returns any final data which may have been\n\
pending in the context (i.e. the output data is flushed from the\n\
context). If there was no final data the returned\n\
data buffer will have a length of zero.\n\
");
static PyObject *
PK11Context_digest_final(PyPK11Context *self, PyObject *args)
{
    void *out_buf = NULL;
    Py_ssize_t out_buf_alloc_len;
    unsigned int suggested_out_len = 0, actual_out_len;
    PyObject *out_string;

    TraceMethodEnter(self);

    /*
     * We call the PK11 function with a NULL output buffer and it returns an
     * upper bound on the size of the output data buffer. We create a string to
     * hold the data using the upper bound as it's size. We then invoke the PK11
     * function again which performs the operation writing into string buffer.
     * It returns the exact number of bytes written. If the allocated size does
     * not equal the actual number of bytes written we resize the string before
     * returning it so the caller sees a string whose length exactly matches
     * the number of bytes written by the PK11 function.
     */

    if (PK11_DigestFinal(self->pk11_context, NULL, &suggested_out_len, 0) != SECSuccess) {
        return set_nspr_error(NULL);
    }

    out_buf_alloc_len = suggested_out_len;

    if ((out_string = PyString_FromStringAndSize(NULL, out_buf_alloc_len)) == NULL) {
        return NULL;
    }
    out_buf = PyString_AsString(out_string);

    /*
     * Now that we have the output buffer perform the cipher operation.
     */
    if (PK11_DigestFinal(self->pk11_context, out_buf,
                         &actual_out_len, out_buf_alloc_len) != SECSuccess) {
        Py_DECREF(out_string);
        return set_nspr_error(NULL);
    }

    if (actual_out_len != out_buf_alloc_len) {
        if (_PyString_Resize(&out_string, actual_out_len) < 0) {
        return NULL;
        }
    }

    return out_string;
}

static PyObject *
PK11Context_repr(PyPK11Context *self)
{
    return PyString_FromFormat("<%s object at %p>",
                               Py_TYPE(self)->tp_name, self);
}

static PyObject *
PK11Context_str(PyPK11Context *self)
{
    return PK11Context_repr(self);
}

static PyMethodDef PK11Context_methods[] = {
    {"digest_key",    (PyCFunction)PK11Context_digest_key,    METH_VARARGS, PK11Context_digest_key_doc},
    {"clone_context", (PyCFunction)PK11Context_clone_context, METH_VARARGS, PK11Context_clone_context_doc},
    {"digest_begin",  (PyCFunction)PK11Context_digest_begin,  METH_NOARGS,  PK11Context_digest_begin_doc},
    {"digest_op",     (PyCFunction)PK11Context_digest_op,     METH_VARARGS, PK11Context_digest_op_doc},
    {"cipher_op",     (PyCFunction)PK11Context_cipher_op,     METH_VARARGS, PK11Context_cipher_op_doc},
    {"finalize",      (PyCFunction)PK11Context_finalize,      METH_NOARGS,  PK11Context_finalize_doc},
    {"digest_final",  (PyCFunction)PK11Context_digest_final,  METH_NOARGS,  PK11Context_digest_final_doc},
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
PK11Context_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyPK11Context *self;

    TraceObjNewEnter(type);

    if ((self = (PyPK11Context *)type->tp_alloc(type, 0)) == NULL) return NULL;

    self->pk11_context = NULL;

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static void
PK11Context_dealloc(PyPK11Context* self)
{
    TraceMethodEnter(self);

    if (self->pk11_context)
        PK11_DestroyContext(self->pk11_context, PR_TRUE);

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(PK11Context_doc,
"\n\
");
static int
PK11Context_init(PyPK11Context *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter(self);

    return 0;
}

static PyTypeObject PK11ContextType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.PK11Context",			/* tp_name */
    sizeof(PyPK11Context),			/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)PK11Context_dealloc,		/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    0,						/* tp_compare */
    (reprfunc)PK11Context_repr,			/* tp_repr */
    0,						/* tp_as_number */
    0,						/* tp_as_sequence */
    0,						/* tp_as_mapping */
    0,						/* tp_hash */
    0,						/* tp_call */
    (reprfunc)PK11Context_str,			/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
    PK11Context_doc,				/* tp_doc */
    0,						/* tp_traverse */
    0,						/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    PK11Context_methods,			/* tp_methods */
    PK11Context_members,			/* tp_members */
    PK11Context_getseters,			/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)PK11Context_init,			/* tp_init */
    0,						/* tp_alloc */
    PK11Context_new,				/* tp_new */
};

static PyObject *
PyPK11Context_new_from_PK11Context(PK11Context *pk11_context)

{
    PyPK11Context *self = NULL;

    TraceObjNewEnter(NULL);

    if ((self = (PyPK11Context *) PK11ContextType.tp_new(&PK11ContextType, NULL, NULL)) == NULL)
        return NULL;

    self->pk11_context = pk11_context;

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

/* ========================== PK11 Module Methods =========================== */

static PyObject *password_callback = NULL;

static char *
PK11_password_callback(PK11SlotInfo *slot, PRBool retry, void *arg)
{
    PyObject *pin_args = arg; /* borrowed reference, don't decrement */
    PyObject *py_retry = NULL;
    PyObject *py_slot = NULL;
    PyObject *item;
    PyObject *result = NULL;
    PyObject *new_args = arg;
    Py_ssize_t argc;
    int i, j;
    char *password = NULL;

    TraceMessage("PK11_password_callback: enter");

    if (password_callback == NULL) {
        PySys_WriteStderr("PK11 password callback undefined\n");
        return NULL;;
    }

    argc = 2;
    if (pin_args) {
        if (PyTuple_Check(pin_args)) {
            argc += PyTuple_Size(pin_args);
        } else {
            PySys_WriteStderr("Error, PK11 password callback expected args to be tuple\n");
            PyErr_Print();
        }
    }

    if ((new_args = PyTuple_New(argc)) == NULL) {
        PySys_WriteStderr("PK11 password callback: out of memory\n");
        goto exit;
    }

    py_retry = PyBool_FromLong(retry);
    Py_INCREF(py_retry);

    if ((py_slot = PK11Slot_new_from_PK11SlotInfo(slot)) == NULL) {
        PySys_WriteStderr("exception in PK11 password callback\n");
        PyErr_Print();
        goto exit;
        return NULL;;
    }

    PyTuple_SetItem(new_args, 0, py_slot);
    PyTuple_SetItem(new_args, 1, py_retry);

    for (i = 2, j = 0; i < argc; i++, j++) {
        item = PyTuple_GetItem(pin_args, j);
        Py_INCREF(item);
        PyTuple_SetItem(new_args, i, item);
    }

    if ((result = PyObject_CallObject(password_callback, new_args)) == NULL) {
        PySys_WriteStderr("exception in PK11 password callback\n");
        PyErr_Print();  /* this also clears the error */
        goto exit;
    }

    if (!PyString_Check(result)) {
        PySys_WriteStderr("Error, PK11 password callback expected string result.\n");
        goto exit;
    }

    password = PyString_AsString(result);

 exit:
    TraceMessage("PK11_password_callback: exiting");

    Py_XDECREF(new_args);
    //FIXME Py_XDECREF(py_retry);
    //FIXME Py_XDECREF(py_slot);
    Py_XDECREF(result);

    if (password)
        return PORT_Strdup(password);
    else
        return NULL;
}

PyDoc_STRVAR(pk11_set_password_callback_doc,
"set_password_callback(callback)\n\
\n\
:Parameters:\n\
    callback : function pointer\n\
        The callback function\n\
        \n\
The callback has the signature::\n\
    \n\
    password_callback(slot, retry, [user_data1, ...])\n\
\n\
slot\n\
    PK11Slot object\n\
retry\n\
    boolean indicating if this is a retry\n\
user_dataN\n\
    zero or more caller supplied optional parameters\n\
");

static PyObject *
pk11_set_password_callback(PyObject *self, PyObject *args)
{
    PyObject *callback;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "O:set_password_callback", &callback)) {
        return NULL;
    }

    if (!PyCallable_Check(callback)) {
        PyErr_SetString(PyExc_TypeError, "callback must be callable");
        return NULL;
    }

    Py_XDECREF(password_callback);
    password_callback = callback;
    Py_INCREF(callback);

    PK11_SetPasswordFunc(PK11_password_callback);

    Py_RETURN_NONE;
}

PyDoc_STRVAR(pk11_find_cert_from_nickname_doc,
"find_cert_from_nickname(nickname, [user_data1, ...]) -> Certificate\n\
\n\
:Parameters:\n\
    nickname : string\n\
        certificate nickname to search for\n\
    user_dataN : object ...\n\
        zero or more caller supplied parameters which will\n\
        be passed to the password callback function\n\
\n\
A nickname is an alias for a certificate subject. There may be\n\
multiple certificates with the same subject, and hence the same\n\
nickname. This function will return the newest certificate that\n\
matches the subject, based on the NotBefore / NotAfter fields of the\n\
certificate.\n\
");

static PyObject *
pk11_find_cert_from_nickname(PyObject *self, PyObject *args)
{
    Py_ssize_t n_base_args = 1;
    Py_ssize_t argc;
    PyObject *parse_args = NULL;
    PyObject *pin_args = NULL;
    char *nickname = NULL;
    CERTCertificate *cert = NULL;
    PyObject *py_cert = NULL;

    TraceMethodEnter(self);

    argc = PyTuple_Size(args);
    if (argc == n_base_args) {
        parse_args = args;
    } else {
        parse_args = PyTuple_GetSlice(args, 0, n_base_args);
    }
    if (!PyArg_ParseTuple(parse_args, "s:find_cert_from_nickname", &nickname)) {
        if (parse_args != args) {
            Py_DECREF(parse_args);
        }
        return NULL;
    }
    if (parse_args != args) {
        Py_DECREF(parse_args);
    }

    pin_args = PyTuple_GetSlice(args, n_base_args, argc);
    Py_INCREF(pin_args);

    if ((cert = PK11_FindCertFromNickname(nickname, pin_args)) == NULL) {
        Py_DECREF(pin_args);
        return set_nspr_error(NULL);
    }

    Py_DECREF(pin_args);

    if ((py_cert = Certificate_new_from_CERTCertificate(cert)) == NULL) {
        return NULL;
    }

    return py_cert;
}

PyDoc_STRVAR(pk11_find_key_by_any_cert_doc,
"find_key_by_any_cert(cert, [user_data1, ...]) -> Certificate\n\
\n\
:Parameters:\n\
    cert : Certificate object\n\
        certificate whose private key is being searched for\n\
    user_dataN : object ...\n\
        zero or more caller supplied parameters which will\n\
        be passed to the password callback function\n\
\n\
Finds the private key associated with a specified certificate in any\n\
available slot.\n\
");

static PyObject *
pk11_find_key_by_any_cert(PyObject *self, PyObject *args)
{
    Py_ssize_t n_base_args = 1;
    Py_ssize_t argc;
    PyObject *parse_args = NULL;
    Certificate *py_cert = NULL;
    PyObject *pin_args = NULL;
    SECKEYPrivateKey *private_key;
    PyObject *py_private_key = NULL;

    TraceMethodEnter(self);

    argc = PyTuple_Size(args);
    if (argc == n_base_args) {
        parse_args = args;
    } else {
        parse_args = PyTuple_GetSlice(args, 0, n_base_args);
    }
    if (!PyArg_ParseTuple(parse_args, "O!:find_key_by_any_cert",
                          &CertificateType, &py_cert)) {
        if (parse_args != args) {
            Py_DECREF(parse_args);
        }
        return NULL;
    }
    if (parse_args != args) {
        Py_DECREF(parse_args);
    }

    pin_args = PyTuple_GetSlice(args, n_base_args, argc);
    Py_INCREF(pin_args);

    if ((private_key = PK11_FindKeyByAnyCert(py_cert->cert, pin_args)) == NULL) {
        Py_DECREF(pin_args);
        return set_nspr_error(NULL);
    }

    Py_DECREF(pin_args);

    if ((py_private_key = PrivateKey_new_from_SECKEYPrivateKey(private_key)) == NULL) {
        return NULL;
    }

    return py_private_key;
}

PyDoc_STRVAR(pk11_generate_random_doc,
"generate_random(num_bytes) -> string\n\
\n\
:Parameters:\n\
    num_bytes : integer\n\
        Number of num_bytes to generate (must be non-negative)\n\
\n\
Generates random data..\n\
");

static PyObject *
pk11_generate_random(PyObject *self, PyObject *args)
{
    int num_bytes;
    unsigned char *buf;
    SECStatus status;
    PyObject *res;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "i:generate_random", &num_bytes))
        return NULL;

    if (num_bytes < 0) {
        PyErr_SetString(PyExc_ValueError, "byte count must be non-negative");
        return NULL;
    }

    buf = PyMem_Malloc(num_bytes);
    if (buf == NULL)
        return PyErr_NoMemory();

    status = PK11_GenerateRandom(buf, num_bytes);
    if (status != SECSuccess) {
	PyMem_Free(buf);
	return set_nspr_error(NULL);
    }

    res = PyString_FromStringAndSize((char *)buf, num_bytes);
    PyMem_Free(buf);
    return res;
}

PyDoc_STRVAR(nss_indented_format_doc,
"indented_format(line_pairs, indent='    ') -> string\n\
\n\
:Parameters:\n\
    line_pairs : [(level, string),...]\n\
        A list of pairs. Each pair is a 2 valued tuple with the first pair\n\
        value being the indentation level and the second pair value being\n\
        a string value for the line.\n\
    indent : string\n\
        A string repeated level times and then prepended to the line string.\n\
\n\
This function is equivalent to::\n\
\n\
'\\n'.join([indent*x[0]+x[1] for x in obj.format()])\n\
\n\
But is more efficient and does more error checking.\n\
\n\
Example::\n\
    \n\
    format = [(0, 'line 1'), (1, 'line 2'), (0, 'line 3')]\n\
    nss.indented(format)\n\
\n\
    would print\n\
    line 1\n\
        line 2\n\
    line 3\n\
");

static PyObject *
nss_indented_format(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"lines", "indent", NULL};
    PyObject *py_lines = NULL;
    PyObject *py_indent = NULL;
    long line_level = 0;
    long cur_level = -1;
    PyObject *py_cur_level_indent = NULL;
    char *indent = NULL;
    Py_ssize_t indent_len = 0;
    long cur_indent_len = 0;
    char *indent_end = NULL;
    char *src=NULL, *dst=NULL;
    Py_ssize_t num_lines;
    char *line = NULL;
    Py_ssize_t line_len;
    char *line_end = NULL;
    PyObject *py_pair = NULL;
    PyObject *py_level;
    PyObject *py_line;
    Py_ssize_t cur_formatted_line_len;
    PyObject *py_formatted_str = NULL;
    Py_ssize_t formatted_str_len;
    char *formatted_str;
    long i;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!|S:indented_format", kwlist,
                                     &PyList_Type, &py_lines, &py_indent))
        return NULL;

    if (!py_indent) {
        if ((py_indent = PyString_FromString("    ")) == NULL)
            goto fail;
    } else {
        Py_INCREF(py_indent);
    }

    indent_len = PyString_Size(py_indent);
    formatted_str_len = 0;

    num_lines = PyList_Size(py_lines);

    /* First, scan all the lines and compute the final destination size, do all error in this
       loop so we don't have to do it again during the copy phase */
    for (i = 0; i < num_lines; i++) {
        py_pair = PyList_GET_ITEM(py_lines, i);
        if (!PyTuple_Check(py_pair) || PyTuple_Size(py_pair) != 2) {
            PyErr_Format(PyExc_TypeError, "lines[%ld] must be a 2 valued tuple", i);
            goto fail;
        }

        py_level = PyTuple_GET_ITEM(py_pair, 0);
        py_line  = PyTuple_GET_ITEM(py_pair, 1);

        if (!PyInt_Check(py_level)) {
            PyErr_Format(PyExc_TypeError, "the first item in the pair at lines[%ld] list must be an integer", i);
            goto fail;
        }
        line_level = PyInt_AsLong(py_level);
        if (line_level < 0) {
            PyErr_Format(PyExc_TypeError, "the first item in the pair at lines[%ld] list must be a non-negative integer", i);
            goto fail;
        }

        if (!PyString_Check(py_line)) {
            PyErr_Format(PyExc_TypeError, "the second item in the pair at lines[%ld] list must be a string", i);
            goto fail;
        }
        if (PyString_AsStringAndSize(py_line, &line, &line_len) == -1)
            goto fail;

        cur_indent_len = line_level * indent_len;
        cur_formatted_line_len = cur_indent_len + line_len + 1; /* +1 for newline */
        formatted_str_len += cur_formatted_line_len;
    }

    /* Now copy the strings into the destination, note all error checking has been done above */
    if (num_lines > 0) formatted_str_len -= 1; /* last line doesn't get a new line appended */
    if ((py_formatted_str = PyString_FromStringAndSize(NULL, formatted_str_len)) == NULL)
        goto fail;

    formatted_str = PyString_AsString(py_formatted_str);
    dst = formatted_str;

    for (i = 0; i < num_lines; i++) {
        py_pair = PyList_GET_ITEM(py_lines, i);
        py_level = PyTuple_GET_ITEM(py_pair, 0);
        py_line  = PyTuple_GET_ITEM(py_pair, 1);

        line_level = PyInt_AsLong(py_level);
        PyString_AsStringAndSize(py_line, &line, &line_len);
        line_end = line + line_len;

        if (line_level != cur_level) {
            cur_level = line_level;
            if ((py_cur_level_indent = PySequence_Repeat(py_indent, cur_level)) == NULL) {
                goto fail;
            }
            if (PyString_AsStringAndSize(py_cur_level_indent, &indent, &indent_len) == -1)
                goto fail;
            indent_end = indent + indent_len;
        }

        for (src = indent; src < indent_end; *dst++ = *src++);
        for (src = line; src < line_end; *dst++ = *src++);
        if (i < num_lines-1)
            *dst++ = '\n';
    }

    assert(formatted_str + PyString_Size(py_formatted_str) == dst);
    Py_DECREF(py_indent);
    Py_XDECREF(py_cur_level_indent);
    return py_formatted_str;

    fail:
        Py_XDECREF(py_indent);
        Py_XDECREF(py_cur_level_indent);
        Py_XDECREF(py_formatted_str);
        return NULL;
}

/* ============================== Module Methods ============================= */

PyDoc_STRVAR(nss_init_doc,
"nss_init(cert_dir)\n\
\n\
:Parameters:\n\
    cert_dir : string\n\
        Pathname of the directory where the certificate, key, and\n\
        security module databases reside.\n\
\n\
Sets up configuration files and performs other tasks required to run\n\
Network Security Services.\n\
");

static PyObject *
nss_init(PyObject *self, PyObject *args)
{
    char *cert_dir;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "s:nss_init", &cert_dir)) {
        return NULL;
    }

    if (NSS_Init(cert_dir) != SECSuccess) {
        return set_nspr_error(NULL);
    }
    Py_RETURN_NONE;
}

PyDoc_STRVAR(nss_init_nodb_doc,
"nss_init_nodb()\n\
\n\
Performs tasks required to run Network Security Services without setting up\n\
configuration files. Important: This NSS function is not intended for use with\n\
SSL, which requires that the certificate and key database files be opened.\n\
\n\
nss_init_nodb opens only the temporary database and the internal PKCS #112\n\
module. Unlike nss_init, nss_init_nodb allows applications that do not have\n\
access to storage for databases to run raw crypto, hashing, and certificate\n\
functions. nss_init_nodb is not idempotent, so call it only once. The policy\n\
flags for all cipher suites are turned off by default, disallowing all cipher\n\
suites. Therefore, an application cannot use NSS to perform any cryptographic\n\
operations until after it enables appropriate cipher suites by calling one of\n\
the SSL Export Policy Functions.\n\
");

static PyObject *
nss_init_nodb(PyObject *self, PyObject *args)
{
    TraceMethodEnter(self);

    if (NSS_NoDB_Init(NULL) != SECSuccess) {
        return set_nspr_error(NULL);
    }
    Py_RETURN_NONE;
}

PyDoc_STRVAR(nss_shutdown_doc,
"nss_shutdown()\n\
\n\
Closes the key and certificate databases that were opened by nss_init().\n\
\n\
Note that if any reference to an NSS object is leaked (for example, if an SSL\n\
client application doesn't call clear_session_cache() first) then nss_shutdown fails\n\
with the error code SEC_ERROR_BUSY.\n\
");

static PyObject *
nss_shutdown(PyObject *self, PyObject *args)
{
    TraceMethodEnter(self);

    if (NSS_Shutdown() != SECSuccess) {
        return set_nspr_error(NULL);
    }
    Py_RETURN_NONE;
}

PyDoc_STRVAR(cert_sec_oid_tag_str_doc,
"sec_oid_tag_str(oid_tag) -> string\n\
\n\
:Parameters:\n\
    oid_tag : int\n\
        sec_oid constant\n\
\n\
Given a sec_oid constant return it's name as a string\n\
");
static PyObject *
cert_sec_oid_tag_str(PyObject *self, PyObject *args)
{
    unsigned long tag;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "k:sec_oid_tag_str", &tag))
        return NULL;

    return PyString_FromString(sec_oid_tag_str(tag));
}

PyDoc_STRVAR(cert_sec_oid_tag_from_name_doc,
"sec_oid_tag_from_name(name) -> int\n\
\n\
:Parameters:\n\
    name : string\n\
        name of a sec oid constant (SEC_OID_*)\n\
\n\
Given the name of a sec_oid constant return it's integer constant\n\
The string comparison is case insensitive and will match with\n\
or without the SEC_OID\\_ prefix\n\
");
static PyObject *
cert_sec_oid_tag_from_name(PyObject *self, PyObject *args)
{
    PyObject *py_name;
    PyObject *py_upper_name;
    PyObject *py_value;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "S:sec_oid_tag_from_name", &py_name))
        return NULL;

    if ((py_upper_name = PyObject_CallMethod(py_name, "upper", NULL)) == NULL) {
        return NULL;
    }

    if ((py_value = PyDict_GetItem(sec_oid_name_to_value, py_upper_name)) == NULL) {
	PyErr_Format(PyExc_KeyError, "oid tag name not found: %s", PyString_AsString(py_name));
        Py_DECREF(py_upper_name);
        return NULL;
    }

    Py_DECREF(py_upper_name);
    Py_INCREF(py_value);

    return py_value;
}

PyDoc_STRVAR(cert_sec_oid_tag_name_doc,
"sec_oid_tag_name(tag) -> string\n\
\n\
:Parameters:\n\
    tag : integer\n\
        sec oid constant (SEC_OID_*)\n\
\n\
Given an integer constant (SEC_OID_*) return the string name\n\
of the constant.  Note, this is different from\n\
sec_oid_tag_str() which returns a more verbose description\n\
of the tag constant. This returns the actual constant name.\n\
");
static PyObject *
cert_sec_oid_tag_name(PyObject *self, PyObject *args)
{
    unsigned long tag;
    PyObject *py_value;
    PyObject *py_name;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "k:sec_oid_tag_name", &tag))
        return NULL;

    if ((py_value = PyInt_FromLong(tag)) == NULL) {
        return NULL;
    }

    if ((py_name = PyDict_GetItem(sec_oid_value_to_name, py_value)) == NULL) {
	PyErr_Format(PyExc_KeyError, "oid tag not found: %#lx", tag);
        Py_DECREF(py_value);
        return NULL;
    }

    Py_DECREF(py_value);
    Py_INCREF(py_name);
    return py_name;
}

static const char *
key_mechanism_type_name(CK_MECHANISM_TYPE mechanism)
{
    PyObject *py_value;
    PyObject *py_name;
    const char *name;
    static char buf[80];

    if ((py_value = PyInt_FromLong(mechanism)) == NULL) {
        goto fail;
    }

    if ((py_name = PyDict_GetItem(ckm_value_to_name, py_value)) == NULL) {
        goto fail;
    }

    name = PyString_AsString(py_name);
    strncpy(buf, name, sizeof(buf));
    buf[sizeof(buf) - 1] = 0;
    Py_DECREF(py_value);
    return buf;

 fail:
    Py_XDECREF(py_value);
    snprintf(buf, sizeof(buf), "unknown(%#lx)", mechanism);
    return buf;
}

PyDoc_STRVAR(pk11_key_mechanism_type_name_doc,
"key_mechanism_type_name(mechanism) -> string\n\
\n\
:Parameters:\n\
    mechanism : int\n\
        key mechanism enumeration constant (CKM_*)\n\
\n\
Given a key mechanism enumeration constant (CKM_*)\n\
return it's name as a string\n\
");
static PyObject *
pk11_key_mechanism_type_name(PyObject *self, PyObject *args)
{
    unsigned long mechanism;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "k:key_mechanism_type_name", &mechanism))
        return NULL;

    return PyString_FromString(key_mechanism_type_name(mechanism));
}

PyDoc_STRVAR(pk11_key_mechanism_type_from_name_doc,
"key_mechanism_type_from_name(name) -> int\n\
\n\
:Parameters:\n\
    name : string\n\
        name of key mechanism enumeration constant (CKM_*)\n\
\n\
Given the name of a key mechanism enumeration constant (CKM_*)\n\
return it's integer constant\n\
The string comparison is case insensitive and will match with\n\
or without the CKM\\_ prefix\n\
");
static PyObject *
pk11_key_mechanism_type_from_name(PyObject *self, PyObject *args)
{
    PyObject *py_name;
    PyObject *py_upper_name;
    PyObject *py_value;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "S:key_mechanism_type_from_name", &py_name))
        return NULL;

    if ((py_upper_name = PyObject_CallMethod(py_name, "upper", NULL)) == NULL) {
        return NULL;
    }

    if ((py_value = PyDict_GetItem(ckm_name_to_value, py_upper_name)) == NULL) {
	PyErr_Format(PyExc_KeyError, "mechanism name not found: %s", PyString_AsString(py_name));
        Py_DECREF(py_upper_name);
        return NULL;
    }

    Py_DECREF(py_upper_name);
    Py_INCREF(py_value);

    return py_value;
}

static const char *
pk11_attribute_type_name(CK_ATTRIBUTE_TYPE type)
{
    PyObject *py_value;
    PyObject *py_name;
    const char *name;
    static char buf[80];

    if ((py_value = PyInt_FromLong(type)) == NULL) {
        goto fail;
    }

    if ((py_name = PyDict_GetItem(cka_value_to_name, py_value)) == NULL) {
        goto fail;
    }

    name = PyString_AsString(py_name);
    strncpy(buf, name, sizeof(buf));
    buf[sizeof(buf) - 1] = 0;
    Py_DECREF(py_value);
    return buf;

 fail:
    Py_XDECREF(py_value);
    snprintf(buf, sizeof(buf), "unknown(%#lx)", type);
    return buf;

}

PyDoc_STRVAR(pk11_attribute_type_name_doc,
"pk11_attribute_type_name(type) -> string\n\
\n\
:Parameters:\n\
    type : int\n\
        PK11 attribute type constant (CKA_*)\n\
\n\
Given a PK11 attribute type constant (CKA_*)\n\
return it's name as a string\n\
");
static PyObject *
pk11_pk11_attribute_type_name(PyObject *self, PyObject *args)
{
    unsigned long type;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "k:pk11_attribute_type_name", &type))
        return NULL;

    return PyString_FromString(pk11_attribute_type_name(type));
}

PyDoc_STRVAR(pk11_pk11_attribute_type_from_name_doc,
"pk11_attribute_type_from_name(name) -> int\n\
\n\
:Parameters:\n\
    name : string\n\
        name of PK11 attribute type constant (CKA_*)\n\
\n\
Given the name of a PK11 attribute type constant (CKA_*)\n\
return it's integer constant\n\
The string comparison is case insensitive and will match with\n\
or without the CKA\\_ prefix\n\
");
static PyObject *
pk11_pk11_attribute_type_from_name(PyObject *self, PyObject *args)
{
    PyObject *py_name;
    PyObject *py_upper_name;
    PyObject *py_value;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "S:pk11_attribute_type_from_name", &py_name))
        return NULL;

    if ((py_upper_name = PyObject_CallMethod(py_name, "upper", NULL)) == NULL) {
        return NULL;
    }

    if ((py_value = PyDict_GetItem(cka_name_to_value, py_upper_name)) == NULL) {
	PyErr_Format(PyExc_KeyError, "attribute name not found: %s", PyString_AsString(py_name));
        Py_DECREF(py_upper_name);
        return NULL;
    }

    Py_DECREF(py_upper_name);
    Py_INCREF(py_value);

    return py_value;
}

PyDoc_STRVAR(pk11_get_best_slot_doc,
"get_best_slot(mechanism, [user_data1, ...]) -> PK11Slot\n\
\n\
:Parameters:\n\
    mechanism : int\n\
        key mechanism enumeration constant (CKM_*)\n\
    user_dataN : object ...\n\
        zero or more caller supplied parameters which will\n\
        be passed to the password callback function\n\
\n\
Find the best slot which supports the given mechanism.\n\
");

static PyObject *
pk11_get_best_slot(PyObject *self, PyObject *args)
{
    Py_ssize_t n_base_args = 1;
    Py_ssize_t argc;
    PyObject *parse_args = NULL;
    PyObject *pin_args = NULL;
    unsigned long mechanism;
    PK11SlotInfo *slot = NULL;
    PyObject *py_slot = NULL;

    TraceMethodEnter(self);

    argc = PyTuple_Size(args);
    if (argc == n_base_args) {
        parse_args = args;
    } else {
        parse_args = PyTuple_GetSlice(args, 0, n_base_args);
    }
    if (!PyArg_ParseTuple(parse_args, "k:get_best_slot", &mechanism)) {
        if (parse_args != args) {
            Py_DECREF(parse_args);
        }
        return NULL;
    }
    if (parse_args != args) {
        Py_DECREF(parse_args);
    }

    pin_args = PyTuple_GetSlice(args, n_base_args, argc);
    Py_INCREF(pin_args);

    if ((slot = PK11_GetBestSlot(mechanism, pin_args)) == NULL) {
        Py_DECREF(pin_args);
        return set_nspr_error(NULL);
    }

    Py_DECREF(pin_args);

    if ((py_slot = PK11Slot_new_from_PK11SlotInfo(slot)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "unable to create PK11Slot object");
        return NULL;
    }

    return py_slot;
}

PyDoc_STRVAR(pk11_get_internal_key_slot_doc,
"get_internal_key_slot() -> PK11Slot\n\
\n\
Get the internal default slot.\n\
");

static PyObject *
pk11_get_internal_key_slot(PyObject *self, PyObject *args)
{
    PK11SlotInfo *slot = NULL;
    PyObject *py_slot = NULL;

    TraceMethodEnter(self);

    if ((slot = PK11_GetInternalKeySlot()) == NULL) {
        return set_nspr_error(NULL);
    }

    if ((py_slot = PK11Slot_new_from_PK11SlotInfo(slot)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "unable to create PK11Slot object");
        return NULL;
    }

    return py_slot;
}

PyDoc_STRVAR(pk11_create_context_by_sym_key_doc,
"create_context_by_sym_key(mechanism, operation, sym_key, sec_param=None) -> PK11Context\n\
\n\
:Parameters:\n\
    mechanism : int\n\
        key mechanism enumeration constant (CKM_*)\n\
    operation : int\n\
        type of operation this context will be doing. A (CKA_*) constant\n\
        (e.g. CKA_ENCRYPT, CKA_DECRYPT, CKA_SIGN, CKA_VERIFY, CKA_DIGEST)\n\
    sym_key : PK11SymKey object\n\
        symmetric key\n\
    sec_param : SecItem object or None\n\
        mechanism parameters used to build this context or None.\n\
\n\
Create a context from a symmetric key)\n\
");
static PyObject *
pk11_create_context_by_sym_key(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"mechanism", "operation", "sym_key", "sec_param", NULL};
    unsigned long mechanism;
    unsigned long operation;
    PyPK11SymKey *py_sym_key;
    SecItem *py_sec_param;
    PK11Context *pk11_context;
    PyObject *py_pk11_context;
    SECItem null_param = {0};

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "kkO!|O&:create_context_by_sym_key", kwlist,
                                     &mechanism, &operation,
                                     &PK11SymKeyType, &py_sym_key,
                                     SecItemOrNoneConvert, &py_sec_param))
        return NULL;

    if ((pk11_context =
         PK11_CreateContextBySymKey(mechanism, operation, py_sym_key->pk11_sym_key,
                                    py_sec_param ? &py_sec_param->item : &null_param)) == NULL) {
        return set_nspr_error(NULL);
    }

    if ((py_pk11_context = PyPK11Context_new_from_PK11Context(pk11_context)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "unable to create PK11Context object");
        return NULL;
    }

    return py_pk11_context;
}

PyDoc_STRVAR(pk11_import_sym_key_doc,
"import_sym_key(slot, mechanism, origin, operation, key_data, [user_data1, ...]) -> PK11SymKey\n\
\n\
:Parameters:\n\
    slot : PK11Slot object\n\
        designated PK11 slot\n\
    mechanism : int\n\
        key mechanism enumeration constant (CKM_*)\n\
    origin : int\n\
        PK11 origin enumeration (PK11Origin*)\n\
        e.g. PK11_OriginDerive, PK11_OriginUnwrap, etc.\n\
    operation : int\n\
        type of operation this context will be doing. A (CKA_*) constant\n\
        (e.g. CKA_ENCRYPT, CKA_DECRYPT, CKA_SIGN, CKA_VERIFY, CKA_DIGEST)\n\
    key_data: SecItem object\n\
        key data encapsulated in a SECItem used to build the symmetric key.\n\
    user_dataN : object ...\n\
        zero or more caller supplied parameters which will\n\
        be passed to the password callback function\n\
\n\
Create a PK11SymKey from data)\n\
");
static PyObject *
pk11_import_sym_key(PyObject *self, PyObject *args)
{
    Py_ssize_t n_base_args = 5;
    Py_ssize_t argc;
    PyObject *parse_args = NULL;
    PyObject *pin_args = NULL;
    PK11Slot *py_slot;
    unsigned long mechanism;
    unsigned long origin;
    unsigned long operation;
    SecItem *py_key_data;
    PK11SymKey *sym_key;

    TraceMethodEnter(self);

    argc = PyTuple_Size(args);
    if (argc == n_base_args) {
        parse_args = args;
    } else {
        parse_args = PyTuple_GetSlice(args, 0, n_base_args);
    }

    if (!PyArg_ParseTuple(parse_args, "O!kkkO!:import_sym_key",
                          &PK11SlotType, &py_slot,
                          &mechanism, &origin, &operation,
                          &SecItemType, &py_key_data)) {
        if (parse_args != args) {
            Py_DECREF(parse_args);
        }
        return NULL;
    }
    if (parse_args != args) {
        Py_DECREF(parse_args);
    }

    pin_args = PyTuple_GetSlice(args, n_base_args, argc);
    Py_INCREF(pin_args);

    if ((sym_key = PK11_ImportSymKey(py_slot->slot, mechanism, origin, operation,
                                     &py_key_data->item, pin_args)) == NULL) {
        Py_DECREF(pin_args);
        return set_nspr_error(NULL);
    }

    Py_DECREF(pin_args);

    return PyPK11SymKey_new_from_PK11SymKey(sym_key);
}

PyDoc_STRVAR(pk11_create_digest_context_doc,
"create_digest_context(hash_alg) -> PK11Context\n\
\n\
:Parameters:\n\
    hash_alg : int\n\
        hash algorithm enumeration (SEC_OID_*)\n\
        e.g.: SEC_OID_MD5, SEC_OID_SHA1, SEC_OID_SHA256, SEC_OID_SHA512, etc.\n\
\n\
Create a context for performing digest (hash) operations)\n\
");
static PyObject *
pk11_create_digest_context(PyObject *self, PyObject *args)
{
    unsigned long hash_alg;
    PK11Context *pk11_context;
    PyObject *py_pk11_context;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "k:create_digest_context", &hash_alg))
        return NULL;

    if ((pk11_context = PK11_CreateDigestContext(hash_alg)) == NULL) {
        return set_nspr_error(NULL);
    }

    if ((py_pk11_context =
         PyPK11Context_new_from_PK11Context(pk11_context)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "unable to create PK11Context object");
        return NULL;
    }

    return py_pk11_context;
}

PyDoc_STRVAR(pk11_param_from_iv_doc,
"param_from_iv(mechanism, iv=None) -> SecItem\n\
\n\
:Parameters:\n\
    mechanism : int\n\
        key mechanism enumeration constant (CKM_*)\n\
    iv : SecItem object\n\
        initialization vector. If there is no initialization vector you may also pass\n\
        None or an empty SecItem object (e.g. SecItem())\n\
\n\
Return a SecItem to be used as the initialization vector for encryption/decryption.\n\
");
static PyObject *
pk11_param_from_iv(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"mechanism", "iv", NULL};
    unsigned long mechanism;
    SecItem *py_iv;
    SECItem *sec_param;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "k|O&:param_from_iv", kwlist,
                                     &mechanism, SecItemOrNoneConvert, &py_iv))
        return NULL;

    if ((sec_param = PK11_ParamFromIV(mechanism, py_iv ? &py_iv->item : NULL)) == NULL) {
        return set_nspr_error(NULL);
    }

    return SecItem_new_from_SECItem(sec_param, SECITEM_iv_param);
}

PyDoc_STRVAR(pk11_param_from_algid_doc,
"param_from_algid(algid) -> SecItem\n\
\n\
:Parameters:\n\
    algid : SignatureAlgorithm object\n\
        algorithm id\n\
\n\
Return a SecItem containing a encryption param derived from a SignatureAlgorithm.\n\
");
static PyObject *
pk11_param_from_algid(PyObject *self, PyObject *args)
{
    SignatureAlgorithm *py_algorithm;
    SECItem *param;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "O!:param_from_algid", &SignatureAlgorithmType, &py_algorithm))
        return NULL;

    if ((param = PK11_ParamFromAlgid(&py_algorithm->id)) == NULL) {
        return set_nspr_error(NULL);
    }

    return SecItem_new_from_SECItem(param, SECITEM_unknown);
}

PyDoc_STRVAR(pk11_generate_new_param_doc,
"generate_new_param(mechanism, sym_key=None) -> SecItem\n\
\n\
:Parameters:\n\
    mechanism : int\n\
        key mechanism enumeration constant (CKM_*)\n\
    sym_key : PK11SymKey object or None\n\
        symmetric key or None\n\
\n\
Return a SecItem containing a encryption param.\n\
");
static PyObject *
pk11_generate_new_param(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"mechanism", "sym_key", NULL};
    unsigned long mechanism;
    PyPK11SymKey *py_sym_key;
    SECItem *param;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "k|O&:generate_new_param", kwlist,
                                     &mechanism, SymKeyOrNoneConvert, &py_sym_key))
        return NULL;

    if ((param = PK11_GenerateNewParam(mechanism,
                                       py_sym_key ? py_sym_key->pk11_sym_key : NULL)) == NULL) {
        return set_nspr_error(NULL);
    }

    return SecItem_new_from_SECItem(param, SECITEM_unknown);
}

PyDoc_STRVAR(pk11_algtag_to_mechanism_doc,
"algtag_to_mechanism(algtag) -> mechanism\n\
\n\
:Parameters:\n\
    algtag : int\n\
        algorithm tag (e.g. SEC_OID_*)\n\
\n\
Returns the key mechanism enumeration constant (CKM_*)\n\
given an algorithm tag. Throws a KeyError exception if the \n\
algorithm tag is invalid.\n\
");
static PyObject *
pk11_algtag_to_mechanism(PyObject *self, PyObject *args)
{
    unsigned long algtag;
    unsigned long mechanism;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "k:algtag_to_mechanism", &algtag))
        return NULL;

    if ((mechanism = PK11_AlgtagToMechanism(algtag)) == CKM_INVALID_MECHANISM) {
	PyErr_Format(PyExc_KeyError, "algtag not found: %#lx", algtag);
        return NULL;
    }

    return PyInt_FromLong(mechanism);
}

PyDoc_STRVAR(pk11_mechanism_to_algtag_doc,
"mechanism_to_algtag(mechanism) -> algtag\n\
\n\
:Parameters:\n\
    mechanism : int\n\
        key mechanism enumeration constant (CKM_*)\n\
\n\
Returns the algtag given key mechanism enumeration constant (CKM_*)\n\
Throws an KeyError exception if the mechanism is invalid.\n\
");
static PyObject *
pk11_mechanism_to_algtag(PyObject *self, PyObject *args)
{
    unsigned long algtag;
    unsigned long mechanism;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "k:mechanism_to_algtag", &mechanism))
        return NULL;

    if ((algtag = PK11_MechanismToAlgtag(mechanism)) == SEC_OID_UNKNOWN) {
	PyErr_Format(PyExc_KeyError, "mechanism not found: %#lx", mechanism);
        return NULL;
    }

    return PyInt_FromLong(algtag);
}
PyDoc_STRVAR(pk11_get_iv_length_doc,
"get_iv_length(mechanism) -> algtag\n\
\n\
:Parameters:\n\
    mechanism : int\n\
        key mechanism enumeration constant (CKM_*)\n\
\n\
Returns the length of the mechanism's initialization vector.\n\
");
static PyObject *
pk11_get_iv_length(PyObject *self, PyObject *args)
{
    unsigned long mechanism;
    int iv_length;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "k:get_iv_length", &mechanism))
        return NULL;

    iv_length = PK11_GetIVLength(mechanism);

    return PyInt_FromLong(iv_length);
}

PyDoc_STRVAR(pk11_get_block_size_doc,
"get_block_size(mechanism, sec_param=None) -> int\n\
\n\
:Parameters:\n\
    mechanism : int\n\
        key mechanism enumeration constant (CKM_*)\n\
    sec_param : SecItem object or None\n\
        mechanism parameters used to build this context or None.\n\
\n\
Get the mechanism block size\n\
");
static PyObject *
pk11_get_block_size(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"mechanism", "sec_param", NULL};
    unsigned long mechanism;
    SecItem *py_sec_param;
    int block_size;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "k|O&:get_block_size", kwlist,
                                     &mechanism, SecItemOrNoneConvert, &py_sec_param))
        return NULL;

    block_size = PK11_GetBlockSize(mechanism, py_sec_param ? &py_sec_param->item : NULL);

    return PyInt_FromLong(block_size);
}

PyDoc_STRVAR(pk11_get_pad_mechanism_doc,
"get_pad_mechanism(mechanism) -> int\n\
\n\
:Parameters:\n\
    mechanism : int\n\
        key mechanism enumeration constant (CKM_*)\n\
\n\
Determine appropriate mechanism to use when padding is required.\n\
If the mechanism does not map to a padding mechanism return the mechanism.\n\
");
static PyObject *
pk11_get_pad_mechanism(PyObject *self, PyObject *args)
{
    unsigned long mechanism;
    unsigned long pad_mechanism;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "k:get_pad_mechanism", &mechanism))
        return NULL;

    pad_mechanism = PK11_GetPadMechanism(mechanism);

    return PyInt_FromLong(pad_mechanism);
}

/* List of functions exported by this module. */
static PyMethodDef
module_methods[] = {
    {"nss_init",                      (PyCFunction)nss_init,                           METH_VARARGS,               nss_init_doc},
    {"nss_init_nodb",                 (PyCFunction)nss_init_nodb,                      METH_NOARGS,                nss_init_nodb_doc},
    {"nss_shutdown",                  (PyCFunction)nss_shutdown,                       METH_NOARGS,                nss_shutdown_doc},
    {"set_password_callback",         (PyCFunction)pk11_set_password_callback,         METH_VARARGS,               pk11_set_password_callback_doc},
    {"find_cert_from_nickname",       (PyCFunction)pk11_find_cert_from_nickname,       METH_VARARGS,               pk11_find_cert_from_nickname_doc},
    {"find_key_by_any_cert",          (PyCFunction)pk11_find_key_by_any_cert,          METH_VARARGS,               pk11_find_key_by_any_cert_doc},
    {"generate_random",               (PyCFunction)pk11_generate_random,               METH_VARARGS,               pk11_generate_random_doc},
    {"get_default_certdb",            (PyCFunction)cert_get_default_certdb,            METH_NOARGS,                cert_get_default_certdb_doc},
    {"get_cert_nicknames",            (PyCFunction)cert_get_cert_nicknames,            METH_VARARGS,               cert_get_cert_nicknames_doc},
    {"data_to_hex",                   (PyCFunction)cert_data_to_hex,                   METH_VARARGS|METH_KEYWORDS, cert_data_to_hex_doc},
    {"read_hex",                      (PyCFunction)read_hex,                           METH_VARARGS|METH_KEYWORDS, read_hex_doc},
    {"hash_buf",                      (PyCFunction)pk11_hash_buf,                      METH_VARARGS,               pk11_hash_buf_doc},
    {"md5_digest",                    (PyCFunction)pk11_md5_digest,                    METH_VARARGS,               pk11_md5_digest_doc},
    {"sha1_digest",                   (PyCFunction)pk11_sha1_digest,                   METH_VARARGS,               pk11_sha1_digest_doc},
    {"sha256_digest",                 (PyCFunction)pk11_sha256_digest,                 METH_VARARGS,               pk11_sha256_digest_doc},
    {"sha512_digest",                 (PyCFunction)pk11_sha512_digest,                 METH_VARARGS,               pk11_sha512_digest_doc},
    {"indented_format",               (PyCFunction)nss_indented_format,                METH_VARARGS|METH_KEYWORDS, nss_indented_format_doc},
    {"sec_oid_tag_str",               (PyCFunction)cert_sec_oid_tag_str,               METH_VARARGS,               cert_sec_oid_tag_str_doc},
    {"sec_oid_tag_name",              (PyCFunction)cert_sec_oid_tag_name,              METH_VARARGS,               cert_sec_oid_tag_name_doc},
    {"sec_oid_tag_from_name",         (PyCFunction)cert_sec_oid_tag_from_name,         METH_VARARGS,               cert_sec_oid_tag_from_name_doc},
    {"key_mechanism_type_name",       (PyCFunction)pk11_key_mechanism_type_name,       METH_VARARGS,               pk11_key_mechanism_type_name_doc},
    {"key_mechanism_type_from_name",  (PyCFunction)pk11_key_mechanism_type_from_name,  METH_VARARGS,               pk11_key_mechanism_type_from_name_doc},
    {"pk11_attribute_type_name",      (PyCFunction)pk11_pk11_attribute_type_name,      METH_VARARGS,               pk11_attribute_type_name_doc},
    {"pk11_attribute_type_from_name", (PyCFunction)pk11_pk11_attribute_type_from_name, METH_VARARGS,               pk11_pk11_attribute_type_from_name_doc},
    {"get_best_slot",                 (PyCFunction)pk11_get_best_slot,                 METH_VARARGS,               pk11_get_best_slot_doc},
    {"get_internal_key_slot",         (PyCFunction)pk11_get_internal_key_slot,         METH_NOARGS,                pk11_get_internal_key_slot_doc},
    {"create_context_by_sym_key",     (PyCFunction)pk11_create_context_by_sym_key,     METH_VARARGS|METH_KEYWORDS, pk11_create_context_by_sym_key_doc},
    {"import_sym_key",                (PyCFunction)pk11_import_sym_key,                METH_VARARGS,               pk11_import_sym_key_doc},
    {"create_digest_context",         (PyCFunction)pk11_create_digest_context,         METH_VARARGS,               pk11_create_digest_context_doc},
    {"param_from_iv",                 (PyCFunction)pk11_param_from_iv,                 METH_VARARGS|METH_KEYWORDS, pk11_param_from_iv_doc},
    {"param_from_algid",              (PyCFunction)pk11_param_from_algid,              METH_VARARGS,               pk11_param_from_algid_doc},
    {"generate_new_param",            (PyCFunction)pk11_generate_new_param,            METH_VARARGS|METH_KEYWORDS, pk11_generate_new_param_doc},
    {"algtag_to_mechanism",           (PyCFunction)pk11_algtag_to_mechanism,           METH_VARARGS,               pk11_algtag_to_mechanism_doc},
    {"mechanism_to_algtag",           (PyCFunction)pk11_mechanism_to_algtag,           METH_VARARGS,               pk11_mechanism_to_algtag_doc},
    {"get_iv_length",                 (PyCFunction)pk11_get_iv_length,                 METH_VARARGS,               pk11_get_iv_length_doc},
    {"get_block_size",                (PyCFunction)pk11_get_block_size,                METH_VARARGS|METH_KEYWORDS, pk11_get_block_size_doc},
    {"get_pad_mechanism",             (PyCFunction)pk11_get_pad_mechanism,             METH_VARARGS,               pk11_get_pad_mechanism_doc},
    {NULL, NULL} /* Sentinel */
};

/* ============================== Module Exports ============================= */

static PyNSPR_NSS_C_API_Type nspr_nss_c_api =
{
    &PK11SlotType,
    &CertDBType,
    &CertificateType,
    &PrivateKeyType,
    &SecItemType,
    Certificate_new_from_CERTCertificate,
    PrivateKey_new_from_SECKEYPrivateKey,
    SecItem_new_from_SECItem,
    cert_distnames_new_from_CERTDistNames,
    cert_distnames_as_CERTDistNames
};

/* ============================== Module Construction ============================= */

#define TYPE_READY(type)                                                \
{                                                                       \
    if (PyType_Ready(&type) < 0)                                        \
        return;                                                         \
    Py_INCREF(&type);                                                   \
    PyModule_AddObject(m, rindex(type.tp_name, '.')+1, (PyObject *)&type); \
}

PyDoc_STRVAR(module_doc,
"This module implements the NSS functions\n\
\n\
");

PyMODINIT_FUNC
initnss(void)
{
    PyObject *m;

    if (import_nspr_error_c_api() < 0) {
        return;
    }

    if ((m = Py_InitModule3("nss.nss", module_methods, module_doc)) == NULL) {
        return;
    }

    if ((empty_tuple = PyTuple_New(0)) == NULL) {
        return;
    }

    Py_INCREF(empty_tuple);

    TYPE_READY(SecItemType);
    TYPE_READY(SignatureAlgorithmType);
    TYPE_READY(KEYPQGParamsType);
    TYPE_READY(RSAPublicKeyType);
    TYPE_READY(DSAPublicKeyType);
    TYPE_READY(SignedDataType);
    TYPE_READY(PublicKeyType);
    TYPE_READY(SubjectPublicKeyInfoType);
    TYPE_READY(CertDBType);
    TYPE_READY(CertificateType);
    TYPE_READY(PrivateKeyType);
    TYPE_READY(PK11SlotType);
    TYPE_READY(PK11SymKeyType);
    TYPE_READY(PK11ContextType);

    /* Export C API */
    if (PyModule_AddObject(m, "_C_API", PyCObject_FromVoidPtr((void *)&nspr_nss_c_api, NULL)) != 0) {
        return;
    }

    AddIntConstant(certificateUsageCheckAllUsages);
    AddIntConstant(certificateUsageSSLClient);
    AddIntConstant(certificateUsageSSLServer);
    AddIntConstant(certificateUsageSSLServerWithStepUp);
    AddIntConstant(certificateUsageSSLCA);
    AddIntConstant(certificateUsageEmailSigner);
    AddIntConstant(certificateUsageEmailRecipient);
    AddIntConstant(certificateUsageObjectSigner);
    AddIntConstant(certificateUsageUserCertImport);
    AddIntConstant(certificateUsageVerifyCA);
    AddIntConstant(certificateUsageProtectedObjectSigner);
    AddIntConstant(certificateUsageStatusResponder);
    AddIntConstant(certificateUsageAnyCA);

    AddIntConstant(ssl_kea_null);
    AddIntConstant(ssl_kea_rsa);
    AddIntConstant(ssl_kea_dh);
    AddIntConstant(ssl_kea_fortezza);
    AddIntConstant(ssl_kea_ecdh);

    AddIntConstant(nullKey);
    AddIntConstant(rsaKey);
    AddIntConstant(dsaKey);
    AddIntConstant(fortezzaKey);
    AddIntConstant(dhKey);
    AddIntConstant(keaKey);
    AddIntConstant(ecKey);

    AddIntConstant(SEC_CERT_NICKNAMES_ALL);
    AddIntConstant(SEC_CERT_NICKNAMES_USER);
    AddIntConstant(SEC_CERT_NICKNAMES_SERVER);
    AddIntConstant(SEC_CERT_NICKNAMES_CA);

    AddIntConstant(secCertTimeValid);
    AddIntConstant(secCertTimeExpired);
    AddIntConstant(secCertTimeNotValidYet);


    /***************************************************************************
     * Mechanism Types
     ***************************************************************************/

    if ((ckm_name_to_value = PyDict_New()) == NULL) {
        return;
    }
    if ((ckm_value_to_name = PyDict_New()) == NULL) {
        return;
    }

#define ExportConstant(constant)                      \
if (_AddIntConstantWithLookup(m, #constant, constant, \
    "CKM_", ckm_name_to_value, ckm_value_to_name) < 0) return;

    ExportConstant(CKM_RSA_PKCS_KEY_PAIR_GEN);
    ExportConstant(CKM_RSA_PKCS);
    ExportConstant(CKM_RSA_9796);
    ExportConstant(CKM_RSA_X_509);

    /* CKM_MD2_RSA_PKCS, CKM_MD5_RSA_PKCS, and CKM_SHA1_RSA_PKCS
     * are new for v2.0.  They are mechanisms which hash and sign */
    ExportConstant(CKM_MD2_RSA_PKCS);
    ExportConstant(CKM_MD5_RSA_PKCS);
    ExportConstant(CKM_SHA1_RSA_PKCS);

    /* CKM_RIPEMD128_RSA_PKCS, CKM_RIPEMD160_RSA_PKCS, and
     * CKM_RSA_PKCS_OAEP are new for v2.10 */
    ExportConstant(CKM_RIPEMD128_RSA_PKCS);
    ExportConstant(CKM_RIPEMD160_RSA_PKCS);
    ExportConstant(CKM_RSA_PKCS_OAEP);

    /* CKM_RSA_X9_31_KEY_PAIR_GEN, CKM_RSA_X9_31, CKM_SHA1_RSA_X9_31,
     * CKM_RSA_PKCS_PSS, and CKM_SHA1_RSA_PKCS_PSS are new for v2.11 */
    ExportConstant(CKM_RSA_X9_31_KEY_PAIR_GEN);
    ExportConstant(CKM_RSA_X9_31);
    ExportConstant(CKM_SHA1_RSA_X9_31);
    ExportConstant(CKM_RSA_PKCS_PSS);
    ExportConstant(CKM_SHA1_RSA_PKCS_PSS);

    ExportConstant(CKM_DSA_KEY_PAIR_GEN);
    ExportConstant(CKM_DSA);
    ExportConstant(CKM_DSA_SHA1);
    ExportConstant(CKM_DH_PKCS_KEY_PAIR_GEN);
    ExportConstant(CKM_DH_PKCS_DERIVE);

    /* CKM_X9_42_DH_KEY_PAIR_GEN, CKM_X9_42_DH_DERIVE,
     * CKM_X9_42_DH_HYBRID_DERIVE, and CKM_X9_42_MQV_DERIVE are new for
     * v2.11 */
    ExportConstant(CKM_X9_42_DH_KEY_PAIR_GEN);
    ExportConstant(CKM_X9_42_DH_DERIVE);
    ExportConstant(CKM_X9_42_DH_HYBRID_DERIVE);
    ExportConstant(CKM_X9_42_MQV_DERIVE);

    /* CKM_SHA256/384/512 are new for v2.20 */
    ExportConstant(CKM_SHA256_RSA_PKCS);
    ExportConstant(CKM_SHA384_RSA_PKCS);
    ExportConstant(CKM_SHA512_RSA_PKCS);
    ExportConstant(CKM_SHA256_RSA_PKCS_PSS);
    ExportConstant(CKM_SHA384_RSA_PKCS_PSS);
    ExportConstant(CKM_SHA512_RSA_PKCS_PSS);

    /* CKM_SHA224 new for v2.20 amendment 3 */
    ExportConstant(CKM_SHA224_RSA_PKCS);
    ExportConstant(CKM_SHA224_RSA_PKCS_PSS);

    ExportConstant(CKM_RC2_KEY_GEN);
    ExportConstant(CKM_RC2_ECB);
    ExportConstant(CKM_RC2_CBC);
    ExportConstant(CKM_RC2_MAC);

    /* CKM_RC2_MAC_GENERAL and CKM_RC2_CBC_PAD are new for v2.0 */
    ExportConstant(CKM_RC2_MAC_GENERAL);
    ExportConstant(CKM_RC2_CBC_PAD);

    ExportConstant(CKM_RC4_KEY_GEN);
    ExportConstant(CKM_RC4);
    ExportConstant(CKM_DES_KEY_GEN);
    ExportConstant(CKM_DES_ECB);
    ExportConstant(CKM_DES_CBC);
    ExportConstant(CKM_DES_MAC);

    /* CKM_DES_MAC_GENERAL and CKM_DES_CBC_PAD are new for v2.0 */
    ExportConstant(CKM_DES_MAC_GENERAL);
    ExportConstant(CKM_DES_CBC_PAD);

    ExportConstant(CKM_DES2_KEY_GEN);
    ExportConstant(CKM_DES3_KEY_GEN);
    ExportConstant(CKM_DES3_ECB);
    ExportConstant(CKM_DES3_CBC);
    ExportConstant(CKM_DES3_MAC);

    /* CKM_DES3_MAC_GENERAL, CKM_DES3_CBC_PAD, CKM_CDMF_KEY_GEN,
     * CKM_CDMF_ECB, CKM_CDMF_CBC, CKM_CDMF_MAC,
     * CKM_CDMF_MAC_GENERAL, and CKM_CDMF_CBC_PAD are new for v2.0 */
    ExportConstant(CKM_DES3_MAC_GENERAL);
    ExportConstant(CKM_DES3_CBC_PAD);
    ExportConstant(CKM_CDMF_KEY_GEN);
    ExportConstant(CKM_CDMF_ECB);
    ExportConstant(CKM_CDMF_CBC);
    ExportConstant(CKM_CDMF_MAC);
    ExportConstant(CKM_CDMF_MAC_GENERAL);
    ExportConstant(CKM_CDMF_CBC_PAD);

    /* the following four DES mechanisms are new for v2.20 */
    ExportConstant(CKM_DES_OFB64);
    ExportConstant(CKM_DES_OFB8);
    ExportConstant(CKM_DES_CFB64);
    ExportConstant(CKM_DES_CFB8);

    ExportConstant(CKM_MD2);

    /* CKM_MD2_HMAC and CKM_MD2_HMAC_GENERAL are new for v2.0 */
    ExportConstant(CKM_MD2_HMAC);
    ExportConstant(CKM_MD2_HMAC_GENERAL);

    ExportConstant(CKM_MD5);

    /* CKM_MD5_HMAC and CKM_MD5_HMAC_GENERAL are new for v2.0 */
    ExportConstant(CKM_MD5_HMAC);
    ExportConstant(CKM_MD5_HMAC_GENERAL);

    ExportConstant(CKM_SHA_1);

    /* CKM_SHA_1_HMAC and CKM_SHA_1_HMAC_GENERAL are new for v2.0 */
    ExportConstant(CKM_SHA_1_HMAC);
    ExportConstant(CKM_SHA_1_HMAC_GENERAL);

    /* CKM_RIPEMD128, CKM_RIPEMD128_HMAC,
     * CKM_RIPEMD128_HMAC_GENERAL, CKM_RIPEMD160, CKM_RIPEMD160_HMAC,
     * and CKM_RIPEMD160_HMAC_GENERAL are new for v2.10 */
    ExportConstant(CKM_RIPEMD128);
    ExportConstant(CKM_RIPEMD128_HMAC);
    ExportConstant(CKM_RIPEMD128_HMAC_GENERAL);
    ExportConstant(CKM_RIPEMD160);
    ExportConstant(CKM_RIPEMD160_HMAC);
    ExportConstant(CKM_RIPEMD160_HMAC_GENERAL);

    /* CKM_SHA256/384/512 are new for v2.20 */
    ExportConstant(CKM_SHA256);
    ExportConstant(CKM_SHA256_HMAC);
    ExportConstant(CKM_SHA256_HMAC_GENERAL);
    ExportConstant(CKM_SHA384);
    ExportConstant(CKM_SHA384_HMAC);
    ExportConstant(CKM_SHA384_HMAC_GENERAL);
    ExportConstant(CKM_SHA512);
    ExportConstant(CKM_SHA512_HMAC);
    ExportConstant(CKM_SHA512_HMAC_GENERAL);

    /* CKM_SHA224 new for v2.20 amendment 3 */
    ExportConstant(CKM_SHA224);
    ExportConstant(CKM_SHA224_HMAC);
    ExportConstant(CKM_SHA224_HMAC_GENERAL);

    /* All of the following mechanisms are new for v2.0 */
    /* Note that CAST128 and CAST5 are the same algorithm */
    ExportConstant(CKM_CAST_KEY_GEN);
    ExportConstant(CKM_CAST_ECB);
    ExportConstant(CKM_CAST_CBC);
    ExportConstant(CKM_CAST_MAC);
    ExportConstant(CKM_CAST_MAC_GENERAL);
    ExportConstant(CKM_CAST_CBC_PAD);
    ExportConstant(CKM_CAST3_KEY_GEN);
    ExportConstant(CKM_CAST3_ECB);
    ExportConstant(CKM_CAST3_CBC);
    ExportConstant(CKM_CAST3_MAC);
    ExportConstant(CKM_CAST3_MAC_GENERAL);
    ExportConstant(CKM_CAST3_CBC_PAD);
    ExportConstant(CKM_CAST5_KEY_GEN);
    ExportConstant(CKM_CAST128_KEY_GEN);
    ExportConstant(CKM_CAST5_ECB);
    ExportConstant(CKM_CAST128_ECB);
    ExportConstant(CKM_CAST5_CBC);
    ExportConstant(CKM_CAST128_CBC);
    ExportConstant(CKM_CAST5_MAC);
    ExportConstant(CKM_CAST128_MAC);
    ExportConstant(CKM_CAST5_MAC_GENERAL);
    ExportConstant(CKM_CAST128_MAC_GENERAL);
    ExportConstant(CKM_CAST5_CBC_PAD);
    ExportConstant(CKM_CAST128_CBC_PAD);
    ExportConstant(CKM_RC5_KEY_GEN);
    ExportConstant(CKM_RC5_ECB);
    ExportConstant(CKM_RC5_CBC);
    ExportConstant(CKM_RC5_MAC);
    ExportConstant(CKM_RC5_MAC_GENERAL);
    ExportConstant(CKM_RC5_CBC_PAD);
    ExportConstant(CKM_IDEA_KEY_GEN);
    ExportConstant(CKM_IDEA_ECB);
    ExportConstant(CKM_IDEA_CBC);
    ExportConstant(CKM_IDEA_MAC);
    ExportConstant(CKM_IDEA_MAC_GENERAL);
    ExportConstant(CKM_IDEA_CBC_PAD);
    ExportConstant(CKM_GENERIC_SECRET_KEY_GEN);
    ExportConstant(CKM_CONCATENATE_BASE_AND_KEY);
    ExportConstant(CKM_CONCATENATE_BASE_AND_DATA);
    ExportConstant(CKM_CONCATENATE_DATA_AND_BASE);
    ExportConstant(CKM_XOR_BASE_AND_DATA);
    ExportConstant(CKM_EXTRACT_KEY_FROM_KEY);
    ExportConstant(CKM_SSL3_PRE_MASTER_KEY_GEN);
    ExportConstant(CKM_SSL3_MASTER_KEY_DERIVE);
    ExportConstant(CKM_SSL3_KEY_AND_MAC_DERIVE);

    /* CKM_SSL3_MASTER_KEY_DERIVE_DH, CKM_TLS_PRE_MASTER_KEY_GEN,
     * CKM_TLS_MASTER_KEY_DERIVE, CKM_TLS_KEY_AND_MAC_DERIVE, and
     * CKM_TLS_MASTER_KEY_DERIVE_DH are new for v2.11 */
    ExportConstant(CKM_SSL3_MASTER_KEY_DERIVE_DH);
    ExportConstant(CKM_TLS_PRE_MASTER_KEY_GEN);
    ExportConstant(CKM_TLS_MASTER_KEY_DERIVE);
    ExportConstant(CKM_TLS_KEY_AND_MAC_DERIVE);
    ExportConstant(CKM_TLS_MASTER_KEY_DERIVE_DH);

    /* CKM_TLS_PRF is new for v2.20 */
    ExportConstant(CKM_TLS_PRF);

    ExportConstant(CKM_SSL3_MD5_MAC);
    ExportConstant(CKM_SSL3_SHA1_MAC);
    ExportConstant(CKM_MD5_KEY_DERIVATION);
    ExportConstant(CKM_MD2_KEY_DERIVATION);
    ExportConstant(CKM_SHA1_KEY_DERIVATION);

    /* CKM_SHA256/384/512 are new for v2.20 */
    ExportConstant(CKM_SHA256_KEY_DERIVATION);
    ExportConstant(CKM_SHA384_KEY_DERIVATION);
    ExportConstant(CKM_SHA512_KEY_DERIVATION);

    /* CKM_SHA224 new for v2.20 amendment 3 */
    ExportConstant(CKM_SHA224_KEY_DERIVATION);

    ExportConstant(CKM_PBE_MD2_DES_CBC);
    ExportConstant(CKM_PBE_MD5_DES_CBC);
    ExportConstant(CKM_PBE_MD5_CAST_CBC);
    ExportConstant(CKM_PBE_MD5_CAST3_CBC);
    ExportConstant(CKM_PBE_MD5_CAST5_CBC);
    ExportConstant(CKM_PBE_MD5_CAST128_CBC);
    ExportConstant(CKM_PBE_SHA1_CAST5_CBC);
    ExportConstant(CKM_PBE_SHA1_CAST128_CBC);
    ExportConstant(CKM_PBE_SHA1_RC4_128);
    ExportConstant(CKM_PBE_SHA1_RC4_40);
    ExportConstant(CKM_PBE_SHA1_DES3_EDE_CBC);
    ExportConstant(CKM_PBE_SHA1_DES2_EDE_CBC);
    ExportConstant(CKM_PBE_SHA1_RC2_128_CBC);
    ExportConstant(CKM_PBE_SHA1_RC2_40_CBC);

    /* CKM_PKCS5_PBKD2 is new for v2.10 */
    ExportConstant(CKM_PKCS5_PBKD2);

    ExportConstant(CKM_PBA_SHA1_WITH_SHA1_HMAC);

    /* WTLS mechanisms are new for v2.20 */
    ExportConstant(CKM_WTLS_PRE_MASTER_KEY_GEN);
    ExportConstant(CKM_WTLS_MASTER_KEY_DERIVE);
    ExportConstant(CKM_WTLS_MASTER_KEY_DERIVE_DH_ECC);
    ExportConstant(CKM_WTLS_PRF);
    ExportConstant(CKM_WTLS_SERVER_KEY_AND_MAC_DERIVE);
    ExportConstant(CKM_WTLS_CLIENT_KEY_AND_MAC_DERIVE);

    ExportConstant(CKM_KEY_WRAP_LYNKS);
    ExportConstant(CKM_KEY_WRAP_SET_OAEP);

    /* CKM_CMS_SIG is new for v2.20 */
    ExportConstant(CKM_CMS_SIG);

    /* Fortezza mechanisms */
    ExportConstant(CKM_SKIPJACK_KEY_GEN);
    ExportConstant(CKM_SKIPJACK_ECB64);
    ExportConstant(CKM_SKIPJACK_CBC64);
    ExportConstant(CKM_SKIPJACK_OFB64);
    ExportConstant(CKM_SKIPJACK_CFB64);
    ExportConstant(CKM_SKIPJACK_CFB32);
    ExportConstant(CKM_SKIPJACK_CFB16);
    ExportConstant(CKM_SKIPJACK_CFB8);
    ExportConstant(CKM_SKIPJACK_WRAP);
    ExportConstant(CKM_SKIPJACK_PRIVATE_WRAP);
    ExportConstant(CKM_SKIPJACK_RELAYX);
    ExportConstant(CKM_KEA_KEY_PAIR_GEN);
    ExportConstant(CKM_KEA_KEY_DERIVE);
    ExportConstant(CKM_FORTEZZA_TIMESTAMP);
    ExportConstant(CKM_BATON_KEY_GEN);
    ExportConstant(CKM_BATON_ECB128);
    ExportConstant(CKM_BATON_ECB96);
    ExportConstant(CKM_BATON_CBC128);
    ExportConstant(CKM_BATON_COUNTER);
    ExportConstant(CKM_BATON_SHUFFLE);
    ExportConstant(CKM_BATON_WRAP);

    /* CKM_ECDSA_KEY_PAIR_GEN is deprecated in v2.11,
     * CKM_EC_KEY_PAIR_GEN is preferred */
    ExportConstant(CKM_ECDSA_KEY_PAIR_GEN);
    ExportConstant(CKM_EC_KEY_PAIR_GEN);

    ExportConstant(CKM_ECDSA);
    ExportConstant(CKM_ECDSA_SHA1);

    /* CKM_ECDH1_DERIVE, CKM_ECDH1_COFACTOR_DERIVE, and CKM_ECMQV_DERIVE
     * are new for v2.11 */
    ExportConstant(CKM_ECDH1_DERIVE);
    ExportConstant(CKM_ECDH1_COFACTOR_DERIVE);
    ExportConstant(CKM_ECMQV_DERIVE);

    ExportConstant(CKM_JUNIPER_KEY_GEN);
    ExportConstant(CKM_JUNIPER_ECB128);
    ExportConstant(CKM_JUNIPER_CBC128);
    ExportConstant(CKM_JUNIPER_COUNTER);
    ExportConstant(CKM_JUNIPER_SHUFFLE);
    ExportConstant(CKM_JUNIPER_WRAP);
    ExportConstant(CKM_FASTHASH);

    /* CKM_AES_KEY_GEN, CKM_AES_ECB, CKM_AES_CBC, CKM_AES_MAC,
     * CKM_AES_MAC_GENERAL, CKM_AES_CBC_PAD, CKM_DSA_PARAMETER_GEN,
     * CKM_DH_PKCS_PARAMETER_GEN, and CKM_X9_42_DH_PARAMETER_GEN are
     * new for v2.11 */
    ExportConstant(CKM_AES_KEY_GEN);
    ExportConstant(CKM_AES_ECB);
    ExportConstant(CKM_AES_CBC);
    ExportConstant(CKM_AES_MAC);
    ExportConstant(CKM_AES_MAC_GENERAL);
    ExportConstant(CKM_AES_CBC_PAD);

    /* BlowFish and TwoFish are new for v2.20 */
    ExportConstant(CKM_BLOWFISH_KEY_GEN);
    ExportConstant(CKM_BLOWFISH_CBC);
    ExportConstant(CKM_TWOFISH_KEY_GEN);
    ExportConstant(CKM_TWOFISH_CBC);

    /* Camellia is proposed for v2.20 Amendment 3 */
    ExportConstant(CKM_CAMELLIA_KEY_GEN);
    ExportConstant(CKM_CAMELLIA_ECB);
    ExportConstant(CKM_CAMELLIA_CBC);
    ExportConstant(CKM_CAMELLIA_MAC);
    ExportConstant(CKM_CAMELLIA_MAC_GENERAL);
    ExportConstant(CKM_CAMELLIA_CBC_PAD);
    ExportConstant(CKM_CAMELLIA_ECB_ENCRYPT_DATA);
    ExportConstant(CKM_CAMELLIA_CBC_ENCRYPT_DATA);

    ExportConstant(CKM_SEED_KEY_GEN);
    ExportConstant(CKM_SEED_ECB);
    ExportConstant(CKM_SEED_CBC);
    ExportConstant(CKM_SEED_MAC);
    ExportConstant(CKM_SEED_MAC_GENERAL);
    ExportConstant(CKM_SEED_CBC_PAD);
    ExportConstant(CKM_SEED_ECB_ENCRYPT_DATA);
    ExportConstant(CKM_SEED_CBC_ENCRYPT_DATA);

    /* CKM_xxx_ENCRYPT_DATA mechanisms are new for v2.20 */
    ExportConstant(CKM_DES_ECB_ENCRYPT_DATA);
    ExportConstant(CKM_DES_CBC_ENCRYPT_DATA);
    ExportConstant(CKM_DES3_ECB_ENCRYPT_DATA);
    ExportConstant(CKM_DES3_CBC_ENCRYPT_DATA);
    ExportConstant(CKM_AES_ECB_ENCRYPT_DATA);
    ExportConstant(CKM_AES_CBC_ENCRYPT_DATA);

    ExportConstant(CKM_DSA_PARAMETER_GEN);
    ExportConstant(CKM_DH_PKCS_PARAMETER_GEN);
    ExportConstant(CKM_X9_42_DH_PARAMETER_GEN);

#undef ExportConstant

    /***************************************************************************
     * Attribute Types
     ***************************************************************************/
    if ((cka_name_to_value = PyDict_New()) == NULL) {
        return;
    }
    if ((cka_value_to_name = PyDict_New()) == NULL) {
        return;
    }

#define ExportConstant(constant)                      \
if (_AddIntConstantWithLookup(m, #constant, constant, \
    "CKA_", cka_name_to_value, cka_value_to_name) < 0) return;

    /* The following attribute types are defined: */
    ExportConstant(CKA_CLASS);
    ExportConstant(CKA_TOKEN);
    ExportConstant(CKA_PRIVATE);
    ExportConstant(CKA_LABEL);
    ExportConstant(CKA_APPLICATION);
    ExportConstant(CKA_VALUE);

    /* CKA_OBJECT_ID is new for v2.10 */
    ExportConstant(CKA_OBJECT_ID);

    ExportConstant(CKA_CERTIFICATE_TYPE);
    ExportConstant(CKA_ISSUER);
    ExportConstant(CKA_SERIAL_NUMBER);

    /* CKA_AC_ISSUER, CKA_OWNER, and CKA_ATTR_TYPES are new for v2.10 */
    ExportConstant(CKA_AC_ISSUER);
    ExportConstant(CKA_OWNER);
    ExportConstant(CKA_ATTR_TYPES);

    /* CKA_TRUSTED is new for v2.11 */
    ExportConstant(CKA_TRUSTED);

    /* CKA_CERTIFICATE_CATEGORY ...
     * CKA_CHECK_VALUE are new for v2.20 */
    ExportConstant(CKA_CERTIFICATE_CATEGORY);
    ExportConstant(CKA_JAVA_MIDP_SECURITY_DOMAIN);
    ExportConstant(CKA_URL);
    ExportConstant(CKA_HASH_OF_SUBJECT_PUBLIC_KEY);
    ExportConstant(CKA_HASH_OF_ISSUER_PUBLIC_KEY);
    ExportConstant(CKA_CHECK_VALUE);

    ExportConstant(CKA_KEY_TYPE);
    ExportConstant(CKA_SUBJECT);
    ExportConstant(CKA_ID);
    ExportConstant(CKA_SENSITIVE);
    ExportConstant(CKA_ENCRYPT);
    ExportConstant(CKA_DECRYPT);
    ExportConstant(CKA_WRAP);
    ExportConstant(CKA_UNWRAP);
    ExportConstant(CKA_SIGN);
    ExportConstant(CKA_SIGN_RECOVER);
    ExportConstant(CKA_VERIFY);
    ExportConstant(CKA_VERIFY_RECOVER);
    ExportConstant(CKA_DERIVE);
    ExportConstant(CKA_START_DATE);
    ExportConstant(CKA_END_DATE);
    ExportConstant(CKA_MODULUS);
    ExportConstant(CKA_MODULUS_BITS);
    ExportConstant(CKA_PUBLIC_EXPONENT);
    ExportConstant(CKA_PRIVATE_EXPONENT);
    ExportConstant(CKA_PRIME_1);
    ExportConstant(CKA_PRIME_2);
    ExportConstant(CKA_EXPONENT_1);
    ExportConstant(CKA_EXPONENT_2);
    ExportConstant(CKA_COEFFICIENT);
    ExportConstant(CKA_PRIME);
    ExportConstant(CKA_SUBPRIME);
    ExportConstant(CKA_BASE);

    /* CKA_PRIME_BITS and CKA_SUB_PRIME_BITS are new for v2.11 */
    ExportConstant(CKA_PRIME_BITS);
    ExportConstant(CKA_SUBPRIME_BITS);
    ExportConstant(CKA_SUB_PRIME_BITS);
    /* (To retain backwards-compatibility) */

    ExportConstant(CKA_VALUE_BITS);
    ExportConstant(CKA_VALUE_LEN);

    /* CKA_EXTRACTABLE, CKA_LOCAL, CKA_NEVER_EXTRACTABLE,
     * CKA_ALWAYS_SENSITIVE, CKA_MODIFIABLE, CKA_ECDSA_PARAMS,
     * and CKA_EC_POINT are new for v2.0 */
    ExportConstant(CKA_EXTRACTABLE);
    ExportConstant(CKA_LOCAL);
    ExportConstant(CKA_NEVER_EXTRACTABLE);
    ExportConstant(CKA_ALWAYS_SENSITIVE);

    /* CKA_KEY_GEN_MECHANISM is new for v2.11 */
    ExportConstant(CKA_KEY_GEN_MECHANISM);

    ExportConstant(CKA_MODIFIABLE);

    /* CKA_ECDSA_PARAMS is deprecated in v2.11,
     * CKA_EC_PARAMS is preferred. */
    ExportConstant(CKA_ECDSA_PARAMS);
    ExportConstant(CKA_EC_PARAMS);

    ExportConstant(CKA_EC_POINT);

    /* CKA_SECONDARY_AUTH, CKA_AUTH_PIN_FLAGS,
     * are new for v2.10. Deprecated in v2.11 and onwards. */
    ExportConstant(CKA_SECONDARY_AUTH);
    ExportConstant(CKA_AUTH_PIN_FLAGS);

    /* CKA_ALWAYS_AUTHENTICATE ...
     * CKA_UNWRAP_TEMPLATE are new for v2.20 */
    ExportConstant(CKA_ALWAYS_AUTHENTICATE);

    ExportConstant(CKA_WRAP_WITH_TRUSTED);
    ExportConstant(CKA_WRAP_TEMPLATE);
    ExportConstant(CKA_UNWRAP_TEMPLATE);

    /* CKA_HW_FEATURE_TYPE, CKA_RESET_ON_INIT, and CKA_HAS_RESET
     * are new for v2.10 */
    ExportConstant(CKA_HW_FEATURE_TYPE);
    ExportConstant(CKA_RESET_ON_INIT);
    ExportConstant(CKA_HAS_RESET);

    /* The following attributes are new for v2.20 */
    ExportConstant(CKA_PIXEL_X);
    ExportConstant(CKA_PIXEL_Y);
    ExportConstant(CKA_RESOLUTION);
    ExportConstant(CKA_CHAR_ROWS);
    ExportConstant(CKA_CHAR_COLUMNS);
    ExportConstant(CKA_COLOR);
    ExportConstant(CKA_BITS_PER_PIXEL);
    ExportConstant(CKA_CHAR_SETS);
    ExportConstant(CKA_ENCODING_METHODS);
    ExportConstant(CKA_MIME_TYPES);
    ExportConstant(CKA_MECHANISM_TYPE);
    ExportConstant(CKA_REQUIRED_CMS_ATTRIBUTES);
    ExportConstant(CKA_DEFAULT_CMS_ATTRIBUTES);
    ExportConstant(CKA_SUPPORTED_CMS_ATTRIBUTES);
    ExportConstant(CKA_ALLOWED_MECHANISMS);

    ExportConstant(CKA_VENDOR_DEFINED);

#undef ExportConstant

    /***************************************************************************
     * SEC OID TAGS
     ***************************************************************************/

    if ((sec_oid_name_to_value = PyDict_New()) == NULL) {
        return;
    }
    if ((sec_oid_value_to_name = PyDict_New()) == NULL) {
        return;
    }

#define ExportConstant(constant)                      \
if (_AddIntConstantWithLookup(m, #constant, constant, \
    "SEC_OID_", sec_oid_name_to_value, sec_oid_value_to_name) < 0) return;

    ExportConstant(SEC_OID_UNKNOWN);
    ExportConstant(SEC_OID_MD2);
    ExportConstant(SEC_OID_MD4);
    ExportConstant(SEC_OID_MD5);
    ExportConstant(SEC_OID_SHA1);
    ExportConstant(SEC_OID_RC2_CBC);
    ExportConstant(SEC_OID_RC4);
    ExportConstant(SEC_OID_DES_EDE3_CBC);
    ExportConstant(SEC_OID_RC5_CBC_PAD);
    ExportConstant(SEC_OID_DES_ECB);
    ExportConstant(SEC_OID_DES_CBC);
    ExportConstant(SEC_OID_DES_OFB);
    ExportConstant(SEC_OID_DES_CFB);
    ExportConstant(SEC_OID_DES_MAC);
    ExportConstant(SEC_OID_DES_EDE);
    ExportConstant(SEC_OID_ISO_SHA_WITH_RSA_SIGNATURE);
    ExportConstant(SEC_OID_PKCS1_RSA_ENCRYPTION);
    ExportConstant(SEC_OID_PKCS1_MD2_WITH_RSA_ENCRYPTION);
    ExportConstant(SEC_OID_PKCS1_MD4_WITH_RSA_ENCRYPTION);
    ExportConstant(SEC_OID_PKCS1_MD5_WITH_RSA_ENCRYPTION);
    ExportConstant(SEC_OID_PKCS1_SHA1_WITH_RSA_ENCRYPTION);
    ExportConstant(SEC_OID_PKCS5_PBE_WITH_MD2_AND_DES_CBC);
    ExportConstant(SEC_OID_PKCS5_PBE_WITH_MD5_AND_DES_CBC);
    ExportConstant(SEC_OID_PKCS5_PBE_WITH_SHA1_AND_DES_CBC);
    ExportConstant(SEC_OID_PKCS7);
    ExportConstant(SEC_OID_PKCS7_DATA);
    ExportConstant(SEC_OID_PKCS7_SIGNED_DATA);
    ExportConstant(SEC_OID_PKCS7_ENVELOPED_DATA);
    ExportConstant(SEC_OID_PKCS7_SIGNED_ENVELOPED_DATA);
    ExportConstant(SEC_OID_PKCS7_DIGESTED_DATA);
    ExportConstant(SEC_OID_PKCS7_ENCRYPTED_DATA);
    ExportConstant(SEC_OID_PKCS9_EMAIL_ADDRESS);
    ExportConstant(SEC_OID_PKCS9_UNSTRUCTURED_NAME);
    ExportConstant(SEC_OID_PKCS9_CONTENT_TYPE);
    ExportConstant(SEC_OID_PKCS9_MESSAGE_DIGEST);
    ExportConstant(SEC_OID_PKCS9_SIGNING_TIME);
    ExportConstant(SEC_OID_PKCS9_COUNTER_SIGNATURE);
    ExportConstant(SEC_OID_PKCS9_CHALLENGE_PASSWORD);
    ExportConstant(SEC_OID_PKCS9_UNSTRUCTURED_ADDRESS);
    ExportConstant(SEC_OID_PKCS9_EXTENDED_CERTIFICATE_ATTRIBUTES);
    ExportConstant(SEC_OID_PKCS9_SMIME_CAPABILITIES);
    ExportConstant(SEC_OID_AVA_COMMON_NAME);
    ExportConstant(SEC_OID_AVA_COUNTRY_NAME);
    ExportConstant(SEC_OID_AVA_LOCALITY);
    ExportConstant(SEC_OID_AVA_STATE_OR_PROVINCE);
    ExportConstant(SEC_OID_AVA_ORGANIZATION_NAME);
    ExportConstant(SEC_OID_AVA_ORGANIZATIONAL_UNIT_NAME);
    ExportConstant(SEC_OID_AVA_DN_QUALIFIER);
    ExportConstant(SEC_OID_AVA_DC);

    ExportConstant(SEC_OID_NS_TYPE_GIF);
    ExportConstant(SEC_OID_NS_TYPE_JPEG);
    ExportConstant(SEC_OID_NS_TYPE_URL);
    ExportConstant(SEC_OID_NS_TYPE_HTML);
    ExportConstant(SEC_OID_NS_TYPE_CERT_SEQUENCE);
    ExportConstant(SEC_OID_MISSI_KEA_DSS_OLD);
    ExportConstant(SEC_OID_MISSI_DSS_OLD);
    ExportConstant(SEC_OID_MISSI_KEA_DSS);
    ExportConstant(SEC_OID_MISSI_DSS);
    ExportConstant(SEC_OID_MISSI_KEA);
    ExportConstant(SEC_OID_MISSI_ALT_KEA);

    /* Netscape private certificate extensions */
    ExportConstant(SEC_OID_NS_CERT_EXT_NETSCAPE_OK);
    ExportConstant(SEC_OID_NS_CERT_EXT_ISSUER_LOGO);
    ExportConstant(SEC_OID_NS_CERT_EXT_SUBJECT_LOGO);
    ExportConstant(SEC_OID_NS_CERT_EXT_CERT_TYPE);
    ExportConstant(SEC_OID_NS_CERT_EXT_BASE_URL);
    ExportConstant(SEC_OID_NS_CERT_EXT_REVOCATION_URL);
    ExportConstant(SEC_OID_NS_CERT_EXT_CA_REVOCATION_URL);
    ExportConstant(SEC_OID_NS_CERT_EXT_CA_CRL_URL);
    ExportConstant(SEC_OID_NS_CERT_EXT_CA_CERT_URL);
    ExportConstant(SEC_OID_NS_CERT_EXT_CERT_RENEWAL_URL);
    ExportConstant(SEC_OID_NS_CERT_EXT_CA_POLICY_URL);
    ExportConstant(SEC_OID_NS_CERT_EXT_HOMEPAGE_URL);
    ExportConstant(SEC_OID_NS_CERT_EXT_ENTITY_LOGO);
    ExportConstant(SEC_OID_NS_CERT_EXT_USER_PICTURE);
    ExportConstant(SEC_OID_NS_CERT_EXT_SSL_SERVER_NAME);
    ExportConstant(SEC_OID_NS_CERT_EXT_COMMENT);
    ExportConstant(SEC_OID_NS_CERT_EXT_LOST_PASSWORD_URL);
    ExportConstant(SEC_OID_NS_CERT_EXT_CERT_RENEWAL_TIME);
    ExportConstant(SEC_OID_NS_KEY_USAGE_GOVT_APPROVED);

    /* x.509 v3 Extensions */
    ExportConstant(SEC_OID_X509_SUBJECT_DIRECTORY_ATTR);
    ExportConstant(SEC_OID_X509_SUBJECT_KEY_ID);
    ExportConstant(SEC_OID_X509_KEY_USAGE);
    ExportConstant(SEC_OID_X509_PRIVATE_KEY_USAGE_PERIOD);
    ExportConstant(SEC_OID_X509_SUBJECT_ALT_NAME);
    ExportConstant(SEC_OID_X509_ISSUER_ALT_NAME);
    ExportConstant(SEC_OID_X509_BASIC_CONSTRAINTS);
    ExportConstant(SEC_OID_X509_NAME_CONSTRAINTS);
    ExportConstant(SEC_OID_X509_CRL_DIST_POINTS);
    ExportConstant(SEC_OID_X509_CERTIFICATE_POLICIES);
    ExportConstant(SEC_OID_X509_POLICY_MAPPINGS);
    ExportConstant(SEC_OID_X509_POLICY_CONSTRAINTS);
    ExportConstant(SEC_OID_X509_AUTH_KEY_ID);
    ExportConstant(SEC_OID_X509_EXT_KEY_USAGE);
    ExportConstant(SEC_OID_X509_AUTH_INFO_ACCESS);

    ExportConstant(SEC_OID_X509_CRL_NUMBER);
    ExportConstant(SEC_OID_X509_REASON_CODE);
    ExportConstant(SEC_OID_X509_INVALID_DATE);
    /* End of x.509 v3 Extensions */

    ExportConstant(SEC_OID_X500_RSA_ENCRYPTION);

    /* alg 1485 additions */
    ExportConstant(SEC_OID_RFC1274_UID);
    ExportConstant(SEC_OID_RFC1274_MAIL);

    /* PKCS 12 additions */
    ExportConstant(SEC_OID_PKCS12);
    ExportConstant(SEC_OID_PKCS12_MODE_IDS);
    ExportConstant(SEC_OID_PKCS12_ESPVK_IDS);
    ExportConstant(SEC_OID_PKCS12_BAG_IDS);
    ExportConstant(SEC_OID_PKCS12_CERT_BAG_IDS);
    ExportConstant(SEC_OID_PKCS12_OIDS);
    ExportConstant(SEC_OID_PKCS12_PBE_IDS);
    ExportConstant(SEC_OID_PKCS12_SIGNATURE_IDS);
    ExportConstant(SEC_OID_PKCS12_ENVELOPING_IDS);
   /* SEC_OID_PKCS12_OFFLINE_TRANSPORT_MODE,
    SEC_OID_PKCS12_ONLINE_TRANSPORT_MODE, */
    ExportConstant(SEC_OID_PKCS12_PKCS8_KEY_SHROUDING);
    ExportConstant(SEC_OID_PKCS12_KEY_BAG_ID);
    ExportConstant(SEC_OID_PKCS12_CERT_AND_CRL_BAG_ID);
    ExportConstant(SEC_OID_PKCS12_SECRET_BAG_ID);
    ExportConstant(SEC_OID_PKCS12_X509_CERT_CRL_BAG);
    ExportConstant(SEC_OID_PKCS12_SDSI_CERT_BAG);
    ExportConstant(SEC_OID_PKCS12_PBE_WITH_SHA1_AND_128_BIT_RC4);
    ExportConstant(SEC_OID_PKCS12_PBE_WITH_SHA1_AND_40_BIT_RC4);
    ExportConstant(SEC_OID_PKCS12_PBE_WITH_SHA1_AND_TRIPLE_DES_CBC);
    ExportConstant(SEC_OID_PKCS12_PBE_WITH_SHA1_AND_128_BIT_RC2_CBC);
    ExportConstant(SEC_OID_PKCS12_PBE_WITH_SHA1_AND_40_BIT_RC2_CBC);
    ExportConstant(SEC_OID_PKCS12_RSA_ENCRYPTION_WITH_128_BIT_RC4);
    ExportConstant(SEC_OID_PKCS12_RSA_ENCRYPTION_WITH_40_BIT_RC4);
    ExportConstant(SEC_OID_PKCS12_RSA_ENCRYPTION_WITH_TRIPLE_DES);
    ExportConstant(SEC_OID_PKCS12_RSA_SIGNATURE_WITH_SHA1_DIGEST);
    /* end of PKCS 12 additions */

    /* DSA signatures */
    ExportConstant(SEC_OID_ANSIX9_DSA_SIGNATURE);
    ExportConstant(SEC_OID_ANSIX9_DSA_SIGNATURE_WITH_SHA1_DIGEST);
    ExportConstant(SEC_OID_BOGUS_DSA_SIGNATURE_WITH_SHA1_DIGEST);

    /* Verisign OIDs */
    ExportConstant(SEC_OID_VERISIGN_USER_NOTICES);

    /* PKIX OIDs */
    ExportConstant(SEC_OID_PKIX_CPS_POINTER_QUALIFIER);
    ExportConstant(SEC_OID_PKIX_USER_NOTICE_QUALIFIER);
    ExportConstant(SEC_OID_PKIX_OCSP);
    ExportConstant(SEC_OID_PKIX_OCSP_BASIC_RESPONSE);
    ExportConstant(SEC_OID_PKIX_OCSP_NONCE);
    ExportConstant(SEC_OID_PKIX_OCSP_CRL);
    ExportConstant(SEC_OID_PKIX_OCSP_RESPONSE);
    ExportConstant(SEC_OID_PKIX_OCSP_NO_CHECK);
    ExportConstant(SEC_OID_PKIX_OCSP_ARCHIVE_CUTOFF);
    ExportConstant(SEC_OID_PKIX_OCSP_SERVICE_LOCATOR);
    ExportConstant(SEC_OID_PKIX_REGCTRL_REGTOKEN);
    ExportConstant(SEC_OID_PKIX_REGCTRL_AUTHENTICATOR);
    ExportConstant(SEC_OID_PKIX_REGCTRL_PKIPUBINFO);
    ExportConstant(SEC_OID_PKIX_REGCTRL_PKI_ARCH_OPTIONS);
    ExportConstant(SEC_OID_PKIX_REGCTRL_OLD_CERT_ID);
    ExportConstant(SEC_OID_PKIX_REGCTRL_PROTOCOL_ENC_KEY);
    ExportConstant(SEC_OID_PKIX_REGINFO_UTF8_PAIRS);
    ExportConstant(SEC_OID_PKIX_REGINFO_CERT_REQUEST);
    ExportConstant(SEC_OID_EXT_KEY_USAGE_SERVER_AUTH);
    ExportConstant(SEC_OID_EXT_KEY_USAGE_CLIENT_AUTH);
    ExportConstant(SEC_OID_EXT_KEY_USAGE_CODE_SIGN);
    ExportConstant(SEC_OID_EXT_KEY_USAGE_EMAIL_PROTECT);
    ExportConstant(SEC_OID_EXT_KEY_USAGE_TIME_STAMP);
    ExportConstant(SEC_OID_OCSP_RESPONDER);

    /* Netscape Algorithm OIDs */
    ExportConstant(SEC_OID_NETSCAPE_SMIME_KEA);

    /* Skipjack OID -- ### mwelch temporary */
    ExportConstant(SEC_OID_FORTEZZA_SKIPJACK);

    /* PKCS 12 V2 oids */
    ExportConstant(SEC_OID_PKCS12_V2_PBE_WITH_SHA1_AND_128_BIT_RC4);
    ExportConstant(SEC_OID_PKCS12_V2_PBE_WITH_SHA1_AND_40_BIT_RC4);
    ExportConstant(SEC_OID_PKCS12_V2_PBE_WITH_SHA1_AND_3KEY_TRIPLE_DES_CBC);
    ExportConstant(SEC_OID_PKCS12_V2_PBE_WITH_SHA1_AND_2KEY_TRIPLE_DES_CBC);
    ExportConstant(SEC_OID_PKCS12_V2_PBE_WITH_SHA1_AND_128_BIT_RC2_CBC);
    ExportConstant(SEC_OID_PKCS12_V2_PBE_WITH_SHA1_AND_40_BIT_RC2_CBC);
    ExportConstant(SEC_OID_PKCS12_SAFE_CONTENTS_ID);
    ExportConstant(SEC_OID_PKCS12_PKCS8_SHROUDED_KEY_BAG_ID);

    ExportConstant(SEC_OID_PKCS12_V1_KEY_BAG_ID);
    ExportConstant(SEC_OID_PKCS12_V1_PKCS8_SHROUDED_KEY_BAG_ID);
    ExportConstant(SEC_OID_PKCS12_V1_CERT_BAG_ID);
    ExportConstant(SEC_OID_PKCS12_V1_CRL_BAG_ID);
    ExportConstant(SEC_OID_PKCS12_V1_SECRET_BAG_ID);
    ExportConstant(SEC_OID_PKCS12_V1_SAFE_CONTENTS_BAG_ID);
    ExportConstant(SEC_OID_PKCS9_X509_CERT);
    ExportConstant(SEC_OID_PKCS9_SDSI_CERT);
    ExportConstant(SEC_OID_PKCS9_X509_CRL);
    ExportConstant(SEC_OID_PKCS9_FRIENDLY_NAME);
    ExportConstant(SEC_OID_PKCS9_LOCAL_KEY_ID);
    ExportConstant(SEC_OID_BOGUS_KEY_USAGE);

    /*Diffe Helman OIDS */
    ExportConstant(SEC_OID_X942_DIFFIE_HELMAN_KEY);

    /* Netscape other name types */
    ExportConstant(SEC_OID_NETSCAPE_NICKNAME);

    /* Cert Server OIDS */
    ExportConstant(SEC_OID_NETSCAPE_RECOVERY_REQUEST);

    /* New PSM certificate management OIDs */
    ExportConstant(SEC_OID_CERT_RENEWAL_LOCATOR);
    ExportConstant(SEC_OID_NS_CERT_EXT_SCOPE_OF_USE);

    /* CMS (RFC2630) OIDs */
    ExportConstant(SEC_OID_CMS_EPHEMERAL_STATIC_DIFFIE_HELLMAN);
    ExportConstant(SEC_OID_CMS_3DES_KEY_WRAP);
    ExportConstant(SEC_OID_CMS_RC2_KEY_WRAP);

    /* SMIME attributes */
    ExportConstant(SEC_OID_SMIME_ENCRYPTION_KEY_PREFERENCE);

    /* AES OIDs */
    ExportConstant(SEC_OID_AES_128_ECB);
    ExportConstant(SEC_OID_AES_128_CBC);
    ExportConstant(SEC_OID_AES_192_ECB);
    ExportConstant(SEC_OID_AES_192_CBC);
    ExportConstant(SEC_OID_AES_256_ECB);
    ExportConstant(SEC_OID_AES_256_CBC);

    ExportConstant(SEC_OID_SDN702_DSA_SIGNATURE);

    ExportConstant(SEC_OID_MS_SMIME_ENCRYPTION_KEY_PREFERENCE);

    ExportConstant(SEC_OID_SHA256);
    ExportConstant(SEC_OID_SHA384);
    ExportConstant(SEC_OID_SHA512);

    ExportConstant(SEC_OID_PKCS1_SHA256_WITH_RSA_ENCRYPTION);
    ExportConstant(SEC_OID_PKCS1_SHA384_WITH_RSA_ENCRYPTION);
    ExportConstant(SEC_OID_PKCS1_SHA512_WITH_RSA_ENCRYPTION);

    ExportConstant(SEC_OID_AES_128_KEY_WRAP);
    ExportConstant(SEC_OID_AES_192_KEY_WRAP);
    ExportConstant(SEC_OID_AES_256_KEY_WRAP);

    /* Elliptic Curve Cryptography (ECC) OIDs */
    ExportConstant(SEC_OID_ANSIX962_EC_PUBLIC_KEY);
    ExportConstant(SEC_OID_ANSIX962_ECDSA_SHA1_SIGNATURE);

    ExportConstant(SEC_OID_ANSIX962_ECDSA_SIGNATURE_WITH_SHA1_DIGEST);

    /* ANSI X9.62 named elliptic curves (prime field) */
    ExportConstant(SEC_OID_ANSIX962_EC_PRIME192V1);
    ExportConstant(SEC_OID_ANSIX962_EC_PRIME192V2);
    ExportConstant(SEC_OID_ANSIX962_EC_PRIME192V3);
    ExportConstant(SEC_OID_ANSIX962_EC_PRIME239V1);
    ExportConstant(SEC_OID_ANSIX962_EC_PRIME239V2);
    ExportConstant(SEC_OID_ANSIX962_EC_PRIME239V3);
    ExportConstant(SEC_OID_ANSIX962_EC_PRIME256V1);

    /* SECG named elliptic curves (prime field) */
    ExportConstant(SEC_OID_SECG_EC_SECP112R1);
    ExportConstant(SEC_OID_SECG_EC_SECP112R2);
    ExportConstant(SEC_OID_SECG_EC_SECP128R1);
    ExportConstant(SEC_OID_SECG_EC_SECP128R2);
    ExportConstant(SEC_OID_SECG_EC_SECP160K1);
    ExportConstant(SEC_OID_SECG_EC_SECP160R1);
    ExportConstant(SEC_OID_SECG_EC_SECP160R2);
    ExportConstant(SEC_OID_SECG_EC_SECP192K1);
    /* SEC_OID_SECG_EC_SECP192R1 is SEC_OID_ANSIX962_EC_PRIME192V1 */
    ExportConstant(SEC_OID_SECG_EC_SECP224K1);
    ExportConstant(SEC_OID_SECG_EC_SECP224R1);
    ExportConstant(SEC_OID_SECG_EC_SECP256K1);
    /* SEC_OID_SECG_EC_SECP256R1 is SEC_OID_ANSIX962_EC_PRIME256V1 */
    ExportConstant(SEC_OID_SECG_EC_SECP384R1);
    ExportConstant(SEC_OID_SECG_EC_SECP521R1);

    /* ANSI X9.62 named elliptic curves (characteristic two field) */
    ExportConstant(SEC_OID_ANSIX962_EC_C2PNB163V1);
    ExportConstant(SEC_OID_ANSIX962_EC_C2PNB163V2);
    ExportConstant(SEC_OID_ANSIX962_EC_C2PNB163V3);
    ExportConstant(SEC_OID_ANSIX962_EC_C2PNB176V1);
    ExportConstant(SEC_OID_ANSIX962_EC_C2TNB191V1);
    ExportConstant(SEC_OID_ANSIX962_EC_C2TNB191V2);
    ExportConstant(SEC_OID_ANSIX962_EC_C2TNB191V3);
    ExportConstant(SEC_OID_ANSIX962_EC_C2ONB191V4);
    ExportConstant(SEC_OID_ANSIX962_EC_C2ONB191V5);
    ExportConstant(SEC_OID_ANSIX962_EC_C2PNB208W1);
    ExportConstant(SEC_OID_ANSIX962_EC_C2TNB239V1);
    ExportConstant(SEC_OID_ANSIX962_EC_C2TNB239V2);
    ExportConstant(SEC_OID_ANSIX962_EC_C2TNB239V3);
    ExportConstant(SEC_OID_ANSIX962_EC_C2ONB239V4);
    ExportConstant(SEC_OID_ANSIX962_EC_C2ONB239V5);
    ExportConstant(SEC_OID_ANSIX962_EC_C2PNB272W1);
    ExportConstant(SEC_OID_ANSIX962_EC_C2PNB304W1);
    ExportConstant(SEC_OID_ANSIX962_EC_C2TNB359V1);
    ExportConstant(SEC_OID_ANSIX962_EC_C2PNB368W1);
    ExportConstant(SEC_OID_ANSIX962_EC_C2TNB431R1);

    /* SECG named elliptic curves (characteristic two field) */
    ExportConstant(SEC_OID_SECG_EC_SECT113R1);
    ExportConstant(SEC_OID_SECG_EC_SECT113R2);
    ExportConstant(SEC_OID_SECG_EC_SECT131R1);
    ExportConstant(SEC_OID_SECG_EC_SECT131R2);
    ExportConstant(SEC_OID_SECG_EC_SECT163K1);
    ExportConstant(SEC_OID_SECG_EC_SECT163R1);
    ExportConstant(SEC_OID_SECG_EC_SECT163R2);
    ExportConstant(SEC_OID_SECG_EC_SECT193R1);
    ExportConstant(SEC_OID_SECG_EC_SECT193R2);
    ExportConstant(SEC_OID_SECG_EC_SECT233K1);
    ExportConstant(SEC_OID_SECG_EC_SECT233R1);
    ExportConstant(SEC_OID_SECG_EC_SECT239K1);
    ExportConstant(SEC_OID_SECG_EC_SECT283K1);
    ExportConstant(SEC_OID_SECG_EC_SECT283R1);
    ExportConstant(SEC_OID_SECG_EC_SECT409K1);
    ExportConstant(SEC_OID_SECG_EC_SECT409R1);
    ExportConstant(SEC_OID_SECG_EC_SECT571K1);
    ExportConstant(SEC_OID_SECG_EC_SECT571R1);

    ExportConstant(SEC_OID_NETSCAPE_AOLSCREENNAME);

    ExportConstant(SEC_OID_AVA_SURNAME);
    ExportConstant(SEC_OID_AVA_SERIAL_NUMBER);
    ExportConstant(SEC_OID_AVA_STREET_ADDRESS);
    ExportConstant(SEC_OID_AVA_TITLE);
    ExportConstant(SEC_OID_AVA_POSTAL_ADDRESS);
    ExportConstant(SEC_OID_AVA_POSTAL_CODE);
    ExportConstant(SEC_OID_AVA_POST_OFFICE_BOX);
    ExportConstant(SEC_OID_AVA_GIVEN_NAME);
    ExportConstant(SEC_OID_AVA_INITIALS);
    ExportConstant(SEC_OID_AVA_GENERATION_QUALIFIER);
    ExportConstant(SEC_OID_AVA_HOUSE_IDENTIFIER);
    ExportConstant(SEC_OID_AVA_PSEUDONYM);

    /* More OIDs */
    ExportConstant(SEC_OID_PKIX_CA_ISSUERS);
    ExportConstant(SEC_OID_PKCS9_EXTENSION_REQUEST);

    /* new EC Signature oids */
    ExportConstant(SEC_OID_ANSIX962_ECDSA_SIGNATURE_RECOMMENDED_DIGEST);
    ExportConstant(SEC_OID_ANSIX962_ECDSA_SIGNATURE_SPECIFIED_DIGEST);
    ExportConstant(SEC_OID_ANSIX962_ECDSA_SHA224_SIGNATURE);
    ExportConstant(SEC_OID_ANSIX962_ECDSA_SHA256_SIGNATURE);
    ExportConstant(SEC_OID_ANSIX962_ECDSA_SHA384_SIGNATURE);
    ExportConstant(SEC_OID_ANSIX962_ECDSA_SHA512_SIGNATURE);

    /* More id-ce and id-pe OIDs from RFC 3280 */
    ExportConstant(SEC_OID_X509_HOLD_INSTRUCTION_CODE);
    ExportConstant(SEC_OID_X509_DELTA_CRL_INDICATOR);
    ExportConstant(SEC_OID_X509_ISSUING_DISTRIBUTION_POINT);
    ExportConstant(SEC_OID_X509_CERT_ISSUER);
    ExportConstant(SEC_OID_X509_FRESHEST_CRL);
    ExportConstant(SEC_OID_X509_INHIBIT_ANY_POLICY);
    ExportConstant(SEC_OID_X509_SUBJECT_INFO_ACCESS);

    /* Camellia OIDs (RFC3657)*/
    ExportConstant(SEC_OID_CAMELLIA_128_CBC);
    ExportConstant(SEC_OID_CAMELLIA_192_CBC);
    ExportConstant(SEC_OID_CAMELLIA_256_CBC);

    /* PKCS 5 V2 OIDS */
    ExportConstant(SEC_OID_PKCS5_PBKDF2);
    ExportConstant(SEC_OID_PKCS5_PBES2);
    ExportConstant(SEC_OID_PKCS5_PBMAC1);
    ExportConstant(SEC_OID_HMAC_SHA1);
    ExportConstant(SEC_OID_HMAC_SHA224);
    ExportConstant(SEC_OID_HMAC_SHA256);
    ExportConstant(SEC_OID_HMAC_SHA384);
    ExportConstant(SEC_OID_HMAC_SHA512);

    ExportConstant(SEC_OID_PKIX_TIMESTAMPING);
    ExportConstant(SEC_OID_PKIX_CA_REPOSITORY);

    ExportConstant(SEC_OID_ISO_SHA1_WITH_RSA_SIGNATURE);

    ExportConstant(SEC_OID_SEED_CBC);

    ExportConstant(SEC_OID_X509_ANY_POLICY);

    ExportConstant(SEC_OID_SECG_EC_SECP192R1);
    ExportConstant(SEC_OID_SECG_EC_SECP256R1);
    ExportConstant(SEC_OID_PKCS12_KEY_USAGE);

#undef ExportConstant

    /***************************************************************************
     * PK11Origin
     ***************************************************************************/
    AddIntConstant(PK11_OriginNULL);         /* There is not key, it's a null SymKey */
    AddIntConstant(PK11_OriginDerive);       /* Key was derived from some other key */
    AddIntConstant(PK11_OriginGenerated);    /* Key was generated (also PBE keys) */
    AddIntConstant(PK11_OriginFortezzaHack); /* Key was marked for fortezza hack */
    AddIntConstant(PK11_OriginUnwrap);       /* Key was unwrapped or decrypted */
}

