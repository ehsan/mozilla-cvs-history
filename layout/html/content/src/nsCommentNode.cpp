/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are Copyright (C) 1998
 * Netscape Communications Corporation.  All Rights Reserved.
 */
#include "nsIDOMComment.h"
#include "nsGenericDOMDataNode.h"
#include "nsIScriptObjectOwner.h"
#include "nsIDOMEventReceiver.h"
#include "nsIHTMLContent.h"
#include "nsFrame.h"

class nsCommentNode : public nsIDOMComment,
                      public nsIScriptObjectOwner,
                      public nsIDOMEventReceiver,
                      public nsIHTMLContent
{
public:
  nsCommentNode();
  ~nsCommentNode();

  // nsISupports
  NS_DECL_ISUPPORTS

  // nsIDOMNode
  NS_IMPL_IDOMNODE_USING_GENERIC_DOM_DATA(mInner)

  // nsIDOMCharacterData
  NS_IMPL_IDOMCHARACTERDATA_USING_GENERIC_DOM_DATA(mInner)

  // nsIDOMComment

  // nsIScriptObjectOwner
  NS_IMPL_ISCRIPTOBJECTOWNER_USING_GENERIC_DOM_DATA(mInner)

  // nsIDOMEventReceiver
  NS_IMPL_IDOMEVENTRECEIVER_USING_GENERIC_DOM_DATA(mInner)

  // nsIContent
  NS_IMPL_ICONTENT_USING_GENERIC_DOM_DATA(mInner)

  // nsIHTMLContent
  NS_IMPL_IHTMLCONTENT_USING_GENERIC_DOM_DATA(mInner)

protected:
  nsGenericDOMDataNode mInner;
};

nsresult
NS_NewCommentNode(nsIHTMLContent** aInstancePtrResult)
{
  NS_PRECONDITION(nsnull != aInstancePtrResult, "null ptr");
  if (nsnull == aInstancePtrResult) {
    return NS_ERROR_NULL_POINTER;
  }
  nsIHTMLContent* it = new nsCommentNode();
  if (nsnull == it) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  return it->QueryInterface(kIHTMLContentIID, (void **) aInstancePtrResult);
}

nsCommentNode::nsCommentNode()
{
  NS_INIT_REFCNT();
  mInner.Init(this);
}

nsCommentNode::~nsCommentNode()
{
}

NS_IMPL_ADDREF(nsCommentNode)

NS_IMPL_RELEASE(nsCommentNode)

NS_IMETHODIMP
nsCommentNode::QueryInterface(REFNSIID aIID, void** aInstancePtr)
{
  NS_IMPL_DOM_DATA_QUERY_INTERFACE(aIID, aInstancePtr, this)
  return NS_NOINTERFACE;
}

NS_IMETHODIMP
nsCommentNode::GetNodeType(PRUint16* aNodeType)
{
  *aNodeType = (PRUint16)nsIDOMNode::COMMENT_NODE;
  return NS_OK;
}

NS_IMETHODIMP
nsCommentNode::CloneNode(PRBool aDeep, nsIDOMNode** aReturn)
{
  nsCommentNode* it = new nsCommentNode();
  if (nsnull == it) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
//XXX  mInner.CopyInnerTo(this, &it->mInner);
  return it->QueryInterface(kIDOMNodeIID, (void**) aReturn);
}

NS_IMETHODIMP
nsCommentNode::List(FILE* out, PRInt32 aIndent) const
{
  NS_PRECONDITION(nsnull != mInner.mDocument, "bad content");

  PRInt32 index;
  for (index = aIndent; --index >= 0; ) fputs("  ", out);

  fprintf(out, " refcount=%d<", mRefCnt);

  nsAutoString tmp;
  mInner.ToCString(tmp, 0, mInner.mText.GetLength());
  fputs(tmp, out);

  fputs(">\n", out);
  return NS_OK;
}

NS_IMETHODIMP
nsCommentNode::ToHTML(FILE* out) const
{
  nsAutoString tmp;
  tmp.Append("<!--");
  mInner.mText.AppendTo(tmp);
  tmp.Append(">");
  fputs(tmp, out);
  return NS_OK;
}

NS_IMETHODIMP
nsCommentNode::ToHTMLString(nsString& aBuf) const
{
  aBuf.Truncate(0);
  aBuf.Append("<!--");
  mInner.mText.AppendTo(aBuf);
  aBuf.Append(">");
  return NS_OK;
}

NS_IMETHODIMP
nsCommentNode::HandleDOMEvent(nsIPresContext& aPresContext,
                              nsEvent* aEvent,
                              nsIDOMEvent** aDOMEvent,
                              PRUint32 aFlags,
                              nsEventStatus& aEventStatus)
{
  return mInner.HandleDOMEvent(aPresContext, aEvent, aDOMEvent,
                               aFlags, aEventStatus);
}

nsresult
NS_NewCommentFrame(nsIContent* aContent,
                   nsIFrame* aParentFrame,
                   nsIFrame*& aResult)
{
  nsIFrame* frame;
  nsFrame::NewFrame(&frame, aContent, aParentFrame);
  if (nsnull == frame) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  aResult = frame;
  return NS_OK;
}
