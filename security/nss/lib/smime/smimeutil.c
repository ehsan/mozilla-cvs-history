/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is the Netscape security libraries.
 * 
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are 
 * Copyright (C) 1994-2000 Netscape Communications Corporation.  All
 * Rights Reserved.
 * 
 * Contributor(s):
 * 
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU General Public License Version 2 or later (the
 * "GPL"), in which case the provisions of the GPL are applicable 
 * instead of those above.  If you wish to allow use of your 
 * version of this file only under the terms of the GPL and not to
 * allow others to use your version of this file under the MPL,
 * indicate your decision by deleting the provisions above and
 * replace them with the notice and other provisions required by
 * the GPL.  If you do not delete the provisions above, a recipient
 * may use your version of this file under either the MPL or the
 * GPL.
 */

/*
 * Stuff specific to S/MIME policy and interoperability.
 *
 * $Id: smimeutil.c,v 1.2 2000/06/13 21:56:34 chrisk%netscape.com Exp $
 */

#include "secmime.h"
#include "secoid.h"
#include "pk11func.h"
#include "ciferfam.h"	/* for CIPHER_FAMILY symbols */
#include "secasn1.h"
#include "secitem.h"
#include "cert.h"
#include "key.h"
#include "secerr.h"

/* various integer's ASN.1 encoding */
static unsigned char asn1_int40[] = { SEC_ASN1_INTEGER, 0x01, 0x28 };
static unsigned char asn1_int64[] = { SEC_ASN1_INTEGER, 0x01, 0x40 };
static unsigned char asn1_int128[] = { SEC_ASN1_INTEGER, 0x02, 0x00, 0x80 };

/* RC2 algorithm parameters (used in smime_cipher_map) */
static SECItem param_int40 = { siBuffer, asn1_int40, sizeof(asn1_int40) };
static SECItem param_int64 = { siBuffer, asn1_int64, sizeof(asn1_int64) };
static SECItem param_int128 = { siBuffer, asn1_int128, sizeof(asn1_int128) };

/*
 * XXX Would like the "parameters" field to be a SECItem *, but the
 * encoder is having trouble with optional pointers to an ANY.  Maybe
 * once that is fixed, can change this back...
 */
typedef struct {
    SECItem capabilityID;
    SECItem parameters;
    long cipher;		/* optimization */
} NSSSMIMECapability;

static const SEC_ASN1Template smime_capability_template[] = {
    { SEC_ASN1_SEQUENCE,
	  0, NULL, sizeof(NSSSMIMECapability) },
    { SEC_ASN1_OBJECT_ID,
	  offsetof(NSSSMIMECapability,capabilityID), },
    { SEC_ASN1_OPTIONAL | SEC_ASN1_ANY,
	  offsetof(NSSSMIMECapability,parameters), },
    { 0, }
};

static const SEC_ASN1Template smime_capabilities_template[] = {
    { SEC_ASN1_SEQUENCE_OF, 0, smime_capability_template }
};

/* smime_cipher_map - map of SMIME symmetric "ciphers" to algtag & parameters */
typedef struct {
    unsigned long cipher;
    SECOidTag algtag;
    SECItem *parms;
    PRBool enabled;	/* in the user's preferences */
    PRBool allowed;	/* per export policy */
} smime_cipher_map_entry;

/* global: list of supported SMIME symmetric ciphers, ordered roughly by increasing strength */
static smime_cipher_map_entry smime_cipher_map[] = {
/*    cipher			algtag			parms		enabled  allowed */
/*    ---------------------------------------------------------------------------------- */
    { SMIME_RC2_CBC_40,		SEC_OID_RC2_CBC,	&param_int40,	PR_TRUE, PR_TRUE },
    { SMIME_DES_CBC_56,		SEC_OID_DES_CBC,	NULL,		PR_TRUE, PR_TRUE },
    { SMIME_RC2_CBC_64,		SEC_OID_RC2_CBC,	&param_int64,	PR_TRUE, PR_TRUE },
    { SMIME_RC2_CBC_128,	SEC_OID_RC2_CBC,	&param_int128,	PR_TRUE, PR_TRUE },
    { SMIME_DES_EDE3_168,	SEC_OID_DES_EDE3_CBC,	NULL,		PR_TRUE, PR_TRUE },
    { SMIME_FORTEZZA,		SEC_OID_FORTEZZA_SKIPJACK, NULL,	PR_TRUE, PR_TRUE }
};
static const int smime_cipher_map_count = sizeof(smime_cipher_map) / sizeof(smime_cipher_map_entry);

/* the other global variables */
static PRBool smime_prefs_changed = PR_TRUE;
static NSSSMIMECapability **smime_capabilities;
static SECItem *smime_encoded_caps;
static PRBool lastUsedFortezza;

/*
 * smime_mapi_by_cipher - find index into smime_cipher_map by cipher
 */
static int
smime_mapi_by_cipher(unsigned long cipher)
{
    int i;

    for (i = 0; i < smime_cipher_map_count; i++) {
	if (smime_cipher_map[i].cipher == cipher)
	    return i;	/* bingo */
    }
    return -1;		/* should not happen if we're consistent, right? */
}

/*
 * NSS_SMIME_EnableCipher - this function locally records the user's preference
 */
SECStatus 
NSS_SMIMEUtil_EnableCipher(unsigned long which, PRBool on)
{
    unsigned long mask;
    int mapi;

    mask = which & CIPHER_FAMILYID_MASK;

    PORT_Assert (mask == CIPHER_FAMILYID_SMIME);
    if (mask != CIPHER_FAMILYID_SMIME)
	/* XXX set an error! */
    	return SECFailure;

    mapi = smime_mapi_by_cipher(which);
    if (mapi < 0)
	/* XXX set an error */
	return SECFailure;

    /* do we try to turn on a forbidden cipher? */
    if (!smime_cipher_map[mapi].allowed && on) {
	PORT_SetError (SEC_ERROR_BAD_EXPORT_ALGORITHM);
	return SECFailure;
    }

    if (smime_cipher_map[mapi].enabled != on) {
	smime_cipher_map[mapi].enabled = on;
	smime_prefs_changed = PR_TRUE;
    }
    return SECSuccess;
}


/*
 * this function locally records the export policy
 */
SECStatus 
NSS_SMIMEUtil_AllowCipher(unsigned long which, PRBool on)
{
    unsigned long mask;
    int mapi;

    mask = which & CIPHER_FAMILYID_MASK;

    PORT_Assert (mask == CIPHER_FAMILYID_SMIME);
    if (mask != CIPHER_FAMILYID_SMIME)
	/* XXX set an error! */
    	return SECFailure;

    mapi = smime_mapi_by_cipher(which);
    if (mapi < 0)
	/* XXX set an error */
	return SECFailure;

    if (smime_cipher_map[mapi].allowed != on) {
	smime_cipher_map[mapi].allowed = on;
	smime_prefs_changed = PR_TRUE;
    }
    return SECSuccess;
}

/*
 * Based on the given algorithm (including its parameters, in some cases!)
 * and the given key (may or may not be inspected, depending on the
 * algorithm), find the appropriate policy algorithm specification
 * and return it.  If no match can be made, -1 is returned.
 */
static SECStatus
nss_smime_get_cipher_for_alg_and_key(SECAlgorithmID *algid, PK11SymKey *key, unsigned long *cipher)
{
    SECOidTag algtag;
    unsigned int keylen_bits;
    SECStatus rv = SECSuccess;
    unsigned long c;

    algtag = SECOID_GetAlgorithmTag(algid);
    switch (algtag) {
    case SEC_OID_RC2_CBC:
	keylen_bits = PK11_GetKeyStrength(key, algid);
	switch (keylen_bits) {
	case 40:
	    c = SMIME_RC2_CBC_40;
	case 64:
	    c = SMIME_RC2_CBC_64;
	case 128:
	    c = SMIME_RC2_CBC_128;
	default:
	    rv = SECFailure;
	    break;
	}
	break;
    case SEC_OID_DES_CBC:
	c = SMIME_DES_CBC_56;
    case SEC_OID_DES_EDE3_CBC:
	c = SMIME_DES_EDE3_168;
    case SEC_OID_FORTEZZA_SKIPJACK:
	c = SMIME_FORTEZZA;
    default:
	rv = SECFailure;
    }
    if (rv == SECSuccess)
	*cipher = c;
    return rv;
}

static PRBool
nss_smime_cipher_allowed(unsigned long which)
{
    int mapi;

    mapi = smime_mapi_by_cipher(which);
    if (mapi < 0)
	return PR_FALSE;
    return smime_cipher_map[mapi].allowed;
}

PRBool
NSS_SMIMEUtil_DecryptionAllowed(SECAlgorithmID *algid, PK11SymKey *key)
{
    unsigned long which;

    if (nss_smime_get_cipher_for_alg_and_key(algid, key, &which) != SECSuccess)
	return PR_FALSE;

    return nss_smime_cipher_allowed(which);
}


/*
 * NSS_SMIME_EncryptionPossible - check if any encryption is allowed
 *
 * This tells whether or not *any* S/MIME encryption can be done,
 * according to policy.  Callers may use this to do nicer user interface
 * (say, greying out a checkbox so a user does not even try to encrypt
 * a message when they are not allowed to) or for any reason they want
 * to check whether S/MIME encryption (or decryption, for that matter)
 * may be done.
 *
 * It takes no arguments.  The return value is a simple boolean:
 *   PR_TRUE means encryption (or decryption) is *possible*
 *	(but may still fail due to other reasons, like because we cannot
 *	find all the necessary certs, etc.; PR_TRUE is *not* a guarantee)
 *   PR_FALSE means encryption (or decryption) is not permitted
 *
 * There are no errors from this routine.
 */
PRBool
NSS_SMIMEUtil_EncryptionPossible(void)
{
    int i;

    for (i = 0; i < smime_cipher_map_count; i++) {
	if (smime_cipher_map[i].allowed)
	    return PR_TRUE;
    }
    return PR_FALSE;
}


static int
nss_SMIME_FindCipherForSMIMECap(NSSSMIMECapability *cap)
{
    int i;
    SECOidTag capIDTag;

    /* we need the OIDTag here */
    capIDTag = SECOID_FindOIDTag(&(cap->capabilityID));

    /* go over all the SMIME ciphers we know and see if we find a match */
    for (i = 0; i < smime_cipher_map_count; i++) {
	if (smime_cipher_map[i].algtag != capIDTag)
	    continue;
	/*
	 * XXX If SECITEM_CompareItem allowed NULLs as arguments (comparing
	 * 2 NULLs as equal and NULL and non-NULL as not equal), we could
	 * use that here instead of all of the following comparison code.
	 */
	if (cap->parameters.data == NULL && smime_cipher_map[i].parms == NULL)
	    break;	/* both empty: bingo */

	if (cap->parameters.data != NULL && smime_cipher_map[i].parms != NULL &&
	    cap->parameters.len == smime_cipher_map[i].parms->len &&
	    PORT_Memcmp (cap->parameters.data, smime_cipher_map[i].parms->data,
			     cap->parameters.len) == 0)
	{
	    break;	/* both not empty, same length & equal content: bingo */
	}
    }

    if (i == smime_cipher_map_count)
	return 0;				/* no match found */
    else
	return smime_cipher_map[i].cipher;	/* match found, point to cipher */
}

/*
 * smime_choose_cipher - choose a cipher that works for all the recipients
 *
 * "scert"  - sender's certificate
 * "rcerts" - recipient's certificates
 */
static long
smime_choose_cipher(CERTCertificate *scert, CERTCertificate **rcerts)
{
    PRArenaPool *poolp;
    long cipher;
    long chosen_cipher;
    int *cipher_abilities;
    int *cipher_votes;
    int weak_mapi;
    int strong_mapi;
    int rcount, mapi, max, i;
    PRBool scert_is_fortezza = (scert == NULL) ? PR_FALSE : PK11_FortezzaHasKEA(scert);

    chosen_cipher = SMIME_RC2_CBC_40;		/* the default, LCD */
    weak_mapi = smime_mapi_by_cipher(chosen_cipher);

    poolp = PORT_NewArena (1024);		/* XXX what is right value? */
    if (poolp == NULL)
	goto done;

    cipher_abilities = (int *)PORT_ArenaZAlloc(poolp, smime_cipher_map_count * sizeof(int));
    cipher_votes     = (int *)PORT_ArenaZAlloc(poolp, smime_cipher_map_count * sizeof(int));
    if (cipher_votes == NULL || cipher_abilities == NULL)
	goto done;

    /* If the user has the Fortezza preference turned on, make
     *  that the strong cipher. Otherwise, use triple-DES. */
    strong_mapi = smime_mapi_by_cipher (SMIME_DES_EDE3_168);
    if (scert_is_fortezza) {
	mapi = smime_mapi_by_cipher(SMIME_FORTEZZA);
	if (mapi >= 0 && smime_cipher_map[mapi].enabled)
	    strong_mapi = mapi;
    }

    /* walk all the recipient's certs */
    for (rcount = 0; rcerts[rcount] != NULL; rcount++) {
	SECItem *profile;
	NSSSMIMECapability **caps;
	int pref;

	/* the first cipher that matches in the user's SMIME profile gets
	 * "smime_cipher_map_count" votes; the next one gets "smime_cipher_map_count" - 1
	 * and so on. If every cipher matches, the last one gets 1 (one) vote */
	pref = smime_cipher_map_count;

	/* find recipient's SMIME profile */
	profile = CERT_FindSMimeProfile(rcerts[rcount]);

	if (profile != NULL && profile->data != NULL && profile->len > 0) {
	    /* we have a profile */
	    caps = NULL;
	    /* decode it */
	    if (SEC_ASN1DecodeItem(poolp, &caps, smime_capabilities_template, profile) == SECSuccess &&
		    caps != NULL)
	    {
		/* walk the SMIME capabilities for this recipient */
		for (i = 0; caps[i] != NULL; i++) {
		    cipher = nss_SMIME_FindCipherForSMIMECap(caps[i]);
		    mapi = smime_mapi_by_cipher(cipher);
		    if (mapi >= 0) {
			/* found the cipher */
			cipher_abilities[mapi]++;
			cipher_votes[mapi] += pref;
			--pref;
		    }
		}
	    }
	} else {
	    /* no profile found - so we can only assume that the user can do
	     * the mandatory algorithms which is RC2-40 (weak crypto) and 3DES (strong crypto) */
	    SECKEYPublicKey *key;
	    unsigned int pklen_bits;

	    /*
	     * if recipient's public key length is > 512, vote for a strong cipher
	     * please not that the side effect of this is that if only one recipient
	     * has an export-level public key, the strong cipher is disabled.
	     *
	     * XXX This is probably only good for RSA keys.  What I would
	     * really like is a function to just say;  Is the public key in
	     * this cert an export-length key?  Then I would not have to
	     * know things like the value 512, or the kind of key, or what
	     * a subjectPublicKeyInfo is, etc.
	     */
	    key = CERT_ExtractPublicKey(rcerts[rcount]);
	    pklen_bits = 0;
	    if (key != NULL) {
		pklen_bits = SECKEY_PublicKeyStrength (key) * 8;
		SECKEY_DestroyPublicKey (key);
	    }

	    if (pklen_bits > 512) {
		/* cast votes for the strong algorithm */
		cipher_abilities[strong_mapi]++;
		cipher_votes[strong_mapi] += pref;
		pref--;
	    } 

	    /* always cast (possibly less) votes for the weak algorithm */
	    cipher_abilities[weak_mapi]++;
	    cipher_votes[weak_mapi] += pref;
	}
	if (profile != NULL)
	    SECITEM_FreeItem(profile, PR_TRUE);
    }

    /* find cipher that is agreeable by all recipients and that has the most votes */
    max = 0;
    for (mapi = 0; mapi < smime_cipher_map_count; mapi++) {
	/* if not all of the recipients can do this, forget it */
	if (cipher_abilities[mapi] != rcount)
	    continue;
	/* if cipher is not enabled or not allowed by policy, forget it */
	if (!smime_cipher_map[mapi].enabled || !smime_cipher_map[mapi].allowed)
	    continue;
	/* if we're not doing fortezza, but the cipher is fortezza, forget it */
	if (!scert_is_fortezza  && (smime_cipher_map[mapi].cipher == SMIME_FORTEZZA))
	    continue;
	/* now see if this one has more votes than the last best one */
	if (cipher_votes[mapi] >= max) {
	    /* if equal number of votes, prefer the ones further down in the list */
	    /* with the expectation that these are higher rated ciphers */
	    chosen_cipher = smime_cipher_map[mapi].cipher;
	    max = cipher_votes[mapi];
	}
    }
    /* if no common cipher was found, chosen_cipher stays at the default */

done:
    if (poolp != NULL)
	PORT_FreeArena (poolp, PR_FALSE);

    return chosen_cipher;
}

/*
 * XXX This is a hack for now to satisfy our current interface.
 * Eventually, with more parameters needing to be specified, just
 * looking up the keysize is not going to be sufficient.
 */
static int
smime_keysize_by_cipher (unsigned long which)
{
    int keysize;

    switch (which) {
      case SMIME_RC2_CBC_40:
	keysize = 40;
	break;
      case SMIME_RC2_CBC_64:
	keysize = 64;
	break;
      case SMIME_RC2_CBC_128:
	keysize = 128;
	break;
      case SMIME_DES_CBC_56:
      case SMIME_DES_EDE3_168:
      case SMIME_FORTEZZA:
	/*
	 * These are special; since the key size is fixed, we actually
	 * want to *avoid* specifying a key size.
	 */
	keysize = 0;
	break;
      default:
	keysize = -1;
	break;
    }

    return keysize;
}

/*
 * NSS_SMIMEUtil_FindBulkAlgForRecipients - find bulk algorithm suitable for all recipients
 *
 * it would be great for UI purposes if there would be a way to find out which recipients
 * prevented a strong cipher from being used...
 */
SECStatus
NSS_SMIMEUtil_FindBulkAlgForRecipients(CERTCertificate **rcerts, SECOidTag *bulkalgtag, int *keysize)
{
    unsigned long cipher;
    int mapi;

    cipher = smime_choose_cipher(NULL, rcerts);
    mapi = smime_mapi_by_cipher(cipher);

    *bulkalgtag = smime_cipher_map[mapi].algtag;
    *keysize = smime_keysize_by_cipher(smime_cipher_map[mapi].algtag);

    return SECSuccess;
}

static SECStatus
smime_init_caps(PRBool isFortezza)
{
    NSSSMIMECapability *cap;
    smime_cipher_map_entry *map;
    SECOidData *oiddata;
    SECStatus rv;
    int i, capIndex;

    /* if we have caps, and the prefs did not change, and we are using fortezza as last time */
    /* we're done */
    if (smime_encoded_caps != NULL && (!smime_prefs_changed) && lastUsedFortezza == isFortezza)
	return SECSuccess;

    /* ok, we need to cook up new caps. So throw the old ones away */
    if (smime_encoded_caps != NULL) {
	SECITEM_FreeItem (smime_encoded_caps, PR_TRUE);
	smime_encoded_caps = NULL;
    }

    /* if we have an old NSSSMIMECapability array, we'll reuse it (has the right size) */
    if (smime_capabilities == NULL) {
	smime_capabilities = (NSSSMIMECapability **)PORT_ZAlloc((smime_cipher_map_count + 1)
					  * sizeof(NSSSMIMECapability *));
	if (smime_capabilities == NULL)
	    return SECFailure;
    }

    rv = SECFailure;

    /* 
       The process of creating the encoded CMS cipher capability list
       involves two basic steps: 

       (a) Convert our internal representation of cipher preferences 
           (smime_prefs) into an array containing cipher OIDs and 
	   parameter data (smime_capabilities). This step is
	   performed here.

       (b) Encode, using ASN.1, the cipher information in 
           smime_capabilities, leaving the encoded result in 
	   smime_encoded_caps.

       (In the process of performing (a), Lisa put in some optimizations
       which allow us to avoid needlessly re-populating elements in 
       smime_capabilities as we walk through smime_prefs.)

       We want to use separate loop variables for smime_prefs and
       smime_capabilities because in the case where the Skipjack cipher 
       is turned on in the prefs, but where we don't want to include 
       Skipjack in the encoded capabilities (presumably due to using a 
       non-fortezza cert when sending a message), we want to avoid creating
       an empty element in smime_capabilities. This would otherwise cause 
       the encoding step to produce an empty set, since Skipjack happens 
       to be the first cipher in smime_prefs, if it is turned on.
    */
    capIndex = 0;
    for (i = 0; i < smime_cipher_map_count; i++) {
	/* Find the corresponding entry in the cipher map. */
	map = &(smime_cipher_map[i]);

	if (!map->enabled)
	    continue;

	/* If we're using a non-Fortezza cert, only advertise non-Fortezza
	   capabilities. (We advertise all capabilities if we have a 
	   Fortezza cert.) */
	if ((!isFortezza) && (map->cipher == SMIME_FORTEZZA))
	    continue;

	/* get next SMIME capability */
	cap = smime_capabilities[capIndex];
	if (cap == NULL) {
	    cap = (NSSSMIMECapability *)PORT_ZAlloc(sizeof(NSSSMIMECapability));
	    if (cap == NULL)
		break;
	    smime_capabilities[capIndex] = cap;
	}
	capIndex++;

	if (cap->cipher == smime_cipher_map[i].cipher)
	    continue;		/* no change to this one */

	oiddata = SECOID_FindOIDByTag(map->algtag);
	if (oiddata == NULL)
	    break;

	if (cap->capabilityID.data != NULL) {
	    SECITEM_FreeItem (&(cap->capabilityID), PR_FALSE);
	    cap->capabilityID.data = NULL;
	    cap->capabilityID.len = 0;
	}

	rv = SECITEM_CopyItem(NULL, &(cap->capabilityID), &(oiddata->oid));
	if (rv != SECSuccess)
	    break;

	if (map->parms == NULL) {
	    cap->parameters.data = NULL;
	    cap->parameters.len = 0;
	} else {
	    cap->parameters.data = map->parms->data;
	    cap->parameters.len = map->parms->len;
	}

	cap->cipher = smime_cipher_map[i].cipher;
    }

    while (capIndex < smime_cipher_map_count) {
	cap = smime_capabilities[capIndex];
	if (cap != NULL) {
	    SECITEM_FreeItem(&(cap->capabilityID), PR_FALSE);
	    PORT_Free(cap);
	}
	smime_capabilities[capIndex] = NULL;
	capIndex++;
    }
    smime_capabilities[capIndex] = NULL;	/* last one */

    smime_encoded_caps = SEC_ASN1EncodeItem (NULL, NULL, &smime_capabilities,
					     smime_capabilities_template);
    if (smime_encoded_caps == NULL)
	return SECFailure;

    lastUsedFortezza = isFortezza;

    return SECSuccess;
}

/*
 * NSS_SMIMEUtil_GetSMIMECapabilities - get S/MIME capabilities for this instance of NSS
 *
 * scans the list of allowed and enabled ciphers and construct a PKCS9-compliant
 * S/MIME capabilities attribute value.
 *
 * "cert" - sender's certificate
 */
SECItem *
NSS_SMIMEUtil_GetSMIMECapabilities(CERTCertificate *cert)
{

    PRBool isFortezza = PR_FALSE;

    /* See if the sender's cert specifies Fortezza key exchange. */
    if (cert != NULL)
	isFortezza = PK11_FortezzaHasKEA(cert);

    if (smime_init_caps(isFortezza) != SECSuccess)
	return NULL;

    return smime_encoded_caps;
}
