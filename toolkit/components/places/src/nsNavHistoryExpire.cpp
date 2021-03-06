//* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is Mozilla History System
 *
 * The Initial Developer of the Original Code is
 * Google Inc.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Brett Wilson <brettw@gmail.com> (original author)
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

/**
 * This class handles expiration of history for nsNavHistory. There is a 1-1
 * mapping between nsNavHistory class and a nsNavHistoryExpire class, the
 * code is separated for better understandability.
 */

#include "nsNavHistory.h"
#include "mozStorageHelper.h"
#include "nsNetUtil.h"
#include "nsIAnnotationService.h"
#include "nsPrintfCString.h"

struct nsNavHistoryExpireRecord {
  nsNavHistoryExpireRecord(mozIStorageStatement* statement);

  PRInt64 visitID;
  PRInt64 placeID;
  PRTime visitDate;
  nsCString uri;
  PRInt64 faviconID;
  PRBool hidden;
  PRBool bookmarked;
  PRBool erased; // set to true if/when the history entry is erased
};

// Number of things we'll expire at once. Runtime of expiration is approximately
// linear with the number of things we expire at once. This number was picked so
// we expire "several" things at once, but still run quickly. Just doing 3
// expirations at once isn't much faster than 6 due to constant overhead of
// running the query.
#define EXPIRATION_COUNT_PER_RUN 6

// Larger expiration chunk for idle time and shutdown.
#define EXPIRATION_COUNT_PER_RUN_LARGE 50

// The time in ms to wait after AddURI to try expiration of pages. Short is
// actually better. If expiration takes an unusually long period of time, it
// will interfere with video playback in the browser, for example. Such a blip
// is not likely to be noticable when the page has just appeared.
#define PARTIAL_EXPIRATION_TIMEOUT 3500

// The time in ms to wait after the initial expiration run for additional ones
#define SUBSEQUENT_EXPIRATION_TIMEOUT 20000

// Number of expirations we'll do after the most recent page is loaded before
// stopping. We don't want to keep the computer chugging forever expiring
// annotations if the user stopped using the browser.
//
// This current value of one prevents history expiration while the page is
// being shown, because expiration may interfere with media playback.
#define MAX_SEQUENTIAL_RUNS 1

// Sanitization preferences
#define PREF_SANITIZE_ON_SHUTDOWN   "privacy.sanitize.sanitizeOnShutdown"
#define PREF_SANITIZE_ITEM_HISTORY  "privacy.item.history"

// Expiration policy amounts (in microseconds)
const PRTime EXPIRATION_POLICY_DAYS = ((PRTime)7 * 86400 * PR_USEC_PER_SEC);
const PRTime EXPIRATION_POLICY_WEEKS = ((PRTime)30 * 86400 * PR_USEC_PER_SEC);
const PRTime EXPIRATION_POLICY_MONTHS = ((PRTime)180 * 86400 * PR_USEC_PER_SEC);

// Expiration policy for embedded links (bug #401722)
const PRTime EMBEDDED_LINK_LIFETIME = ((PRTime)1 * 86400 * PR_USEC_PER_SEC);

// Expiration cap for embedded visits
#define EXPIRATION_CAP_EMBEDDED 500

// Expiration cap for dangling moz_places records
#define EXPIRATION_CAP_PLACES 500

// History preferences
#define PREF_BRANCH_BASE                        "browser."
#define PREF_BROWSER_HISTORY_EXPIRE_DAYS        "history_expire_days"

// nsNavHistoryExpire::nsNavHistoryExpire
//
//    Warning: don't do anything with aHistory in the constructor, since
//    this is a member of the nsNavHistory, it is still being constructed
//    when this is called.

nsNavHistoryExpire::nsNavHistoryExpire(nsNavHistory* aHistory) :
    mHistory(aHistory),
    mTimerSet(PR_FALSE),
    mAnyEmptyRuns(PR_FALSE),
    mNextExpirationTime(0),
    mAddCount(0),
    mExpiredItems(0)
{

}


// nsNavHistoryExpire::~nsNavHistoryExpire

nsNavHistoryExpire::~nsNavHistoryExpire()
{

}


// nsNavHistoryExpire::OnAddURI
//
//    Called by history when a URI is added to history. This starts the timer
//    for when we are going to expire.
//
//    The current time is passed in by the history service as an optimization.
//    The AddURI function has already computed the proper time, and getting the
//    time again from the OS is nontrivial.

void
nsNavHistoryExpire::OnAddURI(PRTime aNow)
{
  mAddCount ++;

  if (mTimer && mTimerSet) {
    mTimer->Cancel();
    mTimerSet = PR_FALSE;
  }

  if (mNextExpirationTime != 0 && aNow < mNextExpirationTime)
    return; // we know there's nothing to expire yet

  StartTimer(PARTIAL_EXPIRATION_TIMEOUT);
}

// nsNavHistoryExpire::OnDeleteURI
//
//    Called by history when a URI is deleted from history.
//    This kicks off an expiration of annotations.
//
void
nsNavHistoryExpire::OnDeleteURI()
{
  mozIStorageConnection* connection = mHistory->GetStorageConnection();
  if (!connection) {
    NS_NOTREACHED("No connection");
    return;
  }
  nsresult rv = ExpireAnnotations(connection);
  if (NS_FAILED(rv))
    NS_WARNING("ExpireAnnotations failed.");
}

// nsNavHistoryExpire::OnQuit
//
//    Here we check for some edge cases and fix them

void
nsNavHistoryExpire::OnQuit()
{
  mozIStorageConnection* connection = mHistory->GetStorageConnection();
  if (!connection) {
    NS_NOTREACHED("No connection");
    return;
  }

  // Need to cancel any pending timers so we don't try to expire during shutdown
  if (mTimer)
    mTimer->Cancel();

  // Handle degenerate runs:
  nsresult rv = ExpireForDegenerateRuns();
  if (NS_FAILED(rv))
    NS_WARNING("ExpireForDegenerateRuns failed.");

  // Expire embedded links
  // NOTE: This must come before ExpireHistoryParanoid, or the moz_places
  // records won't be immediately deleted.
  rv = ExpireEmbeddedLinks(connection);
  if (NS_FAILED(rv))
    NS_WARNING("ExpireEmbeddedLinks failed.");

  // Determine whether we can skip partially expiration of dangling entries
  // because we be doing a full expiration on shutdown in ClearHistory()
  nsCOMPtr<nsIPrefBranch> prefs(do_GetService("@mozilla.org/preferences-service;1"));
  PRBool sanitizeOnShutdown, sanitizeHistory;
  prefs->GetBoolPref(PREF_SANITIZE_ON_SHUTDOWN, &sanitizeOnShutdown);
  prefs->GetBoolPref(PREF_SANITIZE_ITEM_HISTORY, &sanitizeHistory);
  if (sanitizeHistory && sanitizeOnShutdown)
    return;

  // vacuum up dangling items
  rv = ExpireHistoryParanoid(connection, EXPIRATION_CAP_PLACES);
  if (NS_FAILED(rv))
    NS_WARNING("ExpireHistoryParanoid failed.");
  rv = ExpireFaviconsParanoid(connection);
  if (NS_FAILED(rv))
    NS_WARNING("ExpireFaviconsParanoid failed.");
  rv = ExpireAnnotationsParanoid(connection);
  if (NS_FAILED(rv))
    NS_WARNING("ExpireAnnotationsParanoid failed.");
  rv = ExpireInputHistoryParanoid(connection);
  if (NS_FAILED(rv))
    NS_WARNING("ExpireInputHistoryParanoid failed.");
}


// nsNavHistoryExpire::ClearHistory
//
//    Performance: ExpireItems sends notifications. We may want to disable this
//    for clear history cases. However, my initial tests show that the
//    notifications are not a significant part of clear history time.

nsresult
nsNavHistoryExpire::ClearHistory()
{
  mozIStorageConnection* connection = mHistory->GetStorageConnection();
  NS_ENSURE_TRUE(connection, NS_ERROR_OUT_OF_MEMORY);

  mozStorageTransaction transaction(connection, PR_FALSE);

  // reset frecency for all items that will _not_ be deleted
  // Note, we set frecency to -visit_count since we use that value in our
  // idle query to figure out which places to recalcuate frecency first.
  // We must do this before deleting visits
  nsresult rv = connection->ExecuteSimpleSQL(NS_LITERAL_CSTRING(
    "UPDATE moz_places SET frecency = -MAX(visit_count, 1) "
    "WHERE id IN("
      "SELECT h.id FROM moz_places h WHERE "
        "EXISTS (SELECT id FROM moz_bookmarks WHERE fk = h.id) "
        "OR EXISTS "
        "(SELECT id FROM moz_annos WHERE place_id = h.id AND expiration = ") +
      nsPrintfCString("%d", nsIAnnotationService::EXPIRE_NEVER) +
      NS_LITERAL_CSTRING(")"));
  if (NS_FAILED(rv))
    NS_WARNING("failed to recent frecency");

  // expire visits, then let the paranoid functions do the cleanup for us
  rv = connection->ExecuteSimpleSQL(NS_LITERAL_CSTRING(
      "DELETE FROM moz_historyvisits"));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = ExpireHistoryParanoid(connection, -1);
  if (NS_FAILED(rv))
    NS_WARNING("ExpireHistoryParanoid failed.");

  rv = ExpireFaviconsParanoid(connection);
  if (NS_FAILED(rv))
    NS_WARNING("ExpireFaviconsParanoid failed.");

  rv = ExpireAnnotationsParanoid(connection);
  if (NS_FAILED(rv))
    NS_WARNING("ExpireAnnotationsParanoid failed.");

  rv = ExpireInputHistoryParanoid(connection);
  if (NS_FAILED(rv))
    NS_WARNING("ExpireInputHistoryParanoid failed.");

  // some of the remaining places could be place: urls or
  // unvisited livemark items, so setting the frecency to -1
  // will cause them to show up in the url bar autocomplete
  // call FixInvalidFrecenciesForExcludedPlaces() to handle that scenario
  rv = mHistory->FixInvalidFrecenciesForExcludedPlaces();
  if (NS_FAILED(rv))
    NS_WARNING("failed to fix invalid frecencies");

  rv = transaction.Commit();
  NS_ENSURE_SUCCESS(rv, rv);

  // XXX todo
  // forcibly call the "on idle" timer here to do a little work
  // but the rest will happen on idle.

  ENUMERATE_WEAKARRAY(mHistory->mObservers, nsINavHistoryObserver,
                      OnClearHistory())

  return NS_OK;
}


// nsNavHistoryExpire::OnExpirationChanged
//
//    Called when the expiration length in days has changed. We clear any
//    next expiration time, meaning that we'll try to expire stuff next time,
//    and recompute the value if there's still nothing to expire.

void
nsNavHistoryExpire::OnExpirationChanged()
{
  mNextExpirationTime = 0;
  // kick off expiration
  (void)OnAddURI(PR_Now());
}


// nsNavHistoryExpire::DoPartialExpiration

nsresult
nsNavHistoryExpire::DoPartialExpiration()
{
  // expire history items
  PRBool keepGoing;
  nsresult rv = ExpireItems(EXPIRATION_COUNT_PER_RUN, &keepGoing);
  if (NS_FAILED(rv))
    NS_WARNING("ExpireItems failed.");
  else if (keepGoing)
    StartTimer(SUBSEQUENT_EXPIRATION_TIMEOUT);
  return NS_OK;
}


// nsNavHistoryExpire::ExpireItems
//
//    Here, we try to expire aNumToExpire items and their associated data,
//    If we expired things and then stopped because we hit this limit,
//    aKeepGoing will be set indicating we should keep expiring. If we ran
//    out of things to expire, it will be unset indicating we should wait.
//
//    As a special case, aNumToExpire can be 0 and we'll expire everything
//    in history.

nsresult
nsNavHistoryExpire::ExpireItems(PRUint32 aNumToExpire, PRBool* aKeepGoing)
{
  mozIStorageConnection* connection = mHistory->GetStorageConnection();
  NS_ENSURE_TRUE(connection, NS_ERROR_OUT_OF_MEMORY);

  // This transaction is important for performance. It makes the DB flush
  // everything to disk in one larger operation rather than many small ones.
  // Note that this transaction always commits.
  mozStorageTransaction transaction(connection, PR_FALSE);

  *aKeepGoing = PR_TRUE;

  PRInt64 expireTime;
  if (aNumToExpire == 0) {
    // special case: erase all history
    expireTime = 0;
  } else {
    expireTime = PR_Now() - GetExpirationTimeAgo(mHistory->mExpireDaysMax);
  }

  // find some visits to expire
  nsTArray<nsNavHistoryExpireRecord> expiredVisits;
  nsresult rv = FindVisits(expireTime, aNumToExpire, connection,
                           expiredVisits);
  NS_ENSURE_SUCCESS(rv, rv);

  // if we didn't find as many things to expire as we could have, then
  // we should note the next time we need to expire.
  if (expiredVisits.Length() < aNumToExpire) {
    *aKeepGoing = PR_FALSE;
    ComputeNextExpirationTime(connection);

    if (expiredVisits.Length() == 0) {
      // Nothing to expire. Set the flag so we know we don't have to do any
      // work on shutdown.
      mAnyEmptyRuns = PR_TRUE;
      return NS_OK;
    }
  }
  mExpiredItems += expiredVisits.Length();

  rv = EraseVisits(connection, expiredVisits);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = EraseHistory(connection, expiredVisits);
  NS_ENSURE_SUCCESS(rv, rv);

  // send observer messages
  nsCOMPtr<nsIURI> uri;
  for (PRUint32 i = 0; i < expiredVisits.Length(); i ++) {
    rv = NS_NewURI(getter_AddRefs(uri), expiredVisits[i].uri);
    if (NS_FAILED(rv)) continue;

    // FIXME bug 325241 provide a way to observe hidden elements
    if (expiredVisits[i].hidden) continue;

    ENUMERATE_WEAKARRAY(mHistory->mObservers, nsINavHistoryObserver,
                        OnPageExpired(uri, expiredVisits[i].visitDate,
                                      expiredVisits[i].erased));
  }

  // don't worry about errors here, it doesn't affect our ability to continue
  rv = EraseFavicons(connection, expiredVisits);
  if (NS_FAILED(rv))
    NS_WARNING("EraseFavicons failed.");

  rv = EraseAnnotations(connection, expiredVisits);
  if (NS_FAILED(rv))
    NS_WARNING("EraseAnnotations failed.");

  // expire annotations by policy
  rv = ExpireAnnotations(connection);
  if (NS_FAILED(rv))
    NS_WARNING("ExpireAnnotations failed.");

  rv = transaction.Commit();
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}


// nsNavHistoryExpireRecord::nsNavHistoryExpireRecord
//
//    Statement should be the one created in FindVisits. The parameters must
//    agree.

nsNavHistoryExpireRecord::nsNavHistoryExpireRecord(
                                              mozIStorageStatement* statement)
{
  visitID = statement->AsInt64(0);
  placeID = statement->AsInt64(1);
  visitDate = statement->AsInt64(2);
  statement->GetUTF8String(3, uri);
  faviconID = statement->AsInt64(4);
  hidden = (statement->AsInt32(5) > 0);
  bookmarked = (statement->AsInt32(6) > 0);
  erased = PR_FALSE;
}


// nsNavHistoryExpire::FindVisits
//
//    Find visits to expire, meeting the following criteria:
//
//    * With a visit date older than browser.history_expire_days ago.
//    * With a visit date older than browser.history_expire_days_min ago
//      if we have more than browser.history_expire_sites unique urls.
//
//    aExpireThreshold is the time at which we will delete visits before.
//    If it is zero, we will match everything.
//
//    aNumToExpire is the maximum number of visits to find. If it is 0, then
//    we will get all matching visits.

nsresult
nsNavHistoryExpire::FindVisits(PRTime aExpireThreshold, PRUint32 aNumToExpire,
                               mozIStorageConnection* aConnection,
                               nsTArray<nsNavHistoryExpireRecord>& aRecords)
{
  // Select a limited number of visits older than a time
  nsCOMPtr<mozIStorageStatement> selectStatement;
  nsresult rv = aConnection->CreateStatement(NS_LITERAL_CSTRING(
      "SELECT v.id, v.place_id, v.visit_date, h.url, h.favicon_id, h.hidden, "
        "(SELECT fk FROM moz_bookmarks WHERE fk = h.id) "
      "FROM moz_places h JOIN moz_historyvisits v ON h.id = v.place_id "
      "WHERE v.visit_date < ?1 "
      "ORDER BY v.visit_date ASC LIMIT ?2"),
    getter_AddRefs(selectStatement));
    NS_ENSURE_SUCCESS(rv, rv);

  // browser.history_expire_days || match all visits
  PRTime expireMaxTime = aExpireThreshold ? aExpireThreshold : LL_MAXINT;
  rv = selectStatement->BindInt64Parameter(0, expireMaxTime);
  NS_ENSURE_SUCCESS(rv, rv);

  // use LIMIT -1 to not limit
  PRInt32 numToExpire = aNumToExpire ? aNumToExpire : -1;
  rv = selectStatement->BindInt64Parameter(1, numToExpire);
  NS_ENSURE_SUCCESS(rv, rv);

  PRBool hasMore = PR_FALSE;
  while (NS_SUCCEEDED(selectStatement->ExecuteStep(&hasMore)) && hasMore) {
    nsNavHistoryExpireRecord record(selectStatement);
    aRecords.AppendElement(record);
  }

  // If we have found less than aNumToExpire over-max-age records, and we are
  // over the unique urls cap, select records older than the min-age cap .
  if (aRecords.Length() < aNumToExpire) {
    // check the number of visited unique urls in the db.
    nsCOMPtr<mozIStorageStatement> countStatement;
    rv = aConnection->CreateStatement(NS_LITERAL_CSTRING(
        "SELECT count(*) FROM moz_places WHERE visit_count > 0"),
      getter_AddRefs(countStatement));
    NS_ENSURE_SUCCESS(rv, rv);

    hasMore = PR_FALSE;
    // initialize to mExpiresites to avoid expiring if something goes wrong
    PRInt32 pageCount = mHistory->mExpireSites;
    if (NS_SUCCEEDED(countStatement->ExecuteStep(&hasMore)) && hasMore) {
      rv = countStatement->GetInt32(0, &pageCount);
      NS_ENSURE_SUCCESS(rv, rv);
    }

    // Don't find any more pages to expire if we have not reached the urls cap
    if (pageCount <= mHistory->mExpireSites)
        return NS_OK;

    rv = selectStatement->Reset();
    NS_ENSURE_SUCCESS(rv, rv);

    // browser.history_expire_days_min
    PRTime expireMinTime = PR_Now() -
                           GetExpirationTimeAgo(mHistory->mExpireDaysMin);
    rv = selectStatement->BindInt64Parameter(0, expireMinTime);
    NS_ENSURE_SUCCESS(rv, rv);
    
    numToExpire = aNumToExpire - aRecords.Length();
    rv = selectStatement->BindInt64Parameter(1, numToExpire);
    NS_ENSURE_SUCCESS(rv, rv);

    hasMore = PR_FALSE;
    while (NS_SUCCEEDED(selectStatement->ExecuteStep(&hasMore)) && hasMore) {
      nsNavHistoryExpireRecord record(selectStatement);
      aRecords.AppendElement(record);
    }
  }

  return NS_OK;
}


// nsNavHistoryExpire::EraseVisits

nsresult
nsNavHistoryExpire::EraseVisits(mozIStorageConnection* aConnection,
    const nsTArray<nsNavHistoryExpireRecord>& aRecords)
{
  // build a comma separated string of visit ids to delete
  // also build a comma separated string of place ids to reset frecency
  nsCString deletedVisitIds;
  nsCString placeIds;
  nsTArray<PRInt64> deletedPlaceIdsArray, deletedVisitIdsArray;
  for (PRUint32 i = 0; i < aRecords.Length(); i ++) {
    // Do not add comma separator for the first visit id
    if (deletedVisitIdsArray.IndexOf(aRecords[i].visitID) == -1) {
      if (!deletedVisitIds.IsEmpty())
        deletedVisitIds.AppendLiteral(",");  
      deletedVisitIds.AppendInt(aRecords[i].visitID);
    }

    // Do not add comma separator for the first place id
    if (deletedPlaceIdsArray.IndexOf(aRecords[i].placeID) == -1) {
      if (!placeIds.IsEmpty())
        placeIds.AppendLiteral(",");
      placeIds.AppendInt(aRecords[i].placeID);
    }
  }

  if (deletedVisitIds.IsEmpty())
    return NS_OK;

  // Reset the frecencies for the places that won't have any visits after
  // we delete them and make sure they aren't bookmarked either. This means we
  // keep the old frecencies when possible as an estimate for the new frecency
  // unless we know it has to be invalidated.
  // We must do this before deleting visits
  nsresult rv = aConnection->ExecuteSimpleSQL(
    NS_LITERAL_CSTRING(
      "UPDATE moz_places "
      "SET frecency = -MAX(visit_count, 1) "
      "WHERE id IN ("
        "SELECT h.id FROM moz_places h "
        "WHERE NOT EXISTS (SELECT b.id FROM moz_bookmarks b WHERE b.fk = h.id) "
          "AND NOT EXISTS "
            "(SELECT v.id FROM moz_historyvisits v WHERE v.place_id = h.id AND "
              "v.id NOT IN (") + deletedVisitIds +
              NS_LITERAL_CSTRING(")) AND "
              "h.id IN (") + placeIds +
    NS_LITERAL_CSTRING("))"));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = aConnection->ExecuteSimpleSQL(
    NS_LITERAL_CSTRING("DELETE FROM moz_historyvisits WHERE id IN (") +
    deletedVisitIds +
    NS_LITERAL_CSTRING(")"));
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}


// nsNavHistoryExpire::EraseHistory
//
//    This erases records in moz_places when there are no more visits.
//    We need to be careful not to delete: bookmarks, items that still have
//    visits and place: URIs.
//
//    This will modify the input by setting the erased flag on each of the
//    array elements according to whether the history item was erased or not.

nsresult
nsNavHistoryExpire::EraseHistory(mozIStorageConnection* aConnection,
    nsTArray<nsNavHistoryExpireRecord>& aRecords)
{
  // build a comma separated string of place ids to delete
  nsCString deletedPlaceIds;
  nsTArray<PRInt64> deletedPlaceIdsArray;
  for (PRUint32 i = 0; i < aRecords.Length(); i ++) {
    // IF bookmarked entries OR "place" URIs do not delete
    if (aRecords[i].bookmarked ||
        StringBeginsWith(aRecords[i].uri, NS_LITERAL_CSTRING("place:")))
      continue;
    // avoid trying to delete the same place id twice
    if (deletedPlaceIdsArray.IndexOf(aRecords[i].placeID) == -1) {
      // Do not add comma separator for the first entry
      if (!deletedPlaceIds.IsEmpty())
        deletedPlaceIds.AppendLiteral(",");
      deletedPlaceIdsArray.AppendElement(aRecords[i].placeID);
      deletedPlaceIds.AppendInt(aRecords[i].placeID);
    }
    aRecords[i].erased = PR_TRUE;
  }

  if (deletedPlaceIds.IsEmpty())
    return NS_OK;

  return aConnection->ExecuteSimpleSQL(
    NS_LITERAL_CSTRING("DELETE FROM moz_places WHERE id IN( "
      "SELECT h.id "
      "FROM moz_places h "
      "WHERE h.id IN(") +
        deletedPlaceIds +
        NS_LITERAL_CSTRING(") AND NOT EXISTS "
          "(SELECT id FROM moz_historyvisits WHERE place_id = h.id LIMIT 1) "
          "AND NOT EXISTS "
          "(SELECT id FROM moz_bookmarks WHERE fk = h.id LIMIT 1) "
          "AND SUBSTR(h.url, 1, 6) <> 'place:')"));
}


// nsNavHistoryExpire::EraseFavicons

nsresult
nsNavHistoryExpire::EraseFavicons(mozIStorageConnection* aConnection,
    const nsTArray<nsNavHistoryExpireRecord>& aRecords)
{
  // build a comma separated string of favicon ids to delete
  nsCString deletedFaviconIds;
  nsTArray<PRInt64> deletedFaviconIdsArray;  
  for (PRUint32 i = 0; i < aRecords.Length(); i ++) {
    // IF main entry not expired OR no favicon DO NOT DELETE
    if (!aRecords[i].erased || aRecords[i].faviconID == 0)
      continue;
    // avoid trying to delete the same favicon id twice
    if (deletedFaviconIdsArray.IndexOf(aRecords[i].faviconID) == -1) {
      // Do not add comma separator for the first entry
      if (!deletedFaviconIds.IsEmpty())
        deletedFaviconIds.AppendLiteral(",");
      deletedFaviconIdsArray.AppendElement(aRecords[i].faviconID);
      deletedFaviconIds.AppendInt(aRecords[i].faviconID);
    }
  }

  if (deletedFaviconIds.IsEmpty())
    return NS_OK;

  // delete only if id is not referenced in moz_places
  return aConnection->ExecuteSimpleSQL(
    NS_LITERAL_CSTRING("DELETE FROM moz_favicons WHERE id IN ( "
      "SELECT f.id FROM moz_favicons f "
      "LEFT OUTER JOIN moz_places h ON f.id = h.favicon_id "
      "WHERE f.id IN (") + deletedFaviconIds +
      NS_LITERAL_CSTRING(") AND h.favicon_id IS NULL)"));
}


// nsNavHistoryExpire::EraseAnnotations

nsresult
nsNavHistoryExpire::EraseAnnotations(mozIStorageConnection* aConnection,
    const nsTArray<nsNavHistoryExpireRecord>& aRecords)
{
  // remove annotations for the set of records passed in
  nsCString placeIds;
  nsTArray<PRInt64> deletedPlaceIdsArray;
  for (PRUint32 i = 0; i < aRecords.Length(); i ++) {
    // avoid trying to delete the same place id twice
    if (deletedPlaceIdsArray.IndexOf(aRecords[i].placeID) == -1) {
      // Do not add comma separator for the first entry
      if (!placeIds.IsEmpty())
        placeIds.AppendLiteral(",");
      deletedPlaceIdsArray.AppendElement(aRecords[i].placeID);
      placeIds.AppendInt(aRecords[i].placeID);
    }
  }
  
  if (placeIds.IsEmpty())
    return NS_OK;
    
  nsresult rv = aConnection->ExecuteSimpleSQL(NS_LITERAL_CSTRING(
    "DELETE FROM moz_annos WHERE place_id in (") +
      placeIds + NS_LITERAL_CSTRING(") AND expiration != ") +
      nsPrintfCString("%d", nsIAnnotationService::EXPIRE_NEVER));
  NS_ENSURE_SUCCESS(rv, rv);
  return NS_OK;
}

// nsAnnotationService::ExpireAnnotations
//
//    Periodic expiration of annotations that have time-sensitive
//    expiration policies.
//
//    NOTE: Always specify the exact policy constant, as they're
//    not guaranteed to be in numerical order.
//
nsresult
nsNavHistoryExpire::ExpireAnnotations(mozIStorageConnection* aConnection)
{
  mozStorageTransaction transaction(aConnection, PR_FALSE);

  // Note: The COALESCE is used to cover a short period where NULLs were inserted
  // into the lastModified column.
  PRTime now = PR_Now();
  nsCOMPtr<mozIStorageStatement> expirePagesStatement;
  nsresult rv = aConnection->CreateStatement(NS_LITERAL_CSTRING(
      "DELETE FROM moz_annos WHERE expiration = ?1 AND (?2 > MAX(COALESCE(lastModified, 0), dateAdded))"),
    getter_AddRefs(expirePagesStatement));
  NS_ENSURE_SUCCESS(rv, rv);
  nsCOMPtr<mozIStorageStatement> expireItemsStatement;
  rv = aConnection->CreateStatement(NS_LITERAL_CSTRING(
      "DELETE FROM moz_items_annos WHERE expiration = ?1 AND (?2 > MAX(COALESCE(lastModified, 0), dateAdded))"),
    getter_AddRefs(expireItemsStatement));
  NS_ENSURE_SUCCESS(rv, rv);

  // remove days annos
  rv = expirePagesStatement->BindInt32Parameter(0, nsIAnnotationService::EXPIRE_DAYS);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = expirePagesStatement->BindInt64Parameter(1, (now - EXPIRATION_POLICY_DAYS));
  NS_ENSURE_SUCCESS(rv, rv);
  rv = expirePagesStatement->Execute();
  NS_ENSURE_SUCCESS(rv, rv);
  
  // remove days item annos
  rv = expireItemsStatement->BindInt32Parameter(0, nsIAnnotationService::EXPIRE_DAYS);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = expireItemsStatement->BindInt64Parameter(1, (now - EXPIRATION_POLICY_DAYS));
  NS_ENSURE_SUCCESS(rv, rv);
  rv = expireItemsStatement->Execute();
  NS_ENSURE_SUCCESS(rv, rv);

  // remove weeks annos
  rv = expirePagesStatement->BindInt32Parameter(0, nsIAnnotationService::EXPIRE_WEEKS);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = expirePagesStatement->BindInt64Parameter(1, (now - EXPIRATION_POLICY_WEEKS));
  NS_ENSURE_SUCCESS(rv, rv);
  rv = expirePagesStatement->Execute();
  NS_ENSURE_SUCCESS(rv, rv);

  // remove weeks item annos
  rv = expireItemsStatement->BindInt32Parameter(0, nsIAnnotationService::EXPIRE_WEEKS);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = expireItemsStatement->BindInt64Parameter(1, (now - EXPIRATION_POLICY_WEEKS));
  NS_ENSURE_SUCCESS(rv, rv);
  rv = expireItemsStatement->Execute();
  NS_ENSURE_SUCCESS(rv, rv);

  // remove months annos
  rv = expirePagesStatement->BindInt32Parameter(0, nsIAnnotationService::EXPIRE_MONTHS);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = expirePagesStatement->BindInt64Parameter(1, (now - EXPIRATION_POLICY_MONTHS));
  NS_ENSURE_SUCCESS(rv, rv);
  rv = expirePagesStatement->Execute();
  NS_ENSURE_SUCCESS(rv, rv);

  // remove months item annos
  rv = expireItemsStatement->BindInt32Parameter(0, nsIAnnotationService::EXPIRE_MONTHS);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = expireItemsStatement->BindInt64Parameter(1, (now - EXPIRATION_POLICY_MONTHS));
  NS_ENSURE_SUCCESS(rv, rv);
  rv = expireItemsStatement->Execute();
  NS_ENSURE_SUCCESS(rv, rv);

  // remove EXPIRE_WITH_HISTORY annos for pages without visits
  rv = aConnection->ExecuteSimpleSQL(NS_LITERAL_CSTRING(
      "DELETE FROM moz_annos WHERE expiration = ") +
        nsPrintfCString("%d", nsIAnnotationService::EXPIRE_WITH_HISTORY) +
        NS_LITERAL_CSTRING(" AND NOT EXISTS "
          "(SELECT id FROM moz_historyvisits "
          "WHERE place_id = moz_annos.place_id LIMIT 1)"));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = transaction.Commit();
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}


// nsNavHistoryExpire::ExpireEmbeddedLinks

nsresult
nsNavHistoryExpire::ExpireEmbeddedLinks(mozIStorageConnection* aConnection)
{
  PRTime maxEmbeddedAge = PR_Now() - EMBEDDED_LINK_LIFETIME;
  nsCOMPtr<mozIStorageStatement> expireEmbeddedLinksStatement;
  // Note: This query also removes visit_type = 0 entries, for bug #375777.
  nsresult rv = aConnection->CreateStatement(NS_LITERAL_CSTRING(
    "DELETE FROM moz_historyvisits WHERE id IN ("
      "SELECT id FROM moz_historyvisits WHERE visit_date < ?1 "
      "AND (visit_type = ?2 OR visit_type = 0) LIMIT ?3)"),
    getter_AddRefs(expireEmbeddedLinksStatement));
  NS_ENSURE_SUCCESS(rv, rv);
  rv = expireEmbeddedLinksStatement->BindInt64Parameter(0, maxEmbeddedAge);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = expireEmbeddedLinksStatement->BindInt32Parameter(1, nsINavHistoryService::TRANSITION_EMBED);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = expireEmbeddedLinksStatement->BindInt32Parameter(2, EXPIRATION_CAP_EMBEDDED);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = expireEmbeddedLinksStatement->Execute();
  NS_ENSURE_SUCCESS(rv, rv);
  return NS_OK;
}


// nsNavHistoryExpire::ExpireHistoryParanoid
//
//    Deletes any dangling history entries that aren't associated with any
//    visits, bookmarks or "place:" URIs.
//
//    The aMaxRecords parameter is an optional cap on the number of 
//    records to delete. If it's value is -1, all records will be deleted.

nsresult
nsNavHistoryExpire::ExpireHistoryParanoid(mozIStorageConnection* aConnection,
                                          PRInt32 aMaxRecords)
{
  nsCAutoString query(
    "DELETE FROM moz_places WHERE id IN ("
      "SELECT h.id FROM moz_places h "
        "LEFT OUTER JOIN moz_historyvisits v ON h.id = v.place_id "
        "LEFT OUTER JOIN moz_bookmarks b ON h.id = b.fk "
      "WHERE v.id IS NULL AND b.id IS NULL AND SUBSTR(h.url, 1, 6) <> 'place:'");
  if (aMaxRecords != -1) {
    query.AppendLiteral(" LIMIT ");
    query.AppendInt(aMaxRecords);
  }
  query.AppendLiteral(")");
  nsresult rv = aConnection->ExecuteSimpleSQL(query);
  NS_ENSURE_SUCCESS(rv, rv);
  return NS_OK;
}


// nsNavHistoryExpire::ExpireFaviconsParanoid
//
//    Deletes any dangling favicons that aren't associated with any pages.

nsresult
nsNavHistoryExpire::ExpireFaviconsParanoid(mozIStorageConnection* aConnection)
{
  nsresult rv = aConnection->ExecuteSimpleSQL(NS_LITERAL_CSTRING(
    "DELETE FROM moz_favicons WHERE id IN "
    "(SELECT f.id FROM moz_favicons f "
     "LEFT OUTER JOIN moz_places h ON f.id = h.favicon_id "
     "WHERE h.favicon_id IS NULL)"));
  NS_ENSURE_SUCCESS(rv, rv);
  return rv;
}


// nsNavHistoryExpire::ExpireAnnotationsParanoid
//
//    Deletes session annotations, dangling annotations
//    and annotation names that are unused.

nsresult
nsNavHistoryExpire::ExpireAnnotationsParanoid(mozIStorageConnection* aConnection)
{
  // delete session annos
  nsCAutoString session_query = NS_LITERAL_CSTRING(
    "DELETE FROM moz_annos WHERE expiration = ") +
    nsPrintfCString("%d", nsIAnnotationService::EXPIRE_SESSION);
  nsresult rv = aConnection->ExecuteSimpleSQL(session_query);
  NS_ENSURE_SUCCESS(rv, rv);

  // delete all uri annos w/o a corresponding place id
  // or without any visits *and* not EXPIRE_NEVER.
  rv = aConnection->ExecuteSimpleSQL(NS_LITERAL_CSTRING(
    "DELETE FROM moz_annos WHERE id IN "
      "(SELECT a.id FROM moz_annos a "
      "LEFT OUTER JOIN moz_places p ON a.place_id = p.id "
      "LEFT OUTER JOIN moz_historyvisits v ON a.place_id = v.place_id "
      "WHERE p.id IS NULL "
      "OR (v.id IS NULL AND a.expiration != ") +
      nsPrintfCString("%d", nsIAnnotationService::EXPIRE_NEVER) +
      NS_LITERAL_CSTRING("))"));
  NS_ENSURE_SUCCESS(rv, rv);

  // XXX REMOVE ME LATE AFTER FINAL
  // Move current charset item annos to page annos.
  // There was a period in which EXPIRE_NEVER annos were undeletable
  // so we moved charset annos from pageAnnos to itemAnnos.
  // Since this has been fixed in RC1, we can use pageAnnos for charset
  // so we must revert old itemAnnos to pageAnnos.
  // see bug 317472 for details.
  nsCAutoString charsetAnno("URIProperties/characterSet");
  // In the migration query we use NULL as the id, since we don't know the
  // new id where the annotation will be inserted
  // The GROUP BY is needed because we have a unique index on annos tables
  // INSERT OR REPLACE is needed to overwrite existing values and not fail
  nsCOMPtr<mozIStorageStatement> migrateCharsetStatement;
  rv = aConnection->CreateStatement(NS_LITERAL_CSTRING(
        "INSERT OR REPLACE INTO moz_annos "
        "SELECT null, b.fk, t.anno_attribute_id, t.mime_type, t.content, "
          "t.flags, t.expiration, t.type, t.dateAdded, t.lastModified "
        "FROM moz_items_annos t "
          "JOIN moz_anno_attributes n ON t.anno_attribute_id = n.id "
          "JOIN moz_bookmarks b ON b.id = t.item_id "
        "WHERE n.name = ?1 "
        "GROUP BY b.fk, t.anno_attribute_id"),
      getter_AddRefs(migrateCharsetStatement));
  NS_ENSURE_SUCCESS(rv, rv);
  rv = migrateCharsetStatement->BindUTF8StringParameter(0, charsetAnno);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = migrateCharsetStatement->Execute();
  NS_ENSURE_SUCCESS(rv, rv);

  // delete old charset item annos
  nsCOMPtr<mozIStorageStatement> deleteCharsetStatement;
  rv = aConnection->CreateStatement(NS_LITERAL_CSTRING(
    "DELETE FROM moz_items_annos WHERE id IN "
      "(SELECT t.id FROM moz_items_annos t "
        "JOIN moz_anno_attributes n ON t.anno_attribute_id = n.id "
        "WHERE n.name = ?1)"),
    getter_AddRefs(deleteCharsetStatement));
  NS_ENSURE_SUCCESS(rv, rv);
  rv = deleteCharsetStatement->BindUTF8StringParameter(0, charsetAnno);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = deleteCharsetStatement->Execute();
  NS_ENSURE_SUCCESS(rv, rv);

  // delete item annos w/o a corresponding item id
  rv = aConnection->ExecuteSimpleSQL(NS_LITERAL_CSTRING(
    "DELETE FROM moz_items_annos WHERE id IN "
      "(SELECT a.id FROM moz_items_annos a "
      "LEFT OUTER JOIN moz_bookmarks b ON a.item_id = b.id "
      "WHERE b.id IS NULL)"));
  NS_ENSURE_SUCCESS(rv, rv);

  // delete all anno names w/o a corresponding uri or item entry
  rv = aConnection->ExecuteSimpleSQL(NS_LITERAL_CSTRING(
    "DELETE FROM moz_anno_attributes WHERE " 
    "id NOT IN (SELECT DISTINCT a.id FROM moz_anno_attributes a "
      "JOIN moz_annos b ON b.anno_attribute_id = a.id "
      "JOIN moz_places p ON b.place_id = p.id) "
    "AND "
    "id NOT IN (SELECT DISTINCT a.id FROM moz_anno_attributes a "
      "JOIN moz_items_annos c ON c.anno_attribute_id = a.id "
      "JOIN moz_bookmarks p ON c.item_id = p.id)"));
  NS_ENSURE_SUCCESS(rv, rv);
  return NS_OK;
}


// nsNavHistoryExpire::ExpireInputHistoryParanoid
//
//    Deletes dangling input history

nsresult
nsNavHistoryExpire::ExpireInputHistoryParanoid(mozIStorageConnection* aConnection)
{
  // Delete dangling input history that have no associated pages
  nsresult rv = aConnection->ExecuteSimpleSQL(NS_LITERAL_CSTRING(
    "DELETE FROM moz_inputhistory WHERE place_id IN "
    "(SELECT i.place_id FROM moz_inputhistory i "
      "LEFT OUTER JOIN moz_places h ON i.place_id = h.id "
      "WHERE h.id IS NULL)"));
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}


// nsNavHistoryExpire::ExpireForDegenerateRuns
//
//    This checks for potential degenerate runs. For example, a tinderbox
//    loads many web pages quickly and we'll never have a chance to expire.
//    Particularly crazy users might also do this. If we detect this, then we
//    want to force some expiration so history doesn't keep increasing.
//
//    Returns true if we did anything.

PRBool
nsNavHistoryExpire::ExpireForDegenerateRuns()
{
  // If there were any times that we didn't have anything to expire, this is
  // not a degenerate run.
  if (mAnyEmptyRuns)
    return PR_FALSE;

  // Expire a larger chunk of runs to catch up.
  PRBool keepGoing;
  nsresult rv = ExpireItems(EXPIRATION_COUNT_PER_RUN_LARGE, &keepGoing);
  if (NS_FAILED(rv))
    NS_WARNING("ExpireItems failed.");
  return PR_TRUE;
}


// nsNavHistoryExpire::ComputeNextExpirationTime
//
//    This computes mNextExpirationTime. See that var in the header file.
//    It is passed the number of microseconds that things expire in.

void
nsNavHistoryExpire::ComputeNextExpirationTime(
    mozIStorageConnection* aConnection)
{
  mNextExpirationTime = 0;

  nsCOMPtr<mozIStorageStatement> statement;
  nsresult rv = aConnection->CreateStatement(NS_LITERAL_CSTRING(
      "SELECT MIN(visit_date) FROM moz_historyvisits"),
    getter_AddRefs(statement));
  NS_ASSERTION(NS_SUCCEEDED(rv), "Could not create statement");
  if (NS_FAILED(rv)) return;

  PRBool hasMore;
  rv = statement->ExecuteStep(&hasMore);
  if (NS_FAILED(rv) || !hasMore)
    return; // no items, we'll leave mNextExpirationTime = 0 and try to expire
            // again next time

  PRTime minTime = statement->AsInt64(0);
  mNextExpirationTime = minTime + GetExpirationTimeAgo(mHistory->mExpireDaysMax);
}


// nsNavHistoryExpire::StartTimer

nsresult
nsNavHistoryExpire::StartTimer(PRUint32 aMilleseconds)
{
  if (!mTimer)
    mTimer = do_CreateInstance("@mozilla.org/timer;1");
  NS_ENSURE_STATE(mTimer); // returns on error
  nsresult rv = mTimer->InitWithFuncCallback(TimerCallback, this,
                                             aMilleseconds,
                                             nsITimer::TYPE_ONE_SHOT);
  NS_ENSURE_SUCCESS(rv, rv);
  return NS_OK;
}


// nsNavHistoryExpire::TimerCallback

void // static
nsNavHistoryExpire::TimerCallback(nsITimer* aTimer, void* aClosure)
{
  nsNavHistoryExpire* that = static_cast<nsNavHistoryExpire*>(aClosure);
  that->mTimerSet = PR_FALSE;
  that->DoPartialExpiration();
}


// nsNavHistoryExpire::GetExpirationTimeAgo

PRTime
nsNavHistoryExpire::GetExpirationTimeAgo(PRInt32 aExpireDays)
{
  // Prevent Int64 overflow for people that type in huge numbers.
  // This number is 2^63 / 24 / 60 / 60 / 1000000 (reversing the math below)
  const PRInt32 maxDays = 106751991;
  if (aExpireDays > maxDays)
    aExpireDays = maxDays;

  // compute how long ago to expire from
  // seconds per day = 86400 = 24*60*60
  return (PRTime)aExpireDays * 86400 * PR_USEC_PER_SEC;
}
