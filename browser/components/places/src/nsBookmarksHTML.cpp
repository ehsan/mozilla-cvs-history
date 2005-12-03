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
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Brett Wilson <brettw@gmail.com>
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
 * Importer/exporter between the mozStorage-based bookmarks and the old-style
 * "bookmarks.html"
 *
 * Format:
 *
 * Primary heading := h1
 *   Old version used this to set attributes on the bookmarks RDF root, such
 *   as the last modified date. We only use H1 to check for the attribute
 *   PLACES_ROOT, which tells us that this hierarchy root is the places root.
 *   For backwards compatability, if we don't find this, we assume that the
 *   hierarchy is rooted at the bookmarks menu.
 * Heading := any heading other than h1
 *   Old version used this to set attributes on the current container. We only
 *   care about the content of the heading container, which contains the title
 *   of the bookmark container.
 * Bookmark := a
 *   HREF is the destination of the bookmark
 *   LAST_CHARSET should be stored as an annotation (FIXME TODO) so that the
 *     next time we go to that page we remember the user's preference.
 *   ICON should be stored in the annotation service (FIXME TODO)
 *   Text of the <a> container is the name of the bookmark
 *   Ignored: ADD_DATE, LAST_VISIT, LAST_MODIFIED, ID
 * Bookmark comment := dd
 *   This affects the previosly added bookmark
 * Separator := hr
 *   Insert a separator into the current container
 * The folder hierarchy is defined by <dl>/<ul>/<menu> (the old importing code
 *     handles all these cases, when we write, use <dl>).
 */

#include "nsBrowserCompsCID.h"
#include "nsCOMPtr.h"
#include "nsIAnnotationService.h"
#include "nsIFile.h"
#include "nsIHTMLContentSink.h"
#include "nsNavBookmarks.h"
#include "nsIParser.h"
#include "nsIServiceManager.h"
#include "nsNavHistory.h"
#include "nsNetUtil.h"
#include "nsParserCIID.h"
#include "nsString.h"
#include "nsTArray.h"
#include "nsUnicharUtils.h"
#include "mozStorageHelper.h"

static NS_DEFINE_CID(kParserCID, NS_PARSER_CID);

#define KEY_TOOLBARFOLDER_LOWER "personal_toolbar_folder"
#define KEY_BOOKMARKSMENU_LOWER "bookmarks_menu"
#define KEY_PLACESROOT_LOWER "places_root"
#define KEY_HREF_LOWER "href"
#define KEY_LASTCHARSET_LOWER "last_charset"
#define KEY_ICON_LOWER "icon"

static const char kWhitespace[] = " \r\n\t\b";

class BookmarkImportFrame
{
public:
  BookmarkImportFrame(PRInt64 aID) :
      mContainerID(aID),
      mContainerNesting(0),
      mLastContainerType(Container_Normal),
      mInDescription(PR_FALSE)
  {
  }

  enum ContainerType { Container_Normal,
                       Container_Menu,
                       Container_Toolbar };

  PRInt64 mContainerID;

  // How many <dl>s have been nested. Each frame/container should start
  // with a heading, and is then followed by a <dl>, <ul>, or <menu>. When
  // that list is complete, then it is the end of this container and we need
  // to pop back up one level for new items. If we never get an open tag for
  // one of these things, we should assume that the container is empty and
  // that things we find should be siblings of it. Normally, these <dl>s won't
  // be nested so this will be 0 or 1.
  PRInt32 mContainerNesting;

  // when we find a heading tag, it actually affects the title of the NEXT
  // container in the list. This stores that heading tag and whether it was
  // special. 'ConsumeHeading' resets this.
  ContainerType mLastContainerType;

  // this contains the text from the last begin tag until now. It is reset
  // at every begin tag. We can check it when we see a </a>, or </h3>
  // to see what the text content of that node should be.
  nsString mPreviousText;

  // true when we hit a <dd>, which contains the description for the preceeding
  // <a> tag. We can't just check for </dd> like we can for </a> or </h3>
  // because if there is a sub-folder, it is actually a child of the <dd>
  // because the tag is never explicitly closed. If this is true and we see a
  // new open tag, that means to commit the description to the previous
  // bookmark.
  //
  // Additional weirdness happens when the previous <dt> tag contains a <h3>:
  // this means there is a new folder with the given description, and whose
  // children are contained in the following <dl> list.
  //
  // This is handled in OpenContainer(), which commits previous text if
  // necessary.
  PRBool mInDescription;

  // contains the URL of the previous bookmark created. This is used so that
  // when we encounter a <dd>, we know what bookmark to associate the text with.
  // This is cleared whenever we hit a <h3>, so that we know NOT to save this
  // with a bookmark, but to keep it until 
  nsCOMPtr<nsIURI> mPreviousLink;


  void ConsumeHeading(nsAString* aHeading, ContainerType* aContainerType)
  {
    *aHeading = mPreviousText;
    *aContainerType = mLastContainerType;
    mPreviousText.Truncate(0);
  }
};


/**
 * The content sink stuff is based loosely on 
 */
class BookmarkContentSink : public nsIHTMLContentSink
{
public:
  nsresult Init(PRBool aAllowRootChanges,
                nsINavBookmarksService* bookmarkService);

  NS_DECL_ISUPPORTS

  // nsIContentSink (superclass of nsIHTMLContentSink)
  NS_IMETHOD WillBuildModel() { return NS_OK; }
  NS_IMETHOD DidBuildModel() { return NS_OK; }
  NS_IMETHOD WillInterrupt() { return NS_OK; }
  NS_IMETHOD WillResume() { return NS_OK; }
  NS_IMETHOD SetParser(nsIParser* aParser) { return NS_OK; }
  virtual void FlushPendingNotifications(mozFlushType aType) { }
  NS_IMETHOD SetDocumentCharset(nsACString& aCharset) { return NS_OK; }
  virtual nsISupports *GetTarget() { return nsnull; }

  // nsIHTMLContentSink
  NS_IMETHOD OpenHead() { return NS_OK; }
  NS_IMETHOD BeginContext(PRInt32 aPosition) { return NS_OK; }
  NS_IMETHOD EndContext(PRInt32 aPosition) { return NS_OK; }
  NS_IMETHOD IsEnabled(PRInt32 aTag, PRBool* aReturn)
    { *aReturn = PR_TRUE; return NS_OK; }
  NS_IMETHOD WillProcessTokens() { return NS_OK; }
  NS_IMETHOD DidProcessTokens() { return NS_OK; }
  NS_IMETHOD WillProcessAToken() { return NS_OK; }
  NS_IMETHOD DidProcessAToken() { return NS_OK; }
  NS_IMETHOD OpenContainer(const nsIParserNode& aNode);
  NS_IMETHOD CloseContainer(const nsHTMLTag aTag);
  NS_IMETHOD AddLeaf(const nsIParserNode& aNode);
  NS_IMETHOD AddComment(const nsIParserNode& aNode) { return NS_OK; }
  NS_IMETHOD AddProcessingInstruction(const nsIParserNode& aNode) { return NS_OK; }
  NS_IMETHOD AddDocTypeDecl(const nsIParserNode& aNode) { return NS_OK; }
  NS_IMETHOD NotifyTagObservers(nsIParserNode* aNode) { return NS_OK; }
  NS_IMETHOD_(PRBool) IsFormOnStack() { return PR_FALSE; }

protected:
  nsCOMPtr<nsINavBookmarksService> mBookmarksService;
  nsCOMPtr<nsINavHistory> mHistoryService;
  nsCOMPtr<nsIAnnotationService> mAnnotationService;

  // if set, we will move root items to where we find them. This should be
  // set when we are loading the default places html file, and should be
  // unset when doing normal imports so that, for example, the toolbar folder
  // will be a child of the menu in old bookmarks.html, and we don't want
  // to reparent it on import.
  PRBool mAllowRootChanges;

  void HandleContainerBegin(const nsIParserNode& node);
  void HandleContainerEnd();
  void HandleHead1Begin(const nsIParserNode& node);
  void HandleHeadBegin(const nsIParserNode& node);
  void HandleHeadEnd();
  void HandleLinkBegin(const nsIParserNode& node);
  void HandleLinkEnd();

  // This is a list of frames. We really want a recursive parser, but the HTML
  // parser gives us tags as a stream. This implements all the state on a stack
  // so we can get the recursive information we need. Use "CurFrame" to get the
  // top "stack frame" with the current state in it.
  nsTArray<BookmarkImportFrame> mFrames;
  BookmarkImportFrame& CurFrame()
  {
    NS_ASSERTION(mFrames.Length() > 0, "Asking for frame when there are none!");
    return mFrames[mFrames.Length() - 1];
  }
  nsresult NewFrame();
  nsresult PopFrame();
};


// BookmarkContentSink::Init
//
//    Note that the bookmark service pointer is passed in. We can not create
//    the bookmark service from here because this can be called from bookmark
//    service creation, making a weird reentrant loop.

nsresult
BookmarkContentSink::Init(PRBool aAllowRootChanges,
                          nsINavBookmarksService* bookmarkService)
{
  nsresult rv;
  mBookmarksService = bookmarkService;
  mHistoryService = do_GetService(NS_NAVHISTORY_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);
  mAnnotationService = do_GetService(NS_ANNOTATIONSERVICE_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  mAllowRootChanges = aAllowRootChanges;

  // initialize the root frame with the menu root
  PRInt64 menuRoot;
  rv = mBookmarksService->GetBookmarksRoot(&menuRoot);
  NS_ENSURE_SUCCESS(rv, rv);
  if (! mFrames.AppendElement(BookmarkImportFrame(menuRoot)))
    return NS_ERROR_OUT_OF_MEMORY;

  return NS_OK;
}


NS_IMPL_ISUPPORTS2(BookmarkContentSink,
                   nsIContentSink,
                   nsIHTMLContentSink)

// nsIContentSink **************************************************************

NS_IMETHODIMP
BookmarkContentSink::OpenContainer(const nsIParserNode& aNode)
{
  // see the comment for the definition of mInDescription. Basically, we commit
  // any text in mPreviousText to the description of the node/folder if there
  // is any.
  BookmarkImportFrame& frame = CurFrame();
  if (frame.mInDescription) {
    frame.mPreviousText.Trim(kWhitespace); // important!
    if (! frame.mPreviousText.IsEmpty()) {
      // FIXME: This description should be stored as an annotation on the URL
      // or folder. We should probably not overwrite existing annotations.
      frame.mPreviousText.Truncate(0);
    }
    frame.mInDescription = PR_FALSE;
  }

  switch(aNode.GetNodeType()) {
    case eHTMLTag_h1:
      HandleHead1Begin(aNode);
      break;
    case eHTMLTag_h2:
    case eHTMLTag_h3:
    case eHTMLTag_h4:
    case eHTMLTag_h5:
    case eHTMLTag_h6:
      HandleHeadBegin(aNode);
      break;
    case eHTMLTag_a:
      HandleLinkBegin(aNode);
      break;
    case eHTMLTag_dl:
    case eHTMLTag_ul:
    case eHTMLTag_menu:
      HandleContainerBegin(aNode);
      break;
  }
  return NS_OK;
}

NS_IMETHODIMP
BookmarkContentSink::CloseContainer(const nsHTMLTag aTag)
{
  switch (aTag) {
    case eHTMLTag_dl:
    case eHTMLTag_ul:
    case eHTMLTag_menu:
      HandleContainerEnd();
      break;
    case eHTMLTag_dt:
      break;
    case eHTMLTag_h1:
      // ignore
      break;
    case eHTMLTag_h2:
    case eHTMLTag_h3:
    case eHTMLTag_h4:
    case eHTMLTag_h5:
    case eHTMLTag_h6:
      HandleHeadEnd();
      break;
    case eHTMLTag_a:
      HandleLinkEnd();
      break;
    default:
      break;
  }
  return NS_OK;
}

NS_IMETHODIMP
BookmarkContentSink::AddLeaf(const nsIParserNode& aNode)
{
  // save any text we find
  if (aNode.GetNodeType() == eHTMLTag_text) {
    CurFrame().mPreviousText += aNode.GetText();
  }

  return NS_OK;
}

// BookmarkContentSink::HandleContainerBegin

void
BookmarkContentSink::HandleContainerBegin(const nsIParserNode& node)
{
  CurFrame().mContainerNesting ++;
}


// BookmarkContentSink::HandleContainerEnd
//
//    Our "indent" count has decreased, and when we hit 0 that means that this
//    container is complete and we need to pop back to the outer frame. Never
//    pop the toplevel frame

void
BookmarkContentSink::HandleContainerEnd()
{
  BookmarkImportFrame& frame = CurFrame();
  if (frame.mContainerNesting > 0)
    frame.mContainerNesting --;
  if (mFrames.Length() > 1 && frame.mContainerNesting == 0)
    PopFrame();
}


// BookmarkContentSink::HandleHead1Begin
//
//    Handles <H1>. We check for the attribute PLACES_ROOT and reset the
//    container id if it's found. Otherwise, the default bookmark menu
//    root is assumed and imported things will go into the bookmarks menu.

void
BookmarkContentSink::HandleHead1Begin(const nsIParserNode& node)
{
  PRInt32 attrCount = node.GetAttributeCount();
  for (PRInt32 i = 0; i < attrCount; i ++) {
    if (node.GetKeyAt(i).LowerCaseEqualsLiteral(KEY_PLACESROOT_LOWER)) {
      if (mFrames.Length() > 1) {
        NS_WARNING("Trying to set the places root from the middle of the hierarchy. "
                   "This can only be set at the beginning.");
        return;
      }
      PRInt64 mPlacesRoot;
      mBookmarksService->GetPlacesRoot(&mPlacesRoot);
      CurFrame().mContainerID = mPlacesRoot;
      break;
    }
  }
}


// BookmarkContentSink::HandleHeadBegin
//
//    Called for h2,h3,h4,h5,h6. This just stores the correct information in
//    the current frame; the actual new frame corresponding to the container
//    associated with the heading will be created when the tag has been closed
//    and we know the title (we don't know to create a new folder or to merge
//    with an existing one until we have the title).

void
BookmarkContentSink::HandleHeadBegin(const nsIParserNode& node)
{
  BookmarkImportFrame& frame = CurFrame();

  // after a heading, a previous bookmark is not applicable (for example, for
  // the descriptions contained in a <dd>). Neither is any previous head type
  frame.mPreviousLink = nsnull;
  frame.mLastContainerType = BookmarkImportFrame::Container_Normal;

  // It is syntactically possible for a heading to appear after another heading
  // but before the <dl> that encloses that folder's contents.  This should not
  // happen in practice, as the file will contain "<dl></dl>" sequence for
  // empty containers.
  //
  // Just to be on the safe side, if we encounter
  //   <h3>FOO</h3>
  //   <h3>BAR</h3>
  //   <dl>...content 1...</dl>
  //   <dl>...content 2...</dl>
  // we'll pop the stack when we find the h3 for BAR, treating that as an
  // implicit ending of the FOO container. The output will be FOO and BAR as
  // siblings. If there's another <dl> following (as in "content 2"), those
  // items will be treated as further siblings of FOO and BAR
  if (frame.mContainerNesting == 0)
    PopFrame();

  // We have to check for some attributes to see if this is a "special"
  // folder, which will have different creation rules when the end tag is
  // processed.
  PRInt32 attrCount = node.GetAttributeCount();
  frame.mLastContainerType = BookmarkImportFrame::Container_Normal;
  for (PRInt32 i = 0; i < attrCount; i ++) {
    if (node.GetKeyAt(i).LowerCaseEqualsLiteral(KEY_TOOLBARFOLDER_LOWER)) {
      frame.mLastContainerType = BookmarkImportFrame::Container_Toolbar;
      break;
    } else if (node.GetKeyAt(i).LowerCaseEqualsLiteral(KEY_BOOKMARKSMENU_LOWER)) {
      frame.mLastContainerType = BookmarkImportFrame::Container_Menu;
      break;
    }
  }
  CurFrame().mPreviousText.Truncate(0);
}


// BookmarkContentSink::HandleHeadEnd
//
//    Creates the new frame for this heading now that we know the name of the
//    container (tokens since the heading open tag will have been placed in
//    mPreviousText).

void
BookmarkContentSink::HandleHeadEnd()
{
  NewFrame();
}


// BookmarkContentSink::HandleLinkBegin
//
//    Handles "<a" tags by creating a new bookmark. The title of the bookmark
//    will be the text content, which will be stuffed in mPreviousText for us
//    and which will be saved by HandleLinkEnd

void
BookmarkContentSink::HandleLinkBegin(const nsIParserNode& node)
{
  BookmarkImportFrame& frame = CurFrame();

  // mPreviousText will hold our link text, clear it so that can be appended to
  frame.mPreviousText.Truncate();

  // get the attributes we care about
  nsAutoString href;
  nsAutoString icon;
  nsAutoString lastCharset;
  PRInt32 attrCount = node.GetAttributeCount();
  for (PRInt32 i = 0; i < attrCount; i ++) {
    const nsAString& key = node.GetKeyAt(i);
    if (key.LowerCaseEqualsLiteral(KEY_HREF_LOWER)) {
      href = node.GetValueAt(i);
    } else if (key.LowerCaseEqualsLiteral(KEY_ICON_LOWER)) {
      icon = node.GetValueAt(i);
    } else if (key.LowerCaseEqualsLiteral(KEY_LASTCHARSET_LOWER)) {
      lastCharset = node.GetValueAt(i);
    }
  }
  href.Trim(kWhitespace);
  icon.Trim(kWhitespace);
  lastCharset.Trim(kWhitespace);

  // ignore <a> tags that have no href: we don't know what to do with them
  if (href.IsEmpty()) {
    frame.mPreviousLink = nsnull;
    return;
  }

  // save this so the link text and descriptions can be associated with it
  nsresult rv = NS_NewURI(getter_AddRefs(frame.mPreviousLink),
                 NS_ConvertUTF16toUTF8(href), nsnull);
  if (NS_FAILED(rv)) {
    frame.mPreviousLink = nsnull;
    return; // invalid link
  }

  mBookmarksService->InsertItem(frame.mContainerID, frame.mPreviousLink, -1);

  // FIXME: save the favicon
  // FIXME: save the last charset
}


// BookmarkContentSink::HandleLinkEnd
//
//    Saves the title for the given bookmark. This will overwrite an existing
//    title in the history, which may not be the right thing since we'd like to
//    keep the original title in there if we already had it, and only save the
//    custom title as an annotation. Doing this shouldn't affect anything,
//    however.

void
BookmarkContentSink::HandleLinkEnd()
{
  BookmarkImportFrame& frame = CurFrame();
  frame.mPreviousText.Trim(kWhitespace);
  if (! frame.mPreviousText.IsEmpty() && frame.mPreviousLink) {
    nsCOMPtr<nsIGlobalHistory2> globalHistory =
      do_QueryInterface(mHistoryService);
    globalHistory->SetPageTitle(frame.mPreviousLink, frame.mPreviousText);
    mAnnotationService->SetAnnotationString(frame.mPreviousLink,
          nsCString(nsNavHistory::kAnnotationTitle), frame.mPreviousText, 0, 0);
  }
  frame.mPreviousText.Truncate(0);
}


// BookmarkContentSink::NewFrame
//
//    This is called when there is a new folder found. The folder takes the
//    name from the previous frame's heading.

nsresult
BookmarkContentSink::NewFrame()
{
  nsresult rv;

  nsString containerName;
  BookmarkImportFrame::ContainerType containerType;
  CurFrame().ConsumeHeading(&containerName, &containerType);

  PRBool updateFolder = PR_FALSE;
  PRInt64 ourID = 0;
  switch (containerType) {
    case BookmarkImportFrame::Container_Normal:
      // regular folder: use an existing folder if that name already exists
      rv = mBookmarksService->GetChildFolder(CurFrame().mContainerID,
                                             containerName, &ourID);
      NS_ENSURE_SUCCESS(rv, rv);
      if (! ourID) {
        // need to append a new folder
        rv = mBookmarksService->CreateFolder(CurFrame().mContainerID,
                                            containerName, -1, &ourID);
        NS_ENSURE_SUCCESS(rv, rv);
      }
      break;
    case BookmarkImportFrame::Container_Menu:
      // menu root
      rv = mBookmarksService->GetBookmarksRoot(&ourID);
      NS_ENSURE_SUCCESS(rv, rv);
      if (mAllowRootChanges)
        updateFolder = PR_TRUE;
      break;
    case BookmarkImportFrame::Container_Toolbar:
      // toolbar root
      rv = mBookmarksService->GetToolbarRoot(&ourID);
      NS_ENSURE_SUCCESS(rv, rv);
      if (mAllowRootChanges)
        updateFolder = PR_TRUE;
      break;
    default:
      NS_NOTREACHED("Unknown container type");
  }

  if (updateFolder) {
    // move the menu/toolbar folder to the current position
    mBookmarksService->MoveFolder(ourID, CurFrame().mContainerID, -1);
    mBookmarksService->SetFolderTitle(ourID, containerName);
  }

  if (! mFrames.AppendElement(BookmarkImportFrame(ourID)))
    return NS_ERROR_OUT_OF_MEMORY;
  return NS_OK;
}


// BookmarkContentSink::PopFrame
//

nsresult
BookmarkContentSink::PopFrame()
{
  // we must always have one frame
  if (mFrames.Length() <= 1) {
    NS_NOTREACHED("Trying to complete more bookmark folders than you started");
    return NS_ERROR_FAILURE;
  }
  mFrames.RemoveElementAt(mFrames.Length() - 1);
  return NS_OK;
}


// SyncChannelStatus
//
//    If a function returns an error, we need to set the channel status to be
//    the same, but only if the channel doesn't have its own error. This returns
//    the error code that should be sent to OnStopRequest.

static nsresult
SyncChannelStatus(nsIChannel* channel, nsresult status)
{
  nsresult channelStatus;
  channel->GetStatus(&channelStatus);
  if (NS_FAILED(channelStatus))
    return channelStatus;

  if (NS_SUCCEEDED(status))
    return NS_OK; // caller and the channel are happy

  // channel was OK, but caller wasn't: set the channel state
  channel->Cancel(status);
  return status;
}


// nsNavBookmarks::ImportBookmarksHTML

NS_IMETHODIMP
nsNavBookmarks::ImportBookmarksHTML(nsIURI* aURL)
{
  // this version is exposed on the interface and disallows changing of roots
  return ImportBookmarksHTMLInternal(aURL, PR_FALSE);
}

nsresult
nsNavBookmarks::ImportBookmarksHTMLInternal(nsIURI* aURL,
                                            PRBool aAllowRootChanges)
{
  // wrap the import in a transaction to make it faster
  mozStorageTransaction transaction(DBConn(), PR_FALSE);

  nsresult rv;
  nsCOMPtr<nsIParser> parser = do_CreateInstance(kParserCID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  BookmarkContentSink* sink = new BookmarkContentSink;
  NS_ENSURE_TRUE(sink, NS_ERROR_OUT_OF_MEMORY);
  rv = sink->Init(aAllowRootChanges, this);
  NS_ENSURE_SUCCESS(rv, rv);
  parser->SetContentSink(sink);

  // channel: note we have to set the content type or the default "unknown" type
  // will confuse the parser
  nsCOMPtr<nsIIOService> ioservice = do_GetIOService(&rv);
  NS_ENSURE_SUCCESS(rv, rv);
  nsCOMPtr<nsIChannel> channel;
  rv = ioservice->NewChannelFromURI(aURL, getter_AddRefs(channel));
  NS_ENSURE_SUCCESS(rv, rv);
  rv = channel->SetContentType(NS_LITERAL_CSTRING("text/html"));
  NS_ENSURE_SUCCESS(rv, rv);

  // streams
  nsCOMPtr<nsIInputStream> stream;
  rv = channel->Open(getter_AddRefs(stream));
  NS_ENSURE_SUCCESS(rv, rv);
  nsCOMPtr<nsIInputStream> bufferedstream;
  rv = NS_NewBufferedInputStream(getter_AddRefs(bufferedstream), stream, 4096);
  NS_ENSURE_SUCCESS(rv, rv);

  // init parser
  rv = parser->Parse(aURL, nsnull, PR_FALSE);
  NS_ENSURE_SUCCESS(rv, rv);

  // feed the parser the data
  // Note: on error, we always need to set the channel's status to be the
  // same, and to always call OnStopRequest with the channel error.
  nsCOMPtr<nsIStreamListener> listener = do_QueryInterface(parser, &rv);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = listener->OnStartRequest(channel, nsnull);
  rv = SyncChannelStatus(channel, rv);
  while(NS_SUCCEEDED(rv))
  {
    PRUint32 available;
    rv = bufferedstream->Available(&available);
    if (rv == NS_BASE_STREAM_CLOSED) {
      rv = NS_OK;
      available = 0;
    }
    if (NS_FAILED(rv)) {
      channel->Cancel(rv);
      break;
    }
    if (! available)
      break; // blocking input stream has none available when done

    rv = listener->OnDataAvailable(channel, nsnull, bufferedstream, 0, available);
    rv = SyncChannelStatus(channel, rv);
    if (NS_FAILED(rv))
      break;
  }
  listener->OnStopRequest(channel, nsnull, rv);
  transaction.Commit();
  return NS_OK;
}


NS_IMETHODIMP
nsNavBookmarks::ExportBookmarksHTML(nsIURI* aURL)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}
