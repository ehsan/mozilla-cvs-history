/* 
 * This file implements the CLIENT Session ID cache.  
 *
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
 *
 * $Id: sslnonce.c,v 1.1 2000/03/31 19:35:15 relyea%netscape.com Exp $
 */

#include "cert.h"
#include "secitem.h"
#include "ssl.h"

#include "sslimpl.h"
#include "sslproto.h"
#include "prlock.h"
#include "nsslocks.h"


PRUint32 ssl_sid_timeout = 100;
PRUint32 ssl3_sid_timeout = 86400L; /* 24 hours */

static sslSessionID *cache;
static PRLock *      cacheLock;

/* sids can be in one of 4 states:
 *
 * never_cached, 	created, but not yet put into cache. 
 * in_client_cache, 	in the client cache's linked list.
 * in_server_cache, 	entry came from the server's cache file.
 * invalid_cache	has been removed from the cache. 
 */

#define LOCK_CACHE 	lock_cache()
#define UNLOCK_CACHE	PR_Unlock(cacheLock)

static void 
lock_cache(void)
{
    /* XXX Since the client session cache has no init function, we must
     * XXX init the cacheLock on the first call.  Fix in NSS 3.0.
     */
    if (!cacheLock)
	nss_InitLock(&cacheLock);
    PR_Lock(cacheLock);
}

/* BEWARE: This function gets called for both client and server SIDs !!
 * If the unreferenced sid is not in the cache, Free sid and its contents.
 */
static void
ssl_DestroySID(sslSessionID *sid)
{
    SSL_TRC(8, ("SSL: destroy sid: sid=0x%x cached=%d", sid, sid->cached));
    PORT_Assert((sid->references == 0));

    if (sid->cached == in_client_cache)
    	return;	/* it will get taken care of next time cache is traversed. */

    if (sid->version < SSL_LIBRARY_VERSION_3_0) {
	SECITEM_ZfreeItem(&sid->u.ssl2.masterKey, PR_FALSE);
	SECITEM_ZfreeItem(&sid->u.ssl2.cipherArg, PR_FALSE);
    }
    if (sid->peerID != NULL)
	PORT_Free((void *)sid->peerID);		/* CONST */

    if (sid->urlSvrName != NULL)
	PORT_Free((void *)sid->urlSvrName);	/* CONST */

    if ( sid->peerCert ) {
	CERT_DestroyCertificate(sid->peerCert);
    }
    
    PORT_ZFree(sid, sizeof(sslSessionID));
}

/* BEWARE: This function gets called for both client and server SIDs !!
 * Decrement reference count, and 
 *    free sid if ref count is zero, and sid is not in the cache. 
 * Does NOT remove from the cache first.  
 * If the sid is still in the cache, it is left there until next time
 * the cache list is traversed.
 */
static void 
ssl_FreeLockedSID(sslSessionID *sid)
{
    PORT_Assert(sid->references >= 1);
    if (--sid->references == 0) {
	ssl_DestroySID(sid);
    }
}

/* BEWARE: This function gets called for both client and server SIDs !!
 * Decrement reference count, and 
 *    free sid if ref count is zero, and sid is not in the cache. 
 * Does NOT remove from the cache first.  
 * These locks are necessary because the sid _might_ be in the cache list.
 */
void
ssl_FreeSID(sslSessionID *sid)
{
    LOCK_CACHE;
    ssl_FreeLockedSID(sid);
    UNLOCK_CACHE;
}

/************************************************************************/

/*
**  Lookup sid entry in cache by Address, port, and peerID string.
**  If found, Increment reference count, and return pointer to caller.
**  If it has timed out or ref count is zero, remove from list and free it.
*/

sslSessionID *
ssl_LookupSID(PRUint32 addr, PRUint16 port, const char *peerID, 
              const char * urlSvrName)
{
    sslSessionID **sidp;
    sslSessionID * sid;
    PRUint32       now;

    if (!urlSvrName)
    	return NULL;
    now = ssl_Time();
    LOCK_CACHE;
    sidp = &cache;
    while ((sid = *sidp) != 0) {
	PORT_Assert(sid->cached == in_client_cache);
	PORT_Assert(sid->references >= 1);

	SSL_TRC(8, ("SSL: Lookup1: sid=0x%x", sid));

	if (((sid->version < SSL_LIBRARY_VERSION_3_0) &&
	                     (sid->time + ssl_sid_timeout  < now)) ||
	    ((sid->version >= SSL_LIBRARY_VERSION_3_0) &&
	                     (sid->time + ssl3_sid_timeout < now)) ||
	    !sid->references) {
	    /*
	    ** This session-id timed out, or was orphaned.
	    ** Don't even care who it belongs to, blow it out of our cache.
	    */
	    SSL_TRC(7, ("SSL: lookup1, throwing sid out, age=%d refs=%d",
			now - sid->time, sid->references));

	    *sidp = sid->next; 			/* delink it from the list. */
	    sid->cached = invalid_cache;	/* mark not on list. */
	    if (!sid->references)
	    	ssl_DestroySID(sid);
	    else
		ssl_FreeLockedSID(sid);		/* drop ref count, free. */

	} else if ((sid->addr == addr) && /* server IP addr matches */
	           (sid->port == port) && /* server port matches */
		   /* proxy (peerID) matches */
		   (((peerID == NULL) && (sid->peerID == NULL)) ||
		    ((peerID != NULL) && (sid->peerID != NULL) &&
		     PORT_Strcmp(sid->peerID, peerID) == 0)) &&
		   /* is cacheable */
		   (sid->version < SSL_LIBRARY_VERSION_3_0 ||
		    sid->u.ssl3.resumable) &&
		   /* server hostname matches. */
	           (sid->urlSvrName != NULL) &&
		   ((0 == PORT_Strcmp(urlSvrName, sid->urlSvrName)) ||
		    ((sid->peerCert != NULL) && (SECSuccess == 
		      CERT_VerifyCertName(sid->peerCert, urlSvrName))) )
		  ) {
	    /* Hit */
	    sid->references++;
	    break;
	} else {
	    sidp = &sid->next;
	}
    }
    UNLOCK_CACHE;
    return sid;
}

/*
** Add an sid to the cache or return a previously cached entry to the cache.
** Although this is static, it is called via ss->sec->cache().
*/
static void 
CacheSID(sslSessionID *sid)
{
    SSL_TRC(8, ("SSL: Cache: sid=0x%x cached=%d addr=0x%08x port=0x%04x "
		"time=%x cached=%d",
		sid, sid->cached, sid->addr, sid->port, sid->time,
		sid->cached));

    if (sid->cached == in_client_cache)
	return;

    /* XXX should be different trace for version 2 vs. version 3 */
    if (sid->version < SSL_LIBRARY_VERSION_3_0) {
	PRINT_BUF(8, (0, "sessionID:",
		  sid->u.ssl2.sessionID, sizeof(sid->u.ssl2.sessionID)));
	PRINT_BUF(8, (0, "masterKey:",
		  sid->u.ssl2.masterKey.data, sid->u.ssl2.masterKey.len));
	PRINT_BUF(8, (0, "cipherArg:",
		  sid->u.ssl2.cipherArg.data, sid->u.ssl2.cipherArg.len));
    } else {
	if (sid->u.ssl3.sessionIDLength == 0) 
	    return;
	PRINT_BUF(8, (0, "sessionID:",
		      sid->u.ssl3.sessionID, sid->u.ssl3.sessionIDLength));
    }

    /*
     * Put sid into the cache.  Bump reference count to indicate that
     * cache is holding a reference. Uncache will reduce the cache
     * reference.
     */
    LOCK_CACHE;
    sid->references++;
    sid->cached = in_client_cache;
    sid->next   = cache;
    cache       = sid;
    sid->time   = ssl_Time();
    UNLOCK_CACHE;
}

/* 
 * If sid "zap" is in the cache,
 *    removes sid from cache, and decrements reference count.
 * Caller must hold cache lock.
 */
static void
UncacheSID(sslSessionID *zap)
{
    sslSessionID **sidp = &cache;
    sslSessionID *sid;

    if (zap->cached != in_client_cache) {
	return;
    }

    SSL_TRC(8,("SSL: Uncache: zap=0x%x cached=%d addr=0x%08x port=0x%04x "
	       "time=%x cipher=%d",
	       zap, zap->cached, zap->addr, zap->port, zap->time,
	       zap->u.ssl2.cipherType));
    if (zap->version < SSL_LIBRARY_VERSION_3_0) {
	PRINT_BUF(8, (0, "sessionID:",
		      zap->u.ssl2.sessionID, sizeof(zap->u.ssl2.sessionID)));
	PRINT_BUF(8, (0, "masterKey:",
		      zap->u.ssl2.masterKey.data, zap->u.ssl2.masterKey.len));
	PRINT_BUF(8, (0, "cipherArg:",
		      zap->u.ssl2.cipherArg.data, zap->u.ssl2.cipherArg.len));
    }

    /* See if it's in the cache, if so nuke it */
    while ((sid = *sidp) != 0) {
	if (sid == zap) {
	    /*
	    ** Bingo. Reduce reference count by one so that when
	    ** everyone is done with the sid we can free it up.
	    */
	    *sidp = zap->next;
	    zap->cached = invalid_cache;
	    ssl_FreeLockedSID(zap);
	    return;
	}
	sidp = &sid->next;
    }
}

/* If sid "zap" is in the cache,
 *    removes sid from cache, and decrements reference count.
 * Although this function is static, it is called externally via 
 *    ss->sec->uncache().
 */
static void
LockAndUncacheSID(sslSessionID *zap)
{
    LOCK_CACHE;
    UncacheSID(zap);
    UNLOCK_CACHE;

}

/* choose client or server cache functions for this sslsocket. */
void 
ssl_ChooseSessionIDProcs(sslSecurityInfo *sec)
{
    if (sec->isServer) {
	sec->cache   = ssl_sid_cache;
	sec->uncache = ssl_sid_uncache;
    } else {
	sec->cache   = CacheSID;
	sec->uncache = LockAndUncacheSID;
    }
}

/* wipe out the entire client session cache. */
void
SSL_ClearSessionCache(void)
{
    LOCK_CACHE;
    while(cache != NULL)
	UncacheSID(cache);
    UNLOCK_CACHE;
}

PRUint32
ssl_Time(void)
{
    PRTime now;
    PRInt64 ll;
    PRUint32 time;

    now = PR_Now();
    LL_I2L(ll, 1000000L);
    LL_DIV(now, now, ll);
    LL_L2UI(time, now);
    return time;
}

