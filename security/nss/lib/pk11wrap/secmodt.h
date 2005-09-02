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
 * The Original Code is the Netscape security libraries.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1994-2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#ifndef _SECMODT_H_
#define _SECMODT_H_ 1

#include "nssrwlkt.h"
#include "nssilckt.h"
#include "secoid.h"
#include "secasn1.h"

/* find a better home for these... */
extern const SEC_ASN1Template SECKEY_PointerToEncryptedPrivateKeyInfoTemplate[];
extern SEC_ASN1TemplateChooser NSS_Get_SECKEY_PointerToEncryptedPrivateKeyInfoTemplate;
extern const SEC_ASN1Template SECKEY_EncryptedPrivateKeyInfoTemplate[];
extern SEC_ASN1TemplateChooser NSS_Get_SECKEY_EncryptedPrivateKeyInfoTemplate;
extern const SEC_ASN1Template SECKEY_PrivateKeyInfoTemplate[];
extern SEC_ASN1TemplateChooser NSS_Get_SECKEY_PrivateKeyInfoTemplate;
extern const SEC_ASN1Template SECKEY_PointerToPrivateKeyInfoTemplate[];
extern SEC_ASN1TemplateChooser NSS_Get_SECKEY_PointerToPrivateKeyInfoTemplate;

/* PKCS11 needs to be included */
typedef struct SECMODModuleStr SECMODModule;
typedef struct SECMODModuleListStr SECMODModuleList;
typedef NSSRWLock SECMODListLock;
typedef struct PK11SlotInfoStr PK11SlotInfo; /* defined in secmodti.h */
typedef struct PK11PreSlotInfoStr PK11PreSlotInfo; /* defined in secmodti.h */
typedef struct PK11SymKeyStr PK11SymKey; /* defined in secmodti.h */
typedef struct PK11ContextStr PK11Context; /* defined in secmodti.h */
typedef struct PK11SlotListStr PK11SlotList;
typedef struct PK11SlotListElementStr PK11SlotListElement;
typedef struct PK11RSAGenParamsStr PK11RSAGenParams;
typedef unsigned long SECMODModuleID;
typedef struct PK11DefaultArrayEntryStr PK11DefaultArrayEntry;
typedef struct PK11GenericObjectStr PK11GenericObject;

struct SECMODModuleStr {
    PRArenaPool	*arena;
    PRBool	internal;	/* true of internally linked modules, false
				 * for the loaded modules */
    PRBool	loaded;		/* Set to true if module has been loaded */
    PRBool	isFIPS;		/* Set to true if module is finst internal */
    char	*dllName;	/* name of the shared library which implements
				 * this module */
    char	*commonName;	/* name of the module to display to the user */
    void	*library;	/* pointer to the library. opaque. used only by
				 * pk11load.c */
    void	*functionList; /* The PKCS #11 function table */
    PZLock	*refLock;	/* only used pk11db.c */
    int		refCount;	/* Module reference count */
    PK11SlotInfo **slots;	/* array of slot points attached to this mod*/
    int		slotCount;	/* count of slot in above array */
    PK11PreSlotInfo *slotInfo;	/* special info about slots default settings */
    int		slotInfoCount;  /* count */
    SECMODModuleID moduleID;	/* ID so we can find this module again */
    PRBool	isThreadSafe;
    unsigned long ssl[2];	/* SSL cipher enable flags */
    char	*libraryParams;  /* Module specific parameters */
    void *moduleDBFunc; /* function to return module configuration data*/
    SECMODModule *parent;	/* module that loaded us */
    PRBool	isCritical;	/* This module must load successfully */
    PRBool	isModuleDB;	/* this module has lists of PKCS #11 modules */
    PRBool	moduleDBOnly;	/* this module only has lists of PKCS #11 modules */
    int		trustOrder;	/* order for this module's certificate trust rollup */
    int		cipherOrder;	/* order for cipher operations */
    unsigned long evControlMask; /* control the running and shutdown of slot
				  * events (SECMOD_WaitForAnyTokenEvent) */
};

/* evControlMask flags */
/*
 * These bits tell the current state of a SECMOD_WaitForAnyTokenEvent.
 *
 * SECMOD_WAIT_PKCS11_EVENT - we're waiting in the PKCS #11 module in
 *  C_WaitForSlotEvent().
 * SECMOD_WAIT_SIMULATED_EVENT - we're waiting in the NSS simulation code
 *  which polls for token insertion and removal events.
 * SECMOD_END_WAIT - SECMOD_CancelWait has been called while the module is
 *  waiting in SECMOD_WaitForAnyTokenEvent. SECMOD_WaitForAnyTokenEvent
 *  should return immediately to it's caller.
 */ 
#define SECMOD_END_WAIT 	    0x01
#define SECMOD_WAIT_SIMULATED_EVENT 0x02 
#define SECMOD_WAIT_PKCS11_EVENT    0x04

struct SECMODModuleListStr {
    SECMODModuleList	*next;
    SECMODModule	*module;
};

struct PK11SlotListStr {
    PK11SlotListElement *head;
    PK11SlotListElement *tail;
    PZLock *lock;
};

struct PK11SlotListElementStr {
    PK11SlotListElement *next;
    PK11SlotListElement *prev;
    PK11SlotInfo *slot;
    int refCount;
};

struct PK11RSAGenParamsStr {
    int keySizeInBits;
    unsigned long pe;
};

typedef enum {
     PK11CertListUnique = 0,     /* get one instance of all certs */
     PK11CertListUser = 1,       /* get all instances of user certs */
     PK11CertListRootUnique = 2, /* get one instance of CA certs without a private key.
                                  * deprecated. Use PK11CertListCAUnique
                                  */
     PK11CertListCA = 3,         /* get all instances of CA certs */
     PK11CertListCAUnique = 4,   /* get one instance of CA certs */
     PK11CertListUserUnique = 5, /* get one instance of user certs */
     PK11CertListAll = 6         /* get all instances of all certs */
} PK11CertListType;

/*
 * Entry into the Array which lists all the legal bits for the default flags
 * in the slot, their definition, and the PKCS #11 mechanism the represent
 * Always Statically allocated. 
 */
struct PK11DefaultArrayEntryStr {
    char *name;
    unsigned long flag;
    unsigned long mechanism; /* this is a long so we don't include the 
			      * whole pkcs 11 world to use this header */
};


#define SECMOD_RSA_FLAG 	0x00000001L
#define SECMOD_DSA_FLAG 	0x00000002L
#define SECMOD_RC2_FLAG 	0x00000004L
#define SECMOD_RC4_FLAG 	0x00000008L
#define SECMOD_DES_FLAG 	0x00000010L
#define SECMOD_DH_FLAG	 	0x00000020L
#define SECMOD_FORTEZZA_FLAG	0x00000040L
#define SECMOD_RC5_FLAG		0x00000080L
#define SECMOD_SHA1_FLAG	0x00000100L
#define SECMOD_MD5_FLAG		0x00000200L
#define SECMOD_MD2_FLAG		0x00000400L
#define SECMOD_SSL_FLAG		0x00000800L
#define SECMOD_TLS_FLAG		0x00001000L
#define SECMOD_AES_FLAG 	0x00002000L
#define SECMOD_SHA256_FLAG	0x00004000L
#define SECMOD_SHA512_FLAG	0x00008000L	/* also for SHA384 */
/* reserved bit for future, do not use */
#define SECMOD_RESERVED_FLAG    0X08000000L
#define SECMOD_FRIENDLY_FLAG	0x10000000L
#define SECMOD_RANDOM_FLAG	0x80000000L

/* need to make SECMOD and PK11 prefixes consistant. */
#define PK11_OWN_PW_DEFAULTS 0x20000000L
#define PK11_DISABLE_FLAG    0x40000000L

/* FAKE PKCS #11 defines */
#define CKM_FAKE_RANDOM       0x80000efeL
#define CKM_INVALID_MECHANISM 0xffffffffL
#define CKA_DIGEST            0x81000000L
#define CKA_FLAGS_ONLY        0 /* CKA_CLASS */

/*
 * PK11AttrFlags
 *
 * A 32-bit bitmask of PK11_ATTR_XXX flags
 */
typedef PRUint32 PK11AttrFlags;

/*
 * PK11_ATTR_XXX
 *
 * The following PK11_ATTR_XXX bitflags are used to specify
 * PKCS #11 object attributes that have Boolean values.  Some NSS
 * functions have a "PK11AttrFlags attrFlags" parameter whose value
 * is the logical OR of these bitflags.  NSS use these bitflags on
 * private keys or secret keys.  Some of these bitflags also apply
 * to the public keys associated with the private keys.
 *
 * Some of these PKCS #11 object attributes have a token-specific
 * default value.  For such attributes, we need two bitflags to
 * specify not only "true" and "false" but also "default".  For
 * example, PK11_ATTR_PRIVATE and PK11_ATTR_PUBLIC control the
 * CKA_PRIVATE attribute.  If PK11_ATTR_PRIVATE is set, we add
 *     { CKA_PRIVATE, &cktrue, sizeof(CK_BBOOL) }
 * to the template.  If PK11_ATTR_PUBLIC is set, we add
 *     { CKA_PRIVATE, &ckfalse, sizeof(CK_BBOOL) }
 * to the template.  If neither flag is set, we don't add any
 * CKA_PRIVATE entry to the template.
 */

/*
 * Attributes for PKCS #11 storage objects, which include not only
 * keys but also certificates and domain parameters.
 */

/*
 * PK11_ATTR_TOKEN
 *
 * If this flag is set, the object is a token object.  If this
 * flag is not set, the object is *by default* a session object.
 * This flag specifies the value of the PKCS #11 CKA_TOKEN
 * attribute.
 */
#define PK11_ATTR_TOKEN         0x00000001L
/*      Reserved                0x00000002L */

/*
 * PK11_ATTR_PRIVATE
 * PK11_ATTR_PUBLIC
 *
 * These two flags determine whether the object is a private or
 * public object.  A user may not access a private object until the
 * user has authenticated to the token.
 *
 * These two flags are related and cannot both be set.
 * If the PK11_ATTR_PRIVATE flag is set, the object is a private
 * object.  If the PK11_ATTR_PUBLIC flag is set, the object is a
 * public object.  If neither flag is set, it is token-specific
 * whether the object is private or public.
 *
 * These two flags specify the value of the PKCS #11 CKA_PRIVATE
 * attribute.  NSS only uses this attribute on private and secret
 * keys, so public keys created by NSS get the token-specific
 * default value of the CKA_PRIVATE attribute.
 */
#define PK11_ATTR_PRIVATE       0x00000004L
#define PK11_ATTR_PUBLIC        0x00000008L

/*
 * PK11_ATTR_READONLY
 *
 * If this flag is set, the object is read-only.  If this flag is
 * not set, the object is *by default* modifiable.
 *
 * This flag specifies the value of the PKCS #11 CKA_MODIFIABLE
 * attribute.
 *
 * XXX Should we name this flag PK11_ATTR_UNMODIFIABLE?
 */
/*      Reserved                0x00000010L */
#define PK11_ATTR_READONLY      0x00000020L

/* Attributes for PKCS #11 key objects. */

/*
 * PK11_ATTR_SENSITIVE
 * PK11_ATTR_INSENSITIVE
 *
 * These two flags are related and cannot both be set.
 * If the PK11_ATTR_SENSITIVE flag is set, the key is sensitive.
 * If the PK11_ATTR_INSENSITIVE flag is set, the key is not
 * sensitive.  If neither flag is set, it is token-specific whether
 * the key is sensitive or not.
 *
 * If a key is sensitive, certain attributes of the key cannot be
 * revealed in plaintext outside the token.
 *
 * This flag specifies the value of the PKCS #11 CKA_SENSITIVE
 * attribute.  Although the default value of the CKA_SENSITIVE
 * attribute for secret keys is CK_FALSE per PKCS #11, some FIPS
 * tokens set the default value to CK_TRUE because only CK_TRUE
 * is allowed.  So in practice the default value of this attribute
 * is token-specific, hence the need for two bitflags.
 */
#define PK11_ATTR_SENSITIVE     0x00000040L
#define PK11_ATTR_INSENSITIVE   0x00000080L

/*
 * PK11_ATTR_EXTRACTABLE
 * PK11_ATTR_UNEXTRACTABLE
 *
 * These two flags are related and cannot both be set.
 * If the PK11_ATTR_EXTRACTABLE flag is set, the key is extractable
 * and can be wrapped.  If the PK11_ATTR_UNEXTRACTABLE flag is set,
 * the key is not extractable, and certain attributes of the key
 * cannot be revealed in plaintext outside the token (just like a
 * sensitive key).  If neither flag is set, it is token-specific
 * whether the key is extractable or not.
 *
 * These two flags specify the value of the PKCS #11 CKA_EXTRACTABLE
 * attribute.
 */
#define PK11_ATTR_EXTRACTABLE   0x00000100L
#define PK11_ATTR_UNEXTRACTABLE 0x00000200L

/* Cryptographic module types */
#define SECMOD_EXTERNAL	0	/* external module */
#define SECMOD_INTERNAL 1	/* internal default module */
#define SECMOD_FIPS	2	/* internal fips module */

/* default module configuration strings */
#define SECMOD_SLOT_FLAGS "slotFlags=[RSA,DSA,DH,RC2,RC4,DES,RANDOM,SHA1,MD5,MD2,SSL,TLS,AES,SHA256,SHA512]"

#define SECMOD_MAKE_NSS_FLAGS(fips,slot) \
"Flags=internal,critical"fips" slotparams=("#slot"={"SECMOD_SLOT_FLAGS"})"

#define SECMOD_INT_NAME "NSS Internal PKCS #11 Module"
#define SECMOD_INT_FLAGS SECMOD_MAKE_NSS_FLAGS("",1)
#define SECMOD_FIPS_NAME "NSS Internal FIPS PKCS #11 Module"
#define SECMOD_FIPS_FLAGS SECMOD_MAKE_NSS_FLAGS(",fips",3)

/*
 * What is the origin of a given Key. Normally this doesn't matter, but
 * the fortezza code needs to know if it needs to invoke the SSL3 fortezza
 * hack.
 */
typedef enum {
    PK11_OriginNULL = 0,	/* There is not key, it's a null SymKey */
    PK11_OriginDerive = 1,	/* Key was derived from some other key */
    PK11_OriginGenerated = 2,	/* Key was generated (also PBE keys) */
    PK11_OriginFortezzaHack = 3,/* Key was marked for fortezza hack */
    PK11_OriginUnwrap = 4	/* Key was unwrapped or decrypted */
} PK11Origin;

/* PKCS #11 disable reasons */
typedef enum {
    PK11_DIS_NONE = 0,
    PK11_DIS_USER_SELECTED = 1,
    PK11_DIS_COULD_NOT_INIT_TOKEN = 2,
    PK11_DIS_TOKEN_VERIFY_FAILED = 3,
    PK11_DIS_TOKEN_NOT_PRESENT = 4
} PK11DisableReasons;

/* types of PKCS #11 objects */
typedef enum {
   PK11_TypeGeneric = 0,
   PK11_TypePrivKey = 1,
   PK11_TypePubKey = 2,
   PK11_TypeCert = 3,
   PK11_TypeSymKey = 4
} PK11ObjectType;



/* function pointer type for password callback function.
 * This type is passed in to PK11_SetPasswordFunc() 
 */
typedef char *(PR_CALLBACK *PK11PasswordFunc)(PK11SlotInfo *slot, PRBool retry, void *arg);
typedef PRBool (PR_CALLBACK *PK11VerifyPasswordFunc)(PK11SlotInfo *slot, void *arg);
typedef PRBool (PR_CALLBACK *PK11IsLoggedInFunc)(PK11SlotInfo *slot, void *arg);

/*
 * PKCS #11 key structures
 */

/*
** Attributes
*/
struct SECKEYAttributeStr {
    SECItem attrType;
    SECItem **attrValue;
};
typedef struct SECKEYAttributeStr SECKEYAttribute;

/*
** A PKCS#8 private key info object
*/
struct SECKEYPrivateKeyInfoStr {
    PLArenaPool *arena;
    SECItem version;
    SECAlgorithmID algorithm;
    SECItem privateKey;
    SECKEYAttribute **attributes;
};
typedef struct SECKEYPrivateKeyInfoStr SECKEYPrivateKeyInfo;

/*
** A PKCS#8 private key info object
*/
struct SECKEYEncryptedPrivateKeyInfoStr {
    PLArenaPool *arena;
    SECAlgorithmID algorithm;
    SECItem encryptedData;
};
typedef struct SECKEYEncryptedPrivateKeyInfoStr SECKEYEncryptedPrivateKeyInfo;

/*
 * token removal detection
 */
typedef enum {
   PK11TokenNotRemovable = 0,
   PK11TokenPresent = 1,
   PK11TokenChanged = 2,
   PK11TokenRemoved = 3
} PK11TokenStatus;

typedef enum {
   PK11TokenRemovedOrChangedEvent = 0,
   PK11TokenPresentEvent = 1
} PK11TokenEvent;

/*
 * CRL Import Flags
 */
#define CRL_IMPORT_DEFAULT_OPTIONS 0x00000000
#define CRL_IMPORT_BYPASS_CHECKS   0x00000001

#endif /*_SECMODT_H_ */
