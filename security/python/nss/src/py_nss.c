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

// FIXME: change all Python _from_ NSS function names to use the NSS typedef for clarity
//        e.g. PrivateKey_new_from_private_key should be PrivateKey_new_from_SECKEYPrivateKey
// FIXME: should we be calling these?
// SECKEY_DestroyEncryptedPrivateKeyInfo
// SECKEY_DestroyPrivateKey	   SECKEY_DestroyPrivateKeyInfo
// SECKEY_DestroyPrivateKeyList	   SECKEY_DestroyPublicKey
// SECKEY_DestroyPublicKeyList	   SECKEY_DestroySubjectPublicKeyInfo

#include "Python.h"
#include "structmember.h"

#include "py_nspr_common.h"
#define NSPR_NSS_MODULE
#include "py_nss.h"
#include "py_nspr_error.h"

#include "secder.h"
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

typedef PyObject *(*format_lines_func)(PyObject *self, PyObject *args, PyObject *kwds);

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
sec_oid_str(SECItem *oid);

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
cert_md5_digest(PyObject *self, PyObject *args);

static PyObject *
cert_sha1_digest(PyObject *self, PyObject *args);

static PyObject *
cert_sha256_digest(PyObject *self, PyObject *args);

static PyObject *
cert_sha512_digest(PyObject *self, PyObject *args);

/* ==================================== */

static char *
key_type_str(KeyType key_type)
{
    switch(key_type) {
    case nullKey:     return "NULL";
    case rsaKey:      return "RSA";
    case dsaKey:      return "DSA";
    case fortezzaKey: return "Fortezza";
    case dhKey:       return "Diffie Helman";
    case keaKey:      return "Key Exchange Algorithm";
    case ecKey:       return "Elliptic Curve";
    }
    return "unknown";
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
        octets_per_line = OCTETS_PER_LINE_DEFAULT;

    if (!separator)
        separator = HEX_SEPARATOR_DEFAULT;

    separator_len = strlen(separator);
    separator_end = separator + separator_len;

    if (octets_per_line <= 0) {
        num_octets = data_len;
        line_size = (num_octets * 2) + ((num_octets-1) * separator_len);

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

static PyObject *
cert_data_to_hex(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"data", "octets_per_line", "separator", NULL};
    PyObject *obj = NULL;
    int octets_per_line = -1;
    char *separator = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|is:cert_data_to_hex", kwlist, 
                                     &obj, &octets_per_line, &separator))
        return NULL; 

    return obj_to_hex(obj, octets_per_line, separator);
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
            if ((hex_str = raw_data_to_hex(tmp.data, tmp.len, 0, NULL))) {
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
	    return raw_data_to_hex(item->data, item->len, 0, NULL);
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
        return raw_data_to_hex(item->data, item->len, 0, NULL);
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
        str = raw_data_to_hex(item->data, item->len, 0, NULL);
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
sec_oid_str(SECItem *oid)
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
	str = sec_oid_str(&stripped_item);

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
        str = raw_data_to_hex(item->data, item->len, 0, NULL);

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

    str = raw_data_to_hex(stripped_item.data, stripped_item.len, 0, NULL);

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
    str = raw_data_to_hex(stripped_item.data, stripped_item.len, 0, NULL);
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
    str = raw_data_to_hex(stripped_item.data, stripped_item.len, 0, NULL);
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
        return raw_data_to_hex(item->data, item->len, 0, NULL);
    }
    Py_RETURN_NONE;
}

static PyObject *
get_algorithm_id_str(SECAlgorithmID *a)
{
    PyObject *str = NULL;

    if ((str = sec_oid_str(&a->algorithm)) == NULL)
        Py_RETURN_NONE;

    if ((a->parameters.len == 0) ||
	(a->parameters.len == 2 && memcmp(a->parameters.data, "\005\000", 2) == 0)) {
	/* No arguments or NULL argument */
    } else {
	/* Print args to algorithm */
        PyObject *hex_args = NULL;

        if ((hex_args = raw_data_to_hex(a->parameters.data, a->parameters.len, 0, NULL)) != NULL) {
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

static PyObject *
data_to_buffer(unsigned char *data, Py_ssize_t len)
{
    PyObject *buf = NULL;
    Py_ssize_t buf_len;
    void* raw_buffer;

    if ((buf = PyBuffer_New(len)) == NULL)
        return NULL;

    if (PyObject_AsWriteBuffer(buf, &raw_buffer, &buf_len) || buf_len < len) {
        Py_DECREF(buf);
        return NULL;
    }

    memcpy(raw_buffer, data, len);
    return buf;
}

/* ========================================================================== */
/* =============================== SecItem Class ============================ */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static
PyGetSetDef SecItem_getseters[] = {
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

    TraceObjNewEnter("SecItem_new", type);

    if ((self = (SecItem *)type->tp_alloc(type, 0)) == NULL) return NULL;
    self->item.type = 0;
    self->item.len = 0;
    self->item.data = NULL;
    self->kind = SECITEM_unknown;

    TraceObjNewLeave("SecItem_new", self);
    return (PyObject *)self;
}

static void
SecItem_dealloc(SecItem* self)
{
    TraceMethodEnter("SecItem_dealloc", self);

    if (self->item.data) {
        PyMem_FREE(self->item.data);
    }

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(SecItem_doc,
"DER encoded data. Used internally by NSS");

static int
SecItem_init(SecItem *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter("SecItem_init", self);

    return 0;
}

static PyObject *
SecItem_repr(SecItem *self)
{
    return PyString_FromFormat("<%s object at %p>",
                               self->ob_type->tp_name, self);
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
        return sec_oid_str(&self->item);
    default:
        return_value =  PyString_Encode((char *)self->item.data, self->item.len, "hex", NULL);
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
SecItem_sq_length(PyObject *obj)
{
    SecItem *self = (SecItem *) obj;
    return self->item.len;
}

static PySequenceMethods SecItem_as_sequence = {
	SecItem_sq_length,	/* sq_length */
	0,			/* sq_concat */
	0,			/* sq_repeat */
	0,			/* sq_item */
	0,			/* sq_slice */
	0,			/* sq_ass_item */
	0,			/* sq_ass_slice */
	0,			/* sq_contains */
	0,			/* sq_inplace_concat */
	0,			/* sq_inplace_repeat */
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
    0,						/* tp_as_mapping */
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
SecItem_new_from_sec_item(SECItem *item, SECItemKind kind)
{
    SecItem *self = NULL;

    TraceObjNewEnter("SecItem_new_from_sec_item", NULL);

    if ((self = (SecItem *) SecItemType.tp_new(&SecItemType, NULL, NULL)) == NULL)
        return NULL;

    self->item.type = item->type;
    self->item.len = item->len;
    if ((self->item.data = PyMem_MALLOC(item->len)) == NULL) {
        return PyErr_NoMemory();
    }
    memmove(self->item.data, item->data, item->len);

    self->kind = kind;

    TraceObjNewLeave("SecItem_new_from_sec_item", self);
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

    TraceObjNewEnter("SignatureAlgorithm_new", type);

    if ((self = (SignatureAlgorithm *)type->tp_alloc(type, 0)) == NULL) return NULL;

    memset(&self->id, 0, sizeof(self->id));
    self->py_id = NULL;
    self->py_parameters = NULL;

    TraceObjNewLeave("SignatureAlgorithm_new", self);
    return (PyObject *)self;
}

static void
SignatureAlgorithm_dealloc(SignatureAlgorithm* self)
{
    TraceMethodEnter("SignatureAlgorithm_dealloc", self);

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(SignatureAlgorithm_doc,
"An object representing a signature algorithm");

static int
SignatureAlgorithm_init(SignatureAlgorithm *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter("SignatureAlgorithm_init", self);

    return 0;
}

static PyObject *
SignatureAlgorithm_repr(SignatureAlgorithm *self)
{
    return PyString_FromFormat("<%s object at %p>",
                               self->ob_type->tp_name, self);
}

static PyObject *
SignatureAlgorithm_str(SignatureAlgorithm *self)
{
    return get_algorithm_id_str(&self->id);
}

static PyTypeObject SignatureAlgorithmType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.SignatureAlgorithm",			/* tp_name */
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
SignatureAlgorithm_new_from_algorithm_id(SECAlgorithmID *id)
{
    SignatureAlgorithm *self = NULL;

    TraceObjNewEnter("SignatureAlgorithm_new_from_sec_item", NULL);

    if ((self = (SignatureAlgorithm *) SignatureAlgorithmType.tp_new(&SignatureAlgorithmType, NULL, NULL)) == NULL)
        return NULL;

    self->id = *id;
    if ((self->py_id = SecItem_new_from_sec_item(&id->algorithm, SECITEM_algorithm)) == NULL)
        return NULL;
    if ((self->py_parameters = SecItem_new_from_sec_item(&id->parameters, SECITEM_unknown)) == NULL)
        return NULL;

    TraceObjNewLeave("SignatureAlgorithm_new_from_sec_item", self);
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

    TraceObjNewEnter("KEYPQGParams_new", type);

    if ((self = (KEYPQGParams *)type->tp_alloc(type, 0)) == NULL) return NULL;

    self->py_prime = NULL;
    self->py_subprime = NULL;
    self->py_base = NULL;

    TraceObjNewLeave("KEYPQGParams_new", self);
    return (PyObject *)self;
}

static void
KEYPQGParams_dealloc(KEYPQGParams* self)
{
    TraceMethodEnter("KEYPQGParams_dealloc", self);

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
    TraceMethodEnter("KEYPQGParams_init", self);

    return 0;
}

static PyObject *
KEYPQGParams_repr(KEYPQGParams *self)
{
    return PyString_FromFormat("<%s object at %p>",
                               self->ob_type->tp_name, self);
}

static PyObject *
KEYPQGParams_str(KEYPQGParams *self)
{
    PyObject *fmt = NULL;
    PyObject *args = NULL;
    PyObject *str = NULL;

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

    TraceObjNewEnter("KEYPQGParams_new_from_sec_item", NULL);

    if ((self = (KEYPQGParams *) KEYPQGParamsType.tp_new(&KEYPQGParamsType, NULL, NULL)) == NULL)
        return NULL;

    if ((self->py_prime = SecItem_new_from_sec_item(&params->prime, SECITEM_unknown)) == NULL)
        return NULL;

    if ((self->py_subprime = SecItem_new_from_sec_item(&params->subPrime, SECITEM_unknown)) == NULL)
        return NULL;

    if ((self->py_base = SecItem_new_from_sec_item(&params->base, SECITEM_unknown)) == NULL)
        return NULL;

    TraceObjNewLeave("KEYPQGParams_new_from_sec_item", self);
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

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:format_lines", kwlist, &level))
        return NULL; 

    if ((lines = PyList_New(0)) == NULL) {
        return NULL;
    }

    FMT_LABEL_AND_APPEND(level, "Modulus", lines, fail);

    if ((obj = RSAPublicKey_get_modulus(self, NULL)) == NULL)
        goto fail;
    obj_lines = obj_to_hex(obj, -1, NULL);
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
    return format_from_lines((format_lines_func)RSAPublicKey_format_lines, (PyObject *)self, args, kwds);
}

static PyObject *
RSAPublicKey_str(RSAPublicKey *self)
{
    PyObject *py_formatted_result = NULL;

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

    TraceObjNewEnter("RSAPublicKey_new", type);

    if ((self = (RSAPublicKey *)type->tp_alloc(type, 0)) == NULL) return NULL;

    self->py_modulus = NULL;
    self->py_exponent = NULL;

    TraceObjNewLeave("RSAPublicKey_new", self);
    return (PyObject *)self;
}

static void
RSAPublicKey_dealloc(RSAPublicKey* self)
{
    TraceMethodEnter("RSAPublicKey_dealloc", self);

    Py_XDECREF(self->py_modulus);
    Py_XDECREF(self->py_exponent);

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(RSAPublicKey_doc,
"An object representing an RSA Public Key");

static int
RSAPublicKey_init(RSAPublicKey *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter("RSAPublicKey_init", self);

    return 0;
}

static PyObject *
RSAPublicKey_repr(RSAPublicKey *self)
{
    return PyString_FromFormat("<%s object at %p>",
                               self->ob_type->tp_name, self);
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

    TraceObjNewEnter("RSAPublicKey_new_from_sec_item", NULL);

    if ((self = (RSAPublicKey *) RSAPublicKeyType.tp_new(&RSAPublicKeyType, NULL, NULL)) == NULL)
        return NULL;

    if ((self->py_modulus = SecItem_new_from_sec_item(&rsa->modulus, SECITEM_unknown)) == NULL)
        return NULL;

    if ((self->py_exponent = SecItem_new_from_sec_item(&rsa->publicExponent, SECITEM_unknown)) == NULL)
        return NULL;

    TraceObjNewLeave("RSAPublicKey_new_from_sec_item", self);
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

    TraceObjNewEnter("DSAPublicKey_new", type);

    if ((self = (DSAPublicKey *)type->tp_alloc(type, 0)) == NULL) return NULL;

    self->py_pqg_params = NULL;
    self->py_public_value = NULL;

    TraceObjNewLeave("DSAPublicKey_new", self);
    return (PyObject *)self;
}

static void
DSAPublicKey_dealloc(DSAPublicKey* self)
{
    TraceMethodEnter("DSAPublicKey_dealloc", self);

    Py_XDECREF(self->py_pqg_params);
    Py_XDECREF(self->py_public_value);

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(DSAPublicKey_doc,
"A object representing a DSA Public Key");

static int
DSAPublicKey_init(DSAPublicKey *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter("DSAPublicKey_init", self);

    return 0;
}

static PyObject *
DSAPublicKey_repr(DSAPublicKey *self)
{
    return PyString_FromFormat("<%s object at %p>",
                               self->ob_type->tp_name, self);
}

static PyObject *
DSAPublicKey_str(DSAPublicKey *self)
{
    PyObject *fmt = NULL;
    PyObject *args = NULL;
    PyObject *str = NULL;

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

    TraceObjNewEnter("DSAPublicKey_new_from_sec_item", NULL);

    if ((self = (DSAPublicKey *) DSAPublicKeyType.tp_new(&DSAPublicKeyType, NULL, NULL)) == NULL)
        return NULL;

    if ((self->py_pqg_params = KEYPQGParams_new_from_SECKEYPQGParams(&dsa->params)) == NULL)
        return NULL;

    if ((self->py_public_value = SecItem_new_from_sec_item(&dsa->publicValue, SECITEM_unknown)) == NULL)
        return NULL;

    TraceObjNewLeave("DSAPublicKey_new_from_sec_item", self);
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
    obj_lines = obj_to_hex(obj, -1, NULL);
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

    TraceObjNewEnter("SignedData_new", type);

    if ((self = (SignedData *)type->tp_alloc(type, 0)) == NULL) return NULL;

    self->py_data = NULL;
    self->py_algorithm = NULL;
    self->py_signature = NULL;

    if ((self->arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE)) == NULL ) {
        return PyErr_NoMemory();
    }
    
    memset(&self->signed_data, 0, sizeof(self->signed_data));

    TraceObjNewLeave("SignedData_new", self);
    return (PyObject *)self;
}

static void
SignedData_dealloc(SignedData* self)
{
    TraceMethodEnter("SignedData_dealloc", self);

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
    TraceMethodEnter("SignedData_init", self);

    return 0;
}

static PyObject *
SignedData_repr(SignedData *self)
{
    return PyString_FromFormat("<%s object at %p>",
                               self->ob_type->tp_name, self);
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
SignedData_new_from_sec_item(SECItem *item)
{
    SignedData *self = NULL;

    TraceObjNewEnter("SignedData_new_from_sec_item", NULL);

    if ((self = (SignedData *) SignedDataType.tp_new(&SignedDataType, NULL, NULL)) == NULL)
        return NULL;

    if (SEC_ASN1DecodeItem(self->arena, &self->signed_data,
                           SEC_ASN1_GET(CERT_SignedDataTemplate), item) != SECSuccess) {
        return NULL;
    }

    if ((self->py_data = SecItem_new_from_sec_item(&self->signed_data.data, SECITEM_unknown)) == NULL)
        return NULL;

    if ((self->py_algorithm = SignatureAlgorithm_new_from_algorithm_id(&self->signed_data.signatureAlgorithm)) == NULL)
        return NULL;

    DER_ConvertBitString(&self->signed_data.signature);
    if ((self->py_signature = SecItem_new_from_sec_item(&self->signed_data.signature, SECITEM_signature)) == NULL)
        return NULL;

    TraceObjNewLeave("SignedData_new_from_sec_item", self);
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
                     self->ob_type->tp_name, key_type_str(self->pk->keyType));
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
                     self->ob_type->tp_name, key_type_str(self->pk->keyType));
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

    TraceObjNewEnter("PublicKey_new", type);

    if ((self = (PublicKey *)type->tp_alloc(type, 0)) == NULL) return NULL;

    self->py_pk11slot = NULL;
    self->py_rsa_key = NULL;
    self->py_dsa_key = NULL;

    memset(&self->pk, 0, sizeof(self->pk));

    TraceObjNewLeave("PublicKey_new", self);
    return (PyObject *)self;
}

static void
PublicKey_dealloc(PublicKey* self)
{
    TraceMethodEnter("PublicKey_dealloc", self);

    Py_XDECREF(self->py_pk11slot);
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
    TraceMethodEnter("PublicKey_init", self);

    return 0;
}

static PyObject *
PublicKey_repr(PublicKey *self)
{
    return PyString_FromFormat("<%s object at %p>",
                               self->ob_type->tp_name, self);
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

    TraceObjNewEnter("PublicKey_new_from_sec_item", NULL);

    if ((self = (PublicKey *) PublicKeyType.tp_new(&PublicKeyType, NULL, NULL)) == NULL)
        return NULL;


    self->pk = pk;

    if (self->pk->pkcs11Slot) {
        if ((self->py_pk11slot = PK11Slot_new_from_slotinfo(self->pk->pkcs11Slot)) == NULL)
            return NULL;
    } else {
        Py_INCREF(Py_None);
        self->py_pk11slot = Py_None;

    }

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

    TraceObjNewLeave("PublicKey_new_from_sec_item", self);
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

    TraceObjNewEnter("SubjectPublicKeyInfo_new", type);

    if ((self = (SubjectPublicKeyInfo *)type->tp_alloc(type, 0)) == NULL) return NULL;

    self->py_algorithm = NULL;
    self->py_public_key = NULL;

    TraceObjNewLeave("SubjectPublicKeyInfo_new", self);
    return (PyObject *)self;
}

static void
SubjectPublicKeyInfo_dealloc(SubjectPublicKeyInfo* self)
{
    TraceMethodEnter("SubjectPublicKeyInfo_dealloc", self);

    Py_XDECREF(self->py_algorithm);
    Py_XDECREF(self->py_public_key);

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(SubjectPublicKeyInfo_doc,
"An object representing a Subject Public Key");

static int
SubjectPublicKeyInfo_init(SubjectPublicKeyInfo *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter("SubjectPublicKeyInfo_init", self);

    return 0;
}

static PyObject *
SubjectPublicKeyInfo_repr(SubjectPublicKeyInfo *self)
{
    return PyString_FromFormat("<%s object at %p>",
                               self->ob_type->tp_name, self);
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

    TraceObjNewEnter("SubjectPublicKeyInfo_new_from_sec_item", NULL);

    if ((self = (SubjectPublicKeyInfo *) SubjectPublicKeyInfoType.tp_new(&SubjectPublicKeyInfoType, NULL, NULL)) == NULL)
        return NULL;

    if ((self->py_algorithm = SignatureAlgorithm_new_from_algorithm_id(&spki->algorithm)) == NULL)
        return NULL;

    if ((pk = SECKEY_ExtractPublicKey(spki)) == NULL) {
        Py_DECREF(self->py_algorithm);
        return set_nspr_error(NULL);
    }

    if ((self->py_public_key = PublicKey_new_from_SECKEYPublicKey(pk)) == NULL) {
        Py_DECREF(self->py_algorithm);
        return NULL;
    }

    TraceObjNewLeave("SubjectPublicKeyInfo_new_from_sec_item", self);
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

    TraceObjNewEnter("CertDB_new", type);

    if ((self = (CertDB *)type->tp_alloc(type, 0)) == NULL) return NULL;
    self->handle = NULL;

    TraceObjNewLeave("CertDB_new", self);
    return (PyObject *)self;
}

static void
CertDB_dealloc(CertDB* self)
{
    TraceMethodEnter("CertDB_dealloc", self);

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(CertDB_doc,
"An object representing a Certificate Database");

static int
CertDB_init(CertDB *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter("CertDB_init", self);
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
CertDB_new_from_handle(CERTCertDBHandle *cert_handle)
{
    CertDB *self = NULL;

    TraceObjNewEnter("CertDB_new_from_handle", NULL);
    if ((self = (CertDB *) CertDBType.tp_new(&CertDBType, NULL, NULL)) == NULL)
        return NULL;

    self->handle = cert_handle;

    TraceObjNewLeave("CertDB_new_from_handle", self);
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
        if ((py_sec_item = SecItem_new_from_sec_item(&names->names[i], SECITEM_dist_name)) == NULL) {
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
    return SignatureAlgorithm_new_from_algorithm_id(&self->cert->signature);
}

static PyObject *
Certificate_get_signed_data(Certificate *self, void *closure)
{
    PyObject *py_signed_data = NULL;

    return py_signed_data = SignedData_new_from_sec_item(&self->cert->derCert);
}

static PyObject *
Certificate_get_der_data(Certificate *self, void *closure)
{
    PyObject *sig_buf = NULL;
    SECItem der;

    der = self->cert->derCert;
    sig_buf = data_to_buffer(der.data, der.len);

    return sig_buf;
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

    TraceMethodEnter("verify_hostname", self);

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

    TraceMethodEnter("has_signer_in_ca_names", self);

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

    TraceMethodEnter("check_valid_times", self);

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

#if 0
extern SECStatus
CERT_VerifyCertificate(CERTCertDBHandle *handle, CERTCertificate *cert,
		PRBool checkSig, SECCertificateUsage requiredUsages,
                int64 t, void *wincx, CERTVerifyLog *log,
                SECCertificateUsage* returnedUsages);

/* same as above, but uses current time */
extern SECStatus
CERT_VerifyCertificateNow(CERTCertDBHandle *handle, CERTCertificate *cert,
		   PRBool checkSig, SECCertificateUsage requiredUsages,
                   void *wincx, SECCertificateUsage* returnedUsages);

#endif

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
    Py_ssize_t argc;
    PyObject *pin_args = NULL;
    PyObject *py_certdb = NULL;
    PyObject *py_check_sig = NULL;
    PyObject *py_required_usages = NULL;
    int check_sig = 0;
    unsigned long required_usages = 0;
    SECCertificateUsage returned_usages;

    TraceMethodEnter("verify_now", self);

    argc = PyTuple_Size(args);
    if (argc < 3) {
        PyErr_Format(PyExc_TypeError, "verify_now: expected at least 3 arguments, but got %jd", (intmax_t)argc);
        return NULL;
    }

    py_certdb          = PyTuple_GetItem(args, 0);
    py_check_sig       = PyTuple_GetItem(args, 1);
    py_required_usages = PyTuple_GetItem(args, 2);

    if (!PyCertDB_Check(py_certdb)) {
        PyErr_Format(PyExc_TypeError, "verify_now: certdb parameter must be a %s", CertDBType.tp_name);
        return NULL;
    }
    if (!(PyBool_Check(py_check_sig) || PyInt_Check(py_check_sig))) {
        PyErr_SetString(PyExc_TypeError, "verify_now: check_sig parameter must be a boolean or integer");
        return NULL;
    }
    if (!PyInt_Check(py_required_usages)) {
        PyErr_SetString(PyExc_TypeError, "verify_now: required_usages parameter must be an integer");
        return NULL;
    }

    pin_args = PyTuple_GetSlice(args, 3, argc);
    Py_INCREF(pin_args);

    if (CERT_VerifyCertificateNow(((CertDB *)py_certdb)->handle, self->cert, check_sig,
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
    if ((obj = cert_md5_digest(NULL, tmp_args)) == NULL)
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
    if ((obj = cert_sha1_digest(NULL, tmp_args)) == NULL)
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

    TraceObjNewEnter("Certificate_new", type);

    if ((self = (Certificate *)type->tp_alloc(type, 0)) == NULL) return NULL;
    self->cert = NULL;
    self->py_subject_public_key_info = NULL;

    TraceObjNewLeave("Certificate_new", self);
    return (PyObject *)self;
}

static void
Certificate_dealloc(Certificate* self)
{
    TraceMethodEnter("Certificate_dealloc", self);

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
    TraceMethodEnter("Certificate_init", self);

    return 0;
}

static PyObject *
Certificate_repr(Certificate *self)
{
    return PyString_FromFormat("<%s object at %p Certificate %p>",
                               self->ob_type->tp_name, self, self->cert);
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
Certificate_new_from_cert(CERTCertificate *cert)
{
    Certificate *self = NULL;

    TraceObjNewEnter("Certificate_new_from_cert", NULL);

    if ((self = (Certificate *) CertificateType.tp_new(&CertificateType, NULL, NULL)) == NULL)
        return NULL;

    self->cert = cert;
    self->py_subject_public_key_info = SubjectPublicKeyInfo_new_from_CERTSubjectPublicKeyInfo(&cert->subjectPublicKeyInfo);

    TraceObjNewLeave("Certificate_new_from_cert", self);
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

    TraceObjNewEnter("PrivateKey_new", type);

    if ((self = (PrivateKey *)type->tp_alloc(type, 0)) == NULL) return NULL;
    self->private_key = NULL;

    TraceObjNewLeave("PrivateKey_new", self);
    return (PyObject *)self;
}

static void
PrivateKey_dealloc(PrivateKey* self)
{
    TraceMethodEnter("PrivateKey_dealloc", self);

    if (self->private_key)
        SECKEY_DestroyPrivateKey(self->private_key);

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(PrivateKey_doc,
"An object representing a Private Key");

static int
PrivateKey_init(PrivateKey *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter("PrivateKey_init", self);
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
PrivateKey_new_from_private_key(SECKEYPrivateKey *private_key)
{
    PrivateKey *self = NULL;

    TraceObjNewEnter("PrivateKey_new_from_private_key", NULL);
    if ((self = (PrivateKey *) PrivateKeyType.tp_new(&PrivateKeyType, NULL, NULL)) == NULL)
        return NULL;

    self->private_key = private_key;

    TraceObjNewLeave("PrivateKey_new_from_private_key", self);
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

    if ((cert_handle = CERT_GetDefaultCertDB()) == NULL)
        Py_RETURN_NONE;

    return CertDB_new_from_handle(cert_handle);
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
    Py_ssize_t argc;
    PyObject *pin_args = NULL;
    PyObject *py_what = NULL;
    int what;
    PyObject *py_certdb = NULL;
    CERTCertNicknames *cert_nicknames = NULL;
    PyObject *py_nicknames = NULL;
    PyObject *py_nickname = NULL;
    int i, len;
    
    TraceMethodEnter("cert_get_cert_nicknames", self);

    argc = PyTuple_Size(args);

    if ((py_certdb = PyTuple_GetItem(args, 0)) == NULL) {
        PyErr_SetString(PyExc_TypeError, "get_cert_nicknames: missing certdb argument");
        return NULL;
    }
    if (!PyCertDB_Check(py_certdb)) {
        PyErr_Format(PyExc_TypeError, "get_cert_nicknames: certdb parameter must be a %s", CertDBType.tp_name);
        return NULL;
    }

    if ((py_what = PyTuple_GetItem(args, 1)) == NULL) {
        PyErr_SetString(PyExc_TypeError, "get_cert_nicknames: missing what argument");
        return NULL;
    }
    if (!PyInt_Check(py_what)) {
        PyErr_SetString(PyExc_TypeError, "get_cert_nicknames: what parameter must be an int");
        return NULL;
    }
    what = PyInt_AsLong(py_what);

    pin_args = PyTuple_GetSlice(args, 2, argc);
    Py_INCREF(pin_args);

    if ((cert_nicknames = CERT_GetCertNicknames(((CertDB *)py_certdb)->handle, what, pin_args)) == NULL) {
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

PyDoc_STRVAR(cert_md5_digest_doc,
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
cert_md5_digest(PyObject *self, PyObject *args)
{
    PyObject *data_buf = NULL; 
    unsigned char *raw_data = NULL;
    Py_ssize_t raw_data_len = 0;
    PK11Context *md5  = NULL;
    unsigned char digest[MD5_LENGTH];
    unsigned int digest_len = 0;

    if (!PyArg_ParseTuple(args, "O!:cert_md5_digest", &PyBuffer_Type, &data_buf)) {
        return NULL;
    }

    if (PyObject_AsReadBuffer(data_buf, (void *)&raw_data, &raw_data_len))
        return NULL;

    if ((md5  = PK11_CreateDigestContext (SEC_OID_MD5)) == NULL)
        return set_nspr_error(NULL);
        
    if (PK11_DigestBegin (md5) != SECSuccess)
        return set_nspr_error(NULL);

    if (PK11_DigestOp (md5,  (unsigned char*)raw_data, raw_data_len) != SECSuccess)
        return set_nspr_error(NULL);

    if (PK11_DigestFinal (md5,  digest,  &digest_len,  MD5_LENGTH) != SECSuccess)
        return set_nspr_error(NULL);

    PK11_DestroyContext (md5,  PR_TRUE);
  
    return data_to_buffer(digest, digest_len);
}

PyDoc_STRVAR(cert_sha1_digest_doc,
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
cert_sha1_digest(PyObject *self, PyObject *args)
{
    PyObject *data_buf = NULL; 
    unsigned char *raw_data = NULL;
    Py_ssize_t raw_data_len = 0;
    PK11Context *sha1  = NULL;
    unsigned char digest[SHA1_LENGTH];
    unsigned int digest_len = 0;

    if (!PyArg_ParseTuple(args, "O!:cert_sha1_digest", &PyBuffer_Type, &data_buf)) {
        return NULL;
    }

    if (PyObject_AsReadBuffer(data_buf, (void *)&raw_data, &raw_data_len))
        return NULL;

    if ((sha1  = PK11_CreateDigestContext (SEC_OID_SHA1)) == NULL)
        return set_nspr_error(NULL);
        
    if (PK11_DigestBegin (sha1) != SECSuccess)
        return set_nspr_error(NULL);

    if (PK11_DigestOp (sha1,  (unsigned char*)raw_data, raw_data_len) != SECSuccess)
        return set_nspr_error(NULL);

    if (PK11_DigestFinal (sha1,  digest,  &digest_len,  SHA1_LENGTH) != SECSuccess)
        return set_nspr_error(NULL);

    PK11_DestroyContext (sha1,  PR_TRUE);
  
    return data_to_buffer(digest, digest_len);
}

PyDoc_STRVAR(cert_sha256_digest_doc,
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
cert_sha256_digest(PyObject *self, PyObject *args)
{
    PyObject *data_buf = NULL; 
    unsigned char *raw_data = NULL;
    Py_ssize_t raw_data_len = 0;
    PK11Context *sha256  = NULL;
    unsigned char digest[SHA256_LENGTH];
    unsigned int digest_len = 0;

    if (!PyArg_ParseTuple(args, "O!:cert_sha256_digest", &PyBuffer_Type, &data_buf)) {
        return NULL;
    }

    if (PyObject_AsReadBuffer(data_buf, (void *)&raw_data, &raw_data_len))
        return NULL;

    if ((sha256  = PK11_CreateDigestContext (SEC_OID_SHA256)) == NULL)
        return set_nspr_error(NULL);
        
    if (PK11_DigestBegin (sha256) != SECSuccess)
        return set_nspr_error(NULL);

    if (PK11_DigestOp (sha256,  (unsigned char*)raw_data, raw_data_len) != SECSuccess)
        return set_nspr_error(NULL);

    if (PK11_DigestFinal (sha256,  digest,  &digest_len,  SHA256_LENGTH) != SECSuccess)
        return set_nspr_error(NULL);

    PK11_DestroyContext (sha256,  PR_TRUE);
  
    return data_to_buffer(digest, digest_len);
}

PyDoc_STRVAR(cert_sha512_digest_doc,
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
cert_sha512_digest(PyObject *self, PyObject *args)
{
    PyObject *data_buf = NULL; 
    unsigned char *raw_data = NULL;
    Py_ssize_t raw_data_len = 0;
    PK11Context *sha512  = NULL;
    unsigned char digest[SHA512_LENGTH];
    unsigned int digest_len = 0;

    if (!PyArg_ParseTuple(args, "O!:cert_sha512_digest", &PyBuffer_Type, &data_buf)) {
        return NULL;
    }

    if (PyObject_AsReadBuffer(data_buf, (void *)&raw_data, &raw_data_len))
        return NULL;

    if ((sha512  = PK11_CreateDigestContext (SEC_OID_SHA512)) == NULL)
        return set_nspr_error(NULL);
        
    if (PK11_DigestBegin (sha512) != SECSuccess)
        return set_nspr_error(NULL);

    if (PK11_DigestOp (sha512,  (unsigned char*)raw_data, raw_data_len) != SECSuccess)
        return set_nspr_error(NULL);

    if (PK11_DigestFinal (sha512,  digest,  &digest_len,  SHA512_LENGTH) != SECSuccess)
        return set_nspr_error(NULL);

    PK11_DestroyContext (sha512,  PR_TRUE);
  
    return data_to_buffer(digest, digest_len);
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
PK11Slot_is_hw(PK11Slot *self, PyObject *args, PyObject *kwds)
{
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
PK11Slot_is_present(PK11Slot *self, PyObject *args, PyObject *kwds)
{
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
PK11Slot_is_read_only(PK11Slot *self, PyObject *args, PyObject *kwds)
{
    if (PK11_IsReadOnly(self->slot))
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;

}

static PyMethodDef PK11Slot_methods[] = {
    {"is_hw",        (PyCFunction)PK11Slot_is_hw,        METH_NOARGS, PK11Slot_is_hw_doc},
    {"is_present",   (PyCFunction)PK11Slot_is_present,   METH_NOARGS, PK11Slot_is_present_doc},
    {"is_read_only", (PyCFunction)PK11Slot_is_read_only, METH_NOARGS, PK11Slot_is_read_only_doc},
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
PK11Slot_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PK11Slot *self;

    TraceObjNewEnter("PK11Slot_new", type);

    if ((self = (PK11Slot *)type->tp_alloc(type, 0)) == NULL) return NULL;
    self->slot = NULL;

    TraceObjNewLeave("PK11Slot_new", type);
    return (PyObject *)self;
}

static void
PK11Slot_dealloc(PK11Slot* self)
{
    TraceMethodEnter("PK11Slot_dealloc", self);

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(PK11Slot_doc,
"An object representing a PKCS #11 Slot");

static int
PK11Slot_init(PK11Slot *self, PyObject *args, PyObject *kwds)
{
    PyObject *arg1 = NULL;
    static char *kwlist[] = {"arg1", NULL};

    TraceMethodEnter("PK11Slot_init", self);

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
PK11Slot_new_from_slotinfo(PK11SlotInfo *slot)
{
    PK11Slot *self = NULL;

    TraceObjNewEnter("PK11Slot_new_from_slotinfo", NULL);

    if ((self = (PK11Slot *) PK11SlotType.tp_new(&PK11SlotType, NULL, NULL)) == NULL)
        return NULL;

    self->slot = slot;
    TraceObjNewLeave("PK11Slot_new_from_slotinfo", self);
    return (PyObject *) self;
}

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

    if ((py_slot = PK11Slot_new_from_slotinfo(slot)) == NULL) {
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

/* ========================== PK11 Module Methods =========================== */

PyDoc_STRVAR(PK11_set_password_callback_doc,
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
PK11_set_password_callback(PyObject *self, PyObject *args)
{
    PyObject *callback;

    TraceMethodEnter("PK11_set_password_callback", self);

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

PyDoc_STRVAR(PK11_find_cert_from_nickname_doc,
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
PK11_find_cert_from_nickname(PyObject *self, PyObject *args)
{
    PyObject *py_nickname = NULL;
    char *nickname = NULL;
    Py_ssize_t argc;
    PyObject *pin_args = NULL;
    CERTCertificate *cert = NULL;
    PyObject *py_cert = NULL;

    TraceMethodEnter("PK11_find_cert_from_nickname", self);

    argc = PyTuple_Size(args);

    if ((py_nickname = PyTuple_GetItem(args, 0)) == NULL) {
        PyErr_SetString(PyExc_TypeError, "find_cert_from_nickname: missing nickname argument");
        return NULL;
    }

    if (!PyString_Check(py_nickname)) {
        PyErr_SetString(PyExc_TypeError, "find_cert_from_nickname: nickname parameter must be a string");
        return NULL;
    }

    nickname = PyString_AsString(py_nickname);
    pin_args = PyTuple_GetSlice(args, 1, argc);
    Py_INCREF(pin_args);

    if ((cert = PK11_FindCertFromNickname(nickname, pin_args)) == NULL) {
        Py_DECREF(pin_args);
        return set_nspr_error(NULL);
    } 

    Py_DECREF(pin_args);

    if ((py_cert = Certificate_new_from_cert(cert)) == NULL) {
        return NULL;
    }

    return py_cert;
}

PyDoc_STRVAR(PK11_find_key_by_any_cert_doc,
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
PK11_find_key_by_any_cert(PyObject *self, PyObject *args)
{
    PyObject *py_cert = NULL;
    Py_ssize_t argc;
    PyObject *pin_args = NULL;
    SECKEYPrivateKey *private_key;
    PyObject *py_private_key = NULL;

    TraceMethodEnter("PK11_find_key_by_any_cert", self);

    argc = PyTuple_Size(args);

    if ((py_cert = PyTuple_GetItem(args, 0)) == NULL) {
        PyErr_SetString(PyExc_TypeError, "find_key_by_any_cert: missing cert argument");
        return NULL;
    }

    if (!PyCertificate_Check(py_cert)) {
        PyErr_Format(PyExc_TypeError, "find_key_by_any_cert: cert parameter must be a %s", CertificateType.tp_name);
        return NULL;
    }

    pin_args = PyTuple_GetSlice(args, 1, argc);
    Py_INCREF(pin_args);

    if ((private_key = PK11_FindKeyByAnyCert(((Certificate *)py_cert)->cert, pin_args)) == NULL) {
        Py_DECREF(pin_args);
        return set_nspr_error(NULL);
    } 

    Py_DECREF(pin_args);

    if ((py_private_key = PrivateKey_new_from_private_key(private_key)) == NULL) {
        return NULL;
    }

    return py_private_key;
}

PyDoc_STRVAR(PK11_generate_random_doc,
"generate_random(num_bytes) -> string\n\
\n\
:Parameters:\n\
    num_bytes : integer\n\
        Number of num_bytes to generate (must be non-negative)\n\
\n\
Generates random data..\n\
");

static PyObject *
PK11_generate_random(PyObject *self, PyObject *args)
{
    int num_bytes;
    unsigned char *buf;
    SECStatus status;
    PyObject *res;

    TraceMethodEnter("PK11_generate_random", self);

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

PyDoc_STRVAR(cert_data_to_hex_doc,
"data_to_hex(data, octets_per_line=16, separator=\":\")\n\
\n\
:Parameters:\n\
    data : buffer\n\
        binary data\n\
    octets_per_line : integer\n\
        number of octets formatted on one line, if 0 then\n\
        return a single string instead of an array of lines\n\
    separator : string\n\
        string used to seperate each octet\n\
\n\
Format the binary data as a hex string. If octets_per_line is an integer then return a list of lines\n\
otherwise return a single string\n\
");

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



/* List of functions exported by this module. */
static PyMethodDef
module_methods[] = {
    {"set_password_callback",   PK11_set_password_callback,      METH_VARARGS,               PK11_set_password_callback_doc},
    {"find_cert_from_nickname", PK11_find_cert_from_nickname,    METH_VARARGS,               PK11_find_cert_from_nickname_doc},
    {"find_key_by_any_cert",    PK11_find_key_by_any_cert,       METH_VARARGS,               PK11_find_key_by_any_cert_doc},
    {"generate_random",         PK11_generate_random,            METH_VARARGS,               PK11_generate_random_doc},
    {"get_default_certdb",      cert_get_default_certdb,         METH_NOARGS,                cert_get_default_certdb_doc},
    {"get_cert_nicknames",      cert_get_cert_nicknames,         METH_VARARGS,               cert_get_cert_nicknames_doc},
    {"data_to_hex",             (PyCFunction)cert_data_to_hex,   METH_VARARGS|METH_KEYWORDS, cert_data_to_hex_doc},
    {"md5_digest",              (PyCFunction)cert_md5_digest,    METH_VARARGS,               cert_md5_digest_doc},
    {"sha1_digest",             (PyCFunction)cert_sha1_digest,   METH_VARARGS,               cert_sha1_digest_doc},
    {"sha256_digest",           (PyCFunction)cert_sha256_digest, METH_VARARGS,               cert_sha256_digest_doc},
    {"sha512_digest",           (PyCFunction)cert_sha512_digest, METH_VARARGS,               cert_sha512_digest_doc},
    {"indented_format",         (PyCFunction)nss_indented_format,METH_VARARGS|METH_KEYWORDS, nss_indented_format_doc},
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
    Certificate_new_from_cert,
    PrivateKey_new_from_private_key,
    SecItem_new_from_sec_item,
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

    if (import_nspr_error_c_api() < 0)
        return;

    if ((m = Py_InitModule3("nss.nss", module_methods, module_doc)) == NULL)
        return;

    if ((empty_tuple = PyTuple_New(0)) == NULL)
        return;
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

    /* Export C API */
    if (PyModule_AddObject(m, "_C_API", PyCObject_FromVoidPtr((void *)&nspr_nss_c_api, NULL)) != 0)
        return;

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


}
