/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim:cindent:ts=2:et:sw=2:
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 *   Pierre Phaneuf <pp@ludusdesign.com>
 *
 * This Original Code has been modified by IBM Corporation. Modifications made by IBM 
 * described herein are Copyright (c) International Business Machines Corporation, 2000.
 * Modifications to Mozilla code or documentation identified per MPL Section 3.3
 *
 * Date             Modified by     Description of modification
 * 04/20/2000       IBM Corp.      OS/2 VisualAge build.
 */
#include "nscore.h"
#include "nsIFrameManager.h"
#include "nsIFrame.h"
#include "nsIPresContext.h"
#include "nsIPresShell.h"
#include "nsIStyleSet.h"
#include "nsStyleContext.h"
#include "nsStyleChangeList.h"
#include "nsIEventQueueService.h"
#include "nsIServiceManager.h"
#include "nsCOMPtr.h"
#include "prthread.h"
#include "plhash.h"
#include "pldhash.h"
#include "nsPlaceholderFrame.h"
#include "nsLayoutAtoms.h"
#include "nsCSSAnonBoxes.h"
#include "nsCSSPseudoElements.h"
#include "nsHTMLAtoms.h"
#ifdef NS_DEBUG
#include "nsISupportsArray.h"
#include "nsIStyleRule.h"
#endif
#include "nsILayoutHistoryState.h"
#include "nsIStatefulFrame.h"
#include "nsIPresState.h"
#include "nsIContent.h"
#include "nsINameSpaceManager.h"
#include "nsIXMLContent.h"
#include "nsIXBLBinding.h"
#include "nsIDocument.h"
#include "nsIBindingManager.h"
#include "nsIScrollableFrame.h"

#include "nsIHTMLDocument.h"
#include "nsIDOMHTMLDocument.h"
#include "nsIDOMNodeList.h"
#include "nsIDOMHTMLCollection.h"
#include "nsIFormControl.h"
#include "nsIDOMElement.h"
#include "nsIDOMHTMLFormElement.h"
#include "nsIForm.h"
#include "nsIContentList.h"
#include "nsContentUtils.h"
#include "nsReadableUtils.h"
#include "nsUnicharUtils.h"
#include "nsPrintfCString.h"
#include "nsDummyLayoutRequest.h"
#include "nsLayoutErrors.h"
#include "nsLayoutUtils.h"
#include "nsAutoPtr.h"

  #ifdef DEBUG
    //#define NOISY_DEBUG
    //#define DEBUG_UNDISPLAYED_MAP
  #else
    #undef NOISY_DEBUG
    #undef DEBUG_UNDISPLAYED_MAP
  #endif

  #ifdef NOISY_DEBUG
    #define NOISY_TRACE(_msg) \
      printf("%s",_msg);
    #define NOISY_TRACE_FRAME(_msg,_frame) \
      printf("%s ",_msg); nsFrame::ListTag(stdout,_frame); printf("\n");
  #else
    #define NOISY_TRACE(_msg);
    #define NOISY_TRACE_FRAME(_msg,_frame);
  #endif

// Class IID's
static NS_DEFINE_CID(kEventQueueServiceCID, NS_EVENTQUEUESERVICE_CID);

// IID's

//----------------------------------------------------------------------

struct PlaceholderMapEntry : public PLDHashEntryHdr {
  // key (the out of flow frame) can be obtained through placeholder frame
  nsPlaceholderFrame *placeholderFrame;
};

PR_STATIC_CALLBACK(const void *)
PlaceholderMapGetKey(PLDHashTable *table, PLDHashEntryHdr *hdr)
{
  PlaceholderMapEntry *entry = NS_STATIC_CAST(PlaceholderMapEntry*, hdr);
  return entry->placeholderFrame->GetOutOfFlowFrame();
}

PR_STATIC_CALLBACK(PRBool)
PlaceholderMapMatchEntry(PLDHashTable *table, const PLDHashEntryHdr *hdr,
                         const void *key)
{
  const PlaceholderMapEntry *entry =
    NS_STATIC_CAST(const PlaceholderMapEntry*, hdr);
  return entry->placeholderFrame->GetOutOfFlowFrame() == key;
}

static PLDHashTableOps PlaceholderMapOps = {
  PL_DHashAllocTable,
  PL_DHashFreeTable,
  PlaceholderMapGetKey,
  PL_DHashVoidPtrKeyStub,
  PlaceholderMapMatchEntry,
  PL_DHashMoveEntryStub,
  PL_DHashClearEntryStub,
  PL_DHashFinalizeStub,
  NULL
};

//----------------------------------------------------------------------

struct PropertyListMapEntry : public PLDHashEntryHdr {
  nsIFrame *key;
  void *value;
};

//----------------------------------------------------------------------

struct PrimaryFrameMapEntry : public PLDHashEntryHdr {
  // key (the content node) can almost always be obtained through the
  // frame.  If it weren't for the way image maps (mis)used the primary
  // frame map, we'd be able to have a 2 word entry instead of a 3 word
  // entry.
  nsIContent *content;
  nsIFrame *frame;
};

  // These ops should be used if/when we switch back to a 2-word entry.
  // See comment in |PrimaryFrameMapEntry| above.
#if 0
PR_STATIC_CALLBACK(const void *)
PrimaryFrameMapGetKey(PLDHashTable *table, PLDHashEntryHdr *hdr)
{
  PrimaryFrameMapEntry *entry = NS_STATIC_CAST(PrimaryFrameMapEntry*, hdr);
  return entry->frame->GetContent();
}

PR_STATIC_CALLBACK(PRBool)
PrimaryFrameMapMatchEntry(PLDHashTable *table, const PLDHashEntryHdr *hdr,
                         const void *key)
{
  const PrimaryFrameMapEntry *entry =
    NS_STATIC_CAST(const PrimaryFrameMapEntry*, hdr);
  return entry->frame->GetContent() == key;
}

static PLDHashTableOps PrimaryFrameMapOps = {
  PL_DHashAllocTable,
  PL_DHashFreeTable,
  PrimaryFrameMapGetKey,
  PL_DHashVoidPtrKeyStub,
  PrimaryFrameMapMatchEntry,
  PL_DHashMoveEntryStub,
  PL_DHashClearEntryStub,
  PL_DHashFinalizeStub,
  NULL
};
#endif /* 0 */

//----------------------------------------------------------------------

// XXXldb This seems too complicated for what I think it's doing, and it
// should also be using pldhash rather than plhash to use less memory.
MOZ_DECL_CTOR_COUNTER(UndisplayedNode)

class UndisplayedNode {
public:
  UndisplayedNode(nsIContent* aContent, nsStyleContext* aStyle)
    : mContent(aContent),
      mStyle(aStyle),
      mNext(nsnull)
  {
    MOZ_COUNT_CTOR(UndisplayedNode);
  }

  ~UndisplayedNode()
  {
    MOZ_COUNT_DTOR(UndisplayedNode);
    delete mNext;
  }

  nsCOMPtr<nsIContent>      mContent;
  nsRefPtr<nsStyleContext>  mStyle;
  UndisplayedNode*          mNext;
};

class UndisplayedMap {
public:
  UndisplayedMap(PRUint32 aNumBuckets = 16);
  ~UndisplayedMap(void);

  UndisplayedNode* GetFirstNode(nsIContent* aParentContent);

  nsresult AddNodeFor(nsIContent* aParentContent, nsIContent* aChild, nsStyleContext* aStyle);

  nsresult RemoveNodeFor(nsIContent* aParentContent, UndisplayedNode* aNode);
  nsresult RemoveNodesFor(nsIContent* aParentContent);

  // Removes all entries from the hash table
  void  Clear(void);

protected:
  PLHashEntry** GetEntryFor(nsIContent* aParentContent);
  nsresult      AppendNodeFor(UndisplayedNode* aNode, nsIContent* aParentContent);

  PLHashTable*  mTable;
  PLHashEntry** mLastLookup;
};

//----------------------------------------------------------------------

class FrameManager;

  // A CantRenderReplacedElementEvent has a weak pointer to the frame
  // manager, and the frame manager has a weak pointer to the event.
  // The event queue owns the event and the FrameManager will delete
  // the event if it's going to go away.
struct CantRenderReplacedElementEvent : public PLEvent {
  CantRenderReplacedElementEvent(FrameManager* aFrameManager, nsIFrame* aFrame, nsIPresShell* aPresShell);
  ~CantRenderReplacedElementEvent();
  // XXXldb Should the pres shell maintain a reference count on a single
  // dummy layout request instead of doing creation of a separate one
  // here (and per-event!)?
  nsresult AddLoadGroupRequest(nsIPresShell* aPresShell);
  nsresult RemoveLoadGroupRequest();

  nsIFrame*  mFrame;                     // the frame that can't be rendered
  CantRenderReplacedElementEvent* mNext; // next event in the list
  nsCOMPtr<nsIRequest> mDummyLayoutRequest; // load group request
  nsWeakPtr mPresShell;                     // for removing load group request later
};

class FrameManager : public nsIFrameManager
{
public:
  FrameManager();
  virtual ~FrameManager();

  NS_DECL_AND_IMPL_ZEROING_OPERATOR_NEW

  // nsISupports
  NS_DECL_ISUPPORTS

  // nsIFrameManager
  NS_IMETHOD Init(nsIPresShell* aPresShell, nsIStyleSet* aStyleSet);
  NS_IMETHOD Destroy();

  // Gets and sets the root frame
  NS_IMETHOD GetRootFrame(nsIFrame** aRootFrame) const;
  NS_IMETHOD SetRootFrame(nsIFrame* aRootFrame);
  
  // Get the canvas frame: searches from the Root frame down, may be null
  NS_IMETHOD GetCanvasFrame(nsIFrame** aCanvasFrame) const;

  // Primary frame functions
  NS_IMETHOD GetPrimaryFrameFor(nsIContent* aContent,
                                nsIFrame** aPrimaryFrame);
  NS_IMETHOD SetPrimaryFrameFor(nsIContent* aContent,
                                nsIFrame*   aPrimaryFrame);
  NS_IMETHOD ClearPrimaryFrameMap();

  // Placeholder frame functions
  NS_IMETHOD GetPlaceholderFrameFor(nsIFrame*  aFrame,
                                    nsIFrame** aPlaceholderFrame) const;
  NS_IMETHOD RegisterPlaceholderFrame(nsPlaceholderFrame* aPlaceholderFrame);
  NS_IMETHOD UnregisterPlaceholderFrame(nsPlaceholderFrame* aPlaceholderFrame);
  NS_IMETHOD ClearPlaceholderFrameMap();

  // Undisplayed content functions
  virtual nsStyleContext* GetUndisplayedContent(nsIContent* aContent);
  virtual void SetUndisplayedContent(nsIContent* aContent,
                                     nsStyleContext* aStyleContext);
  virtual void ChangeUndisplayedContent(nsIContent* aContent,
                                        nsStyleContext* aStyleContext);
  NS_IMETHOD ClearUndisplayedContentIn(nsIContent* aContent,
                                       nsIContent* aParentContent);
  NS_IMETHOD ClearAllUndisplayedContentIn(nsIContent* aParentContent);
  NS_IMETHOD ClearUndisplayedContentMap();

  // Functions for manipulating the frame model
  NS_IMETHOD AppendFrames(nsIFrame*       aParentFrame,
                          nsIAtom*        aListName,
                          nsIFrame*       aFrameList);
  NS_IMETHOD InsertFrames(nsIFrame*       aParentFrame,
                          nsIAtom*        aListName,
                          nsIFrame*       aPrevFrame,
                          nsIFrame*       aFrameList);
  NS_IMETHOD RemoveFrame(nsIFrame*       aParentFrame,
                         nsIAtom*        aListName,
                         nsIFrame*       aOldFrame);
  NS_IMETHOD ReplaceFrame(nsIFrame*       aParentFrame,
                          nsIAtom*        aListName,
                          nsIFrame*       aOldFrame,
                          nsIFrame*       aNewFrame);
  
  NS_IMETHOD CantRenderReplacedElement(nsIFrame* aFrame);

  NS_IMETHOD NotifyDestroyingFrame(nsIFrame* aFrame);

  NS_IMETHOD ReParentStyleContext(nsIFrame* aFrame, 
                                  nsStyleContext* aNewParentContext);
  NS_IMETHOD ComputeStyleChangeFor(nsIFrame* aFrame, 
                                   PRInt32 aAttrNameSpaceID,
                                   nsIAtom* aAttribute,
                                   nsStyleChangeList& aChangeList,
                                   nsChangeHint aMinChange,
                                   nsChangeHint& aTopLevelChange);
  NS_IMETHOD HasAttributeDependentStyle(nsIContent *aContent,
                                        nsIAtom *aAttribute,
                                        PRInt32 aModType,
                                        PRBool *aResult);

  // Capture state from the entire frame heirarchy and store in aState
  NS_IMETHOD CaptureFrameState(nsIFrame*              aFrame,
                               nsILayoutHistoryState* aState);
  NS_IMETHOD RestoreFrameState(nsIFrame*              aFrame,
                               nsILayoutHistoryState* aState);
  // Add/restore state for one frame (special, global type, like scroll position)
  NS_IMETHOD CaptureFrameStateFor(nsIFrame*              aFrame,
                                  nsILayoutHistoryState* aState,
                                  nsIStatefulFrame::SpecialStateID aID = nsIStatefulFrame::eNoID);
  NS_IMETHOD RestoreFrameStateFor(nsIFrame*              aFrame,
                                  nsILayoutHistoryState* aState,
                                  nsIStatefulFrame::SpecialStateID aID = nsIStatefulFrame::eNoID);
  // Gets and sets properties on a given frame
  NS_IMETHOD GetFrameProperty(nsIFrame* aFrame,
                              nsIAtom*  aPropertyName,
                              PRUint32  aOptions,
                              void**    aPropertyValue);
  NS_IMETHOD SetFrameProperty(nsIFrame*               aFrame,
                              nsIAtom*                aPropertyName,
                              void*                   aPropertyValue,
                              NSFramePropertyDtorFunc aPropDtorFunc);
  NS_IMETHOD RemoveFrameProperty(nsIFrame* aFrame,
                                 nsIAtom*  aPropertyName);

#ifdef NS_DEBUG
  NS_IMETHOD DebugVerifyStyleTree(nsIFrame* aFrame);
#endif

  struct PropertyList {
    nsCOMPtr<nsIAtom>       mName;          // property name
    PLDHashTable            mFrameValueMap; // map of frame/value pairs
    NSFramePropertyDtorFunc mDtorFunc;      // property specific value dtor function
    PropertyList*           mNext;

    PropertyList(nsIAtom*                aName,
                 NSFramePropertyDtorFunc aDtorFunc);
    ~PropertyList();

    // Removes the property associated with the given frame, and destroys
    // the property value
    PRBool RemovePropertyForFrame(nsIPresContext* aPresContext, nsIFrame* aFrame);

    // Destroy all remaining properties (without removing them)
    void Destroy(nsIPresContext* aPresContext);
  };
private:

  nsIPresShell* GetPresShell() const { return mPresShell; }
  nsIPresContext* GetPresContext() const {
    return mPresShell->GetPresContext();
  }
  nsIStyleSet* GetStyleSet() const { return mStyleSet; }

  nsIPresShell*                   mPresShell;    // weak link, because the pres shell owns us
  nsIStyleSet*                    mStyleSet;     // weak link. pres shell holds a reference
  nsIFrame*                       mRootFrame;
  PLDHashTable                    mPrimaryFrameMap;
  PLDHashTable                    mPlaceholderMap;
  UndisplayedMap*                 mUndisplayedMap;
  CantRenderReplacedElementEvent* mPostedEvents;
  PropertyList*                   mPropertyList;
  PRBool                          mIsDestroyingFrames;

  void ReResolveStyleContext(nsIPresContext* aPresContext,
                             nsIFrame* aFrame,
                             nsIContent* aParentContent,
                             PRInt32 aAttrNameSpaceID,
                             nsIAtom* aAttribute,
                             nsStyleChangeList& aChangeList, 
                             nsChangeHint aMinChange,
                             nsChangeHint& aResultChange);

  nsresult RevokePostedEvents();
  CantRenderReplacedElementEvent** FindPostedEventFor(nsIFrame* aFrame);
  void DequeuePostedEventFor(nsIFrame* aFrame);
  void DestroyPropertyList(nsIPresContext* aPresContext);
  PropertyList* GetPropertyListFor(nsIAtom* aPropertyName) const;
  void RemoveAllPropertiesFor(nsIPresContext* aPresContext, nsIFrame* aFrame);

  friend struct CantRenderReplacedElementEvent;
  static void HandlePLEvent(CantRenderReplacedElementEvent* aEvent);
  static void DestroyPLEvent(CantRenderReplacedElementEvent* aEvent);
};

//----------------------------------------------------------------------

nsresult
NS_NewFrameManager(nsIFrameManager** aInstancePtrResult)
{
  NS_ENSURE_ARG_POINTER(aInstancePtrResult);
  if (!aInstancePtrResult) {
    return NS_ERROR_NULL_POINTER;
  }
  FrameManager* it = new FrameManager;
  if (!it) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  return it->QueryInterface(NS_GET_IID(nsIFrameManager), (void **)aInstancePtrResult);
}

FrameManager::FrameManager()
{
}

FrameManager::~FrameManager()
{
  NS_ASSERTION(!mPresShell, "FrameManager::Destroy never called");
}

NS_IMPL_ISUPPORTS1(FrameManager, nsIFrameManager)

NS_IMETHODIMP
FrameManager::Init(nsIPresShell* aPresShell,
                   nsIStyleSet*  aStyleSet)
{
  NS_ASSERTION(aPresShell, "null aPresShell");
  NS_ASSERTION(aStyleSet, "null aStyleSet");

  mPresShell = aPresShell;
  mStyleSet = aStyleSet;

  return NS_OK;
}

NS_IMETHODIMP
FrameManager::Destroy()
{
  NS_ASSERTION(mPresShell, "Frame manager already shut down.");

  nsCOMPtr<nsIPresContext> presContext;
  mPresShell->GetPresContext(getter_AddRefs(presContext));
  
  // Destroy the frame hierarchy. Don't destroy the property lists until after
  // we've destroyed the frame hierarchy because some frames may expect to be
  // able to retrieve their properties during destruction
  mPresShell->SetIgnoreFrameDestruction(PR_TRUE);

  mIsDestroyingFrames = PR_TRUE;  // This flag prevents GetPrimaryFrameFor from returning pointers to destroyed frames

  if (mRootFrame) {
    mRootFrame->Destroy(presContext);
    mRootFrame = nsnull;
  }
  
  if (mPrimaryFrameMap.ops) {
    PL_DHashTableFinish(&mPrimaryFrameMap);
    mPrimaryFrameMap.ops = nsnull;
  }
  if (mPlaceholderMap.ops) {
    PL_DHashTableFinish(&mPlaceholderMap);
    mPlaceholderMap.ops = nsnull;
  }
  delete mUndisplayedMap;
  DestroyPropertyList(presContext);

  // If we're not going to be used anymore, we should revoke the
  // pending |CantRenderReplacedElementEvent|s being sent to us.
  nsresult rv = RevokePostedEvents();
  NS_ASSERTION(NS_SUCCEEDED(rv), "RevokePostedEvents failed:  might crash");

  mPresShell = nsnull; // mPresShell isn't valid anymore.  We
                       // won't use it, either, but we check it
                       // at the start of every function so that we'll
                       // be OK when nsIPresShell is converted to IDL.

  return rv;
}

NS_IMETHODIMP
FrameManager::GetRootFrame(nsIFrame** aRootFrame) const
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);
  NS_ENSURE_ARG_POINTER(aRootFrame);
  *aRootFrame = mRootFrame;
  return NS_OK;
}

NS_IMETHODIMP
FrameManager::SetRootFrame(nsIFrame* aRootFrame)
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);
  NS_PRECONDITION(!mRootFrame, "already have a root frame");
  if (mRootFrame) {
    return NS_ERROR_UNEXPECTED;
  }

  mRootFrame = aRootFrame;
  return NS_OK;
}

NS_IMETHODIMP
FrameManager::GetCanvasFrame(nsIFrame** aCanvasFrame) const
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);
  NS_PRECONDITION(aCanvasFrame, "aCanvasFrame argument cannot be null");
  nsIPresContext *presContext = GetPresContext();

  *aCanvasFrame = nsnull;
  if (mRootFrame) {
    // walk the children of the root frame looking for a frame with type==canvas
    // start at the root
    nsIFrame* childFrame = mRootFrame;
    while (childFrame) {
      // get each sibling of the child and check them, startig at the child
      nsIFrame *siblingFrame = childFrame;
      while (siblingFrame) {
        if (siblingFrame->GetType() == nsLayoutAtoms::canvasFrame) {
          // this is it: set the out-arg and stop looking
          *aCanvasFrame = siblingFrame;
          break;
        } else {
          siblingFrame = siblingFrame->GetNextSibling();
        }
      }
      // move on to the child's child
      childFrame->FirstChild(presContext, nsnull, &childFrame);
    }
  }
  return NS_OK;
}

//----------------------------------------------------------------------

// Primary frame functions
NS_IMETHODIMP
FrameManager::GetPrimaryFrameFor(nsIContent* aContent, nsIFrame** aResult)
{
  NS_ASSERTION(aResult, "null out-param not supported");
  *aResult = nsnull;  // initialize out param (before possibly returning due to null args/members)

  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);
  NS_ENSURE_ARG_POINTER(aContent);

  if (mIsDestroyingFrames) {
#ifdef DEBUG
    printf("GetPrimaryFrameFor() called while FrameManager is being destroyed!\n");
#endif
    return NS_ERROR_FAILURE;
  }

  if (mPrimaryFrameMap.ops) {
    PrimaryFrameMapEntry *entry = NS_STATIC_CAST(PrimaryFrameMapEntry*,
        PL_DHashTableOperate(&mPrimaryFrameMap, aContent, PL_DHASH_LOOKUP));
    if (PL_DHASH_ENTRY_IS_BUSY(entry)) {
      *aResult = entry->frame;
    } else {
      // XXX: todo:  Add a lookup into the undisplay map to skip searches 
      //             if we already know the content has no frame.
      //             nsCSSFrameConstructor calls SetUndisplayedContent() for every
      //             content node that has display: none.
      //             Today, the undisplay map doesn't quite support what we need.
      //             We need to see if we can add a method to make a search for aContent 
      //             very fast in the embedded hash table.
      //             This would almost completely remove the lookup penalty for things
      //             like <SCRIPT> and comments in very large documents.
      nsCOMPtr<nsIStyleSet>    styleSet;
      nsCOMPtr<nsIPresContext> presContext;

      // Give the frame construction code the opportunity to return the
      // frame that maps the content object
      mPresShell->GetStyleSet(getter_AddRefs(styleSet));
      NS_ASSERTION(styleSet, "bad style set");
      mPresShell->GetPresContext(getter_AddRefs(presContext));
      NS_ASSERTION(presContext, "bad presContext");
      if (!styleSet || !presContext) {
        return NS_ERROR_NULL_POINTER;
      }

      // if the prev sibling of aContent has a cached primary frame,
      // pass that data in to the style set to speed things up
      // if any methods in here fail, don't report that failure
      // we're just trying to enhance performance here, not test for correctness
      nsFindFrameHint hint;
      nsIContent* parent = aContent->GetParent();
      if (parent)
      {
        PRInt32 index = parent->IndexOf(aContent);
        if (index > 0)  // no use looking if it's the first child
        {
          nsIContent *prevSibling;
          nsCOMPtr<nsIAtom> tag;
          do {
            prevSibling = parent->GetChildAt(--index);
            prevSibling->GetTag(getter_AddRefs(tag));
          } while (index &&
                   (tag == nsLayoutAtoms::textTagName ||
                    tag == nsLayoutAtoms::commentTagName ||
                    tag == nsLayoutAtoms::processingInstructionTagName));
          if (prevSibling) {
            entry = NS_STATIC_CAST(PrimaryFrameMapEntry*,
                PL_DHashTableOperate(&mPrimaryFrameMap, prevSibling,
                                     PL_DHASH_LOOKUP));
            if (PL_DHASH_ENTRY_IS_BUSY(entry))
              hint.mPrimaryFrameForPrevSibling = entry->frame;
          }
        }
      }

      // walk the frame tree to find the frame that maps aContent.  
      // Use the hint if we have it.
      styleSet->FindPrimaryFrameFor(presContext, this, aContent, aResult, 
                                    hint.mPrimaryFrameForPrevSibling ? &hint : nsnull);
      
    }
  }
  return NS_OK;
}

NS_IMETHODIMP
FrameManager::SetPrimaryFrameFor(nsIContent* aContent,
                                 nsIFrame*   aPrimaryFrame)
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);
  NS_ENSURE_ARG_POINTER(aContent);
  // it's ok if aPrimaryFrame is null

  // If aPrimaryFrame is NULL, then remove the mapping
  if (!aPrimaryFrame) {
    if (mPrimaryFrameMap.ops) {
      PL_DHashTableOperate(&mPrimaryFrameMap, aContent, PL_DHASH_REMOVE);
    }
  } else {
  // This code should be used if/when we switch back to a 2-word entry
  // in the primary frame map.
#if 0
    NS_PRECONDITION(aPrimaryFrame->GetContent() == aContent, "wrong content");
#endif

    // Create a new hashtable if necessary
    if (!mPrimaryFrameMap.ops) {
      if (!PL_DHashTableInit(&mPrimaryFrameMap, PL_DHashGetStubOps(), nsnull,
                             sizeof(PrimaryFrameMapEntry), 16)) {
        mPrimaryFrameMap.ops = nsnull;
        return NS_ERROR_OUT_OF_MEMORY;
      }
    }

    // Add a mapping to the hash table
    PrimaryFrameMapEntry *entry = NS_STATIC_CAST(PrimaryFrameMapEntry*,
        PL_DHashTableOperate(&mPrimaryFrameMap, aContent, PL_DHASH_ADD));
#ifdef DEBUG_dbaron
    if (entry->frame) {
      NS_WARNING("already have primary frame for content");
    }
#endif
    entry->frame = aPrimaryFrame;
    entry->content = aContent;
  }
    
  return NS_OK;
}

NS_IMETHODIMP
FrameManager::ClearPrimaryFrameMap()
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);
  if (mPrimaryFrameMap.ops) {
    PL_DHashTableFinish(&mPrimaryFrameMap);
    mPrimaryFrameMap.ops = nsnull;
  }
  return NS_OK;
}

// Placeholder frame functions
NS_IMETHODIMP
FrameManager::GetPlaceholderFrameFor(nsIFrame*  aFrame,
                                     nsIFrame** aResult) const
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);
  NS_PRECONDITION(aFrame, "null param unexpected");
  NS_PRECONDITION(aResult, "null out param unexpected");

  if (mPlaceholderMap.ops) {
    PlaceholderMapEntry *entry = NS_STATIC_CAST(PlaceholderMapEntry*,
           PL_DHashTableOperate(NS_CONST_CAST(PLDHashTable*, &mPlaceholderMap),
                                aFrame, PL_DHASH_LOOKUP));
    if (PL_DHASH_ENTRY_IS_BUSY(entry)) {
      *aResult = entry->placeholderFrame;
      return NS_OK;
    }
  }

  *aResult = nsnull;
  return NS_OK;
}

NS_IMETHODIMP
FrameManager::RegisterPlaceholderFrame(nsPlaceholderFrame* aPlaceholderFrame)
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);
  NS_PRECONDITION(aPlaceholderFrame, "null param unexpected");
  NS_PRECONDITION(nsLayoutAtoms::placeholderFrame == aPlaceholderFrame->GetType(),
                  "unexpected frame type");
  if (!mPlaceholderMap.ops) {
    if (!PL_DHashTableInit(&mPlaceholderMap, &PlaceholderMapOps, nsnull,
                           sizeof(PlaceholderMapEntry), 16)) {
      mPlaceholderMap.ops = nsnull;
      return NS_ERROR_OUT_OF_MEMORY;
    }
  }
  PlaceholderMapEntry *entry = NS_STATIC_CAST(PlaceholderMapEntry*, 
         PL_DHashTableOperate(&mPlaceholderMap,
                              aPlaceholderFrame->GetOutOfFlowFrame(),
                              PL_DHASH_ADD));
  if (!entry)
    return NS_ERROR_OUT_OF_MEMORY;

  NS_ASSERTION(!entry->placeholderFrame, "Registering a placeholder for a frame that already has a placeholder!");
  entry->placeholderFrame = aPlaceholderFrame;
  return NS_OK;
}

NS_IMETHODIMP
FrameManager::UnregisterPlaceholderFrame(nsPlaceholderFrame* aPlaceholderFrame)
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);
  NS_PRECONDITION(aPlaceholderFrame, "null param unexpected");
  NS_PRECONDITION(nsLayoutAtoms::placeholderFrame == aPlaceholderFrame->GetType(),
                  "unexpected frame type");

  /*
   * nsCSSFrameConstructor::ReconstructDocElementHierarchy calls
   * ClearPlaceholderFrameMap and _then_ removes the fixed-positioned
   * frames one by one.  As these are removed they call
   * UnregisterPlaceholderFrame on their placeholders, but this is all
   * happening when mPlaceholderMap is already finished, so there is
   * nothing to do here.  See bug 144479.
   */
  if (mPlaceholderMap.ops) {
    PL_DHashTableOperate(&mPlaceholderMap,
                         aPlaceholderFrame->GetOutOfFlowFrame(),
                         PL_DHASH_REMOVE);
  }
  
  return NS_OK;
}

NS_IMETHODIMP
FrameManager::ClearPlaceholderFrameMap()
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);
  if (mPlaceholderMap.ops) {
    PL_DHashTableFinish(&mPlaceholderMap);
    mPlaceholderMap.ops = nsnull;
  }
  return NS_OK;
}

//----------------------------------------------------------------------

nsStyleContext*
FrameManager::GetUndisplayedContent(nsIContent* aContent)
{
  if (!aContent || !mUndisplayedMap)
    return nsnull;

  nsIContent* parent = aContent->GetParent();
  if (!parent)
    return nsnull;

  for (UndisplayedNode* node = mUndisplayedMap->GetFirstNode(parent);
         node; node = node->mNext) {
    if (node->mContent == aContent)
      return node->mStyle;
  }

  return nsnull;
}
  
void
FrameManager::SetUndisplayedContent(nsIContent* aContent, 
                                    nsStyleContext* aStyleContext)
{
  if (!mPresShell)
    return;
  
#ifdef DEBUG_UNDISPLAYED_MAP
   static int i = 0;
   printf("SetUndisplayedContent(%d): p=%p \n", i++, (void *)aContent);
#endif

  if (! mUndisplayedMap) {
    mUndisplayedMap = new UndisplayedMap;
  }
  if (mUndisplayedMap) {
    nsIContent* parent = aContent->GetParent();
    NS_ASSERTION(parent, "undisplayed content must have a parent");
    if (parent) {
      mUndisplayedMap->AddNodeFor(parent, aContent, aStyleContext);
    }
  }
}

void
FrameManager::ChangeUndisplayedContent(nsIContent* aContent, 
                                       nsStyleContext* aStyleContext)
{
  if (!mPresShell)
    return;
  NS_ASSERTION(mUndisplayedMap, "no existing undisplayed content");
  
#ifdef DEBUG_UNDISPLAYED_MAP
   static int i = 0;
   printf("ChangeUndisplayedContent(%d): p=%p \n", i++, (void *)aContent);
#endif

  for (UndisplayedNode* node = mUndisplayedMap->GetFirstNode(aContent->GetParent());
         node; node = node->mNext) {
    if (node->mContent == aContent) {
      node->mStyle = aStyleContext;
      return;
    }
  }

  NS_NOTREACHED("no existing undisplayed content");
}

NS_IMETHODIMP
FrameManager::ClearUndisplayedContentIn(nsIContent* aContent, nsIContent* aParentContent)
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);

#ifdef DEBUG_UNDISPLAYED_MAP
  static int i = 0;
  printf("ClearUndisplayedContent(%d): content=%p parent=%p --> ", i++, (void *)aContent, (void*)aParentContent);
#endif
  
  if (mUndisplayedMap) {
    UndisplayedNode* node = mUndisplayedMap->GetFirstNode(aParentContent);
    while (node) {
      if (node->mContent == aContent) {
        nsresult rv = mUndisplayedMap->RemoveNodeFor(aParentContent, node);

#ifdef DEBUG_UNDISPLAYED_MAP
        printf( "REMOVED! (rv=%d)\n", (int)rv);
#endif
#ifdef DEBUG
        // make sure that there are no more entries for the same content
        nsStyleContext *context = GetUndisplayedContent(aContent);
        NS_ASSERTION(context == nsnull, "Found more undisplayed content data after removal");
#endif
        return rv;
      }
      node = node->mNext;
    }
  }
  return NS_OK;
}

NS_IMETHODIMP
FrameManager::ClearAllUndisplayedContentIn(nsIContent* aParentContent)
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);
 
#ifdef DEBUG_UNDISPLAYED_MAP
  static int i = 0;
  printf("ClearAllUndisplayedContentIn(%d): parent=%p \n", i++, (void*)aParentContent);
#endif

  if (mUndisplayedMap) {
    return mUndisplayedMap->RemoveNodesFor(aParentContent);
  }
  return NS_OK;
}

NS_IMETHODIMP
FrameManager::ClearUndisplayedContentMap()
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);

#ifdef DEBUG_UNDISPLAYED_MAP
  static int i = 0;
  printf("ClearUndisplayedContentMap(%d)\n", i++);
#endif

  if (mUndisplayedMap) {
    mUndisplayedMap->Clear();
  }
  return NS_OK;
}

//----------------------------------------------------------------------

NS_IMETHODIMP
FrameManager::AppendFrames(nsIFrame*       aParentFrame,
                           nsIAtom*        aListName,
                           nsIFrame*       aFrameList)
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);
  return aParentFrame->AppendFrames(GetPresContext(), *GetPresShell(),
                                    aListName, aFrameList);
}

NS_IMETHODIMP
FrameManager::InsertFrames(nsIFrame*       aParentFrame,
                           nsIAtom*        aListName,
                           nsIFrame*       aPrevFrame,
                           nsIFrame*       aFrameList)
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);
#ifdef IBMBIDI
  if (aPrevFrame) {
    // Insert aFrameList after the last bidi continuation of aPrevFrame.
    nsIFrame* nextBidi;
    for (; ;) {
      GetFrameProperty(aPrevFrame, nsLayoutAtoms::nextBidi, 0, (void**) &nextBidi);
      if (!nextBidi) {
        break;
      }
      aPrevFrame = nextBidi;
    }
  }
#endif // IBMBIDI

  return aParentFrame->InsertFrames(GetPresContext(), *GetPresShell(),
                                    aListName, aPrevFrame, aFrameList);
}

NS_IMETHODIMP
FrameManager::RemoveFrame(nsIFrame*       aParentFrame,
                          nsIAtom*        aListName,
                          nsIFrame*       aOldFrame)
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);

#ifdef IBMBIDI
  // Don't let the parent remove next bidi. In the other cases the it should NOT be removed.
  nsIFrame* nextBidi;
  GetFrameProperty(aOldFrame, nsLayoutAtoms::nextBidi, 0, (void**) &nextBidi);
  if (nextBidi) {
    RemoveFrame(aParentFrame, aListName, nextBidi);
  }
#endif // IBMBIDI

  return aParentFrame->RemoveFrame(GetPresContext(), *GetPresShell(),
                                   aListName, aOldFrame);
}

NS_IMETHODIMP
FrameManager::ReplaceFrame(nsIFrame*       aParentFrame,
                           nsIAtom*        aListName,
                           nsIFrame*       aOldFrame,
                           nsIFrame*       aNewFrame)
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);
  return aParentFrame->ReplaceFrame(GetPresContext(), *GetPresShell(),
                                    aListName, aOldFrame, aNewFrame);
}

//----------------------------------------------------------------------

NS_IMETHODIMP
FrameManager::NotifyDestroyingFrame(nsIFrame* aFrame)
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);
  // Dequeue and destroy and posted events for this frame
  DequeuePostedEventFor(aFrame);

  // Remove all properties associated with the frame
  nsCOMPtr<nsIPresContext> presContext;
  mPresShell->GetPresContext(getter_AddRefs(presContext));

  RemoveAllPropertiesFor(presContext, aFrame);

#ifdef DEBUG
  if (mPrimaryFrameMap.ops) {
    PrimaryFrameMapEntry *entry = NS_STATIC_CAST(PrimaryFrameMapEntry*,
        PL_DHashTableOperate(&mPrimaryFrameMap, aFrame->GetContent(), PL_DHASH_LOOKUP));
    NS_ASSERTION(!PL_DHASH_ENTRY_IS_BUSY(entry) || entry->frame != aFrame,
                 "frame was not removed from primary frame map before "
                 "destruction or was readded to map after being removed");
  }
#endif

  return NS_OK;
}

nsresult
FrameManager::RevokePostedEvents()
{
  nsresult rv = NS_OK;
#ifdef NOISY_EVENTS
  printf("%p ~RevokePostedEvents() start\n", this);
#endif
  if (mPostedEvents) {
    mPostedEvents = nsnull;

    // Revoke any events in the event queue that are owned by us
    nsIEventQueueService* eventService;

    rv = nsServiceManager::GetService(kEventQueueServiceCID,
                                      NS_GET_IID(nsIEventQueueService),
                                      (nsISupports **)&eventService);
    if (NS_SUCCEEDED(rv)) {
      nsCOMPtr<nsIEventQueue> eventQueue;
      rv = eventService->GetThreadEventQueue(NS_CURRENT_THREAD, 
                                             getter_AddRefs(eventQueue));
      nsServiceManager::ReleaseService(kEventQueueServiceCID, eventService);

      if (NS_SUCCEEDED(rv) && eventQueue) {
        rv = eventQueue->RevokeEvents(this);
      }
    }
  }
#ifdef NOISY_EVENTS
  printf("%p ~RevokePostedEvents() end\n", this);
#endif
  return rv;
}

CantRenderReplacedElementEvent**
FrameManager::FindPostedEventFor(nsIFrame* aFrame)
{
  CantRenderReplacedElementEvent** event = &mPostedEvents;

  while (*event) {
    if ((*event)->mFrame == aFrame) {
      return event;
    }
    event = &(*event)->mNext;
  }

  return event;
}

void
FrameManager::DequeuePostedEventFor(nsIFrame* aFrame)
{
  // If there's a posted event for this frame, then remove it
  CantRenderReplacedElementEvent** event = FindPostedEventFor(aFrame);
  if (*event) {
    CantRenderReplacedElementEvent* tmp = *event;

    // Remove it from our linked list of posted events
    *event = (*event)->mNext;
    
    // Dequeue it from the event queue
    nsIEventQueueService* eventService;
    nsresult              rv;

    rv = nsServiceManager::GetService(kEventQueueServiceCID,
                                      NS_GET_IID(nsIEventQueueService),
                                      (nsISupports **)&eventService);
    NS_ASSERTION(NS_SUCCEEDED(rv),
            "will crash soon due to event holding dangling pointer to frame");
    if (NS_SUCCEEDED(rv)) {
      nsCOMPtr<nsIEventQueue> eventQueue;
      rv = eventService->GetThreadEventQueue(NS_CURRENT_THREAD, 
                                             getter_AddRefs(eventQueue));
      nsServiceManager::ReleaseService(kEventQueueServiceCID, eventService);

      NS_ASSERTION(NS_SUCCEEDED(rv) && eventQueue,
            "will crash soon due to event holding dangling pointer to frame");
      if (NS_SUCCEEDED(rv) && eventQueue) {
        PLEventQueue* plqueue;

        eventQueue->GetPLEventQueue(&plqueue);
        NS_ASSERTION(plqueue,
            "will crash soon due to event holding dangling pointer to frame");
        if (plqueue) {
          // Remove the event and then destroy it
          PL_DequeueEvent(tmp, plqueue);
          PL_DestroyEvent(tmp);
        }
      }
    }
  }
}

void
FrameManager::HandlePLEvent(CantRenderReplacedElementEvent* aEvent)
{
#ifdef NOISY_EVENTS
  printf("FrameManager::HandlePLEvent() start for FM %p\n", aEvent->owner);
#endif
  FrameManager* frameManager = (FrameManager*)aEvent->owner;
  NS_ASSERTION(frameManager, "null frame manager");

  if (!frameManager->mPresShell) {
    NS_ASSERTION(frameManager->mPresShell,
                 "event not removed from queue on shutdown");
    return;
  }

  // Remove the posted event from the linked list
  CantRenderReplacedElementEvent** events = &frameManager->mPostedEvents;
  while (*events) {
    if (*events == aEvent) {
      *events = (*events)->mNext;
      break;
    }
    events = &(*events)->mNext;
    NS_ASSERTION(*events, "event not in queue");
  }

  // Notify the style system and then process any reflow commands that
  // are generated
  nsCOMPtr<nsIPresContext> presContext;    
  frameManager->mPresShell->GetPresContext(getter_AddRefs(presContext));
  frameManager->mStyleSet->CantRenderReplacedElement(presContext,
                                                     aEvent->mFrame);        
#ifdef NOISY_EVENTS
  printf("FrameManager::HandlePLEvent() end for FM %p\n", aEvent->owner);
#endif
}

void
FrameManager::DestroyPLEvent(CantRenderReplacedElementEvent* aEvent)
{
  delete aEvent;
}

CantRenderReplacedElementEvent::CantRenderReplacedElementEvent(FrameManager* aFrameManager,
                                                               nsIFrame*     aFrame,
                                                               nsIPresShell* aPresShell)
{
  PL_InitEvent(this, aFrameManager,
               (PLHandleEventProc)&FrameManager::HandlePLEvent,
               (PLDestroyEventProc)&FrameManager::DestroyPLEvent);
  mFrame = aFrame;
  
  if (nsLayoutAtoms::objectFrame == aFrame->GetType()) {
    AddLoadGroupRequest(aPresShell);
  }
}

CantRenderReplacedElementEvent::~CantRenderReplacedElementEvent()
{
  RemoveLoadGroupRequest();
}

// Add a load group request in order to delay the onLoad handler when we have
// pending replacements
nsresult CantRenderReplacedElementEvent::AddLoadGroupRequest(nsIPresShell* aPresShell)
{
  nsCOMPtr<nsIDocument> doc;
  aPresShell->GetDocument(getter_AddRefs(doc));
  if (!doc) return NS_ERROR_FAILURE;

  nsresult rv = nsDummyLayoutRequest::Create(getter_AddRefs(mDummyLayoutRequest), aPresShell);
  if (NS_FAILED(rv)) return rv;
  if (!mDummyLayoutRequest) return NS_ERROR_FAILURE;

  nsCOMPtr<nsILoadGroup> loadGroup = doc->GetDocumentLoadGroup();
  if (!loadGroup) return NS_ERROR_FAILURE;
  
  rv = mDummyLayoutRequest->SetLoadGroup(loadGroup);
  if (NS_FAILED(rv)) return rv;
  
  mPresShell = do_GetWeakReference(aPresShell);

  return loadGroup->AddRequest(mDummyLayoutRequest, nsnull);
}

// Remove the load group request added above
nsresult CantRenderReplacedElementEvent::RemoveLoadGroupRequest()
{
  nsresult rv = NS_OK;

  if (mDummyLayoutRequest) {
    nsCOMPtr<nsIRequest> request = mDummyLayoutRequest;
    mDummyLayoutRequest = nsnull;

    nsCOMPtr<nsIPresShell> presShell = do_QueryReferent(mPresShell);
    if (!presShell) return NS_ERROR_FAILURE;
    
    nsCOMPtr<nsIDocument> doc;
    presShell->GetDocument(getter_AddRefs(doc));
    if (!doc) return NS_ERROR_FAILURE;;

    nsCOMPtr<nsILoadGroup> loadGroup = doc->GetDocumentLoadGroup();
    if (!loadGroup) return NS_ERROR_FAILURE;

    rv = loadGroup->RemoveRequest(request, nsnull, NS_OK);
  }
  return rv;
}

NS_IMETHODIMP
FrameManager::CantRenderReplacedElement(nsIFrame*       aFrame)
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);
#ifdef NOISY_EVENTS
  printf("%p FrameManager::CantRenderReplacedElement called\n", this);
#endif

  // We need to notify the style stystem, but post the notification so it
  // doesn't happen now
  nsresult rv;
  nsCOMPtr<nsIEventQueueService> eventService = do_GetService(kEventQueueServiceCID, &rv);

  if (eventService) {
    nsCOMPtr<nsIEventQueue> eventQueue;
    rv = eventService->GetThreadEventQueue(NS_CURRENT_THREAD, 
                                           getter_AddRefs(eventQueue));

    if (NS_SUCCEEDED(rv) && eventQueue) {
      // Verify that there isn't already a posted event associated with
      // this frame.
      if (*FindPostedEventFor(aFrame))
        return NS_OK;

      CantRenderReplacedElementEvent* ev;

      // Create a new event
      ev = new CantRenderReplacedElementEvent(this, aFrame, mPresShell);

      // Post the event
      rv = eventQueue->PostEvent(ev);
      if (NS_FAILED(rv)) {
        NS_ERROR("failed to post event");
        PL_DestroyEvent(ev);
      }
      else {
        // Add the event to our linked list of posted events
        ev->mNext = mPostedEvents;
        mPostedEvents = ev;
      }
    }
  }

  return rv;
}

#ifdef NS_DEBUG
static void
DumpContext(nsIFrame* aFrame, nsStyleContext* aContext)
{
  if (aFrame) {
    fputs("frame: ", stdout);
    nsAutoString  name;
    nsIFrameDebug*  frameDebug;

    if (NS_SUCCEEDED(aFrame->QueryInterface(NS_GET_IID(nsIFrameDebug), (void**)&frameDebug))) {
      frameDebug->GetFrameName(name);
      fputs(NS_LossyConvertUCS2toASCII(name).get(), stdout);
    }
    fprintf(stdout, " (%p)", NS_STATIC_CAST(void*, aFrame));
  }
  if (aContext) {
    fprintf(stdout, " style: %p ", NS_STATIC_CAST(void*, aContext));

    nsIAtom* pseudoTag = aContext->GetPseudoType();
    if (pseudoTag) {
      nsAutoString  buffer;
      pseudoTag->ToString(buffer);
      fputs(NS_LossyConvertUCS2toASCII(buffer).get(), stdout);
      fputs(" ", stdout);
    }

/* XXXdwh fix debugging here.  Need to add a List method to nsRuleNode
   and have the context call list on its rule node.
    PRInt32 count = aContext->GetStyleRuleCount();
    if (0 < count) {
      fputs("{\n", stdout);
      nsISupportsArray* rules = aContext->GetStyleRules();
      PRInt32 ix;
      for (ix = 0; ix < count; ix++) {
        nsIStyleRule* rule = (nsIStyleRule*)rules->ElementAt(ix);
        rule->List(stdout, 1);
        NS_RELEASE(rule);
      }
      NS_RELEASE(rules);
      fputs("}\n", stdout);
    }
    else 
    */
    {
      fputs("{}\n", stdout);
    }
  }
}

static void
VerifySameTree(nsStyleContext* aContext1, nsStyleContext* aContext2)
{
  nsStyleContext* top1 = aContext1;
  nsStyleContext* top2 = aContext2;
  nsStyleContext* parent;
  for (;;) {
    parent = top1->GetParent();
    if (!parent)
      break;
    top1 = parent;
  }
  for (;;) {
    parent = top2->GetParent();
    if (!parent)
      break;
    top2 = parent;
  }
  if (top1 != top2)
    printf("Style contexts are not in the same style context tree.\n");
}

static void
VerifyContextParent(nsIPresContext* aPresContext, nsIFrame* aFrame, 
                    nsStyleContext* aContext, nsStyleContext* aParentContext)
{
  // get the contexts not provided
  if (!aContext) {
    aContext = aFrame->GetStyleContext();
  }

  if (!aParentContext) {
    // Get the correct parent context from the frame
    //  - if the frame is a placeholder, we get the out of flow frame's context 
    //    as the parent context instead of asking the frame

    // get the parent context from the frame (indirectly)
    nsIFrame* providerFrame = nsnull;
    PRBool providerIsChild;
    aFrame->GetParentStyleContextFrame(aPresContext,
                                       &providerFrame, &providerIsChild);
    if (providerFrame)
      aParentContext = providerFrame->GetStyleContext();
    // aParentContext could still be null
  }

  NS_ASSERTION(aContext, "Failure to get required contexts");
  nsStyleContext* actualParentContext = aContext->GetParent();

  if (aParentContext) {
    if (aParentContext != actualParentContext) {
      DumpContext(aFrame, aContext);
      if (aContext == aParentContext) {
        fputs("Using parent's style context\n\n", stdout);
      }
      else {
        fputs("Wrong parent style context: ", stdout);
        DumpContext(nsnull, actualParentContext);
        fputs("should be using: ", stdout);
        DumpContext(nsnull, aParentContext);
        VerifySameTree(actualParentContext, aParentContext);
        fputs("\n", stdout);
      }
    }
  }
  else {
    if (actualParentContext) {
      DumpContext(aFrame, aContext);
      fputs("Has parent context: ", stdout);
      DumpContext(nsnull, actualParentContext);
      fputs("Should be null\n\n", stdout);
    }
  }
}

static void
VerifyStyleTree(nsIPresContext* aPresContext, nsIFrame* aFrame, nsStyleContext* aParentContext)
{
  nsStyleContext*  context = aFrame->GetStyleContext();
  VerifyContextParent(aPresContext, aFrame, context, nsnull);

  PRInt32 listIndex = 0;
  nsIAtom* childList = nsnull;
  nsIFrame* child;

  do {
    child = nsnull;
    nsresult result = aFrame->FirstChild(aPresContext, childList, &child);
    while ((NS_SUCCEEDED(result)) && child) {
      if (NS_FRAME_OUT_OF_FLOW != (child->GetStateBits() & NS_FRAME_OUT_OF_FLOW)) {
        // only do frames that are in flow
        if (nsLayoutAtoms::placeholderFrame == child->GetType()) { 
          // placeholder: first recirse and verify the out of flow frame,
          // then verify the placeholder's context
          nsIFrame* outOfFlowFrame = ((nsPlaceholderFrame*)child)->GetOutOfFlowFrame();
          NS_ASSERTION(outOfFlowFrame, "no out-of-flow frame");

          // recurse to out of flow frame, letting the parent context get resolved
          VerifyStyleTree(aPresContext, outOfFlowFrame, nsnull);

          // verify placeholder using the parent frame's context as
          // parent context
          VerifyContextParent(aPresContext, child, nsnull, nsnull);
        }
        else { // regular frame
          VerifyStyleTree(aPresContext, child, nsnull);
        }
      }
      child = child->GetNextSibling();
    }

    NS_IF_RELEASE(childList);
    aFrame->GetAdditionalChildListName(listIndex++, &childList);
  } while (childList);
  
  // do additional contexts 
  PRInt32 contextIndex = -1;
  while (1) {
    nsStyleContext* extraContext = aFrame->GetAdditionalStyleContext(++contextIndex);
    if (extraContext) {
      VerifyContextParent(aPresContext, aFrame, extraContext, context);
    }
    else {
      break;
    }
  }
}

NS_IMETHODIMP
FrameManager::DebugVerifyStyleTree(nsIFrame* aFrame)
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);
  if (aFrame) {
    nsStyleContext* context = aFrame->GetStyleContext();
    nsStyleContext* parentContext = context->GetParent();
    VerifyStyleTree(GetPresContext(), aFrame, parentContext);
  }
  return NS_OK;
}

#endif // DEBUG

NS_IMETHODIMP
FrameManager::ReParentStyleContext(nsIFrame* aFrame, 
                                   nsStyleContext* aNewParentContext)
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);
  nsresult result = NS_ERROR_NULL_POINTER;
  if (aFrame) {
#ifdef NS_DEBUG
    DebugVerifyStyleTree(aFrame);
#endif

    nsStyleContext* oldContext = aFrame->GetStyleContext();
    if (oldContext) {
      nsIPresContext *presContext = GetPresContext();
      nsRefPtr<nsStyleContext> newContext;
      result = NS_OK;
      newContext = mStyleSet->ReParentStyleContext(presContext, oldContext,
                                                   aNewParentContext);
      if (newContext) {
        if (newContext != oldContext) {
          PRInt32 listIndex = 0;
          nsIAtom* childList = nsnull;
          nsIFrame* child;

          do {
            child = nsnull;
            result = aFrame->FirstChild(presContext, childList, &child);
            while ((NS_SUCCEEDED(result)) && child) {
              if (NS_FRAME_OUT_OF_FLOW != (child->GetStateBits() & NS_FRAME_OUT_OF_FLOW)) {
                // only do frames that are in flow
                if (nsLayoutAtoms::placeholderFrame == child->GetType()) {
                  // get out of flow frame and recurse there
                  nsIFrame* outOfFlowFrame = ((nsPlaceholderFrame*)child)->GetOutOfFlowFrame();
                  NS_ASSERTION(outOfFlowFrame, "no out-of-flow frame");

                  result = ReParentStyleContext(outOfFlowFrame, newContext);

                  // reparent placeholder's context under out of flow frame
                  nsStyleContext* outOfFlowContext = outOfFlowFrame->GetStyleContext();
                  ReParentStyleContext(child, outOfFlowContext);
                }
                else { // regular frame
                  result = ReParentStyleContext(child, newContext);
                }
              }

              child = child->GetNextSibling();
            }

            NS_IF_RELEASE(childList);
            aFrame->GetAdditionalChildListName(listIndex++, &childList);
          } while (childList);
          
          aFrame->SetStyleContext(presContext, newContext);

          // do additional contexts 
          PRInt32 contextIndex = -1;
          while (1) {
            nsStyleContext* oldExtraContext = aFrame->GetAdditionalStyleContext(++contextIndex);
            if (oldExtraContext) {
              nsRefPtr<nsStyleContext> newExtraContext;
              newExtraContext = mStyleSet->ReParentStyleContext(presContext,
                                                                oldExtraContext,
                                                                newContext);
              if (newExtraContext) {
                aFrame->SetAdditionalStyleContext(contextIndex, newExtraContext);
              }
            }
            else {
              result = NS_OK; // ok not to have extras (or run out)
              break;
            }
          }
#ifdef NS_DEBUG
          VerifyStyleTree(GetPresContext(), aFrame, aNewParentContext);
#endif
        }
      }
    }
  }
  return result;
}

static PRBool
HasAttributeContent(nsStyleContext* aStyleContext, 
                    PRInt32 aNameSpaceID,
                    nsIAtom* aAttribute)
{
  PRBool  result = PR_FALSE;
  if (aStyleContext) {
    const nsStyleContent* content = aStyleContext->GetStyleContent();
    PRUint32 count = content->ContentCount();
    while ((0 < count) && (! result)) {
      const nsStyleContentData &data = content->ContentAt(--count);
      if (eStyleContentType_Attr == data.mType) {
        nsIAtom* attrName = nsnull;
        PRInt32 attrNameSpace = kNameSpaceID_None;
        nsAutoString contentString(data.mContent.mString);
        PRInt32 barIndex = contentString.FindChar('|'); // CSS namespace delimiter
        if (-1 != barIndex) {
          nsAutoString  nameSpaceVal;
          contentString.Left(nameSpaceVal, barIndex);
          PRInt32 error;
          attrNameSpace = nameSpaceVal.ToInteger(&error, 10);
          contentString.Cut(0, barIndex + 1);
          if (!contentString.IsEmpty()) {
            attrName = NS_NewAtom(contentString);
          }
        }
        else {
          attrName = NS_NewAtom(contentString);
        }
        if ((attrName == aAttribute) && 
            ((attrNameSpace == aNameSpaceID) || 
             (attrNameSpace == kNameSpaceID_Unknown))) {
          result = PR_TRUE;
        }
        NS_IF_RELEASE(attrName);
      }
    }
  }
  return result;
}

static nsChangeHint
CaptureChange(nsStyleContext* aOldContext, nsStyleContext* aNewContext,
              nsIFrame* aFrame, nsIContent* aContent,
              nsStyleChangeList& aChangeList, nsChangeHint aMinChange)
{
  nsChangeHint ourChange = NS_STYLE_HINT_NONE;
  ourChange = aOldContext->CalcStyleDifference(aNewContext);
  if (NS_UpdateHint(aMinChange, ourChange)) {
    aChangeList.AppendChange(aFrame, aContent, ourChange);
  }
  return aMinChange;
}

void
FrameManager::ReResolveStyleContext(nsIPresContext* aPresContext,
                                    nsIFrame* aFrame,
                                    nsIContent* aParentContent,
                                    PRInt32 aAttrNameSpaceID,
                                    nsIAtom* aAttribute,
                                    nsStyleChangeList& aChangeList, 
                                    nsChangeHint aMinChange,
                                    nsChangeHint& aResultChange)
{
  // XXXldb get new context from prev-in-flow if possible, to avoid
  // duplication.  (Or should we just let |GetContext| handle that?)
  // Getting the hint would be nice too, but that's harder.

  // XXXbryner we may be able to avoid some of the refcounting goop here.
  // We do need a reference to oldContext for the lifetime of this function, and it's possible
  // that the frame has the last reference to it, so AddRef it here.

  nsStyleContext* oldContext = aFrame->GetStyleContext();
  nsresult result = NS_OK;

  if (oldContext) {
    oldContext->AddRef();
    nsIAtom* pseudoTag = oldContext->GetPseudoType();
    nsIContent* localContent = aFrame->GetContent();
    nsIContent* content = localContent ? localContent : aParentContent;

    if (aParentContent && aAttribute) { // attribute came from parent, we don't care about it here when recursing
      if (!(aFrame->GetStateBits() & NS_FRAME_GENERATED_CONTENT)) { // keep it for generated content
        aAttribute = nsnull;
      }
    }

    nsStyleContext* parentContext;
    nsIFrame* resolvedChild = nsnull;
    // Get the frame providing the parent style context.  If it is a
    // child, then resolve the provider first.
    nsIFrame* providerFrame = nsnull;
    PRBool providerIsChild = PR_FALSE;
    aFrame->GetParentStyleContextFrame(aPresContext,
                                       &providerFrame, &providerIsChild); 
    if (!providerIsChild) {
      if (providerFrame)
        parentContext = providerFrame->GetStyleContext();
      else
        parentContext = nsnull;
    }
    else {
      // resolve the provider here (before aFrame below)
      ReResolveStyleContext(aPresContext, providerFrame, content, 
                            aAttrNameSpaceID, aAttribute, aChangeList,
                            aMinChange, aResultChange);
      // The provider's new context becomes the parent context of
      // aFrame's context.
      parentContext = providerFrame->GetStyleContext();
      // Set |resolvedChild| so we don't bother resolving the
      // provider again.
      resolvedChild = providerFrame;
    }
    
    // do primary context
    nsStyleContext* newContext = nsnull;
    if (pseudoTag == nsCSSAnonBoxes::mozNonElement) {
      NS_ASSERTION(localContent,
                   "non pseudo-element frame without content node");
      newContext = aPresContext->ResolveStyleContextForNonElement(parentContext).get();
    }
    else if (pseudoTag) {
      nsIContent* pseudoContent =
          aParentContent ? aParentContent : localContent;
      if (pseudoTag == nsCSSPseudoElements::before ||
          pseudoTag == nsCSSPseudoElements::after) {
        // XXX what other pseudos do we need to treat like this?
        newContext = aPresContext->ProbePseudoStyleContextFor(pseudoContent,
                                                              pseudoTag,
                                                              parentContext).get();
        if (!newContext) {
          // This pseudo should no longer exist; gotta reframe
          NS_UpdateHint(aMinChange, nsChangeHint_ReconstructFrame);
          aChangeList.AppendChange(aFrame, pseudoContent,
                                   nsChangeHint_ReconstructFrame);
          // We're reframing anyway; just keep the same context
          newContext = oldContext;
          newContext->AddRef();
        }
      } else {
        newContext = aPresContext->ResolvePseudoStyleContextFor(pseudoContent,
                                                                pseudoTag,
                                                                parentContext).get();
      }
    }
    else {
      NS_ASSERTION(localContent,
                   "non pseudo-element frame without content node");
      newContext = aPresContext->ResolveStyleContextFor(content, parentContext).get();
    }
    NS_ASSERTION(newContext, "failed to get new style context");
    if (newContext) {
      if (!parentContext) {
        if (oldContext->GetRuleNode() == newContext->GetRuleNode()) {
          // We're the root of the style context tree and the new style
          // context returned has the same rule node.  This means that
          // we can use FindChildWithRules to keep a lot of the old
          // style contexts around.  However, we need to start from the
          // same root.
          newContext->Release();
          newContext = oldContext;
          newContext->AddRef();
        }
      }

      if (newContext != oldContext) {
        aMinChange = CaptureChange(oldContext, newContext, aFrame, content, aChangeList, aMinChange);
        if (!(aMinChange & (nsChangeHint_ReconstructFrame | nsChangeHint_ReconstructDoc))) {
          // if frame gets regenerated, let it keep old context
          aFrame->SetStyleContext(aPresContext, newContext);
        }
        // if old context had image and new context does not have the same image, 
        // stop the image load for the frame
        const nsStyleBackground* oldColor = oldContext->GetStyleBackground();
        const nsStyleBackground* newColor = newContext->GetStyleBackground();

        PRBool equal;
        if (oldColor->mBackgroundImage &&
            (!newColor->mBackgroundImage ||
             NS_FAILED(oldColor->mBackgroundImage->Equals(
                                        newColor->mBackgroundImage, &equal)) ||
             !equal)) {
          // stop the image loading for the frame, the image has changed
          aPresContext->StopImagesFor(aFrame);
        }
      }
      else {
        if (pseudoTag && pseudoTag != nsCSSAnonBoxes::mozNonElement &&
            aAttribute &&
            (!(aMinChange &
               (nsChangeHint_ReflowFrame | nsChangeHint_ReconstructFrame
                | nsChangeHint_ReconstructDoc))) &&
            HasAttributeContent(oldContext, aAttrNameSpaceID, aAttribute)) {
          aChangeList.AppendChange(aFrame, content, NS_STYLE_HINT_REFLOW);
        }
      }
      oldContext->Release();
    }
    else {
      NS_ERROR("resolve style context failed");
      newContext = oldContext;  // new context failed, recover... (take ref)
      oldContext = nsnull;
    }

    // do additional contexts 
    PRInt32 contextIndex = -1;
    while (1 == 1) {
      nsStyleContext* oldExtraContext = nsnull;
      oldExtraContext = aFrame->GetAdditionalStyleContext(++contextIndex);
      if (oldExtraContext) {
        nsStyleContext* newExtraContext = nsnull;
        pseudoTag = oldExtraContext->GetPseudoType();
        NS_ASSERTION(pseudoTag &&
                     pseudoTag != nsCSSAnonBoxes::mozNonElement,
                     "extra style context is not pseudo element");
        newExtraContext = aPresContext->ResolvePseudoStyleContextFor(content,
                                                                     pseudoTag,
                                                                     newContext).get();
        if (newExtraContext) {
          if (oldExtraContext != newExtraContext) {
            aMinChange = CaptureChange(oldExtraContext, newExtraContext, aFrame, 
                                       content, aChangeList, aMinChange);
            if (!(aMinChange & (nsChangeHint_ReconstructFrame | nsChangeHint_ReconstructDoc))) {
              aFrame->SetAdditionalStyleContext(contextIndex, newExtraContext);
            }
          }
          else {
#if 0
            // XXXldb |oldContext| is null by this point, so this will
            // never do anything.
            if (pseudoTag && pseudoTag != nsCSSAnonBoxes::mozNonElement &&
                aAttribute && (aMinChange < NS_STYLE_HINT_REFLOW) &&
                HasAttributeContent(oldContext, aAttrNameSpaceID, aAttribute)) {
              aChangeList.AppendChange(aFrame, content, NS_STYLE_HINT_REFLOW);
            }
#endif
          }
          newExtraContext->Release();
        }
      }
      else {
        break;
      }
    }

    // now look for undisplayed child content and pseudos
    if (localContent && mUndisplayedMap) {
      for (UndisplayedNode* undisplayed =
                                   mUndisplayedMap->GetFirstNode(localContent);
           undisplayed; undisplayed = undisplayed->mNext) {
        nsRefPtr<nsStyleContext> undisplayedContext;
        pseudoTag = undisplayed->mStyle->GetPseudoType();
        if (pseudoTag == nsnull) {  // child content
          undisplayedContext = aPresContext->ResolveStyleContextFor(undisplayed->mContent,
                                                                    newContext);
        }
        else if (pseudoTag == nsCSSAnonBoxes::mozNonElement) {
          undisplayedContext = aPresContext->ResolveStyleContextForNonElement(newContext);
        }
        else {  // pseudo element
          NS_NOTREACHED("no pseudo elements in undisplayed map");
          NS_ASSERTION(pseudoTag, "pseudo element without tag");
          undisplayedContext = aPresContext->ResolvePseudoStyleContextFor(localContent,
                                                                          pseudoTag,
                                                                          newContext);
        }
        if (undisplayedContext) {
          const nsStyleDisplay* display = undisplayedContext->GetStyleDisplay();
          if (display->mDisplay != NS_STYLE_DISPLAY_NONE) {
            aChangeList.AppendChange(nsnull,
                                     undisplayed->mContent
                                       ? NS_STATIC_CAST(nsIContent*,
                                                        undisplayed->mContent)
                                       : localContent, 
                                     NS_STYLE_HINT_FRAMECHANGE);
            // The node should be removed from the undisplayed map when
            // we reframe it.
          } else {
            // update the undisplayed node with the new context
            undisplayed->mStyle = undisplayedContext;
          }
        }
      }
    }

    aResultChange = aMinChange;

    if (!(aMinChange & (nsChangeHint_ReconstructFrame | nsChangeHint_ReconstructDoc))) {
      // Make sure not to do this for pseudo-frames -- those can't have :before
      // or :after content.
      if (!pseudoTag && localContent &&
          localContent->IsContentOfType(nsIContent::eELEMENT)) {
        // Check for a new :before pseudo and an existing :before
        // frame, but only if the frame is the first-in-flow.
        nsIFrame* prevInFlow = nsnull;
        aFrame->GetPrevInFlow(&prevInFlow);
        if (!prevInFlow) {
          // Checking for a :before frame is cheaper than getting the
          // :before style context.
          if (!nsLayoutUtils::GetBeforeFrame(aFrame, aPresContext) &&
              nsLayoutUtils::HasPseudoStyle(localContent, newContext,
                                            nsCSSPseudoElements::before,
                                            aPresContext)) {
            // Have to create the new :before frame
            NS_UpdateHint(aMinChange, nsChangeHint_ReconstructFrame);
            aChangeList.AppendChange(aFrame, content,
                                     nsChangeHint_ReconstructFrame);
          }
        }
      }
    }

    
    if (!(aMinChange & (nsChangeHint_ReconstructFrame | nsChangeHint_ReconstructDoc))) {
      // Make sure not to do this for pseudo-frames -- those can't have :before
      // or :after content.
      if (!pseudoTag && localContent &&
          localContent->IsContentOfType(nsIContent::eELEMENT)) {
        // Check for new :after content, but only if the frame is the
        // first-in-flow.
        nsIFrame* nextInFlow = nsnull;
        aFrame->GetNextInFlow(&nextInFlow);

        if (!nextInFlow) {
          // Getting the :after frame is more expensive than getting the pseudo
          // context, so get the pseudo context first.
          if (nsLayoutUtils::HasPseudoStyle(localContent, newContext,
                                            nsCSSPseudoElements::after,
                                            aPresContext) &&
              !nsLayoutUtils::GetAfterFrame(aFrame, aPresContext)) {
            // have to create the new :after frame
            NS_UpdateHint(aMinChange, nsChangeHint_ReconstructFrame);
            aChangeList.AppendChange(aFrame, content,
                                     nsChangeHint_ReconstructFrame);
          }
        }      
      }
    }
    
    if (!(aMinChange & (nsChangeHint_ReconstructFrame | nsChangeHint_ReconstructDoc))) {
      
      // There is no need to waste time crawling into a frame's children on a frame change.
      // The act of reconstructing frames will force new style contexts to be resolved on all
      // of this frame's descendants anyway, so we want to avoid wasting time processing
      // style contexts that we're just going to throw away anyway. - dwh

      // now do children
      PRInt32 listIndex = 0;
      nsIAtom* childList = nsnull;
      nsChangeHint childChange;

      do {
        nsIFrame* child = nsnull;
        result = aFrame->FirstChild(aPresContext, childList, &child);
        while (NS_SUCCEEDED(result) && child) {
          if (!(child->GetStateBits() & NS_FRAME_OUT_OF_FLOW)) {
            // only do frames that are in flow
            if (nsLayoutAtoms::placeholderFrame == child->GetType()) { // placeholder
              // get out of flow frame and recur there
              nsIFrame* outOfFlowFrame =
                NS_STATIC_CAST(nsPlaceholderFrame*,child)->GetOutOfFlowFrame();
              NS_ASSERTION(outOfFlowFrame, "no out-of-flow frame");
              NS_ASSERTION(outOfFlowFrame != resolvedChild,
                           "out-of-flow frame not a true descendant");

              // |nsFrame::GetParentStyleContextFrame| checks being out
              // of flow so that this works correctly.
              ReResolveStyleContext(aPresContext, outOfFlowFrame, content,
                                    aAttrNameSpaceID, aAttribute,
                                    aChangeList, aMinChange, childChange);

              // reresolve placeholder's context under the same parent
              // as the out-of-flow frame
              ReResolveStyleContext(aPresContext, child, content,
                                    kNameSpaceID_Unknown, nsnull,
                                    aChangeList, aMinChange, childChange);
            }
            else {  // regular child frame
              if (child != resolvedChild) {
                ReResolveStyleContext(aPresContext, child, content,
                                      aAttrNameSpaceID, aAttribute,
                                      aChangeList, aMinChange, childChange);
              } else {
                NOISY_TRACE_FRAME("child frame already resolved as descendent, skipping",aFrame);
              }
            }
          }
          child = child->GetNextSibling();
        }

        NS_IF_RELEASE(childList);
        aFrame->GetAdditionalChildListName(listIndex++, &childList);
      } while (childList);
      // XXX need to do overflow frames???
    }

    newContext->Release();
  }
}

NS_IMETHODIMP
FrameManager::ComputeStyleChangeFor(nsIFrame* aFrame, 
                                    PRInt32 aAttrNameSpaceID,
                                    nsIAtom* aAttribute,
                                    nsStyleChangeList& aChangeList,
                                    nsChangeHint aMinChange,
                                    nsChangeHint& aTopLevelChange)
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);
  aTopLevelChange = aMinChange;
  nsIFrame* frame = aFrame;
  nsIFrame* frame2 = aFrame;

#ifdef DEBUG
  {
    nsIFrame* prevInFlow;
    frame->GetPrevInFlow(&prevInFlow);
    NS_ASSERTION(!prevInFlow, "must start with the first in flow");
  }
#endif

  // We want to start with this frame and walk all its next-in-flows,
  // as well as all its special siblings and their next-in-flows,
  // reresolving style on all the frames we encounter in this walk.
  
  do {
    // Outer loop over special siblings
    do {
      // Inner loop over next-in-flows of the current frame
      nsChangeHint frameChange;
      ReResolveStyleContext(GetPresContext(), frame, nsnull,
                            aAttrNameSpaceID, aAttribute,
                            aChangeList, aTopLevelChange, frameChange);
      NS_UpdateHint(aTopLevelChange, frameChange);

      if (aTopLevelChange & (nsChangeHint_ReconstructDoc | nsChangeHint_ReconstructFrame)) {
        // If it's going to cause a framechange, then don't bother
        // with the continuations or special siblings since they'll be
        // clobbered by the frame reconstruct anyway.
#ifdef NS_DEBUG
        nsIFrame* prevInFlow;
        frame->GetPrevInFlow(&prevInFlow);
        NS_ASSERTION(!prevInFlow, "continuing frame had more severe impact than first-in-flow");
#endif
        return NS_OK;
      }

      frame->GetNextInFlow(&frame);
    } while (frame);

    // Might we have special siblings?
    if (!(frame2->GetStateBits() & NS_FRAME_IS_SPECIAL)) {
      // nothing more to do here
      return NS_OK;
    }
    
    GetFrameProperty(frame2, nsLayoutAtoms::IBSplitSpecialSibling, 0, (void**)&frame2);
    frame = frame2;
  } while (frame2);
  return NS_OK;
}


NS_IMETHODIMP
FrameManager::HasAttributeDependentStyle(nsIContent *aContent,
                                         nsIAtom *aAttribute,
                                         PRInt32 aModType,
                                         PRBool *aResult)
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);

  if (aAttribute == nsHTMLAtoms::style) {
    // Perhaps should check that it's XUL, SVG, (or HTML) namespace, but
    // it doesn't really matter.  Or we could even let
    // HTMLCSSStyleSheetImpl::HasAttributeDependentStyle handle it.
    *aResult = PR_TRUE;
    return NS_OK;
  }

  return mStyleSet->HasAttributeDependentStyle(GetPresContext(), aContent,
                                               aAttribute, aModType,
                                               aResult);
}

// Capture state for a given frame.
// Accept a content id here, in some cases we may not have content (scroll position)
NS_IMETHODIMP
FrameManager::CaptureFrameStateFor(nsIFrame* aFrame,
                                   nsILayoutHistoryState* aState,
                                   nsIStatefulFrame::SpecialStateID aID)
{
  NS_ENSURE_TRUE(mPresShell && aFrame && aState, NS_ERROR_FAILURE);

  // Only capture state for stateful frames
  nsIStatefulFrame* statefulFrame = nsnull;
  aFrame->QueryInterface(NS_GET_IID(nsIStatefulFrame), (void**) &statefulFrame);
  if (!statefulFrame) {
    return NS_OK;
  }

  // Capture the state, exit early if we get null (nothing to save)
  nsCOMPtr<nsIPresState> frameState;
  nsresult rv = NS_OK;
  rv = statefulFrame->SaveState(GetPresContext(), getter_AddRefs(frameState));
  if (!frameState) {
    return NS_OK;
  }

  // Generate the hash key to store the state under
  // Exit early if we get empty key
  nsCAutoString stateKey;
  rv = nsContentUtils::GenerateStateKey(aFrame->GetContent(), aID, stateKey);
  if(NS_FAILED(rv) || stateKey.IsEmpty()) {
    return rv;
  }

  // Store the state
  return aState->AddState(stateKey, frameState);
}

NS_IMETHODIMP
FrameManager::CaptureFrameState(nsIFrame* aFrame,
                                nsILayoutHistoryState* aState)
{
  nsresult rv = NS_OK;
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);
  NS_PRECONDITION(nsnull != aFrame && nsnull != aState, "null parameters passed in");

  rv = CaptureFrameStateFor(aFrame, aState);

  // Now capture state recursively for the frame hierarchy rooted at aFrame
  nsIAtom*  childListName = nsnull;
  PRInt32   childListIndex = 0;
  do {    
    nsIFrame* childFrame;
    aFrame->FirstChild(GetPresContext(), childListName, &childFrame);
    while (childFrame) {             
      rv = CaptureFrameState(childFrame, aState);
      // Get the next sibling child frame
      childFrame = childFrame->GetNextSibling();
    }
    NS_IF_RELEASE(childListName);
    aFrame->GetAdditionalChildListName(childListIndex++, &childListName);
  } while (childListName);

  return rv;
}

// Restore state for a given frame.
// Accept a content id here, in some cases we may not have content (scroll position)
NS_IMETHODIMP
FrameManager::RestoreFrameStateFor(nsIFrame* aFrame,
                                   nsILayoutHistoryState* aState,
                                   nsIStatefulFrame::SpecialStateID aID)
{
  NS_ENSURE_TRUE(mPresShell && aFrame && aState, NS_ERROR_FAILURE);

  // Only capture state for stateful frames
  nsIStatefulFrame* statefulFrame = nsnull;
  aFrame->QueryInterface(NS_GET_IID(nsIStatefulFrame), (void**) &statefulFrame);
  if (!statefulFrame) {
    return NS_OK;
  }

  // Generate the hash key the state was stored under
  // Exit early if we get empty key
  nsIContent* content = aFrame->GetContent();
  // If we don't have content, we can't generate a hash
  // key and there's probably no state information for us.
  if (!content) {
    return NS_OK;
  }

  nsCAutoString stateKey;
  nsresult rv = nsContentUtils::GenerateStateKey(content, aID, stateKey);
  if (NS_FAILED(rv) || stateKey.IsEmpty()) {
    return rv;
  }

  // Get the state from the hash
  nsCOMPtr<nsIPresState> frameState;
  rv = aState->GetState(stateKey, getter_AddRefs(frameState));
  if (!frameState) {
    return NS_OK;
  }

  // Restore it
  rv = statefulFrame->RestoreState(GetPresContext(), frameState);
  NS_ENSURE_SUCCESS(rv, rv);

  // If we restore ok, remove the state from the state table
  return aState->RemoveState(stateKey);
}

NS_IMETHODIMP
FrameManager::RestoreFrameState(nsIFrame* aFrame, nsILayoutHistoryState* aState)
{
  nsresult rv = NS_OK;
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);
  NS_PRECONDITION(nsnull != aFrame && nsnull != aState, "null parameters passed in");
  
  rv = RestoreFrameStateFor(aFrame, aState);

  // Now restore state recursively for the frame hierarchy rooted at aFrame
  nsIAtom*  childListName = nsnull;
  PRInt32   childListIndex = 0;
  do {    
    nsIFrame* childFrame;
    aFrame->FirstChild(GetPresContext(), childListName, &childFrame);
    while (childFrame) {
      rv = RestoreFrameState(childFrame, aState);
      // Get the next sibling child frame
      childFrame = childFrame->GetNextSibling();
    }
    NS_IF_RELEASE(childListName);
    aFrame->GetAdditionalChildListName(childListIndex++, &childListName);
  } while (childListName);

  return rv;
}

//----------------------------------------------------------------------

static PLHashNumber
HashKey(void* key)
{
  return NS_PTR_TO_INT32(key);
}

static PRIntn
CompareKeys(void* key1, void* key2)
{
  return key1 == key2;
}

void
FrameManager::DestroyPropertyList(nsIPresContext* aPresContext)
{
  if (mPropertyList) {
    while (mPropertyList) {
      PropertyList* tmp = mPropertyList;

      mPropertyList = mPropertyList->mNext;
      tmp->Destroy(aPresContext);
      delete tmp;
    }
  }
}

FrameManager::PropertyList*
FrameManager::GetPropertyListFor(nsIAtom* aPropertyName) const
{
  PropertyList* result;

  for (result = mPropertyList; result; result = result->mNext) {
    if (result->mName.get() == aPropertyName) {
      break;
    }
  }

  return result;
}

void
FrameManager::RemoveAllPropertiesFor(nsIPresContext* aPresContext,
                                     nsIFrame*       aFrame)
{
  for (PropertyList* prop = mPropertyList; prop; prop = prop->mNext) {
    prop->RemovePropertyForFrame(aPresContext, aFrame);
  }
}

NS_IMETHODIMP
FrameManager::GetFrameProperty(nsIFrame* aFrame,
                               nsIAtom*  aPropertyName,
                               PRUint32  aOptions,
                               void**    aPropertyValue)
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);
  NS_PRECONDITION(aPropertyName && aFrame, "unexpected null param");

  PropertyList* propertyList = GetPropertyListFor(aPropertyName);
  if (propertyList) {
    PropertyListMapEntry *entry = NS_STATIC_CAST(PropertyListMapEntry*,
        PL_DHashTableOperate(&propertyList->mFrameValueMap, aFrame,
                             PL_DHASH_LOOKUP));
    if (PL_DHASH_ENTRY_IS_BUSY(entry)) {
      *aPropertyValue = entry->value;
      if (aOptions & NS_IFRAME_MGR_REMOVE_PROP) {
        // don't call propertyList->mDtorFunc.  That's the caller's job now.
        PL_DHashTableRawRemove(&propertyList->mFrameValueMap, entry);
      }
      return NS_OK;
    }
  }

  *aPropertyValue = 0;
  return NS_IFRAME_MGR_PROP_NOT_THERE;
}

NS_IMETHODIMP
FrameManager::SetFrameProperty(nsIFrame*               aFrame,
                               nsIAtom*                aPropertyName,
                               void*                   aPropertyValue,
                               NSFramePropertyDtorFunc aPropDtorFunc)
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);
  NS_PRECONDITION(aPropertyName && aFrame, "unexpected null param");

  PropertyList* propertyList = GetPropertyListFor(aPropertyName);

  if (propertyList) {
    // Make sure the dtor function matches
    if (aPropDtorFunc != propertyList->mDtorFunc) {
      return NS_ERROR_INVALID_ARG;
    }

  } else {
    propertyList = new PropertyList(aPropertyName, aPropDtorFunc);
    if (!propertyList)
      return NS_ERROR_OUT_OF_MEMORY;
    if (!propertyList->mFrameValueMap.ops) {
      delete propertyList;
      return NS_ERROR_OUT_OF_MEMORY;
    }

    propertyList->mNext = mPropertyList;
    mPropertyList = propertyList;
  }

  // The current property value (if there is one) is replaced and the current
  // value is destroyed
  nsresult result = NS_OK;
  PropertyListMapEntry *entry = NS_STATIC_CAST(PropertyListMapEntry*,
    PL_DHashTableOperate(&propertyList->mFrameValueMap, aFrame, PL_DHASH_ADD));
  if (!entry)
    return NS_ERROR_OUT_OF_MEMORY;
  // A NULL entry->key is the sign that the entry has just been allocated
  // for us.  If it's non-NULL then we have an existing entry.
  if (entry->key && propertyList->mDtorFunc) {
    nsCOMPtr<nsIPresContext> presContext;
    mPresShell->GetPresContext(getter_AddRefs(presContext));
    propertyList->mDtorFunc(presContext, aFrame, aPropertyName, entry->value);
    result = NS_IFRAME_MGR_PROP_OVERWRITTEN;
  }
  entry->key = aFrame;
  entry->value = aPropertyValue;

  return result;
}

NS_IMETHODIMP
FrameManager::RemoveFrameProperty(nsIFrame* aFrame,
                                  nsIAtom*  aPropertyName)
{
  NS_ENSURE_TRUE(mPresShell, NS_ERROR_NOT_AVAILABLE);
  NS_PRECONDITION(aPropertyName && aFrame, "unexpected null param");

  PropertyList* propertyList = GetPropertyListFor(aPropertyName);
  if (propertyList) {
    nsCOMPtr<nsIPresContext> presContext;
    mPresShell->GetPresContext(getter_AddRefs(presContext));
    
    if (propertyList->RemovePropertyForFrame(presContext, aFrame))
      return NS_OK;
  }

  return NS_IFRAME_MGR_PROP_NOT_THERE;
}

//----------------------------------------------------------------------

MOZ_DECL_CTOR_COUNTER(UndisplayedMap)

UndisplayedMap::UndisplayedMap(PRUint32 aNumBuckets)
{
  MOZ_COUNT_CTOR(UndisplayedMap);
  mTable = PL_NewHashTable(aNumBuckets, (PLHashFunction)HashKey,
                           (PLHashComparator)CompareKeys,
                           (PLHashComparator)nsnull,
                           nsnull, nsnull);
  mLastLookup = nsnull;
}

UndisplayedMap::~UndisplayedMap(void)
{
  MOZ_COUNT_DTOR(UndisplayedMap);
  Clear();
  PL_HashTableDestroy(mTable);
}

PLHashEntry**  
UndisplayedMap::GetEntryFor(nsIContent* aParentContent)
{
  if (mLastLookup && (aParentContent == (*mLastLookup)->key)) {
    return mLastLookup;
  }
  PLHashNumber hashCode = NS_PTR_TO_INT32(aParentContent);
  PLHashEntry** entry = PL_HashTableRawLookup(mTable, hashCode, aParentContent);
  if (*entry) {
    mLastLookup = entry;
  }
  return entry;
}

UndisplayedNode* 
UndisplayedMap::GetFirstNode(nsIContent* aParentContent)
{
  PLHashEntry** entry = GetEntryFor(aParentContent);
  if (*entry) {
    return (UndisplayedNode*)((*entry)->value);
  }
  return nsnull;
}

nsresult      
UndisplayedMap::AppendNodeFor(UndisplayedNode* aNode, nsIContent* aParentContent)
{
  PLHashEntry** entry = GetEntryFor(aParentContent);
  if (*entry) {
    UndisplayedNode*  node = (UndisplayedNode*)((*entry)->value);
    while (node->mNext) {
      if (node->mContent == aNode->mContent) {
        // We actually need to check this in optimized builds because
        // there are some callers that do this.  See bug 118014, bug
        // 136704, etc.
        NS_NOTREACHED("node in map twice");
        delete aNode;
        return NS_OK;
      }
      node = node->mNext;
    }
    node->mNext = aNode;
  }
  else {
    PLHashNumber hashCode = NS_PTR_TO_INT32(aParentContent);
    PL_HashTableRawAdd(mTable, entry, hashCode, aParentContent, aNode);
    mLastLookup = nsnull; // hashtable may have shifted bucket out from under us
  }
  return NS_OK;
}

nsresult 
UndisplayedMap::AddNodeFor(nsIContent* aParentContent, nsIContent* aChild, 
                           nsStyleContext* aStyle)
{
  UndisplayedNode*  node = new UndisplayedNode(aChild, aStyle);
  if (! node) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  return AppendNodeFor(node, aParentContent);
}

nsresult 
UndisplayedMap::RemoveNodeFor(nsIContent* aParentContent, UndisplayedNode* aNode)
{
  PLHashEntry** entry = GetEntryFor(aParentContent);
  NS_ASSERTION(*entry, "content not in map");
  if (*entry) {
    if ((UndisplayedNode*)((*entry)->value) == aNode) {  // first node
      if (aNode->mNext) {
        (*entry)->value = aNode->mNext;
        aNode->mNext = nsnull;
      }
      else {
        PL_HashTableRawRemove(mTable, entry, *entry);
        mLastLookup = nsnull; // hashtable may have shifted bucket out from under us
      }
    }
    else {
      UndisplayedNode*  node = (UndisplayedNode*)((*entry)->value);
      while (node->mNext) {
        if (node->mNext == aNode) {
          node->mNext = aNode->mNext;
          aNode->mNext = nsnull;
          break;
        }
        node = node->mNext;
      }
    }
  }
  delete aNode;
  return NS_OK;
}

nsresult
UndisplayedMap::RemoveNodesFor(nsIContent* aParentContent)
{
  PLHashEntry** entry = GetEntryFor(aParentContent);
  NS_ASSERTION(entry, "content not in map");
  if (*entry) {
    UndisplayedNode*  node = (UndisplayedNode*)((*entry)->value);
    NS_ASSERTION(node, "null node for non-null entry in UndisplayedMap");
    delete node;
    PL_HashTableRawRemove(mTable, entry, *entry);
    mLastLookup = nsnull; // hashtable may have shifted bucket out from under us
  }
  return NS_OK;
}

static PRIntn PR_CALLBACK
RemoveUndisplayedEntry(PLHashEntry* he, PRIntn i, void* arg)
{
  UndisplayedNode*  node = (UndisplayedNode*)(he->value);
  delete node;
  // Remove and free this entry and continue enumerating
  return HT_ENUMERATE_REMOVE | HT_ENUMERATE_NEXT;
}

void
UndisplayedMap::Clear(void)
{
  mLastLookup = nsnull;
  PL_HashTableEnumerateEntries(mTable, RemoveUndisplayedEntry, 0);
}

//----------------------------------------------------------------------
    
FrameManager::PropertyList::PropertyList(nsIAtom*                aName,
                                         NSFramePropertyDtorFunc aDtorFunc)
  : mName(aName), mDtorFunc(aDtorFunc), mNext(nsnull)
{
  PL_DHashTableInit(&mFrameValueMap, PL_DHashGetStubOps(), this,
                    sizeof(PropertyListMapEntry), 16);
}

FrameManager::PropertyList::~PropertyList()
{
  PL_DHashTableFinish(&mFrameValueMap);
}

PR_STATIC_CALLBACK(PLDHashOperator)
DestroyPropertyEnumerator(PLDHashTable *table, PLDHashEntryHdr *hdr,
                          PRUint32 number, void *arg)
{
  FrameManager::PropertyList *propList =
      NS_STATIC_CAST(FrameManager::PropertyList*, table->data);
  nsIPresContext *presContext = NS_STATIC_CAST(nsIPresContext*, arg);
  PropertyListMapEntry* entry = NS_STATIC_CAST(PropertyListMapEntry*, hdr);

  propList->mDtorFunc(presContext, entry->key, propList->mName, entry->value);
  return PL_DHASH_NEXT;
}

void
FrameManager::PropertyList::Destroy(nsIPresContext* aPresContext)
{
  // Enumerate any remaining frame/value pairs and destroy the value object
  if (mDtorFunc)
    PL_DHashTableEnumerate(&mFrameValueMap, DestroyPropertyEnumerator,
                           aPresContext);
}

PRBool
FrameManager::PropertyList::RemovePropertyForFrame(nsIPresContext* aPresContext,
                                                   nsIFrame*       aFrame)
{
  PropertyListMapEntry *entry = NS_STATIC_CAST(PropertyListMapEntry*,
      PL_DHashTableOperate(&mFrameValueMap, aFrame, PL_DHASH_LOOKUP));
  if (!PL_DHASH_ENTRY_IS_BUSY(entry))
    return PR_FALSE;

  if (mDtorFunc)
    mDtorFunc(aPresContext, aFrame, mName, entry->value);

  PL_DHashTableRawRemove(&mFrameValueMap, entry);

  return PR_TRUE;
}
