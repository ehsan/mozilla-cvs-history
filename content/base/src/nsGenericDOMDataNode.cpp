/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

/*
 * Base class for DOM Core's Comment, DocumentType, Text,
 * CDATASection, and ProcessingInstruction nodes.
 */

#include "nsGenericDOMDataNode.h"
#include "nsGenericElement.h"
#include "nsIDocument.h"
#include "nsIEventListenerManager.h"
#include "nsIDOMRange.h"
#include "nsIDOMDocument.h"
#include "nsRange.h"
#include "nsISelection.h"
#include "nsISelectionPrivate.h"
#include "nsReadableUtils.h"
#include "nsMutationEvent.h"
#include "nsINameSpaceManager.h"
#include "nsIDOM3Node.h"
#include "nsIURI.h"
#include "nsIPrivateDOMEvent.h"
#include "nsIDOMEvent.h"
#include "nsIDOMText.h"
#include "nsCOMPtr.h"
#include "nsDOMString.h"
#include "nsLayoutAtoms.h"
#include "nsIDOMUserDataHandler.h"
#include "nsChangeHint.h"
#include "nsEventDispatcher.h"
#include "nsCOMArray.h"

#include "pldhash.h"
#include "prprf.h"

nsGenericDOMDataNode::nsGenericDOMDataNode(nsINodeInfo *aNodeInfo)
  : nsITextContent(aNodeInfo)
{
}

nsGenericDOMDataNode::~nsGenericDOMDataNode()
{
  if (CouldHaveProperties()) {
    nsIDocument *document = GetOwnerDoc();
    if (document) {
      document->CallUserDataHandler(nsIDOMUserDataHandler::NODE_DELETED,
                                    this, nsnull, nsnull);
      document->PropertyTable()->DeleteAllPropertiesFor(this);
    }
  }

  if (CouldHaveEventListenerManager()) {
    nsContentUtils::RemoveListenerManager(this);
  }

  if (CouldHaveRangeList()) {
    nsContentUtils::RemoveRangeList(this);
  }
}


NS_IMPL_ADDREF(nsGenericDOMDataNode)
NS_IMPL_RELEASE(nsGenericDOMDataNode)

NS_INTERFACE_MAP_BEGIN(nsGenericDOMDataNode)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIContent)
  NS_INTERFACE_MAP_ENTRY(nsIDOMGCParticipant)
  NS_INTERFACE_MAP_ENTRY_TEAROFF(nsIDOMEventReceiver,
                                 nsDOMEventRTTearoff::Create(this))
  NS_INTERFACE_MAP_ENTRY_TEAROFF(nsIDOMEventTarget,
                                 nsDOMEventRTTearoff::Create(this))
  NS_INTERFACE_MAP_ENTRY_TEAROFF(nsIDOM3EventTarget,
                                 nsDOMEventRTTearoff::Create(this))
  NS_INTERFACE_MAP_ENTRY_TEAROFF(nsIDOMNSEventTarget,
                                 nsDOMEventRTTearoff::Create(this))
  NS_INTERFACE_MAP_ENTRY(nsIContent)
  // No nsITextContent since all subclasses might not want that.
  NS_INTERFACE_MAP_ENTRY_TEAROFF(nsIDOM3Node, new nsNode3Tearoff(this))
  NS_INTERFACE_MAP_ENTRY(nsINode)
NS_INTERFACE_MAP_END


nsresult
nsGenericDOMDataNode::GetNodeValue(nsAString& aNodeValue)
{
  return GetData(aNodeValue);
}

nsresult
nsGenericDOMDataNode::SetNodeValue(const nsAString& aNodeValue)
{
  return SetData(aNodeValue);
}

nsresult
nsGenericDOMDataNode::GetParentNode(nsIDOMNode** aParentNode)
{
  nsresult rv = NS_OK;

  nsIContent *parent = GetParent();
  if (parent) {
    rv = CallQueryInterface(parent, aParentNode);
  }
  else if (IsInDoc()) {
    rv = CallQueryInterface(GetCurrentDoc(), aParentNode);
  }
  else {
    *aParentNode = nsnull;
  }

  NS_ASSERTION(NS_SUCCEEDED(rv), "Must be a DOM Node");

  return rv;
}

nsresult
nsGenericDOMDataNode::GetPreviousSibling(nsIDOMNode** aPrevSibling)
{
  nsresult rv = NS_OK;

  nsIContent *sibling = nsnull;
  nsIContent *parent = GetParent();
  if (parent) {
    PRInt32 pos = parent->IndexOf(this);
    if (pos > 0) {
      sibling = parent->GetChildAt(pos - 1);
    }
  }
  else {
    nsIDocument *doc = GetCurrentDoc();
    if (doc) {
      PRInt32 pos = doc->IndexOf(this);
      if (pos > 0) {
        sibling = doc->GetChildAt(pos - 1);
      }
    }
  }

  if (sibling) {
    rv = CallQueryInterface(sibling, aPrevSibling);
    NS_ASSERTION(NS_SUCCEEDED(rv), "Must be a DOM Node");
  } else {
    *aPrevSibling = nsnull;
  }

  return rv;
}

nsresult
nsGenericDOMDataNode::GetNextSibling(nsIDOMNode** aNextSibling)
{
  nsresult rv = NS_OK;

  nsIContent *sibling = nsnull;
  nsIContent *parent = GetParent();
  if (parent) {
    PRInt32 pos = parent->IndexOf(this);
    if (pos > -1) {
      sibling = parent->GetChildAt(pos + 1);
    }
  }
  else {
    nsIDocument *doc = GetCurrentDoc();
    if (doc) {
      PRInt32 pos = doc->IndexOf(this);
      if (pos > -1) {
        sibling = doc->GetChildAt(pos + 1);
      }
    }
  }

  if (sibling) {
    rv = CallQueryInterface(sibling, aNextSibling);
    NS_ASSERTION(NS_SUCCEEDED(rv), "Must be a DOM Node");
  } else {
    *aNextSibling = nsnull;
  }

  return rv;
}

nsresult
nsGenericDOMDataNode::GetChildNodes(nsIDOMNodeList** aChildNodes)
{
  // XXX Since we believe this won't be done very often, we won't
  // burn another slot in the data node and just create a new
  // (empty) childNodes list every time we're asked.
  nsChildContentList* list = new nsChildContentList(nsnull);
  if (!list) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  return CallQueryInterface(list, aChildNodes);
}

nsresult
nsGenericDOMDataNode::GetOwnerDocument(nsIDOMDocument** aOwnerDocument)
{
  nsIDocument *document = GetOwnerDoc();
  if (document) {
    return CallQueryInterface(document, aOwnerDocument);
  }

  *aOwnerDocument = nsnull;

  return NS_OK;
}

nsresult
nsGenericDOMDataNode::GetNamespaceURI(nsAString& aNamespaceURI)
{
  SetDOMStringToNull(aNamespaceURI);

  return NS_OK;
}

nsresult
nsGenericDOMDataNode::GetPrefix(nsAString& aPrefix)
{
  SetDOMStringToNull(aPrefix);

  return NS_OK;
}

nsresult
nsGenericDOMDataNode::SetPrefix(const nsAString& aPrefix)
{
  return NS_ERROR_DOM_NAMESPACE_ERR;
}

nsresult
nsGenericDOMDataNode::GetLocalName(nsAString& aLocalName)
{
  SetDOMStringToNull(aLocalName);

  return NS_OK;
}

nsresult
nsGenericDOMDataNode::Normalize()
{
  return NS_OK;
}

nsresult
nsGenericDOMDataNode::IsSupported(const nsAString& aFeature,
                                  const nsAString& aVersion,
                                  PRBool* aReturn)
{
  return nsGenericElement::InternalIsSupported(NS_STATIC_CAST(nsIContent*, this),
                                               aFeature, aVersion, aReturn);
}

nsresult
nsGenericDOMDataNode::GetBaseURI(nsAString& aURI)
{
  nsCOMPtr<nsIURI> baseURI = GetBaseURI();
  nsCAutoString spec;

  if (baseURI) {
    baseURI->GetSpec(spec);
  }

  CopyUTF8toUTF16(spec, aURI);

  return NS_OK;
}

nsresult
nsGenericDOMDataNode::CloneNode(PRBool aDeep, nsIDOMNode *aSource,
                                nsIDOMNode **aResult) const
{
  *aResult = nsnull;

  nsCOMPtr<nsIContent> newContent;
  nsresult rv = CloneContent(mNodeInfo->NodeInfoManager(), aDeep,
                             getter_AddRefs(newContent));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = CallQueryInterface(newContent, aResult);

  nsIDocument *ownerDoc = GetOwnerDoc();
  if (NS_SUCCEEDED(rv) && ownerDoc && CouldHaveProperties()) {
    ownerDoc->CallUserDataHandler(nsIDOMUserDataHandler::NODE_CLONED,
                                  NS_STATIC_CAST(const nsIContent*, this),
                                  aSource, *aResult);
  }

  return rv;
}

nsresult
nsGenericDOMDataNode::LookupPrefix(const nsAString& aNamespaceURI,
                                   nsAString& aPrefix)
{
  aPrefix.Truncate();

  nsIContent *parent_weak = GetParent();

  // DOM Data Node passes the query on to its parent
  nsCOMPtr<nsIDOM3Node> node(do_QueryInterface(parent_weak));
  if (node) {
    return node->LookupPrefix(aNamespaceURI, aPrefix);
  }

  return NS_OK;
}

nsresult
nsGenericDOMDataNode::LookupNamespaceURI(const nsAString& aNamespacePrefix,
                                         nsAString& aNamespaceURI)
{
  aNamespaceURI.Truncate();

  nsIContent *parent_weak = GetParent();

  // DOM Data Node passes the query on to its parent
  nsCOMPtr<nsIDOM3Node> node(do_QueryInterface(parent_weak));

  if (node) {
    return node->LookupNamespaceURI(aNamespacePrefix, aNamespaceURI);
  }

  return NS_OK;
}

//----------------------------------------------------------------------

// Implementation of nsIDOMCharacterData

nsresult
nsGenericDOMDataNode::GetData(nsAString& aData) const
{
  if (mText.Is2b()) {
    aData.Assign(mText.Get2b(), mText.GetLength());
  } else {
    // Must use Substring() since nsDependentCString() requires null
    // terminated strings.

    const char *data = mText.Get1b();

    if (data) {
      CopyASCIItoUTF16(Substring(data, data + mText.GetLength()), aData);
    } else {
      aData.Truncate();
    }
  }

  return NS_OK;
}

nsresult
nsGenericDOMDataNode::SetData(const nsAString& aData)
{
  // inform any enclosed ranges of change
  // we can lie and say we are deleting all the text, since in a total
  // text replacement we should just collapse all the ranges.

  const nsVoidArray *rangeList = GetRangeList();
  if (rangeList) {
    nsRange::TextOwnerChanged(this, rangeList, 0, mText.GetLength(), 0);
  }

  nsCOMPtr<nsITextContent> textContent = do_QueryInterface(this);

  SetText(aData, PR_TRUE);

  return NS_OK;
}

nsresult
nsGenericDOMDataNode::GetLength(PRUint32* aLength)
{
  *aLength = mText.GetLength();
  return NS_OK;
}

nsresult
nsGenericDOMDataNode::SubstringData(PRUint32 aStart, PRUint32 aCount,
                                    nsAString& aReturn)
{
  aReturn.Truncate();

  // XXX add <0 checks if types change
  PRUint32 textLength = PRUint32( mText.GetLength() );
  if (aStart > textLength) {
    return NS_ERROR_DOM_INDEX_SIZE_ERR;
  }

  PRUint32 amount = aCount;
  if (amount > textLength - aStart) {
    amount = textLength - aStart;
  }

  if (mText.Is2b()) {
    aReturn.Assign(mText.Get2b() + aStart, amount);
  } else {
    // Must use Substring() since nsDependentCString() requires null
    // terminated strings.

    const char *data = mText.Get1b() + aStart;
    CopyASCIItoUTF16(Substring(data, data + amount), aReturn);
  }

  return NS_OK;
}

//----------------------------------------------------------------------

nsresult
nsGenericDOMDataNode::AppendData(const nsAString& aData)
{
  // Apparently this is called often enough that we don't want to just simply
  // call SetText like ReplaceData does. See bug 77585 and comment in
  // ReplaceData.
  nsIDocument *document = GetCurrentDoc();
  
  // FIXME, but 330872: We can't call BeginUpdate here because it confuses the
  // poor little nsHTMLContentSink.
  // mozAutoDocUpdate updateBatch(document, UPDATE_CONTENT_MODEL, PR_TRUE);

  PRBool haveMutationListeners =
    document && nsGenericElement::HasMutationListeners(this, NS_EVENT_BITS_MUTATION_CHARACTERDATAMODIFIED);

  nsCOMPtr<nsIAtom> oldValue;
  if (haveMutationListeners) {
    oldValue = GetCurrentValueAtom();
  }
    
  mText.Append(aData);

  SetBidiStatus();

  if (haveMutationListeners) {
    nsMutationEvent mutation(PR_TRUE, NS_MUTATION_CHARACTERDATAMODIFIED);

    mutation.mPrevAttrValue = oldValue;
    mutation.mNewAttrValue = GetCurrentValueAtom();

    nsEventDispatcher::Dispatch(this, nsnull, &mutation);
  }

  // Trigger a reflow
  if (document) {
    document->CharacterDataChanged(this, PR_TRUE);
  }

  return NS_OK;
}

nsresult
nsGenericDOMDataNode::InsertData(PRUint32 aOffset,
                                 const nsAString& aData)
{
  return ReplaceData(aOffset, 0, aData);
}

nsresult
nsGenericDOMDataNode::DeleteData(PRUint32 aOffset, PRUint32 aCount)
{
  return ReplaceData(aOffset, aCount, EmptyString());
}

nsresult
nsGenericDOMDataNode::ReplaceData(PRUint32 aOffset, PRUint32 aCount,
                                  const nsAString& aData)
{
  // sanitize arguments
  PRUint32 textLength = mText.GetLength();
  if (aOffset > textLength) {
    return NS_ERROR_DOM_INDEX_SIZE_ERR;
  }

  // Fast path (hit by editor when typing at the end of the paragraph, for
  // example): aOffset == textLength (so just doing an append; note that in
  // this case any value of aCount would just get converted to 0 by the very
  // next if block).  Call AppendData so that we pass PR_TRUE for our aAppend
  // arg to CharacterDataChanged.
  if (aOffset == textLength) {
    return AppendData(aData);
  }

  // Allocate new buffer
  PRUint32 endOffset = aOffset + aCount;
  if (endOffset > textLength) {
    aCount = textLength - aOffset;
    endOffset = textLength;
  }
  PRInt32 dataLength = aData.Length();
  PRInt32 newLength = textLength - aCount + dataLength;
  PRUnichar* to = new PRUnichar[newLength + 1];
  if (!to) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  // inform any enclosed ranges of change
  const nsVoidArray *rangeList = GetRangeList();
  if (rangeList) {
    nsRange::TextOwnerChanged(this, rangeList, aOffset, endOffset, dataLength);
  }

  // Copy over appropriate data
  if (0 != aOffset) {
    mText.CopyTo(to, 0, aOffset);
  }
  if (0 != dataLength) {
    CopyUnicodeTo(aData, 0, to+aOffset, dataLength);
  }
  if (endOffset != textLength) {
    mText.CopyTo(to + aOffset + dataLength, endOffset, textLength - endOffset);
  }

  // Null terminate the new buffer...
  to[newLength] = (PRUnichar)0;

  SetText(to, newLength, PR_TRUE);
  delete [] to;

  return NS_OK;
}

//----------------------------------------------------------------------

nsresult
nsGenericDOMDataNode::GetListenerManager(PRBool aCreateIfNotFound,
                                         nsIEventListenerManager** aResult)
{
  // No need to call nsContentUtils::GetListenerManager if we're sure that
  // there is no event listener manager.
  if (!aCreateIfNotFound && !CouldHaveEventListenerManager()) {
    *aResult = nsnull;
    return NS_OK;
  }

  PRBool created;
  nsresult rv = nsContentUtils::GetListenerManager(this, aCreateIfNotFound,
                                                   aResult, &created);
  if (NS_SUCCEEDED(rv) && created) {
    SetHasEventListenerManager();
  }
  return rv;
}

//----------------------------------------------------------------------

// Implementation of nsIContent

#ifdef DEBUG
void
nsGenericDOMDataNode::ToCString(nsAString& aBuf, PRInt32 aOffset,
                                PRInt32 aLen) const
{
  if (mText.Is2b()) {
    const PRUnichar* cp = mText.Get2b() + aOffset;
    const PRUnichar* end = cp + aLen;

    while (cp < end) {
      PRUnichar ch = *cp++;
      if (ch == '\r') {
        aBuf.AppendLiteral("\\r");
      } else if (ch == '\n') {
        aBuf.AppendLiteral("\\n");
      } else if (ch == '\t') {
        aBuf.AppendLiteral("\\t");
      } else if ((ch < ' ') || (ch >= 127)) {
        char buf[10];
        PR_snprintf(buf, sizeof(buf), "\\u%04x", ch);
        AppendASCIItoUTF16(buf, aBuf);
      } else {
        aBuf.Append(ch);
      }
    }
  } else {
    unsigned char* cp = (unsigned char*)mText.Get1b() + aOffset;
    const unsigned char* end = cp + aLen;

    while (cp < end) {
      PRUnichar ch = *cp++;
      if (ch == '\r') {
        aBuf.AppendLiteral("\\r");
      } else if (ch == '\n') {
        aBuf.AppendLiteral("\\n");
      } else if (ch == '\t') {
        aBuf.AppendLiteral("\\t");
      } else if ((ch < ' ') || (ch >= 127)) {
        char buf[10];
        PR_snprintf(buf, sizeof(buf), "\\u%04x", ch);
        AppendASCIItoUTF16(buf, aBuf);
      } else {
        aBuf.Append(ch);
      }
    }
  }
}
#endif

/**
 * See comment for nsGenericElement::GetSCCIndex
 */
nsIDOMGCParticipant*
nsGenericDOMDataNode::GetSCCIndex()
{
  // This is an optimized way of walking nsIDOMNode::GetParentNode to
  // the top of the tree.
  nsIDOMGCParticipant *result = GetCurrentDoc();
  if (!result) {
    nsIContent *top = this;
    while (top->GetParent())
      top = top->GetParent();
    result = top;
  }

  return result;
}

void
nsGenericDOMDataNode::AppendReachableList(nsCOMArray<nsIDOMGCParticipant>& aArray)
{
  NS_ASSERTION(GetCurrentDoc() == nsnull,
               "shouldn't be an SCC index if we're in a doc");

  // This node is the root of a subtree that's been removed from the
  // document (since AppendReachableList is only called on SCC index
  // nodes).  The document is reachable from it (through
  // .ownerDocument), but it's not reachable from the document.
  aArray.AppendObject(GetOwnerDoc());
}

nsresult
nsGenericDOMDataNode::BindToTree(nsIDocument* aDocument, nsIContent* aParent,
                                 nsIContent* aBindingParent,
                                 PRBool aCompileEventHandlers)
{
  NS_PRECONDITION(aParent || aDocument, "Must have document if no parent!");
  // XXXbz XUL elements are confused about their current doc when they're
  // cloned, so we don't assert if aParent is a XUL element and aDocument is
  // null, even if aParent->GetCurrentDoc() is non-null
  //  NS_PRECONDITION(!aParent || aDocument == aParent->GetCurrentDoc(),
  //                  "aDocument must be current doc of aParent");
  NS_PRECONDITION(!aParent ||
                  (aParent->IsContentOfType(eXUL) && aDocument == nsnull) ||
                  aDocument == aParent->GetCurrentDoc(),
                  "aDocument must be current doc of aParent");
  NS_PRECONDITION(!GetCurrentDoc(), "Already have a document.  Unbind first!");
  // Note that as we recurse into the kids, they'll have a non-null parent.  So
  // only assert if our parent is _changing_ while we have a parent.
  NS_PRECONDITION(!GetParent() || aParent == GetParent(),
                  "Already have a parent.  Unbind first!");
  // XXXbz GetBindingParent() is broken for us, so can't assert
  // anything about it yet.
  //  NS_PRECONDITION(!GetBindingParent() ||
  //                  aBindingParent == GetBindingParent() ||
  //                  (aParent &&
  //                   aParent->GetBindingParent() == GetBindingParent()),
  //                  "Already have a binding parent.  Unbind first!");

  // XXXbz we don't keep track of the binding parent yet.  We should.
  
  nsresult rv;

  // Set parent
  PtrBits new_bits = NS_REINTERPRET_CAST(PtrBits, aParent);
  new_bits |= mParentPtrBits & nsIContent::kParentBitMask;
  mParentPtrBits = new_bits;

  // Set document
  if (aDocument) {
    mParentPtrBits |= PARENT_BIT_INDOCUMENT;
    if (mText.IsBidi()) {
      aDocument->SetBidiEnabled(PR_TRUE);
    }

    nsIDocument *ownerDocument = GetOwnerDoc();
    if (aDocument != ownerDocument) {
      if (ownerDocument && CouldHaveProperties()) {
        // Copy UserData to the new document.
        ownerDocument->CopyUserData(this, aDocument);

        // Remove all properties.
        ownerDocument->PropertyTable()->DeleteAllPropertiesFor(this);
      }

      // get a new nodeinfo
      nsNodeInfoManager *nodeInfoManager = aDocument->NodeInfoManager();
      nsCOMPtr<nsINodeInfo> newNodeInfo;
      // optimize common cases
      nsIAtom* name = mNodeInfo->NameAtom();
      if (name == nsLayoutAtoms::textTagName) {
        newNodeInfo = nodeInfoManager->GetTextNodeInfo();
        NS_ENSURE_TRUE(newNodeInfo, NS_ERROR_OUT_OF_MEMORY);
      }
      else if (name == nsLayoutAtoms::commentTagName) {
        newNodeInfo = nodeInfoManager->GetCommentNodeInfo();
        NS_ENSURE_TRUE(newNodeInfo, NS_ERROR_OUT_OF_MEMORY);
      }
      else {
        rv = nodeInfoManager->GetNodeInfo(name,
                                          mNodeInfo->GetPrefixAtom(),
                                          mNodeInfo->NamespaceID(),
                                          getter_AddRefs(newNodeInfo));
        NS_ENSURE_SUCCESS(rv, rv);
      }
      mNodeInfo.swap(newNodeInfo);
    }
  }

  NS_POSTCONDITION(aDocument == GetCurrentDoc(), "Bound to wrong document");
  NS_POSTCONDITION(aParent == GetParent(), "Bound to wrong parent");
  // XXXbz GetBindingParent() is broken for us, so can't assert
  // anything about it yet.
  //  NS_POSTCONDITION(aBindingParent = GetBindingParent(),
  //                   "Bound to wrong binding parent");

  return NS_OK;
}

void
nsGenericDOMDataNode::UnbindFromTree(PRBool aDeep, PRBool aNullParent)
{
  mParentPtrBits &= ~PARENT_BIT_INDOCUMENT;
  if (aNullParent) {
    mParentPtrBits &= nsIContent::kParentBitMask;
  }
}

PRBool
nsGenericDOMDataNode::IsNativeAnonymous() const
{
  nsIContent* parent = GetParent();
  return parent && parent->IsNativeAnonymous();
}

void
nsGenericDOMDataNode::SetNativeAnonymous(PRBool aAnonymous)
{
  // XXX Need to fix this to do something - bug 165110
}

nsIAtom *
nsGenericDOMDataNode::GetIDAttributeName() const
{
  return nsnull;
}

already_AddRefed<nsINodeInfo>
nsGenericDOMDataNode::GetExistingAttrNameFromQName(const nsAString& aStr) const
{
  return nsnull;
}

nsresult
nsGenericDOMDataNode::SetAttr(PRInt32 aNameSpaceID, nsIAtom* aAttr,
                              nsIAtom* aPrefix, const nsAString& aValue,
                              PRBool aNotify)
{
  return NS_OK;
}

nsresult
nsGenericDOMDataNode::UnsetAttr(PRInt32 aNameSpaceID, nsIAtom* aAttr,
                                PRBool aNotify)
{
  return NS_OK;
}

PRBool
nsGenericDOMDataNode::GetAttr(PRInt32 aNameSpaceID, nsIAtom *aAttr,
                              nsAString& aResult) const
{
  aResult.Truncate();

  return PR_FALSE;
}

PRBool
nsGenericDOMDataNode::HasAttr(PRInt32 aNameSpaceID, nsIAtom *aAttribute) const
{
  return PR_FALSE;
}

const nsAttrName*
nsGenericDOMDataNode::GetAttrNameAt(PRUint32 aIndex) const
{
  return nsnull;
}

PRUint32
nsGenericDOMDataNode::GetAttrCount() const
{
  return 0;
}

nsresult
nsGenericDOMDataNode::PreHandleEvent(nsEventChainPreVisitor& aVisitor)
{
  //FIXME! Handle event retargeting, bug 329122.
  aVisitor.mCanHandle = PR_TRUE;
  aVisitor.mParentTarget = GetParent();
  if (!aVisitor.mParentTarget) {
    aVisitor.mParentTarget = GetCurrentDoc();
  }
  return NS_OK;
}

nsresult
nsGenericDOMDataNode::PostHandleEvent(nsEventChainPostVisitor& /*aVisitor*/)
{
  return NS_OK;
}

nsresult
nsGenericDOMDataNode::DispatchDOMEvent(nsEvent* aEvent,
                                       nsIDOMEvent* aDOMEvent,
                                       nsPresContext* aPresContext,
                                       nsEventStatus* aEventStatus)
{
  return nsEventDispatcher::DispatchDOMEvent(NS_STATIC_CAST(nsINode*, this),
                                             aEvent, aDOMEvent,
                                             aPresContext, aEventStatus);
}

PRUint32
nsGenericDOMDataNode::GetChildCount() const
{
  return 0;
}

nsIContent *
nsGenericDOMDataNode::GetChildAt(PRUint32 aIndex) const
{
  return nsnull;
}

PRInt32
nsGenericDOMDataNode::IndexOf(nsIContent* aPossibleChild) const
{
  return -1;
}

nsresult
nsGenericDOMDataNode::InsertChildAt(nsIContent* aKid, PRUint32 aIndex,
                                    PRBool aNotify)
{
  return NS_OK;
}

nsresult
nsGenericDOMDataNode::AppendChildTo(nsIContent* aKid, PRBool aNotify)
{
  return NS_OK;
}

nsresult
nsGenericDOMDataNode::RemoveChildAt(PRUint32 aIndex, PRBool aNotify)
{
  return NS_OK;
}

nsresult
nsGenericDOMDataNode::SetProperty(nsIAtom *aPropertyName,
                                  void *aValue,
                                  NSPropertyDtorFunc aDtor)
{
  nsresult rv = nsITextContent::SetProperty(aPropertyName, aValue, aDtor);

  if (NS_SUCCEEDED(rv))
    SetIsInAHash();

  return rv;
}

// virtual
PRBool
nsGenericDOMDataNode::MayHaveFrame() const
{
  nsIContent* parent = GetParent();
  return parent && parent->MayHaveFrame();
}

nsresult
nsGenericDOMDataNode::RangeAdd(nsIDOMRange* aRange)
{
  PRBool created;
  nsresult rv = nsContentUtils::AddToRangeList(this, aRange, &created);
  if (NS_SUCCEEDED(rv) && created) {
    SetHasRangeList();
  }
  return rv;
}

void
nsGenericDOMDataNode::RangeRemove(nsIDOMRange* aRange)
{
  if (CouldHaveRangeList()) {
    nsContentUtils::RemoveFromRangeList(this, aRange);
  }
}

const nsVoidArray *
nsGenericDOMDataNode::GetRangeList() const
{
  return CouldHaveRangeList() ? nsContentUtils::LookupRangeList(this) : nsnull;
}

nsIContent *
nsGenericDOMDataNode::GetBindingParent() const
{
  nsIContent* parent = GetParent();
  return parent ? parent->GetBindingParent() : nsnull;
}

PRBool
nsGenericDOMDataNode::IsContentOfType(PRUint32 aFlags) const
{
  return PR_FALSE;
}

#ifdef DEBUG
void
nsGenericDOMDataNode::List(FILE* out, PRInt32 aIndent) const
{
}

void
nsGenericDOMDataNode::DumpContent(FILE* out, PRInt32 aIndent,
                                  PRBool aDumpAll) const 
{
}
#endif

already_AddRefed<nsIURI>
nsGenericDOMDataNode::GetBaseURI() const
{
  // DOM Data Node inherits the base from its parent element/document
  nsIContent *parent = GetParent();
  if (parent) {
    return parent->GetBaseURI();
  }

  nsIURI *uri;
  nsIDocument *doc = GetOwnerDoc();
  if (doc) {
    NS_IF_ADDREF(uri = doc->GetBaseURI());
  }
  else {
    uri = nsnull;
  }

  return uri;
}

//----------------------------------------------------------------------

// Implementation of the nsIDOMText interface

nsresult
nsGenericDOMDataNode::SplitText(PRUint32 aOffset, nsIDOMText** aReturn)
{
  nsresult rv = NS_OK;
  nsAutoString cutText;
  PRUint32 length = TextLength();

  if (aOffset > length) {
    return NS_ERROR_DOM_INDEX_SIZE_ERR;
  }

  rv = SubstringData(aOffset, length - aOffset, cutText);
  if (NS_FAILED(rv)) {
    return rv;
  }

  rv = DeleteData(aOffset, length - aOffset);
  if (NS_FAILED(rv)) {
    return rv;
  }

  /*
   * Use Clone for creating the new node so that the new node is of same class
   * as this node!
   */

  nsCOMPtr<nsITextContent> newContent = Clone(mNodeInfo, PR_FALSE);
  if (!newContent) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  newContent->SetText(cutText, PR_TRUE);

  nsIContent* parent = GetParent();

  if (parent) {
    PRInt32 index = parent->IndexOf(this);

    nsCOMPtr<nsIContent> content(do_QueryInterface(newContent));

    parent->InsertChildAt(content, index+1, PR_TRUE);
  }

  // No need to handle the case of document being the parent since text
  // isn't allowed as direct child of documents

  return CallQueryInterface(newContent, aReturn);
}

//----------------------------------------------------------------------

// Implementation of the nsITextContent interface

const nsTextFragment *
nsGenericDOMDataNode::Text()
{
  return &mText;
}

PRUint32
nsGenericDOMDataNode::TextLength()
{
  return mText.GetLength();
}

void
nsGenericDOMDataNode::SetText(const PRUnichar* aBuffer,
                              PRUint32 aLength,
                              PRBool aNotify)
{
  if (!aBuffer) {
    NS_ERROR("Null buffer passed to SetText()!");

    return;
  }

  nsIDocument *document = GetCurrentDoc();
  mozAutoDocUpdate updateBatch(document, UPDATE_CONTENT_MODEL, aNotify);

  PRBool haveMutationListeners =
    document && nsGenericElement::HasMutationListeners(this, NS_EVENT_BITS_MUTATION_CHARACTERDATAMODIFIED);

  nsCOMPtr<nsIAtom> oldValue;
  if (haveMutationListeners) {
    oldValue = GetCurrentValueAtom();
  }
    
  mText.SetTo(aBuffer, aLength);

  SetBidiStatus();

  if (haveMutationListeners) {
    nsMutationEvent mutation(PR_TRUE, NS_MUTATION_CHARACTERDATAMODIFIED);

    mutation.mPrevAttrValue = oldValue;
    if (aLength > 0) {
      // Must use Substring() since nsDependentString() requires null
      // terminated strings.
      mutation.mNewAttrValue =
        do_GetAtom(Substring(aBuffer, aBuffer + aLength));
    }

    nsEventDispatcher::Dispatch(this, nsnull, &mutation);
  }

  // Trigger a reflow
  if (aNotify && document) {
    document->CharacterDataChanged(this, PR_FALSE);
  }
}

PRBool
nsGenericDOMDataNode::IsOnlyWhitespace()
{
  if (mText.Is2b()) {
    // The fragment contains non-8bit characters and such characters
    // are never considered whitespace.
    return PR_FALSE;
  }

  const char* cp = mText.Get1b();
  const char* end = cp + mText.GetLength();

  while (cp < end) {
    char ch = *cp;

    if (!XP_IS_SPACE(ch)) {
      return PR_FALSE;
    }

    ++cp;
  }

  return PR_TRUE;
}

void
nsGenericDOMDataNode::AppendTextTo(nsAString& aResult)
{
  mText.AppendTo(aResult);
}

void nsGenericDOMDataNode::SetBidiStatus()
{
  nsIDocument *document = GetCurrentDoc();
  if (document && document->GetBidiEnabled()) {
    // OK, we already know it's Bidi, so we won't test again
    return;
  }

  mText.SetBidiFlag();

  if (document && mText.IsBidi()) {
    document->SetBidiEnabled(PR_TRUE);
  }
}

already_AddRefed<nsIAtom>
nsGenericDOMDataNode::GetCurrentValueAtom()
{
  nsAutoString val;
  GetData(val);
  return NS_NewAtom(val);
}

nsresult
nsGenericDOMDataNode::CloneContent(nsNodeInfoManager *aNodeInfoManager,
                                   PRBool aDeep, nsIContent **aResult) const
{
  nsINodeInfo *nodeInfo = NodeInfo();
  nsCOMPtr<nsINodeInfo> newNodeInfo;
  if (aNodeInfoManager != nodeInfo->NodeInfoManager()) {
    nsresult rv = aNodeInfoManager->GetNodeInfo(nodeInfo->NameAtom(),
                                                nodeInfo->GetPrefixAtom(),
                                                nodeInfo->NamespaceID(),
                                                getter_AddRefs(newNodeInfo));
    NS_ENSURE_SUCCESS(rv, rv);

    nodeInfo = newNodeInfo;
  }

  *aResult = Clone(nodeInfo, PR_TRUE);
  if (!*aResult) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  NS_ADDREF(*aResult);

  return NS_OK;
}

nsIAtom*
nsGenericDOMDataNode::GetID() const
{
  return nsnull;
}

const nsAttrValue*
nsGenericDOMDataNode::GetClasses() const
{
  return nsnull;
}

NS_IMETHODIMP
nsGenericDOMDataNode::WalkContentStyleRules(nsRuleWalker* aRuleWalker)
{
  return NS_OK;
}

nsICSSStyleRule*
nsGenericDOMDataNode::GetInlineStyleRule()
{
  return nsnull;
}

NS_IMETHODIMP
nsGenericDOMDataNode::SetInlineStyleRule(nsICSSStyleRule* aStyleRule,
                                         PRBool aNotify)
{
  NS_NOTREACHED("How come we're setting inline style on a non-element?");
  return NS_ERROR_UNEXPECTED;
}

NS_IMETHODIMP_(PRBool)
nsGenericDOMDataNode::IsAttributeMapped(const nsIAtom* aAttribute) const
{
  return PR_FALSE;
}

nsChangeHint
nsGenericDOMDataNode::GetAttributeChangeHint(const nsIAtom* aAttribute,
                                             PRInt32 aModType) const
{
  NS_NOTREACHED("Shouldn't be calling this!");
  return nsChangeHint(0);
}

nsIAtom*
nsGenericDOMDataNode::GetClassAttributeName() const
{
  return nsnull;
}
