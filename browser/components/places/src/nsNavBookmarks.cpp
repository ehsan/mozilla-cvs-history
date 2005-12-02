/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is Places.
 *
 * The Initial Developer of the Original Code is
 * Google Inc.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Brian Ryner <bryner@brianryner.com> (original author)
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

#include "nsAppDirectoryServiceDefs.h"
#include "nsNavBookmarks.h"
#include "nsNavHistory.h"
#include "mozStorageHelper.h"
#include "nsIServiceManager.h"
#include "nsNetUtil.h"

const PRInt32 nsNavBookmarks::kFindBookmarksIndex_ItemChild = 0;
const PRInt32 nsNavBookmarks::kFindBookmarksIndex_FolderChild = 1;
const PRInt32 nsNavBookmarks::kFindBookmarksIndex_Parent = 2;
const PRInt32 nsNavBookmarks::kFindBookmarksIndex_Position = 3;

const PRInt32 nsNavBookmarks::kGetFolderInfoIndex_FolderID = 0;
const PRInt32 nsNavBookmarks::kGetFolderInfoIndex_Title = 1;

// These columns sit to the right of the kGetInfoIndex_* columns.
const PRInt32 nsNavBookmarks::kGetChildrenIndex_Position = 6;
const PRInt32 nsNavBookmarks::kGetChildrenIndex_ItemChild = 7;
const PRInt32 nsNavBookmarks::kGetChildrenIndex_FolderChild = 8;
const PRInt32 nsNavBookmarks::kGetChildrenIndex_FolderTitle = 9;

nsNavBookmarks* nsNavBookmarks::sInstance = nsnull;

struct UpdateBatcher
{
  UpdateBatcher() { nsNavBookmarks::GetBookmarksService()->BeginUpdateBatch(); }
  ~UpdateBatcher() { nsNavBookmarks::GetBookmarksService()->EndUpdateBatch(); }
};

nsNavBookmarks::nsNavBookmarks()
  : mRoot(0), mBookmarksRoot(0), mToolbarRoot(0), mBatchLevel(0)
{
  NS_ASSERTION(!sInstance, "Multiple nsNavBookmarks instances!");
  sInstance = this;
}

nsNavBookmarks::~nsNavBookmarks()
{
  NS_ASSERTION(sInstance == this, "Expected sInstance == this");
  sInstance = nsnull;
}

NS_IMPL_ISUPPORTS2(nsNavBookmarks,
                   nsINavBookmarksService, nsINavHistoryObserver)

nsresult
nsNavBookmarks::Init()
{
  nsresult rv;

  nsNavHistory *history = History();
  NS_ENSURE_TRUE(history, NS_ERROR_UNEXPECTED);
  history->AddObserver(this); // allows us to notify on title changes
  mozIStorageConnection *dbConn = DBConn();
  mozStorageTransaction transaction(dbConn, PR_FALSE);

  PRBool exists = PR_FALSE;
  dbConn->TableExists(NS_LITERAL_CSTRING("moz_bookmarks"), &exists);
  if (!exists) {
    rv = dbConn->ExecuteSimpleSQL(NS_LITERAL_CSTRING("CREATE TABLE moz_bookmarks (item_child INTEGER, folder_child INTEGER, parent INTEGER, position INTEGER)"));
    NS_ENSURE_SUCCESS(rv, rv);
  }

  dbConn->TableExists(NS_LITERAL_CSTRING("moz_bookmarks_folders"), &exists);
  if (!exists) {
    rv = dbConn->ExecuteSimpleSQL(NS_LITERAL_CSTRING("CREATE TABLE moz_bookmarks_folders (id INTEGER PRIMARY KEY, name LONGVARCHAR)"));
    NS_ENSURE_SUCCESS(rv, rv);
  }

  dbConn->TableExists(NS_LITERAL_CSTRING("moz_bookmarks_roots"), &exists);
  if (!exists) {
    rv = dbConn->ExecuteSimpleSQL(NS_LITERAL_CSTRING("CREATE TABLE moz_bookmarks_roots (root_name VARCHAR(16) UNIQUE, folder_id INTEGER)"));
    NS_ENSURE_SUCCESS(rv, rv);
  }

  rv = dbConn->CreateStatement(NS_LITERAL_CSTRING("SELECT id, name FROM moz_bookmarks_folders WHERE id = ?1"),
                               getter_AddRefs(mDBGetFolderInfo));
  NS_ENSURE_SUCCESS(rv, rv);

  {
    nsCOMPtr<mozIStorageStatement> statement;
    rv = dbConn->CreateStatement(NS_LITERAL_CSTRING("SELECT folder_child FROM moz_bookmarks WHERE parent IS NULL"),
                                 getter_AddRefs(statement));
    NS_ENSURE_SUCCESS(rv, rv);

    PRBool results;
    rv = statement->ExecuteStep(&results);
    NS_ENSURE_SUCCESS(rv, rv);
    if (results) {
      mRoot = statement->AsInt64(0);
    }
  }

  nsCAutoString buffer;

  nsCOMPtr<nsIStringBundleService> bundleService =
    do_GetService(NS_STRINGBUNDLE_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = bundleService->CreateBundle(
      "chrome://browser/locale/places/places.properties",
      getter_AddRefs(mBundle));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = dbConn->CreateStatement(NS_LITERAL_CSTRING("SELECT a.* FROM moz_bookmarks a, moz_history h WHERE h.url = ?1 AND a.item_child = h.id"),
                               getter_AddRefs(mDBFindURIBookmarks));
  NS_ENSURE_SUCCESS(rv, rv);

  // Construct a result where the first columns exactly match those returned by
  // mDBGetVisitPageInfo, and additionally contains columns for position,
  // item_child, and folder_child from moz_bookmarks.  This selects only
  // _item_ children which are in moz_history.
  NS_NAMED_LITERAL_CSTRING(selectItemChildren,
                           "SELECT h.id, h.url, h.title, h.visit_count, MAX(fullv.visit_date), h.rev_host, a.position, a.item_child, a.folder_child, null FROM moz_bookmarks a JOIN moz_history h ON a.item_child = h.id LEFT JOIN moz_historyvisit v ON h.id = v.page_id LEFT JOIN moz_historyvisit fullv ON h.id = fullv.page_id WHERE a.parent = ?1 AND a.position >= ?2 AND a.position <= ?3 GROUP BY h.id");

  // Construct a result where the first columns are padded out to the width
  // of mDBGetVisitPageInfo, containing additional columns for position,
  // item_child, and folder_child from moz_bookmarks, and name from
  // moz_bookmarks_folders.  This selects only _folder_ children which are
  // in moz_bookmarks_folders.
  NS_NAMED_LITERAL_CSTRING(selectFolderChildren,
                           "SELECT null, null, null, null, null, null, a.position, a.item_child, a.folder_child, c.name FROM moz_bookmarks a JOIN moz_bookmarks_folders c ON c.id = a.folder_child WHERE a.parent = ?1 AND a.position >= ?2 AND a.position <= ?3");

  NS_NAMED_LITERAL_CSTRING(orderByPosition, " ORDER BY a.position");

  // Select only folder children of a given folder (unsorted)
  rv = dbConn->CreateStatement(selectFolderChildren,
                               getter_AddRefs(mDBGetFolderChildren));
  NS_ENSURE_SUCCESS(rv, rv);

  // Select all children of a given folder, sorted by position
  rv = dbConn->CreateStatement(selectItemChildren +
                               NS_LITERAL_CSTRING(" UNION ALL ") +
                               selectFolderChildren + orderByPosition,
                               getter_AddRefs(mDBGetChildren));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = dbConn->CreateStatement(NS_LITERAL_CSTRING("SELECT COUNT(*) FROM moz_bookmarks WHERE parent = ?1"),
                               getter_AddRefs(mDBFolderCount));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = InitRoots();
  NS_ENSURE_SUCCESS(rv, rv);

  return transaction.Commit();
}

struct RenumberItem {
  PRInt64 folderChild;
  nsCOMPtr<nsIURI> itemURI;
  PRInt32 position;
};

struct RenumberItemsArray {
  nsVoidArray items;
  ~RenumberItemsArray();
};

RenumberItemsArray::~RenumberItemsArray()
{
  for (PRInt32 i = 0; i < items.Count(); ++i) {
    delete NS_STATIC_CAST(RenumberItem*, items[i]);
  }
}

// nsNavBookmarks::InitRoots
//
//    This locates and creates if necessary the root items in the bookmarks
//    folder hierarchy. These items are stored in a special roots table that
//    maps short predefined names to folder IDs.
//
//    Normally, these folders will exist already and we will save their IDs
//    which are exposed through the bookmark service interface.
//
//    If the root does not exist, a folder is created for it and the ID is
//    saved in the root table. No user-visible name is given to these folders
//    and they have no parent or other attributes.
//
//    These attributes are set when the default_places.html file is imported.
//    It defines the hierarchy, and has special attributes that tell us when
//    a folder is one of our well-known roots. We then insert the root in the
//    defined point in the hierarchy and set its attributes from this.
//
//    This should be called as the last part of the init process so that
//    all of the statements are set up and the service is ready to use.

nsresult
nsNavBookmarks::InitRoots()
{
  nsresult rv;
  nsCOMPtr<mozIStorageStatement> getRootStatement;
  rv = DBConn()->CreateStatement(NS_LITERAL_CSTRING("SELECT folder_id FROM moz_bookmarks_roots WHERE root_name = ?1"),
                                 getter_AddRefs(getRootStatement));
  NS_ENSURE_SUCCESS(rv, rv);

  PRBool importDefaults = PR_FALSE;
  rv = CreateRoot(getRootStatement, NS_LITERAL_CSTRING("places"), &mRoot, &importDefaults);
  NS_ENSURE_SUCCESS(rv, rv);

  getRootStatement->Reset();
  rv = CreateRoot(getRootStatement, NS_LITERAL_CSTRING("menu"), &mBookmarksRoot, nsnull);
  NS_ENSURE_SUCCESS(rv, rv);

  getRootStatement->Reset();
  rv = CreateRoot(getRootStatement, NS_LITERAL_CSTRING("toolbar"), &mToolbarRoot, nsnull);
  NS_ENSURE_SUCCESS(rv, rv);

  if (importDefaults) {
    // when there is no places root, we should define the hierarchy by
    // importing the default one.
    nsCOMPtr<nsIURI> defaultPlaces;
    rv = NS_NewURI(getter_AddRefs(defaultPlaces),
                   NS_LITERAL_CSTRING("chrome://browser/locale/places/default_places.html"),
                   nsnull);
    NS_ENSURE_SUCCESS(rv, rv);
    rv = ImportBookmarksHTMLInternal(defaultPlaces, PR_TRUE);
    NS_ENSURE_SUCCESS(rv, rv);

    // migrate the user's old bookmarks
    // FIXME: move somewhere else to some profile migrator thingy
    nsCOMPtr<nsIFile> bookmarksFile;
    rv = NS_GetSpecialDirectory(NS_APP_BOOKMARKS_50_FILE,
                                getter_AddRefs(bookmarksFile));
    PRBool bookmarksFileExists;
    rv = bookmarksFile->Exists(&bookmarksFileExists);
    if (NS_SUCCEEDED(rv) && bookmarksFileExists) {
      nsCOMPtr<nsIIOService> ioservice = do_GetService(
                                    "@mozilla.org/network/io-service;1", &rv);
      NS_ENSURE_SUCCESS(rv, rv);
      nsCOMPtr<nsIURI> bookmarksFileURI;
      rv = ioservice->NewFileURI(bookmarksFile,
                                 getter_AddRefs(bookmarksFileURI));
      NS_ENSURE_SUCCESS(rv, rv);
      rv = ImportBookmarksHTMLInternal(bookmarksFileURI, PR_FALSE);
      NS_ENSURE_SUCCESS(rv, rv);
    }
  }
  return NS_OK;
}


// nsNavBookmarks::CreateRoot
//
//    This gets or creates a root folder of the given type. aWasCreated
//    (optional) is true if the folder had to be created, false if we just used
//    an old one. The statement that gets a folder ID from a root name is
//    passed in so the DB only needs to parse the statement once, and we don't
//    have to have a global for this. Creation is less optimized because it
//    happens rarely.

nsresult
nsNavBookmarks::CreateRoot(mozIStorageStatement* aGetRootStatement,
                           const nsCString& name, PRInt64* aID,
                           PRBool* aWasCreated)
{
  PRBool hasResult = PR_FALSE;
  nsresult rv = aGetRootStatement->BindUTF8StringParameter(0, name);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = aGetRootStatement->ExecuteStep(&hasResult);
  NS_ENSURE_SUCCESS(rv, rv);
  if (hasResult) {
    if (aWasCreated)
      *aWasCreated = PR_FALSE;
    rv = aGetRootStatement->GetInt64(0, aID);
    NS_ENSURE_SUCCESS(rv, rv);
    NS_ASSERTION(*aID != 0, "Root is 0 for some reason, folders can't have 0 ID");
    return NS_OK;
  }
  if (aWasCreated)
    *aWasCreated = PR_TRUE;

  // create folder with no name or attributes
  nsCOMPtr<mozIStorageStatement> insertStatement;
  rv = CreateFolder(0, NS_LITERAL_STRING(""), -1, aID);
  NS_ENSURE_SUCCESS(rv, rv);

  // save root ID
  rv = DBConn()->CreateStatement(NS_LITERAL_CSTRING("INSERT INTO moz_bookmarks_roots (root_name,folder_id) VALUES (?1, ?2)"),
                                 getter_AddRefs(insertStatement));
  NS_ENSURE_SUCCESS(rv, rv);
  rv = insertStatement->BindUTF8StringParameter(0, name);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = insertStatement->BindInt64Parameter(1, *aID);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = insertStatement->Execute();
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

nsresult
nsNavBookmarks::AdjustIndices(PRInt64 aFolder,
                              PRInt32 aStartIndex, PRInt32 aEndIndex,
                              PRInt32 aDelta)
{
  mozIStorageConnection *dbConn = DBConn();
  mozStorageTransaction transaction(dbConn, PR_FALSE);

  nsCAutoString buffer;
  buffer.AssignLiteral("UPDATE moz_bookmarks SET position = position + ");
  buffer.AppendInt(aDelta);
  buffer.AppendLiteral(" WHERE parent = ");
  buffer.AppendInt(aFolder);

  if (aStartIndex != 0) {
    buffer.AppendLiteral(" AND position >= ");
    buffer.AppendInt(aStartIndex);
  }
  if (aEndIndex != PR_INT32_MAX) {
    buffer.AppendLiteral(" AND position <= ");
    buffer.AppendInt(aEndIndex);
  }

  nsresult rv = DBConn()->ExecuteSimpleSQL(buffer);
  NS_ENSURE_SUCCESS(rv, rv);

  // If we have any observers that want all details, we'll need to notify them
  // about the renumbering.
  nsCOMArray<nsINavBookmarkObserver> detailObservers;
  PRInt32 i;
  for (i = 0; i < mObservers.Count(); ++i) {
    PRBool wantDetails;
    rv = mObservers[i]->GetWantAllDetails(&wantDetails);
    if (NS_SUCCEEDED(rv) && wantDetails) {
      if (!detailObservers.AppendObject(mObservers[i])) {
        return NS_ERROR_OUT_OF_MEMORY;
      }
    }
  }

  if (detailObservers.Count() == 0) {
    return transaction.Commit();
  }

  RenumberItemsArray itemsArray;
  nsVoidArray *items = &itemsArray.items;
  {
    mozStorageStatementScoper scope(mDBGetChildren);
 
    rv = mDBGetChildren->BindInt64Parameter(0, aFolder);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = mDBGetChildren->BindInt32Parameter(1, aStartIndex + aDelta);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = mDBGetChildren->BindInt32Parameter(2, aEndIndex + aDelta);
    NS_ENSURE_SUCCESS(rv, rv);

    PRBool results;
    while (NS_SUCCEEDED(mDBGetChildren->ExecuteStep(&results)) && results) {
      RenumberItem *item = new RenumberItem();
      if (!item) {
        return NS_ERROR_OUT_OF_MEMORY;
      }

      if (mDBGetChildren->IsNull(kGetChildrenIndex_ItemChild)) {
        item->folderChild = mDBGetChildren->AsInt64(kGetChildrenIndex_FolderChild);
      } else {
        nsCAutoString spec;
        mDBGetChildren->GetUTF8String(nsNavHistory::kGetInfoIndex_URL, spec);
        rv = NS_NewURI(getter_AddRefs(item->itemURI), spec, nsnull);
        NS_ENSURE_SUCCESS(rv, rv);
      }
      item->position = mDBGetChildren->AsInt32(2);
      if (!items->AppendElement(item)) {
        delete item;
        return NS_ERROR_OUT_OF_MEMORY;
      }
    }
  }

  rv = transaction.Commit();
  NS_ENSURE_SUCCESS(rv, rv);

  UpdateBatcher batch;

  for (i = 0; i < detailObservers.Count(); ++i) {
    for (PRInt32 j = 0; j < items->Count(); ++j) {
      RenumberItem *item = NS_STATIC_CAST(RenumberItem*, (*items)[j]);
      PRInt32 newPosition = item->position;
      PRInt32 oldPosition = newPosition - aDelta;
      if (item->itemURI) {
        nsIURI *uri = item->itemURI;
        detailObservers[i]->OnItemMoved(uri, aFolder, oldPosition, newPosition);
      } else {
        detailObservers[i]->OnFolderMoved(item->folderChild,
                                          aFolder, oldPosition,
                                          aFolder, newPosition);
      }
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsNavBookmarks::GetPlacesRoot(PRInt64 *aRoot)
{
  *aRoot = mRoot;
  return NS_OK;
}

NS_IMETHODIMP
nsNavBookmarks::GetBookmarksRoot(PRInt64 *aRoot)
{
  *aRoot = mBookmarksRoot;
  return NS_OK;
}

NS_IMETHODIMP
nsNavBookmarks::GetToolbarRoot(PRInt64 *aRoot)
{
  *aRoot = mToolbarRoot;
  return NS_OK;
}

NS_IMETHODIMP
nsNavBookmarks::InsertItem(PRInt64 aFolder, nsIURI *aItem, PRInt32 aIndex)
{
  mozIStorageConnection *dbConn = DBConn();
  mozStorageTransaction transaction(dbConn, PR_FALSE);

  PRInt64 childID;
  nsresult rv = History()->GetUrlIdFor(aItem, &childID, PR_TRUE);
  NS_ENSURE_SUCCESS(rv, rv);

  PRInt32 index = (aIndex == -1) ? FolderCount(aFolder) : aIndex;

  rv = AdjustIndices(aFolder, index, PR_INT32_MAX, 1);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCAutoString buffer;
  buffer.AssignLiteral("INSERT INTO moz_bookmarks (item_child, parent, position) VALUES (");
  buffer.AppendInt(childID);
  buffer.AppendLiteral(", ");
  buffer.AppendInt(aFolder);
  buffer.AppendLiteral(", ");
  buffer.AppendInt(index);
  buffer.AppendLiteral(")");

  rv = dbConn->ExecuteSimpleSQL(buffer);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = transaction.Commit();
  NS_ENSURE_SUCCESS(rv, rv);

  for (PRInt32 i = 0; i < mObservers.Count(); ++i) {
    mObservers[i]->OnItemAdded(aItem, aFolder, index);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsNavBookmarks::RemoveItem(PRInt64 aFolder, nsIURI *aItem)
{
  mozIStorageConnection *dbConn = DBConn();
  mozStorageTransaction transaction(dbConn, PR_FALSE);

  PRInt64 childID;
  nsresult rv = History()->GetUrlIdFor(aItem, &childID, PR_FALSE);
  NS_ENSURE_SUCCESS(rv, rv);
  if (childID == 0) {
    return NS_OK; // the item isn't in history at all
  }

  PRInt32 childIndex;
  nsCAutoString buffer;
  {
    buffer.AssignLiteral("SELECT position FROM moz_bookmarks WHERE parent = ");
    buffer.AppendInt(aFolder);
    buffer.AppendLiteral(" AND item_child = ");
    buffer.AppendInt(childID);

    nsCOMPtr<mozIStorageStatement> statement;
    rv = dbConn->CreateStatement(buffer, getter_AddRefs(statement));
    NS_ENSURE_SUCCESS(rv, rv);

    PRBool results;
    rv = statement->ExecuteStep(&results);
    NS_ENSURE_SUCCESS(rv, rv);

    // We _should_ always have a result here, but maybe we don't if the table
    // has become corrupted.  Just silently skip adjusting indices.
    childIndex = results ? statement->AsInt32(0) : -1;
  }

  buffer.AssignLiteral("DELETE FROM moz_bookmarks WHERE parent = ");
  buffer.AppendInt(aFolder);
  buffer.AppendLiteral(" AND item_child = ");
  buffer.AppendInt(childID);

  rv = dbConn->ExecuteSimpleSQL(buffer);
  NS_ENSURE_SUCCESS(rv, rv);

  if (childIndex != -1) {
    rv = AdjustIndices(aFolder, childIndex + 1, PR_INT32_MAX, -1);
    NS_ENSURE_SUCCESS(rv, rv);
  }

  rv = transaction.Commit();
  NS_ENSURE_SUCCESS(rv, rv);

  for (PRInt32 i = 0; i < mObservers.Count(); ++i) {
    mObservers[i]->OnItemRemoved(aItem, aFolder, childIndex);
  }

  return NS_OK;
}


NS_IMETHODIMP
nsNavBookmarks::ReplaceItem(PRInt64 aFolder, nsIURI *aItem, nsIURI *aNewItem)
{
  mozIStorageConnection *dbConn = DBConn();
  nsNavHistory *history = History();
  mozStorageTransaction transaction(dbConn, PR_FALSE);

  PRInt64 childID;
  nsresult rv = history->GetUrlIdFor(aItem, &childID, PR_FALSE);
  NS_ENSURE_SUCCESS(rv, rv);
  if (childID == 0) {
    return NS_ERROR_INVALID_ARG; // the item isn't in history at all
  }

  PRInt64 newChildID;
  rv = history->GetUrlIdFor(aNewItem, &newChildID, PR_TRUE);
  NS_ENSURE_SUCCESS(rv, rv);
  NS_ASSERTION(newChildID != 0, "must have an item id");

  nsCAutoString buffer;
  buffer.AssignLiteral("UPDATE moz_bookmarks_assoc SET item_child = ");
  buffer.AppendInt(newChildID);
  buffer.AppendLiteral(" WHERE item_child = ");
  buffer.AppendInt(childID);
  buffer.AppendLiteral(" AND parent = ");
  buffer.AppendInt(aFolder);

  rv = dbConn->ExecuteSimpleSQL(buffer);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = transaction.Commit();
  NS_ENSURE_SUCCESS(rv, rv);

  for (PRInt32 i = 0; i < mObservers.Count(); ++i) {
    mObservers[i]->OnItemReplaced(aFolder, aItem, aNewItem);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsNavBookmarks::CreateFolder(PRInt64 aParent, const nsAString &aName,
                             PRInt32 aIndex, PRInt64 *aNewFolder)
{
  mozIStorageConnection *dbConn = DBConn();
  mozStorageTransaction transaction(dbConn, PR_FALSE);

  PRInt32 index = (aIndex == -1) ? FolderCount(aParent) : aIndex;

  nsresult rv = AdjustIndices(aParent, index, PR_INT32_MAX, 1);
  NS_ENSURE_SUCCESS(rv, rv);

  {
    nsCOMPtr<mozIStorageStatement> statement;
    rv = dbConn->CreateStatement(NS_LITERAL_CSTRING("INSERT INTO moz_bookmarks_folders (name) VALUES (?1)"),
                                 getter_AddRefs(statement));
    NS_ENSURE_SUCCESS(rv, rv);

    rv = statement->BindStringParameter(0, aName);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = statement->Execute();
    NS_ENSURE_SUCCESS(rv, rv);
  }

  PRInt64 child;
  rv = dbConn->GetLastInsertRowID(&child);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCAutoString buffer;
  buffer.AssignLiteral("INSERT INTO moz_bookmarks (folder_child, parent, position) VALUES (");
  buffer.AppendInt(child);
  buffer.AppendLiteral(", ");
  buffer.AppendInt(aParent);
  buffer.AppendLiteral(", ");
  buffer.AppendInt(index);
  buffer.AppendLiteral(")");
  rv = dbConn->ExecuteSimpleSQL(buffer);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = transaction.Commit();
  NS_ENSURE_SUCCESS(rv, rv);

  for (PRInt32 i = 0; i < mObservers.Count(); ++i) {
    mObservers[i]->OnFolderAdded(child, aParent, index);
  }

  *aNewFolder = child;
  return NS_OK;
}


NS_IMETHODIMP
nsNavBookmarks::RemoveFolder(PRInt64 aFolder)
{
  mozIStorageConnection *dbConn = DBConn();
  mozStorageTransaction transaction(dbConn, PR_FALSE);

  nsCAutoString buffer;
  buffer.AssignLiteral("SELECT parent, position FROM moz_bookmarks WHERE folder_child = ");
  buffer.AppendInt(aFolder);

  nsresult rv;
  PRInt64 parent;
  PRInt32 index;
  {
    nsCOMPtr<mozIStorageStatement> statement;
    rv = dbConn->CreateStatement(buffer, getter_AddRefs(statement));
    NS_ENSURE_SUCCESS(rv, rv);

    PRBool results;
    rv = statement->ExecuteStep(&results);
    NS_ENSURE_SUCCESS(rv, rv);
    if (!results) {
      return NS_ERROR_INVALID_ARG; // folder is not in the hierarchy
    }

    parent = statement->AsInt64(0);
    index = statement->AsInt32(1);
  }

  // Now locate all of the folder children, so we can recursively remove them.
  nsTArray<PRInt64> folderChildren;
  {
    mozStorageStatementScoper scope(mDBGetFolderChildren);
    rv = mDBGetChildren->BindInt64Parameter(0, aFolder);
    NS_ENSURE_SUCCESS(rv, rv);

    PRBool more;
    while (NS_SUCCEEDED(rv = mDBGetChildren->ExecuteStep(&more)) && more) {
      folderChildren.AppendElement(mDBGetChildren->AsInt64(kGetChildrenIndex_FolderChild));
    }
  }

  for (PRUint32 i = 0; i < folderChildren.Length(); ++i) {
    RemoveFolder(folderChildren[i]);
  }

  // Now remove the remaining items
  buffer.AssignLiteral("DELETE FROM moz_bookmarks WHERE parent = ");
  buffer.AppendInt(aFolder);
  rv = dbConn->ExecuteSimpleSQL(buffer);
  NS_ENSURE_SUCCESS(rv, rv);

  buffer.AssignLiteral("DELETE FROM moz_bookmarks WHERE folder_child = ");
  // Remove the folder from its parent
  buffer.AssignLiteral("DELETE FROM moz_bookmarks WHERE folder_child = ");
  buffer.AppendInt(aFolder);
  rv = dbConn->ExecuteSimpleSQL(buffer);
  NS_ENSURE_SUCCESS(rv, rv);

  // And remove the folder from the folder table
  buffer.AssignLiteral("DELETE FROM moz_bookmarks_folders WHERE id = ");
  buffer.AppendInt(aFolder);
  rv = dbConn->ExecuteSimpleSQL(buffer);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = AdjustIndices(parent, index + 1, PR_INT32_MAX, -1);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = transaction.Commit();
  NS_ENSURE_SUCCESS(rv, rv);

  for (PRInt32 j = 0; j < mObservers.Count(); ++j) {
    mObservers[j]->OnFolderRemoved(aFolder, parent, index);
  }

  return NS_OK;
}


NS_IMETHODIMP
nsNavBookmarks::MoveFolder(PRInt64 aFolder, PRInt64 aNewParent, PRInt32 aIndex)
{
  mozIStorageConnection *dbConn = DBConn();
  mozStorageTransaction transaction(dbConn, PR_FALSE);

  nsCAutoString buffer;
  buffer.AssignLiteral("SELECT parent, position FROM moz_bookmarks WHERE folder_child = ");
  buffer.AppendInt(aFolder);

  nsresult rv;
  PRInt64 parent;

  PRInt32 oldIndex;

  {
    nsCOMPtr<mozIStorageStatement> statement;
    rv = dbConn->CreateStatement(buffer, getter_AddRefs(statement));
    NS_ENSURE_SUCCESS(rv, rv);

    PRBool results;
    rv = statement->ExecuteStep(&results);
    NS_ENSURE_SUCCESS(rv, rv);
    if (!results) {
      return NS_ERROR_INVALID_ARG; // folder is not in the hierarchy
    }

    parent = statement->AsInt64(0);
    oldIndex = statement->AsInt32(1);
  }

  PRInt32 newIndex;
  if (aIndex == -1) {
    newIndex = FolderCount(aNewParent);
    // If the parent remains the same, then the folder is really being moved
    // to count - 1 (since it's being removed from the old position)
    if (parent == aNewParent) {
      --newIndex;
    }
  } else {
    newIndex = aIndex;

    if (parent == aNewParent && newIndex > oldIndex) {
      // when an item is being moved lower in the same folder, the new index
      // refers to the index before it was removed. Removal causes everything
      // to shift up.
      --newIndex;
    }
  }

  if (aNewParent == parent && newIndex == oldIndex) {
    // Nothing to do!
    return NS_OK;
  }

  if (parent == aNewParent) {
    // We can optimize the updates if moving within the same container
    if (oldIndex > newIndex) {
      rv = AdjustIndices(parent, newIndex, oldIndex - 1, 1);
    } else {
      rv = AdjustIndices(parent, oldIndex + 1, newIndex, -1);
    }
  } else {
    rv = AdjustIndices(parent, oldIndex + 1, PR_INT32_MAX, -1);
    NS_ENSURE_SUCCESS(rv, rv);
    rv = AdjustIndices(aNewParent, newIndex, PR_INT32_MAX, 1);
  }
  NS_ENSURE_SUCCESS(rv, rv);

  buffer.AssignLiteral("UPDATE moz_bookmarks SET parent = ");
  buffer.AppendInt(aNewParent);
  buffer.AppendLiteral(", position = ");
  buffer.AppendInt(newIndex);
  buffer.AppendLiteral(" WHERE folder_child = ");
  buffer.AppendInt(aFolder);

  rv = dbConn->ExecuteSimpleSQL(buffer);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = transaction.Commit();
  NS_ENSURE_SUCCESS(rv, rv);

  for (PRInt32 i = 0; i < mObservers.Count(); ++i) {
    mObservers[i]->OnFolderMoved(aFolder, parent, oldIndex,
                                 aNewParent, newIndex);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsNavBookmarks::GetChildFolder(PRInt64 aFolder, const nsAString& aSubFolder,
                               PRInt64* _result)
{
  // note: we allow empty folder names
  nsresult rv;
  if (aFolder == 0)
    return NS_ERROR_INVALID_ARG;

  // If this gets used a lot, we'll want a precompiled statement
  nsCOMPtr<mozIStorageStatement> statement;
  rv = DBConn()->CreateStatement(NS_LITERAL_CSTRING("SELECT c.id FROM moz_bookmarks a JOIN moz_bookmarks_folders c ON a.folder_child = c.id WHERE a.parent = ?1 AND c.name = ?2"),
                                 getter_AddRefs(statement));
  NS_ENSURE_SUCCESS(rv, rv);
  statement->BindInt64Parameter(0, aFolder);
  statement->BindStringParameter(1, aSubFolder);

  PRBool hasResult = PR_FALSE;
  rv = statement->ExecuteStep(&hasResult);
  NS_ENSURE_SUCCESS(rv, rv);

  if (! hasResult) {
    // item not found
    *_result = 0;
    return NS_OK;
  }

  return statement->GetInt64(0, _result);
}

NS_IMETHODIMP
nsNavBookmarks::SetItemTitle(nsIURI *aURI, const nsAString &aTitle)
{
  return History()->SetPageTitle(aURI, aTitle);
}


NS_IMETHODIMP
nsNavBookmarks::GetItemTitle(nsIURI *aURI, nsAString &aTitle)
{
  mozIStorageStatement *statement = DBGetURLPageInfo();
  nsresult rv = BindStatementURI(statement, 0, aURI);
  NS_ENSURE_SUCCESS(rv, rv);

  mozStorageStatementScoper scope(statement);

  PRBool results;
  rv = statement->ExecuteStep(&results);
  NS_ENSURE_SUCCESS(rv, rv);

  if (!results) {
    return NS_ERROR_INVALID_ARG;
  }

  return statement->GetString(nsNavHistory::kGetInfoIndex_Title, aTitle);
}

NS_IMETHODIMP
nsNavBookmarks::SetFolderTitle(PRInt64 aFolder, const nsAString &aTitle)
{
  nsCOMPtr<mozIStorageStatement> statement;
  nsresult rv = DBConn()->CreateStatement(NS_LITERAL_CSTRING("UPDATE moz_bookmarks_folders SET name = ?2 WHERE id = ?1"),
                                 getter_AddRefs(statement));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = statement->BindInt64Parameter(0, aFolder);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = statement->BindStringParameter(1, aTitle);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = statement->Execute();
  NS_ENSURE_SUCCESS(rv, rv);

  for (PRInt32 i = 0; i < mObservers.Count(); ++i) {
    mObservers[i]->OnFolderChanged(aFolder, NS_LITERAL_CSTRING("title"));
  }

  return NS_OK;
}


NS_IMETHODIMP
nsNavBookmarks::GetFolderTitle(PRInt64 aFolder, nsAString &aTitle)
{
  mozStorageStatementScoper scope(mDBGetFolderInfo);
  nsresult rv = mDBGetFolderInfo->BindInt64Parameter(0, aFolder);
  NS_ENSURE_SUCCESS(rv, rv);

  PRBool results;
  rv = mDBGetFolderInfo->ExecuteStep(&results);
  NS_ENSURE_SUCCESS(rv, rv);

  if (!results) {
    return NS_ERROR_INVALID_ARG;
  }

  return mDBGetFolderInfo->GetString(kGetFolderInfoIndex_Title, aTitle);
}

nsresult
nsNavBookmarks::ResultNodeForFolder(PRInt64 aID,
                                    nsINavHistoryQuery *aQuery,
                                    nsINavHistoryQueryOptions *aOptions,
                                    nsNavHistoryResultNode **aNode)
{
  mozStorageStatementScoper scope(mDBGetFolderInfo);
  mDBGetFolderInfo->BindInt64Parameter(0, aID);

  PRBool results;
  nsresult rv = mDBGetFolderInfo->ExecuteStep(&results);
  NS_ENSURE_SUCCESS(rv, rv);
  NS_ASSERTION(results, "ResultNodeForFolder expects a valid folder id");

  nsAutoString title;
  rv = mDBGetFolderInfo->GetString(kGetFolderInfoIndex_Title, title);
  NS_ENSURE_SUCCESS(rv, rv);

  // Create Query and QueryOptions objects to generate this folder's children
  nsCOMPtr<nsINavHistoryQuery> query;
  rv = aQuery->Clone(getter_AddRefs(query));
  NS_ENSURE_SUCCESS(rv, rv);

  query->SetFolders(&aID, 1);

  nsCOMPtr<nsINavHistoryQueryOptions> options;
  rv = aOptions->Clone(getter_AddRefs(options));
  NS_ENSURE_SUCCESS(rv, rv);

  nsRefPtr<nsNavHistoryQueryNode> node = new nsNavHistoryQueryNode();
  NS_ENSURE_TRUE(node, NS_ERROR_OUT_OF_MEMORY);

  node->mTitle = title;
  node->mType = nsINavHistoryResultNode::RESULT_TYPE_QUERY;
  node->mQueries = NS_STATIC_CAST(nsINavHistoryQuery**,
                                  nsMemory::Alloc(sizeof(nsINavHistoryQuery*)));
  NS_ENSURE_TRUE(node->mQueries, NS_ERROR_OUT_OF_MEMORY);
  node->mQueries[0] = nsnull;
  query.swap(node->mQueries[0]);
  node->mQueryCount = 1;
  node->mOptions = do_QueryInterface(options);

  NS_ADDREF(*aNode = node);
  return NS_OK;
}


nsresult
nsNavBookmarks::QueryFolderChildren(nsINavHistoryQuery *aQuery,
                                    nsINavHistoryQueryOptions *aOptions,
                                    nsCOMArray<nsNavHistoryResultNode> *aChildren)
{
  mozStorageTransaction transaction(DBConn(), PR_FALSE);

  PRUint32 itemTypes;
  aQuery->GetItemTypes(&itemTypes);

  PRBool includeQueries = (itemTypes & nsINavHistoryQuery::INCLUDE_QUERIES) != 0;
  PRBool includeItems = (itemTypes & nsINavHistoryQuery::INCLUDE_ITEMS) != 0;

  nsresult rv;
  nsCOMArray<nsNavHistoryResultNode> folderChildren;
  {
    mozStorageStatementScoper scope(mDBGetChildren);

    PRInt64 *folders;
    PRUint32 folderCount;
    aQuery->GetFolders(&folderCount, &folders);
    NS_ASSERTION(folderCount == 1, "querying > 1 folder not yet implemented");

    rv = mDBGetChildren->BindInt64Parameter(0, folders[0]);
    nsMemory::Free(folders);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = mDBGetChildren->BindInt64Parameter(1, 0);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = mDBGetChildren->BindInt64Parameter(2, PR_INT32_MAX);
    NS_ENSURE_SUCCESS(rv, rv);

    PRBool results;
    while (NS_SUCCEEDED(mDBGetChildren->ExecuteStep(&results)) && results) {
      PRBool isFolder = !mDBGetChildren->IsNull(kGetChildrenIndex_FolderChild);
      nsCOMPtr<nsNavHistoryResultNode> node;
      if (isFolder) {
        PRInt64 folder = mDBGetChildren->AsInt64(kGetChildrenIndex_FolderChild);
        rv = ResultNodeForFolder(folder, aQuery,
                                 aOptions, getter_AddRefs(node));
      } else {
        rv = History()->RowToResult(mDBGetChildren, PR_FALSE,
                                    getter_AddRefs(node));
        PRBool isQuery = IsQueryURI(node->URL());
        if ((isQuery && !includeQueries) || (!isQuery && !includeItems)) {
          continue;
        }
      }

      NS_ENSURE_SUCCESS(rv, rv);
      if (isFolder) {
        NS_ENSURE_TRUE(folderChildren.AppendObject(node),
                       NS_ERROR_OUT_OF_MEMORY);
      }
      NS_ENSURE_TRUE(aChildren->AppendObject(node), NS_ERROR_OUT_OF_MEMORY);
    }
  }

  // Now build children for any folder children we just created.  This is
  // pretty cheap (only go down 1 level) and allows us to know whether
  // to draw a twisty.
  static PRBool queryingChildren = PR_FALSE;

  if (!queryingChildren) {
    queryingChildren = PR_TRUE; // don't return before resetting to false
    for (PRInt32 i = 0; i < folderChildren.Count(); ++i) {
      rv = folderChildren[i]->BuildChildren();
      if (NS_FAILED(rv)) {
        break;
      }
    }
    queryingChildren = PR_FALSE;
    NS_ENSURE_SUCCESS(rv, rv);
  }

  return transaction.Commit();
}

PRInt32
nsNavBookmarks::FolderCount(PRInt64 aFolder)
{
  mozStorageStatementScoper scope(mDBFolderCount);

  nsresult rv = mDBFolderCount->BindInt64Parameter(0, aFolder);
  NS_ENSURE_SUCCESS(rv, 0);

  PRBool results;
  rv = mDBFolderCount->ExecuteStep(&results);
  NS_ENSURE_SUCCESS(rv, rv);

  return mDBFolderCount->AsInt32(0);
}

NS_IMETHODIMP
nsNavBookmarks::IsBookmarked(nsIURI *aURI, PRBool *aBookmarked)
{
  *aBookmarked = PR_FALSE;

  mozStorageStatementScoper scope(mDBFindURIBookmarks);

  nsresult rv = BindStatementURI(mDBFindURIBookmarks, 0, aURI);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mDBFindURIBookmarks->ExecuteStep(aBookmarked);
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

NS_IMETHODIMP
nsNavBookmarks::GetBookmarkCategories(nsIURI *aURI, PRUint32 *aCount,
                                      PRInt64 **aCategories)
{
  *aCount = 0;
  *aCategories = nsnull;

  mozStorageStatementScoper scope(mDBFindURIBookmarks);
  mozStorageTransaction transaction(DBConn(), PR_FALSE);

  nsresult rv = BindStatementURI(mDBFindURIBookmarks, 0, aURI);
  NS_ENSURE_SUCCESS(rv, rv);

  PRUint32 arraySize = 8;
  PRInt64 *categories = NS_STATIC_CAST(PRInt64*,
                                       nsMemory::Alloc(arraySize * sizeof(PRInt64)));
  NS_ENSURE_TRUE(categories, NS_ERROR_OUT_OF_MEMORY);

  PRUint32 count = 0;
  PRBool more;

  while (NS_SUCCEEDED((rv = mDBFindURIBookmarks->ExecuteStep(&more))) && more) {
    if (count >= arraySize) {
      arraySize <<= 1;
      PRInt64 *res = NS_STATIC_CAST(PRInt64*, nsMemory::Realloc(categories,
                                                 arraySize * sizeof(PRInt64)));
      if (!res) {
        delete categories;
        return NS_ERROR_OUT_OF_MEMORY;
      }
      categories = res;
    }
    categories[count++] =
      mDBFindURIBookmarks->AsInt64(kFindBookmarksIndex_Parent);
  }

  NS_ENSURE_SUCCESS(rv, rv);

  *aCount = count;
  *aCategories = categories;

  return transaction.Commit();
}

NS_IMETHODIMP
nsNavBookmarks::BeginUpdateBatch()
{
  if (mBatchLevel++ == 0) {
    for (PRInt32 i = 0; i < mObservers.Count(); ++i) {
      mObservers[i]->OnBeginUpdateBatch();
    }
  }
  return NS_OK;
}

NS_IMETHODIMP
nsNavBookmarks::EndUpdateBatch()
{
  if (--mBatchLevel == 0) {
    for (PRInt32 i = 0; i < mObservers.Count(); ++i) {
      mObservers[i]->OnEndUpdateBatch();
    }
  }
  return NS_OK;
}

NS_IMETHODIMP
nsNavBookmarks::AddObserver(nsINavBookmarkObserver *aObserver)
{
  NS_ENSURE_TRUE(mObservers.AppendObject(aObserver), NS_ERROR_OUT_OF_MEMORY);
  return NS_OK;
}

NS_IMETHODIMP
nsNavBookmarks::RemoveObserver(nsINavBookmarkObserver *aObserver)
{
  mObservers.RemoveObject(aObserver);
  return NS_OK;
}

// nsNavBookmarks::nsINavHistoryObserver

NS_IMETHODIMP
nsNavBookmarks::OnBeginUpdateBatch()
{
  // These aren't passed through to bookmark observers currently.
  return NS_OK;
}

NS_IMETHODIMP
nsNavBookmarks::OnEndUpdateBatch()
{
  // These aren't passed through to bookmark observers currently.
  return NS_OK;
}

NS_IMETHODIMP
nsNavBookmarks::GetWantAllDetails(PRBool *aWant)
{
  *aWant = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP
nsNavBookmarks::OnAddURI(nsIURI *aURI, PRTime aTime)
{
  // If the page is bookmarked, we need to notify observers
  PRBool bookmarked;
  IsBookmarked(aURI, &bookmarked);
  if (bookmarked) {
    for (PRInt32 i = 0; i < mObservers.Count(); ++i) {
      mObservers[i]->OnItemChanged(aURI, NS_LITERAL_CSTRING("time"));
    }
  }
  return NS_OK;
}

NS_IMETHODIMP
nsNavBookmarks::OnDeleteURI(nsIURI *aURI)
{
  // If the page is bookmarked, we need to notify observers
  PRBool bookmarked;
  IsBookmarked(aURI, &bookmarked);
  if (bookmarked) {
    for (PRInt32 i = 0; i < mObservers.Count(); ++i) {
      mObservers[i]->OnItemChanged(aURI, NS_LITERAL_CSTRING("time"));
    }
  }
  return NS_OK;
}

NS_IMETHODIMP
nsNavBookmarks::OnClearHistory()
{
  // Nothing being cleared should be bookmarked.
  return NS_OK;
}

NS_IMETHODIMP
nsNavBookmarks::OnPageChanged(nsIURI *aURI, PRUint32 aWhat,
                              const nsAString &aValue)
{
  if (aWhat == ATTRIBUTE_TITLE) {
    for (PRInt32 i = 0; i < mObservers.Count(); ++i) {
      mObservers[i]->OnItemChanged(aURI, NS_LITERAL_CSTRING("title"));
    }
  }

  return NS_OK;
}
