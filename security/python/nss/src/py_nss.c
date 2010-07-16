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

#if 0

//Template for new classes

/* ========================================================================== */
/* ============================= NewType Class ============================== */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static PyObject *
NewType_get_classproperty(NewType *self, void *closure)
{
    TraceMethodEnter(self);

    return NULL;
}

static
PyGetSetDef NewType_getseters[] = {
    {"classproperty", (getter)NewType_get_classproperty,    (setter)NULL,
     "xxx", NULL},
    {NULL}  /* Sentinel */
};

static PyMemberDef NewType_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

PyDoc_STRVAR(cert_func_name_doc,
"func_name() -> \n\
\n\
:Parameters:\n\
    arg1 : object\n\
        xxx\n\
\n\
xxx\n\
");

static PyObject *
cert_func_name(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"arg1", NULL};
    PyObject *arg;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i:func_name", kwlist,
                                     &arg))
        return NULL;

    return NULL;
}

static PyMethodDef NewType_methods[] = {
    {"func_name", (PyCFunction)cert_func_name, METH_VARARGS|METH_KEYWORDS, cert_func_name_doc},
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
NewType_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    NewType *self;

    TraceObjNewEnter(type);

    if ((self = (NewType *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

Py_TPFLAGS_HAVE_GC
static int
NewType_traverse(NewType *self, visitproc visit, void *arg)
{
    Py_VISIT(self->obj);
    return 0;
}

static int
NewType_clear(NewType* self)
{
    TraceMethodEnter(self);

    Py_CLEAR(self->obj);
    return 0;
}

static void
NewType_dealloc(NewType* self)
{
    TraceMethodEnter(self);

    NewType_clear(self);
    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(NewType_doc,
"An object representing xxx");

static int
NewType_init(NewType *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"arg", NULL};
    PyObject *arg;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i:NewType", kwlist,
                                     &arg))
        return -1;

    return 0;
}

static PyObject *
NewType_repr(NewType *self)
{
    return PyString_FromFormat("<%s object at %p>",
                               Py_TYPE(self)->tp_name, self);
}

static PyTypeObject NewTypeType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.NewType",				/* tp_name */
    sizeof(NewType),				/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)NewType_dealloc,		/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    0,						/* tp_compare */
    (reprfunc)NewType_repr,			/* tp_repr */
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
    NewType_doc,				/* tp_doc */
    (traverseproc)0,				/* tp_traverse */
    (inquiry)0,					/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    NewType_methods,				/* tp_methods */
    NewType_members,				/* tp_members */
    NewType_getseters,				/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)NewType_init,			/* tp_init */
    0,						/* tp_alloc */
    NewType_new,				/* tp_new */
};

static PyObject *
NewType_new_from_NSSType(NSSType *id)
{
    NewType *self = NULL;

    TraceObjNewEnter(NULL);

    if ((self = (NewType *) NewTypeType.tp_new(&NewTypeType, NULL, NULL)) == NULL) {
        return NULL;
    }

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

#endif

// FIXME: should we be calling these?
// SECKEY_DestroyEncryptedPrivateKeyInfo
// SECKEY_DestroyPrivateKey	   SECKEY_DestroyPrivateKeyInfo
// SECKEY_DestroyPrivateKeyList	   SECKEY_DestroyPublicKey
// SECKEY_DestroyPublicKeyList	   SECKEY_DestroySubjectPublicKeyInfo

#define CERT_DecodeDERCertificate __CERT_DecodeDERCertificate

#include <stdbool.h>

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
#include "nssb64.h"
#include "secport.h"
#include "secerr.h"

#define MAX_AVAS 10
#define MAX_RDNS 10
#define OCTETS_PER_LINE_DEFAULT 16
#define HEX_SEPARATOR_DEFAULT ":"

/* FIXME: convert all equality tests to Py_None to PyNone_Check() */
#define PyNone_Check(x) ((x) == Py_None)

//FIXME, should be in py_nss.h
#define PyAVA_Check(op)  PyObject_TypeCheck(op, &AVAType)
#define PyRDN_Check(op)  PyObject_TypeCheck(op, &RDNType)
#define PyDN_Check(op) PyObject_TypeCheck(op, &DNType)


#define FMT_OBJ_AND_APPEND(dst_pairs, label, src_obj, level, fail)      \
{                                                                       \
    PyObject *pair = NULL;                                              \
                                                                        \
    if ((pair = fmt_pair(level, label, src_obj)) == NULL) {             \
        goto fail;                                                      \
    }                                                                   \
    if (PyList_Append(dst_pairs, pair) != 0) {                          \
        Py_DECREF(pair);                                                \
        goto fail;                                                      \
    }                                                                   \
}

#define FMT_LABEL_AND_APPEND(dst_pairs, label, level, fail)     \
{                                                               \
    PyObject *pair = NULL;                                      \
                                                                \
    if ((pair = fmt_label(level, label)) == NULL) {             \
        goto fail;                                              \
    }                                                           \
    if (PyList_Append(dst_pairs, pair) != 0) {                  \
        Py_DECREF(pair);                                        \
        goto fail;                                              \
    }                                                           \
}

#define APPEND_LINE_PAIRS_AND_CLEAR(dst_pairs, src_pairs, fail) \
{                                                               \
    PyObject *src_obj;                                          \
    Py_ssize_t len, i;                                          \
    if (src_pairs) {                                            \
        len = PyList_Size(src_pairs);                           \
        for (i = 0; i < len; i++) {                             \
            src_obj = PyList_GetItem(src_pairs, i);             \
            PyList_Append(dst_pairs, src_obj);                  \
        }                                                       \
        Py_CLEAR(src_pairs);                                    \
    }                                                           \
}

#define APPEND_LINES_AND_CLEAR(dst_pairs, src_lines, level, fail)       \
{                                                                       \
    PyObject *src_obj;                                                  \
    Py_ssize_t len, i;                                                  \
    if (src_lines) {                                                    \
        len = PySequence_Size(src_lines);                               \
        for (i = 0; i < len; i++) {                                     \
            src_obj = PySequence_GetItem(src_lines, i);                 \
            FMT_OBJ_AND_APPEND(dst_pairs, NULL, src_obj, level, fail);  \
            Py_DECREF(src_obj);                                         \
        }                                                               \
        Py_CLEAR(src_lines);                                            \
    }                                                                   \
}

#define CALL_FORMAT_LINES_AND_APPEND(dst_pairs, obj, level, fail)       \
{                                                                       \
    PyObject *obj_line_pairs;                                           \
                                                                        \
    if ((obj_line_pairs =                                               \
         PyObject_CallMethod(obj, "format_lines",                       \
                             "(i)", level)) == NULL) {                  \
        goto fail;                                                      \
    }                                                                   \
                                                                        \
    APPEND_LINE_PAIRS_AND_CLEAR(dst_pairs, obj_line_pairs, fail);       \
}


#define APPEND_OBJ_TO_HEX_LINES_AND_CLEAR(dst_pairs, obj, level, fail)  \
{                                                                       \
    PyObject *obj_lines;                                                \
                                                                        \
    if ((obj_lines = obj_to_hex(obj, OCTETS_PER_LINE_DEFAULT,           \
                                HEX_SEPARATOR_DEFAULT)) == NULL) {      \
        goto fail;                                                      \
    }                                                                   \
    Py_CLEAR(obj);                                                      \
    APPEND_LINES_AND_CLEAR(dst_pairs, obj_lines, level, fail);          \
}

/* Copied from mozilla/security/nss/lib/certdb/alg1485.c */
typedef struct DnAvaPropsStr {
    const char * name;
    unsigned int maxLen; /* max bytes in UTF8 encoded string value */
    SECOidTag    oid_tag;
    int		 value_type;
} DnAvaProps;

static const DnAvaProps dn_ava_props[] = {
/* IANA registered type names
 * (See: http://www.iana.org/assignments/ldap-parameters) 
 */
/* RFC 3280, 4630 MUST SUPPORT */
    { "CN",             64, SEC_OID_AVA_COMMON_NAME,    SEC_ASN1_UTF8_STRING},
    { "ST",            128, SEC_OID_AVA_STATE_OR_PROVINCE,
							SEC_ASN1_UTF8_STRING},
    { "O",              64, SEC_OID_AVA_ORGANIZATION_NAME,
							SEC_ASN1_UTF8_STRING},
    { "OU",             64, SEC_OID_AVA_ORGANIZATIONAL_UNIT_NAME,
                                                        SEC_ASN1_UTF8_STRING},
    { "dnQualifier", 32767, SEC_OID_AVA_DN_QUALIFIER, SEC_ASN1_PRINTABLE_STRING},
    { "C",               2, SEC_OID_AVA_COUNTRY_NAME, SEC_ASN1_PRINTABLE_STRING},
    { "serialNumber",   64, SEC_OID_AVA_SERIAL_NUMBER,SEC_ASN1_PRINTABLE_STRING},

/* RFC 3280, 4630 SHOULD SUPPORT */
    { "L",             128, SEC_OID_AVA_LOCALITY,       SEC_ASN1_UTF8_STRING},
    { "title",          64, SEC_OID_AVA_TITLE,          SEC_ASN1_UTF8_STRING},
    { "SN",             64, SEC_OID_AVA_SURNAME,        SEC_ASN1_UTF8_STRING},
    { "givenName",      64, SEC_OID_AVA_GIVEN_NAME,     SEC_ASN1_UTF8_STRING},
    { "initials",       64, SEC_OID_AVA_INITIALS,       SEC_ASN1_UTF8_STRING},
    { "generationQualifier",
                        64, SEC_OID_AVA_GENERATION_QUALIFIER,
                                                        SEC_ASN1_UTF8_STRING},
/* RFC 3280, 4630 MAY SUPPORT */
    { "DC",            128, SEC_OID_AVA_DC,             SEC_ASN1_IA5_STRING},
    { "MAIL",          256, SEC_OID_RFC1274_MAIL,       SEC_ASN1_IA5_STRING},
    { "UID",           256, SEC_OID_RFC1274_UID,        SEC_ASN1_UTF8_STRING},

/* values from draft-ietf-ldapbis-user-schema-05 (not in RFC 3280) */
    { "postalAddress", 128, SEC_OID_AVA_POSTAL_ADDRESS, SEC_ASN1_UTF8_STRING},
    { "postalCode",     40, SEC_OID_AVA_POSTAL_CODE,    SEC_ASN1_UTF8_STRING},
    { "postOfficeBox",  40, SEC_OID_AVA_POST_OFFICE_BOX,SEC_ASN1_UTF8_STRING},
    { "houseIdentifier",64, SEC_OID_AVA_HOUSE_IDENTIFIER,SEC_ASN1_UTF8_STRING},
/* end of IANA registered type names */

/* legacy keywords */
    { "E",             128, SEC_OID_PKCS9_EMAIL_ADDRESS,SEC_ASN1_IA5_STRING},

#if 0 /* removed.  Not yet in any IETF draft or RFC. */
    { "pseudonym",      64, SEC_OID_AVA_PSEUDONYM,      SEC_ASN1_UTF8_STRING},
#endif

    { 0,           256, SEC_OID_UNKNOWN                      , 0},
};

/* ========================================================================== */
typedef struct {
    unsigned short len;
    char *encoded;
} AsciiEscapes;

static AsciiEscapes ascii_encoding_table[256] = {
    {4, "\\x00"}, /*   0      */    {4, "\\x01"}, /*   1      */
    {4, "\\x02"}, /*   2      */    {4, "\\x03"}, /*   3      */
    {4, "\\x04"}, /*   4      */    {4, "\\x05"}, /*   5      */
    {4, "\\x06"}, /*   6      */    {2, "\\a"  }, /*   7 BELL */
    {2, "\\b"  }, /*   8 BS   */    {2, "\\t"  }, /*   9 HTAB */
    {2, "\\n"  }, /*  10 NL   */    {2, "\\v"  }, /*  11 VTAB */
    {2, "\\f"  }, /*  12 FF   */    {2, "\\r"  }, /*  13 CR   */
    {4, "\\x0E"}, /*  14      */    {4, "\\x0F"}, /*  15      */
    {4, "\\x10"}, /*  16      */    {4, "\\x11"}, /*  17      */
    {4, "\\x12"}, /*  18      */    {4, "\\x13"}, /*  19      */
    {4, "\\x14"}, /*  20      */    {4, "\\x15"}, /*  21      */
    {4, "\\x16"}, /*  22      */    {4, "\\x17"}, /*  23      */
    {4, "\\x18"}, /*  24      */    {4, "\\x19"}, /*  25      */
    {4, "\\x1A"}, /*  26      */    {4, "\\x1B"}, /*  27      */
    {4, "\\x1C"}, /*  28      */    {4, "\\x1D"}, /*  29      */
    {4, "\\x1E"}, /*  30      */    {4, "\\x1F"}, /*  31      */
    {1, " "    }, /*  32      */    {1, "!"    }, /*  33 !    */
    {2, "\\\"" }, /*  34 "    */    {1, "#"    }, /*  35 #    */
    {1, "$"    }, /*  36 $    */    {1, "%"    }, /*  37 %    */
    {1, "&"    }, /*  38 &    */    {2, "\\'"  }, /*  39 '    */
    {1, "("    }, /*  40 (    */    {1, ")"    }, /*  41 )    */
    {1, "*"    }, /*  42 *    */    {1, "+"    }, /*  43 +    */
    {1, ","    }, /*  44 ,    */    {1, "-"    }, /*  45 -    */
    {1, "."    }, /*  46 .    */    {1, "/"    }, /*  47 /    */
    {1, "0"    }, /*  48 0    */    {1, "1"    }, /*  49 1    */
    {1, "2"    }, /*  50 2    */    {1, "3"    }, /*  51 3    */
    {1, "4"    }, /*  52 4    */    {1, "5"    }, /*  53 5    */
    {1, "6"    }, /*  54 6    */    {1, "7"    }, /*  55 7    */
    {1, "8"    }, /*  56 8    */    {1, "9"    }, /*  57 9    */
    {1, ":"    }, /*  58 :    */    {1, ";"    }, /*  59 ;    */
    {1, "<"    }, /*  60 <    */    {1, "="    }, /*  61 =    */
    {1, ">"    }, /*  62 >    */    {2, "\\?"  }, /*  63 ?    */
    {1, "@"    }, /*  64 @    */    {1, "A"    }, /*  65 A    */
    {1, "B"    }, /*  66 B    */    {1, "C"    }, /*  67 C    */
    {1, "D"    }, /*  68 D    */    {1, "E"    }, /*  69 E    */
    {1, "F"    }, /*  70 F    */    {1, "G"    }, /*  71 G    */
    {1, "H"    }, /*  72 H    */    {1, "I"    }, /*  73 I    */
    {1, "J"    }, /*  74 J    */    {1, "K"    }, /*  75 K    */
    {1, "L"    }, /*  76 L    */    {1, "M"    }, /*  77 M    */
    {1, "N"    }, /*  78 N    */    {1, "O"    }, /*  79 O    */
    {1, "P"    }, /*  80 P    */    {1, "Q"    }, /*  81 Q    */
    {1, "R"    }, /*  82 R    */    {1, "S"    }, /*  83 S    */
    {1, "T"    }, /*  84 T    */    {1, "U"    }, /*  85 U    */
    {1, "V"    }, /*  86 V    */    {1, "W"    }, /*  87 W    */
    {1, "X"    }, /*  88 X    */    {1, "Y"    }, /*  89 Y    */
    {1, "Z"    }, /*  90 Z    */    {1, "["    }, /*  91 [    */
    {2, "\\\\" }, /*  92 \    */    {1, "]"    }, /*  93 ]    */
    {1, "^"    }, /*  94 ^    */    {1, "_"    }, /*  95 _    */
    {1, "`"    }, /*  96 `    */    {1, "a"    }, /*  97 a    */
    {1, "b"    }, /*  98 b    */    {1, "c"    }, /*  99 c    */
    {1, "d"    }, /* 100 d    */    {1, "e"    }, /* 101 e    */
    {1, "f"    }, /* 102 f    */    {1, "g"    }, /* 103 g    */
    {1, "h"    }, /* 104 h    */    {1, "i"    }, /* 105 i    */
    {1, "j"    }, /* 106 j    */    {1, "k"    }, /* 107 k    */
    {1, "l"    }, /* 108 l    */    {1, "m"    }, /* 109 m    */
    {1, "n"    }, /* 110 n    */    {1, "o"    }, /* 111 o    */
    {1, "p"    }, /* 112 p    */    {1, "q"    }, /* 113 q    */
    {1, "r"    }, /* 114 r    */    {1, "s"    }, /* 115 s    */
    {1, "t"    }, /* 116 t    */    {1, "u"    }, /* 117 u    */
    {1, "v"    }, /* 118 v    */    {1, "w"    }, /* 119 w    */
    {1, "x"    }, /* 120 x    */    {1, "y"    }, /* 121 y    */
    {1, "z"    }, /* 122 z    */    {1, "{"    }, /* 123 {    */
    {1, "|"    }, /* 124 |    */    {1, "}"    }, /* 125 }    */
    {1, "~"    }, /* 126 ~    */    {4, "\\x7F"}, /* 127      */
    {4, "\\x80"}, /* 128      */    {4, "\\x81"}, /* 129      */
    {4, "\\x82"}, /* 130      */    {4, "\\x83"}, /* 131      */
    {4, "\\x84"}, /* 132      */    {4, "\\x85"}, /* 133      */
    {4, "\\x86"}, /* 134      */    {4, "\\x87"}, /* 135      */
    {4, "\\x88"}, /* 136      */    {4, "\\x89"}, /* 137      */
    {4, "\\x8A"}, /* 138      */    {4, "\\x8B"}, /* 139      */
    {4, "\\x8C"}, /* 140      */    {4, "\\x8D"}, /* 141      */
    {4, "\\x8E"}, /* 142      */    {4, "\\x8F"}, /* 143      */
    {4, "\\x90"}, /* 144      */    {4, "\\x91"}, /* 145      */
    {4, "\\x92"}, /* 146      */    {4, "\\x93"}, /* 147      */
    {4, "\\x94"}, /* 148      */    {4, "\\x95"}, /* 149      */
    {4, "\\x96"}, /* 150      */    {4, "\\x97"}, /* 151      */
    {4, "\\x98"}, /* 152      */    {4, "\\x99"}, /* 153      */
    {4, "\\x9A"}, /* 154      */    {4, "\\x9B"}, /* 155      */
    {4, "\\x9C"}, /* 156      */    {4, "\\x9D"}, /* 157      */
    {4, "\\x9E"}, /* 158      */    {4, "\\x9F"}, /* 159      */
    {4, "\\xA0"}, /* 160      */    {4, "\\xA1"}, /* 161      */
    {4, "\\xA2"}, /* 162      */    {4, "\\xA3"}, /* 163      */
    {4, "\\xA4"}, /* 164      */    {4, "\\xA5"}, /* 165      */
    {4, "\\xA6"}, /* 166      */    {4, "\\xA7"}, /* 167      */
    {4, "\\xA8"}, /* 168      */    {4, "\\xA9"}, /* 169      */
    {4, "\\xAA"}, /* 170      */    {4, "\\xAB"}, /* 171      */
    {4, "\\xAC"}, /* 172      */    {4, "\\xAD"}, /* 173      */
    {4, "\\xAE"}, /* 174      */    {4, "\\xAF"}, /* 175      */
    {4, "\\xB0"}, /* 176      */    {4, "\\xB1"}, /* 177      */
    {4, "\\xB2"}, /* 178      */    {4, "\\xB3"}, /* 179      */
    {4, "\\xB4"}, /* 180      */    {4, "\\xB5"}, /* 181      */
    {4, "\\xB6"}, /* 182      */    {4, "\\xB7"}, /* 183      */
    {4, "\\xB8"}, /* 184      */    {4, "\\xB9"}, /* 185      */
    {4, "\\xBA"}, /* 186      */    {4, "\\xBB"}, /* 187      */
    {4, "\\xBC"}, /* 188      */    {4, "\\xBD"}, /* 189      */
    {4, "\\xBE"}, /* 190      */    {4, "\\xBF"}, /* 191      */
    {4, "\\xC0"}, /* 192      */    {4, "\\xC1"}, /* 193      */
    {4, "\\xC2"}, /* 194      */    {4, "\\xC3"}, /* 195      */
    {4, "\\xC4"}, /* 196      */    {4, "\\xC5"}, /* 197      */
    {4, "\\xC6"}, /* 198      */    {4, "\\xC7"}, /* 199      */
    {4, "\\xC8"}, /* 200      */    {4, "\\xC9"}, /* 201      */
    {4, "\\xCA"}, /* 202      */    {4, "\\xCB"}, /* 203      */
    {4, "\\xCC"}, /* 204      */    {4, "\\xCD"}, /* 205      */
    {4, "\\xCE"}, /* 206      */    {4, "\\xCF"}, /* 207      */
    {4, "\\xD0"}, /* 208      */    {4, "\\xD1"}, /* 209      */
    {4, "\\xD2"}, /* 210      */    {4, "\\xD3"}, /* 211      */
    {4, "\\xD4"}, /* 212      */    {4, "\\xD5"}, /* 213      */
    {4, "\\xD6"}, /* 214      */    {4, "\\xD7"}, /* 215      */
    {4, "\\xD8"}, /* 216      */    {4, "\\xD9"}, /* 217      */
    {4, "\\xDA"}, /* 218      */    {4, "\\xDB"}, /* 219      */
    {4, "\\xDC"}, /* 220      */    {4, "\\xDD"}, /* 221      */
    {4, "\\xDE"}, /* 222      */    {4, "\\xDF"}, /* 223      */
    {4, "\\xE0"}, /* 224      */    {4, "\\xE1"}, /* 225      */
    {4, "\\xE2"}, /* 226      */    {4, "\\xE3"}, /* 227      */
    {4, "\\xE4"}, /* 228      */    {4, "\\xE5"}, /* 229      */
    {4, "\\xE6"}, /* 230      */    {4, "\\xE7"}, /* 231      */
    {4, "\\xE8"}, /* 232      */    {4, "\\xE9"}, /* 233      */
    {4, "\\xEA"}, /* 234      */    {4, "\\xEB"}, /* 235      */
    {4, "\\xEC"}, /* 236      */    {4, "\\xED"}, /* 237      */
    {4, "\\xEE"}, /* 238      */    {4, "\\xEF"}, /* 239      */
    {4, "\\xF0"}, /* 240      */    {4, "\\xF1"}, /* 241      */
    {4, "\\xF2"}, /* 242      */    {4, "\\xF3"}, /* 243      */
    {4, "\\xF4"}, /* 244      */    {4, "\\xF5"}, /* 245      */
    {4, "\\xF6"}, /* 246      */    {4, "\\xF7"}, /* 247      */
    {4, "\\xF8"}, /* 248      */    {4, "\\xF9"}, /* 249      */
    {4, "\\xFA"}, /* 250      */    {4, "\\xFB"}, /* 251      */
    {4, "\\xFC"}, /* 252      */    {4, "\\xFD"}, /* 253      */
    {4, "\\xFE"}, /* 254      */    {4, "\\xFF"}, /* 255      */
};

/*
 * Returns the number of bytes needed to escape an ascii string.
 */
static size_t
ascii_encoded_strnlen(const char *str, size_t len)
{
    size_t result;
    const unsigned char *s;     /* must be unsigned for table indexing to work */

    for (s = (unsigned char *)str, result = 0; len; s++, len--) {
        result += ascii_encoding_table[*s].len;
    }
    return result;
}

/* ========================================================================== */
static char time_format[] = "%a %b %d %H:%M:%S %Y UTC";
static char hex_chars[] = "0123456789abcdef";
static PyObject *empty_tuple = NULL;
static PyObject *sec_oid_name_to_value = NULL;
static PyObject *sec_oid_value_to_name = NULL;
static PyObject *ckm_name_to_value = NULL;
static PyObject *ckm_value_to_name = NULL;
static PyObject *cka_name_to_value = NULL;
static PyObject *cka_value_to_name = NULL;
static PyObject *general_name_name_to_value = NULL;
static PyObject *general_name_value_to_name = NULL;
static PyObject *crl_reason_name_to_value = NULL;
static PyObject *crl_reason_value_to_name = NULL;

typedef PyObject *(*format_lines_func)(PyObject *self, PyObject *args, PyObject *kwds);

static PyTypeObject PK11SymKeyType;
static PyTypeObject PK11ContextType;
static PyTypeObject SecItemType;
static PyTypeObject PK11SymKeyType;
static PyTypeObject AVAType;
static PyTypeObject RDNType;
static PyTypeObject DNType;

/* === Forward Declarations */

static PyTypeObject CertificateType;


/* === Prototypes === */

static PyObject *
obj_to_hex(PyObject *obj, int octets_per_line, char *separator);

static PyObject *
raw_data_to_hex(unsigned char *data, int data_len, int octets_per_line, char *separator);

static SECStatus
sec_strip_tag_and_length(SECItem *item);

static PyObject *
der_context_specific_secitem_to_pystr(SECItem *item);

static PyObject *
secitem_to_pystr_hex(SECItem *item);

static PyObject *
der_any_secitem_to_pystr(SECItem *item);

static PyObject *
der_set_or_str_secitem_to_pylist_of_pystr(SECItem *item);

static PyObject *
boolean_secitem_to_pystr(SECItem *item);

static PyObject *
der_boolean_secitem_to_pystr(SECItem *item);

static PyObject *
integer_secitem_to_pylong(SECItem *item);

static PyObject *
integer_secitem_to_pystr(SECItem *item);

static PyObject *
der_integer_secitem_to_pystr(SECItem *item);

static bool
is_oid_string(const char *oid_string);

static SECOidTag
ava_name_to_oid_tag(const char *name);

static PyObject *
oid_secitem_to_pystr_desc(SECItem *oid);

static PyObject *
oid_secitem_to_pyint_tag(SECItem *oid);

static PyObject *
oid_secitem_to_pystr_dotted_decimal(SECItem *oid);

static PyObject *
der_oid_secitem_to_pystr_desc(SECItem *item);

static PyObject *
der_utc_time_secitem_to_pystr(SECItem *item);

static PyObject *
der_generalized_time_secitem_to_pystr(SECItem *item);

static PRTime
time_choice_secitem_to_prtime(SECItem *item);

static PyObject *
time_choice_secitem_to_pystr(SECItem *item);

static PyObject *
der_octet_secitem_to_pystr(SECItem *item, int octets_per_line, char *separator);

static PyObject *
ascii_string_secitem_to_escaped_ascii_pystr(SECItem *item);

static PyObject *
der_ascii_string_secitem_to_escaped_ascii_pystr(SECItem *item);

static PyObject *
der_utf8_string_secitem_to_pyunicode(SECItem *item);

static PyObject *
der_bmp_string_secitem_to_pyunicode(SECItem *item);

static PyObject *
der_universal_string_secitem_to_pyunicode(SECItem *item);

static PyObject *
der_bit_string_secitem_to_pystr(SECItem *item);

static PyObject *
der_universal_secitem_to_pystr(SECItem *item);

static PyObject *
ip_addr_secitem_to_pystr(SECItem *item);

static PyObject *
CERTGeneralName_to_pystr_with_label(CERTGeneralName *general_name);

static PyObject *
CERTGeneralName_to_pystr(CERTGeneralName *general_name);

static PyObject *
SECAlgorithmID_to_pystr(SECAlgorithmID *a);

static PyObject *
cert_oid_tag_name(PyObject *self, PyObject *args);

static PyObject *
cert_trust_flags_str(unsigned int flags);

static PyObject *
SecItem_new_from_SECItem(SECItem *item, SECItemKind kind);

static PyObject *
fmt_pair(int level, const char *label, PyObject *obj);

static PyObject *
make_line_pairs(int level, PyObject *src);

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

static PyObject *
key_mechanism_type_to_pystr(CK_MECHANISM_TYPE mechanism);

static PyObject *
pk11_attribute_type_to_pystr(CK_ATTRIBUTE_TYPE type);

static PyObject *
SignedCRL_new_from_CERTSignedCRL(CERTSignedCrl *signed_crl);

static PyObject *
AVA_repr(AVA *self);

static bool
CERTRDN_has_tag(CERTRDN *rdn, int tag);

static PyObject *
CERTAVA_value_to_pystr(CERTAVA *ava);

static Py_ssize_t
CERTRDN_ava_count(CERTRDN *rdn);

static Py_ssize_t
DN_length(DN *self);

static PyObject *
DN_item(DN *self, register Py_ssize_t i);

static PyObject *
general_name_type_to_pystr(CERTGeneralNameType type);

static PyObject *
CERTGeneralName_type_string_to_pystr(CERTGeneralName *general_name);

static PyObject *
CRLDistributionPt_general_names_tuple(CRLDistributionPt *self, RepresentationKind repr_kind);

PyObject *
GeneralName_new_from_CERTGeneralName(CERTGeneralName *name);

static Py_ssize_t
AuthKeyID_general_names_count(AuthKeyID *self);

static PyObject *
AuthKeyID_general_names_tuple(AuthKeyID *self, RepresentationKind repr_kind);

static int
set_thread_local(const char *name, PyObject *obj);

static PyObject *
get_thread_local(const char *name);

static PyObject *
SECItem_der_to_hex(SECItem *item, int octets_per_line, char *separator);

static PyObject *
cert_x509_key_usage(PyObject *self, PyObject *args);

PyObject *
CRLDistributionPts_new_from_SECItem(SECItem *item);

PyObject *
AuthKeyID_new_from_SECItem(SECItem *item);

static PyObject *
cert_x509_ext_key_usage(PyObject *self, PyObject *args, PyObject *kwds);

PyObject *
BasicConstraints_new_from_SECItem(SECItem *item);

static PyObject *
cert_x509_alt_name(PyObject *self, PyObject *args, PyObject *kwds);

PyObject *
DN_new_from_CERTName(CERTName *name);

/* ==================================== */

typedef struct BitStringTableStr {
    int enum_value;
    const char *enum_description;
} BitStringTable;

static BitStringTable CRLReasonDef[] = {
    {crlEntryReasonUnspecified,          _("Unspecified")           }, /* bit 0  */
    {crlEntryReasonKeyCompromise,        _("Key Compromise")        }, /* bit 1  */
    {crlEntryReasonCaCompromise,         _("CA Compromise")         }, /* bit 2  */
    {crlEntryReasonAffiliationChanged,   _("Affiliation Changed")   }, /* bit 3  */
    {crlEntryReasonSuperseded,           _("Superseded")            }, /* bit 4  */
    {crlEntryReasonCessationOfOperation, _("Cessation Of Operation")}, /* bit 5  */
    {crlEntryReasoncertificatedHold,     _("Certificate On Hold")   }, /* bit 6  */
    {-1,                                 NULL,                      }, /* bit 7  */
    {crlEntryReasonRemoveFromCRL,        _("Remove From CRL")       }, /* bit 8  */
    {crlEntryReasonPrivilegeWithdrawn,   _("Privilege Withdrawn")   }, /* bit 9  */
    {crlEntryReasonAaCompromise,         _("AA Compromise")         }, /* bit 10 */
};

static BitStringTable KeyUsageDef[] = {
    {KU_DIGITAL_SIGNATURE, _("Digital Signature")  }, /* bit 0 */
    {KU_NON_REPUDIATION,   _("Non-Repudiation")    }, /* bit 1 */
    {KU_KEY_ENCIPHERMENT,  _("Key Encipherment")   }, /* bit 2 */
    {KU_DATA_ENCIPHERMENT, _("Data Encipherment")  }, /* bit 3 */
    {KU_KEY_AGREEMENT,     _("Key Agreement")      }, /* bit 4 */
    {KU_KEY_CERT_SIGN,     _("Certificate Signing")}, /* bit 5 */
    {KU_CRL_SIGN,          _("CRL Signing")        }, /* bit 6 */
    {KU_ENCIPHER_ONLY,     _("Encipher Only")      }, /* bit 7 */
#ifdef KU_DECIPHER_ONLY
    {KU_DECIPHER_ONLY,     _("Decipher Only")      }, /* bit 8 */
#endif
};

/*
 * NSS WART
 * NSS encodes a bit string in a SECItem by setting the len field
 * to a bit count and stripping off the leading "unused" octet.
 */
static int
der_bitstring_to_nss_bitstring(SECItem *dst, SECItem *src) {
    unsigned long data_len;
    int src_len;
    unsigned char *src_data, octet, unused;

    if (!src || !dst) {
	PORT_SetError(SEC_ERROR_INVALID_ARGS);
        return SECFailure;
    }

    src_len = src->len;
    src_data = src->data;
    
    /* First octet is ASN1 type */
    if (src_len <= 0) goto bad_data;
    octet = *src_data++; src_len--;

    if ((octet & SEC_ASN1_TAGNUM_MASK) != SEC_ASN1_BIT_STRING)
        goto bad_data;

    /* Next octets are ASN1 length */
    if (src_len <= 0) goto bad_data;
    octet = *src_data++; src_len--;

    data_len = octet;
    if (data_len & 0x80) {
        int  len_count = data_len & 0x7f;

        if (len_count > src_len)
            goto bad_data;

        for (data_len = 0, octet = *src_data++, src_len--;
             len_count;
             len_count--,  octet = *src_data++, src_len--) {
            data_len = (data_len << 8) | octet;
        }
    }

    /* After ASN1 length comes one octet containing the unused bit count */
    if (src_len <= 0) goto bad_data;
    unused = *src_data++; src_len--;

    if (data_len <= 1) {
        goto bad_data;
    } else {
        data_len--;             /* account for unused octet */
        dst->len = (data_len << 3) - (unused & 0x07);
        dst->data = src_len > 0 ? src_data : NULL;
    }

    return SECSuccess;

 bad_data:
    PORT_SetError(SEC_ERROR_BAD_DATA);
    return SECFailure;
}

/*
 * Given a decoded bit string in a SECItem (where len is a bit count and
 * the high bit of data[0] is bit[0] of the bitstring) return a tuple of
 * every enabled bit in the bit string. The members of the tuple come from
 * a table of predined values for the bit string. The repr_kind
 * enumeration specifies what type of item should be put in the tuple, for
 * example the string name for the bit position, or the enumerated constant
 * representing that bit postion, or the bit posisiton.
 */
static PyObject *
bitstr_table_to_tuple(SECItem *bitstr, BitStringTable *table,
                      size_t table_len, RepresentationKind repr_kind)
{
    PyObject *tuple = NULL;
    size_t bitstr_len, len, count, i, j;
    unsigned char *data, octet = 0, mask = 0x80;

    bitstr_len = bitstr->len;
    len = MIN(table_len, bitstr_len);

    /*
     * Get a count of how many bits are enabled.
     * Skip any undefined entries in the table.
     */
    count = 0;
    if (bitstr->data != NULL) {
        for (i = 0, data = bitstr->data; i < len; i++) {
            if ((i % 8) == 0) {
                octet = *data++;
                mask = 0x80;
            }
            if (octet & mask) {
                if (table[i].enum_description) { /* only if defined in table */
                    count++;
                }
            }
            mask >>= 1;
        }
    }

    if ((tuple = PyTuple_New(count)) == NULL) {
        return NULL;
    }

    if (count == 0) {
        return tuple;
    }

    /* Populate the tuple */
    for (i = j = 0, data = bitstr->data; i < len; i++) {
        if ((i % 8) == 0) {
            octet = *data++;
            mask = 0x80;
        }
        if (octet & mask) {
            if (table[i].enum_description) { /* only if defined in table */
                switch(repr_kind) {
                case AsEnum:
                    PyTuple_SetItem(tuple, j++, PyInt_FromLong(table[i].enum_value));
                    break;
                case AsEnumDescription:
                    PyTuple_SetItem(tuple, j++, PyString_FromString(table[i].enum_description));
                    break;
                case AsIndex:
                    PyTuple_SetItem(tuple, j++, PyInt_FromLong(i));
                    break;
                default:
                    PyErr_Format(PyExc_ValueError, "Unsupported representation kind (%d)", repr_kind);
                    Py_DECREF(tuple);
                    return NULL;
                    break;
                }
            }
        }
        mask >>= 1;
    }

    return tuple;
}

static PyObject *
crl_reason_bitstr_to_tuple(SECItem *bitstr, RepresentationKind repr_kind)
{
    size_t table_len;

    table_len = sizeof(CRLReasonDef) / sizeof(CRLReasonDef[0]);
    return bitstr_table_to_tuple(bitstr, CRLReasonDef, table_len, repr_kind);
}

static PyObject *
key_usage_bitstr_to_tuple(SECItem *bitstr, RepresentationKind repr_kind)
{
    size_t table_len;

    table_len = sizeof(KeyUsageDef) / sizeof(KeyUsageDef[0]);
    return bitstr_table_to_tuple(bitstr, KeyUsageDef, table_len, repr_kind);
}

static PyObject *
decode_oid_sequence_to_tuple(SECItem *item, RepresentationKind repr_kind)
{
    int i, n_oids;
    PyObject *tuple;
    CERTOidSequence *os;
    SECItem **op;
    PyObject *py_oid;

    if (!item || !item->len || !item->data) {
        PyErr_SetString(PyExc_ValueError, "missing DER encoded OID data");
        return NULL;
    }

    if ((os = CERT_DecodeOidSequence(item)) == NULL) {
        return set_nspr_error("unable to decode OID sequence");
    }

    /* Get a count of how many OID's there were */
    for(op = os->oids, n_oids = 0; *op; op++, n_oids++);

    if ((tuple = PyTuple_New(n_oids)) == NULL) {
        CERT_DestroyOidSequence(os);
        return NULL;
    }

    /* Iterate over each OID and insert into tuple */
    for(op = os->oids, i = 0; *op; op++, i++) {
        switch(repr_kind) {
        case AsObject:
            if ((py_oid = SecItem_new_from_SECItem(*op, SECITEM_oid)) == NULL) {
                Py_DECREF(tuple);
                CERT_DestroyOidSequence(os);
                return NULL;
            }
            break;
        case AsString:
            if ((py_oid = oid_secitem_to_pystr_desc(*op)) == NULL) {
                Py_DECREF(tuple);
                CERT_DestroyOidSequence(os);
                return NULL;
            }
            break;
        case AsDottedDecimal:
            if ((py_oid = oid_secitem_to_pystr_dotted_decimal(*op)) == NULL) {
                Py_DECREF(tuple);
                CERT_DestroyOidSequence(os);
                return NULL;
            }
            break;
        case AsEnum:
            if ((py_oid = oid_secitem_to_pyint_tag(*op)) == NULL) {
                Py_DECREF(tuple);
                CERT_DestroyOidSequence(os);
                return NULL;
            }
            break;
        default:
            PyErr_Format(PyExc_ValueError, "Unsupported representation kind (%d)", repr_kind);
            Py_DECREF(tuple);
            CERT_DestroyOidSequence(os);
            return NULL;
        }
        PyTuple_SetItem(tuple, i, py_oid);
    }
    CERT_DestroyOidSequence(os);

    return tuple;
}

/* NSS WART: CERT_CopyAVA is hidden, but we need it, copied here from secname.c */ 
CERTAVA *
CERT_CopyAVA(PRArenaPool *arena, CERTAVA *from)
{
    CERTAVA *ava;
    int rv;

    ava = (CERTAVA*) PORT_ArenaZAlloc(arena, sizeof(CERTAVA));
    if (ava) {
	rv = SECITEM_CopyItem(arena, &ava->type, &from->type);
	if (rv) goto loser;
	rv = SECITEM_CopyItem(arena, &ava->value, &from->value);
	if (rv) goto loser;
    }
    return ava;

  loser:
    return NULL;
}

static SECStatus
CERT_CopyGeneralName(PRArenaPool *arena, CERTGeneralName **pdst, CERTGeneralName *src)
{
    SECStatus result = SECSuccess;
    void *mark = NULL;
    CERTGeneralName *dst;

    /*
     * NSS WART
     * There is no public API to create a CERTGeneralName, copy it, or free it.
     * You don't know what arena was used to create the general name.
     * GeneralNames are linked in a list, this makes it difficult for a 
     * general name to exist independently, it would have been better if there
     * was a list container independent general names could be placed in,
     * then you wouldn't have to worry about the link fields in each independent name.
     * 
     * The logic here is copied from cert_CopyOneGeneralName in certdb/genname.c
     */

    if (!arena) {
	PORT_SetError(SEC_ERROR_INVALID_ARGS);
        return SECFailure;
    }

    if (!src) {
	PORT_SetError(SEC_ERROR_INVALID_ARGS);
        return SECFailure;
    }

    mark = PORT_ArenaMark(arena);

    if ((dst = PORT_ArenaZNew(arena, CERTGeneralName)) == NULL) {
        result = SECFailure;
        goto exit;
    }

    dst->l.prev = dst->l.next = &dst->l;
    dst->type = src->type;

    switch (src->type) {
    case certDirectoryName: 
	if ((result = SECITEM_CopyItem(arena, &dst->derDirectoryName, 
                                       &src->derDirectoryName)) != SECSuccess) {
            goto exit;
        }
        if ((result = CERT_CopyName(arena, &dst->name.directoryName, 
                                    &src->name.directoryName)) != SECSuccess) {
            goto exit;
        }
	break;

    case certOtherName: 
	if ((result = SECITEM_CopyItem(arena, &dst->name.OthName.name, 
                                       &src->name.OthName.name)) != SECSuccess) {
            goto exit;
        }
        if ((result = SECITEM_CopyItem(arena, &dst->name.OthName.oid, 
                                       &src->name.OthName.oid)) != SECSuccess) {
            goto exit;
        }
	break;

    default: 
	if ((result = SECITEM_CopyItem(arena, &dst->name.other, 
                                       &src->name.other)) != SECSuccess) {
            goto exit;
        }
	break;

    }

    
 exit:
    if (result == SECSuccess) {
        *pdst = dst;
        PORT_ArenaUnmark(arena, mark);
    } else {
        *pdst = NULL;
        PORT_ArenaRelease(arena, mark);
    }
    return result;
}

static Py_ssize_t
CERTGeneralName_list_count(CERTGeneralName *head)
{
    CERTGeneralName *cur;
    Py_ssize_t count;

    count = 0;
    if (!head) {
        return count;
    }

    cur = head;
    do {
        count++;
        cur = CERT_GetNextGeneralName(cur);
    } while (cur != head);

    return count;
}

static PyObject *
CERTGeneralName_list_to_tuple(CERTGeneralName *head, RepresentationKind repr_kind)
{
    CERTGeneralName *cur;
    Py_ssize_t n_names, i;
    PyObject *names;

    n_names = CERTGeneralName_list_count(head);

    if ((names = PyTuple_New(n_names)) == NULL) {
        return NULL;
    }

    if (n_names == 0) {
        return names;
    }

    i = 0;
    cur = head;
    do {
        PyObject *name;

        switch(repr_kind) {
        case AsObject:
            name = GeneralName_new_from_CERTGeneralName(cur);
            break;
        case AsString:
            name = CERTGeneralName_to_pystr(cur);
            break;
        case AsTypeString:
            name = CERTGeneralName_type_string_to_pystr(cur);
            break;
        case AsTypeEnum:
            name = PyInt_FromLong(cur->type);
            break;
        case AsLabeledString:
            name = CERTGeneralName_to_pystr_with_label(cur);
            break;
        default:
            PyErr_Format(PyExc_ValueError, "Unsupported representation kind (%d)", repr_kind);
            Py_DECREF(names);
            return NULL;
        }
        PyTuple_SetItem(names, i, name);
        cur = CERT_GetNextGeneralName(cur);
        i++;
    } while (cur != head);


    return names;
}

static SECStatus
CERT_CopyGeneralNameList(PRArenaPool *arena, CERTGeneralName **pdst, CERTGeneralName *src)
{
    SECStatus result = SECSuccess;
    void *mark = NULL;
    CERTGeneralName *src_head, *dst_head;
    CERTGeneralName *cur, *prev;

    /*
     * NSS WART
     * There is no publice API to copy a list of GeneralNames.
     *
     * GeneralNames are an exception to all other NSS data containers.
     * Normally homogeneous collections are stored in a array of pointers to 
     * the items with the last array element being NULL. However GeneralNames are
     * exception, they embed a linked list for assembling them in a list. Not only
     * is this an awkward deviation but it means the GeneralName cannot belong to
     * more than one collection.
     *
     * The logic here is copied from CERT_CopyGeneralName in certdb/genname.c
     *
     * The linked list is circular. The logic to stop traversal is if the link
     * pointed to by CERT_GetNextGeneralName/CERT_GetPrevGeneralName is the same
     * as the GeneralName the traversal started with.
     */

    if (!arena) {
	PORT_SetError(SEC_ERROR_INVALID_ARGS);
        return SECFailure;
    }

    if (!src) {
	PORT_SetError(SEC_ERROR_INVALID_ARGS);
        return SECFailure;
    }

    mark = PORT_ArenaMark(arena);

    src_head = src;
    dst_head = cur = NULL;
    do {
        prev = cur;
        if (CERT_CopyGeneralName(arena, &cur, src) != SECSuccess) {
            result = SECFailure;
            goto exit;
        }
        if (dst_head == NULL) { /* first node */
            dst_head = cur;
            prev = cur;
        }

        cur->l.next = &dst_head->l; /* tail node's next points to head */
        cur->l.prev = &prev->l;     /* tail node's prev points to prev */
        dst_head->l.prev = &cur->l; /* head's prev points to tail node */
        prev->l.next = &cur->l;     /* prev node's next point to tail node */

        src = CERT_GetNextGeneralName(src);
    } while (src != src_head);

 exit:
    if (result == SECSuccess) {
        *pdst = dst_head;
        PORT_ArenaUnmark(arena, mark);
    } else {
        *pdst = NULL;
        PORT_ArenaRelease(arena, mark);
    }
    return result;
}

static SECStatus
CERT_CopyCRLDistributionPoint(PRArenaPool *arena, CRLDistributionPoint **pdst, CRLDistributionPoint *src)
{
    SECStatus result = SECSuccess;
    CERTRDN *rdn;
    void *mark = NULL;
    CRLDistributionPoint *dst;
    SECItem tmp_item;

    /*
     * NSS WART
     * There is no public API to create a CRLDistributionPoint or copy it.
     */
    mark = PORT_ArenaMark(arena);

    if ((dst = PORT_ArenaZNew(arena, CRLDistributionPoint)) == NULL) {
        result = SECFailure;
        goto exit;
    }        
    
    switch((dst->distPointType = src->distPointType)) {
    case generalName:
        if ((result = CERT_CopyGeneralNameList(arena,
                                               &dst->distPoint.fullName,
                                               src->distPoint.fullName)) != SECSuccess) {
            goto exit;
        }
        break;
    case relativeDistinguishedName:
        if ((rdn = CERT_CreateRDN(arena, NULL)) == NULL) {
            result = SECFailure;
            goto exit;
        }
        dst->distPoint.relativeName = *rdn;
        if ((result = CERT_CopyRDN(arena,
                                   &dst->distPoint.relativeName,
                                   &src->distPoint.relativeName)) != SECSuccess) {
            goto exit;
        }
        break;
    default:
        PORT_SetError(SEC_ERROR_INVALID_ARGS);
        result = SECFailure;
        goto exit;
    }

    if ((result = SECITEM_CopyItem(arena, &dst->reasons, &src->reasons)) != SECSuccess) {
        goto exit;
    }
    
    /*
     * WARNING: NSS WART
     * src->bitsmap is a SECItem whose length is a bit count and whose data
     * omits the leading DER bitstring "unused" octet.
     */
    
    tmp_item = src->bitsmap;
    DER_ConvertBitString(&tmp_item); /* make len a byte count */
    if ((result = SECITEM_CopyItem(arena, &dst->bitsmap, &tmp_item)) != SECSuccess) {
        goto exit;
    }
    dst->bitsmap.len = src->bitsmap.len;
    
    if (src->crlIssuer) {
        if ((result = CERT_CopyGeneralName(arena, &dst->crlIssuer, src->crlIssuer)) != SECSuccess) {
            goto exit;
        }
    }
    
    /*
     * WARNING: we don't copy these because they're only used during decoding:
     * derDistPoint, derRelativeName, derCrlIssuer, derFullName
     */

 exit:
    if (result == SECSuccess) {
        *pdst = dst;
        PORT_ArenaUnmark(arena, mark);
    } else {
        *pdst = NULL;
        PORT_ArenaRelease(arena, mark);
    }
    return result;
}

/*
 * NSS WART
 * There is no public API to copy a CERTAuthKeyID
 */
static SECStatus
CERT_CopyAuthKeyID(PRArenaPool *arena, CERTAuthKeyID **pdst, CERTAuthKeyID *src)
{
    SECStatus result = SECSuccess;
    void *mark = NULL;
    CERTAuthKeyID *dst;

    mark = PORT_ArenaMark(arena);

    if ((dst = PORT_ArenaZNew(arena, CERTAuthKeyID)) == NULL) {
        result = SECFailure;
        goto exit;
    }        
    
    if ((result = SECITEM_CopyItem(arena, &dst->keyID, &src->keyID)) != SECSuccess) {
        goto exit;
    }

    if ((result = CERT_CopyGeneralNameList(arena, &dst->authCertIssuer,
                                           src->authCertIssuer)) != SECSuccess) {
        goto exit;
    }

    if ((result = SECITEM_CopyItem(arena, &dst->authCertSerialNumber,
                                   &src->authCertSerialNumber)) != SECSuccess) {
        goto exit;
    }

 exit:
    if (result == SECSuccess) {
        *pdst = dst;
        PORT_ArenaUnmark(arena, mark);
    } else {
        *pdst = NULL;
        PORT_ArenaRelease(arena, mark);
    }
    return result;
}

static int
oid_tag_from_name(const char *name)
{
    PyObject *py_name;
    PyObject *py_lower_name;
    PyObject *py_value;
    int oid_tag;

    if ((py_name = PyString_FromString(name)) == NULL) {
        return -1;
    }

    if ((py_lower_name = PyObject_CallMethod(py_name, "lower", NULL)) == NULL) {
        Py_DECREF(py_name);
        return -1;
    }

    if ((py_value = PyDict_GetItem(sec_oid_name_to_value, py_lower_name)) == NULL) {
	PyErr_Format(PyExc_KeyError, "oid tag name not found: %s", PyString_AsString(py_name));
        Py_DECREF(py_name);
        Py_DECREF(py_lower_name);
        return -1;
    }

    oid_tag = PyInt_AsLong(py_value);

    Py_DECREF(py_name);
    Py_DECREF(py_lower_name);

    return oid_tag;
}

static PyObject *
oid_tag_name_from_tag(int oid_tag)
{
    PyObject *py_value;
    PyObject *py_name;

    if ((py_value = PyInt_FromLong(oid_tag)) == NULL) {
        return NULL;
    }

    if ((py_name = PyDict_GetItem(sec_oid_value_to_name, py_value)) == NULL) {
	PyErr_Format(PyExc_KeyError, "oid tag not found: %#x", oid_tag);
        Py_DECREF(py_value);
        return NULL;
    }

    Py_DECREF(py_value);
    Py_INCREF(py_name);

    return py_name;
}

static int
get_oid_tag_from_object(PyObject *obj)
{
    int oid_tag = SEC_OID_UNKNOWN;

    if (PyString_Check(obj) || PyUnicode_Check(obj)) {
        PyObject *py_obj_string_utf8 = NULL;
        char *type_string;

        if (PyString_Check(obj)) {
            py_obj_string_utf8 = obj;
            Py_INCREF(py_obj_string_utf8);
        } else {
            py_obj_string_utf8 = PyUnicode_AsUTF8String(obj);
        }

        if ((type_string = PyString_AsString(py_obj_string_utf8)) == NULL) {
            Py_DECREF(py_obj_string_utf8);
            return -1;
        }

        /*
         * First see if it's a canonical name,
         * if not try a dotted-decimal OID,
         * if not then try tag name.
         */
        if ((oid_tag = ava_name_to_oid_tag(type_string)) == SEC_OID_UNKNOWN) {
            if (is_oid_string(type_string)) { /* is dotted-decimal OID */
                SECItem item;

                item.data = NULL;
                item.len = 0;

                /* Convert dotted-decimal OID string to SECItem */
                if (SEC_StringToOID(NULL, &item, type_string, 0) != SECSuccess) {
                    Py_DECREF(py_obj_string_utf8);
                    PyErr_Format(PyExc_ValueError, "failed to convert oid string \"%s\" to SECItem",
                                 type_string);
                    return -1;
                }
                /* Get the OID tag from the SECItem */
                if ((oid_tag = SECOID_FindOIDTag(&item)) == SEC_OID_UNKNOWN) {
                    Py_DECREF(py_obj_string_utf8);
                    SECITEM_FreeItem(&item, PR_FALSE);
                    PyErr_Format(PyExc_ValueError, "could not convert \"%s\" to OID tag", type_string);
                    return -1;
                }
                SECITEM_FreeItem(&item, PR_FALSE);
            } else {
                oid_tag = oid_tag_from_name(type_string);
            }
        }
	Py_DECREF(py_obj_string_utf8);
    } else if (PyInt_Check(obj)) {
        oid_tag = PyInt_AsLong(obj);
    } else if (PySecItem_Check(obj)) {
        oid_tag = SECOID_FindOIDTag(&((SecItem *)obj)->item);
    } else {
        PyErr_Format(PyExc_TypeError, "oid must be a string, an integer, or a SecItem, not %.200s",
                     Py_TYPE(obj)->tp_name);
        return -1;
    }

    return oid_tag;
}

static bool
is_oid_string(const char *oid_string)
{
    const char *p;
    int n_integers, n_dots;

    n_integers = n_dots = 0;
    p = oid_string;
    if (strncasecmp("OID.", p, 4) == 0) p += 4;    /* skip optional OID. prefix */
    while (*p) {
        if (isdigit(*p)) {
            n_integers++;
            for (p++; *p && isdigit(*p); p++); /* consume rest of digits in integer */
        } else if (*p == '.') {                /* found a dot */
            n_dots++;
            p++;
        } else {                               /* not a dot or digit */
            if (isspace(*p)) {                 /* permit trailing white space */
                for (p++; *p && isspace(*p); p++);
                if (!*p) break;
            }
            return false;
        }
    }

    return (n_integers > 0) && (n_integers == n_dots+1);
}

static const char *
ava_oid_tag_to_name(SECOidTag tag)
{
    const DnAvaProps *ava = dn_ava_props;

    for (ava = dn_ava_props;
         ava->oid_tag != tag && ava->oid_tag != SEC_OID_UNKNOWN;
         ava++);

    return (ava->oid_tag != SEC_OID_UNKNOWN) ? ava->name : NULL;
}

static int
ava_oid_tag_to_value_type(SECOidTag tag)
{
    const DnAvaProps *ava = dn_ava_props;

    for (ava = dn_ava_props;
         ava->oid_tag != tag && ava->oid_tag != SEC_OID_UNKNOWN;
         ava++);

    return (ava->oid_tag != SEC_OID_UNKNOWN) ? ava->value_type : SEC_ASN1_UTF8_STRING;
}

/*
 * Given a canonical ava name (e.g. "CN") return the oid tag for it. Case
 * is not significant. If not found SEC_OID_UNKNOWN is returned.
 */
static SECOidTag
ava_name_to_oid_tag(const char *name)
{
    const DnAvaProps *ava = dn_ava_props;

    for (ava = dn_ava_props;
         ava->oid_tag != SEC_OID_UNKNOWN && strcasecmp(ava->name, name);
         ava++);

    return ava->oid_tag;
}


static int
_AddIntConstantWithLookup(PyObject *module, const char *name, long value, const char *prefix,
                          PyObject *name_to_value, PyObject *value_to_name)
{
    PyObject *module_dict;
    PyObject *py_name = NULL;
    PyObject *py_name_sans_prefix = NULL;
    PyObject *py_lower_name = NULL;
    PyObject *py_value = NULL;

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

    if ((py_lower_name = PyObject_CallMethod(py_name, "lower", NULL)) == NULL) {
        Py_DECREF(py_name);
        return -1;
    }

    if ((py_value = PyInt_FromLong(value)) == NULL) {
        Py_DECREF(py_name);
        Py_DECREF(py_lower_name);
        return -1;
    }

    if (PyDict_SetItem(module_dict, py_name, py_value) != 0) {
        Py_DECREF(py_name);
        Py_DECREF(py_lower_name);
        Py_DECREF(py_value);
        return -1;
    }

    if (PyDict_SetItem(value_to_name, py_value, py_name) != 0) {
        Py_DECREF(py_name);
        Py_DECREF(py_lower_name);
        Py_DECREF(py_value);
        return -1;
    }

    if (PyDict_SetItem(name_to_value, py_lower_name, py_value) != 0) {
        Py_DECREF(py_name);
        Py_DECREF(py_lower_name);
        Py_DECREF(py_value);
        return -1;
    }

    if (prefix) {
        size_t prefix_len = strlen(prefix);

        if (strlen(name) > prefix_len &&
            strncasecmp(prefix, name, prefix_len) == 0) {

            if ((py_name_sans_prefix = PyString_FromString(PyString_AS_STRING(py_lower_name) + prefix_len)) == NULL) {
                Py_DECREF(py_name);
                Py_DECREF(py_lower_name);
                Py_DECREF(py_value);
                return -1;
            }

            if (PyDict_SetItem(name_to_value, py_name_sans_prefix, py_value) != 0) {
                Py_DECREF(py_name);
                Py_DECREF(py_name_sans_prefix);
                Py_DECREF(py_lower_name);
                Py_DECREF(py_value);
                return -1;
            }
        }
    }

    Py_DECREF(py_name);
    Py_XDECREF(py_name_sans_prefix);
    Py_DECREF(py_lower_name);
    Py_DECREF(py_value);
    return 0;
}

static int
set_thread_local(const char *name, PyObject *obj)
{
    PyObject *tdict;
    PyObject *thread_local_dict;

    /* Get this threads thread local dict */
    if ((tdict = PyThreadState_GetDict()) == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "cannot get thread state");
        return -1;
    }

    /* Get our (i.e. NSS's) thread local dict */
    if ((thread_local_dict = PyDict_GetItemString(tdict, NSS_THREAD_LOCAL_KEY)) == NULL) {
        /*
         * Our thread local dict does not yet exist so create it
         * and set it in the thread's thread local dict.
         */
        if ((thread_local_dict = PyDict_New()) == NULL) {
            PyErr_SetString(PyExc_RuntimeError, "cannot create thread local data dict");
            return -1;
        }
        if (PyDict_SetItemString(tdict, NSS_THREAD_LOCAL_KEY, thread_local_dict) < 0) {
            Py_DECREF(thread_local_dict);
            PyErr_SetString(PyExc_RuntimeError, "cannot store thread local data dict");
            return -1;
        }
    }

    if (PyDict_SetItemString(thread_local_dict, name, obj) < 0) {
        PyErr_SetString(PyExc_RuntimeError, "cannot store object in thread local data dict");
        return -1;
    }
    
    return 0;
}

/* Same return behavior as PyDict_GetItem() */
static PyObject *
get_thread_local(const char *name)
{
    PyObject *tdict;
    PyObject *thread_local_dict;

    /* Get this threads thread local dict */
    if ((tdict = PyThreadState_GetDict()) == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "cannot get thread state");
        return NULL;
    }

    /* Get our (i.e. NSS's) thread local dict */
    if ((thread_local_dict = PyDict_GetItemString(tdict, NSS_THREAD_LOCAL_KEY)) == NULL) {
        /*
         * Our thread local dict does not yet exist thus the item can't be
         * in the dict, thus it's not found.
         */
        return NULL;
    }

    return PyDict_GetItemString(thread_local_dict, name);
}

static int
SecItemOrNoneConvert(PyObject *obj, PyObject **param)
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

static int
SymKeyOrNoneConvert(PyObject *obj, PyObject **param)
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


static const char *
oid_tag_str(SECOidTag tag)
{
    static char buf[80];

    SECOidData *oiddata;

    if ((oiddata = SECOID_FindOIDByTag(tag)) != NULL) {
	return oiddata->desc;
    }
    snprintf(buf, sizeof(buf), "unknown(%#x)", tag);
    return buf;
}

static PyObject *
obj_sprintf(const char *fmt, ...)
{
    va_list va;
    Py_ssize_t n_fmts, i;
    PyObject *args = NULL;
    PyObject *obj = NULL;
    PyObject *py_fmt = NULL;
    PyObject *result = NULL;
    const char *s;
    
    for (s = fmt, n_fmts = 0; *s; s++) {
        if (*s == '%') {
            if (s > fmt) {
                if (s[-1] != '%') {
                    n_fmts++;
                }
            } else {
                n_fmts++;
            }
        }
    }

    if ((args = PyTuple_New(n_fmts)) == NULL) {
        return NULL;
    }

    va_start(va, fmt);
    for (i = 0; i < n_fmts; i++) {
        obj = va_arg(va, PyObject *);
        Py_INCREF(obj);
        PyTuple_SetItem(args, i, obj);
    }
    va_end(va);

    if ((py_fmt = PyString_FromString(fmt)) == NULL) {
        Py_DECREF(args);
        return NULL;
    }

    result = PyString_Format(py_fmt, args);
    Py_DECREF(py_fmt);
    Py_DECREF(args);

    return result;
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


/* Steals reference to obj_str */
static PyObject *
line_pair(int level, PyObject *obj_str) {
    PyObject *pair = NULL;

    if ((pair = PyTuple_New(2)) == NULL) {
        return NULL;
    }

    PyTuple_SetItem(pair, 0, PyInt_FromLong(level));
    PyTuple_SetItem(pair, 1, obj_str);

    return pair;
}

static PyObject *
make_line_pairs(int level, PyObject *src)
{
    PyObject *lines = NULL;
    PyObject *obj = NULL;
    PyObject *pair = NULL;
    PyObject *seq = NULL;
    Py_ssize_t n_objs, i;

    if (PyList_Check(src) || PyTuple_Check(src)) {
        seq = src;
        n_objs = PySequence_Size(seq);
        Py_INCREF(seq);
    } else {
        obj = src;
        Py_INCREF(obj);
        n_objs = 1;
    }

    if ((lines = PyList_New(n_objs)) == NULL) {
        goto exit;
    }

    if (seq) {
        for (i = 0; i < n_objs; i++) {
            if ((obj = PySequence_GetItem(seq, i)) == NULL) { /* new reference */
                Py_DECREF(lines);
                goto exit;
            }
            if ((pair = fmt_pair(level, NULL, obj)) == NULL) {
                Py_DECREF(lines);
                goto exit;
            }
            PyList_SetItem(lines, i, pair);
            Py_CLEAR(obj);
        }
    } else {
        if ((pair = fmt_pair(level, NULL, obj)) == NULL) {
            Py_DECREF(lines);
            goto exit;
        }
        PyList_SetItem(lines, 0, pair);
    }

 exit:
    Py_XDECREF(obj);
    Py_XDECREF(seq);
    return lines;
}

PyDoc_STRVAR(nss_make_line_pairs_doc,
"make_line_pairs(level, obj) -> [(level, str), ...]\n\
\n\
:Parameters:\n\
    obj : object\n\
        If obj is a tuple or list then each member will be wrapped\n\
        in a 2-tuple of (level, str). If obj is a scalar object\n\
        then obj will be wrapped in a 2-tuple of (level, obj)\n\
    level : integer\n\
        Initial indentation level, all subsequent indents are relative\n\
        to this starting level.\n\
\n\
Return a list of 2-tuple line pairs sutible to passing to\n\
indented_format(). Each tuple pair consists of a integer\n\
level value and a string object. This is equivalent to:\n\
[(level, str(x)) for x in obj].\n\
As a special case convenience if obj is a scalar object (i.e.\n\
not a list or tuple) then [(level, str(obj))] will be returned.\n\
");

static PyObject *
nss_make_line_pairs(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"level", "obj", NULL};
    int level = 0;
    PyObject *obj;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "iO:make_line_pairs", kwlist,
                                     &level, &obj))
        return NULL;

    return make_line_pairs(level, obj);
}

static PyObject *
fmt_pair(int level, const char *label, PyObject *obj)
{
    PyObject *obj_str = NULL;

    if (PyString_Check(obj) || PyUnicode_Check(obj)) {
        Py_INCREF(obj);
        obj_str = obj;
    } else {
        if ((obj_str = PyObject_Str(obj)) == NULL) {
            return NULL;
        }
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

    return line_pair(level, obj_str); /* steals reference to obj_str */
}

static PyObject *
fmt_label(int level, char *label)
{
    PyObject *pair = NULL;
    PyObject *label_str = NULL;

    if (label) {
        if ((label_str = PyString_FromFormat("%s:", label)) == NULL) {
            return NULL;
        }
    } else {
        if ((label_str = PyString_FromString("")) == NULL) {
            return NULL;
        }
    }

    if ((pair = PyTuple_New(2)) == NULL) {
        return NULL;
    }

    PyTuple_SetItem(pair, 0, PyInt_FromLong(level));
    PyTuple_SetItem(pair, 1, label_str);

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
        if ((py_indent = PyString_FromString("    ")) == NULL) {
            goto fail;
        }
    } else {
        Py_INCREF(py_indent);
    }

    if ((tmp_args = Py_BuildValue("(i)", level)) == NULL) {
        goto fail;
    }
    if ((py_lines = formatter(self, tmp_args, NULL)) == NULL) {
        goto fail;
    }
    Py_CLEAR(tmp_args);

    if ((tmp_args = Py_BuildValue("OO", py_lines, py_indent)) == NULL) {
        goto fail;
    }
    if ((py_formatted_result = nss_indented_format(NULL, tmp_args, NULL)) == NULL) {
        goto fail;
    }

    Py_DECREF(tmp_args);
    Py_DECREF(py_lines);
    Py_DECREF(py_indent);
    return py_formatted_result;

 fail:
    Py_XDECREF(tmp_args);
    Py_XDECREF(py_lines);
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
"data_to_hex(data, octets_per_line=0, separator=':') -> string or list of strings\n\
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
separator will be used.\n\
");

static PyObject *
cert_data_to_hex(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"data", "octets_per_line", "separator", NULL};
    PyObject *obj = NULL;
    int octets_per_line = 0;
    char *separator = HEX_SEPARATOR_DEFAULT;

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

/* ================== Convert NSS Object to Python String =================== */

static PyObject *
CERTName_to_pystr(CERTName *cert_name)
{
    char *name;
    PyObject *py_name = NULL;

    if (!cert_name) {
        return PyString_FromString("");
    }

    if ((name = CERT_NameToAscii(cert_name)) == NULL) {
        return PyString_FromString("");
    }

    py_name = PyString_FromString(name);
    PORT_Free(name);
    return py_name;
}


static PyObject *
der_context_specific_secitem_to_pystr(SECItem *item)
{
    PyObject *py_str = NULL;
    PyObject *hex_str = NULL;
    int type        = item->data[0] & SEC_ASN1_TAGNUM_MASK;
    int constructed = item->data[0] & SEC_ASN1_CONSTRUCTED;
    SECItem tmp;

    if (constructed) {
        py_str = PyString_FromFormat("[%d]", type);
    } else {
        tmp = *item;
        if (sec_strip_tag_and_length(&tmp) == SECSuccess) {
            if ((hex_str = raw_data_to_hex(tmp.data, tmp.len, 0, HEX_SEPARATOR_DEFAULT))) {
                py_str = PyString_FromFormat("[%d] %s", type, PyString_AsString(hex_str));
                Py_DECREF(hex_str);
            }
        }
        if (!py_str) {
            py_str = PyString_FromFormat("[%d]", type);
        }
    }

    return py_str;
}

static PyObject *
secitem_to_pystr_hex(SECItem *item)
{
    return raw_data_to_hex(item->data, item->len, 0, HEX_SEPARATOR_DEFAULT);
}

static PyObject *
der_any_secitem_to_pystr(SECItem *item)
{
    if (item && item->len && item->data) {
	switch (item->data[0] & SEC_ASN1_CLASS_MASK) {
	case SEC_ASN1_CONTEXT_SPECIFIC:
	    return der_context_specific_secitem_to_pystr(item);
	    break;
	case SEC_ASN1_UNIVERSAL:
	    return der_universal_secitem_to_pystr(item);
	    break;
	default:
	    return raw_data_to_hex(item->data, item->len, 0, HEX_SEPARATOR_DEFAULT);
	}
    }
    return PyString_FromString("(null)");
}


/* return a ASN1 SET or SEQUENCE as a list of strings */
static PyObject *
der_set_or_str_secitem_to_pylist_of_pystr(SECItem *item)
{
    int type        = item->data[0] & SEC_ASN1_TAGNUM_MASK;
    int constructed = item->data[0] & SEC_ASN1_CONSTRUCTED;
    char *label = NULL;
    SECItem tmp_item = *item;
    PyObject *py_items = NULL;
    PyObject *py_item = NULL;

    if (!constructed) {
        return raw_data_to_hex(item->data, item->len, 0, HEX_SEPARATOR_DEFAULT);
    }

    if (sec_strip_tag_and_length(&tmp_item) != SECSuccess) {
        Py_RETURN_NONE;
    }

    if ((py_items = PyList_New(0)) == NULL) {
        return NULL;
    }

    if (type == SEC_ASN1_SET)
    	label = "Set ";
    else if (type == SEC_ASN1_SEQUENCE)
    	label = "Sequence ";
    else
    	label = "";

    while (tmp_item.len >= 2) {
	SECItem  tmp_item = tmp_item;

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
	if (tmp_item.len > tmp_item.len) {
	    tmp_item.len = tmp_item.len;
	}
	tmp_item.data += tmp_item.len;
	tmp_item.len  -= tmp_item.len;

        py_item = der_any_secitem_to_pystr(&tmp_item);
        PyList_Append(py_items, py_item);
    }

    return py_items;
}

static PyObject *
boolean_secitem_to_pystr(SECItem *item)
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
der_boolean_secitem_to_pystr(SECItem *item)
{
    PyObject *str = NULL;
    SECItem tmp_item = *item;

    if (sec_strip_tag_and_length(&tmp_item) == SECSuccess)
	str = boolean_secitem_to_pystr(&tmp_item);

    return str;
}

/*
 * Decodes ASN1 integer. Properly handles large magnitude.
 * PyInt object returned if value fits, PyLong object otherwise.
 * 
 * item is a decoded ASN1 integer, if the integer is a DER encoded
 * integer with a tag and length then call encoded_integer_secitem_to_pylong
 */
static PyObject *
integer_secitem_to_pylong(SECItem *item)
{
    int len;
    unsigned char *data, octet;
    PyObject *l = NULL;
    PyObject *eight = NULL;
    PyObject *new_bits = NULL;
    PyObject *tmp = NULL;
    bool negative;

    if (!item || !item->len || !item->data) {
        return PyInt_FromLong(0);
    }

    len = item->len;
    data = item->data;
    octet = *data++;
    negative = octet & 0x80;

    if (negative) {
        if ((l = PyInt_FromLong(-1)) == NULL) {
            goto error;
        }
    } else {
        if ((l = PyInt_FromLong(0)) == NULL) {
            goto error;
        }      
    }

    if ((eight = PyInt_FromLong(8)) == NULL) {
        return NULL;
    }

    do {
        if ((new_bits = PyInt_FromLong(octet)) == NULL) {
            goto error;
        }

        if ((tmp = PyNumber_Lshift(l, eight)) == NULL) {
            goto error;
        }

        Py_CLEAR(l);
        
        if ((l = PyNumber_Or(tmp, new_bits)) == NULL) {
            goto error;
        }
        
        Py_CLEAR(tmp);
        Py_CLEAR(new_bits);
        
        octet = *data++;
        len--;
    } while (len);
    
    goto exit;

 error:
    Py_CLEAR(l);
 exit:
    Py_XDECREF(eight);
    Py_XDECREF(new_bits);
    Py_XDECREF(tmp);
    return l;
}

static PyObject *
integer_secitem_to_pystr(SECItem *item)
{
    PyObject *py_int = NULL;
    PyObject *py_str = NULL;

    if ((py_int = integer_secitem_to_pylong(item)) == NULL) {
        return NULL;
    }

    py_str = PyObject_Str(py_int);

    Py_DECREF(py_int);
    return py_str;
}

static PyObject *
der_integer_secitem_to_pystr(SECItem *item)
{
    PyObject *py_str = NULL;
    SECItem tmp_item = *item;

    if (sec_strip_tag_and_length(&tmp_item) == SECSuccess)
	py_str = integer_secitem_to_pystr(&tmp_item);

    return py_str;
}

static PyObject *
oid_secitem_to_pystr_desc(SECItem *oid)
{
    SECOidData *oiddata;
    char *oid_string = NULL;
    PyObject *py_oid_str = NULL;

    if ((oiddata = SECOID_FindOID(oid)) != NULL) {
	return PyString_FromString(oiddata->desc);
    }
    if ((oid_string = CERT_GetOidString(oid)) != NULL) {
        py_oid_str = PyString_FromString(oid_string);
	PR_smprintf_free(oid_string);
	return py_oid_str;
    }
    return obj_to_hex((PyObject *)oid, 0, HEX_SEPARATOR_DEFAULT);
}

static PyObject *
oid_secitem_to_pyint_tag(SECItem *oid)
{
    SECOidTag oid_tag;

    oid_tag = SECOID_FindOIDTag(oid);
    return PyInt_FromLong(oid_tag);
}

static PyObject *
oid_secitem_to_pystr_dotted_decimal(SECItem *oid)
{
    char *oid_string = NULL;
    PyObject *py_oid_string;

    if ((oid_string = CERT_GetOidString(oid)) == NULL) {
        return PyString_FromString("");
    }
    if ((py_oid_string = PyString_FromString(oid_string)) == NULL) {
        PR_smprintf_free(oid_string);
        return NULL;
    }
    PR_smprintf_free(oid_string);
    return py_oid_string;
}

static PyObject *
der_oid_secitem_to_pystr_desc(SECItem *item)
{
    PyObject *str = NULL;
    SECItem tmp_item = *item;

    if (sec_strip_tag_and_length(&tmp_item) == SECSuccess)
	str = oid_secitem_to_pystr_desc(&tmp_item);

    return str;
}

static PyObject *
der_utc_time_secitem_to_pystr(SECItem *item)
{
    PRTime pr_time = 0;
    PRExplodedTime exploded_time;
    char time_str[100];

    if ((DER_UTCTimeToTime(&pr_time, item) != SECSuccess)) {
        Py_RETURN_NONE;
    }
    PR_ExplodeTime(pr_time, PR_GMTParameters, &exploded_time);
    PR_FormatTime(time_str, sizeof(time_str), time_format, &exploded_time);

    return PyString_FromString(time_str);
}


static PyObject *
der_generalized_time_secitem_to_pystr(SECItem *item)
{
    PRTime pr_time = 0;
    PRExplodedTime exploded_time;
    char time_str[100];

    if ((DER_GeneralizedTimeToTime(&pr_time, item) != SECSuccess)) {
        Py_RETURN_NONE;
    }
    PR_ExplodeTime(pr_time, PR_GMTParameters, &exploded_time);
    PR_FormatTime(time_str, sizeof(time_str), time_format, &exploded_time);

    return PyString_FromString(time_str);
}


static PRTime
time_choice_secitem_to_prtime(SECItem *item)
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
time_choice_secitem_to_pystr(SECItem *item)
{
    PRTime pr_time = 0;
    PRExplodedTime exploded_time;
    char time_str[100];

    pr_time = time_choice_secitem_to_prtime(item);
    PR_ExplodeTime(pr_time, PR_GMTParameters, &exploded_time);
    PR_FormatTime(time_str, sizeof(time_str), time_format, &exploded_time);

    return PyString_FromString(time_str);
}

static PyObject *
der_octet_secitem_to_pystr(SECItem *item, int octets_per_line, char *separator)
{
    PyObject *str = NULL;
    SECItem tmp_item = *item;

    if (sec_strip_tag_and_length(&tmp_item) == SECSuccess)
        str = raw_data_to_hex(tmp_item.data, tmp_item.len, octets_per_line, separator);

    return str;
}

static PyObject *
der_bit_string_secitem_to_pystr(SECItem *item)
{
    PyObject *str = NULL;
    SECItem tmp_item = *item;
    int unused_bits;

    if (sec_strip_tag_and_length(&tmp_item) != SECSuccess || tmp_item.len < 2) {
        Py_RETURN_NONE;
    }

    unused_bits = *tmp_item.data++;
    tmp_item.len--;

    str = raw_data_to_hex(tmp_item.data, tmp_item.len, 0, HEX_SEPARATOR_DEFAULT);

    if (unused_bits) {
	PyString_ConcatAndDel(&str, PyString_FromFormat("(%d least significant bits unused)", unused_bits));
    }

    return str;
}

static PyObject *
ascii_string_secitem_to_escaped_ascii_pystr(SECItem *item)
{
    PyObject *py_str = NULL;
    size_t escaped_len;
    const unsigned char *s; /* must be unsigned for table indexing to work */
    char *escaped_str, *dst, *src;
    AsciiEscapes *encode;
    unsigned int len;

    escaped_len = ascii_encoded_strnlen((const char *)item->data, item->len);

    if ((py_str = PyString_FromStringAndSize(NULL, escaped_len)) == NULL) {
        return NULL;
    }

    escaped_str = PyString_AS_STRING(py_str);

    for (s = (unsigned char *)item->data, len = item->len, dst = escaped_str;
         len;
         s++, len--) {
        encode = &ascii_encoding_table[*s];
        for (src = encode->encoded; *src; src++) {
            *dst++ = *src;
        }
    }

    *dst = 0;                   /* shouldn't be necessary, PyString's are always NULL terminated */

    return py_str;
}

static PyObject *
der_ascii_string_secitem_to_escaped_ascii_pystr(SECItem *item)
{
    SECItem tmp_item = *item;

    if (sec_strip_tag_and_length(&tmp_item) != SECSuccess) {
        PyErr_SetString(PyExc_ValueError, "malformed raw ascii string buffer");
        return NULL;
    }

    return ascii_string_secitem_to_escaped_ascii_pystr(&tmp_item);
}

static PyObject *
der_utf8_string_secitem_to_pyunicode(SECItem *item)
{
    SECItem tmp_item = *item;

    if (sec_strip_tag_and_length(&tmp_item) != SECSuccess) {
        PyErr_SetString(PyExc_ValueError, "malformed raw ASN.1 BMP string buffer");
        return NULL;
    }

    return PyUnicode_DecodeUTF8((const char *)tmp_item.data, tmp_item.len, NULL);
}


static PyObject *
der_bmp_string_secitem_to_pyunicode(SECItem *item)
{
    SECItem tmp_item = *item;
    int byte_order = 1;         /* 1 = big endian, asn.1 DER is always big endian */

    if (sec_strip_tag_and_length(&tmp_item) != SECSuccess) {
        PyErr_SetString(PyExc_ValueError, "malformed raw ASN.1 BMP string buffer");
        return NULL;
    }

    if (tmp_item.len % 2) {
        PyErr_SetString(PyExc_ValueError, "raw ASN.1 BMP string length must be multiple of 2");
        return NULL;
    }

    return PyUnicode_DecodeUTF16((const char *)tmp_item.data, tmp_item.len,
                                 NULL, &byte_order);
}


static PyObject *
der_universal_string_secitem_to_pyunicode(SECItem *item)
{
    SECItem tmp_item = *item;
    int byte_order = 1;         /* 1 = big endian, asn.1 DER is always big endian */

    if (sec_strip_tag_and_length(&tmp_item) != SECSuccess) {
        PyErr_SetString(PyExc_ValueError, "malformed raw ASN.1 Universal string buffer");
        return NULL;
    }

    if (tmp_item.len % 4) {
        PyErr_SetString(PyExc_ValueError, "raw ASN.1 Universal string length must be multiple of 4");
        return NULL;
    }

    return PyUnicode_DecodeUTF32((const char *)tmp_item.data, tmp_item.len,
                                 NULL, &byte_order);
}

static PyObject *
der_universal_secitem_to_pystr(SECItem *item)
{
    switch (item->data[0] & SEC_ASN1_TAGNUM_MASK) {
    case SEC_ASN1_ENUMERATED:
    case SEC_ASN1_INTEGER:
        return der_integer_secitem_to_pystr(item);
    case SEC_ASN1_OBJECT_ID:
        return der_oid_secitem_to_pystr_desc(item);
    case SEC_ASN1_BOOLEAN:
        return der_boolean_secitem_to_pystr(item);
    case SEC_ASN1_UTF8_STRING:
        return der_utf8_string_secitem_to_pyunicode(item);
    case SEC_ASN1_PRINTABLE_STRING:
    case SEC_ASN1_VISIBLE_STRING:
    case SEC_ASN1_IA5_STRING:
    case SEC_ASN1_T61_STRING:
        return der_ascii_string_secitem_to_escaped_ascii_pystr(item);
    case SEC_ASN1_GENERALIZED_TIME:
        return der_generalized_time_secitem_to_pystr(item);
    case SEC_ASN1_UTC_TIME:
        return der_utc_time_secitem_to_pystr(item);
    case SEC_ASN1_NULL:
        return PyString_FromString("(null)");
    case SEC_ASN1_SET:
    case SEC_ASN1_SEQUENCE:
        return der_set_or_str_secitem_to_pylist_of_pystr(item);
    case SEC_ASN1_OCTET_STRING:
        return der_octet_secitem_to_pystr(item, 0, HEX_SEPARATOR_DEFAULT);
    case SEC_ASN1_BIT_STRING:
        der_bit_string_secitem_to_pystr(item);
        break;
    case SEC_ASN1_BMP_STRING:
        return der_bmp_string_secitem_to_pyunicode(item);
    case SEC_ASN1_UNIVERSAL_STRING:
        return der_universal_string_secitem_to_pyunicode(item);
    default:
        return raw_data_to_hex(item->data, item->len, 0, HEX_SEPARATOR_DEFAULT);
    }
    Py_RETURN_NONE;
}

PyDoc_STRVAR(cert_der_universal_secitem_fmt_lines_doc,
"der_universal_secitem_fmt_lines(sec_item, level=0, octets_per_line=0, separator=':') -> list of (indent, string) tuples\n\
\n\
:Parameters:\n\
    sec_item : SecItem object\n\
        A SecItem containing a DER encoded ASN1 universal type\n\
    level : integer\n\
        Initial indentation level, all subsequent indents are relative\n\
        to this starting level.\n\
    octets_per_line : integer\n\
        Number of octets formatted on one line, if 0 then\n\
        return a single string instead of an array of lines\n\
    separator : string\n\
        String used to seperate each octet\n\
        If None it will be as if the empty string had been\n\
        passed and no separator will be used.\n\
\n\
Given a SecItem in DER format which encodes a ASN.1 universal\n\
type convert the item to a string and return a list of\n\
(indent, string) tuples.\n\
");
static PyObject *
cert_der_universal_secitem_fmt_lines(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"level", "octets_per_line", "separator", NULL};
    SecItem *py_sec_item = NULL;
    int level = 0;
    int octets_per_line = OCTETS_PER_LINE_DEFAULT;
    char *hex_separator = HEX_SEPARATOR_DEFAULT;
    PyObject *lines = NULL;
    PyObject *obj = NULL;
    SECItem *item = NULL;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!|iiz:der_universal_secitem_fmt_lines", kwlist,
                                     &SecItemType, &py_sec_item,
                                     &level, &octets_per_line, &hex_separator))
        return NULL;

    item = &py_sec_item->item;

    if ((lines = PyList_New(0)) == NULL) {
        return NULL;
    }

    switch (item->data[0] & SEC_ASN1_TAGNUM_MASK) {
    case SEC_ASN1_ENUMERATED:
    case SEC_ASN1_INTEGER:
        obj = der_integer_secitem_to_pystr(item);
    case SEC_ASN1_OBJECT_ID:
        obj = der_oid_secitem_to_pystr_desc(item);
    case SEC_ASN1_BOOLEAN:
        obj = der_boolean_secitem_to_pystr(item);
    case SEC_ASN1_UTF8_STRING:
        obj = der_utf8_string_secitem_to_pyunicode(item);
    case SEC_ASN1_PRINTABLE_STRING:
    case SEC_ASN1_VISIBLE_STRING:
    case SEC_ASN1_IA5_STRING:
    case SEC_ASN1_T61_STRING:
        obj = der_ascii_string_secitem_to_escaped_ascii_pystr(item);
    case SEC_ASN1_GENERALIZED_TIME:
        obj = der_generalized_time_secitem_to_pystr(item);
    case SEC_ASN1_UTC_TIME:
        obj = der_utc_time_secitem_to_pystr(item);
    case SEC_ASN1_NULL:
        obj = PyString_FromString("(null)");
    case SEC_ASN1_SET:
    case SEC_ASN1_SEQUENCE:
        obj = der_set_or_str_secitem_to_pylist_of_pystr(item);
    case SEC_ASN1_OCTET_STRING:
        obj = der_octet_secitem_to_pystr(item, octets_per_line, hex_separator);
    case SEC_ASN1_BIT_STRING:
        der_bit_string_secitem_to_pystr(item);
        break;
    case SEC_ASN1_BMP_STRING:
        obj = der_bmp_string_secitem_to_pyunicode(item);
    case SEC_ASN1_UNIVERSAL_STRING:
        obj = der_universal_string_secitem_to_pyunicode(item);
    default:
        obj = raw_data_to_hex(item->data, item->len, octets_per_line, hex_separator);
    }

    if (PyList_Check(obj)) {
        APPEND_LINES_AND_CLEAR(lines, obj, level, fail);
    } else {
        FMT_OBJ_AND_APPEND(lines, NULL, obj, level, fail);
    }

    return lines;

 fail:
    Py_XDECREF(lines);
    return NULL;
}

static PyObject *
CERTGeneralName_type_string_to_pystr(CERTGeneralName *general_name)
{
    
    switch(general_name->type) {
    case certOtherName: {
        PyObject *py_oid = oid_secitem_to_pystr_desc(&general_name->name.OthName.oid);
        if (py_oid) {
            PyObject *result = PyString_FromFormat(_("Other Name (%s)"), PyString_AS_STRING(py_oid));
            Py_DECREF(py_oid);
            return result;
        } else {
            return PyString_FromString(_("Other Name"));
        }
    }
    case certRFC822Name:
        return PyString_FromString(_("RFC822 Name"));
    case certDNSName:
        return PyString_FromString(_("DNS name"));
    case certX400Address:
        return PyString_FromString(_("X400 Address"));
    case certDirectoryName:
        return PyString_FromString(_("Directory Name"));
    case certEDIPartyName:
        return PyString_FromString(_("EDI Party"));
    case certURI:
        return PyString_FromString(_("URI"));
    case certIPAddress:
        return PyString_FromString(_("IP Address"));
    case certRegisterID:
        return PyString_FromString(_("Registered ID"));
    default:
	return PyString_FromFormat(_("unknown type [%d]"), (int)general_name->type - 1);
    }
}

static PyObject *
CERTGeneralName_to_pystr(CERTGeneralName *general_name)
{
    switch(general_name->type) {
    case certOtherName:
        return der_any_secitem_to_pystr(&general_name->name.OthName.name);
    case certRFC822Name:
        return ascii_string_secitem_to_escaped_ascii_pystr(&general_name->name.other);
    case certDNSName:
        return ascii_string_secitem_to_escaped_ascii_pystr(&general_name->name.other);
    case certX400Address:
        return der_any_secitem_to_pystr(&general_name->name.other);
    case certDirectoryName:
        return CERTName_to_pystr(&general_name->name.directoryName);
    case certEDIPartyName:
        return der_any_secitem_to_pystr(&general_name->name.other);
    case certURI:
        return ascii_string_secitem_to_escaped_ascii_pystr(&general_name->name.other);
    case certIPAddress:
        return ip_addr_secitem_to_pystr(&general_name->name.other);
    case certRegisterID:
        return oid_secitem_to_pystr_desc(&general_name->name.other);
    default:
        PyErr_Format(PyExc_ValueError, _("unknown type [%d]"), (int)general_name->type - 1);
        return NULL;
        
    }
}

static PyObject *
CERTGeneralName_to_pystr_with_label(CERTGeneralName *general_name)
{
    PyObject *py_label = NULL;
    PyObject *py_value = NULL;
    PyObject *result = NULL;

    if (!general_name) {
        return NULL;
    }

    py_label = CERTGeneralName_type_string_to_pystr(general_name);
    py_value = CERTGeneralName_to_pystr(general_name);

    if (py_label && py_value) {
        result = PyString_FromFormat("%s: %s", 
                                     PyString_AS_STRING(py_label),
                                     PyString_AS_STRING(py_value));
    } else if (py_value) {
        Py_INCREF(py_value);
        result = py_value;
    }

    Py_XDECREF(py_label);
    Py_XDECREF(py_value);

    return result;
}

static PyObject *
SECAlgorithmID_to_pystr(SECAlgorithmID *a)
{
    PyObject *str = NULL;

    if ((str = oid_secitem_to_pystr_desc(&a->algorithm)) == NULL) {
        return NULL;
    }

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
CERTAVA_value_to_pystr(CERTAVA *ava)
{
    PyObject *result = NULL;
    SECOidTag oid_tag;
    const char *attr_name;
    char *oid_name;
    char value_buf[1024];
    SECItem *value_item;

    if (!ava) {
        return PyString_FromString("");
    }

    value_buf[0] = 0;
    attr_name = NULL;
    oid_name = NULL;

    /*
     * Get the AVA's attribute name (e.g. type) as a string.  If we
     * can't get the canonical name use a dotted-decimal OID
     * representation instead.
     */
    if ((oid_tag = CERT_GetAVATag(ava)) != -1) {
        attr_name = ava_oid_tag_to_name(oid_tag);
    }

    if (attr_name == NULL) {
        if ((oid_name = CERT_GetOidString(&ava->type)) == NULL) {
            return set_nspr_error("cannot convert AVA type to OID string");
        }
    }

    /* Get the AVA's attribute value as a string */
    if ((value_item = CERT_DecodeAVAValue(&ava->value)) == NULL) {
        if (oid_name) PR_smprintf_free(oid_name);
        return set_nspr_error("unable to decode AVA value");
    }
    if (CERT_RFC1485_EscapeAndQuote(value_buf, sizeof(value_buf),
                                    (char *)value_item->data,
                                    value_item->len) != SECSuccess) {
        if (oid_name) PR_smprintf_free(oid_name);
        SECITEM_FreeItem(value_item, PR_TRUE);
        return set_nspr_error("unable to escape AVA value string");
    }
    SECITEM_FreeItem(value_item, PR_TRUE);

    /* Format "name=value" */
    if ((result = PyString_FromFormat("%s=%s",
                                      attr_name ? attr_name : oid_name,
                                      value_buf)) == NULL) {
        if (oid_name) PR_smprintf_free(oid_name);
        return NULL;
    }

    if (oid_name) PR_smprintf_free(oid_name);

    return result;
}

static PyObject *
CERTRDN_to_pystr(CERTRDN *rdn)
{
    PyObject *result = NULL;
    CERTAVA **avas, *ava;
    SECOidTag oid_tag;
    const char *attr_name;
    char *oid_name;
    bool first;
    char value_buf[1024];
    SECItem *value_item;

    if (!rdn || !(avas = rdn->avas) || *avas == NULL) {
        return PyString_FromString("");
    }

    first = true;
    while ((ava = *avas++) != NULL) {
        value_buf[0] = 0;
        attr_name = NULL;
        oid_name = NULL;

        /*
         * Get the AVA's attribute name (e.g. type) as a string.  If we
         * can't get the canonical name use a dotted-decimal OID
         * representation instead.
         */
        if ((oid_tag = CERT_GetAVATag(ava)) != -1) {
            attr_name = ava_oid_tag_to_name(oid_tag);
        }

        if (attr_name == NULL) {
            if ((oid_name = CERT_GetOidString(&ava->type)) == NULL) {
                return set_nspr_error("cannot convert AVA type to OID string");
            }
        }

        /* Get the AVA's attribute value as a string */
        if ((value_item = CERT_DecodeAVAValue(&ava->value)) == NULL) {
            if (oid_name) PR_smprintf_free(oid_name);
            return set_nspr_error("unable to decode AVA value");
        }
        if (CERT_RFC1485_EscapeAndQuote(value_buf, sizeof(value_buf),
                                        (char *)value_item->data,
                                        value_item->len) != SECSuccess) {
            if (oid_name) PR_smprintf_free(oid_name);
            SECITEM_FreeItem(value_item, PR_TRUE);
            return set_nspr_error("unable to escape AVA value string");
        }
        SECITEM_FreeItem(value_item, PR_TRUE);

        /*
         * Format "name=value", if there is more than one AVA join them
         * together with a "+". Typically there is only one AVA.
         */
        if (first) {
            if ((result = PyString_FromFormat("%s=%s",
                                              attr_name ? attr_name : oid_name,
                                              value_buf)) == NULL) {
                if (oid_name) PR_smprintf_free(oid_name);
                return NULL;
            }
        } else {
            PyObject *temp;

            if ((temp = PyString_FromFormat("+%s=%s",
                                            attr_name ? attr_name : oid_name,
                                            value_buf)) == NULL) {
                if (oid_name) PR_smprintf_free(oid_name);
                return NULL;
            }
            PyString_ConcatAndDel(&result, temp);
            if (result == NULL) {
                if (oid_name) PR_smprintf_free(oid_name);
                return NULL;
            }
        }

        if (oid_name) PR_smprintf_free(oid_name);
        first = false;
    }
    return result;
}

static PyObject *
cert_trust_flags_str(unsigned int flags)
{
    PyObject *py_flags = NULL;
    PyObject *py_flag = NULL;

    if ((py_flags = PyList_New(0)) == NULL)
        return NULL;

    if (flags & CERTDB_VALID_PEER) {
        flags &= ~CERTDB_VALID_PEER;
	if ((py_flag = PyString_FromString(_("Valid Peer"))) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
	Py_DECREF(py_flag);
    }
    if (flags & CERTDB_TRUSTED) {
        flags &= ~CERTDB_TRUSTED;
	if ((py_flag = PyString_FromString(_("Trusted"))) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
	Py_DECREF(py_flag);
    }
    if (flags & CERTDB_SEND_WARN) {
        flags &= ~CERTDB_SEND_WARN;
	if ((py_flag = PyString_FromString(_("Warn When Sending"))) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
	Py_DECREF(py_flag);
    }
    if (flags & CERTDB_VALID_CA) {
        flags &= ~CERTDB_VALID_CA;
	if ((py_flag = PyString_FromString(_("Valid CA"))) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
	Py_DECREF(py_flag);
    }
    if (flags & CERTDB_TRUSTED_CA) {
        flags &= ~CERTDB_TRUSTED_CA;
	if ((py_flag = PyString_FromString(_("Trusted CA"))) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
	Py_DECREF(py_flag);
    }
    if (flags & CERTDB_NS_TRUSTED_CA) {
        flags &= ~CERTDB_NS_TRUSTED_CA;
	if ((py_flag = PyString_FromString(_("Netscape Trusted CA"))) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
	Py_DECREF(py_flag);
    }
    if (flags & CERTDB_USER) {
        flags &= ~CERTDB_USER;
	if ((py_flag = PyString_FromString(_("User"))) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
	Py_DECREF(py_flag);
    }
    if (flags & CERTDB_TRUSTED_CLIENT_CA) {
        flags &= ~CERTDB_TRUSTED_CLIENT_CA;
	if ((py_flag = PyString_FromString(_("Trusted Client CA"))) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
	Py_DECREF(py_flag);
    }
    if (flags & CERTDB_GOVT_APPROVED_CA) {
        flags &= ~CERTDB_GOVT_APPROVED_CA;
	if ((py_flag = PyString_FromString(_("Step-up"))) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
	Py_DECREF(py_flag);
    }

    if (flags) {
        if ((py_flag = PyString_FromFormat("unknown bit flags %#x", flags)) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
	Py_DECREF(py_flag);
    }

    if (PyList_Sort(py_flags) == -1) {
            Py_DECREF(py_flags);
            return NULL;
    }

    return py_flags;
}

static PyObject *
cert_usage_flags(unsigned int flags)
{
    PyObject *py_flags = NULL;
    PyObject *py_flag = NULL;

    if ((py_flags = PyList_New(0)) == NULL)
        return NULL;

    if (flags & certificateUsageSSLClient) {
        flags &= ~certificateUsageSSLClient;
	if ((py_flag = PyString_FromString(_("SSLClient"))) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
	Py_DECREF(py_flag);
    }
    if (flags & certificateUsageSSLServer) {
        flags &= ~certificateUsageSSLServer;
	if ((py_flag = PyString_FromString(_("SSLServer"))) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
	Py_DECREF(py_flag);
    }
    if (flags & certificateUsageSSLServerWithStepUp) {
        flags &= ~certificateUsageSSLServerWithStepUp;
	if ((py_flag = PyString_FromString(_("SSLServerWithStepUp"))) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
	Py_DECREF(py_flag);
    }
    if (flags & certificateUsageSSLCA) {
        flags &= ~certificateUsageSSLCA;
	if ((py_flag = PyString_FromString(_("SSLCA"))) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
	Py_DECREF(py_flag);
    }
    if (flags & certificateUsageEmailSigner) {
        flags &= ~certificateUsageEmailSigner;
	if ((py_flag = PyString_FromString(_("EmailSigner"))) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
	Py_DECREF(py_flag);
    }
    if (flags & certificateUsageEmailRecipient) {
        flags &= ~certificateUsageEmailRecipient;
	if ((py_flag = PyString_FromString(_("EmailRecipient"))) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
	Py_DECREF(py_flag);
    }
    if (flags & certificateUsageObjectSigner) {
        flags &= ~certificateUsageObjectSigner;
	if ((py_flag = PyString_FromString(_("ObjectSigner"))) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
	Py_DECREF(py_flag);
    }
    if (flags & certificateUsageUserCertImport) {
        flags &= ~certificateUsageUserCertImport;
	if ((py_flag = PyString_FromString(_("UserCertImport"))) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
	Py_DECREF(py_flag);
    }
    if (flags & certificateUsageVerifyCA) {
        flags &= ~certificateUsageVerifyCA;
	if ((py_flag = PyString_FromString(_("VerifyCA"))) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
	Py_DECREF(py_flag);
    }
    if (flags & certificateUsageProtectedObjectSigner) {
        flags &= ~certificateUsageProtectedObjectSigner;
	if ((py_flag = PyString_FromString(_("ProtectedObjectSigner"))) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
	Py_DECREF(py_flag);
    }
    if (flags & certificateUsageStatusResponder) {
        flags &= ~certificateUsageStatusResponder;
	if ((py_flag = PyString_FromString(_("StatusResponder"))) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
	Py_DECREF(py_flag);
    }
    if (flags & certificateUsageAnyCA) {
        flags &= ~certificateUsageAnyCA;
	if ((py_flag = PyString_FromString(_("AnyCA"))) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
	Py_DECREF(py_flag);
    }

    if (flags) {
        if ((py_flag = PyString_FromFormat("unknown bit flags %#x", flags)) == NULL) {
            Py_DECREF(py_flags);
            return NULL;
        }
        PyList_Append(py_flags, py_flag);
	Py_DECREF(py_flag);
    }

    if (PyList_Sort(py_flags) == -1) {
            Py_DECREF(py_flags);
            return NULL;
    }

    return py_flags;
}


static PyObject *
ip_addr_secitem_to_pystr(SECItem *item)
{
    PRNetAddr  addr;
    char buf[1024];

    memset(&addr, 0, sizeof(addr));
    if (item->len == 4) {
	addr.inet.family = PR_AF_INET;
	memcpy(&addr.inet.ip, item->data, item->len);
    } else if (item->len == 16) {
	addr.ipv6.family = PR_AF_INET6;
	memcpy(addr.ipv6.ip.pr_s6_addr, item->data, item->len);
	if (PR_IsNetAddrType(&addr, PR_IpAddrV4Mapped)) {
	    /* convert to IPv4.  */
	    addr.inet.family = PR_AF_INET;
	    memcpy(&addr.inet.ip, &addr.ipv6.ip.pr_s6_addr[12], 4);
	    memset(&addr.inet.pad[0], 0, sizeof addr.inet.pad);
	}
    } else {
        return secitem_to_pystr_hex(item);
    }

    if (PR_NetAddrToString(&addr, buf, sizeof(buf)) != PR_SUCCESS) {
        return secitem_to_pystr_hex(item);
    }

    return PyString_FromString(buf);
}

static PyObject *
SECItem_der_to_hex(SECItem *item, int octets_per_line, char *separator)
{
    SECItem tmp_item = *item;

    if (sec_strip_tag_and_length(&tmp_item) != SECSuccess) {
        PyErr_SetString(PyExc_ValueError, "malformed ASN.1 DER data");
        return NULL;
    }

    return raw_data_to_hex(tmp_item.data, tmp_item.len, octets_per_line, separator);
}

/* ========================================================================== */
/* =============================== SecItem Class ============================ */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static PyObject *
SecItem_get_type(SecItem *self, void *closure)
{
    TraceMethodEnter(self);

    return PyInt_FromLong(self->item.type);
}

static PyObject *
SecItem_get_len(SecItem *self, void *closure)
{
    TraceMethodEnter(self);

    return PyInt_FromLong(self->item.len);
}

static PyObject *
SecItem_get_data(SecItem *self, void *closure)
{
    TraceMethodEnter(self);

    return PyString_FromStringAndSize((const char *)self->item.data, self->item.len);
}

static
PyGetSetDef SecItem_getseters[] = {
    {"type",       (getter)SecItem_get_type,    (setter)NULL,
     "the SecItem type (si* constant)", NULL},
    {"len",        (getter)SecItem_get_len,     (setter)NULL,
     "number of octets in SecItem buffer", NULL},
    {"data",       (getter)SecItem_get_data,    (setter)NULL,
     "contents of SecItem buffer", NULL},
    {NULL}  /* Sentinel */
};

static PyMemberDef SecItem_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

PyDoc_STRVAR(SecItem_get_oid_sequence_doc,
"get_oid_sequence(sec_item, repr_kind=AsString) -> (obj, ...)\n\
\n\
:Parameters:\n\
    sec_item : SecItem object\n\
        A SecItem containing a DER encoded sequence of OID's\n\
    repr_kind : RepresentationKind constant\n\
        Specifies what the contents of the returned tuple will be.\n\
        May be one of:\n\
\n\
        AsObject\n\
            Each extended key usage will be a SecItem object embedding\n\
            the OID in DER format.\n\
        AsString\n\
            Each extended key usage will be a descriptive string.\n\
            (e.g. \"TLS Web Server Authentication Certificate\")\n\
        AsDottedDecimal\n\
            Each extended key usage will be OID rendered as a dotted decimal string.\n\
            (e.g. \"OID.1.3.6.1.5.5.7.3.1\")\n\
        AsEnum\n\
            Each extended key usage will be OID tag enumeration constant (int).\n\
            (e.g. nss.SEC_OID_EXT_KEY_USAGE_SERVER_AUTH)\n\
\n\
Return a tuple of OID's according the representation kind.\n\
");
static PyObject *
SecItem_get_oid_sequence(SecItem *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"repr_kind", NULL};
    int repr_kind = AsString;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:get_oid_sequence", kwlist,
                                     &repr_kind))
        return NULL;

    return decode_oid_sequence_to_tuple(&self->item, repr_kind);
}

PyDoc_STRVAR(SecItem_to_hex_doc,
"to_hex(octets_per_line=0, separator=':') -> string or list of strings\n\
\n\
:Parameters:\n\
    octets_per_line : integer\n\
        Number of octets formatted on one line, if 0 then\n\
        return a single string instead of an array of lines\n\
    separator : string\n\
        String used to seperate each octet\n\
        If None it will be as if the empty string had been\n\
        passed and no separator will be used.\n\
\n\
Equivalent to calling data_to_hex(sec_item)\n\
");

static PyObject *
SecItem_to_hex(SecItem *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"octets_per_line", "separator", NULL};
    int octets_per_line = 0;
    char *separator = HEX_SEPARATOR_DEFAULT;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|iz:to_hex", kwlist,
                                     &octets_per_line, &separator))
        return NULL;

    return raw_data_to_hex(self->item.data, self->item.len, octets_per_line, separator);
}

PyDoc_STRVAR(SecItem_der_to_hex_doc,
"der_to_hex(octets_per_line=0, separator=':') -> string or list of strings\n\
\n\
:Parameters:\n\
    octets_per_line : integer\n\
        Number of octets formatted on one line, if 0 then\n\
        return a single string instead of an array of lines\n\
    separator : string\n\
        String used to seperate each octet\n\
        If None it will be as if the empty string had been\n\
        passed and no separator will be used.\n\
\n\
Interpret the SecItem as containing DER encoded data consisting\n\
of a <type,length,value> triplet (e.g. TLV). This function skips\n\
the type and length components and returns the value component as\n\
a hexadecimal string or a list of hexidecimal strings with a\n\
maximum of octets_per_line in each list element. See data_to_hex()\n\
for a more detailed explanation.\n\
");

static PyObject *
SecItem_der_to_hex(SecItem *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"octets_per_line", "separator", NULL};
    int octets_per_line = 0;
    char *separator = HEX_SEPARATOR_DEFAULT;
    SECItem tmp_item = self->item;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|iz:der_to_hex", kwlist,
                                     &octets_per_line, &separator))
        return NULL;


    tmp_item = self->item;
    if (sec_strip_tag_and_length(&tmp_item) != SECSuccess) {
        PyErr_SetString(PyExc_ValueError, "malformed ASN.1 DER data");
        return NULL;
    }

    return raw_data_to_hex(tmp_item.data, tmp_item.len, octets_per_line, separator);
}

static PyMethodDef SecItem_methods[] = {
    {"get_oid_sequence", (PyCFunction)SecItem_get_oid_sequence, METH_NOARGS,                SecItem_get_oid_sequence_doc},
    {"to_hex",           (PyCFunction)SecItem_to_hex,           METH_VARARGS|METH_KEYWORDS, SecItem_to_hex_doc},
    {"der_to_hex",       (PyCFunction)SecItem_der_to_hex,       METH_VARARGS|METH_KEYWORDS, SecItem_der_to_hex_doc},
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
SecItem_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    SecItem *self;

    TraceObjNewEnter(type);

    if ((self = (SecItem *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }
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
        return oid_secitem_to_pystr_desc(&self->item);
    default:
        return_value =  obj_to_hex((PyObject *)self, 0, HEX_SEPARATOR_DEFAULT);

        break;
    }
    return return_value;
}

static int
SecItem_compare(SecItem *self, SecItem *other)
{
    if (!PySecItem_Check(other)) {
        PyErr_SetString(PyExc_TypeError, "Bad type, must be SecItem");
        return -1;
    }

    if (self->item.data == NULL && other->item.data == NULL) {
        return 0;
    }

    if (self->item.len > other->item.len) {
        return 1;
    }

    if (self->item.len < other->item.len) {
        return -1;
    }

    return memcmp(self->item.data, other->item.data, self->item.len);
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
    (cmpfunc)SecItem_compare,			/* tp_compare */
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

static PyObject *
SecItem_new_from_SECItem(SECItem *item, SECItemKind kind)
{
    SecItem *self = NULL;

    TraceObjNewEnter(NULL);

    if ((self = (SecItem *) SecItemType.tp_new(&SecItemType, NULL, NULL)) == NULL) {
        return NULL;
    }

    self->item.type = item->type;
    self->item.len = item->len;
    if ((self->item.data = PyMem_MALLOC(item->len)) == NULL) {
        Py_CLEAR(self);
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

    if ((self = (SignatureAlgorithm *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }

    memset(&self->id, 0, sizeof(self->id));
    self->py_id = NULL;
    self->py_parameters = NULL;

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static int
SignatureAlgorithm_traverse(SignatureAlgorithm *self, visitproc visit, void *arg)
{
    Py_VISIT(self->py_id);
    Py_VISIT(self->py_parameters);
    return 0;
}

static int
SignatureAlgorithm_clear(SignatureAlgorithm* self)
{
    TraceMethodEnter(self);

    Py_CLEAR(self->py_id);
    Py_CLEAR(self->py_parameters);
    return 0;
}

static void
SignatureAlgorithm_dealloc(SignatureAlgorithm* self)
{
    TraceMethodEnter(self);

    SignatureAlgorithm_clear(self);
    SECOID_DestroyAlgorithmID(&self->id, PR_FALSE);
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
    return SECAlgorithmID_to_pystr(&self->id);
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
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,	/* tp_flags */
    SignatureAlgorithm_doc,			/* tp_doc */
    (traverseproc)SignatureAlgorithm_traverse,	/* tp_traverse */
    (inquiry)SignatureAlgorithm_clear,		/* tp_clear */
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

    if ((self = (SignatureAlgorithm *) SignatureAlgorithmType.tp_new(&SignatureAlgorithmType, NULL, NULL)) == NULL) {
        return NULL;
    }

    if (SECOID_CopyAlgorithmID(NULL, &self->id, id) != SECSuccess) {
        set_nspr_error(NULL);
        Py_CLEAR(self);
        return NULL;
    }

    if ((self->py_id = SecItem_new_from_SECItem(&id->algorithm, SECITEM_algorithm)) == NULL) {
        SECOID_DestroyAlgorithmID(&self->id, PR_FALSE);
        Py_CLEAR(self);
        return NULL;
    }

    if ((self->py_parameters = SecItem_new_from_SECItem(&id->parameters, SECITEM_unknown)) == NULL) {
        SECOID_DestroyAlgorithmID(&self->id, PR_FALSE);
        Py_CLEAR(self);
        return NULL;
    }

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
    TraceMethodEnter(self);

    Py_INCREF(self->py_prime);
    return self->py_prime;
}

static PyObject *
KEYPQGParams_get_subprime(KEYPQGParams *self, void *closure)
{
    TraceMethodEnter(self);

    Py_INCREF(self->py_subprime);
    return self->py_subprime;
}

static PyObject *
KEYPQGParams_get_base(KEYPQGParams *self, void *closure)
{
    TraceMethodEnter(self);

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

    if ((self = (KEYPQGParams *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }

    self->py_prime = NULL;
    self->py_subprime = NULL;
    self->py_base = NULL;

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static int
KEYPQGParams_traverse(KEYPQGParams *self, visitproc visit, void *arg)
{
    Py_VISIT(self->py_prime);
    Py_VISIT(self->py_subprime);
    Py_VISIT(self->py_base);
    return 0;
}

static int
KEYPQGParams_clear(KEYPQGParams* self)
{
    TraceMethodEnter(self);

    Py_CLEAR(self->py_prime);
    Py_CLEAR(self->py_subprime);
    Py_CLEAR(self->py_base);
    return 0;
}

static void
KEYPQGParams_dealloc(KEYPQGParams* self)
{
    TraceMethodEnter(self);

    KEYPQGParams_clear(self);
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

    if ((fmt = PyString_FromString("prime(p)=%s subprime(q)=%s base(g)=%s")) == NULL) {
        return NULL;
    }
    if ((args = PyTuple_New(3)) == NULL) {
        Py_DECREF(fmt);
        return NULL;
    }

    PyTuple_SetItem(args, 0, PyObject_Str(self->py_prime));
    PyTuple_SetItem(args, 1, PyObject_Str(self->py_subprime));
    PyTuple_SetItem(args, 2, PyObject_Str(self->py_base));

    str = PyString_Format(fmt, args);

    Py_DECREF(fmt);
    Py_DECREF(args);
    return str;
}

static PyTypeObject KEYPQGParamsType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.KEYPQGParams",			/* tp_name */
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
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,	/* tp_flags */
    KEYPQGParams_doc,				/* tp_doc */
    (traverseproc)KEYPQGParams_traverse,	/* tp_traverse */
    (inquiry)KEYPQGParams_clear,		/* tp_clear */
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

    if ((self = (KEYPQGParams *) KEYPQGParamsType.tp_new(&KEYPQGParamsType, NULL, NULL)) == NULL) {
        return NULL;
    }

    if ((self->py_prime = SecItem_new_from_SECItem(&params->prime, SECITEM_unknown)) == NULL) {
        Py_CLEAR(self);
        return NULL;
    }

    if ((self->py_subprime = SecItem_new_from_SECItem(&params->subPrime, SECITEM_unknown)) == NULL) {
        Py_CLEAR(self);
        return NULL;
    }

    if ((self->py_base = SecItem_new_from_SECItem(&params->base, SECITEM_unknown)) == NULL) {
        Py_CLEAR(self);
        return NULL;
    }

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
    TraceMethodEnter(self);

    Py_INCREF(self->py_modulus);
    return self->py_modulus;
}

static PyObject *
RSAPublicKey_get_exponent(RSAPublicKey *self, void *closure)
{
    TraceMethodEnter(self);

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
    PyObject *exponent = NULL;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:format_lines", kwlist, &level))
        return NULL;

    if ((lines = PyList_New(0)) == NULL) {
        return NULL;
    }

    FMT_LABEL_AND_APPEND(lines, _("Modulus"), level, fail);

    if ((obj = RSAPublicKey_get_modulus(self, NULL)) == NULL) {
        goto fail;
    }
    APPEND_OBJ_TO_HEX_LINES_AND_CLEAR(lines, obj, level+1, fail);

    if ((exponent = RSAPublicKey_get_exponent(self, NULL)) == NULL) {
        goto fail;
    }
    if ((obj = obj_sprintf("%d (%#x)", exponent, exponent)) == NULL) {
        goto fail;
    }
    FMT_OBJ_AND_APPEND(lines, _("Exponent"), obj, level, fail);
    Py_CLEAR(exponent);
    Py_CLEAR(obj);

    return lines;
 fail:
    Py_XDECREF(obj);
    Py_XDECREF(exponent);
    Py_XDECREF(lines);
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

    if ((self = (RSAPublicKey *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }

    self->py_modulus = NULL;
    self->py_exponent = NULL;

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static int
RSAPublicKey_traverse(RSAPublicKey *self, visitproc visit, void *arg)
{
    Py_VISIT(self->py_modulus);
    Py_VISIT(self->py_exponent);
    return 0;
}

static int
RSAPublicKey_clear(RSAPublicKey* self)
{
    TraceMethodEnter(self);

    Py_CLEAR(self->py_modulus);
    Py_CLEAR(self->py_exponent);
    return 0;
}

static void
RSAPublicKey_dealloc(RSAPublicKey* self)
{
    TraceMethodEnter(self);

    RSAPublicKey_clear(self);
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
    "nss.nss.RSAPublicKey",			/* tp_name */
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
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,	/* tp_flags */
    RSAPublicKey_doc,				/* tp_doc */
    (traverseproc)RSAPublicKey_traverse,	/* tp_traverse */
    (inquiry)RSAPublicKey_clear,		/* tp_clear */
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

    if ((self = (RSAPublicKey *) RSAPublicKeyType.tp_new(&RSAPublicKeyType, NULL, NULL)) == NULL) {
        return NULL;
    }

    if ((self->py_modulus = SecItem_new_from_SECItem(&rsa->modulus, SECITEM_unknown)) == NULL) {
        Py_CLEAR(self);
        return NULL;
    }

    if ((self->py_exponent = integer_secitem_to_pylong(&rsa->publicExponent)) == NULL) {
        Py_CLEAR(self);
        return NULL;
    }

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
    TraceMethodEnter(self);

    Py_INCREF(self->py_pqg_params);
    return self->py_pqg_params;
}

static PyObject *
DSAPublicKey_get_public_value(DSAPublicKey *self, void *closure)
{
    TraceMethodEnter(self);

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

    if ((self = (DSAPublicKey *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }

    self->py_pqg_params = NULL;
    self->py_public_value = NULL;

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static int
DSAPublicKey_traverse(DSAPublicKey *self, visitproc visit, void *arg)
{
    Py_VISIT(self->py_pqg_params);
    Py_VISIT(self->py_public_value);
    return 0;
}

static int
DSAPublicKey_clear(DSAPublicKey* self)
{
    TraceMethodEnter(self);

    Py_CLEAR(self->py_pqg_params);
    Py_CLEAR(self->py_public_value);
    return 0;
}

static void
DSAPublicKey_dealloc(DSAPublicKey* self)
{
    TraceMethodEnter(self);

    DSAPublicKey_clear(self);
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

    if ((fmt = PyString_FromString("pqg_params=[%s] public_value=%s")) == NULL) {
        return NULL;
    }

    if ((args = PyTuple_New(2)) == NULL) {
        return NULL;
    }

    PyTuple_SetItem(args, 0, PyObject_Str(self->py_pqg_params));
    PyTuple_SetItem(args, 1, PyObject_Str(self->py_public_value));

    str = PyString_Format(fmt, args);

    Py_DECREF(fmt);
    Py_DECREF(args);
    return str;
}

static PyTypeObject DSAPublicKeyType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.DSAPublicKey",			/* tp_name */
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
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,	/* tp_flags */
    DSAPublicKey_doc,				/* tp_doc */
    (traverseproc)DSAPublicKey_traverse,	/* tp_traverse */
    (inquiry)DSAPublicKey_clear,		/* tp_clear */
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

    if ((self = (DSAPublicKey *) DSAPublicKeyType.tp_new(&DSAPublicKeyType, NULL, NULL)) == NULL) {
        return NULL;
    }

    if ((self->py_pqg_params = KEYPQGParams_new_from_SECKEYPQGParams(&dsa->params)) == NULL) {
        Py_CLEAR(self);
        return NULL;
    }

    if ((self->py_public_value = SecItem_new_from_SECItem(&dsa->publicValue, SECITEM_unknown)) == NULL) {
        Py_CLEAR(self);
        return NULL;
    }

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
    TraceMethodEnter(self);

    Py_INCREF(self->py_algorithm);
    return self->py_algorithm;
}

static PyObject *
SignedData_get_signature(SignedData *self, void *closure)
{
    TraceMethodEnter(self);

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

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:format_lines", kwlist, &level))
        return NULL;

    if ((lines = PyList_New(0)) == NULL) {
        return NULL;
    }

    if ((obj = SignedData_get_algorithm(self, NULL)) == NULL) {
        goto fail;
    }
    FMT_OBJ_AND_APPEND(lines, _("Signature Algorithm"), obj, level, fail);
    Py_CLEAR(obj);

    FMT_LABEL_AND_APPEND(lines, _("Signature Data"), level, fail);

    if ((obj = SignedData_get_signature(self, NULL)) == NULL) {
        goto fail;
    }
    APPEND_OBJ_TO_HEX_LINES_AND_CLEAR(lines, obj, level+1, fail);

    return lines;

 fail:
    Py_XDECREF(obj);
    Py_XDECREF(lines);
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

    if ((self = (SignedData *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }

    self->py_data = NULL;
    self->py_algorithm = NULL;
    self->py_signature = NULL;

    if ((self->arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE)) == NULL ) {
        type->tp_free(self);
        return set_nspr_error(NULL);
    }

    memset(&self->signed_data, 0, sizeof(self->signed_data));

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static int
SignedData_traverse(SignedData *self, visitproc visit, void *arg)
{
    Py_VISIT(self->py_data);
    Py_VISIT(self->py_algorithm);
    Py_VISIT(self->py_signature);
    return 0;
}

static int
SignedData_clear(SignedData* self)
{
    TraceMethodEnter(self);

    Py_CLEAR(self->py_data);
    Py_CLEAR(self->py_algorithm);
    Py_CLEAR(self->py_signature);
    return 0;
}

static void
SignedData_dealloc(SignedData* self)
{
    TraceMethodEnter(self);

    SignedData_clear(self);
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
    "nss.nss.SignedData",			/* tp_name */
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
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,	/* tp_flags */
    SignedData_doc,				/* tp_doc */
    (traverseproc)SignedData_traverse,		/* tp_traverse */
    (inquiry)SignedData_clear,			/* tp_clear */
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

    if ((self = (SignedData *) SignedDataType.tp_new(&SignedDataType, NULL, NULL)) == NULL) {
        return NULL;
    }

    if (SEC_ASN1DecodeItem(self->arena, &self->signed_data,
                           SEC_ASN1_GET(CERT_SignedDataTemplate), item) != SECSuccess) {
        set_nspr_error("cannot decode DER encoded signed data");
        Py_CLEAR(self);
        return NULL;
    }

    if ((self->py_data =
         SecItem_new_from_SECItem(&self->signed_data.data, SECITEM_unknown)) == NULL) {
        Py_CLEAR(self);
        return NULL;
    }

    if ((self->py_algorithm = 
         SignatureAlgorithm_new_from_SECAlgorithmID(&self->signed_data.signatureAlgorithm)) == NULL) {
        Py_CLEAR(self);
        return NULL;
    }

    DER_ConvertBitString(&self->signed_data.signature);
    if ((self->py_signature =
         SecItem_new_from_SECItem(&self->signed_data.signature, SECITEM_signature)) == NULL) {
        Py_CLEAR(self);
        return NULL;
    }

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
    TraceMethodEnter(self);

    return PyInt_FromLong(self->pk->keyType);
}

static PyObject *
PublicKey_get_key_type_str(PublicKey *self, void *closure)
{
    TraceMethodEnter(self);

    return PyString_FromString(key_type_str(self->pk->keyType));
}

static PyObject *
PublicKey_get_rsa(PublicKey *self, void *closure)
{
    TraceMethodEnter(self);

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
    TraceMethodEnter(self);

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

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:format_lines", kwlist, &level))
        return NULL;

    if ((lines = PyList_New(0)) == NULL) {
        goto fail;
    }

    switch(self->pk->keyType) {       /* FIXME: handle the other cases */
    case rsaKey:
        FMT_LABEL_AND_APPEND(lines, _("RSA Public Key"), level, fail);
        CALL_FORMAT_LINES_AND_APPEND(lines, self->py_rsa_key, level+1, fail);
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

    return lines;

 fail:
    Py_XDECREF(lines);
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

    if ((self = (PublicKey *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }

    self->py_rsa_key = NULL;
    self->py_dsa_key = NULL;

    memset(&self->pk, 0, sizeof(self->pk));

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static int
PublicKey_traverse(PublicKey *self, visitproc visit, void *arg)
{
    Py_VISIT(self->py_rsa_key);
    Py_VISIT(self->py_dsa_key);
    return 0;
}

static int
PublicKey_clear(PublicKey* self)
{
    TraceMethodEnter(self);

    Py_CLEAR(self->py_rsa_key);
    Py_CLEAR(self->py_dsa_key);
    return 0;
}

static void
PublicKey_dealloc(PublicKey* self)
{
    TraceMethodEnter(self);

    PublicKey_clear(self);
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
    "nss.nss.PublicKey",			/* tp_name */
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
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,	/* tp_flags */
    PublicKey_doc,				/* tp_doc */
    (traverseproc)PublicKey_traverse,		/* tp_traverse */
    (inquiry)PublicKey_clear,			/* tp_clear */
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

    if ((self = (PublicKey *) PublicKeyType.tp_new(&PublicKeyType, NULL, NULL)) == NULL) {
        return NULL;
    }

    self->pk = pk;

    switch(pk->keyType) {       /* FIXME: handle the other cases */
    case rsaKey:
        if ((self->py_rsa_key = RSAPublicKey_new_from_SECKEYRSAPublicKey(&pk->u.rsa)) == NULL) {
            Py_CLEAR(self);
            return NULL;
        }
        break;
    case dsaKey:
        if ((self->py_dsa_key = DSAPublicKey_new_from_SECKEYDSAPublicKey(&pk->u.dsa)) == NULL) {
            Py_CLEAR(self);
            return NULL;
        }
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
    TraceMethodEnter(self);

    Py_INCREF(self->py_algorithm);
    return self->py_algorithm;
}

static PyObject *
SubjectPublicKeyInfo_get_public_key(SubjectPublicKeyInfo *self, void *closure)
{
    TraceMethodEnter(self);

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
    PyObject *py_public_key = NULL;
    PyObject *obj = NULL;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:format_lines", kwlist, &level))
        return NULL;

    if ((lines = PyList_New(0)) == NULL) {
        return NULL;
    }

    if ((obj = SubjectPublicKeyInfo_get_algorithm(self, NULL)) == NULL) {
        goto fail;
    }
    FMT_OBJ_AND_APPEND(lines, _("Public Key Algorithm"), obj, level, fail);
    Py_CLEAR(obj);

    if ((py_public_key = SubjectPublicKeyInfo_get_public_key(self, NULL)) == NULL) {
        goto fail;
    }

    CALL_FORMAT_LINES_AND_APPEND(lines, py_public_key, level+1, fail);
    Py_CLEAR(py_public_key);

    return lines;
 fail:
    Py_XDECREF(lines);
    Py_XDECREF(py_public_key);
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

    if ((self = (SubjectPublicKeyInfo *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }

    self->py_algorithm = NULL;
    self->py_public_key = NULL;

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static int
SubjectPublicKeyInfo_traverse(SubjectPublicKeyInfo *self, visitproc visit, void *arg)
{
    Py_VISIT(self->py_algorithm);
    Py_VISIT(self->py_public_key);
    return 0;
}

static int
SubjectPublicKeyInfo_clear(SubjectPublicKeyInfo* self)
{
    TraceMethodEnter(self);

    DumpRefCount(self->py_public_key);
    Py_CLEAR(self->py_algorithm);
    Py_CLEAR(self->py_public_key);
    return 0;
}

static void
SubjectPublicKeyInfo_dealloc(SubjectPublicKeyInfo* self)
{
    TraceMethodEnter(self);

    SubjectPublicKeyInfo_clear(self);
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
    "nss.nss.SubjectPublicKeyInfo",		/* tp_name */
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
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,	/* tp_flags */
    SubjectPublicKeyInfo_doc,			/* tp_doc */
    (traverseproc)SubjectPublicKeyInfo_traverse, /* tp_traverse */
    (inquiry)SubjectPublicKeyInfo_clear,	/* tp_clear */
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

    if ((self = (SubjectPublicKeyInfo *) SubjectPublicKeyInfoType.tp_new(&SubjectPublicKeyInfoType, NULL, NULL)) == NULL) {
        return NULL;
    }

    if ((self->py_algorithm = SignatureAlgorithm_new_from_SECAlgorithmID(&spki->algorithm)) == NULL) {
        Py_CLEAR(self);
        return NULL;
    }

    if ((pk = SECKEY_ExtractPublicKey(spki)) == NULL) {
        set_nspr_error(NULL);
        Py_CLEAR(self);
        return NULL;
    }

    if ((self->py_public_key = PublicKey_new_from_SECKEYPublicKey(pk)) == NULL) {
	SECKEY_DestroyPublicKey(pk);
        Py_CLEAR(self);
        return NULL;
    }
    DumpRefCount(self->py_public_key);

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

/* ============================== Utilities ============================= */

static CERTDistNames *
cert_distnames_as_CERTDistNames(PyObject *py_distnames)
{
    PRArenaPool *arena = NULL;
    CERTDistNames *names = NULL;
    int i;
    SecItem *py_sec_item;

    if (!(PyList_Check(py_distnames) || PyTuple_Check(py_distnames))) {
        PyErr_SetString(PyExc_TypeError, "cert distnames must be a list or tuple");
        return NULL;
    }

    /* allocate an arena to use */
    if ((arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE)) == NULL ) {
        set_nspr_error(NULL);
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
            py_sec_item = (SecItem *)PySequence_GetItem(py_distnames, i); /* new reference */
            if ((!PySecItem_Check(py_sec_item)) || (py_sec_item->kind != SECITEM_dist_name)) {
                PyErr_Format(PyExc_TypeError, "item must be a %s containing a DistName",
                             SecItemType.tp_name);
                Py_DECREF(py_sec_item);
                PORT_FreeArena(arena, PR_FALSE);
                return NULL;
            }
            if (SECITEM_CopyItem(arena, &names->names[i], &py_sec_item->item) != SECSuccess) {
                Py_DECREF(py_sec_item);
                PORT_FreeArena(arena, PR_FALSE);
                return NULL;
            }
            Py_DECREF(py_sec_item);
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

PyDoc_STRVAR(CertDB_find_crl_by_name_doc,
"find_crl_by_name(name, type=SEC_CRL_TYPE) -> SignedCRL object\n\
\n\
:Parameters:\n\
    name : string\n\
        name to lookup\n\
    type : int\n\
        revocation list type\n\
        \n\
        may be one of:\n\
          - SEC_CRL_TYPE\n\
          - SEC_KRL_TYPE\n\
\n\
Returns a SignedCRL object found in the database given a name and revocation list type.\n\
"
);
static PyObject *
CertDB_find_crl_by_name(CertDB *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"name", "type", NULL};
    char *name;
    int type = SEC_CRL_TYPE;
    CERTName *cert_name;
    SECItem *der_name;
    CERTSignedCrl *signed_crl;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|i:find_crl_by_name", kwlist,
                                     &name, &type))
        return NULL;

    if ((cert_name = CERT_AsciiToName(name)) == NULL) {
        return set_nspr_error(NULL);
    }

    if ((der_name = SEC_ASN1EncodeItem (NULL, NULL, (void *)cert_name,
                                        SEC_ASN1_GET(CERT_NameTemplate))) == NULL) {
        CERT_DestroyName(cert_name);
        return set_nspr_error(NULL);
    }
    CERT_DestroyName(cert_name);

    if ((signed_crl = SEC_FindCrlByName(self->handle, der_name, type)) == NULL) {
        SECITEM_FreeItem(der_name, PR_TRUE);
        return set_nspr_error(NULL);
    }
    SECITEM_FreeItem(der_name, PR_TRUE);

    return SignedCRL_new_from_CERTSignedCRL(signed_crl);
}

PyDoc_STRVAR(CertDB_find_crl_by_cert_doc,
"find_crl_by_cert(cert, type=SEC_CRL_TYPE) -> SignedCRL object\n\
\n\
:Parameters:\n\
    cert : Certificate object\n\
        certificate used to lookup the CRL.\n\
    type : int\n\
        revocation list type\n\
        \n\
        may be one of:\n\
          - SEC_CRL_TYPE\n\
          - SEC_KRL_TYPE\n\
\n\
Returns a SignedCRL object found in the database given a certificate and revocation list type.\n\
"
);
static PyObject *
CertDB_find_crl_by_cert(CertDB *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"cert", "type", NULL};
    int type = SEC_CRL_TYPE;
    Certificate *py_cert = NULL;
    SECItem *der_cert;
    CERTSignedCrl *signed_crl;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!|i:find_crl_by_cert", kwlist,
                                     &CertificateType, &py_cert, &type))
        return NULL;

    der_cert = &py_cert->cert->derCert;
    if ((signed_crl = SEC_FindCrlByDERCert(self->handle, der_cert, type)) == NULL) {
        return set_nspr_error(NULL);
    }

    return SignedCRL_new_from_CERTSignedCRL(signed_crl);
}


static PyMethodDef CertDB_methods[] = {
    {"find_crl_by_name", (PyCFunction)CertDB_find_crl_by_name, METH_VARARGS|METH_KEYWORDS, CertDB_find_crl_by_name_doc},
    {"find_crl_by_cert", (PyCFunction)CertDB_find_crl_by_cert, METH_VARARGS|METH_KEYWORDS, CertDB_find_crl_by_cert_doc},
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
CertDB_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    CertDB *self;

    TraceObjNewEnter(type);

    if ((self = (CertDB *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }
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
    if ((self = (CertDB *) CertDBType.tp_new(&CertDBType, NULL, NULL)) == NULL) {
        return NULL;
    }

    self->handle = cert_handle;

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

static PyObject *
cert_distnames_new_from_CERTDistNames(CERTDistNames *names)
{
    PyObject *py_distnames = NULL;
    PyObject *py_sec_item = NULL;
    int i, len;

    len = names->nnames;
    if ((py_distnames = PyTuple_New(len)) == NULL) {
        return NULL;
    }

    for (i = 0; i< names->nnames; i++) {
        if ((py_sec_item = SecItem_new_from_SECItem(&names->names[i], SECITEM_dist_name)) == NULL) {
            Py_DECREF(py_distnames);
            return NULL;
        }
        PyTuple_SetItem(py_distnames, i, py_sec_item);
    }

    return py_distnames;
}

/* ========================================================================== */
/* ======================== CertificateExtension Class ====================== */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static PyObject *
CertificateExtension_get_name(CertificateExtension *self, void *closure)
{
    TraceMethodEnter(self);

    return oid_secitem_to_pystr_desc(&self->py_oid->item);
}

static PyObject *
CertificateExtension_get_critical(CertificateExtension *self, void *closure)
{
    TraceMethodEnter(self);

    return PyBool_FromLong(self->critical);
}

static PyObject *
CertificateExtension_get_oid(CertificateExtension *self, void *closure)
{
    TraceMethodEnter(self);

    Py_INCREF(self->py_oid);
    return (PyObject *)self->py_oid;
}

static PyObject *
CertificateExtension_get_oid_tag(CertificateExtension *self, void *closure)
{
    TraceMethodEnter(self);

    return oid_secitem_to_pyint_tag(&self->py_oid->item);
}

static PyObject *
CertificateExtension_get_value(CertificateExtension *self, void *closure)
{
    TraceMethodEnter(self);

    Py_INCREF(self->py_value);
    return (PyObject *)self->py_value;
}

static
PyGetSetDef CertificateExtension_getseters[] = {
    {"name",     (getter)CertificateExtension_get_name,     (setter)NULL, "name of extension", NULL},
    {"critical", (getter)CertificateExtension_get_critical, (setter)NULL, "extension is critical flag (boolean)", NULL},
    {"oid",      (getter)CertificateExtension_get_oid,      (setter)NULL, "oid of extension as SecItem", NULL},
    {"oid_tag",  (getter)CertificateExtension_get_oid_tag,  (setter)NULL, "oid of extension as a enumerated constant (e.g. tag)", NULL},
    {"value",    (getter)CertificateExtension_get_value,    (setter)NULL, "extension data as SecItem", NULL},
    {NULL}  /* Sentinel */
};

static PyMemberDef CertificateExtension_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

static PyObject *
CertificateExtension_str(CertificateExtension *self)
{
    return oid_secitem_to_pystr_desc(&self->py_oid->item);
}

static PyObject *
CertificateExtension_repr(CertificateExtension *self)
{
    return CertificateExtension_str(self);
}

static PyObject *
CertificateExtension_format_lines(CertificateExtension *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"level", NULL};
    int level = 0;
    Py_ssize_t len, i;
    PyObject *lines = NULL;
    PyObject *obj = NULL;
    PyObject *obj1 = NULL;
    SECOidTag oid_tag;
    PyObject *obj_lines = NULL;
    PyObject *tmp_args = NULL;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:format_lines", kwlist, &level))
        return NULL;

    if ((lines = PyList_New(0)) == NULL) {
        goto fail;
    }

    if ((obj = CertificateExtension_get_name(self, NULL)) == NULL) {
        goto fail;
    }
    FMT_OBJ_AND_APPEND(lines, _("Name"), obj, level, fail);
    Py_CLEAR(obj);

    if ((obj = CertificateExtension_get_critical(self, NULL)) == NULL) {
        goto fail;
    }
    FMT_OBJ_AND_APPEND(lines, _("Critical"), obj, level, fail);
    Py_CLEAR(obj);

    oid_tag = SECOID_FindOIDTag(&self->py_oid->item);

    switch(oid_tag) {
    case SEC_OID_PKCS12_KEY_USAGE:
        FMT_LABEL_AND_APPEND(lines, _("Usages"), level, fail);
        if ((tmp_args = Py_BuildValue("(O)", self->py_value)) == NULL) {
            goto fail;
        }
        if ((obj = cert_x509_key_usage(NULL, tmp_args)) == NULL) {
            goto fail;
        }
        Py_CLEAR(tmp_args);
        if ((obj_lines = make_line_pairs(level+1, obj)) == NULL) {
            goto fail;
        }
        APPEND_LINE_PAIRS_AND_CLEAR(lines, obj_lines, fail);
        break;

    case SEC_OID_X509_SUBJECT_KEY_ID:
        FMT_LABEL_AND_APPEND(lines, _("Data"), level, fail);
        if ((obj_lines = SECItem_der_to_hex(&self->py_value->item,
                                            OCTETS_PER_LINE_DEFAULT, HEX_SEPARATOR_DEFAULT)) == NULL) {
            goto fail;
        }
        APPEND_LINES_AND_CLEAR(lines, obj_lines, level+1, fail);
        break;

    case SEC_OID_X509_CRL_DIST_POINTS:
        if ((obj = CRLDistributionPts_new_from_SECItem(&self->py_value->item)) == NULL) {
            goto fail;
        }
        len = PyObject_Size(obj);
        if ((obj1 = PyString_FromFormat("CRL Distribution Points: [%d total]", len)) == NULL) {
            goto fail;
        }
        FMT_OBJ_AND_APPEND(lines, NULL, obj1, level, fail);
        Py_CLEAR(obj1);

        for (i = 0; i < len; i++) {
            if ((obj1 = PyString_FromFormat("Point [%d]:", i+1)) == NULL) {
                goto fail;
            }
            FMT_OBJ_AND_APPEND(lines, NULL, obj1, level+1, fail);
            Py_CLEAR(obj1);
            if ((obj1 = PySequence_GetItem(obj, i)) == NULL) {
                goto fail;
            }
            CALL_FORMAT_LINES_AND_APPEND(lines, obj1, level+2, fail);
            Py_CLEAR(obj1);
        }

        break;

    case SEC_OID_X509_AUTH_KEY_ID:
        if ((obj = AuthKeyID_new_from_SECItem(&self->py_value->item)) == NULL) {
            goto fail;
        }
        CALL_FORMAT_LINES_AND_APPEND(lines, obj, level, fail);
        Py_CLEAR(obj);

        break;

    case SEC_OID_X509_EXT_KEY_USAGE:
        FMT_LABEL_AND_APPEND(lines, _("Usages"), level, fail);
        if ((tmp_args = Py_BuildValue("(O)", self->py_value)) == NULL) {
            goto fail;
        }
        if ((obj = cert_x509_ext_key_usage(NULL, tmp_args, NULL)) == NULL) {
            goto fail;
        }
        Py_CLEAR(tmp_args);
        if ((obj_lines = make_line_pairs(level+1, obj)) == NULL) {
            goto fail;
        }
        APPEND_LINE_PAIRS_AND_CLEAR(lines, obj_lines, fail);
        break;

    case SEC_OID_X509_BASIC_CONSTRAINTS:
        if ((obj = BasicConstraints_new_from_SECItem(&self->py_value->item)) == NULL) {
            goto fail;
        }
        CALL_FORMAT_LINES_AND_APPEND(lines, obj, level, fail);
        Py_CLEAR(obj);

        break;

    case SEC_OID_X509_SUBJECT_ALT_NAME:
    case SEC_OID_X509_ISSUER_ALT_NAME:
        FMT_LABEL_AND_APPEND(lines, _("Names"), level, fail);
        if ((tmp_args = Py_BuildValue("(O)", self->py_value)) == NULL) {
            goto fail;
        }
        if ((obj = cert_x509_alt_name(NULL, tmp_args, NULL)) == NULL) {
            goto fail;
        }
        Py_CLEAR(tmp_args);
        if ((obj_lines = make_line_pairs(level+1, obj)) == NULL) {
            goto fail;
        }
        APPEND_LINE_PAIRS_AND_CLEAR(lines, obj_lines, fail);
        break;

    default:
        break;
    }

    return lines;

 fail:
    Py_XDECREF(lines);
    Py_XDECREF(obj);
    Py_XDECREF(obj1);
    Py_XDECREF(obj_lines);
    Py_XDECREF(tmp_args);
    return NULL;
}

static PyObject *
CertificateExtension_format(RSAPublicKey *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter(self);

    return format_from_lines((format_lines_func)CertificateExtension_format_lines, (PyObject *)self, args, kwds);
}

static PyMethodDef CertificateExtension_methods[] = {
    {"format_lines", (PyCFunction)CertificateExtension_format_lines,   METH_VARARGS|METH_KEYWORDS, generic_format_lines_doc},
    {"format",       (PyCFunction)CertificateExtension_format,         METH_VARARGS|METH_KEYWORDS, generic_format_doc},
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
CertificateExtension_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    CertificateExtension *self;

    TraceObjNewEnter(type);

    if ((self = (CertificateExtension *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }

    self->py_oid = NULL;
    self->py_value = NULL;
    self->critical = 0;

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static int
CertificateExtension_traverse(CertificateExtension *self, visitproc visit, void *arg)
{
    Py_VISIT(self->py_oid);
    Py_VISIT(self->py_value);
    return 0;
}

static int
CertificateExtension_clear(CertificateExtension* self)
{
    TraceMethodEnter(self);

    Py_CLEAR(self->py_oid);
    Py_CLEAR(self->py_value);
    return 0;
}

static void
CertificateExtension_dealloc(CertificateExtension* self)
{
    TraceMethodEnter(self);

    CertificateExtension_clear(self);
    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(CertificateExtension_doc,
"An object representing a certificate extension");

static int
CertificateExtension_init(CertificateExtension *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter(self);
    return 0;
}

static PyTypeObject CertificateExtensionType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.CertificateExtension",		/* tp_name */
    sizeof(CertificateExtension),		/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)CertificateExtension_dealloc,	/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    0,						/* tp_compare */
    (reprfunc)CertificateExtension_repr,	/* tp_repr */
    0,						/* tp_as_number */
    0,						/* tp_as_sequence */
    0,						/* tp_as_mapping */
    0,						/* tp_hash */
    0,						/* tp_call */
    (reprfunc)CertificateExtension_str,		/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,	/* tp_flags */
    CertificateExtension_doc,			/* tp_doc */
    (traverseproc)CertificateExtension_traverse,/* tp_traverse */
    (inquiry)CertificateExtension_clear,	/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    CertificateExtension_methods,		/* tp_methods */
    CertificateExtension_members,		/* tp_members */
    CertificateExtension_getseters,		/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)CertificateExtension_init,	/* tp_init */
    0,						/* tp_alloc */
    CertificateExtension_new,			/* tp_new */
};

PyObject *
CertificateExtension_new_from_CERTCertExtension(CERTCertExtension *extension)
{
    CertificateExtension *self = NULL;

    TraceObjNewEnter(NULL);
    if ((self = (CertificateExtension *) CertificateExtensionType.tp_new(&CertificateExtensionType, NULL, NULL)) == NULL) {
        return NULL;
    }

    if ((self->py_oid = (SecItem *)
         SecItem_new_from_SECItem(&extension->id, SECITEM_cert_extension_oid)) == NULL) {
        Py_CLEAR(self);
        return NULL;
    }

    if ((self->py_value = (SecItem *)
         SecItem_new_from_SECItem(&extension->value, SECITEM_cert_extension_value)) == NULL) {
        Py_CLEAR(self);
        return NULL;
    }

    if (extension->critical.data && extension->critical.len) {
	self->critical = extension->critical.data[0];
    }

    TraceObjNewLeave(self);
    return (PyObject *) self;
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

    TraceMethodEnter(self);

    pr_time = time_choice_secitem_to_prtime(&self->cert->validity.notBefore);
    LL_L2D(d_time, pr_time);

    return PyFloat_FromDouble(d_time);
}

static PyObject *
Certificate_get_valid_not_before_str(Certificate *self, void *closure)
{
    TraceMethodEnter(self);

    return time_choice_secitem_to_pystr(&self->cert->validity.notBefore);
}

static PyObject *
Certificate_get_valid_not_after(Certificate *self, void *closure)
{
    PRTime pr_time = 0;
    double d_time;

    TraceMethodEnter(self);

    pr_time = time_choice_secitem_to_prtime(&self->cert->validity.notAfter);
    LL_L2D(d_time, pr_time);

    return PyFloat_FromDouble(d_time);
}

static PyObject *
Certificate_get_valid_not_after_str(Certificate *self, void *closure)
{
    TraceMethodEnter(self);

    return time_choice_secitem_to_pystr(&self->cert->validity.notAfter);
}

static PyObject *
Certificate_get_subject(Certificate *self, void *closure)
{
    TraceMethodEnter(self);

    return DN_new_from_CERTName(&self->cert->subject);
}

static PyObject *
Certificate_get_subject_common_name(Certificate *self, void *closure)
{
    char *cn;
    PyObject *py_cn = NULL;

    TraceMethodEnter(self);

    if ((cn = CERT_GetCommonName(&self->cert->subject)) == NULL) {
        Py_RETURN_NONE;
    }

    py_cn = PyString_FromString(cn);
    PORT_Free(cn);

    return py_cn;
}

static PyObject *
Certificate_get_issuer(Certificate *self, void *closure)
{
    TraceMethodEnter(self);

    return DN_new_from_CERTName(&self->cert->issuer);
}

static PyObject *
Certificate_get_version(Certificate *self, void *closure)
{
    TraceMethodEnter(self);

    return integer_secitem_to_pylong(&self->cert->version);
}

static PyObject *
Certificate_get_serial_number(Certificate *self, void *closure)
{
    TraceMethodEnter(self);

    return integer_secitem_to_pylong(&self->cert->serialNumber);
}

// FIXME: should this come from SignedData?
static PyObject *
Certificate_get_signature_algorithm(Certificate *self, void *closure)
{
    TraceMethodEnter(self);

    return SignatureAlgorithm_new_from_SECAlgorithmID(&self->cert->signature);
}

static PyObject *
Certificate_get_signed_data(Certificate *self, void *closure)
{
    PyObject *py_signed_data = NULL;

    TraceMethodEnter(self);

    return py_signed_data = SignedData_new_from_SECItem(&self->cert->derCert);
}

static PyObject *
Certificate_get_der_data(Certificate *self, void *closure)
{
    SECItem der;

    TraceMethodEnter(self);

    der = self->cert->derCert;
    return PyString_FromStringAndSize((char *)der.data, der.len);
}

static PyObject *
Certificate_get_ssl_trust_str(Certificate *self, void *closure)
{
    TraceMethodEnter(self);

    if (self->cert->trust)
        return cert_trust_flags_str(self->cert->trust->sslFlags);
    else
        Py_RETURN_NONE;
}

static PyObject *
Certificate_get_email_trust_str(Certificate *self, void *closure)
{
    TraceMethodEnter(self);

    if (self->cert->trust)
        return cert_trust_flags_str(self->cert->trust->emailFlags);
    else
        Py_RETURN_NONE;
}

static PyObject *
Certificate_get_signing_trust_str(Certificate *self, void *closure)
{
    TraceMethodEnter(self);

    if (self->cert->trust)
        return cert_trust_flags_str(self->cert->trust->objectSigningFlags);
    else
        Py_RETURN_NONE;
}

static PyObject *
Certificate_get_subject_public_key_info(Certificate *self, void *closure)
{
    TraceMethodEnter(self);

    return SubjectPublicKeyInfo_new_from_CERTSubjectPublicKeyInfo(
               &self->cert->subjectPublicKeyInfo);
}

static PyObject *
Certificate_get_extensions(Certificate *self, void *closure)
{
    CERTCertExtension **extensions = NULL;
    int num_extensions, i;
    PyObject *extensions_tuple;

    TraceMethodEnter(self);

    /* First count how many extensions the cert has */
    for (extensions = self->cert->extensions, num_extensions = 0;
         extensions && *extensions;
         extensions++, num_extensions++);

    /* Allocate a tuple */
    if ((extensions_tuple = PyTuple_New(num_extensions)) == NULL) {
        return NULL;
    }

    /* Copy the extensions into the tuple */
    for (extensions = self->cert->extensions, i = 0; extensions && *extensions; extensions++, i++) {
        CERTCertExtension *extension = *extensions;
        PyObject *py_extension;
        
        if ((py_extension = CertificateExtension_new_from_CERTCertExtension(extension)) == NULL) {
            Py_DECREF(extensions_tuple);
            return NULL;
        }

        PyTuple_SetItem(extensions_tuple, i, py_extension);
    }

    return extensions_tuple;
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
     "certificate subject as a `DN` object", NULL},

    {"subject_common_name",     (getter)Certificate_get_subject_common_name,     NULL,
     "certificate subject", NULL},

    {"issuer",                  (getter)Certificate_get_issuer,                  NULL,
     "certificate issuer as a `DN` object",  NULL},

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

    {"subject_public_key_info", (getter)Certificate_get_subject_public_key_info, NULL,
     "certificate public info as SubjectPublicKeyInfo object",  NULL},

    {"extensions", (getter)Certificate_get_extensions, NULL,
     "certificate extensions as a tuple of CertificateExtension objects",  NULL},
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

    if (!PyArg_ParseTuple(args, "O:has_signer_in_ca_names",
                          &py_ca_names))
        return NULL;

    if ((ca_names = cert_distnames_as_CERTDistNames(py_ca_names)) == NULL) {
        return NULL;
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
"verify_now(certdb, check_sig, required_usages, [user_data1, ...]) -> valid_usages\n\
\n\
:Parameters:\n\
    certdb : CertDB object\n\
        CertDB certificate database object\n\
    check_sig : bool\n\
        True if certificate signatures should be checked\n\
    required_usages : integer\n\
        A bitfield of all cert usages that are required for verification\n\
        to succeed. If zero return all possible valid usages.\n\
    user_dataN : object\n\
        zero or more caller supplied parameters which will\n\
        be passed to the password callback function\n\
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
        Py_INCREF(args);
        parse_args = args;
    } else {
        parse_args = PyTuple_GetSlice(args, 0, n_base_args);
    }
    if (!PyArg_ParseTuple(parse_args, "O!O!l:verify_now",
                          &CertDBType, &py_certdb,
                          &PyBool_Type, &py_check_sig,
                          &required_usages)) {
        Py_DECREF(parse_args);
        return NULL;
    }
    Py_DECREF(parse_args);

    check_sig = PyInt_AsLong(py_check_sig);
    pin_args = PyTuple_GetSlice(args, n_base_args, argc);

    Py_BEGIN_ALLOW_THREADS
    if (CERT_VerifyCertificateNow(py_certdb->handle, self->cert, check_sig,
                                  required_usages, pin_args, &returned_usages) != SECSuccess) {
	Py_BLOCK_THREADS
        Py_DECREF(pin_args);
        return set_nspr_error(NULL);
    }
    Py_END_ALLOW_THREADS
    Py_DECREF(pin_args);

    return PyInt_FromLong(returned_usages);
}

static PyObject *
Certificate_format_lines(Certificate *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"level", NULL};
    int level = 0;
    Py_ssize_t len, i;
    PyObject *lines = NULL;
    PyObject *obj = NULL;
    PyObject *obj1 = NULL;
    PyObject *obj2 = NULL;
    PyObject *obj3 = NULL;
    PyObject *obj_line_pairs = NULL;
    PyObject *obj_lines = NULL;
    PyObject *ssl_trust_lines = NULL, *email_trust_lines = NULL, *signing_trust_lines = NULL;
    PyObject *tmp_args = NULL;
    PyObject *extensions = NULL;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:format_lines", kwlist, &level))
        return NULL;

    if ((lines = PyList_New(0)) == NULL) {
        goto fail;
    }

    FMT_LABEL_AND_APPEND(lines, _("Data"), level+1, fail);

    if ((obj = Certificate_get_version(self, NULL)) == NULL) {
        goto fail;
    }
    if ((obj1 = PyInt_FromLong(1)) == NULL) {
        goto fail;
    }
    if ((obj2 = PyNumber_Add(obj, obj1)) == NULL) {
        goto fail;
    }
    if ((obj3 = obj_sprintf("%d (%#x)", obj2, obj)) == NULL) {
        goto fail;
    }
    FMT_OBJ_AND_APPEND(lines, _("Version"), obj3, level+2, fail);
    Py_CLEAR(obj);
    Py_CLEAR(obj1);
    Py_CLEAR(obj2);
    Py_CLEAR(obj3);

    if ((obj = Certificate_get_serial_number(self, NULL)) == NULL) {
        goto fail;
    }
    if ((obj1 = obj_sprintf("%d (%#x)", obj, obj)) == NULL) {
        goto fail;
    }
    FMT_OBJ_AND_APPEND(lines, _("Serial Number"), obj1, level+2, fail);
    Py_CLEAR(obj);
    Py_CLEAR(obj1);

    if ((obj = Certificate_get_signature_algorithm(self, NULL)) == NULL) {
        goto fail;
    }
    FMT_OBJ_AND_APPEND(lines, _("Signature Algorithm"), obj, level+2, fail);
    Py_CLEAR(obj);

    if ((obj = Certificate_get_issuer(self, NULL)) == NULL) {
        goto fail;
    }
    FMT_OBJ_AND_APPEND(lines, _("Issuer"), obj, level+2, fail);
    Py_CLEAR(obj);

    FMT_LABEL_AND_APPEND(lines, _("Validity"), level+2, fail);

    if ((obj = Certificate_get_valid_not_before_str(self, NULL)) == NULL) {
        goto fail;
    }
    FMT_OBJ_AND_APPEND(lines, _("Not Before"), obj, level+3, fail);
    Py_CLEAR(obj);

    if ((obj = Certificate_get_valid_not_after_str(self, NULL)) == NULL) {
        goto fail;
    }
    FMT_OBJ_AND_APPEND(lines, _("Not After "), obj, level+3, fail);
    Py_CLEAR(obj);

    if ((obj = Certificate_get_subject(self, NULL)) == NULL) {
        goto fail;
    }
    FMT_OBJ_AND_APPEND(lines, _("Subject"), obj, level+2, fail);
    Py_CLEAR(obj);

    FMT_LABEL_AND_APPEND(lines, _("Subject Public Key Info"), level+2, fail);

    if ((obj = Certificate_get_subject_public_key_info(self, NULL)) == NULL) {
        goto fail;
    }

    CALL_FORMAT_LINES_AND_APPEND(lines, obj, level+3, fail);
    Py_CLEAR(obj);

    if ((extensions = Certificate_get_extensions(self, NULL)) == NULL) {
        goto fail;
    }

    len = PyTuple_Size(extensions);
    if ((obj = PyString_FromFormat("Signed Extensions: (%d)", len)) == NULL) {
        goto fail;
    }
    FMT_OBJ_AND_APPEND(lines, NULL, obj, level+1, fail);
    Py_CLEAR(obj);

    for (i = 0; i < len; i++) {
        obj = PyTuple_GetItem(extensions, i);
        CALL_FORMAT_LINES_AND_APPEND(lines, obj, level+2, fail);
        FMT_LABEL_AND_APPEND(lines, NULL, 0, fail);
    }
    Py_CLEAR(extensions);

    if ((ssl_trust_lines = Certificate_get_ssl_trust_str(self, NULL)) == NULL) {
        goto fail;
    }
    if ((email_trust_lines = Certificate_get_email_trust_str(self, NULL)) == NULL) {
        goto fail;
    }
    if ((signing_trust_lines = Certificate_get_signing_trust_str(self, NULL)) == NULL) {
        goto fail;
    }

    if ((ssl_trust_lines != Py_None) || (email_trust_lines != Py_None) || (signing_trust_lines != Py_None)) {
        FMT_LABEL_AND_APPEND(lines, _("Certificate Trust Flags"), level+2, fail);

        if (PyList_Check(ssl_trust_lines)) {
            FMT_LABEL_AND_APPEND(lines, _("SSL Flags"), level+3, fail);
            APPEND_LINES_AND_CLEAR(lines, ssl_trust_lines, level+4, fail);
        }

        if (PyList_Check(email_trust_lines)) {
            FMT_LABEL_AND_APPEND(lines, _("Email Flags"), level+3, fail);
            APPEND_LINES_AND_CLEAR(lines, email_trust_lines, level+4, fail);
        }

        if (PyList_Check(signing_trust_lines)) {
            FMT_LABEL_AND_APPEND(lines, _("Object Signing Flags"), level+3, fail);
            APPEND_LINES_AND_CLEAR(lines, signing_trust_lines, level+4, fail);
        }

    }
    Py_CLEAR(ssl_trust_lines);
    Py_CLEAR(email_trust_lines);
    Py_CLEAR(signing_trust_lines);

    FMT_LABEL_AND_APPEND(lines, _("Fingerprint (MD5)"), level+1, fail);

    if ((obj = Certificate_get_der_data(self, NULL)) == NULL) {
        goto fail;
    }

    if ((tmp_args = Py_BuildValue("(O)", obj)) == NULL) {
        goto fail;
    }
    Py_CLEAR(obj);

    if ((obj = pk11_md5_digest(NULL, tmp_args)) == NULL) {
        goto fail;
    }
    Py_CLEAR(tmp_args);

    APPEND_OBJ_TO_HEX_LINES_AND_CLEAR(lines, obj, level+2, fail);

    FMT_LABEL_AND_APPEND(lines, _("Fingerprint (SHA1)"), level+1, fail);

    if ((obj = Certificate_get_der_data(self, NULL)) == NULL) {
        goto fail;
    }

    if ((tmp_args = Py_BuildValue("(O)", obj)) == NULL) {
        goto fail;
    }
    Py_CLEAR(obj);

    if ((obj = pk11_sha1_digest(NULL, tmp_args)) == NULL) {
        goto fail;
    }
    Py_CLEAR(tmp_args);

    APPEND_OBJ_TO_HEX_LINES_AND_CLEAR(lines, obj, level+2, fail);

    FMT_LABEL_AND_APPEND(lines, _("Signature"), level+1, fail);

    if ((obj = Certificate_get_signed_data(self, NULL)) == NULL) {
        goto fail;
    }

    CALL_FORMAT_LINES_AND_APPEND(lines, obj, level+2, fail);
    Py_CLEAR(obj);

    return lines;
 fail:
    Py_XDECREF(obj);
    Py_XDECREF(obj1);
    Py_XDECREF(obj2);
    Py_XDECREF(obj3);
    Py_XDECREF(lines);
    Py_XDECREF(obj_line_pairs);
    Py_XDECREF(obj_lines);
    Py_XDECREF(tmp_args);
    Py_XDECREF(ssl_trust_lines);
    Py_XDECREF(email_trust_lines);
    Py_XDECREF(signing_trust_lines);
    Py_XDECREF(extensions);
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
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
Certificate_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Certificate *self;

    TraceObjNewEnter(type);

    if ((self = (Certificate *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }
    self->cert = NULL;

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static void
Certificate_dealloc(Certificate* self)
{
    TraceMethodEnter(self);

    if (self->cert)
        CERT_DestroyCertificate(self->cert);

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(Certificate_doc,
"Certificate(data=None)\n\
\n\
:Parameters:\n\
    data : SecItem or str or any buffer compatible object\n\
        Data to initialize the certificate from, must be in DER format\n\
\n\
An object representing a Certificate");

static int
Certificate_init(Certificate *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"data", NULL};
    PyObject *py_data = NULL;
    SECItem tmp_item;
    SECItem *der_item = NULL;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O:Certificate", kwlist,
                                     &py_data))
        return -1;

    if (py_data) {
        if (PySecItem_Check(py_data)) {
            der_item = &((SecItem *)py_data)->item;
        } else if (PyObject_CheckReadBuffer(py_data)) {
            unsigned char *data = NULL;
            Py_ssize_t data_len;

            if (PyObject_AsReadBuffer(py_data, (void *)&data, &data_len))
                return -1;

            tmp_item.data = data;
            tmp_item.len = data_len;
            der_item = &tmp_item;
        } else {
            PyErr_SetString(PyExc_TypeError, "data must be SecItem or buffer compatible");
            return -1;
        }

        if ((self->cert = CERT_DecodeDERCertificate(der_item, PR_TRUE, NULL)) == NULL) {
            set_nspr_error("bad certificate initialization data");
            return -1;
        }
    }

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
    "nss.nss.Certificate",			/* tp_name */
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

static PyObject *
Certificate_new_from_CERTCertificate(CERTCertificate *cert)
{
    Certificate *self = NULL;

    TraceObjNewEnter(NULL);

    if ((self = (Certificate *) CertificateType.tp_new(&CertificateType, NULL, NULL)) == NULL) {
        return NULL;
    }

    self->cert = cert;

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

    if ((self = (PrivateKey *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }
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
    "nss.nss.PrivateKey",			/* tp_name */
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

static PyObject *
PrivateKey_new_from_SECKEYPrivateKey(SECKEYPrivateKey *private_key)
{
    PrivateKey *self = NULL;

    TraceObjNewEnter(NULL);
    if ((self = (PrivateKey *) PrivateKeyType.tp_new(&PrivateKeyType, NULL, NULL)) == NULL) {
        return NULL;
    }

    self->private_key = private_key;

    TraceObjNewLeave(self);
    return (PyObject *) self;
}


/* ========================================================================== */
/* ============================== SignedCRL Class =========================== */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static
PyGetSetDef SignedCRL_getseters[] = {
    {NULL}  /* Sentinel */
};

static PyMemberDef SignedCRL_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

PyDoc_STRVAR(SignedCRL_delete_permanently_doc,
"delete_permanently()\n\
\n\
Permanently remove the CRL from the database.\n\
");

static PyObject *
SignedCRL_delete_permanently(SignedCRL *self, PyObject *args)
{
    TraceMethodEnter(self);

    if (SEC_DeletePermCRL(self->signed_crl) != SECSuccess) {
        return set_nspr_error(NULL);
    }

    Py_RETURN_NONE;
}

static PyMethodDef SignedCRL_methods[] = {
    {"delete_permanently", (PyCFunction)SignedCRL_delete_permanently, METH_NOARGS,  SignedCRL_delete_permanently_doc},
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
SignedCRL_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    SignedCRL *self;

    TraceObjNewEnter(type);

    if ((self = (SignedCRL *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }
    self->signed_crl = NULL;

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static void
SignedCRL_dealloc(SignedCRL* self)
{
    TraceMethodEnter(self);

    if (self->signed_crl)
        SEC_DestroyCrl(self->signed_crl);

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(SignedCRL_doc,
"An object representing a signed certificate revocation list");

static int
SignedCRL_init(SignedCRL *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter(self);
    return 0;
}

static PyTypeObject SignedCRLType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.SignedCRL",			/* tp_name */
    sizeof(SignedCRL),				/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)SignedCRL_dealloc,		/* tp_dealloc */
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
    SignedCRL_doc,				/* tp_doc */
    0,						/* tp_traverse */
    0,						/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    SignedCRL_methods,				/* tp_methods */
    SignedCRL_members,				/* tp_members */
    SignedCRL_getseters,			/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)SignedCRL_init,			/* tp_init */
    0,						/* tp_alloc */
    SignedCRL_new,				/* tp_new */
};

static PyObject *
SignedCRL_new_from_CERTSignedCRL(CERTSignedCrl *signed_crl)
{
    SignedCRL *self = NULL;

    TraceObjNewEnter(NULL);
    if ((self = (SignedCRL *) SignedCRLType.tp_new(&SignedCRLType, NULL, NULL)) == NULL) {
        return NULL;
    }

    self->signed_crl = signed_crl;

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

/* ========================================================================== */
/* =============================== AVA Class ================================ */
/* ========================================================================== */

/*
 * Note: CERT_CopyAVA, CERT_GetAVATag, CERT_CompareAVA, CERT_CreateAVA,
 * and CERT_DecodeAVAValue are defined in cert.h
 *
 * But only CERT_GetAVATag, CERT_CreateAVA, CERT_DecodeAVAValue are exported
 * by nss.def
 *
 * That means CERT_CopyAVA and CERT_CompareAVA are defined as public but aren't.
 */

/* ============================ Attribute Access ============================ */

static PyObject *
AVA_get_oid(AVA *self, void *closure)
{
    TraceMethodEnter(self);

    return SecItem_new_from_SECItem(&self->ava->type, SECITEM_oid);
}

static PyObject *
AVA_get_oid_tag(AVA *self, void *closure)
{
    TraceMethodEnter(self);

    return PyInt_FromLong(CERT_GetAVATag(self->ava));
}

static PyObject *
AVA_get_value(AVA *self, void *closure)
{
    TraceMethodEnter(self);

    return SecItem_new_from_SECItem(&self->ava->value, SECITEM_utf8_string);
}

static PyObject *
AVA_get_value_str(AVA *self, void *closure)
{
    TraceMethodEnter(self);

    return AVA_repr(self);
}

static
PyGetSetDef AVA_getseters[] = {
    {"oid",       (getter)AVA_get_oid, (setter)NULL,
     "The OID (e.g. type) of the AVA as a SecItem", NULL},
    {"oid_tag",   (getter)AVA_get_oid_tag, (setter)NULL,
     "The OID tag enumerated constant (i.e. SEC_OID_AVA_*) of the AVA's type", NULL},
    {"value",     (getter)AVA_get_value, (setter)NULL,
     "The value of the AVA as a SecItem", NULL},
    {"value_str", (getter)AVA_get_value_str, (setter)NULL,
     "The value of the AVA as a UTF-8 encoded string", NULL},
    {NULL}  /* Sentinel */
};

static PyMemberDef AVA_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */


/*
 * Compares two CERTAVA's, returns -1 if a < b, 0 if a == b, 1 if a > b
 * If error, returns -2
 */
static int
CERTAVA_compare(CERTAVA *a, CERTAVA *b)
{
    SECComparison cmp_result;
    PyObject *a_val_str, *b_val_str;

    if (a == NULL && b == NULL) return 0;
    if (a == NULL && b != NULL) return -1;
    if (a != NULL && b == NULL) return 1;

    if ((cmp_result = SECITEM_CompareItem(&a->type, &b->type)) != SECEqual) {
        return cmp_result == SECLessThan ? -1 : 1;
    }

    /* Attribute types matched, are values equal? */
    if ((cmp_result = SECITEM_CompareItem(&a->value,
                                          &b->value)) == SECEqual) {
        return 0;
    }

    /* No values not equal, compare as case insenstive strings */
    a_val_str = CERTAVA_value_to_pystr(a);
    b_val_str = CERTAVA_value_to_pystr(b);
    if (a_val_str == NULL || b_val_str == NULL) {
        Py_XDECREF(a_val_str);
        Py_XDECREF(b_val_str);
        PyErr_SetString(PyExc_ValueError, "Failed to convert AVA value to string");
        return -2;
    }

    cmp_result = strcasecmp(PyString_AS_STRING(a_val_str),
                            PyString_AS_STRING(b_val_str));
    Py_DECREF(a_val_str);
    Py_DECREF(b_val_str);
    return (cmp_result == 0) ? 0 : ((cmp_result < 0) ? -1 : 1);
}

static int
AVA_compare(AVA *self, AVA *other)
{
    int cmp_result;

    if (!PyAVA_Check(other)) {
        PyErr_SetString(PyExc_TypeError, "Bad type, must be AVA");
        return -1;
    }

    cmp_result = CERTAVA_compare(self->ava, other->ava);
    if (cmp_result == -2) {
        return -1;
    }
    return cmp_result;
}

static PyMethodDef AVA_methods[] = {
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
AVA_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    AVA *self;

    TraceObjNewEnter(type);

    if ((self = (AVA *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }

    if ((self->arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE)) == NULL) {
        type->tp_free(self);
        return set_nspr_error(NULL);
    }

    self->ava = NULL;
    
    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static void
AVA_dealloc(AVA* self)
{
    TraceMethodEnter(self);

    if (self->arena) {
        PORT_FreeArena(self->arena, PR_FALSE);
    }

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(AVA_doc,
"An object representing an AVA (attribute value assertion).\n\
\n\
AVA(type, value)\n\
\n\
:Parameters:\n\
     type : may be one of integer, string, SecItem\n\
         What kind of attribute is being created. May be\n\
         one of:\n\
\n\
         * integer: A SEC OID enumeration constant (i.e. SEC_OID_*)\n\
           for example SEC_OID_AVA_COMMON_NAME.\n\
         * string: A string either as the ava name, for example 'cn'\n\
           or as the dotted decimal representation, for example\n\
           'OID.2.5.4.3'. Case is not significant for either form.\n\
         * SecItem: A SecItem object encapsulating the OID in \n\
           DER format.\n\
     value : string\n\
         The value of the AVA, must be a string.\n\
\n\
RDN's (Relative Distinguished Name) are composed from AVA's.\n\
An `RDN` is a sequence of AVA's.\n\
\n\
An example of an AVA is \"CN=www.redhat.com\" where CN is the X500\n\
directory abbrevation for \"Common Name\".\n\
\n\
An AVA is composed of two items:\n\
\n\
type\n\
    Specifies the attribute (e.g. CN). AVA types are specified by\n\
    predefined OID's (Object Identifiers). For example the OID of CN\n\
    is 2.5.4.3 ({joint-iso-itu-t(2) ds(5) attributeType(4) commonName(3)})\n\
    OID's in NSS are encapsulated in a SecItem as a DER encoded OID.\n\
    Because DER encoded OID's are less than ideal mechanisms by which\n\
    to specify an item NSS has mapped each OID to a integral enumerated\n\
    constant called an OID tag (i.e. SEC_OID_*). Many of the NSS API's\n\
    will accept an OID tag number instead of DER encoded OID in a SecItem.\n\
    One can easily convert between DER encoded OID's, tags, and their\n\
    string representation in dotted-decimal format. The enumerated OID\n\
    constants are the most efficient in most cases.\n\
value\n\
    The value of the attribute (e.g. 'www.redhat.com').\n\
\n\
Examples::\n\
\n\
    The AVA cn=www.redhat.com can be created in any of the follow ways:\n\
\n\
    ava = nss.AVA('cn', 'www.redhat.com')\n\
    ava = nss.AVA(nss.SEC_OID_AVA_COMMON_NAME, 'www.redhat.com')\n\
    ava = nss.AVA('2.5.4.3', 'www.redhat.com')\n\
    ava = nss.AVA('OID.2.5.4.3', 'www.redhat.com')\n\
");

static int
AVA_init(AVA *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"type", "value", NULL};
    PyObject *py_type = NULL;
    PyObject *py_value = NULL;
    PyObject *py_value_utf8 = NULL;
    int oid_tag = SEC_OID_UNKNOWN;
    int value_type;
    char *value_string;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO:AVA", kwlist,
                                     &py_type, &py_value))
        return -1;

    if ((oid_tag = get_oid_tag_from_object(py_type)) == -1) {
        return -1;
    }

    if (oid_tag == SEC_OID_UNKNOWN) {
        PyObject *type_str = PyObject_Str(py_type);
        PyErr_Format(PyExc_ValueError, "unable to convert \"%s\" to known OID",
                     PyString_AsString(type_str));
        Py_DECREF(type_str);
        return -1;
    }

    if (PyString_Check(py_value) || PyUnicode_Check(py_value)) {
        if (PyString_Check(py_value)) {
            py_value_utf8 = py_value;
            Py_INCREF(py_value_utf8);
        } else {
            py_value_utf8 =  PyUnicode_AsUTF8String(py_value);
        }

        if ((value_string = PyString_AsString(py_value_utf8)) == NULL) {
            Py_DECREF(py_value_utf8);
            return -1;
        }
    } else {
        PyErr_Format(PyExc_TypeError, "AVA value must be a string, not %.200s",
                     Py_TYPE(py_type)->tp_name);
        return -1;
    }

    value_type = ava_oid_tag_to_value_type(oid_tag);
    if ((self->ava = CERT_CreateAVA(self->arena, oid_tag, value_type, value_string)) == NULL) {
        set_nspr_error("could not create AVA, oid tag = %d, value = \"%s\"",
                       oid_tag, value_string);
	Py_XDECREF(py_value_utf8);
        return -1;
    }

    Py_XDECREF(py_value_utf8);
    return 0;
}

static PyObject *
AVA_repr(AVA *self)
{
    PyObject *py_value_str;

    if ((py_value_str = CERTAVA_value_to_pystr(self->ava)) == NULL) {
        return PyString_FromFormat("<%s object at %p>",
                                   Py_TYPE(self)->tp_name, self);
    }
    return py_value_str;
}

static PyTypeObject AVAType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.AVA",				/* tp_name */
    sizeof(AVA),				/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)AVA_dealloc,			/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    (cmpfunc)AVA_compare,			/* tp_compare */
    (reprfunc)AVA_repr,				/* tp_repr */
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
    AVA_doc,					/* tp_doc */
    0,						/* tp_traverse */
    0,						/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    AVA_methods,				/* tp_methods */
    AVA_members,				/* tp_members */
    AVA_getseters,				/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)AVA_init,				/* tp_init */
    0,						/* tp_alloc */
    AVA_new,					/* tp_new */
};

PyObject *
AVA_new_from_CERTAVA(CERTAVA *ava)
{
    AVA *self = NULL;

    TraceObjNewEnter(NULL);

    if ((self = (AVA *) AVAType.tp_new(&AVAType, NULL, NULL)) == NULL)
        return NULL;

    if ((self->ava = (CERTAVA*) PORT_ArenaZNew(self->arena, CERTAVA)) == NULL) {
        set_nspr_error(NULL);
        Py_CLEAR(self);
        return NULL;
    }

    if (SECITEM_CopyItem(NULL, &self->ava->type, &ava->type) != SECSuccess) {
        set_nspr_error(NULL);
        Py_CLEAR(self);
        return NULL;
    }
    self->ava->type.type = siDEROID; /* NSS often fails to set this so force it */

    if (SECITEM_CopyItem(NULL, &self->ava->value, &ava->value) != SECSuccess) {
        set_nspr_error(NULL);
        Py_CLEAR(self);
        return NULL;
    }

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

/* ========================================================================== */
/* =============================== RDN Class ================================ */
/* ========================================================================== */

/*
 * Note: CERT_AddAVA, CERT_AddRDN, CERT_CompareRDN, CERT_CopyRDN, CERT_CreateName
 * CERT_CreateRDN, CERT_DestroyRDN are defined in cert.h
 *
 * But only CERT_AddRDN, CERT_CopyRDN, CERT_CreateName, CERT_CreateRDN
 * are exported by nss.def
 *
 * That means CERT_AddAVA, CERT_CompareRDN, CERT_DestroyRDN
 * are defined as public but aren't. Note CERT_DestroyRDN has no implementation.
 */

/* ============================ Attribute Access ============================ */

static
PyGetSetDef RDN_getseters[] = {
    {NULL}  /* Sentinel */
};

static PyMemberDef RDN_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

/*
 * Compares two CERTRDN's, returns -1 if a < b, 0 if a == b, 1 if a > b
 * If error, returns -2
 */
static int
CERTRDN_compare(CERTRDN *a, CERTRDN *b)
{
    SECComparison cmp_result;
    int a_len, b_len;
    CERTAVA **a_avas, *a_ava;
    CERTAVA **b_avas, *b_ava;

    if (a == NULL && b == NULL) return 0;
    if (a == NULL && b != NULL) return -1;
    if (a != NULL && b == NULL) return 1;

    a_len = CERTRDN_ava_count(a);
    b_len = CERTRDN_ava_count(b);

    if (a_len < b_len) return -1;
    if (a_len > b_len) return  1;

    for (a_avas = a->avas, b_avas = b->avas;
         a_avas && (a_ava = *a_avas) && b_avas && (b_ava = *b_avas);
         a_avas++, b_avas++) {
        if ((cmp_result = CERTAVA_compare(a_ava, b_ava)) != 0) {
            return cmp_result;
        }
    }
    return 0;
}

static int
RDN_compare(RDN *self, RDN *other)
{
    int cmp_result;

    if (!PyRDN_Check(other)) {
        PyErr_SetString(PyExc_TypeError, "Bad type, must be RDN");
        return -1;
    }

    cmp_result = CERTRDN_compare(self->rdn, other->rdn);
    if (cmp_result == -2) {
        return -1;
    }
    return cmp_result;
}

PyDoc_STRVAR(RDN_has_key_doc,
"has_key(arg) -> bool\n\
\n\
:Parameters:\n\
    arg : string or integer\n\
        canonical name (e.g. 'cn') or oid dotted-decimal or\n\
        SEC_OID_* enumeration constant\n\
\n\
return True if RDN has an AVA whose oid can be identified by arg.\n\
");

static PyObject *
RDN_has_key(RDN *self, PyObject *args)
{
    PyObject *arg;
    int oid_tag;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "O:has_key",
                          &arg))
        return NULL;

    oid_tag = get_oid_tag_from_object(arg);
    if (oid_tag == SEC_OID_UNKNOWN || oid_tag == -1) {
        Py_RETURN_FALSE;
    }

    if (CERTRDN_has_tag(self->rdn, oid_tag)) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

/* =========================== Sequence Protocol ============================ */

static Py_ssize_t
CERTRDN_ava_count(CERTRDN *rdn)
{
    Py_ssize_t count;
    CERTAVA **avas;

    if (!rdn) return 0;
    for (avas = rdn->avas, count = 0; *avas; avas++, count++);

    return count;
}

static Py_ssize_t
RDN_length(RDN *self)
{
    return CERTRDN_ava_count(self->rdn);
}

static PyObject *
RDN_item(RDN *self, register Py_ssize_t i)
{
    Py_ssize_t count = 0;
    CERTAVA **avas;

    if (i < 0 || !self->rdn || self->rdn->avas == NULL) {
        PyErr_SetString(PyExc_IndexError, "RDN index out of range");
        return NULL;
    }

    for (avas = self->rdn->avas, count = 0; *avas && count < i; avas++, count++);

    if (!*avas) {
        PyErr_SetString(PyExc_IndexError, "RDN index out of range");
        return NULL;
    }

    return AVA_new_from_CERTAVA(*avas);
}


static bool
CERTRDN_has_tag(CERTRDN *rdn, int tag)
{
    CERTAVA **avas;
    CERTAVA *ava = NULL;
    
    if (!rdn) return false;
    for (avas = rdn->avas; avas && (ava = *avas); avas++) {
        int ava_tag = CERT_GetAVATag(ava);
        if (tag == ava_tag) {
            return true;
        }
    }
    return false;
}

static PyObject *
CERTRDN_get_matching_tag_list(CERTRDN *rdn, int tag)
{
    PyObject *list = NULL;
    PyObject *py_ava = NULL;
    CERTAVA **avas, *ava;

    if ((list = PyList_New(0)) == NULL) {
        return NULL;
    }

    if (!rdn) {
        return list;
    }

    for (avas = rdn->avas; avas && (ava = *avas); avas++) {
        int ava_tag = CERT_GetAVATag(ava);
        if (tag == ava_tag) {
            if ((py_ava = AVA_new_from_CERTAVA(ava)) == NULL) {
                Py_DECREF(list);
                return NULL;
            }
            PyList_Append(list, py_ava);
        }
    }

    return list;
}
static PyObject*
RDN_subscript(RDN *self, PyObject* item)
{
    PyObject* result;

    if (PyIndex_Check(item)) {
        Py_ssize_t i = PyNumber_AsSsize_t(item, PyExc_IndexError);
	
        if (i == -1 && PyErr_Occurred())
            return NULL;
        if (i < 0)
            i += RDN_length(self);
        return RDN_item(self, i);
    } else if (PySlice_Check(item)) {
        Py_ssize_t start, stop, step, slicelength, cur, i;
        PyObject* py_ava;

        if (PySlice_GetIndicesEx((PySliceObject*)item, RDN_length(self),
				 &start, &stop, &step, &slicelength) < 0) {
            return NULL;
        }

        if (slicelength <= 0) {
            return PyList_New(0);
        } else {
            result = PyList_New(slicelength);
            if (!result) return NULL;
            
            for (cur = start, i = 0; i < slicelength; cur += step, i++) {
                /* We don't need to bump the ref count because RDN_item
                 * returns a new object */
                py_ava = RDN_item(self, cur);
                if (PyList_SetItem(result, i, py_ava) == -1) {
                    Py_DECREF(result);
                    return NULL;
                }
            }
            return result;
	}
    } else if (PyString_Check(item) || PyUnicode_Check(item) || PySecItem_Check(item)) {
        int oid_tag;

        if ((oid_tag = get_oid_tag_from_object(item)) == -1) {
            return NULL;
        }

        if (oid_tag == SEC_OID_UNKNOWN) {
            if (PyString_Check(item) || PyUnicode_Check(item)) {
                char *name = PyString_AsString(item);
                PyErr_Format(PyExc_KeyError, "oid name unknown: \"%s\"", name);
                return NULL;
            } else {
                PyErr_SetString(PyExc_KeyError, "oid unknown");
                return NULL;
            }
        }

        if ((result = CERTRDN_get_matching_tag_list(self->rdn, oid_tag)) == NULL) {
            return NULL;
        }

        if (PyList_Size(result) == 0) {
            Py_DECREF(result);
            if (PyString_Check(item) || PyUnicode_Check(item)) {
                char *name = PyString_AsString(item);
                PyErr_Format(PyExc_KeyError, "oid name not found: \"%s\"", name);
                return NULL;
            } else {
                PyErr_SetString(PyExc_KeyError, "oid not found");
                return NULL;
            }
        } else {
            return result;
        }
    } else {
        PyErr_Format(PyExc_TypeError,
                     "indices must be integers or strings, not %.200s",
                     item->ob_type->tp_name);
        return NULL;
    }
    return NULL;
}

static PyObject *
RDN_repr(RDN *self)
{
    return CERTRDN_to_pystr(self->rdn);
}

static PyMethodDef RDN_methods[] = {
    {"has_key", (PyCFunction)RDN_has_key, METH_VARARGS, RDN_has_key_doc},
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
RDN_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    RDN *self;

    TraceObjNewEnter(type);

    if ((self = (RDN *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }

    if ((self->arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE)) == NULL) {
        type->tp_free(self);
        return set_nspr_error(NULL);
    }

    self->rdn = NULL;

    TraceObjNewLeave(self);

    return (PyObject *)self;
}

static void
RDN_dealloc(RDN* self)
{
    TraceMethodEnter(self);

    if (self->arena) {
        PORT_FreeArena(self->arena, PR_FALSE);
    }

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(RDN_doc,
"An object representing an X501 Relative Distinguished Name (e.g. RDN).\n\
\n\
RDN objects contain an ordered list of `AVA` objects. \n\
\n\
Examples::\n\
\n\
    RDN()\n\
    RDN(nss.AVA('cn', 'www.redhat.com'))\n\
    RDN([ava0, ava1])\n\
\n\
The RDN object constructor may be invoked with zero or more\n\
`AVA` objects, or you may optionally pass a list or tuple of `AVA`\n\
objects.\n\
\n\
RDN objects contain an ordered list of `AVA` objects. The\n\
RDN object has both sequence and mapping behaviors with respect to\n\
the AVA's they contain. Thus you can index an AVA by position, by\n\
name, or by SecItem (if it's an OID). You can iterate over the list,\n\
get it's length or take a slice.\n\
\n\
If you index by string the string may be either a canonical name for\n\
the AVA type (e.g. 'cn') or the dotted-decimal notation for the OID\n\
(e.g. 2.5.4.3). There may be multiple AVA's in a RDN whose type matches\n\
(e.g. OU=engineering+OU=boston). It is not common to have more than\n\
one AVA in a RDN with the same type. However because of the possiblity\n\
of being multi-valued when indexing by type a list is always returned\n\
containing the matching AVA's. Thus::\n\
\n\
    rdn = nss.RDN(nss.AVA('OU', 'engineering'))\n\
    rdn['ou']\n\
        returns [AVA('OU=engineering')\n\
\n\
    rdn = nss.RDN(nss.AVA('OU', 'engineering'), nss.AVA('OU', 'boston'))\n\
    rdn['ou']\n\
        returns [AVA('OU=boston'), AVA('OU=engineering')]\n\
\n\
Examples::\n\
\n\
    rdn = nss.RDN(nss.AVA('cn', 'www.redhat.com'))\n\
    str(rdn)\n\
       returns 'CN=www.redhat.com'\n\
    rdn[0]\n\
       returns an `AVA` object with the value C=US\n\
    rdn['cn']\n\
        returns a list comprised of an `AVA` object with the value CN=www.redhat.com\n\
    rdn['2.5.4.3']\n\
        returns a list comprised of an `AVA` object with the value CN=www.redhat.com\n\
        because 2.5.4.3 is the dotted-decimal OID for common name (i.e. cn)\n\
    rdn.has_key('cn')\n\
        returns True because the RDN has a common name RDN\n\
    rdn.has_key('2.5.4.3')\n\
        returns True because the RDN has a common name AVA\n\
        because 2.5.4.3 is the dotted-decimal OID for common name (i.e. cn)\n\
    len(rdn)\n\
       returns 1 because there is one `AVA` object in it\n\
    list(rdn)\n\
       returns a list of each `AVA` object in it\n\
\n\
");

static int
RDN_init(RDN *self, PyObject *args, PyObject *kwds)
{
    PyObject *sequence, *item;
    Py_ssize_t sequence_len, i;
    AVA *py_ava;
    CERTAVA *ava_arg[MAX_AVAS+1];  /* +1 for NULL terminator */

    TraceMethodEnter(self);

    if (PyTuple_GET_SIZE(args) > 0) {
        sequence = PyTuple_GetItem(args, 0);
        if (!(PyTuple_Check(sequence) || PyList_Check(sequence))) {
            sequence = args;
        }
        sequence_len = PySequence_Length(sequence);
        if (sequence_len > MAX_AVAS) {
            PyErr_Format(PyExc_ValueError, "to many AVA items, maximum is %d, received %d",
                         MAX_AVAS-1, sequence_len);
            return -1;
        }
        for (i = 0; i < sequence_len && i < MAX_AVAS; i++) {
            item = PySequence_ITEM(sequence, i);
            if (PyAVA_Check(item)) {
                py_ava = (AVA *)item;
                if ((ava_arg[i] = CERT_CopyAVA(self->arena, py_ava->ava)) == NULL) {
                    set_nspr_error(NULL);
                    Py_DECREF(item);
                    return -1;
                }
                Py_DECREF(item);
            } else {
                PyErr_Format(PyExc_TypeError, "item %d must be an AVA object, not %.200s",
                             i, Py_TYPE(item)->tp_name);
                Py_DECREF(item);
                return -1;
                }
            }

        for (; i < MAX_AVAS+1; i++) ava_arg[i] = NULL;

        if ((self->rdn = CERT_CreateRDN(self->arena,
                                        ava_arg[0], ava_arg[1], ava_arg[2], ava_arg[3],
                                        ava_arg[4], ava_arg[5], ava_arg[6], ava_arg[7],
                                        ava_arg[8], ava_arg[9], ava_arg[10])) == NULL) {
            set_nspr_error(NULL);
            return -1;
        }
    }

    return 0;
}

static PySequenceMethods RDN_as_sequence = {
    (lenfunc)RDN_length,			/* sq_length */
    0,						/* sq_concat */
    0,						/* sq_repeat */
    (ssizeargfunc)RDN_item,			/* sq_item */
    0,						/* sq_slice */
    0,						/* sq_ass_item */
    0,						/* sq_ass_slice */
    0,						/* sq_contains */
    0,						/* sq_inplace_concat */
    0,						/* sq_inplace_repeat */
};

static PyMappingMethods RDN_as_mapping = {
    (lenfunc)RDN_length,			/* mp_length */
    (binaryfunc)RDN_subscript,			/* mp_subscript */
    0,						/* mp_ass_subscript */
};

static PyTypeObject RDNType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.RDN",				/* tp_name */
    sizeof(RDN),				/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)RDN_dealloc,			/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    (cmpfunc)RDN_compare,			/* tp_compare */
    (reprfunc)RDN_repr,				/* tp_repr */
    0,						/* tp_as_number */
    &RDN_as_sequence,				/* tp_as_sequence */
    &RDN_as_mapping,				/* tp_as_mapping */
    0,						/* tp_hash */
    0,						/* tp_call */
    0,						/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
    RDN_doc,					/* tp_doc */
    0,						/* tp_traverse */
    0,						/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    RDN_methods,				/* tp_methods */
    RDN_members,				/* tp_members */
    RDN_getseters,				/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)RDN_init,				/* tp_init */
    0,						/* tp_alloc */
    RDN_new,					/* tp_new */
};

PyObject *
RDN_new_from_CERTRDN(CERTRDN *rdn)
{
    RDN *self = NULL;
    int i;
    CERTAVA *ava_arg[MAX_AVAS+1];  /* +1 for NULL terminator */
    CERTAVA **avas, *ava;

    TraceObjNewEnter(NULL);

    if ((self = (RDN *) RDNType.tp_new(&RDNType, NULL, NULL)) == NULL) {
        return NULL;
    }
    
    i = 0;
    if (rdn) {
        for (avas = rdn->avas; i < MAX_AVAS && avas && (ava = *avas); avas++, i++) {
            if ((ava_arg[i] = CERT_CopyAVA(self->arena, ava)) == NULL) {
                set_nspr_error(NULL);
                Py_CLEAR(self);
                return NULL;
            }
        }
    }

    for (; i < MAX_AVAS+1; i++) ava_arg[i] = NULL;

    if ((self->rdn = CERT_CreateRDN(self->arena,
                                    ava_arg[0], ava_arg[1], ava_arg[2], ava_arg[3],
                                    ava_arg[4], ava_arg[5], ava_arg[6], ava_arg[7],
                                    ava_arg[8], ava_arg[9], ava_arg[10])) == NULL) {
        set_nspr_error(NULL);
        Py_CLEAR(self);
        return NULL;
    }

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

/* ========================================================================== */
/* =============================== DN Class ================================= */
/* ========================================================================== */

/*
 * NSS WART
 *
 * CERT_CopyRDN() does not return a new CERTRDN, requires calling CERT_CreateRDN()
 *
 * CERT_CreateName() does not copy it's rdn arguments, must call CERT_CopyRDN()
 *
 * CERT_CreateName() does not allow you to pass in an arena, it creates one
 * and stores it internally. But to call CERT_CreateName() you have to call
 * CERT_CopyRDN() which requires an arena. This means a CERTName object has to 
 * have 2 arenas when one would have sufficed, it also means more bookkeeping
 * to manage the second unnecessary arena.
 * 
 * CERT_CopyAVA() doesn't return SECStatus unlike other copy routines.
 */

/*
 * All these are defined in cert.h and all are exported in nss.def
 *
 * CERT_AsciiToName
 * CERT_NameToAscii
 * CERT_NameToAsciiInvertible
 * CERT_CreateName
 * CERT_CopyName
 * CERT_DestroyName
 * CERT_AddRDN
 * CERT_CompareName
 * CERT_FormatName
 * CERT_GetCertEmailAddress
 * CERT_GetCommonName
 * CERT_GetCountryName
 * CERT_GetLocalityName
 * CERT_GetStateName
 * CERT_GetOrgName
 * CERT_GetOrgUnitName
 * CERT_GetDomainComponentName
 * CERT_GetCertUid
 */

static bool
CERTName_has_tag(CERTName *name, int tag)
{
    CERTRDN **rdns, *rdn;
    CERTAVA **avas, *ava;

    if (!name) return false;
    for (rdns = name->rdns; rdns && (rdn = *rdns); rdns++) {
	for (avas = rdn->avas; avas && (ava = *avas); avas++) {
	    int ava_tag = CERT_GetAVATag(ava);
	    if (tag == ava_tag) {
                return true;
	    }
	}
    }

    return false;
}

static PyObject *
CERTName_get_matching_tag_list(CERTName *name, int tag)
{
    PyObject *list = NULL;
    PyObject *py_rdn = NULL;
    CERTRDN **rdns, *rdn;
    CERTAVA **avas, *ava;

    if ((list = PyList_New(0)) == NULL) {
        return NULL;
    }

    if (!name) {
        return list;
    }

    for (rdns = name->rdns; rdns && (rdn = *rdns); rdns++) {
	for (avas = rdn->avas; avas && (ava = *avas); avas++) {
	    int ava_tag = CERT_GetAVATag(ava);
	    if (tag == ava_tag) {
                if ((py_rdn = RDN_new_from_CERTRDN(rdn)) == NULL) {
                    Py_DECREF(list);
                    return NULL;
                }
                PyList_Append(list, py_rdn);
                break;
	    }
	}
    }

    return list;
}

static PyObject*
DN_subscript(DN *self, PyObject* item)
{
    PyObject* result = NULL;

    if (PyIndex_Check(item)) {
        Py_ssize_t i = PyNumber_AsSsize_t(item, PyExc_IndexError);
	
        if (i == -1 && PyErr_Occurred())
            return NULL;
        if (i < 0)
            i += DN_length(self);
        return DN_item(self, i);
    } else if (PySlice_Check(item)) {
        Py_ssize_t start, stop, step, slicelength, cur, i;
        PyObject* py_ava;

        if (PySlice_GetIndicesEx((PySliceObject*)item, DN_length(self),
				 &start, &stop, &step, &slicelength) < 0) {
            return NULL;
        }

        if (slicelength <= 0) {
            return PyList_New(0);
        } else {
            result = PyList_New(slicelength);
            if (!result) return NULL;
            
            for (cur = start, i = 0; i < slicelength; cur += step, i++) {
                /* We don't need to bump the ref count because RDN_item
                 * returns a new object */
                py_ava = DN_item(self, cur);
                if (PyList_SetItem(result, i, py_ava) == -1) {
                    Py_DECREF(result);
                    return NULL;
                }
            }
            return result;
	}
    } else if (PyString_Check(item) || PyUnicode_Check(item) || PySecItem_Check(item)) {
        int oid_tag;

        if ((oid_tag = get_oid_tag_from_object(item)) == -1) {
            return NULL;
        }

        if (oid_tag == SEC_OID_UNKNOWN) {
            if (PyString_Check(item) || PyUnicode_Check(item)) {
                char *name = PyString_AsString(item);
                PyErr_Format(PyExc_KeyError, "oid name unknown: \"%s\"", name);
                return NULL;
            } else {
                PyErr_SetString(PyExc_KeyError, "oid unknown");
                return NULL;
            }
        }

        if ((result = CERTName_get_matching_tag_list(&self->name, oid_tag)) == NULL) {
            return NULL;
        }

        if (PyList_Size(result) == 0) {
            Py_DECREF(result);
            if (PyString_Check(item) || PyUnicode_Check(item)) {
                char *name = PyString_AsString(item);
                PyErr_Format(PyExc_KeyError, "oid name not found: \"%s\"", name);
                return NULL;
            } else {
                PyErr_SetString(PyExc_KeyError, "oid not found");
                return NULL;
            }
        } else {
            return result;
        }
    } else {
        PyErr_Format(PyExc_TypeError,
                     "indices must be integers or strings, not %.200s",
                     item->ob_type->tp_name);
        return NULL;
    }
    return NULL;
}

PyDoc_STRVAR(DN_has_key_doc,
"has_key(arg) -> bool\n\
\n\
:Parameters:\n\
    arg : string or integer\n\
        canonical name (e.g. 'cn') or oid dotted-decimal or\n\
        SEC_OID_* enumeration constant\n\
\n\
return True if Name has an AVA whose oid can be identified by arg.\n\
");

static PyObject *
DN_has_key(DN *self, PyObject *args)
{
    PyObject *arg;
    int oid_tag;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "O:has_key",
                          &arg))
        return NULL;

    oid_tag = get_oid_tag_from_object(arg);
    if (oid_tag == SEC_OID_UNKNOWN || oid_tag == -1) {
        Py_RETURN_FALSE;
    }

    if (CERTName_has_tag(&self->name, oid_tag)) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

/* =========================== Sequence Protocol ============================ */

static Py_ssize_t
CERTName_rdn_count(CERTName *name)
{
    Py_ssize_t count = 0;
    CERTRDN **rdns;

    for (rdns = name->rdns, count = 0; *rdns; rdns++, count++);

    return count;
}

static Py_ssize_t
DN_length(DN *self)
{
    return CERTName_rdn_count(&self->name);
}

static PyObject *
DN_item(DN *self, register Py_ssize_t i)
{
    Py_ssize_t count = 0;
    CERTRDN **rdns;

    if (i < 0 || self->name.rdns == NULL) {
        PyErr_SetString(PyExc_IndexError, "DN index out of range");
        return NULL;
    }

    for (rdns = self->name.rdns, count = 0; *rdns && count < i; rdns++, count++);

    if (!*rdns) {
        PyErr_SetString(PyExc_IndexError, "DN index out of range");
        return NULL;
    }

    return RDN_new_from_CERTRDN(*rdns);
}

/* ============================ Attribute Access ============================ */

static PyObject *
DN_get_email_address(DN *self, void *closure)
{
    char *value;

    TraceMethodEnter(self);

    if ((value = CERT_GetCertEmailAddress(&self->name)) == NULL) {
        Py_RETURN_NONE;
    }
    return PyString_FromString(value);
}

static PyObject *
DN_get_common_name(DN *self, void *closure)
{
    char *value;

    TraceMethodEnter(self);

    if ((value = CERT_GetCommonName(&self->name)) == NULL) {
        Py_RETURN_NONE;
    }
    return PyString_FromString(value);
}

static PyObject *
DN_get_country_name(DN *self, void *closure)
{
    char *value;

    TraceMethodEnter(self);

    if ((value = CERT_GetCountryName(&self->name)) == NULL) {
        Py_RETURN_NONE;
    }
    return PyString_FromString(value);
}

static PyObject *
DN_get_locality_name(DN *self, void *closure)
{
    char *value;

    TraceMethodEnter(self);

    if ((value = CERT_GetLocalityName(&self->name)) == NULL) {
        Py_RETURN_NONE;
    }
    return PyString_FromString(value);
}

static PyObject *
DN_get_state_name(DN *self, void *closure)
{
    char *value;

    TraceMethodEnter(self);

    if ((value = CERT_GetStateName(&self->name)) == NULL) {
        Py_RETURN_NONE;
    }
    return PyString_FromString(value);
}

static PyObject *
DN_get_org_name(DN *self, void *closure)
{
    char *value;

    TraceMethodEnter(self);

    if ((value = CERT_GetOrgName(&self->name)) == NULL) {
        Py_RETURN_NONE;
    }
    return PyString_FromString(value);
}

static PyObject *
DN_get_org_unit_name(DN *self, void *closure)
{
    char *value;

    TraceMethodEnter(self);

    if ((value = CERT_GetOrgUnitName(&self->name)) == NULL) {
        Py_RETURN_NONE;
    }
    return PyString_FromString(value);
}

static PyObject *
DN_get_domain_component_name(DN *self, void *closure)
{
    char *value;

    TraceMethodEnter(self);

    if ((value = CERT_GetDomainComponentName(&self->name)) == NULL) {
        Py_RETURN_NONE;
    }
    return PyString_FromString(value);
}

static PyObject *
DN_get_cert_uid(DN *self, void *closure)
{
    char *value;

    TraceMethodEnter(self);

    if ((value = CERT_GetCertUid(&self->name)) == NULL) {
        Py_RETURN_NONE;
    }
    return PyString_FromString(value);
}

static
PyGetSetDef DN_getseters[] = {
    {"email_address", (getter)DN_get_email_address, (setter)NULL,
     "Returns the email address member as a string. Returns None if not found.", NULL},
    {"common_name", (getter)DN_get_common_name, (setter)NULL,
     "Returns the common name member (i.e. CN) as a string. Returns None if not found.", NULL},
    {"country_name", (getter)DN_get_country_name, (setter)NULL,
     "Returns the country name member (i.e. C) as a string. Returns None if not found.", NULL},
    {"locality_name", (getter)DN_get_locality_name, (setter)NULL,
     "Returns the locality name member (i.e. L) as a string. Returns None if not found.", NULL},
    {"state_name", (getter)DN_get_state_name, (setter)NULL,
     "Returns the state name member (i.e. ST) as a string. Returns None if not found.", NULL},
    {"org_name", (getter)DN_get_org_name, (setter)NULL,
     "Returns the organization name member (i.e. O) as a string. Returns None if not found.", NULL},
    {"org_unit_name", (getter)DN_get_org_unit_name, (setter)NULL,
     "Returns the organizational unit name member (i.e. OU) as a string. Returns None if not found.", NULL},
    {"dc_name", (getter)DN_get_domain_component_name, (setter)NULL,
     "Returns the domain component name member (i.e. DC) as a string. Returns None if not found.", NULL},
    {"cert_uid", (getter)DN_get_cert_uid, (setter)NULL,
     "Returns the certificate uid member (i.e. UID) as a string. Returns None if not found.", NULL},
    {NULL}  /* Sentinel */
};

static PyMemberDef DN_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

static int
DN_compare(DN *self, DN *other)
{
    if (!PyDN_Check(other)) {
        PyErr_SetString(PyExc_TypeError, "Bad type, must be DN");
        return -1;
    }

    return CERT_CompareName(&self->name, &other->name);
}

PyDoc_STRVAR(DN_add_rdn_doc,
"add_rdn(rdn) \n\
\n\
:Parameters:\n\
    rdn : RDN object\n\
        The rnd to add to the name\n\
\n\
Adds a RDN to the name.\n\
");

static PyObject *
DN_add_rdn(DN *self, PyObject *args)
{
    RDN *py_rdn;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "O!:add_rdn",
                          &RDNType, &py_rdn))
        return NULL;

    if (CERT_AddRDN(&self->name, py_rdn->rdn) != SECSuccess) {
        return set_nspr_error(NULL);
    }

    Py_RETURN_NONE;
}

static PyMethodDef DN_methods[] = {
    {"has_key", (PyCFunction)DN_has_key, METH_VARARGS, DN_has_key_doc},
    {"add_rdn", (PyCFunction)DN_add_rdn, METH_VARARGS, DN_add_rdn_doc},
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
DN_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    DN *self;

    TraceObjNewEnter(type);

    if ((self = (DN *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }

    if ((self->arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE)) == NULL) {
        type->tp_free(self);
        return set_nspr_error(NULL);
    }

    memset(&self->name, 0, sizeof(self->name));

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static void
DN_dealloc(DN* self)
{
    TraceMethodEnter(self);

    CERT_DestroyName(&self->name);

    if (self->arena) {
        PORT_FreeArena(self->arena, PR_FALSE);
    }

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(DN_doc,
"An object representing an X501 Distinguished Name (e.g DN).\n\
\n\
DN objects contain an ordered list of `RDN` objects.\n\
\n\
The DN object constructor may be invoked with a string\n\
representing an X500 name. Zero or more `RDN` objects, or you may\n\
optionally pass a list or tuple of `RDN` objects.\n\
\n\
Examples::\n\
\n\
    DN()\n\
    DN('CN=www.redhat.com,OU=Web Operations,O=Red Hat Inc,L=Raleigh,ST=North Carolina,C=US')\n\
    DN(rdn0, ...)\n\
    DN([rdn0, rdn1])\n\
\n\
**The string representation of a Distinguished Name (DN) has reverse\n\
ordering from it's sequential components.**\n\
\n\
The ordering is a requirement of the relevant RFC's. When a\n\
Distinguished Name is rendered as a string it is ordered from most\n\
specific to least specific. However it's components (RDN's) as a\n\
sequence are ordered from least specific to most specific.\n\
\n\
DN objects contain an ordered list of `RDN` objects. The\n\
DN object has both sequence and mapping behaviors with respect to\n\
the RDN's they contain. Thus you can index an RDN by position, by\n\
name, or by SecItem (if it's an OID). You can iterate over the list,\n\
get it's length or take a slice.\n\
\n\
If you index by string the string may be either a canonical name for\n\
the RDN type (e.g. 'cn') or the dotted-decimal notation for the OID\n\
(e.g. 2.5.4.3). There may be multiple RDN's in a DN whose type matches\n\
(e.g. OU=engineering, OU=boston). It is not common to have more than\n\
one RDN in a DN with the same type. However because of the possiblity\n\
of being multi-valued when indexing by type a list is always returned\n\
containing the matching RDN's. Thus::\n\
\n\
    dn = nss.DN('OU=engineering')\n\
    dn['ou']\n\
        returns [RDN('OU=engineering')\n\
\n\
    dn = nss.DN('OU=engineering, OU=boston')\n\
    dn['ou']\n\
        returns [RDN('OU=boston'), RDN('OU=engineering')]\n\
        Note the reverse ordering between string representation and RDN sequencing\n\
\n\
Note, if you use properties to access the RDN values (e.g. name.common_name,\n\
name.org_unit_name) the string value is returned or None if not found.\n\
If the item was multi-valued then the most appropriate item will be selected\n\
and returned as a string value.\n\
\n\
Note it is not possible to index by oid tag\n\
(e.g. nss.SEC_OID_AVA_COMMON_NAME) because oid tags are integers and\n\
it's impossible to distinguish between an integer representing the\n\
n'th member of the sequence and the integer representing the oid\n\
tag. In this case positional indexing wins (e.g. rdn[0] means the\n\
first element).\n\
\n\
Examples::\n\
\n\
    subject_name = 'CN=www.redhat.com,OU=Web Operations,O=Red Hat Inc,L=Raleigh,ST=North Carolina,C=US'\n\
    name = nss.DN(subject_name)\n\
    str(name)\n\
       returns 'CN=www.redhat.com,OU=Web Operations,O=Red Hat Inc,L=Raleigh,ST=North Carolina,C=US'\n\
    name[0]\n\
       returns an `RDN` object with the value C=US\n\
    name['cn']\n\
        returns a list comprised of an `RDN` object with the value CN=www.redhat.com\n\
    name['2.5.4.3']\n\
        returns a list comprised of an `RDN` object with the value CN=www.redhat.com\n\
        because 2.5.4.3 is the dotted-decimal OID for common name (i.e. cn)\n\
    name.common_name\n\
        returns the string www.redhat.com\n\
        common_name is easy shorthand property, it only retuns a single string\n\
        value or None, if it was multi-valued the most appropriate item is selected.\n\
    name.has_key('cn')\n\
        returns True because the DN has a common name RDN\n\
    name.has_key('2.5.4.3')\n\
        returns True because the DN has a common name RDN\n\
        because 2.5.4.3 is the dotted-decimal OID for common name (i.e. cn)\n\
\n\
    cn_rdn = nss.RDN(nss.AVA('cn', 'www.redhat.com'))\n\
    ou_rdn = nss.RDN(nss.AVA('ou', 'Web Operations'))\n\
    name = nss.DN(cn_rdn)\n\
    name\n\
       is a DN with one RDN (e.g. CN=www.redhat.com)\n\
    len(name)\n\
       returns 1 because there is one RDN in it\n\
    name.add_rdn(ou_rdn)\n\
    name\n\
       name is now a DN with two RDN's (e.g. OU=Web Operations,CN=www.redhat.com)\n\
    len(name)\n\
       returns 2 because there are now two RDN's in it\n\
    list(name)\n\
       returns a list with the two RDN's in it\n\
    name[:]\n\
       same as list(name)\n\
    for rdn in name:\n\
       iterate over each RDN in name\n\
    name = nss.DN(cn_rdn, ou_rdn)\n\
        This is an alternate way to build the above DN\n\
");

static int
DN_init(DN *self, PyObject *args, PyObject *kwds)
{
    PyObject *sequence, *item;
    Py_ssize_t sequence_len, i;
    RDN *py_rdn;
    CERTRDN *new_rdn;
    CERTName *cert_name;
    CERTRDN *rdn_arg[MAX_RDNS+1];  /* +1 for NULL terminator */

    TraceMethodEnter(self);

    CERT_DestroyName(&self->name);

    if (PyTuple_GET_SIZE(args) > 0) {
        item = PyTuple_GetItem(args, 0);
        if (PyString_Check(item) || PyUnicode_Check(item)) {
            char *ascii_name;

            if ((ascii_name = PyString_AsString(item)) == NULL) {
                return -1;
            }

            if (strlen(ascii_name) == 0) goto empty_name;

            if ((cert_name = CERT_AsciiToName(ascii_name)) == NULL) {
                set_nspr_error("cannot parse X500 name \"%s\"", ascii_name);
                return -1;
            }

            self->name = *cert_name;
            return 0;
        }

        if (PyRDN_Check(item)) {
            sequence = args;
        } else if (PyList_Check(item) || PyTuple_Check(item)) {
            sequence = item;
        } else {
            PyErr_Format(PyExc_TypeError, "must be an RDN object or list or tuple of RDN objects, not %.200s",
                         Py_TYPE(item)->tp_name);
            return -1;
        }

        sequence_len = PySequence_Length(sequence);

        if (sequence_len > MAX_RDNS) {
            PyErr_Format(PyExc_ValueError, "to many RDN items, maximum is %d, received %d",
                         MAX_RDNS-1, sequence_len);
            return -1;
        }

        for (i = 0; i < sequence_len && i < MAX_RDNS; i++) {
            item = PySequence_ITEM(sequence, i);
            if (PyRDN_Check(item)) {
                py_rdn = (RDN *)item;

                if ((new_rdn = CERT_CreateRDN(self->arena, NULL)) == NULL) {
                    set_nspr_error(NULL);
                    Py_DECREF(item);
                    return -1;
                }

                if (CERT_CopyRDN(self->arena, new_rdn, py_rdn->rdn) != SECSuccess) {
                    set_nspr_error(NULL);
                    Py_DECREF(item);
                    return -1;
                }
                rdn_arg[i] = new_rdn;
            } else {
                PyErr_Format(PyExc_TypeError, "item %d must be an RDN object, not %.200s",
                             i, Py_TYPE(item)->tp_name);
                Py_DECREF(item);
                return -1;
            }
            Py_DECREF(item);
        }

        for (; i < MAX_RDNS+1; i++) rdn_arg[i] = NULL;

        if ((cert_name = CERT_CreateName(rdn_arg[0], rdn_arg[1], rdn_arg[2], rdn_arg[3],
                                         rdn_arg[4], rdn_arg[5], rdn_arg[6], rdn_arg[7],
                                         rdn_arg[8], rdn_arg[9], rdn_arg[10])) == NULL) {
            set_nspr_error(NULL);
            return -1;
        }
        self->name = *cert_name;
    } else {
    empty_name:
        if ((cert_name = CERT_CreateName(NULL)) == NULL) {
            set_nspr_error(NULL);
            return -1;
        }
        self->name = *cert_name;
    }
    return 0;
}

static PyObject *
DN_repr(DN *self)
{
    return CERTName_to_pystr(&self->name);    
}

static PySequenceMethods DN_as_sequence = {
    (lenfunc)DN_length,				/* sq_length */
    0,						/* sq_concat */
    0,						/* sq_repeat */
    (ssizeargfunc)DN_item,			/* sq_item */
    0,						/* sq_slice */
    0,						/* sq_ass_item */
    0,						/* sq_ass_slice */
    0,						/* sq_contains */
    0,						/* sq_inplace_concat */
    0,						/* sq_inplace_repeat */
};

static PyMappingMethods DN_as_mapping = {
    (lenfunc)DN_length,				/* mp_length */
    (binaryfunc)DN_subscript,			/* mp_subscript */
    0,						/* mp_ass_subscript */
};

static PyTypeObject DNType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.DN",				/* tp_name */
    sizeof(DN),					/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)DN_dealloc,			/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    (cmpfunc)DN_compare,			/* tp_compare */
    (reprfunc)DN_repr,				/* tp_repr */
    0,						/* tp_as_number */
    &DN_as_sequence,				/* tp_as_sequence */
    &DN_as_mapping,				/* tp_as_mapping */
    0,						/* tp_hash */
    0,						/* tp_call */
    (reprfunc)DN_repr,				/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
    DN_doc,					/* tp_doc */
    0,						/* tp_traverse */
    0,						/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    DN_methods,					/* tp_methods */
    DN_members,					/* tp_members */
    DN_getseters,				/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)DN_init,				/* tp_init */
    0,						/* tp_alloc */
    DN_new,					/* tp_new */
};

PyObject *
DN_new_from_CERTName(CERTName *name)
{
    DN *self = NULL;
    PRArenaPool *arena;

    TraceObjNewEnter(NULL);

    if ((self = (DN *) DNType.tp_new(&DNType, NULL, NULL)) == NULL) {
        return NULL;
    }

    if ((arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE)) == NULL) {
        set_nspr_error(NULL);
        Py_CLEAR(self);
        return NULL;
    }

    if (CERT_CopyName(arena, &self->name, name) != SECSuccess) {
        set_nspr_error(NULL);
        Py_CLEAR(self);
        return NULL;
    }

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

/* ========================================================================== */
/* =========================== GeneralName Class ============================ */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static PyObject *
GeneralName_get_type_enum(GeneralName *self, void *closure)
{
    TraceMethodEnter(self);

    if (!self->name) {
        return PyErr_Format(PyExc_ValueError, "%s is uninitialized", Py_TYPE(self)->tp_name);
    }
    return PyInt_FromLong(self->name->type);
}

static PyObject *
GeneralName_get_type_name(GeneralName *self, void *closure)
{
    TraceMethodEnter(self);

    if (!self->name) {
        return PyErr_Format(PyExc_ValueError, "%s is uninitialized", Py_TYPE(self)->tp_name);
    }
    return general_name_type_to_pystr(self->name->type);
}

static PyObject *
GeneralName_get_type_string(GeneralName *self, void *closure)
{
    TraceMethodEnter(self);

    if (!self->name) {
        return PyErr_Format(PyExc_ValueError, "%s is uninitialized", Py_TYPE(self)->tp_name);
    }
    return CERTGeneralName_type_string_to_pystr(self->name);
}

static
PyGetSetDef GeneralName_getseters[] = {
    {"type_enum", (getter)GeneralName_get_type_enum, (setter)NULL,
     "Returns the general name type enumerated constant", NULL},
    {"type_name", (getter)GeneralName_get_type_name, (setter)NULL,
     "Returns the general name type enumerated constant as a string", NULL},
    {"type_string", (getter)GeneralName_get_type_string, (setter)NULL,
     "Returns the type of the general name as a string (e.g. \"URI\")", NULL},
    {NULL}  /* Sentinel */
};

static PyMemberDef GeneralName_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

PyDoc_STRVAR(GeneralName_get_name_doc,
"get_name(repr_kind=AsString) -> \n\
\n\
:Parameters:\n\
    repr_kind : RepresentationKind constant\n\
        Specifies what the contents of the returned tuple will be.\n\
        May be one of:\n\
\n\
        AsObject\n\
            The general name as a nss.GeneralName object\n\
        AsString\n\
            The general name as a string.\n\
            (e.g. \"http://crl.geotrust.com/crls/secureca.crl\")\n\
        AsTypeString\n\
            The general name type as a string.\n\
             (e.g. \"URI\")\n\
        AsTypeEnum\n\
            The general name type as a general name type enumerated constant.\n\
             (e.g. nss.certURI )\n\
        AsLabeledString\n\
            The general name as a string with it's type prepended.\n\
            (e.g. \"URI: http://crl.geotrust.com/crls/secureca.crl\"\n\
\n\
Returns the value of the GeneralName according to the representation type parameter.\n\
");

static PyObject *
GeneralName_get_name(GeneralName *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"arg1", NULL};
    PyObject *name;
    int repr_kind = AsString;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i:get_name", kwlist,
                                     &repr_kind))
        return NULL;


    if (!self->name) {
        return PyErr_Format(PyExc_ValueError, "%s is uninitialized", Py_TYPE(self)->tp_name);
    }

    switch(repr_kind) {
    case AsObject:
        Py_INCREF(self);
        name = (PyObject *)self;
        break;
    case AsString:
        name = CERTGeneralName_to_pystr(self->name);
        break;
    case AsTypeString:
        name = CERTGeneralName_type_string_to_pystr(self->name);
        break;
    case AsTypeEnum:
        name = PyInt_FromLong(self->name->type);
        break;
    case AsLabeledString:
        name = CERTGeneralName_to_pystr_with_label(self->name);
        break;
    default:
        PyErr_Format(PyExc_ValueError, "Unsupported representation kind (%d)", repr_kind);
        return NULL;
    }

    return name;
}

static PyMethodDef GeneralName_methods[] = {
    {"get_name", (PyCFunction)GeneralName_get_name, METH_VARARGS|METH_KEYWORDS, GeneralName_get_name_doc},
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
GeneralName_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    GeneralName *self;

    TraceObjNewEnter(type);

    if ((self = (GeneralName *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }

    if ((self->arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE)) == NULL) {
        type->tp_free(self);
        return set_nspr_error(NULL);
    }

    self->name = NULL;

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static void
GeneralName_dealloc(GeneralName* self)
{
    TraceMethodEnter(self);

    if (self->arena) {
        PORT_FreeArena(self->arena, PR_FALSE);
    }

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(GeneralName_doc,
"An object representing a GeneralName or list of GeneralNames.\n\
\n\
");

static int
GeneralName_init(GeneralName *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"sec_item", NULL};
    SecItem *py_sec_item;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i:GeneralName", kwlist,
                                     &SecItemType, &py_sec_item))
        return -1;

    if ((self->name = CERT_DecodeGeneralName(self->arena, &py_sec_item->item, NULL)) == NULL) {
        set_nspr_error(NULL);
        return -1;
    }

    return 0;
}

static PyObject *
GeneralName_repr(GeneralName *self)
{
    PyObject *result = NULL;

    if (!self->name) {
        return PyErr_Format(PyExc_ValueError, "%s is uninitialized", Py_TYPE(self)->tp_name);
    }

    if ((result = CERTGeneralName_to_pystr_with_label(self->name)) == NULL) {
        result = PyString_FromFormat("<%s object at %p>",
                                     Py_TYPE(self)->tp_name, self);
    }

    return result;
}

static Py_ssize_t
GeneralName_length(GeneralName *self)
{
    if (!self->name) {
        PyErr_Format(PyExc_ValueError, "%s is uninitialized", Py_TYPE(self)->tp_name);
        return -1;
    }

    return CERTGeneralName_list_count(self->name);
}

static PyObject *
GeneralName_item(GeneralName *self, register Py_ssize_t i)
{
    CERTGeneralName *head, *cur;
    Py_ssize_t index;

    if (!self->name) {
        return PyErr_Format(PyExc_ValueError, "%s is uninitialized", Py_TYPE(self)->tp_name);
    }

    index = 0;
    cur = head = self->name;
    do {
        cur = CERT_GetNextGeneralName(cur);
        if (i == index) {
            return GeneralName_new_from_CERTGeneralName(cur);
        }
        index++;
    } while (cur != head);

    PyErr_SetString(PyExc_IndexError, "GeneralName index out of range");
    return NULL;
}

static PySequenceMethods GeneralName_as_sequence = {
    (lenfunc)GeneralName_length,		/* sq_length */
    0,						/* sq_concat */
    0,						/* sq_repeat */
    (ssizeargfunc)GeneralName_item,		/* sq_item */
    0,						/* sq_slice */
    0,						/* sq_ass_item */
    0,						/* sq_ass_slice */
    0,						/* sq_contains */
    0,						/* sq_inplace_concat */
    0,						/* sq_inplace_repeat */
};

static PyTypeObject GeneralNameType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.GeneralName",			/* tp_name */
    sizeof(GeneralName),			/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)GeneralName_dealloc,		/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    0,						/* tp_compare */
    (reprfunc)GeneralName_repr,			/* tp_repr */
    0,						/* tp_as_number */
    &GeneralName_as_sequence,			/* tp_as_sequence */
    0,						/* tp_as_mapping */
    0,						/* tp_hash */
    0,						/* tp_call */
    0,						/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
    GeneralName_doc,				/* tp_doc */
    0,						/* tp_traverse */
    0,						/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    GeneralName_methods,			/* tp_methods */
    GeneralName_members,			/* tp_members */
    GeneralName_getseters,			/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)GeneralName_init,			/* tp_init */
    0,						/* tp_alloc */
    0,						/* tp_new */
};

PyObject *
GeneralName_new_from_CERTGeneralName(CERTGeneralName *name)
{
    GeneralName *self = NULL;

    TraceObjNewEnter(NULL);

    if ((self = (GeneralName *) GeneralName_new(&GeneralNameType, NULL, NULL)) == NULL) {
        return NULL;
    }

    /*
     * NSS WART
     * There is no public API to create a CERTGeneralName, copy it, or free it.
     * You don't know what arena was used to create the general name.
     * GeneralNames are linked in a list, this makes it difficult for a 
     * general name to exist independently, it would have been better if there
     * was a list container independent general names could be placed in,
     * then you wouldn't have to worry about the link fields in each independent name.
     */

    if (CERT_CopyGeneralName(self->arena, &self->name, name) != SECSuccess) {
        set_nspr_error(NULL);
        Py_CLEAR(self);
        return NULL;
    }

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

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

    if ((cert_handle = CERT_GetDefaultCertDB()) == NULL) {
        Py_RETURN_NONE;
    }

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
        Py_INCREF(args);
        parse_args = args;
    } else {
        parse_args = PyTuple_GetSlice(args, 0, n_base_args);
    }
    if (!PyArg_ParseTuple(parse_args, "O!i:get_cert_nicknames",
                          &CertDBType, &py_certdb, &what)) {
        Py_DECREF(parse_args);
        return NULL;
    }
    Py_DECREF(parse_args);

    pin_args = PyTuple_GetSlice(args, n_base_args, argc);

    Py_BEGIN_ALLOW_THREADS
    if ((cert_nicknames = CERT_GetCertNicknames(py_certdb->handle, what, pin_args)) == NULL) {
	Py_BLOCK_THREADS
        Py_DECREF(pin_args);
        return set_nspr_error(NULL);
    }
    Py_END_ALLOW_THREADS

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
                              oid_tag_str(hash_alg));
    }

    out_buf_len = hash_len;

    if ((py_out_buf = PyString_FromStringAndSize(NULL, out_buf_len)) == NULL) {
        return NULL;
    }

    if ((out_buf = PyString_AsString(py_out_buf)) == NULL) {
        return NULL;
    }

    if (PK11_HashBuf(hash_alg, out_buf, in_data, in_data_len) != SECSuccess) {
        return set_nspr_error(NULL);
    }

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

    TraceMethodEnter(self);

    if ((slot_name = PK11_GetSlotName(self->slot)) == NULL) {
        Py_RETURN_NONE;
    }

    return PyString_FromString(slot_name);
}

static PyObject *
PK11_get_token_name(PK11Slot *self, void *closure)
{
    char *token_name = NULL;

    TraceMethodEnter(self);

    if ((token_name = PK11_GetTokenName(self->slot)) == NULL) {
        Py_RETURN_NONE;
    }

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
        Py_INCREF(args);
        parse_args = args;
    } else {
        parse_args = PyTuple_GetSlice(args, 0, n_base_args);
    }
    if (!PyArg_ParseTuple(parse_args, "kO&i:key_gen",
                          &mechanism, SecItemOrNoneConvert, &py_sec_param,
                          &key_size)) {
        Py_DECREF(parse_args);
        return NULL;
    }
    Py_DECREF(parse_args);

    pin_args = PyTuple_GetSlice(args, n_base_args, argc);

    Py_BEGIN_ALLOW_THREADS
    if ((sym_key = PK11_KeyGen(self->slot, mechanism, py_sec_param ? &py_sec_param->item : NULL,
                               key_size, pin_args)) == NULL) {
	Py_BLOCK_THREADS
        Py_DECREF(pin_args);
        return set_nspr_error(NULL);
    }
    Py_END_ALLOW_THREADS

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

    if ((self = (PK11Slot *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }
    self->slot = NULL;

    TraceObjNewLeave(self);
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

    if ((self = (PK11Slot *) PK11SlotType.tp_new(&PK11SlotType, NULL, NULL)) == NULL) {
        return NULL;
    }

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
    TraceMethodEnter(self);

    return PyInt_FromLong(PK11_GetMechanism(self->pk11_sym_key));
}

static PyObject *
PK11SymKey_get_key_data(PyPK11SymKey *self, void *closure)
{
    SECItem *sec_item;

    TraceMethodEnter(self);

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
    TraceMethodEnter(self);

    return PyInt_FromLong(PK11_GetKeyLength(self->pk11_sym_key));
}

static PyObject *
PK11SymKey_get_slot(PyPK11SymKey *self, void *closure)
{
    PK11SlotInfo *slot = NULL;
    PyObject *py_slot = NULL;

    TraceMethodEnter(self);

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

    if (self->pk11_sym_key) {
        PK11_FreeSymKey(self->pk11_sym_key);
    }

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

    if ((self = PyObject_NEW(PyPK11SymKey, &PK11SymKeyType)) == NULL) {
        return NULL;
    }

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
    PyObject *py_out_string;
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

    if ((py_out_string = PyString_FromStringAndSize(NULL, out_buf_alloc_len)) == NULL) {
        return NULL;
    }
    out_buf = PyString_AsString(py_out_string);

    /*
     * Now that we have both the input and output buffers perform the cipher operation.
     */
    if (PK11_CipherOp(self->pk11_context, out_buf, &actual_out_len, out_buf_alloc_len,
                      (unsigned char *)in_buf, in_buf_len) != SECSuccess) {
        Py_DECREF(py_out_string);
        return set_nspr_error(NULL);
    }

    if (actual_out_len != out_buf_alloc_len) {
        if (_PyString_Resize(&py_out_string, actual_out_len) < 0) {
        return NULL;
        }
    }

    return py_out_string;
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
    PyObject *py_out_string;

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

    if ((py_out_string = PyString_FromStringAndSize(NULL, out_buf_alloc_len)) == NULL) {
        return NULL;
    }
    out_buf = PyString_AsString(py_out_string);

    /*
     * Now that we have the output buffer perform the cipher operation.
     */
    if (PK11_DigestFinal(self->pk11_context, out_buf,
                         &actual_out_len, out_buf_alloc_len) != SECSuccess) {
        Py_DECREF(py_out_string);
        return set_nspr_error(NULL);
    }

    if (actual_out_len != out_buf_alloc_len) {
        if (_PyString_Resize(&py_out_string, actual_out_len) < 0) {
        return NULL;
        }
    }

    return py_out_string;
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

    if ((self = (PyPK11Context *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }

    self->pk11_context = NULL;

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static void
PK11Context_dealloc(PyPK11Context* self)
{
    TraceMethodEnter(self);

    if (self->pk11_context) {
        PK11_DestroyContext(self->pk11_context, PR_TRUE);
    }

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

    if ((self = (PyPK11Context *) PK11ContextType.tp_new(&PK11ContextType, NULL, NULL)) == NULL) {
        return NULL;
    }

    self->pk11_context = pk11_context;

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

/* ========================================================================== */
/* ======================== CRLDistributionPt Class ========================= */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static PyObject *
CRLDistributionPt_get_crl_issuer(CRLDistributionPt *self, void *closure)
{
    TraceMethodEnter(self);

    if (!self->pt || !self->pt->crlIssuer) {
        Py_RETURN_NONE;
    }
    return GeneralName_new_from_CERTGeneralName(self->pt->crlIssuer);
}

static
PyGetSetDef CRLDistributionPt_getseters[] = {
    {"issuer", (getter)CRLDistributionPt_get_crl_issuer, (setter)NULL,
     "returns the CRL Issuer as a GeneralName object if defined, returns None if not defined", NULL},
    {NULL}  /* Sentinel */
};

static PyMemberDef CRLDistributionPt_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

PyDoc_STRVAR(CRLDistributionPt_get_general_names_doc,
"get_general_names(repr_kind=AsString) -> (general_name, ...)\n\
\n\
:Parameters:\n\
    repr_kind : RepresentationKind constant\n\
        Specifies what the contents of the returned tuple will be.\n\
        May be one of:\n\
\n\
        AsObject\n\
            The general name as a nss.GeneralName object\n\
        AsString\n\
            The general name as a string.\n\
            (e.g. \"http://crl.geotrust.com/crls/secureca.crl\")\n\
        AsTypeString\n\
            The general name type as a string.\n\
             (e.g. \"URI\")\n\
        AsTypeEnum\n\
            The general name type as a general name type enumerated constant.\n\
             (e.g. nss.certURI )\n\
        AsLabeledString\n\
            The general name as a string with it's type prepended.\n\
            (e.g. \"URI: http://crl.geotrust.com/crls/secureca.crl\"\n\
\n\
Returns a tuple of general names in the CRL Distribution Point. If the\n\
distribution point type is not nss.generalName or the list was empty then\n\
the returned tuple will be empty.\n\
\n\
You may specify how the each member of the tuple is represented, by default\n\
it will be as a string.\n\
");

static PyObject *
CRLDistributionPt_get_general_names(CRLDistributionPt *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"repr_kind", NULL};
    int repr_kind = AsString;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:get_general_names", kwlist,
                                     &repr_kind))
        return NULL;

    return CRLDistributionPt_general_names_tuple(self, repr_kind);
}

PyDoc_STRVAR(CRLDistributionPt_get_reasons_doc,
"get_reasons(repr_kind=AsEnumDescription) -> (reason, ...)\n\
\n\
:Parameters:\n\
    repr_kind : RepresentationKind constant\n\
        Specifies what the contents of the returned tuple will be.\n\
        May be one of:\n\
\n\
        AsEnum\n\
            The enumerated constant.\n\
            (e.g. nss.crlEntryReasonCaCompromise)\n\
        AsEnumDescription\n\
            A friendly human readable description of the enumerated constant as a string.\n\
             (e.g. \"CA Compromise\")\n\
        AsIndex\n\
            The bit position within the bit string.\n\
\n\
Returns a tuple of reasons in the CRL Distribution Point. If no\n\
reasons were defined the returned tuple will be empty.\n\
\n\
You may specify how the each member of the tuple is represented, by default\n\
it will be as a string.\n\
");

static PyObject *
CRLDistributionPt_get_reasons(CRLDistributionPt *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"repr_kind", NULL};
    int repr_kind = AsEnumDescription;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:get_reasons", kwlist,
                                     &repr_kind))
        return NULL;

    return crl_reason_bitstr_to_tuple(&self->pt->bitsmap, repr_kind);
}

PyObject *
CRLDistributionPt_format_lines(CRLDistributionPt *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"level", NULL};
    int level = 0;
    Py_ssize_t len;
    PyObject *lines = NULL;
    PyObject *obj = NULL;
    PyObject *obj1 = NULL;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:format_lines", kwlist, &level))
        return NULL;

    if ((lines = PyList_New(0)) == NULL) {
        return NULL;
    }

    if (!self->pt) {
        return lines;
    }

    if (self->pt->distPointType == generalName) {
        if ((obj = CRLDistributionPt_general_names_tuple(self, AsString)) == NULL) {
            goto fail;
        }
        len = PyTuple_GET_SIZE(obj);

        if ((obj1 = PyString_FromFormat("General Names: [%d total]", len)) == NULL) {
            goto fail;
        }
        FMT_OBJ_AND_APPEND(lines, NULL, obj1, level, fail);
        Py_CLEAR(obj1);

        APPEND_LINES_AND_CLEAR(lines, obj, level+1, fail);

    } else if (self->pt->distPointType == relativeDistinguishedName) {

        if ((obj = RDN_new_from_CERTRDN(&self->pt->distPoint.relativeName)) == NULL) {
            goto fail;
        }

        FMT_OBJ_AND_APPEND(lines, _("Relative Distinguished Name"), obj, level, fail);
        Py_CLEAR(obj);
    } else {
        PyErr_Format(PyExc_ValueError, "unknown distribution point type (%d), "
                     "expected generalName or relativeDistinguishedName",
                     self->pt->distPointType);
        goto fail;
    }

    if ((obj = CRLDistributionPt_get_crl_issuer(self, NULL)) == NULL) {
        goto fail;
    }

    FMT_OBJ_AND_APPEND(lines, _("Issuer"), obj, level, fail);
    Py_CLEAR(obj);

    if ((obj = crl_reason_bitstr_to_tuple(&self->pt->bitsmap, AsEnumDescription)) == NULL) {
        goto fail;
    }

    FMT_OBJ_AND_APPEND(lines, _("Reasons"), obj, level, fail);
    Py_CLEAR(obj);

    return lines;

 fail:
    Py_XDECREF(lines);
    Py_XDECREF(obj);
    Py_XDECREF(obj1);
    return NULL;
}

static PyObject *
CRLDistributionPt_format(CRLDistributionPt *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter(self);

    return format_from_lines((format_lines_func)CRLDistributionPt_format_lines, (PyObject *)self, args, kwds);
}

static PyObject *
CRLDistributionPt_str(CRLDistributionPt *self)
{
    PyObject *py_formatted_result = NULL;

    TraceMethodEnter(self);

    py_formatted_result =  CRLDistributionPt_format(self, empty_tuple, NULL);
    return py_formatted_result;

}

static PyObject *
CRLDistributionPt_repr(CRLDistributionPt *self)
{
    PyObject *result = NULL;
    PyObject *rdn = NULL;
    PyObject *names = NULL;
    PyObject *name_str = NULL;
    PyObject *name_desc = NULL;
    PyObject *crl_issuer = NULL;
    PyObject *crl_issuer_str = NULL;
    PyObject *reasons = NULL;
    PyObject *reasons_str = NULL;
    PyObject *sep = NULL;

    if (!self->pt) {
        return PyString_FromFormat("<%s object at %p>",
                                   Py_TYPE(self)->tp_name, self);
    }

    if ((sep = PyString_FromString(", ")) == NULL) {
        goto exit;
    }

    if (self->pt->distPointType == generalName) {
        Py_ssize_t n_names = 0;

        if ((names = CRLDistributionPt_general_names_tuple(self, AsString)) == NULL) {
            goto exit;
        }
        n_names = PyTuple_GET_SIZE(names);

        /* Paste them all together with ", " between. */
        if ((name_str = _PyString_Join(sep, names)) == NULL) {
            goto exit;
        }

        name_desc = PyString_FromFormat(_("General Name List: [%s]"),
                                        PyString_AsString(name_str));

    } else if (self->pt->distPointType == relativeDistinguishedName) {

        if ((rdn = RDN_new_from_CERTRDN(&self->pt->distPoint.relativeName)) == NULL) {
            goto exit;
        }

        if ((name_str = PyObject_Str(rdn)) == NULL) {
            goto exit;
        }

        name_desc = PyString_FromFormat(_("Relative Distinguished Name: %s"),
                                        PyString_AsString(name_str));
        
    } else {
        PyErr_Format(PyExc_ValueError, "unknown distribution point type (%d), "
                     "expected generalName or relativeDistinguishedName",
                     self->pt->distPointType);
        goto exit;
    }

    if ((crl_issuer = CRLDistributionPt_get_crl_issuer(self, NULL)) == NULL) {
        goto exit;
    }

    if ((crl_issuer_str = PyObject_Str(crl_issuer)) == NULL) {
        goto exit;
    }
        
    if ((reasons = crl_reason_bitstr_to_tuple(&self->pt->bitsmap, AsEnumDescription)) == NULL) {
        goto exit;
    }

    if ((reasons_str = _PyString_Join(sep, reasons)) == NULL) {
        goto exit;
    }

    result = PyString_FromFormat("%s, Issuer: %s, Reasons: [%s]",
                                 PyString_AsString(name_desc),
                                 PyString_AsString(crl_issuer_str),
                                 PyString_AsString(reasons_str));

 exit:
    Py_XDECREF(rdn);
    Py_XDECREF(names);
    Py_XDECREF(name_str);
    Py_XDECREF(name_desc);
    Py_XDECREF(crl_issuer);
    Py_XDECREF(crl_issuer_str);
    Py_XDECREF(reasons);
    Py_XDECREF(reasons_str);
    Py_XDECREF(sep);

    return result;
}

static PyMethodDef CRLDistributionPt_methods[] = {
    {"format_lines",      (PyCFunction)CRLDistributionPt_format_lines,      METH_VARARGS|METH_KEYWORDS, generic_format_lines_doc},
    {"format",            (PyCFunction)CRLDistributionPt_format,            METH_VARARGS|METH_KEYWORDS, generic_format_doc},
    {"get_general_names", (PyCFunction)CRLDistributionPt_get_general_names, METH_VARARGS|METH_KEYWORDS, CRLDistributionPt_get_general_names_doc},
    {"get_reasons",       (PyCFunction)CRLDistributionPt_get_reasons,       METH_VARARGS|METH_KEYWORDS, CRLDistributionPt_get_reasons_doc},
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
CRLDistributionPt_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    CRLDistributionPt *self;

    TraceObjNewEnter(type);

    if ((self = (CRLDistributionPt *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }

    if ((self->arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE)) == NULL) {
        type->tp_free(self);
        return set_nspr_error(NULL);
    }

    self->pt = NULL;

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static void
CRLDistributionPt_dealloc(CRLDistributionPt* self)
{
    TraceMethodEnter(self);

    if (self->arena) {
        PORT_FreeArena(self->arena, PR_FALSE);
    }

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(CRLDistributionPt_doc,
"An object representing a CRL Distribution Point");

static int
CRLDistributionPt_init(CRLDistributionPt *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"arg1", NULL};
    PyObject *arg;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i:CRLDistributionPt", kwlist,
                                     &arg))
        return -1;

    return 0;
}

static Py_ssize_t
CRLDistributionPt_general_names_count(CRLDistributionPt *self)
{
    if (!self->pt || self->pt->distPointType != generalName) {
        return 0;
    }

    return CERTGeneralName_list_count(self->pt->distPoint.fullName);
}

static PyObject *
CRLDistributionPt_general_names_tuple(CRLDistributionPt *self, RepresentationKind repr_kind)
{
    Py_ssize_t n_names;

    n_names = CRLDistributionPt_general_names_count(self);

    if (n_names == 0) {
        Py_INCREF(empty_tuple);
        return empty_tuple;
    }

    return CERTGeneralName_list_to_tuple(self->pt->distPoint.fullName, repr_kind);
}


static PyTypeObject CRLDistributionPtType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.CRLDistributionPoint",		/* tp_name */
    sizeof(CRLDistributionPt),			/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)CRLDistributionPt_dealloc,	/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    0,						/* tp_compare */
    (reprfunc)CRLDistributionPt_repr,		/* tp_repr */
    0,						/* tp_as_number */
    0,						/* tp_as_sequence */
    0,						/* tp_as_mapping */
    0,						/* tp_hash */
    0,						/* tp_call */
    (reprfunc)CRLDistributionPt_str,		/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
    CRLDistributionPt_doc,			/* tp_doc */
    0,						/* tp_traverse */
    0,						/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    CRLDistributionPt_methods,			/* tp_methods */
    CRLDistributionPt_members,			/* tp_members */
    CRLDistributionPt_getseters,		/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)CRLDistributionPt_init,		/* tp_init */
    0,						/* tp_alloc */
    CRLDistributionPt_new,			/* tp_new */
};

PyObject *
CRLDistributionPt_new_from_CRLDistributionPoint(CRLDistributionPoint *pt)
{
    CRLDistributionPt *self = NULL;

    TraceObjNewEnter(NULL);

    if ((self = (CRLDistributionPt *) CRLDistributionPtType.tp_new(&CRLDistributionPtType, NULL, NULL)) == NULL) {
        return NULL;
    }

    if (CERT_CopyCRLDistributionPoint(self->arena, &self->pt, pt) != SECSuccess) {
        set_nspr_error(NULL);
        Py_CLEAR(self);
        return NULL;
    }

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

/* ========================================================================== */
/* ======================== CRLDistributionPts Class ======================== */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static
PyGetSetDef CRLDistributionPts_getseters[] = {
    {NULL}  /* Sentinel */
};

static PyMemberDef CRLDistributionPts_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

/* =========================== Sequence Protocol ============================ */

static Py_ssize_t
CERTCrlDistributionPoints_count(CERTCrlDistributionPoints *dist_pts)
{
    Py_ssize_t count;
    CRLDistributionPoint **pts;

    if (!dist_pts) return 0;
    for (pts = dist_pts->distPoints, count = 0; *pts; pts++, count++);

    return count;
}

static Py_ssize_t
CRLDistributionPts_length(CRLDistributionPts *self)
{
    if (!self->py_pts) return 0;
    return PyTuple_Size(self->py_pts);
}

static PyObject *
CRLDistributionPts_item(CRLDistributionPts *self, register Py_ssize_t i)
{
    PyObject *py_pt = NULL;

    if (!self->py_pts) {
        return PyErr_Format(PyExc_ValueError, "%s is uninitialized", Py_TYPE(self)->tp_name);
    }
    py_pt = PyTuple_GetItem(self->py_pts, i);
    Py_XINCREF(py_pt);
    return py_pt;
}

static PyMethodDef CRLDistributionPts_methods[] = {
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static int
CRLDistributionPts_init_from_SECItem(CRLDistributionPts *self, SECItem *item)
{
    CERTCrlDistributionPoints *dist_pts;
    CRLDistributionPoint **pts, *pt;
    PLArenaPool *arena;
    Py_ssize_t count, i;
    PyObject *py_pts = NULL;

    Py_CLEAR(self->py_pts);

    if ((arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE)) == NULL) {
        return -1;
    }

    if ((dist_pts = CERT_DecodeCRLDistributionPoints(arena, item)) == NULL) {
        PyErr_SetString(PyExc_ValueError, "Failed to parse CRL Distribution Point Extension");
        PORT_FreeArena(arena, PR_FALSE);
        return -1;
    }

    count = CERTCrlDistributionPoints_count(dist_pts);
    
    if ((py_pts = PyTuple_New(count)) == NULL) {
        PORT_FreeArena(arena, PR_FALSE);
	return -1;
    }

    for (pts = dist_pts->distPoints, i = 0; (pt = *pts); pts++, i++) {
        PyObject *py_crl_dist_pt;

        if ((py_crl_dist_pt = CRLDistributionPt_new_from_CRLDistributionPoint(pt)) == NULL) {
            PORT_FreeArena(arena, PR_FALSE);
            Py_CLEAR(py_pts);
            return -1;
        }

        PyTuple_SetItem(py_pts, i, py_crl_dist_pt);
    }

    ASSIGN_NEW_REF(self->py_pts, py_pts);

    PORT_FreeArena(arena, PR_FALSE);

    return 0;
}

static PyObject *
CRLDistributionPts_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    CRLDistributionPts *self;

    TraceObjNewEnter(type);

    if ((self = (CRLDistributionPts *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }

    self->py_pts = NULL;

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static int
CRLDistributionPts_traverse(CRLDistributionPts *self, visitproc visit, void *arg)
{
    Py_VISIT(self->py_pts);
    return 0;
}

static int
CRLDistributionPts_clear(CRLDistributionPts* self)
{
    TraceMethodEnter(self);

    Py_CLEAR(self->py_pts);
    return 0;
}

static void
CRLDistributionPts_dealloc(CRLDistributionPts* self)
{
    TraceMethodEnter(self);

    CRLDistributionPts_clear(self);
    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(CRLDistributionPts_doc,
"An object representing CRL Distribution Points list");

static int
CRLDistributionPts_init(CRLDistributionPts *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"crl_dist_pt_extension", NULL};
    SecItem *py_sec_item;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!:CRLDistributionPts", kwlist,
                                     &SecItemType, &py_sec_item))
        return -1;

    return CRLDistributionPts_init_from_SECItem(self, &py_sec_item->item);
}

static PyObject *
CRLDistributionPts_repr(CRLDistributionPts *self)
{
    return PyString_FromFormat("<%s object at %p>",
                               Py_TYPE(self)->tp_name, self);
}

static PySequenceMethods CRLDistributionPts_as_sequence = {
    (lenfunc)CRLDistributionPts_length,		/* sq_length */
    0,						/* sq_concat */
    0,						/* sq_repeat */
    (ssizeargfunc)CRLDistributionPts_item,	/* sq_item */
    0,						/* sq_slice */
    0,						/* sq_ass_item */
    0,						/* sq_ass_slice */
    0,						/* sq_contains */
    0,						/* sq_inplace_concat */
    0,						/* sq_inplace_repeat */
};

static PyTypeObject CRLDistributionPtsType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.CRLDistributionPts",		/* tp_name */
    sizeof(CRLDistributionPts),			/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)CRLDistributionPts_dealloc,	/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    0,						/* tp_compare */
    (reprfunc)CRLDistributionPts_repr,		/* tp_repr */
    0,						/* tp_as_number */
    &CRLDistributionPts_as_sequence,		/* tp_as_sequence */
    0,						/* tp_as_mapping */
    0,						/* tp_hash */
    0,						/* tp_call */
    0,						/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,	/* tp_flags */
    CRLDistributionPts_doc,			/* tp_doc */
    (traverseproc)CRLDistributionPts_traverse,	/* tp_traverse */
    (inquiry)CRLDistributionPts_clear,		/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    CRLDistributionPts_methods,			/* tp_methods */
    CRLDistributionPts_members,			/* tp_members */
    CRLDistributionPts_getseters,		/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)CRLDistributionPts_init,		/* tp_init */
    0,						/* tp_alloc */
    CRLDistributionPts_new,			/* tp_new */
};

PyObject *
CRLDistributionPts_new_from_SECItem(SECItem *item)
{
    CRLDistributionPts *self = NULL;

    TraceObjNewEnter(NULL);

    if ((self = (CRLDistributionPts *) CRLDistributionPtsType.tp_new(&CRLDistributionPtsType, NULL, NULL)) == NULL) {
        return NULL;
    }

    if (CRLDistributionPts_init_from_SECItem(self, item) < 0) {
        Py_CLEAR(self);
        return NULL;
    }

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

/* ========================================================================== */
/* ============================ AuthKeyID Class ============================= */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static PyObject *
AuthKeyID_get_key_id(AuthKeyID *self, void *closure)
{
    TraceMethodEnter(self);

    if (!self->auth_key_id) {
        return PyErr_Format(PyExc_ValueError, "%s is uninitialized", Py_TYPE(self)->tp_name);
    }

    if (!self->auth_key_id->keyID.len || !self->auth_key_id->keyID.data) {
        Py_RETURN_NONE;
    }

    return SecItem_new_from_SECItem(&self->auth_key_id->keyID, SECITEM_unknown);
}

static PyObject *
AuthKeyID_get_serial_number(AuthKeyID *self, void *closure)
{
    TraceMethodEnter(self);

    if (!self->auth_key_id) {
        return PyErr_Format(PyExc_ValueError, "%s is uninitialized", Py_TYPE(self)->tp_name);
    }

    if (!self->auth_key_id->authCertSerialNumber.len || !self->auth_key_id->authCertSerialNumber.data) {
        Py_RETURN_NONE;
    }

    return integer_secitem_to_pylong(&self->auth_key_id->authCertSerialNumber);
}

static
PyGetSetDef AuthKeyID_getseters[] = {
    {"key_id", (getter)AuthKeyID_get_key_id,    (setter)NULL,
     "Returns the key id as a SecItem", NULL},
    {"serial_number", (getter)AuthKeyID_get_serial_number,    (setter)NULL,
     "Returns the key id as a SecItem", NULL},
    {NULL}  /* Sentinel */
};

static PyMemberDef AuthKeyID_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

PyDoc_STRVAR(AuthKeyID_get_general_names_doc,
"get_general_names(repr_kind=AsString) -> (general_name, ...)\n\
\n\
:Parameters:\n\
    repr_kind : RepresentationKind constant\n\
        Specifies what the contents of the returned tuple will be.\n\
        May be one of:\n\
\n\
        AsObject\n\
            The general name as a nss.GeneralName object\n\
        AsString\n\
            The general name as a string.\n\
            (e.g. \"http://crl.geotrust.com/crls/secureca.crl\")\n\
        AsTypeString\n\
            The general name type as a string.\n\
             (e.g. \"URI\")\n\
        AsTypeEnum\n\
            The general name type as a general name type enumerated constant.\n\
             (e.g. nss.certURI )\n\
        AsLabeledString\n\
            The general name as a string with it's type prepended.\n\
            (e.g. \"URI: http://crl.geotrust.com/crls/secureca.crl\"\n\
\n\
Returns a tuple of general names in the authentication key id extension\n\
for the issuer. If the issuer was not defined then the returned tuple\n\
will be empty.\n\
\n\
You may specify how the each member of the tuple is represented, by default\n\
it will be as a string.\n\
");

static PyObject *
AuthKeyID_get_general_names(AuthKeyID *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"repr_kind", NULL};
    int repr_kind = AsString;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:get_general_names", kwlist,
                                     &repr_kind))
        return NULL;

    if (!self->auth_key_id) {
        return PyErr_Format(PyExc_ValueError, "%s is uninitialized", Py_TYPE(self)->tp_name);
    }

    return AuthKeyID_general_names_tuple(self, repr_kind);
}

static PyObject *
AuthKeyID_format_lines(AuthKeyID *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"level", NULL};
    int level = 0;
    Py_ssize_t len;
    PyObject *lines = NULL;
    PyObject *obj = NULL;
    PyObject *obj1 = NULL;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:format_lines", kwlist, &level))
        return NULL;

    if ((lines = PyList_New(0)) == NULL) {
        return NULL;
    }

    if (!self->auth_key_id) {
        return lines;
    }

    FMT_LABEL_AND_APPEND(lines, _("Key ID"), level, fail);
    
    if ((obj = AuthKeyID_get_key_id(self, NULL)) == NULL) {
        goto fail;
    }
    APPEND_OBJ_TO_HEX_LINES_AND_CLEAR(lines, obj, level+1, fail);

    if ((obj = AuthKeyID_get_serial_number(self, NULL)) == NULL) {
        goto fail;
    }

    if ((obj1 = PyObject_Str(obj)) == NULL) {
        goto fail;
    }
    Py_CLEAR(obj);

    FMT_OBJ_AND_APPEND(lines, _("Serial Number"), obj1, level, fail);
    Py_CLEAR(obj1);

    if ((obj = AuthKeyID_general_names_tuple(self, AsString)) == NULL) {
        goto fail;
    }
    len = PyObject_Size(obj);
    if ((obj1 = PyString_FromFormat("General Names: [%d total]", len)) == NULL) {
        goto fail;
    }
    FMT_OBJ_AND_APPEND(lines, NULL, obj1, level, fail);
    Py_CLEAR(obj1);

    APPEND_LINES_AND_CLEAR(lines, obj, level+1, fail);

    return lines;

 fail:
    Py_XDECREF(obj);
    Py_XDECREF(obj1);
    Py_XDECREF(lines);
    return NULL;
}

static PyObject *
AuthKeyID_format(AuthKeyID *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter(self);

    return format_from_lines((format_lines_func)AuthKeyID_format_lines, (PyObject *)self, args, kwds);
}

static PyObject *
AuthKeyID_str(AuthKeyID *self)
{
    PyObject *py_formatted_result = NULL;

    TraceMethodEnter(self);

    py_formatted_result =  AuthKeyID_format(self, empty_tuple, NULL);
    return py_formatted_result;

}


static PyMethodDef AuthKeyID_methods[] = {
    {"format_lines",      (PyCFunction)AuthKeyID_format_lines,      METH_VARARGS|METH_KEYWORDS, generic_format_lines_doc},
    {"format",            (PyCFunction)AuthKeyID_format,            METH_VARARGS|METH_KEYWORDS, generic_format_doc},
    {"get_general_names", (PyCFunction)AuthKeyID_get_general_names, METH_VARARGS|METH_KEYWORDS, AuthKeyID_get_general_names_doc},
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
AuthKeyID_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    AuthKeyID *self;

    TraceObjNewEnter(type);

    if ((self = (AuthKeyID *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }

    if ((self->arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE)) == NULL) {
        type->tp_free(self);
        return set_nspr_error(NULL);
    }

    self->auth_key_id = NULL;

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static void
AuthKeyID_dealloc(AuthKeyID* self)
{
    TraceMethodEnter(self);

    if (self->arena) {
        PORT_FreeArena(self->arena, PR_FALSE);
    }

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(AuthKeyID_doc,
"An object representing Authentication Key ID extension");

static int
AuthKeyID_init(AuthKeyID *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"auth_key_id", NULL};
    SecItem *py_sec_item;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!:AuthKeyID", kwlist,
                                     &SecItemType, &py_sec_item))
        return -1;

    if ((self->auth_key_id = CERT_DecodeAuthKeyID(self->arena, &py_sec_item->item)) == NULL) {
        set_nspr_error("cannot decode AuthKeyID");
        return -1;
    }

    return 0;
}

static Py_ssize_t
AuthKeyID_general_names_count(AuthKeyID *self)
{
    if (!self->auth_key_id || !self->auth_key_id->authCertIssuer) {
        return 0;
    }

    return CERTGeneralName_list_count(self->auth_key_id->authCertIssuer);
}

static PyObject *
AuthKeyID_general_names_tuple(AuthKeyID *self, RepresentationKind repr_kind)
{
    Py_ssize_t n_names;

    n_names = AuthKeyID_general_names_count(self);

    if (n_names == 0) {
        Py_INCREF(empty_tuple);
        return empty_tuple;
    }

    return CERTGeneralName_list_to_tuple(self->auth_key_id->authCertIssuer, repr_kind);
}

static PyObject *
AuthKeyID_repr(AuthKeyID *self)
{
    Py_ssize_t n_names;
    PyObject *result = NULL;
    PyObject *sep = NULL;
    PyObject *names = NULL;
    PyObject *name_str = NULL;
    PyObject *key_id = NULL;
    PyObject *key_id_str = NULL;
    PyObject *serial_number = NULL;
    PyObject *serial_number_str = NULL;

    if (!self->auth_key_id) {
        return PyString_FromFormat("<%s object at %p>",
                                   Py_TYPE(self)->tp_name, self);
    }

    if ((sep = PyString_FromString(", ")) == NULL) {
        goto exit;
    }

    if ((names = AuthKeyID_general_names_tuple(self, AsString)) == NULL) {
        goto exit;
    }
    n_names = PyTuple_GET_SIZE(names);

    /* Paste them all together with ", " between. */
    if ((name_str = _PyString_Join(sep, names)) == NULL) {
        goto exit;
    }

    if ((key_id = AuthKeyID_get_key_id(self, NULL)) == NULL) {
        goto exit;
    }

    if ((key_id_str = PyObject_Str(key_id)) == NULL) {
        goto exit;
    }

    if ((serial_number = AuthKeyID_get_serial_number(self, NULL)) == NULL) {
        goto exit;
    }

    if ((serial_number_str = PyObject_Str(serial_number)) == NULL) {
        goto exit;
    }

    result = PyString_FromFormat("ID: %s, Serial Number: %s, Issuer: [%s]",
                                 PyString_AsString(key_id_str),
                                 PyString_AsString(serial_number_str),
                                 PyString_AsString(name_str));


    exit:
    Py_XDECREF(sep);
    Py_XDECREF(names);
    Py_XDECREF(name_str);
    Py_XDECREF(key_id);
    Py_XDECREF(key_id_str);
    Py_XDECREF(serial_number);
    Py_XDECREF(serial_number_str);
    return result;
}

static PyTypeObject AuthKeyIDType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.AuthKeyID",			/* tp_name */
    sizeof(AuthKeyID),				/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)AuthKeyID_dealloc,		/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    0,						/* tp_compare */
    (reprfunc)AuthKeyID_repr,			/* tp_repr */
    0,						/* tp_as_number */
    0,						/* tp_as_sequence */
    0,						/* tp_as_mapping */
    0,						/* tp_hash */
    0,						/* tp_call */
    (reprfunc)AuthKeyID_str,			/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
    AuthKeyID_doc,				/* tp_doc */
    0,						/* tp_traverse */
    0,						/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    AuthKeyID_methods,				/* tp_methods */
    AuthKeyID_members,				/* tp_members */
    AuthKeyID_getseters,			/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)AuthKeyID_init,			/* tp_init */
    0,						/* tp_alloc */
    AuthKeyID_new,				/* tp_new */
};

PyObject *
AuthKeyID_new_from_CERTAuthKeyID(CERTAuthKeyID *auth_key_id)
{
    AuthKeyID *self = NULL;

    TraceObjNewEnter(NULL);

    if ((self = (AuthKeyID *) AuthKeyIDType.tp_new(&AuthKeyIDType, NULL, NULL)) == NULL) {
        return NULL;
    }

    if (CERT_CopyAuthKeyID(self->arena, &self->auth_key_id, auth_key_id) != SECSuccess) {
        set_nspr_error(NULL);
        Py_CLEAR(self);
        return NULL;
    }

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

PyObject *
AuthKeyID_new_from_SECItem(SECItem *item)
{
    AuthKeyID *self = NULL;

    TraceObjNewEnter(NULL);

    if ((self = (AuthKeyID *) AuthKeyIDType.tp_new(&AuthKeyIDType, NULL, NULL)) == NULL) {
        return NULL;
    }

    if ((self->auth_key_id = CERT_DecodeAuthKeyID(self->arena, item)) == NULL) {
        set_nspr_error("cannot decode AuthKeyID");
        Py_CLEAR(self);
        return NULL;
    }

    TraceObjNewLeave(self);
    return (PyObject *) self;
}



/* ========================================================================== */
/* ======================== BasicConstraints Class ========================== */
/* ========================================================================== */

/* ============================ Attribute Access ============================ */

static PyObject *
BasicConstraints_get_is_ca(BasicConstraints *self, void *closure)
{
    TraceMethodEnter(self);

    return PyBool_FromLong(self->bc.isCA);

    return NULL;
}

static PyObject *
BasicConstraints_get_path_len(BasicConstraints *self, void *closure)
{
    TraceMethodEnter(self);

    return PyInt_FromLong(self->bc.pathLenConstraint);

    return NULL;
}

static
PyGetSetDef BasicConstraints_getseters[] = {
    {"is_ca", (getter)BasicConstraints_get_is_ca,    (setter)NULL,
     "returns boolean, True if certificate is a certificate authority (i.e. CA)", NULL},
    {"path_len", (getter)BasicConstraints_get_path_len,    (setter)NULL,
     "returns max path length constraint as an integer", NULL},
    {NULL}  /* Sentinel */
};

static PyMemberDef BasicConstraints_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

static PyObject *
BasicConstraints_format_lines(BasicConstraints *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"level", NULL};
    int level = 0;
    PyObject *lines = NULL;
    PyObject *obj = NULL;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:format_lines", kwlist, &level))
        return NULL;

    if ((lines = PyList_New(0)) == NULL) {
        return NULL;
    }

    obj = self->bc.isCA ? Py_True : Py_False;
    Py_INCREF(obj);
    FMT_OBJ_AND_APPEND(lines, _("Is CA"), obj, level, fail);
    Py_CLEAR(obj);
    
    if ((obj = PyString_FromFormat("%d", self->bc.pathLenConstraint)) == NULL) {
        goto fail;
    }
    FMT_OBJ_AND_APPEND(lines, _("Path Length"), obj, level, fail);
    Py_CLEAR(obj);

    return lines;

 fail:
    Py_XDECREF(obj);
    Py_XDECREF(lines);
    return NULL;
}

static PyObject *
BasicConstraints_format(BasicConstraints *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter(self);

    return format_from_lines((format_lines_func)BasicConstraints_format_lines, (PyObject *)self, args, kwds);
}

static PyMethodDef BasicConstraints_methods[] = {
    {"format_lines", (PyCFunction)BasicConstraints_format_lines,   METH_VARARGS|METH_KEYWORDS, generic_format_lines_doc},
    {"format",       (PyCFunction)BasicConstraints_format,         METH_VARARGS|METH_KEYWORDS, generic_format_doc},
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
BasicConstraints_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    BasicConstraints *self;

    TraceObjNewEnter(type);

    if ((self = (BasicConstraints *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }

    memset(&self->bc, 0, sizeof(self->bc));


    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static void
BasicConstraints_dealloc(BasicConstraints* self)
{
    TraceMethodEnter(self);

    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(BasicConstraints_doc,
"An object representing X509 Basic Constraints Extension");

static int
BasicConstraints_init(BasicConstraints *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"basic_constraints", NULL};
    SecItem *py_sec_item;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i:BasicConstraints", kwlist,
                                     &SecItemType, &py_sec_item))

        return -1;

    if (CERT_DecodeBasicConstraintValue(&self->bc, &py_sec_item->item) != SECSuccess) {
        set_nspr_error("cannot decode Basic Constraints");
        return -1;
    }

    return 0;
}

static PyObject *
BasicConstraints_repr(BasicConstraints *self)
{
    return PyString_FromFormat("<%s object at %p>",
                               Py_TYPE(self)->tp_name, self);
}

static PyObject *
BasicConstraints_str(BasicConstraints *self)
{
    return PyString_FromFormat("is_ca=%s path_len=%d",
                               self->bc.isCA ? "True" : "False", self->bc.pathLenConstraint);
}

static PyTypeObject BasicConstraintsType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.BasicConstraints",			/* tp_name */
    sizeof(BasicConstraints),			/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)BasicConstraints_dealloc,	/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    0,						/* tp_compare */
    (reprfunc)BasicConstraints_repr,		/* tp_repr */
    0,						/* tp_as_number */
    0,						/* tp_as_sequence */
    0,						/* tp_as_mapping */
    0,						/* tp_hash */
    0,						/* tp_call */
    (reprfunc)BasicConstraints_str,		/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
    BasicConstraints_doc,			/* tp_doc */
    (traverseproc)0,				/* tp_traverse */
    (inquiry)0,					/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    BasicConstraints_methods,			/* tp_methods */
    BasicConstraints_members,			/* tp_members */
    BasicConstraints_getseters,			/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)BasicConstraints_init,		/* tp_init */
    0,						/* tp_alloc */
    BasicConstraints_new,			/* tp_new */
};

PyObject *
BasicConstraints_new_from_SECItem(SECItem *item)
{
    BasicConstraints *self = NULL;

    TraceObjNewEnter(NULL);

    if ((self = (BasicConstraints *) BasicConstraintsType.tp_new(&BasicConstraintsType, NULL, NULL)) == NULL) {
        return NULL;
    }

    if (CERT_DecodeBasicConstraintValue(&self->bc, item) != SECSuccess) {
        set_nspr_error("cannot decode Basic Constraints");
        Py_CLEAR(self);
        return NULL;
    }

    TraceObjNewLeave(self);
    return (PyObject *) self;
}

/* ========================================================================== */
/* ======================= CertificateRequest Class ========================= */
/* ========================================================================== */

static int
CertificateRequest_init_from_SECItem(CertificateRequest *self, SECItem *der_cert_req)
{
    if ((self->cert_req = PORT_ArenaZAlloc(self->arena, sizeof(CERTCertificateRequest))) == NULL) {
        set_nspr_error(NULL);
        return -1;
    }
    self->cert_req->arena = self->arena;

    /* Since cert request is a signed data, must decode to get the inner data */
    if (SEC_ASN1DecodeItem(self->arena, &self->signed_data, 
                           SEC_ASN1_GET(CERT_SignedDataTemplate),
                           der_cert_req) != SECSuccess) {
        set_nspr_error(NULL);
        return -1;
    }

    if (SEC_ASN1DecodeItem(self->arena, self->cert_req, 
                           SEC_ASN1_GET(CERT_CertificateRequestTemplate),
                           &self->signed_data.data) != SECSuccess) {
        set_nspr_error(NULL);
        return -1;
    }

    if (CERT_VerifySignedDataWithPublicKeyInfo(&self->signed_data,
                                               &self->cert_req->subjectPublicKeyInfo,
                                               NULL) != SECSuccess) {
        set_nspr_error(NULL);
        return -1;
    }

   return 0;
}
/* ============================ Attribute Access ============================ */

static PyObject *
CertificateRequest_get_subject(CertificateRequest *self, void *closure)
{
    TraceMethodEnter(self);

    return DN_new_from_CERTName(&self->cert_req->subject);
}

static PyObject *
CertificateRequest_get_version(CertificateRequest *self, void *closure)
{
    TraceMethodEnter(self);

    return integer_secitem_to_pylong(&self->cert_req->version);
}

static PyObject *
CertificateRequest_get_subject_public_key_info(CertificateRequest *self, void *closure)
{
    TraceMethodEnter(self);

    return SubjectPublicKeyInfo_new_from_CERTSubjectPublicKeyInfo(
               &self->cert_req->subjectPublicKeyInfo);
}


static PyObject *
CertificateRequest_get_extensions(CertificateRequest *self, void *closure)
{
    CERTCertExtension **extensions_list = NULL, **extensions = NULL;
    int num_extensions, i;
    PyObject *extensions_tuple;

    TraceMethodEnter(self);

    if (self->cert_req->attributes != NULL                   &&
        self->cert_req->attributes[0] != NULL                &&
        self->cert_req->attributes[0]->attrType.data != NULL &&
        self->cert_req->attributes[0]->attrType.len > 0      &&
        SECOID_FindOIDTag(&self->cert_req->attributes[0]->attrType) == SEC_OID_PKCS9_EXTENSION_REQUEST) {
        if (CERT_GetCertificateRequestExtensions(self->cert_req, &extensions_list)  != SECSuccess) {
            return set_nspr_error("CERT_GetCertificateRequestExtensions failed");
        }
    } else {
        Py_INCREF(empty_tuple);
        return empty_tuple;
    }

    /* First count how many extensions the cert request has */
    for (extensions = extensions_list, num_extensions = 0;
         extensions && *extensions;
         extensions++, num_extensions++);

    /* Allocate a tuple */
    if ((extensions_tuple = PyTuple_New(num_extensions)) == NULL) {
        return NULL;
    }

    /* Copy the extensions into the tuple */
    for (extensions = extensions_list, i = 0; extensions && *extensions; extensions++, i++) {
        CERTCertExtension *extension = *extensions;
        PyObject *py_extension;
        
        if ((py_extension = CertificateExtension_new_from_CERTCertExtension(extension)) == NULL) {
            Py_DECREF(extensions_tuple);
            return NULL;
        }

        PyTuple_SetItem(extensions_tuple, i, py_extension);
    }

    return extensions_tuple;
}

static
PyGetSetDef CertificateRequest_getseters[] = {
    {"subject", (getter)CertificateRequest_get_subject, (setter)NULL,
     "subject as an `DN` object", NULL},
    {"version", (getter)CertificateRequest_get_version, (setter)NULL,
     "version as integer", NULL},
    {"subject_public_key_info", (getter)CertificateRequest_get_subject_public_key_info, NULL,
     "certificate public info as SubjectPublicKeyInfo object",  NULL},
    {"extensions", (getter)CertificateRequest_get_extensions, NULL,
     "certificate extensions as a tuple of CertificateExtension objects",  NULL},

    {NULL}  /* Sentinel */
};

static PyMemberDef CertificateRequest_members[] = {
    {NULL}  /* Sentinel */
};

/* ============================== Class Methods ============================= */

static PyObject *
CertificateRequest_format_lines(CertificateRequest *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"level", NULL};
    int level = 0;
    Py_ssize_t len, i;
    PyObject *lines = NULL;
    PyObject *obj = NULL;
    PyObject *obj1 = NULL;
    PyObject *obj2 = NULL;
    PyObject *obj3 = NULL;
    PyObject *extensions = NULL;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i:format_lines", kwlist, &level))
        return NULL;

    if ((lines = PyList_New(0)) == NULL) {
        goto fail;
    }

    FMT_LABEL_AND_APPEND(lines, _("Data"), level+1, fail);

    if ((obj = CertificateRequest_get_version(self, NULL)) == NULL) {
        goto fail;
    }
    if ((obj1 = PyInt_FromLong(1)) == NULL) {
        goto fail;
    }
    if ((obj2 = PyNumber_Add(obj, obj1)) == NULL) {
        goto fail;
    }
    if ((obj3 = obj_sprintf("%d (%#x)", obj2, obj)) == NULL) {
        goto fail;
    }
    FMT_OBJ_AND_APPEND(lines, _("Version"), obj3, level+2, fail);
    Py_CLEAR(obj);
    Py_CLEAR(obj1);
    Py_CLEAR(obj2);
    Py_CLEAR(obj3);

    if ((obj = CertificateRequest_get_subject(self, NULL)) == NULL) {
        goto fail;
    }
    FMT_OBJ_AND_APPEND(lines, _("Subject"), obj, level+2, fail);
    Py_CLEAR(obj);

    FMT_LABEL_AND_APPEND(lines, _("Subject Public Key Info"), level+2, fail);

    if ((obj = CertificateRequest_get_subject_public_key_info(self, NULL)) == NULL) {
        goto fail;
    }

    CALL_FORMAT_LINES_AND_APPEND(lines, obj, level+3, fail);
    Py_CLEAR(obj);

    if ((extensions = CertificateRequest_get_extensions(self, NULL)) == NULL) {
        goto fail;
    }

    len = PyTuple_Size(extensions);
    if ((obj = PyString_FromFormat("Signed Extensions: (%d)", len)) == NULL) {
        goto fail;
    }
    FMT_OBJ_AND_APPEND(lines, NULL, obj, level+1, fail);
    Py_CLEAR(obj);

    for (i = 0; i < len; i++) {
        obj = PyTuple_GetItem(extensions, i);
        CALL_FORMAT_LINES_AND_APPEND(lines, obj, level+2, fail);
        FMT_LABEL_AND_APPEND(lines, NULL, 0, fail);
    }
    Py_CLEAR(extensions);

    return lines;

 fail:
    Py_XDECREF(obj);
    Py_XDECREF(obj1);
    Py_XDECREF(obj2);
    Py_XDECREF(obj3);
    Py_XDECREF(lines);
    Py_XDECREF(extensions);
    return NULL;
}

static PyObject *
CertificateRequest_format(CertificateRequest *self, PyObject *args, PyObject *kwds)
{
    TraceMethodEnter(self);

    return format_from_lines((format_lines_func)CertificateRequest_format_lines, (PyObject *)self, args, kwds);
}

static PyObject *
CertificateRequest_str(CertificateRequest *self)
{
    PyObject *py_formatted_result = NULL;

    py_formatted_result = CertificateRequest_format(self, empty_tuple, NULL);
    return py_formatted_result;

}


static PyMethodDef CertificateRequest_methods[] = {
    {"format_lines",           (PyCFunction)CertificateRequest_format_lines,           METH_VARARGS|METH_KEYWORDS, generic_format_lines_doc},
    {"format",                 (PyCFunction)CertificateRequest_format,                 METH_VARARGS|METH_KEYWORDS, generic_format_doc},
    {NULL, NULL}  /* Sentinel */
};

/* =========================== Class Construction =========================== */

static PyObject *
CertificateRequest_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    CertificateRequest *self;

    TraceObjNewEnter(type);

    if ((self = (CertificateRequest *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }

    if ((self->arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE)) == NULL) {
        type->tp_free(self);
        return set_nspr_error(NULL);
    }

    self->cert_req = NULL;
    memset(&self->signed_data, 0, sizeof(self->signed_data));

    TraceObjNewLeave(self);
    return (PyObject *)self;
}

static void
CertificateRequest_dealloc(CertificateRequest* self)
{
    TraceMethodEnter(self);

    /* 
     * We could call CERT_DestroyCertificateRequest() but all
     * CERT_DestroyCertificateRequest() does is call PORT_FreeArena() on
     * the arena stored in the CERTCertificateRequest. All the other
     * dealloc routines for objects with arenas call PORT_FreeArena()
     * explicitly, so for consistency and to make sure the freeing of
     * the arena is explicit rather than hidden we do the same here.
     *
     * Also, self->signed_data does not need to be explicitly freed
     * because it's allocated out of the arena.
     */

    if (self->arena) {
        PORT_FreeArena(self->arena, PR_FALSE);
    }
    self->ob_type->tp_free((PyObject*)self);
}

PyDoc_STRVAR(CertificateRequest_doc,
"CertificateRequest(data=None)\n\
\n\
:Parameters:\n\
    data : SecItem or str or any buffer compatible object\n\
        Data to initialize the certificate request from, must be in DER format\n\
\n\
An object representing a certificate request");

static int
CertificateRequest_init(CertificateRequest *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"data", NULL};
    PyObject *py_data = NULL;
    SECItem tmp_item;
    SECItem *der_item = NULL;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O:CertificateRequest", kwlist,
                                     &py_data))
        return -1;

    if (py_data) {
        if (PySecItem_Check(py_data)) {
            der_item = &((SecItem *)py_data)->item;
        } else if (PyObject_CheckReadBuffer(py_data)) {
            unsigned char *data = NULL;
            Py_ssize_t data_len;

            if (PyObject_AsReadBuffer(py_data, (void *)&data, &data_len))
                return -1;

            tmp_item.data = data;
            tmp_item.len = data_len;
            der_item = &tmp_item;
        } else {
            PyErr_SetString(PyExc_TypeError, "data must be SecItem or buffer compatible");
            return -1;
        }

        return CertificateRequest_init_from_SECItem(self, der_item);
    }

    return 0;
}

static PyObject *
CertificateRequest_repr(CertificateRequest *self)
{
    return PyString_FromFormat("<%s object at %p>",
                               Py_TYPE(self)->tp_name, self);
}

static PyTypeObject CertificateRequestType = {
    PyObject_HEAD_INIT(NULL)
    0,						/* ob_size */
    "nss.nss.CertificateRequest",		/* tp_name */
    sizeof(CertificateRequest),			/* tp_basicsize */
    0,						/* tp_itemsize */
    (destructor)CertificateRequest_dealloc,	/* tp_dealloc */
    0,						/* tp_print */
    0,						/* tp_getattr */
    0,						/* tp_setattr */
    0,						/* tp_compare */
    (reprfunc)CertificateRequest_repr,		/* tp_repr */
    0,						/* tp_as_number */
    0,						/* tp_as_sequence */
    0,						/* tp_as_mapping */
    0,						/* tp_hash */
    0,						/* tp_call */
    (reprfunc)CertificateRequest_str,		/* tp_str */
    0,						/* tp_getattro */
    0,						/* tp_setattro */
    0,						/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
    CertificateRequest_doc,			/* tp_doc */
    (traverseproc)0,				/* tp_traverse */
    (inquiry)0,					/* tp_clear */
    0,						/* tp_richcompare */
    0,						/* tp_weaklistoffset */
    0,						/* tp_iter */
    0,						/* tp_iternext */
    CertificateRequest_methods,			/* tp_methods */
    CertificateRequest_members,			/* tp_members */
    CertificateRequest_getseters,		/* tp_getset */
    0,						/* tp_base */
    0,						/* tp_dict */
    0,						/* tp_descr_get */
    0,						/* tp_descr_set */
    0,						/* tp_dictoffset */
    (initproc)CertificateRequest_init,		/* tp_init */
    0,						/* tp_alloc */
    CertificateRequest_new,			/* tp_new */
};

/* ========================== PK11 Methods =========================== */

static char *
PK11_password_callback(PK11SlotInfo *slot, PRBool retry, void *arg)
{
    PyGILState_STATE gstate;
    PyObject *password_callback = NULL;
    PyObject *pin_args = arg; /* borrowed reference, don't decrement */
    PyObject *py_slot = NULL;
    PyObject *item;
    PyObject *result = NULL;
    PyObject *new_args = NULL;
    Py_ssize_t argc;
    int i, j;
    char *password = NULL;

    gstate = PyGILState_Ensure();

    TraceMessage("PK11_password_callback: enter");

    if ((password_callback = get_thread_local("password_callback")) == NULL) {
        if (!PyErr_Occurred()) {
            PySys_WriteStderr("PK11 password callback undefined\n");
        } else {
            PyErr_Print();
        }
	PyGILState_Release(gstate);
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

    if ((py_slot = PK11Slot_new_from_PK11SlotInfo(slot)) == NULL) {
        PySys_WriteStderr("exception in PK11 password callback\n");
        PyErr_Print();
        goto exit;
    }

    PyTuple_SetItem(new_args, 0, py_slot);
    PyTuple_SetItem(new_args, 1, PyBool_FromLong(retry));

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

    if (!(PyString_Check(result) || PyUnicode_Check(result))) {
        PySys_WriteStderr("Error, PK11 password callback expected string result.\n");
        goto exit;
    }

    password = PORT_Strdup(PyString_AsString(result));

 exit:
    TraceMessage("PK11_password_callback: exiting");

    Py_XDECREF(new_args);
    Py_XDECREF(result);

    PyGILState_Release(gstate);

    return password;
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

    if (set_thread_local("password_callback", callback) < 0) {
        return NULL;
    }

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
        Py_INCREF(args);
        parse_args = args;
    } else {
        parse_args = PyTuple_GetSlice(args, 0, n_base_args);
    }
    if (!PyArg_ParseTuple(parse_args, "s:find_cert_from_nickname", &nickname)) {
        Py_DECREF(parse_args);
        return NULL;
    }
    Py_DECREF(parse_args);

    pin_args = PyTuple_GetSlice(args, n_base_args, argc);

    Py_BEGIN_ALLOW_THREADS
    if ((cert = PK11_FindCertFromNickname(nickname, pin_args)) == NULL) {
	Py_BLOCK_THREADS
        Py_DECREF(pin_args);
        return set_nspr_error(NULL);
    }
    Py_END_ALLOW_THREADS

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
        Py_INCREF(args);
        parse_args = args;
    } else {
        parse_args = PyTuple_GetSlice(args, 0, n_base_args);
    }
    if (!PyArg_ParseTuple(parse_args, "O!:find_key_by_any_cert",
                          &CertificateType, &py_cert)) {
        Py_DECREF(parse_args);
        return NULL;
    }
    Py_DECREF(parse_args);

    pin_args = PyTuple_GetSlice(args, n_base_args, argc);

    Py_BEGIN_ALLOW_THREADS
    if ((private_key = PK11_FindKeyByAnyCert(py_cert->cert, pin_args)) == NULL) {
	Py_BLOCK_THREADS
        Py_DECREF(pin_args);
        return set_nspr_error(NULL);
    }
    Py_END_ALLOW_THREADS

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
    if (buf == NULL) {
        return PyErr_NoMemory();
    }

    Py_BEGIN_ALLOW_THREADS
    status = PK11_GenerateRandom(buf, num_bytes);
    Py_END_ALLOW_THREADS
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
    static char *kwlist[] = {"lines_pairs", "indent", NULL};
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
        if ((py_indent = PyString_FromString("    ")) == NULL) {
            goto fail;
        }
    } else {
        Py_INCREF(py_indent);
    }

    indent_len = PyString_Size(py_indent);
    formatted_str_len = 0;

    num_lines = PyList_Size(py_lines);

    /* First, scan all the lines and compute the final destination size, do all error in this
       loop so we don't have to do it again during the copy phase */
    for (i = 0; i < num_lines; i++) {
        py_pair = PyList_GetItem(py_lines, i);
        if (!PyTuple_Check(py_pair) || PyTuple_Size(py_pair) != 2) {
            PyErr_Format(PyExc_TypeError, "lines[%ld] must be a 2 valued tuple", i);
            goto fail;
        }

        py_level = PyTuple_GetItem(py_pair, 0);
        py_line  = PyTuple_GetItem(py_pair, 1);

        if (!PyInt_Check(py_level)) {
            PyErr_Format(PyExc_TypeError, "the first item in the pair at lines[%ld] list must be an integer", i);
            goto fail;
        }
        line_level = PyInt_AsLong(py_level);
        if (line_level < 0) {
            PyErr_Format(PyExc_TypeError, "the first item in the pair at lines[%ld] list must be a non-negative integer", i);
            goto fail;
        }

        if (!(PyString_Check(py_line) || PyUnicode_Check(py_line))) {
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
    if ((py_formatted_str = PyString_FromStringAndSize(NULL, formatted_str_len)) == NULL) {
        goto fail;
    }

    formatted_str = PyString_AsString(py_formatted_str);
    dst = formatted_str;

    for (i = 0; i < num_lines; i++) {
        py_pair = PyList_GetItem(py_lines, i);
        py_level = PyTuple_GetItem(py_pair, 0);
        py_line  = PyTuple_GetItem(py_pair, 1);

        line_level = PyInt_AsLong(py_level);
        PyString_AsStringAndSize(py_line, &line, &line_len);
        line_end = line + line_len;

        if (line_level != cur_level) {
            cur_level = line_level;
	    Py_CLEAR(py_cur_level_indent);
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

PyDoc_STRVAR(nss_is_initialized_doc,
"nss_is_initialized() --> bool\n\
\n\
Returns whether Network Security Services has already been initialized or not.\n\
");

static PyObject *
nss_is_initialized(PyObject *self, PyObject *args)
{
    PRBool is_init;
    TraceMethodEnter(self);

    Py_BEGIN_ALLOW_THREADS
    is_init = NSS_IsInitialized();
    Py_END_ALLOW_THREADS

    if (is_init) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

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

    Py_BEGIN_ALLOW_THREADS
    if (NSS_Init(cert_dir) != SECSuccess) {
        Py_BLOCK_THREADS
        return set_nspr_error(NULL);
    }
    Py_END_ALLOW_THREADS

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

    Py_BEGIN_ALLOW_THREADS
    if (NSS_NoDB_Init(NULL) != SECSuccess) {
        Py_BLOCK_THREADS
        return set_nspr_error(NULL);
    }
    Py_END_ALLOW_THREADS

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

    Py_BEGIN_ALLOW_THREADS
    if (NSS_Shutdown() != SECSuccess) {
        Py_BLOCK_THREADS
        return set_nspr_error(NULL);
    }
    Py_END_ALLOW_THREADS

    Py_RETURN_NONE;
}

PyDoc_STRVAR(cert_oid_str_doc,
"oid_str(oid) -> string\n\
\n\
:Parameters:\n\
     oid : may be one of integer, string, SecItem\n\
         May be one of:\n\
         \n\
         * integer:: A SEC OID enumeration constant, also known as a tag\n\
           (i.e. SEC_OID_*) for example SEC_OID_AVA_COMMON_NAME.\n\
         * string:: A string in dotted decimal representation, for example\n\
           'OID.2.5.4.3'. The 'OID.' prefix is optional.\n\
           Or a string for the tag name (e.g. 'SEC_OID_AVA_COMMON_NAME')\n\
           The 'SEC_OID\\_' prefix is optional. Or one of the canonical\n\
           abbreviations (e.g. 'cn'). Case is not significant.\n\
         * SecItem:: A SecItem object encapsulating the OID in \n\
           DER format.\n\
\n\
Given an oid return it's description as a string.\n\
");
static PyObject *
cert_oid_str(PyObject *self, PyObject *args)
{
    PyObject *arg;
    int oid_tag;
    SECOidData *oiddata;

    TraceMethodEnter(self);

   if (!PyArg_ParseTuple(args, "O:oid_str", &arg))
        return NULL;

   oid_tag = get_oid_tag_from_object(arg);
   if (oid_tag == SEC_OID_UNKNOWN || oid_tag == -1) {
       return NULL;
   }

   if ((oiddata = SECOID_FindOIDByTag(oid_tag)) == NULL) {
       return set_nspr_error(NULL);
   }

   return PyString_FromString(oiddata->desc);
}


PyDoc_STRVAR(cert_oid_tag_name_doc,
"oid_tag_name(oid) -> string\n\
\n\
:Parameters:\n\
     oid : may be one of integer, string, SecItem\n\
         May be one of:\n\
         \n\
         * integer:: A SEC OID enumeration constant, also known as a tag\n\
           (i.e. SEC_OID_*) for example SEC_OID_AVA_COMMON_NAME.\n\
         * string:: A string in dotted decimal representation, for example\n\
           'OID.2.5.4.3'. The 'OID.' prefix is optional.\n\
           Or a string for the tag name (e.g. 'SEC_OID_AVA_COMMON_NAME')\n\
           The 'SEC_OID\\_' prefix is optional. Or one of the canonical\n\
           abbreviations (e.g. 'cn'). Case is not significant.\n\
         * SecItem:: A SecItem object encapsulating the OID in \n\
           DER format.\n\
\n\
Given an oid return it's tag constant as a string.\n\
");
static PyObject *
cert_oid_tag_name(PyObject *self, PyObject *args)
{
    PyObject *arg;
    int oid_tag;
    PyObject *py_name;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "O:oid_tag_name", &arg))
        return NULL;
    
    oid_tag = get_oid_tag_from_object(arg);
    if (oid_tag == SEC_OID_UNKNOWN || oid_tag == -1) {
        return NULL;
    }

    py_name = oid_tag_name_from_tag(oid_tag);
    return py_name;
}

PyDoc_STRVAR(cert_oid_tag_doc,
"oid_tag(oid) -> int\n\
\n\
:Parameters:\n\
     oid : may be one of integer, string, SecItem\n\
         May be one of:\n\
         \n\
         * integer:: A SEC OID enumeration constant, also known as a tag\n\
           (i.e. SEC_OID_*) for example SEC_OID_AVA_COMMON_NAME.\n\
         * string:: A string in dotted decimal representation, for example\n\
           'OID.2.5.4.3'. The 'OID.' prefix is optional.\n\
           Or a string for the tag name (e.g. 'SEC_OID_AVA_COMMON_NAME')\n\
           The 'SEC_OID\\_' prefix is optional. Or one of the canonical\n\
           abbreviations (e.g. 'cn'). Case is not significant.\n\
         * SecItem:: A SecItem object encapsulating the OID in \n\
           DER format.\n\
\n\
Given an oid return it's tag constant.\n\
");
static PyObject *
cert_oid_tag(PyObject *self, PyObject *args)
{
    PyObject *result;
    PyObject *arg;
    int oid_tag;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "O:oid_tag", &arg))
        return NULL;
    
    oid_tag = get_oid_tag_from_object(arg);
    if (oid_tag == SEC_OID_UNKNOWN || oid_tag == -1) {
        return NULL;
    }

    result = PyInt_FromLong(oid_tag);
    return result;
}

PyDoc_STRVAR(cert_oid_dotted_decimal_doc,
"oid_dotted_decimal(oid) -> string\n\
\n\
:Parameters:\n\
     oid : may be one of integer, string, SecItem\n\
         May be one of:\n\
         \n\
         * integer:: A SEC OID enumeration constant, also known as a tag\n\
           (i.e. SEC_OID_*) for example SEC_OID_AVA_COMMON_NAME.\n\
         * string:: A string in dotted decimal representation, for example\n\
           'OID.2.5.4.3'. The 'OID.' prefix is optional.\n\
           Or a string for the tag name (e.g. 'SEC_OID_AVA_COMMON_NAME')\n\
           The 'SEC_OID\\_' prefix is optional. Or one of the canonical\n\
           abbreviations (e.g. 'cn'). Case is not significant.\n\
         * SecItem:: A SecItem object encapsulating the OID in \n\
           DER format.\n\
\n\
Given an oid return it's tag constant as a string.\n\
");
static PyObject *
cert_oid_dotted_decimal(PyObject *self, PyObject *args)
{
    PyObject *arg;
    int oid_tag;
    SECOidData *oiddata;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "O:oid_dotted_decimal", &arg))
        return NULL;
    
    oid_tag = get_oid_tag_from_object(arg);
    if (oid_tag == SEC_OID_UNKNOWN || oid_tag == -1) {
        return NULL;
    }

    if ((oiddata = SECOID_FindOIDByTag(oid_tag)) == NULL) {
        return set_nspr_error(NULL);
    }

    return oid_secitem_to_pystr_dotted_decimal(&oiddata->oid);
}


static PyObject *
key_mechanism_type_to_pystr(CK_MECHANISM_TYPE mechanism)
{
    PyObject *py_value;
    PyObject *py_name;

    if ((py_value = PyInt_FromLong(mechanism)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "unable to create object");
        return NULL;
    }

    if ((py_name = PyDict_GetItem(ckm_value_to_name, py_value)) == NULL) {
        Py_DECREF(py_value);
	PyErr_Format(PyExc_KeyError, "mechanism name not found: %lu", mechanism);
        return NULL;
    }

    Py_DECREF(py_value);
    Py_INCREF(py_name);

    return py_name;
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

    return key_mechanism_type_to_pystr(mechanism);
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
    PyObject *py_lower_name;
    PyObject *py_value;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "S:key_mechanism_type_from_name", &py_name))
        return NULL;

    if ((py_lower_name = PyObject_CallMethod(py_name, "lower", NULL)) == NULL) {
        return NULL;
    }

    if ((py_value = PyDict_GetItem(ckm_name_to_value, py_lower_name)) == NULL) {
	PyErr_Format(PyExc_KeyError, "mechanism name not found: %s", PyString_AsString(py_name));
        Py_DECREF(py_lower_name);
        return NULL;
    }

    Py_DECREF(py_lower_name);
    Py_INCREF(py_value);

    return py_value;
}

static PyObject *
pk11_attribute_type_to_pystr(CK_ATTRIBUTE_TYPE type)
{
    PyObject *py_value;
    PyObject *py_name;

    if ((py_value = PyInt_FromLong(type)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "unable to create object");
        return NULL;
    }

    if ((py_name = PyDict_GetItem(cka_value_to_name, py_value)) == NULL) {
        Py_DECREF(py_value);
	PyErr_Format(PyExc_KeyError, "attribute type name not found: %lu", type);
        return NULL;
    }

    Py_DECREF(py_value);
    Py_INCREF(py_name);

    return py_name;
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

    return pk11_attribute_type_to_pystr(type);
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
    PyObject *py_lower_name;
    PyObject *py_value;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "S:pk11_attribute_type_from_name", &py_name))
        return NULL;

    if ((py_lower_name = PyObject_CallMethod(py_name, "lower", NULL)) == NULL) {
        return NULL;
    }

    if ((py_value = PyDict_GetItem(cka_name_to_value, py_lower_name)) == NULL) {
	PyErr_Format(PyExc_KeyError, "attribute name not found: %s", PyString_AsString(py_name));
        Py_DECREF(py_lower_name);
        return NULL;
    }

    Py_DECREF(py_lower_name);
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
        Py_INCREF(args);
        parse_args = args;
    } else {
        parse_args = PyTuple_GetSlice(args, 0, n_base_args);
    }
    if (!PyArg_ParseTuple(parse_args, "k:get_best_slot", &mechanism)) {
        Py_DECREF(parse_args);
        return NULL;
    }
    Py_DECREF(parse_args);

    pin_args = PyTuple_GetSlice(args, n_base_args, argc);

    Py_BEGIN_ALLOW_THREADS
    if ((slot = PK11_GetBestSlot(mechanism, pin_args)) == NULL) {
	Py_BLOCK_THREADS
        Py_DECREF(pin_args);
        return set_nspr_error(NULL);
    }
    Py_END_ALLOW_THREADS

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
        Py_INCREF(args);
        parse_args = args;
    } else {
        parse_args = PyTuple_GetSlice(args, 0, n_base_args);
    }

    if (!PyArg_ParseTuple(parse_args, "O!kkkO!:import_sym_key",
                          &PK11SlotType, &py_slot,
                          &mechanism, &origin, &operation,
                          &SecItemType, &py_key_data)) {
        Py_DECREF(parse_args);
        return NULL;
    }
    Py_DECREF(parse_args);

    pin_args = PyTuple_GetSlice(args, n_base_args, argc);

    Py_BEGIN_ALLOW_THREADS
    if ((sym_key = PK11_ImportSymKey(py_slot->slot, mechanism, origin, operation,
                                     &py_key_data->item, pin_args)) == NULL) {
	Py_BLOCK_THREADS
        Py_DECREF(pin_args);
        return set_nspr_error(NULL);
    }
    Py_END_ALLOW_THREADS

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

PyDoc_STRVAR(pk11_import_crl_doc,
"import_crl(slot, der_crl, url, type, import_options, decode_options, [user_data1, ...]) -> SignedCRL\n\
\n\
:Parameters:\n\
    slot : PK11Slot object\n\
        designated PK11 slot\n\
    der_crl : SecItem object\n\
        signed DER CRL data encapsulated in a SecItem object.\n\
    url : string\n\
        URL of the CRL\n\
    type : int\n\
        revocation list type\n\
        \n\
        may be one of:\n\
          - SEC_CRL_TYPE\n\
          - SEC_KRL_TYPE\n\
        \n\
    import_options : int\n\
        bit-wise OR of the following flags:\n\
          - CRL_IMPORT_BYPASS_CHECKS\n\
        \n\
        or use CRL_IMPORT_DEFAULT_OPTIONS\n\
    decode_options : int\n\
        bit-wise OR of the following flags:\n\
          - CRL_DECODE_DONT_COPY_DER\n\
          - CRL_DECODE_SKIP_ENTRIES\n\
          - CRL_DECODE_KEEP_BAD_CRL\n\
          - CRL_DECODE_ADOPT_HEAP_DER\n\
        \n\
        or use CRL_DECODE_DEFAULT_OPTIONS\n\
    user_dataN : object\n\
        zero or more caller supplied parameters which will\n\
        be passed to the password callback function\n\
\n\
\n\
");
static PyObject *
pk11_import_crl(PyObject *self, PyObject *args)
{
    Py_ssize_t n_base_args = 6;
    Py_ssize_t argc;
    PyObject *parse_args = NULL;
    PyObject *pin_args = NULL;
    PK11Slot *py_slot;
    char *url;
    int type;
    int import_options;
    int decode_options;
    SecItem *py_der_signed_crl;
    CERTSignedCrl *signed_crl;

    TraceMethodEnter(self);

    argc = PyTuple_Size(args);
    if (argc == n_base_args) {
        Py_INCREF(args);
        parse_args = args;
    } else {
        parse_args = PyTuple_GetSlice(args, 0, n_base_args);
    }

    if (!PyArg_ParseTuple(parse_args, "O!O!siii:import_crl",
                          &PK11SlotType, &py_slot,
                          &SecItemType, &py_der_signed_crl,
                          &url, &type, &import_options, &decode_options)) {
        Py_DECREF(parse_args);
        return NULL;
    }
    Py_DECREF(parse_args);

    pin_args = PyTuple_GetSlice(args, n_base_args, argc);

    Py_BEGIN_ALLOW_THREADS
    if ((signed_crl = PK11_ImportCRL(py_slot->slot, &py_der_signed_crl->item, url,
                                     type, pin_args, import_options, NULL, decode_options)) == NULL) {
	Py_BLOCK_THREADS
        Py_DECREF(pin_args);
        return set_nspr_error(NULL);
    }
    Py_END_ALLOW_THREADS

    Py_DECREF(pin_args);

    return SignedCRL_new_from_CERTSignedCRL(signed_crl);
}

PyDoc_STRVAR(cert_decode_der_crl_doc,
"decode_der_crl(der_crl, type=SEC_CRL_TYPE, decode_options=CRL_DECODE_DEFAULT_OPTIONS) -> SignedCRL\n\
\n\
:Parameters:\n\
    der_crl : SecItem object\n\
        DER encoded CRL data encapsulated in a SECItem.\n\
    type : int\n\
        revocation list type\n\
        \n\
        may be one of:\n\
          - SEC_CRL_TYPE\n\
          - SEC_KRL_TYPE\n\
    decode_options : int\n\
        bit-wise OR of the following flags:\n\
          - CRL_DECODE_DONT_COPY_DER\n\
          - CRL_DECODE_SKIP_ENTRIES\n\
          - CRL_DECODE_KEEP_BAD_CRL\n\
          - CRL_DECODE_ADOPT_HEAP_DER\n\
        \n\
        or use CRL_DECODE_DEFAULT_OPTIONS\n\
\n\
");

static PyObject *
cert_decode_der_crl(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"der_crl", "type", "decode_options", NULL};
    SecItem *py_der_crl;
    int type = SEC_CRL_TYPE;
    int decode_options = CRL_DECODE_DEFAULT_OPTIONS;
    CERTSignedCrl *signed_crl;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!|ii:decode_der_crl", kwlist,
                                     &SecItemType, &py_der_crl,
                                     &py_der_crl, &type, &decode_options))
        return NULL;

    if ((signed_crl = CERT_DecodeDERCrlWithFlags(NULL, &py_der_crl->item, type,  decode_options)) == NULL) {
        return set_nspr_error(NULL);
    }

    return SignedCRL_new_from_CERTSignedCRL(signed_crl);
}

PyDoc_STRVAR(cert_read_der_from_file_doc,
"read_der_from_file(file, ascii=False) -> SecItem\n\
\n\
:Parameters:\n\
    file : file name or file object\n\
        If string treat as file path to open and read,\n\
        if file object read from file object.\n\
    ascii : boolean\n\
        If True treat file contents as ascii data.\n\
        If PEM delimiters are found strip them.\n\
        Then base64 decode the contents.\n\
\n\
Read the contents of a file and return as a SecItem object.\n\
If file is a string then treat it as a file pathname and open\n\
and read the contents of that file. If file is a file object\n\
then read the contents from the file object\n\
\n\
If the file contents begin with a PEM header then treat the\n\
the file as PEM encoded and decode the payload into DER form.\n\
Otherwise the file contents is assumed to already be in DER form.\n\
The returned SecItem contains the DER contents of the file.\n\
");

static PyObject *
cert_read_der_from_file(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"file", "ascii", NULL};
    PyObject *file_arg;
    int ascii = 0;
    PyObject *py_sec_item;
    PyObject *py_file, *py_file_contents;
    SECItem der;

    TraceMethodEnter(self);

    der.data = NULL;
    der.len = 0;
    der.type = siBuffer;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i:read_der_from_file", kwlist,
                                     &file_arg, &ascii))
        return NULL;

    if (PyString_Check(file_arg) || PyUnicode_Check(file_arg)) {
        if ((py_file = PyFile_FromString(PyString_AsString(file_arg), "r")) == NULL) {
            return NULL;
        }
    } else if (PyFile_Check(file_arg)) {
        py_file = file_arg;
	Py_INCREF(py_file);
    } else {
        PyErr_SetString(PyExc_TypeError, "Bad file, must be pathname or file object");
        return NULL;
    }

    if ((py_file_contents = PyObject_CallMethod(py_file, "read", "")) == NULL) {
        Py_DECREF(py_file);
        return NULL;
    }
    Py_DECREF(py_file);
    
    if (ascii) {
        char *p, *tmp, *der_begin, *der_end;

        p = PyString_AsString(py_file_contents);
	/* check for headers and trailers and remove them */
	if ((tmp = strstr(p, "-----BEGIN")) != NULL) {
	    p = tmp;
	    tmp = PORT_Strchr(p, '\n');
	    if (!tmp) {
		tmp = strchr(p, '\r'); /* maybe this is a MAC file */
            }
	    if (!tmp) {
                PyErr_SetString(PyExc_ValueError, "no line ending after PEM BEGIN");
                Py_DECREF(py_file_contents);
                return NULL;              
            }
            p = der_begin = tmp + 1;
            tmp = strstr(p, "-----END");
	    if (tmp != NULL) {
                der_end = tmp;
		*der_end = '\0';
	    } else {
                PyErr_SetString(PyExc_ValueError, "no PEM END found");
                Py_DECREF(py_file_contents);
                return NULL;
	    }
	} else {
	    der_begin = p;
            der_end = p + PyString_GET_SIZE(py_file_contents);
	}
     
	/* Convert to binary */
        if (NSSBase64_DecodeBuffer(NULL, &der, der_begin, der_end - der_begin) == NULL) {
            Py_DECREF(py_file_contents);
            return set_nspr_error("Could not base64 decode contents of file");
	}
        py_sec_item = SecItem_new_from_SECItem(&der, SECITEM_unknown);
        Py_DECREF(py_file_contents);
        SECITEM_FreeItem(&der, PR_FALSE);
        return py_sec_item;
    }
    
    der.data = (unsigned char *)PyString_AS_STRING(py_file_contents);
    der.len = PyString_GET_SIZE(py_file_contents);
    py_sec_item = SecItem_new_from_SECItem(&der, SECITEM_unknown);
    Py_DECREF(py_file_contents);
    return (PyObject *)py_sec_item;
}

PyDoc_STRVAR(cert_x509_key_usage_doc,
"x509_key_usage(bitstr, repr_kind=AsEnumDescription) -> (str, ...)\n\
\n\
:Parameters:\n\
    bitstr : SecItem object\n\
        A SecItem containing a DER encoded bit string.\n\
    repr_kind : RepresentationKind constant\n\
        Specifies what the contents of the returned tuple will be.\n\
        May be one of:\n\
\n\
        AsEnum\n\
            The enumerated constant.\n\
            (e.g. nss.KU_DIGITAL_SIGNATURE)\n\
        AsEnumDescription\n\
            A friendly human readable description of the enumerated constant as a string.\n\
             (e.g. \"Digital Signature\")\n\
        AsIndex\n\
            The bit position within the bit string.\n\
\n\
Return a tuple of string name for each enabled bit in the key\n\
usage bit string.\n\
");

static PyObject *
cert_x509_key_usage(PyObject *self, PyObject *args)
{
    PyObject *result;
    SecItem *py_sec_item;
    SECItem bitstr_item;
    int repr_kind = AsEnumDescription;

    if (!PyArg_ParseTuple(args, "O!|i:x509_key_usage", 
                          &SecItemType, &py_sec_item, &repr_kind))
        return NULL;

    if (der_bitstring_to_nss_bitstring(&bitstr_item, &py_sec_item->item) != SECSuccess) {
        return set_nspr_error(NULL);
    }

    result = key_usage_bitstr_to_tuple(&bitstr_item, repr_kind);

    return result;
}

PyDoc_STRVAR(cert_x509_ext_key_usage_doc,
"x509_ext_key_usage(sec_item, repr_kind=AsString) -> (obj, ...)\n\
\n\
:Parameters:\n\
    sec_item : SecItem object\n\
        A SecItem containing a DER encoded sequence of OID's\n\
    repr_kind : RepresentationKind constant\n\
        Specifies what the contents of the returned tuple will be.\n\
        May be one of:\n\
\n\
        AsObject\n\
            Each extended key usage will be a SecItem object embedding\n\
            the OID in DER format.\n\
        AsString\n\
            Each extended key usage will be a descriptive string.\n\
            (e.g. \"TLS Web Server Authentication Certificate\")\n\
        AsDottedDecimal\n\
            Each extended key usage will be OID rendered as a dotted decimal string.\n\
            (e.g. \"OID.1.3.6.1.5.5.7.3.1\")\n\
        AsEnum\n\
            Each extended key usage will be OID tag enumeration constant (int).\n\
            (e.g. nss.SEC_OID_EXT_KEY_USAGE_SERVER_AUTH)\n\
\n\
Return a tuple of OID's according the representation kind.\n\
");

static PyObject *
cert_x509_ext_key_usage(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"sec_item", "repr_kind", NULL};
    SecItem *py_sec_item;
    int repr_kind = AsString;

    if (!PyArg_ParseTupleAndKeywords(args, kwds,"O!|i:x509_ext_key_usage", kwlist,
                          &SecItemType, &py_sec_item, &repr_kind))
        return NULL;

    return decode_oid_sequence_to_tuple(&py_sec_item->item, repr_kind);
}


PyDoc_STRVAR(cert_x509_alt_name_doc,
"x509_alt_name(sec_item, repr_kind=AsString) -> (SecItem, ...)\n\
\n\
:Parameters:\n\
    sec_item : SecItem object\n\
        A SecItem containing a DER encoded alternative name extension.\n\
    repr_kind : RepresentationKind constant\n\
        Specifies what the contents of the returned tuple will be.\n\
        May be one of:\n\
\n\
        AsObject\n\
            The general name as a nss.GeneralName object\n\
        AsString\n\
            The general name as a string.\n\
            (e.g. \"http://crl.geotrust.com/crls/secureca.crl\")\n\
        AsTypeString\n\
            The general name type as a string.\n\
             (e.g. \"URI\")\n\
        AsTypeEnum\n\
            The general name type as a general name type enumerated constant.\n\
             (e.g. nss.certURI )\n\
        AsLabeledString\n\
            The general name as a string with it's type prepended.\n\
            (e.g. \"URI: http://crl.geotrust.com/crls/secureca.crl\"\n\
\n\
Return a tuple of GeneralNames according the representation kind.\n\
");

static PyObject *
cert_x509_alt_name(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"sec_item", "repr_kind", NULL};
    SecItem *py_sec_item;
    int repr_kind = AsString;
    CERTGeneralName *names;
    PLArenaPool *arena;
    PyObject *result;


    if (!PyArg_ParseTupleAndKeywords(args, kwds,"O!|i:x509_alt_name", kwlist,
                          &SecItemType, &py_sec_item, &repr_kind))
        return NULL;

    if ((arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE)) == NULL ) {
        return set_nspr_error(NULL);
    }

    if ((names = CERT_DecodeAltNameExtension(arena, &py_sec_item->item)) == NULL) {
        set_nspr_error(NULL);
        PORT_FreeArena(arena, PR_FALSE);
        return NULL;
    }

    result = CERTGeneralName_list_to_tuple(names, repr_kind);
    PORT_FreeArena(arena, PR_FALSE);
    return result;
}


static PyObject *
crl_reason_to_pystr(CERTCRLEntryReasonCode reason)
{
    PyObject *py_value;
    PyObject *py_name;

    if ((py_value = PyInt_FromLong(reason)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "unable to create object");
        return NULL;
    }

    if ((py_name = PyDict_GetItem(crl_reason_value_to_name, py_value)) == NULL) {
        Py_DECREF(py_value);
	PyErr_Format(PyExc_KeyError, "GeneralName reason name not found: %u", reason);
        return NULL;
    }

    Py_DECREF(py_value);
    Py_INCREF(py_name);

    return py_name;
}

PyDoc_STRVAR(cert_crl_reason_name_doc,
"crl_reason_name(reason) -> string\n\
\n\
:Parameters:\n\
    reason : int\n\
        CERTCRLEntryReasonCode constant\n\
\n\
Given a CERTCRLEntryReasonCode constant\n\
return it's name as a string\n\
");
static PyObject *
cert_crl_reason_name(PyObject *self, PyObject *args)
{
    unsigned long reason;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "k:crl_reason_name", &reason))
        return NULL;

    return crl_reason_to_pystr(reason);
}

PyDoc_STRVAR(cert_crl_reason_from_name_doc,
"crl_reason_from_name(name) -> int\n\
\n\
:Parameters:\n\
    name : string\n\
        name of CERTCRLEntryReasonCode constant\n\
\n\
Given the name of a CERTCRLEntryReasonCode constant\n\
return it's integer constant\n\
The string comparison is case insensitive.\n\
");
static PyObject *
cert_crl_reason_from_name(PyObject *self, PyObject *args)
{
    PyObject *py_name;
    PyObject *py_lower_name;
    PyObject *py_value;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "S:crl_reason_from_name", &py_name))
        return NULL;

    if ((py_lower_name = PyObject_CallMethod(py_name, "lower", NULL)) == NULL) {
        return NULL;
    }

    if ((py_value = PyDict_GetItem(crl_reason_name_to_value, py_lower_name)) == NULL) {
	PyErr_Format(PyExc_KeyError, "GeneralName reason name not found: %s", PyString_AsString(py_name));
        Py_DECREF(py_lower_name);
        return NULL;
    }

    Py_DECREF(py_lower_name);
    Py_INCREF(py_value);

    return py_value;
}

static PyObject *
general_name_type_to_pystr(CERTGeneralNameType type)
{
    PyObject *py_value;
    PyObject *py_name;

    if ((py_value = PyInt_FromLong(type)) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "unable to create object");
        return NULL;
    }

    if ((py_name = PyDict_GetItem(general_name_value_to_name, py_value)) == NULL) {
        Py_DECREF(py_value);
	PyErr_Format(PyExc_KeyError, "GeneralName type name not found: %u", type);
        return NULL;
    }

    Py_DECREF(py_value);
    Py_INCREF(py_name);

    return py_name;
}

PyDoc_STRVAR(cert_general_name_type_name_doc,
"general_name_type_name(type) -> string\n\
\n\
:Parameters:\n\
    type : int\n\
        CERTGeneralNameType constant\n\
\n\
Given a CERTGeneralNameType constant\n\
return it's name as a string\n\
");
static PyObject *
cert_general_name_type_name(PyObject *self, PyObject *args)
{
    unsigned long type;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "k:general_name_type_name", &type))
        return NULL;

    return general_name_type_to_pystr(type);
}

PyDoc_STRVAR(cert_general_name_type_from_name_doc,
"general_name_type_from_name(name) -> int\n\
\n\
:Parameters:\n\
    name : string\n\
        name of CERTGeneralNameType constant\n\
\n\
Given the name of a CERTGeneralNameType constant\n\
return it's integer constant\n\
The string comparison is case insensitive.\n\
");
static PyObject *
cert_general_name_type_from_name(PyObject *self, PyObject *args)
{
    PyObject *py_name;
    PyObject *py_lower_name;
    PyObject *py_value;

    TraceMethodEnter(self);

    if (!PyArg_ParseTuple(args, "S:general_name_type_from_name", &py_name))
        return NULL;

    if ((py_lower_name = PyObject_CallMethod(py_name, "lower", NULL)) == NULL) {
        return NULL;
    }

    if ((py_value = PyDict_GetItem(general_name_name_to_value, py_lower_name)) == NULL) {
	PyErr_Format(PyExc_KeyError, "GeneralName type name not found: %s", PyString_AsString(py_name));
        Py_DECREF(py_lower_name);
        return NULL;
    }

    Py_DECREF(py_lower_name);
    Py_INCREF(py_value);

    return py_value;
}

PyDoc_STRVAR(cert_cert_usage_flags_doc,
"cert_usage_str(flags) -> ['flag_name', ...]\n\
\n\
:Parameters:\n\
    flags : int\n\
        certificateUsage* bit flags\n\
\n\
Given an integer with certificateUsage*\n\
(e.g. nss.certificateUsageSSLServer) bit flags return a sorted\n\
list of their string names.\n\
");

static PyObject *
cert_cert_usage_flags(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"flags", NULL};
    int flags = 0;

    TraceMethodEnter(self);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i:cert_usage_flags", kwlist,
                                     &flags))
        return NULL;

    return cert_usage_flags(flags);
}

/* List of functions exported by this module. */
static PyMethodDef
module_methods[] = {
    {"nss_is_initialized",               (PyCFunction)nss_is_initialized,                  METH_NOARGS,                nss_is_initialized_doc},
    {"nss_init",                         (PyCFunction)nss_init,                            METH_VARARGS,               nss_init_doc},
    {"nss_init_nodb",                    (PyCFunction)nss_init_nodb,                       METH_NOARGS,                nss_init_nodb_doc},
    {"nss_shutdown",                     (PyCFunction)nss_shutdown,                        METH_NOARGS,                nss_shutdown_doc},
    {"set_password_callback",            (PyCFunction)pk11_set_password_callback,          METH_VARARGS,               pk11_set_password_callback_doc},
    {"find_cert_from_nickname",          (PyCFunction)pk11_find_cert_from_nickname,        METH_VARARGS,               pk11_find_cert_from_nickname_doc},
    {"find_key_by_any_cert",             (PyCFunction)pk11_find_key_by_any_cert,           METH_VARARGS,               pk11_find_key_by_any_cert_doc},
    {"generate_random",                  (PyCFunction)pk11_generate_random,                METH_VARARGS,               pk11_generate_random_doc},
    {"get_default_certdb",               (PyCFunction)cert_get_default_certdb,             METH_NOARGS,                cert_get_default_certdb_doc},
    {"get_cert_nicknames",               (PyCFunction)cert_get_cert_nicknames,             METH_VARARGS,               cert_get_cert_nicknames_doc},
    {"data_to_hex",                      (PyCFunction)cert_data_to_hex,                    METH_VARARGS|METH_KEYWORDS, cert_data_to_hex_doc},
    {"read_hex",                         (PyCFunction)read_hex,                            METH_VARARGS|METH_KEYWORDS, read_hex_doc},
    {"hash_buf",                         (PyCFunction)pk11_hash_buf,                       METH_VARARGS,               pk11_hash_buf_doc},
    {"md5_digest",                       (PyCFunction)pk11_md5_digest,                     METH_VARARGS,               pk11_md5_digest_doc},
    {"sha1_digest",                      (PyCFunction)pk11_sha1_digest,                    METH_VARARGS,               pk11_sha1_digest_doc},
    {"sha256_digest",                    (PyCFunction)pk11_sha256_digest,                  METH_VARARGS,               pk11_sha256_digest_doc},
    {"sha512_digest",                    (PyCFunction)pk11_sha512_digest,                  METH_VARARGS,               pk11_sha512_digest_doc},
    {"indented_format",                  (PyCFunction)nss_indented_format,                 METH_VARARGS|METH_KEYWORDS, nss_indented_format_doc},
    {"make_line_pairs",                  (PyCFunction)nss_make_line_pairs,                 METH_VARARGS|METH_KEYWORDS, nss_make_line_pairs_doc},
    {"der_universal_secitem_fmt_lines",  (PyCFunction)cert_der_universal_secitem_fmt_lines, METH_VARARGS|METH_KEYWORDS, cert_der_universal_secitem_fmt_lines_doc},
    {"oid_str",                          (PyCFunction)cert_oid_str,                        METH_VARARGS,               cert_oid_str_doc},
    {"oid_tag_name",                     (PyCFunction)cert_oid_tag_name,                   METH_VARARGS,               cert_oid_tag_name_doc},
    {"oid_tag",                          (PyCFunction)cert_oid_tag,                        METH_VARARGS,               cert_oid_tag_doc},
    {"oid_dotted_decimal",               (PyCFunction)cert_oid_dotted_decimal,             METH_VARARGS,               cert_oid_dotted_decimal_doc},
    {"key_mechanism_type_name",          (PyCFunction)pk11_key_mechanism_type_name,        METH_VARARGS,               pk11_key_mechanism_type_name_doc},
    {"key_mechanism_type_from_name",     (PyCFunction)pk11_key_mechanism_type_from_name,   METH_VARARGS,               pk11_key_mechanism_type_from_name_doc},
    {"pk11_attribute_type_name",         (PyCFunction)pk11_pk11_attribute_type_name,       METH_VARARGS,               pk11_attribute_type_name_doc},
    {"pk11_attribute_type_from_name",    (PyCFunction)pk11_pk11_attribute_type_from_name,  METH_VARARGS,               pk11_pk11_attribute_type_from_name_doc},
    {"cert_crl_reason_name",             (PyCFunction)cert_crl_reason_name,                METH_VARARGS,               cert_crl_reason_name_doc},
    {"cert_crl_reason_from_name",        (PyCFunction)cert_crl_reason_from_name,           METH_VARARGS,               cert_crl_reason_from_name_doc},
    {"cert_general_name_type_name",      (PyCFunction)cert_general_name_type_name,         METH_VARARGS,               cert_general_name_type_name_doc},
    {"cert_general_name_type_from_name", (PyCFunction)cert_general_name_type_from_name,    METH_VARARGS,               cert_general_name_type_from_name_doc},
    {"get_best_slot",                    (PyCFunction)pk11_get_best_slot,                  METH_VARARGS,               pk11_get_best_slot_doc},
    {"get_internal_key_slot",            (PyCFunction)pk11_get_internal_key_slot,          METH_NOARGS,                pk11_get_internal_key_slot_doc},
    {"create_context_by_sym_key",        (PyCFunction)pk11_create_context_by_sym_key,      METH_VARARGS|METH_KEYWORDS, pk11_create_context_by_sym_key_doc},
    {"import_sym_key",                   (PyCFunction)pk11_import_sym_key,                 METH_VARARGS,               pk11_import_sym_key_doc},
    {"create_digest_context",            (PyCFunction)pk11_create_digest_context,          METH_VARARGS,               pk11_create_digest_context_doc},
    {"param_from_iv",                    (PyCFunction)pk11_param_from_iv,                  METH_VARARGS|METH_KEYWORDS, pk11_param_from_iv_doc},
    {"param_from_algid",                 (PyCFunction)pk11_param_from_algid,               METH_VARARGS,               pk11_param_from_algid_doc},
    {"generate_new_param",               (PyCFunction)pk11_generate_new_param,             METH_VARARGS|METH_KEYWORDS, pk11_generate_new_param_doc},
    {"algtag_to_mechanism",              (PyCFunction)pk11_algtag_to_mechanism,            METH_VARARGS,               pk11_algtag_to_mechanism_doc},
    {"mechanism_to_algtag",              (PyCFunction)pk11_mechanism_to_algtag,            METH_VARARGS,               pk11_mechanism_to_algtag_doc},
    {"get_iv_length",                    (PyCFunction)pk11_get_iv_length,                  METH_VARARGS,               pk11_get_iv_length_doc},
    {"get_block_size",                   (PyCFunction)pk11_get_block_size,                 METH_VARARGS|METH_KEYWORDS, pk11_get_block_size_doc},
    {"get_pad_mechanism",                (PyCFunction)pk11_get_pad_mechanism,              METH_VARARGS,               pk11_get_pad_mechanism_doc},
    {"import_crl",                       (PyCFunction)pk11_import_crl,                     METH_VARARGS,               pk11_import_crl_doc},
    {"decode_der_crl",                   (PyCFunction)cert_decode_der_crl,                 METH_VARARGS|METH_KEYWORDS, cert_decode_der_crl_doc},
    {"read_der_from_file",               (PyCFunction)cert_read_der_from_file,             METH_VARARGS|METH_KEYWORDS, cert_read_der_from_file_doc},
    {"x509_key_usage",                   (PyCFunction)cert_x509_key_usage,                 METH_VARARGS,               cert_x509_key_usage_doc},
    {"x509_ext_key_usage",               (PyCFunction)cert_x509_ext_key_usage,             METH_VARARGS|METH_KEYWORDS, cert_x509_ext_key_usage_doc},
    {"x509_alt_name",                    (PyCFunction)cert_x509_alt_name,                  METH_VARARGS|METH_KEYWORDS, cert_x509_alt_name_doc},
    {"cert_usage_flags",                 (PyCFunction)cert_cert_usage_flags,               METH_VARARGS|METH_KEYWORDS, cert_cert_usage_flags_doc},
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
    TYPE_READY(CertificateExtensionType);
    TYPE_READY(CertificateType);
    TYPE_READY(PrivateKeyType);
    TYPE_READY(SignedCRLType);
    TYPE_READY(PK11SlotType);
    TYPE_READY(PK11SymKeyType);
    TYPE_READY(PK11ContextType);
    TYPE_READY(CRLDistributionPtType);
    TYPE_READY(CRLDistributionPtsType);
    TYPE_READY(AVAType);
    TYPE_READY(RDNType);
    TYPE_READY(DNType);
    TYPE_READY(GeneralNameType);
    TYPE_READY(AuthKeyIDType);
    TYPE_READY(BasicConstraintsType);
    TYPE_READY(CertificateRequestType);

    /* Export C API */
    if (PyModule_AddObject(m, "_C_API", PyCObject_FromVoidPtr((void *)&nspr_nss_c_api, NULL)) != 0) {
        return;
    }

    AddIntConstant(OCTETS_PER_LINE_DEFAULT);
    PyModule_AddStringMacro(m, HEX_SEPARATOR_DEFAULT);

    AddIntConstant(AsObject);
    AddIntConstant(AsString);
    AddIntConstant(AsTypeString);
    AddIntConstant(AsTypeEnum);
    AddIntConstant(AsLabeledString);
    AddIntConstant(AsEnum);
    AddIntConstant(AsEnumName);
    AddIntConstant(AsEnumDescription);
    AddIntConstant(AsIndex);
    AddIntConstant(AsDottedDecimal);

    AddIntConstant(generalName);
    AddIntConstant(relativeDistinguishedName);


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

    AddIntConstant(SEC_CRL_TYPE);
    AddIntConstant(SEC_KRL_TYPE);

    AddIntConstant(CRL_DECODE_DEFAULT_OPTIONS);
    AddIntConstant(CRL_DECODE_DONT_COPY_DER);
    AddIntConstant(CRL_DECODE_SKIP_ENTRIES);
    AddIntConstant(CRL_DECODE_KEEP_BAD_CRL);
    AddIntConstant(CRL_DECODE_ADOPT_HEAP_DER);

    AddIntConstant(CRL_IMPORT_DEFAULT_OPTIONS);
    AddIntConstant(CRL_IMPORT_BYPASS_CHECKS);


    AddIntConstant(secCertTimeValid);
    AddIntConstant(secCertTimeExpired);
    AddIntConstant(secCertTimeNotValidYet);


    /***************************************************************************
     * CRL Reason
     ***************************************************************************/

    if ((crl_reason_name_to_value = PyDict_New()) == NULL) {
        return;
    }
    if ((crl_reason_value_to_name = PyDict_New()) == NULL) {
        return;
    }

#define ExportConstant(constant)                      \
if (_AddIntConstantWithLookup(m, #constant, constant, \
    "crlEntry", crl_reason_name_to_value, crl_reason_value_to_name) < 0) return;

    ExportConstant(crlEntryReasonUnspecified);
    ExportConstant(crlEntryReasonKeyCompromise);
    ExportConstant(crlEntryReasonCaCompromise);
    ExportConstant(crlEntryReasonAffiliationChanged);
    ExportConstant(crlEntryReasonSuperseded);
    ExportConstant(crlEntryReasonCessationOfOperation);
    ExportConstant(crlEntryReasoncertificatedHold);
    ExportConstant(crlEntryReasonRemoveFromCRL);
    ExportConstant(crlEntryReasonPrivilegeWithdrawn);
    ExportConstant(crlEntryReasonAaCompromise);

#undef ExportConstant

    /***************************************************************************
     * General Name Types
     ***************************************************************************/

    if ((general_name_name_to_value = PyDict_New()) == NULL) {
        return;
    }
    if ((general_name_value_to_name = PyDict_New()) == NULL) {
        return;
    }

#define ExportConstant(constant)                      \
if (_AddIntConstantWithLookup(m, #constant, constant, \
    "cert", general_name_name_to_value, general_name_value_to_name) < 0) return;

    ExportConstant(certOtherName);
    ExportConstant(certRFC822Name);
    ExportConstant(certDNSName);
    ExportConstant(certX400Address);
    ExportConstant(certDirectoryName);
    ExportConstant(certEDIPartyName);
    ExportConstant(certURI);
    ExportConstant(certIPAddress);
    ExportConstant(certRegisterID);

#undef ExportConstant

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

#if defined(CKM_SEED_KEY_GEN)
    ExportConstant(CKM_SEED_KEY_GEN);
    ExportConstant(CKM_SEED_ECB);
    ExportConstant(CKM_SEED_CBC);
    ExportConstant(CKM_SEED_MAC);
    ExportConstant(CKM_SEED_MAC_GENERAL);
    ExportConstant(CKM_SEED_CBC_PAD);
    ExportConstant(CKM_SEED_ECB_ENCRYPT_DATA);
    ExportConstant(CKM_SEED_CBC_ENCRYPT_DATA);
#endif

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

#if defined(SEC_OID_SEED_CBC)
    ExportConstant(SEC_OID_SEED_CBC);
#endif

#if defined(SEC_OID_X509_ANY_POLICY)
    ExportConstant(SEC_OID_X509_ANY_POLICY);
#endif

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

