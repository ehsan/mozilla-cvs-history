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

#ifdef DEBUG
static const char CVS_ID[] = "@(#) $RCSfile: tdcache.c,v $ $Revision: 1.11 $ $Date: 2001/11/28 20:19:38 $ $Name:  $";
#endif /* DEBUG */

#ifndef PKIM_H
#include "pkim.h"
#endif /* PKIM_H */

#ifndef PKIT_H
#include "pkit.h"
#endif /* PKIT_H */

#ifndef PKI_H
#include "pki.h"
#endif /* PKI_H */

#ifndef BASE_H
#include "base.h"
#endif /* BASE_H */

#ifdef DEBUG_CACHE
static PRLogModuleInfo *s_log = NULL;
#endif

#ifdef DEBUG_CACHE
static void log_item_dump(const char *msg, NSSItem *it)
{
    char buf[33];
    int i, j;
    for (i=0; i<10 && i<it->size; i++) {
	sprintf(&buf[2*i], "%02X", ((PRUint8 *)it->data)[i]);
    }
    if (it->size>10) {
	sprintf(&buf[2*i], "..");
	i += 1;
	for (j=it->size-1; i<=16 && j>10; i++, j--) {
	    sprintf(&buf[2*i], "%02X", ((PRUint8 *)it->data)[j]);
	}
    }
    PR_LOG(s_log, PR_LOG_DEBUG, ("%s: %s", msg, buf));
}
#endif

#ifdef DEBUG_CACHE
static void log_cert_ref(const char *msg, NSSCertificate *c)
{
    PR_LOG(s_log, PR_LOG_DEBUG, ("%s: %s", msg,
                           (c->nickname) ? c->nickname : c->email));
    log_item_dump("\tserial", &c->serial);
    log_item_dump("\tsubject", &c->subject);
}
#endif

/* Certificate cache routines */

/* XXX
 * Locking is not handled well at all.  A single, global lock with sub-locks
 * in the collection types.  Cleanup needed.
 */

/* should it live in its own arena? */
struct nssTDCertificateCacheStr 
{
    PZLock *lock;
    NSSArena *arena;
    nssHash *issuerAndSN;
    nssHash *subject;
    nssHash *nickname;
    nssHash *email;
};

struct cache_entry_str 
{
    union {
	NSSCertificate *cert;
	nssList *list;
	void *value;
    } entry;
    PRUint32 hits;
    PRTime lastHit;
};

typedef struct cache_entry_str cache_entry;

static cache_entry *
new_cache_entry(NSSArena *arena, void *value)
{
    cache_entry *ce = nss_ZNEW(arena, cache_entry);
    if (ce) {
	ce->entry.value = value;
	ce->hits = 1;
	ce->lastHit = PR_Now();
    }
    return ce;
}

static PLHashNumber
nss_certificate_hash
(
  const void *key
)
{
    int i;
    PLHashNumber h;
    NSSCertificate *c = (NSSCertificate *)key;
    h = 0;
    for (i=0; i<c->issuer.size; i++)
	h = (h >> 28) ^ (h << 4) ^ ((unsigned char *)c->issuer.data)[i];
    for (i=0; i<c->serial.size; i++)
	h = (h >> 28) ^ (h << 4) ^ ((unsigned char *)c->serial.data)[i];
    return h;
}

static int
nss_compare_certs(const void *v1, const void *v2)
{
    PRStatus ignore;
    NSSCertificate *c1 = (NSSCertificate *)v1;
    NSSCertificate *c2 = (NSSCertificate *)v2;
    return (int)(nssItem_Equal(&c1->issuer, &c2->issuer, &ignore) &&
                 nssItem_Equal(&c1->serial, &c2->serial, &ignore));
}

/* sort the subject list from newest to oldest */
static PRIntn subject_list_sort(void *v1, void *v2)
{
    NSSCertificate *c1 = (NSSCertificate *)v1;
    NSSCertificate *c2 = (NSSCertificate *)v2;
    nssDecodedCert *dc1 = nssCertificate_GetDecoding(c1);
    nssDecodedCert *dc2 = nssCertificate_GetDecoding(c2);
    if (dc1->isNewerThan(dc1, dc2)) {
	return -1;
    } else {
	return 1;
    }
}

/* this should not be exposed in a header, but is here to keep the above
 * types/functions static
 */
NSS_IMPLEMENT PRStatus
nssTrustDomain_InitializeCache
(
  NSSTrustDomain *td,
  PRUint32 cacheSize
)
{
    NSSArena *arena;
    nssTDCertificateCache *cache = td->cache;
#ifdef DEBUG_CACHE
    s_log = PR_NewLogModule("nss_cache");
    PR_ASSERT(s_log);
#endif
    PR_ASSERT(!cache);
    arena = nssArena_Create();
    if (!arena) {
	return PR_FAILURE;
    }
    cache = nss_ZNEW(arena, nssTDCertificateCache);
    if (!cache) {
	nssArena_Destroy(arena);
	return PR_FAILURE;
    }
    cache->lock = PZ_NewLock(nssILockCache);
    if (!cache->lock) {
	nssArena_Destroy(arena);
	return PR_FAILURE;
    }
    PZ_Lock(cache->lock);
    /* Create the issuer and serial DER --> certificate hash */
    cache->issuerAndSN = nssHash_Create(arena, cacheSize, 
                                        nss_certificate_hash, 
                                        nss_compare_certs, 
                                        PL_CompareValues);
    if (!cache->issuerAndSN) {
	goto loser;
    }
    /* Create the subject DER --> subject list hash */
    cache->subject = nssHash_CreateItem(arena, cacheSize);
    if (!cache->subject) {
	goto loser;
    }
    /* Create the nickname --> subject list hash */
    cache->nickname = nssHash_CreateString(arena, cacheSize);
    if (!cache->nickname) {
	goto loser;
    }
    /* Create the email --> list of subject lists hash */
    cache->email = nssHash_CreateString(arena, cacheSize);
    if (!cache->email) {
	goto loser;
    }
    cache->arena = arena;
    PZ_Unlock(cache->lock);
    td->cache = cache;
#ifdef DEBUG_CACHE
    PR_LOG(s_log, PR_LOG_DEBUG, ("Cache initialized."));
#endif
    return PR_SUCCESS;
loser:
    PZ_Unlock(cache->lock);
    PZ_DestroyLock(cache->lock);
    nssArena_Destroy(arena);
    td->cache = NULL;
#ifdef DEBUG_CACHE
    PR_LOG(s_log, PR_LOG_DEBUG, ("Cache initialization failed."));
#endif
    return PR_FAILURE;
}

NSS_IMPLEMENT PRStatus
nssTrustDomain_DestroyCache
(
  NSSTrustDomain *td
)
{
    PZ_DestroyLock(td->cache->lock);
    nssArena_Destroy(td->cache->arena);
    td->cache = NULL;
#ifdef DEBUG_CACHE
    PR_LOG(s_log, PR_LOG_DEBUG, ("Cache destroyed."));
#endif
    return PR_SUCCESS;
}

static PRStatus
remove_issuer_and_serial_entry
(
  nssTDCertificateCache *cache,
  NSSCertificate *cert
)
{
    /* Remove the cert from the issuer/serial hash */
    nssHash_Remove(cache->issuerAndSN, cert);
#ifdef DEBUG_CACHE
    log_cert_ref("removed issuer/sn", cert);
#endif
    return PR_SUCCESS;
}

static PRStatus
remove_subject_entry
(
  nssTDCertificateCache *cache,
  NSSCertificate *cert,
  nssList **subjectList
)
{
    PRStatus nssrv;
    cache_entry *ce;
    *subjectList = NULL;
    /* Get the subject list for the cert's subject */
    ce = (cache_entry *)nssHash_Lookup(cache->subject, &cert->subject);
    if (ce) {
	/* Remove the cert from the subject hash */
	nssList_Remove(ce->entry.list, cert);
	*subjectList = ce->entry.list;
	nssrv = PR_SUCCESS;
#ifdef DEBUG_CACHE
	log_cert_ref("removed cert", cert);
	log_item_dump("from subject list", &cert->subject);
#endif
    } else {
	nssrv = PR_FAILURE;
    }
    return nssrv;
}

static PRStatus
remove_nickname_entry
(
  nssTDCertificateCache *cache,
  NSSCertificate *cert,
  nssList *subjectList
)
{
    PRStatus nssrv;
    if (cert->nickname) {
	nssHash_Remove(cache->nickname, cert->nickname);
	nssrv = PR_SUCCESS;
#ifdef DEBUG_CACHE
	PR_LOG(s_log, PR_LOG_DEBUG, ("removed nickname %s", cert->nickname));
#endif
    } else {
	nssrv = PR_FAILURE;
    }
    return nssrv;
}

static PRStatus
remove_email_entry
(
  nssTDCertificateCache *cache,
  NSSCertificate *cert,
  nssList *subjectList
)
{
    PRStatus nssrv = PR_FAILURE;
    cache_entry *ce;
    /* Find the subject list in the email hash */
    if (cert->email) {
	ce = (cache_entry *)nssHash_Lookup(cache->email, cert->email);
	if (ce) {
	    nssList *subjects = ce->entry.list;
	    /* Remove the subject list from the email hash */
	    nssList_Remove(subjects, subjectList);
#ifdef DEBUG_CACHE
	    log_item_dump("removed subject list", &cert->subject);
	    PR_LOG(s_log, PR_LOG_DEBUG, ("for email %s", cert->email));
#endif
	    if (nssList_Count(subjects) == 0) {
		/* No more subject lists for email, delete list and
		* remove hash entry
		*/
		(void)nssList_Destroy(subjects);
		nssHash_Remove(cache->email, cert->email);
#ifdef DEBUG_CACHE
		PR_LOG(s_log, PR_LOG_DEBUG, ("removed email %s", cert->email));
#endif
	    }
	    nssrv = PR_SUCCESS;
	}
    }
    return nssrv;
}

NSS_IMPLEMENT void
nssTrustDomain_RemoveCertFromCache
(
  NSSTrustDomain *td,
  NSSCertificate *cert
)
{
    nssList *subjectList;
    PRStatus nssrv;
#ifdef DEBUG_CACHE
    log_cert_ref("attempt to remove cert", cert);
#endif
    PZ_Lock(td->cache->lock);
    if (!nssHash_Exists(td->cache->issuerAndSN, cert)) {
	PZ_Unlock(td->cache->lock);
#ifdef DEBUG_CACHE
	PR_LOG(s_log, PR_LOG_DEBUG, ("but it wasn't in the cache"));
#endif
	return;
    }
    nssrv = remove_issuer_and_serial_entry(td->cache, cert);
    if (nssrv != PR_SUCCESS) {
	goto loser;
    }
    nssrv = remove_subject_entry(td->cache, cert, &subjectList);
    if (nssrv != PR_SUCCESS) {
	goto loser;
    }
    if (nssList_Count(subjectList) == 0) {
	PRStatus nssrv2;
	nssrv = remove_nickname_entry(td->cache, cert, subjectList);
	nssrv2 = remove_email_entry(td->cache, cert, subjectList);
#ifndef NSS_3_4_CODE
	/* XXX Again, 3.4 allows for certs w/o either nickname or email */
	if (nssrv != PR_SUCCESS && nssrv2 != PR_SUCCESS) {
	    goto loser;
	}
#endif
	(void)nssList_Destroy(subjectList);
	nssHash_Remove(td->cache->subject, &cert->subject);
    }
    PZ_Unlock(td->cache->lock);
    return;
loser:
    /* if here, then the cache is inconsistent.  For now, flush it. */
    PZ_Unlock(td->cache->lock);
#ifdef DEBUG_CACHE
    PR_LOG(s_log, PR_LOG_DEBUG, ("remove cert failed, flushing"));
#endif
    nssTrustDomain_FlushCache(td, -1.0);
}

/* This is used to remove all certs below a certain threshold, where
 * the value is determined by how many times the cert has been requested
 * and when the last request was.
 */
NSS_IMPLEMENT void
nssTrustDomain_FlushCache
(
  NSSTrustDomain *td,
  PRFloat64 threshold
)
{
}

struct token_cert_destructor {
    nssTDCertificateCache *cache;
    NSSToken *token;
};

static void 
remove_token_certs(const void *k, void *v, void *a)
{
#if 0
    struct NSSItem *identifier = (struct NSSItem *)k;
    NSSCertificate *c = (NSSCertificate *)v;
    struct token_cert_destructor *tcd = (struct token_cert_destructor *)a;
    if (c->token == tcd->token) {
	nssHash_Remove(tcd->cache->issuerAndSN, identifier);
	/* remove from the other hashes */
    }
#endif
}

/* 
 * Remove all certs for the given token from the cache.  This is
 * needed if the token is removed.
 */
NSS_IMPLEMENT PRStatus
nssTrustDomain_RemoveTokenCertsFromCache
(
  NSSTrustDomain *td,
  NSSToken *token
)
{
    struct token_cert_destructor tcd;
    tcd.cache = td->cache;
    tcd.token = token;
    PZ_Lock(td->cache->lock);
    nssHash_Iterate(td->cache->issuerAndSN, remove_token_certs, (void *)&tcd);
    PZ_Unlock(td->cache->lock);
    return PR_SUCCESS;
}

static PRStatus
add_issuer_and_serial_entry
(
  NSSArena *arena,
  nssTDCertificateCache *cache, 
  NSSCertificate *cert
)
{
    cache_entry *ce;
    ce = new_cache_entry(arena, (void *)cert);
#ifdef DEBUG_CACHE
    log_cert_ref("added to issuer/sn", cert);
#endif
    return nssHash_Add(cache->issuerAndSN, cert, (void *)ce);
}

static PRStatus
add_subject_entry
(
  NSSArena *arena,
  nssTDCertificateCache *cache, 
  NSSCertificate *cert,
  nssList **subjectList
)
{
    PRStatus nssrv;
    nssList *list;
    cache_entry *ce;
    *subjectList = NULL;  /* this is only set if a new one is created */
    ce = (cache_entry *)nssHash_Lookup(cache->subject, &cert->subject);
    if (ce) {
	ce->hits++;
	ce->lastHit = PR_Now();
	/* The subject is already in, add this cert to the list */
	nssrv = nssList_AddUnique(ce->entry.list, cert);
#ifdef DEBUG_CACHE
	log_cert_ref("added to existing subject list", cert);
#endif
    } else {
	/* Create a new subject list for the subject */
	list = nssList_Create(arena, PR_FALSE);
	if (!list) {
	    return PR_FAILURE;
	}
	ce = new_cache_entry(arena, (void *)list);
	if (!ce) {
	    return PR_FAILURE;
	}
	nssList_SetSortFunction(list, subject_list_sort);
	/* Add the cert entry to this list of subjects */
	nssrv = nssList_AddUnique(list, cert);
	if (nssrv != PR_SUCCESS) {
	    return nssrv;
	}
	/* Add the subject list to the cache */
	nssrv = nssHash_Add(cache->subject, &cert->subject, ce);
	if (nssrv != PR_SUCCESS) {
	    return nssrv;
	}
	*subjectList = list;
#ifdef DEBUG_CACHE
	log_cert_ref("created subject list", cert);
#endif
    }
    return nssrv;
}

static PRStatus
add_nickname_entry
(
  NSSArena *arena,
  nssTDCertificateCache *cache, 
  NSSCertificate *cert,
  nssList *subjectList
)
{
    PRStatus nssrv = PR_SUCCESS;
    cache_entry *ce;
    ce = (cache_entry *)nssHash_Lookup(cache->nickname, cert->nickname);
    if (ce) {
	/* This is a collision.  A nickname entry already exists for this
	 * subject, but a subject entry didn't.  This would imply there are
	 * two subjects using the same nickname, which is not allowed.
	 */
	return PR_FAILURE;
    } else {
	ce = new_cache_entry(arena, subjectList);
	if (!ce) {
	    return PR_FAILURE;
	}
	nssrv = nssHash_Add(cache->nickname, cert->nickname, ce);
#ifdef DEBUG_CACHE
	log_cert_ref("created nickname for", cert);
#endif
    }
    return nssrv;
}

static PRStatus
add_email_entry
(
  NSSArena *arena,
  nssTDCertificateCache *cache, 
  NSSCertificate *cert,
  nssList *subjectList
)
{
    PRStatus nssrv = PR_SUCCESS;
    nssList *subjects;
    cache_entry *ce;
    ce = (cache_entry *)nssHash_Lookup(cache->email, cert->email);
    if (ce) {
	/* Already have an entry for this email address, but not subject */
	subjects = ce->entry.list;
	nssrv = nssList_AddUnique(subjects, subjectList);
	ce->hits++;
	ce->lastHit = PR_Now();
#ifdef DEBUG_CACHE
	log_cert_ref("added subject to email for", cert);
#endif
    } else {
	/* Create a new list of subject lists, add this subject */
	subjects = nssList_Create(arena, PR_TRUE);
	if (!subjects) {
	    return PR_FAILURE;
	}
	/* Add the new subject to the list */
	nssrv = nssList_AddUnique(subjects, subjectList);
	if (nssrv != PR_SUCCESS) {
	    return nssrv;
	}
	/* Add the new entry to the cache */
	ce = new_cache_entry(arena, (void *)subjects);
	if (!ce) {
	    return PR_FAILURE;
	}
	nssrv = nssHash_Add(cache->email, &cert->email, ce);
	if (nssrv != PR_SUCCESS) {
	    return nssrv;
	}
#ifdef DEBUG_CACHE
	log_cert_ref("created email for", cert);
#endif
    }
    return nssrv;
}

static PRStatus
add_cert_to_cache
(
  NSSTrustDomain *td, 
  NSSCertificate *cert
)
{
    NSSArena *arena;
    nssList *subjectList;
    PRStatus nssrv;
    PRUint32 added = 0;
    arena = td->cache->arena;
    PZ_Lock(td->cache->lock);
    /* If it exists in the issuer/serial hash, it's already in all */
    if (nssHash_Exists(td->cache->issuerAndSN, cert)) {
#ifdef DEBUG_CACHE
	log_cert_ref("attempted to add cert already in cache", cert);
#endif
	PZ_Unlock(td->cache->lock);
	return PR_SUCCESS;
    }
    /* create a new cache entry for this cert */
    nssrv = add_issuer_and_serial_entry(arena, td->cache, cert);
    if (nssrv != PR_SUCCESS) {
	goto loser;
    }
    added++;
    /* create a new subject list for this cert, or add to existing */
    nssrv = add_subject_entry(arena, td->cache, cert, &subjectList);
    if (nssrv != PR_SUCCESS) {
	goto loser;
    }
    added++;
    /* If a new subject entry was created, also need nickname and/or email */
    if (subjectList != NULL) {
	PRBool handle = PR_FALSE;
	if (cert->nickname) {
	    nssrv = add_nickname_entry(arena, td->cache, cert, subjectList);
	    if (nssrv != PR_SUCCESS) {
		goto loser;
	    }
	    handle = PR_TRUE;
	    added++;
	}
	if (cert->email) {
	    nssrv = add_email_entry(arena, td->cache, cert, subjectList);
	    if (nssrv != PR_SUCCESS) {
		goto loser;
	    }
	    handle = PR_TRUE;
	    added += 2;
	}
#ifdef nodef
	/* I think either a nickname or email address must be associated
	 * with the cert.  However, certs are passed to NewTemp without
	 * either.  This worked in the old code, so it must work now.
	 */
	if (!handle) {
	    /* Require either nickname or email handle */
	    nssrv = PR_FAILURE;
	    goto loser;
	}
#endif
    }
    PZ_Unlock(td->cache->lock);
    return nssrv;
loser:
    /* Remove any handles that have been created */
    if (added >= 1) {
	(void)remove_issuer_and_serial_entry(td->cache, cert);
    }
    if (added >= 2) {
	(void)remove_subject_entry(td->cache, cert, &subjectList);
    }
    if (added == 3 || added == 5) {
	(void)remove_nickname_entry(td->cache, cert, subjectList);
    }
    if (added >= 4) {
	(void)remove_email_entry(td->cache, cert, subjectList);
    }
    PZ_Unlock(td->cache->lock);
    return PR_FAILURE;
}

NSS_IMPLEMENT PRStatus
nssTrustDomain_AddCertsToCache
(
  NSSTrustDomain *td,
  NSSCertificate **certs,
  PRUint32 numCerts
)
{
    PRUint32 i;
    for (i=0; i<numCerts && certs[i]; i++) {
	if (add_cert_to_cache(td, certs[i]) != PR_SUCCESS) {
	    return PR_FAILURE;
	} 
    }
    return PR_SUCCESS;
}

static NSSCertificate **
collect_subject_certs
(
  nssList *subjectList,
  nssList *rvCertListOpt
)
{
    NSSCertificate *c;
    NSSCertificate **rvArray = NULL;
    PRUint32 count;
    if (rvCertListOpt) {
	nssListIterator *iter = nssList_CreateIterator(subjectList);
	for (c  = (NSSCertificate *)nssListIterator_Start(iter);
	     c != (NSSCertificate *)NULL;
	     c  = (NSSCertificate *)nssListIterator_Next(iter)) {
	    nssList_Add(rvCertListOpt, c);
	}
	nssListIterator_Finish(iter);
	nssListIterator_Destroy(iter);
    } else {
	count = nssList_Count(subjectList);
	rvArray = nss_ZNEWARRAY(NULL, NSSCertificate *, count + 1);
	if (!rvArray) {
	    return (NSSCertificate **)NULL;
	}
	nssList_GetArray(subjectList, (void **)rvArray, count);
    }
    return rvArray;
}

/*
 * Find all cached certs with this subject.
 */
NSS_IMPLEMENT NSSCertificate **
nssTrustDomain_GetCertsForSubjectFromCache
(
  NSSTrustDomain *td,
  NSSDER *subject,
  nssList *certListOpt
)
{
    NSSCertificate **rvArray = NULL;
    cache_entry *ce;
#ifdef DEBUG_CACHE
    log_item_dump("looking for cert by subject", subject);
#endif
    PZ_Lock(td->cache->lock);
    ce = (cache_entry *)nssHash_Lookup(td->cache->subject, subject);
    if (ce) {
	ce->hits++;
	ce->lastHit = PR_Now();
#ifdef DEBUG_CACHE
	PR_LOG(s_log, PR_LOG_DEBUG, ("... found, %d hits", ce->hits));
#endif
    }
    PZ_Unlock(td->cache->lock);
    if (ce) {
	rvArray = collect_subject_certs(ce->entry.list, certListOpt);
    }
    return rvArray;
}

/*
 * Find all cached certs with this label.
 */
NSS_IMPLEMENT NSSCertificate **
nssTrustDomain_GetCertsForNicknameFromCache
(
  NSSTrustDomain *td,
  NSSUTF8 *nickname,
  nssList *certListOpt
)
{
    NSSCertificate **rvArray = NULL;
    cache_entry *ce;
#ifdef DEBUG_CACHE
    PR_LOG(s_log, PR_LOG_DEBUG, ("looking for cert by nick %s", nickname));
#endif
    PZ_Lock(td->cache->lock);
    ce = (cache_entry *)nssHash_Lookup(td->cache->nickname, nickname);
    if (ce) {
	ce->hits++;
	ce->lastHit = PR_Now();
#ifdef DEBUG_CACHE
	PR_LOG(s_log, PR_LOG_DEBUG, ("... found, %d hits", ce->hits));
#endif
    }
    PZ_Unlock(td->cache->lock);
    if (ce) {
	rvArray = collect_subject_certs(ce->entry.list, certListOpt);
    }
    return rvArray;
}

/*
 * Find all cached certs with this email address.
 */
NSS_IMPLEMENT NSSCertificate **
nssTrustDomain_GetCertsForEmailAddressFromCache
(
  NSSTrustDomain *td,
  NSSASCII7 *email,
  nssList *certListOpt
)
{
    NSSCertificate **rvArray = NULL;
    cache_entry *ce;
    nssList *collectList;
#ifdef DEBUG_CACHE
    PR_LOG(s_log, PR_LOG_DEBUG, ("looking for cert by email %s", email));
#endif
    PZ_Lock(td->cache->lock);
    ce = (cache_entry *)nssHash_Lookup(td->cache->email, email);
    if (ce) {
	ce->hits++;
	ce->lastHit = PR_Now();
#ifdef DEBUG_CACHE
	PR_LOG(s_log, PR_LOG_DEBUG, ("... found, %d hits", ce->hits));
#endif
    }
    PZ_Unlock(td->cache->lock);
    if (ce) {
	nssListIterator *iter;
	nssList *subjectList;
	if (certListOpt) {
	    collectList = certListOpt;
	} else {
	    collectList = nssList_Create(NULL, PR_FALSE);
	}
	iter = nssList_CreateIterator(ce->entry.list);
	for (subjectList  = (nssList *)nssListIterator_Start(iter);
	     subjectList != (nssList *)NULL;
	     subjectList  = (nssList *)nssListIterator_Next(iter)) {
	    (void)collect_subject_certs(subjectList, collectList);
	}
	nssListIterator_Finish(iter);
	nssListIterator_Destroy(iter);
    }
    if (!certListOpt) {
	PRUint32 count = nssList_Count(collectList);
	rvArray = nss_ZNEWARRAY(NULL, NSSCertificate *, count);
	if (rvArray) {
	    nssList_GetArray(collectList, (void **)rvArray, count);
	}
	nssList_Destroy(collectList);
    }
    return rvArray;
}

/*
 * Look for a specific cert in the cache
 */
NSS_IMPLEMENT NSSCertificate *
nssTrustDomain_GetCertForIssuerAndSNFromCache
(
  NSSTrustDomain *td,
  NSSDER *issuer,
  NSSDER *serial
)
{
    NSSCertificate certkey;
    NSSCertificate *rvCert = NULL;
    cache_entry *ce;
    certkey.issuer.data = issuer->data;
    certkey.issuer.size = issuer->size;
    certkey.serial.data = serial->data;
    certkey.serial.size = serial->size;
#ifdef DEBUG_CACHE
    log_item_dump("looking for cert by issuer/sn, issuer", issuer);
    log_item_dump("                               serial", serial);
#endif
    PZ_Lock(td->cache->lock);
    ce = (cache_entry *)nssHash_Lookup(td->cache->issuerAndSN, &certkey);
    if (ce) {
	ce->hits++;
	ce->lastHit = PR_Now();
	rvCert = ce->entry.cert;
#ifdef DEBUG_CACHE
	PR_LOG(s_log, PR_LOG_DEBUG, ("... found, %d hits", ce->hits));
#endif
    }
    PZ_Unlock(td->cache->lock);
    return rvCert;
}

#ifdef NSS_3_4_CODE
static PRStatus
issuer_and_serial_from_encoding
(
  NSSBER *encoding, 
  NSSDER *issuer, 
  NSSDER *serial
)
{
    SECItem derCert, derIssuer, derSerial;
    SECStatus secrv;
    derCert.data = (unsigned char *)encoding->data;
    derCert.len = encoding->size;
    secrv = CERT_IssuerNameFromDERCert(&derCert, &derIssuer);
    if (secrv != SECSuccess) {
	return PR_FAILURE;
    }
    secrv = CERT_SerialNumberFromDERCert(&derCert, &derSerial);
    if (secrv != SECSuccess) {
	return PR_FAILURE;
    }
    issuer->data = derIssuer.data;
    issuer->size = derIssuer.len;
    serial->data = derSerial.data;
    serial->size = derSerial.len;
    return PR_SUCCESS;
}
#endif

/*
 * Look for a specific cert in the cache
 */
NSS_IMPLEMENT NSSCertificate *
nssTrustDomain_GetCertByDERFromCache
(
  NSSTrustDomain *td,
  NSSDER *der
)
{
    PRStatus nssrv = PR_FAILURE;
    NSSDER issuer, serial;
    NSSCertificate *rvCert;
#ifdef NSS_3_4_CODE
    nssrv = issuer_and_serial_from_encoding(der, &issuer, &serial);
#endif
    if (nssrv != PR_SUCCESS) {
	return NULL;
    }
#ifdef DEBUG_CACHE
    log_item_dump("looking for cert by DER", der);
#endif
    rvCert = nssTrustDomain_GetCertForIssuerAndSNFromCache(td, 
                                                           &issuer, &serial);
#ifdef NSS_3_4_CODE
    PORT_Free(issuer.data);
    PORT_Free(serial.data);
#endif
    return rvCert;
}

static void cert_iter(const void *k, void *v, void *a)
{
    nssList *certList = (nssList *)a;
    NSSCertificate *c = (NSSCertificate *)v;
    nssList_Add(certList, c);
}

NSS_EXTERN NSSCertificate **
nssTrustDomain_GetCertsFromCache
(
  NSSTrustDomain *td,
  nssList *certListOpt
)
{
    NSSCertificate **rvArray = NULL;
    nssList *certList;
    if (certListOpt) {
	certList = certListOpt;
    } else {
	certList = nssList_Create(NULL, PR_FALSE);
    }
    PZ_Lock(td->cache->lock);
    nssHash_Iterate(td->cache->issuerAndSN, cert_iter, (void *)certList);
    PZ_Unlock(td->cache->lock);
    if (!certListOpt) {
	PRUint32 count = nssList_Count(certList);
	rvArray = nss_ZNEWARRAY(NULL, NSSCertificate *, count);
	nssList_GetArray(certList, (void **)rvArray, count);
	nssList_Destroy(certList);
    }
    return rvArray;
}

