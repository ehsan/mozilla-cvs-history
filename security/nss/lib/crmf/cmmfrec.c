/* -*- Mode: C; tab-width: 8 -*-*/
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
 * This file will implement the functions related to key recovery in 
 * CMMF
 */

#include "nssrenam.h"
#include "cmmf.h"
#include "cmmfi.h"
#include "secitem.h"
#include "keyhi.h"

CMMFKeyRecRepContent*
CMMF_CreateKeyRecRepContent(void)
{
    PRArenaPool          *poolp;
    CMMFKeyRecRepContent *keyRecContent;

    poolp = PORT_NewArena(CRMF_DEFAULT_ARENA_SIZE);
    if (poolp == NULL) {
        return NULL;
    }
    keyRecContent = PORT_ArenaZNew(poolp, CMMFKeyRecRepContent);
    if (keyRecContent == NULL) {
        PORT_FreeArena(poolp, PR_FALSE);
	return NULL;
    }
    keyRecContent->poolp = poolp;
    return keyRecContent;
}

SECStatus
CMMF_DestroyKeyRecRepContent(CMMFKeyRecRepContent *inKeyRecRep)
{
    PORT_Assert(inKeyRecRep != NULL);
    if (inKeyRecRep != NULL && inKeyRecRep->poolp != NULL) {
        if (!inKeyRecRep->isDecoded) {
	    int i;

	    CERT_DestroyCertificate(inKeyRecRep->newSigCert);
	    if (inKeyRecRep->caCerts != NULL) {
	        for (i=0; inKeyRecRep->caCerts[i] != NULL; i++) {
		    CERT_DestroyCertificate(inKeyRecRep->caCerts[i]);
		}
	    }
	    if (inKeyRecRep->keyPairHist != NULL) {
	        for (i=0; inKeyRecRep->keyPairHist[i] != NULL; i++) {
		    if (inKeyRecRep->keyPairHist[i]->certOrEncCert.choice ==
			cmmfCertificate) {
		        CERT_DestroyCertificate(inKeyRecRep->keyPairHist[i]->
					       certOrEncCert.cert.certificate);
		    }
		}
	    }
	}
        PORT_FreeArena(inKeyRecRep->poolp, PR_TRUE);
    }
    return SECSuccess;
}

SECStatus
CMMF_KeyRecRepContentSetPKIStatusInfoStatus(CMMFKeyRecRepContent *inKeyRecRep,
					    CMMFPKIStatus         inPKIStatus)
{
    PORT_Assert(inKeyRecRep != NULL && inPKIStatus >= cmmfGranted &&
		inPKIStatus < cmmfNumPKIStatus);
    if (inKeyRecRep == NULL) {
        return SECFailure;
    }
    
    return cmmf_PKIStatusInfoSetStatus(&inKeyRecRep->status, 
				       inKeyRecRep->poolp,
				       inPKIStatus);
}

SECStatus
CMMF_KeyRecRepContentSetNewSignCert(CMMFKeyRecRepContent *inKeyRecRep,
				    CERTCertificate      *inNewSignCert)
{
    PORT_Assert (inKeyRecRep != NULL && inNewSignCert != NULL);
    if (inKeyRecRep == NULL || inNewSignCert == NULL) {
        return SECFailure;
    }
    inKeyRecRep->newSigCert = CERT_DupCertificate(inNewSignCert);
    return (inKeyRecRep->newSigCert == NULL) ? SECFailure : SECSuccess;    
}

SECStatus
CMMF_KeyRecRepContentSetCACerts(CMMFKeyRecRepContent *inKeyRecRep,
				CERTCertList         *inCACerts)
{
    SECStatus rv;
    void *mark;

    PORT_Assert (inKeyRecRep != NULL && inCACerts != NULL);
    if (inKeyRecRep == NULL || inCACerts == NULL) {
        return SECFailure;
    }
    mark = PORT_ArenaMark(inKeyRecRep->poolp);
    rv = cmmf_ExtractCertsFromList(inCACerts, inKeyRecRep->poolp,
				   &inKeyRecRep->caCerts);
    if (rv != SECSuccess) {
        PORT_ArenaRelease(inKeyRecRep->poolp, mark);
    } else {
        PORT_ArenaUnmark(inKeyRecRep->poolp, mark);
    }
    return rv;
}

SECStatus
CMMF_KeyRecRepContentSetCertifiedKeyPair(CMMFKeyRecRepContent *inKeyRecRep,
					 CERTCertificate      *inCert,
					 SECKEYPrivateKey     *inPrivKey,
					 SECKEYPublicKey      *inPubKey)
{
    CMMFCertifiedKeyPair *keyPair;
    CRMFEncryptedValue   *dummy;
    PRArenaPool          *poolp;
    void                 *mark;
    SECStatus             rv;

    PORT_Assert (inKeyRecRep != NULL &&
		 inCert      != NULL &&
		 inPrivKey   != NULL &&
		 inPubKey    != NULL);
    if (inKeyRecRep == NULL ||
	inCert      == NULL ||
	inPrivKey   == NULL ||
	inPubKey    == NULL) {
        return SECFailure;
    }
    poolp = inKeyRecRep->poolp;
    mark = PORT_ArenaMark(poolp);
    if (inKeyRecRep->keyPairHist == NULL) {
        inKeyRecRep->keyPairHist = PORT_ArenaNewArray(poolp, 
						      CMMFCertifiedKeyPair*,
						      (CMMF_MAX_KEY_PAIRS+1));
	if (inKeyRecRep->keyPairHist == NULL) {
	    goto loser;
	}
	inKeyRecRep->allocKeyPairs = CMMF_MAX_KEY_PAIRS;
	inKeyRecRep->numKeyPairs   = 0;
    }

    if (inKeyRecRep->allocKeyPairs == inKeyRecRep->numKeyPairs) {
        goto loser;
    }
    
    keyPair = PORT_ArenaZNew(poolp, CMMFCertifiedKeyPair);
    if (keyPair == NULL) {
        goto loser;
    }
    rv = cmmf_CertOrEncCertSetCertificate(&keyPair->certOrEncCert,
					  poolp, inCert);
    if (rv != SECSuccess) {
        goto loser;
    }
    keyPair->privateKey = PORT_ArenaZNew(poolp, CRMFEncryptedValue);
    if (keyPair->privateKey == NULL) {
        goto loser;
    }
    dummy = crmf_create_encrypted_value_wrapped_privkey(inPrivKey, inPubKey, 
							keyPair->privateKey);
    PORT_Assert(dummy = keyPair->privateKey);
    if (dummy != keyPair->privateKey) {
        crmf_destroy_encrypted_value(dummy, PR_TRUE);
	goto loser;
    }
    inKeyRecRep->keyPairHist[inKeyRecRep->numKeyPairs] = keyPair;
    inKeyRecRep->numKeyPairs++;
    inKeyRecRep->keyPairHist[inKeyRecRep->numKeyPairs] = NULL;
    PORT_ArenaUnmark(poolp, mark);
    return SECSuccess;

 loser:
    PORT_ArenaRelease(poolp, mark);
    return SECFailure;
}

CMMFPKIStatus
CMMF_KeyRecRepContentGetPKIStatusInfoStatus(CMMFKeyRecRepContent *inKeyRecRep)
{
    PORT_Assert(inKeyRecRep != NULL);
    if (inKeyRecRep == NULL) {
        return cmmfNoPKIStatus;
    }
    return cmmf_PKIStatusInfoGetStatus(&inKeyRecRep->status);
}

CERTCertificate*
CMMF_KeyRecRepContentGetNewSignCert(CMMFKeyRecRepContent *inKeyRecRep)
{
    PORT_Assert(inKeyRecRep != NULL);
    if (inKeyRecRep             == NULL ||
	inKeyRecRep->newSigCert == NULL) {
        return NULL;
    }
    return CERT_NewTempCertificate(CERT_GetDefaultCertDB(), 
			          &inKeyRecRep->newSigCert->signatureWrap.data,
				   NULL, PR_FALSE, PR_TRUE);
}

CERTCertList*
CMMF_KeyRecRepContentGetCACerts(CMMFKeyRecRepContent *inKeyRecRep)
{
    PORT_Assert(inKeyRecRep != NULL);
    if (inKeyRecRep == NULL || inKeyRecRep->caCerts == NULL) {
        return NULL;
    }
    return cmmf_MakeCertList(inKeyRecRep->caCerts);
}

int 
CMMF_KeyRecRepContentGetNumKeyPairs(CMMFKeyRecRepContent *inKeyRecRep)
{
    PORT_Assert(inKeyRecRep != NULL);
    return (inKeyRecRep == NULL) ? 0 : inKeyRecRep->numKeyPairs;
}

PRBool
cmmf_KeyRecRepContentIsValidIndex(CMMFKeyRecRepContent *inKeyRecRep,
				  int                   inIndex)
{
    int numKeyPairs = CMMF_KeyRecRepContentGetNumKeyPairs(inKeyRecRep);
    
    return (PRBool)(inIndex >= 0 && inIndex < numKeyPairs);
}

CMMFCertifiedKeyPair*
CMMF_KeyRecRepContentGetCertKeyAtIndex(CMMFKeyRecRepContent *inKeyRecRep,
				       int                   inIndex)
{
    CMMFCertifiedKeyPair *newKeyPair;
    SECStatus             rv;

    PORT_Assert(inKeyRecRep != NULL &&
		cmmf_KeyRecRepContentIsValidIndex(inKeyRecRep, inIndex));
    if (inKeyRecRep == NULL ||
	!cmmf_KeyRecRepContentIsValidIndex(inKeyRecRep, inIndex)) {
        return NULL;
    }
    newKeyPair = PORT_ZNew(CMMFCertifiedKeyPair);
    if (newKeyPair == NULL) {
        return NULL;
    }
    rv = cmmf_CopyCertifiedKeyPair(NULL, newKeyPair, 
				   inKeyRecRep->keyPairHist[inIndex]);
    if (rv != SECSuccess) {
        CMMF_DestroyCertifiedKeyPair(newKeyPair);
	newKeyPair = NULL;
    }
    return newKeyPair;
}

SECStatus 
CMMF_CertifiedKeyPairUnwrapPrivKey(CMMFCertifiedKeyPair *inKeyPair,
				   SECKEYPrivateKey     *inPrivKey,
				   SECItem              *inNickName,
				   PK11SlotInfo         *inSlot,
				   CERTCertDBHandle     *inCertdb,
				   SECKEYPrivateKey    **destPrivKey,
				   void                 *wincx)
{
    CERTCertificate *cert;
    SECItem keyUsageValue = {siBuffer, NULL, 0};
    unsigned char keyUsage = 0x0;
    SECKEYPublicKey *pubKey;
    SECStatus rv;

    PORT_Assert(inKeyPair != NULL &&
		inPrivKey != NULL && inCertdb != NULL);
    if (inKeyPair             == NULL ||
	inPrivKey             == NULL ||
	inKeyPair->privateKey == NULL ||
	inCertdb              == NULL) {
        return SECFailure;
    }
    
    cert = CMMF_CertifiedKeyPairGetCertificate(inKeyPair, inCertdb);
    CERT_FindKeyUsageExtension(cert, &keyUsageValue);
    if (keyUsageValue.data != NULL) {
        keyUsage = keyUsageValue.data[3];
	PORT_Free(keyUsageValue.data);
    }
    pubKey = CERT_ExtractPublicKey(cert);
    rv = crmf_encrypted_value_unwrap_priv_key(NULL, inKeyPair->privateKey,
					      inPrivKey, pubKey, 
					      inNickName, inSlot, keyUsage, 
					      destPrivKey, wincx);
    SECKEY_DestroyPublicKey(pubKey);
    CERT_DestroyCertificate(cert);
    return rv;
}


PRBool 
CMMF_KeyRecRepContentHasCACerts(CMMFKeyRecRepContent *inKeyRecRep)
{
    PORT_Assert(inKeyRecRep != NULL);
    if (inKeyRecRep == NULL) {
        return PR_FALSE;
    }
    return (PRBool)(inKeyRecRep->caCerts    != NULL && 
		    inKeyRecRep->caCerts[0] != NULL);
}
