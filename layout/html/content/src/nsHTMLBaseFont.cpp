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
#include "nsIDOMHTMLBaseFontElement.h"
#include "nsIScriptObjectOwner.h"
#include "nsIDOMEventReceiver.h"
#include "nsIHTMLContent.h"
#include "nsHTMLGenericContent.h"
#include "nsHTMLAtoms.h"
#include "nsHTMLIIDs.h"
#include "nsIStyleContext.h"
#include "nsStyleConsts.h"
#include "nsIPresContext.h"

static NS_DEFINE_IID(kIDOMHTMLBaseFontElementIID, NS_IDOMHTMLBASEFONTELEMENT_IID);

class nsHTMLBaseFont : public nsIDOMHTMLBaseFontElement,
                       public nsIScriptObjectOwner,
                       public nsIDOMEventReceiver,
                       public nsIHTMLContent
{
public:
  nsHTMLBaseFont(nsIAtom* aTag);
  ~nsHTMLBaseFont();

  // nsISupports
  NS_DECL_ISUPPORTS

  // nsIDOMNode
  NS_IMPL_IDOMNODE_USING_GENERIC(mInner)

  // nsIDOMElement
  NS_IMPL_IDOMELEMENT_USING_GENERIC(mInner)

  // nsIDOMHTMLElement
  NS_IMPL_IDOMHTMLELEMENT_USING_GENERIC(mInner)

  // nsIDOMHTMLBaseElement
  NS_IMETHOD GetColor(nsString& aColor);
  NS_IMETHOD SetColor(const nsString& aColor);
  NS_IMETHOD GetFace(nsString& aFace);
  NS_IMETHOD SetFace(const nsString& aFace);
  NS_IMETHOD GetSize(nsString& aSize);
  NS_IMETHOD SetSize(const nsString& aSize);

  // nsIScriptObjectOwner
  NS_IMPL_ISCRIPTOBJECTOWNER_USING_GENERIC(mInner)

  // nsIDOMEventReceiver
  NS_IMPL_IDOMEVENTRECEIVER_USING_GENERIC(mInner)

  // nsIContent
  NS_IMPL_ICONTENT_USING_GENERIC(mInner)

  // nsIHTMLContent
  NS_IMPL_IHTMLCONTENT_USING_GENERIC(mInner)

protected:
  nsHTMLGenericLeafContent mInner;
};

nsresult
NS_NewHTMLBaseFont(nsIHTMLContent** aInstancePtrResult, nsIAtom* aTag)
{
  NS_PRECONDITION(nsnull != aInstancePtrResult, "null ptr");
  if (nsnull == aInstancePtrResult) {
    return NS_ERROR_NULL_POINTER;
  }
  nsIHTMLContent* it = new nsHTMLBaseFont(aTag);
  if (nsnull == it) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  return it->QueryInterface(kIHTMLContentIID, (void**) aInstancePtrResult);
}

nsHTMLBaseFont::nsHTMLBaseFont(nsIAtom* aTag)
{
  NS_INIT_REFCNT();
  mInner.Init(this, aTag);
}

nsHTMLBaseFont::~nsHTMLBaseFont()
{
}

NS_IMPL_ADDREF(nsHTMLBaseFont)

NS_IMPL_RELEASE(nsHTMLBaseFont)

nsresult
nsHTMLBaseFont::QueryInterface(REFNSIID aIID, void** aInstancePtr)
{
  NS_IMPL_HTML_CONTENT_QUERY_INTERFACE(aIID, aInstancePtr, this)
  if (aIID.Equals(kIDOMHTMLBaseFontElementIID)) {
    nsIDOMHTMLBaseFontElement* tmp = this;
    *aInstancePtr = (void*) tmp;
    AddRef();
    return NS_OK;
  }
  return NS_NOINTERFACE;
}

nsresult
nsHTMLBaseFont::CloneNode(nsIDOMNode** aReturn)
{
  nsHTMLBaseFont* it = new nsHTMLBaseFont(mInner.mTag);
  if (nsnull == it) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  mInner.CopyInnerTo(this, &it->mInner);
  return it->QueryInterface(kIDOMNodeIID, (void**) aReturn);
}

NS_IMETHODIMP
nsHTMLBaseFont::GetColor(nsString& aValue)
{
  mInner.GetAttribute(nsHTMLAtoms::color, aValue);
  return NS_OK;
}

NS_IMETHODIMP
nsHTMLBaseFont::SetColor(const nsString& aValue)
{
  return mInner.SetAttr(nsHTMLAtoms::color, aValue, eSetAttrNotify_Render);
}

NS_IMETHODIMP
nsHTMLBaseFont::GetFace(nsString& aValue)
{
  mInner.GetAttribute(nsHTMLAtoms::face, aValue);
  return NS_OK;
}

NS_IMETHODIMP
nsHTMLBaseFont::SetFace(const nsString& aValue)
{
  return mInner.SetAttr(nsHTMLAtoms::face, aValue, eSetAttrNotify_Reflow);
}

NS_IMETHODIMP
nsHTMLBaseFont::GetSize(nsString& aValue)
{
  mInner.GetAttribute(nsHTMLAtoms::size, aValue);
  return NS_OK;
}

NS_IMETHODIMP
nsHTMLBaseFont::SetSize(const nsString& aValue)
{
  return mInner.SetAttr(nsHTMLAtoms::size, aValue, eSetAttrNotify_Reflow);
}

NS_IMETHODIMP
nsHTMLBaseFont::StringToAttribute(nsIAtom* aAttribute,
                                  const nsString& aValue,
                                  nsHTMLValue& aResult)
{
  return NS_CONTENT_ATTR_NOT_THERE;
}

NS_IMETHODIMP
nsHTMLBaseFont::AttributeToString(nsIAtom* aAttribute,
                                  nsHTMLValue& aValue,
                                  nsString& aResult) const
{
  return mInner.AttributeToString(aAttribute, aValue, aResult);
}

NS_IMETHODIMP
nsHTMLBaseFont::MapAttributesInto(nsIStyleContext* aContext,
                                  nsIPresContext* aPresContext)
{
  return NS_OK;
}

NS_IMETHODIMP
nsHTMLBaseFont::HandleDOMEvent(nsIPresContext& aPresContext,
                               nsEvent* aEvent,
                               nsIDOMEvent** aDOMEvent,
                               PRUint32 aFlags,
                               nsEventStatus& aEventStatus)
{
  return mInner.HandleDOMEvent(aPresContext, aEvent, aDOMEvent,
                               aFlags, aEventStatus);
}
