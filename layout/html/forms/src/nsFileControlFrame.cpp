/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

#include "nsFileControlFrame.h"
#include "nsButtonControlFrame.h"
#include "nsTextControlFrame.h"
#include "nsIContent.h"
#include "prtypes.h"
#include "nsIAtom.h"
#include "nsIPresContext.h"
#include "nsIHTMLContent.h"
#include "nsHTMLIIDs.h"
#include "nsHTMLAtoms.h"
#include "nsIFileWidget.h"
#include "nsITextWidget.h"
#include "nsWidgetsCID.h"
#include "nsRepository.h"
#include "nsIView.h"
#include "nsHTMLParts.h"
#include "nsIDOMHTMLInputElement.h"
#include "nsHTMLParts.h"
#include "nsIFormControl.h"

// XXX make this pixels
#define CONTROL_SPACING 40  

static NS_DEFINE_IID(kCFileWidgetCID, NS_FILEWIDGET_CID);
static NS_DEFINE_IID(kIFileWidgetIID, NS_IFILEWIDGET_IID);
static NS_DEFINE_IID(kITextWidgetIID, NS_ITEXTWIDGET_IID);
static NS_DEFINE_IID(kIFormControlFrameIID, NS_IFORMCONTROLFRAME_IID);

nsresult
NS_NewFileControlFrame(nsIContent* aContent,
                       nsIFrame*   aParent,
                       nsIFrame*&  aResult)
{
  aResult = new nsFileControlFrame(aContent, aParent);
  if (nsnull == aResult) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  return NS_OK;
}

nsFileControlFrame::nsFileControlFrame(nsIContent* aContent, nsIFrame* aParentFrame)
  : nsHTMLContainerFrame(aContent, aParentFrame)
{
  mTextFrame   = nsnull;
  mBrowseFrame = nsnull;
  mFormFrame   = nsnull;
}

nsFileControlFrame::~nsFileControlFrame()
{
}

nsresult
nsFileControlFrame::QueryInterface(const nsIID& aIID, void** aInstancePtr)
{
  NS_PRECONDITION(0 != aInstancePtr, "null ptr");
  if (NULL == aInstancePtr) {
    return NS_ERROR_NULL_POINTER;
  }
  if (aIID.Equals(kIFormControlFrameIID)) {
    *aInstancePtr = (void*) ((nsIFormControlFrame*) this);
    return NS_OK;
  }
  return nsHTMLContainerFrame::QueryInterface(aIID, aInstancePtr);
}

PRBool
nsFileControlFrame::IsSuccessful()
{
  nsAutoString name;
  return (NS_CONTENT_ATTR_HAS_VALUE == GetName(&name));
}

void 
nsFileControlFrame::Reset()
{
  if (mTextFrame) {
    mTextFrame->Reset();
  }
}

NS_IMETHODIMP 
nsFileControlFrame::GetType(PRInt32* aType) const
{
  *aType = NS_FORM_INPUT_FILE;
  return NS_OK;
}

// XXX this should be removed when nsView exposes it
nsIWidget*
GetWindowTemp(nsIView *aView)
{
  nsIWidget *window = nsnull;

  nsIView *ancestor = aView;
  while (nsnull != ancestor) {
    ancestor->GetWidget(window);
	  if (nsnull != window) {
	    return window;
	  }
	  ancestor->GetParent(ancestor);
  }
  return nsnull;
}


// this is in response to the MouseClick from the containing browse button
// XXX still need to get filters from accept attribute
void nsFileControlFrame::MouseClicked(nsIPresContext* aPresContext)
{
  nsIView* textView;
  mTextFrame->GetView(textView);
  if (nsnull == textView) {
    return;
  }
  nsIWidget* widget;
  mTextFrame->GetWidget(&widget);
  if (!widget) {
    return;
  }
 
  nsITextWidget* textWidget;
  nsresult result = widget->QueryInterface(kITextWidgetIID, (void**)&textWidget);
  if (NS_OK != result) {
    NS_RELEASE(widget);
    return;
  }
  
  nsIView*   parentView;
  textView->GetParent(parentView);
  nsIWidget* parentWidget = GetWindowTemp(parentView);
 
  nsIFileWidget *fileWidget;

  nsString title("FileWidget Title <here> mode = save");
  nsRepository::CreateInstance(kCFileWidgetCID, nsnull, kIFileWidgetIID, (void**)&fileWidget);
  
  nsString titles[] = {"all files"};
  nsString filters[] = {"*.*"};
  fileWidget->SetFilterList(1, titles, filters);

  fileWidget->Create(parentWidget, title, eMode_load, nsnull, nsnull);
  result = fileWidget->Show();

  if (result) {
    PRUint32 size;
    nsString fileName;
    fileWidget->GetFile(fileName);
    textWidget->SetText(fileName,size);
  }
  NS_RELEASE(fileWidget);
  NS_RELEASE(parentWidget);
  NS_RELEASE(textWidget);
  NS_RELEASE(widget);
}


void SetType(nsIHTMLContent* aElement, nsString& aValue)
{
  nsIHTMLContent* iContent = nsnull;
  nsresult result = aElement->QueryInterface(kIHTMLContentIID, (void**)&iContent);
  if ((NS_OK == result) && iContent) {
    iContent->SetAttribute("type", aValue, PR_FALSE);
    NS_RELEASE(iContent);
  }
}

NS_IMETHODIMP nsFileControlFrame::Reflow(nsIPresContext&      aPresContext, 
                                         nsReflowMetrics&     aDesiredSize,
                                         const nsReflowState& aReflowState, 
                                         nsReflowStatus&      aStatus)
{
  PRInt32 numChildren = LengthOf(mFirstChild);
  
  nsIFrame* childFrame;
  if (0 == numChildren) {
    // XXX This code should move to Init(), someday when the frame construction
    // changes are all done and Init() is always getting called...
    nsIHTMLContent* text = nsnull;
    nsIAtom* tag = NS_NewAtom("text");
    NS_NewHTMLInputElement(&text, tag);
    text->SetAttribute("type", "text", PR_FALSE);
    NS_NewTextControlFrame(text, this, childFrame);
    childFrame->SetStyleContext(&aPresContext, mStyleContext);
    mTextFrame = (nsTextControlFrame*)childFrame;
    mFirstChild = childFrame;

    nsIHTMLContent* browse = nsnull;
    tag = NS_NewAtom("browse");
    NS_NewHTMLInputElement(&browse, tag);
    browse->SetAttribute("type", "browse", PR_FALSE);
    NS_NewButtonControlFrame(browse, this, childFrame);
    ((nsButtonControlFrame*)childFrame)->SetFileControlFrame(this);
    mBrowseFrame = (nsButtonControlFrame*)childFrame;
    childFrame->SetStyleContext(&aPresContext, mStyleContext);

    mFirstChild->SetNextSibling(childFrame);

    NS_RELEASE(text);
    NS_RELEASE(browse);
  }

  nsSize maxSize = aReflowState.maxSize;
  nsReflowMetrics desiredSize = aDesiredSize;
  aDesiredSize.width = CONTROL_SPACING; 
  aDesiredSize.height = 0;
  childFrame = mFirstChild;
  nsPoint offset(0,0);
  while (nsnull != childFrame) {  // reflow, place, size the children
    nsReflowState   reflowState(childFrame, aReflowState, maxSize);
    childFrame->WillReflow(aPresContext);
    nsresult result = childFrame->Reflow(aPresContext, desiredSize, reflowState, aStatus);
    NS_ASSERTION(NS_FRAME_IS_COMPLETE(aStatus), "bad status");
    nsRect rect(offset.x, offset.y, desiredSize.width, desiredSize.height);
    childFrame->SetRect(rect);
    maxSize.width  -= desiredSize.width;
    aDesiredSize.width  += desiredSize.width; 
    aDesiredSize.height = desiredSize.height;
    childFrame->GetNextSibling(childFrame);
    offset.x += desiredSize.width + CONTROL_SPACING;
  }

  aDesiredSize.ascent = aDesiredSize.height;
  aDesiredSize.descent = 0;

  if (nsnull != aDesiredSize.maxElementSize) {
    aDesiredSize.maxElementSize->width = aDesiredSize.width;
	  aDesiredSize.maxElementSize->height = aDesiredSize.height;
  }

  aStatus = NS_FRAME_COMPLETE;
  return NS_OK;
}

PRIntn
nsFileControlFrame::GetSkipSides() const
{
  return 0;
}


NS_IMETHODIMP
nsFileControlFrame::GetName(nsString* aResult)
{
  nsresult result = NS_FORM_NOTOK;
  if (mContent) {
    nsIHTMLContent* formControl = nsnull;
    result = mContent->QueryInterface(kIHTMLContentIID, (void**)&formControl);
    if ((NS_OK == result) && formControl) {
      nsHTMLValue value;
      result = formControl->GetAttribute(nsHTMLAtoms::name, value);
      if (NS_CONTENT_ATTR_HAS_VALUE == result) {
        if (eHTMLUnit_String == value.GetUnit()) {
          value.GetStringValue(*aResult);
        }
      }
      NS_RELEASE(formControl);
    }
  }
  return result;
}

PRInt32 
nsFileControlFrame::GetMaxNumValues()
{
  return 1;
}
  
PRBool
nsFileControlFrame::GetNamesValues(PRInt32 aMaxNumValues, PRInt32& aNumValues,
                                   nsString* aValues, nsString* aNames)
{
  nsAutoString name;
  nsresult result = GetName(&name);
  if ((aMaxNumValues <= 0) || (NS_CONTENT_ATTR_HAS_VALUE != result)) {
    return PR_FALSE;
  }

  // use our name and the text widgets value 
  aNames[0] = name;
  nsresult status = PR_FALSE;
  nsIWidget*  widget;
  nsITextWidget* textWidget;
  mTextFrame->GetWidget(&widget);
  if (widget && (NS_OK == widget->QueryInterface(kITextWidgetIID, (void**)&textWidget))) {
    PRUint32 actualSize;
    textWidget->GetText(aValues[0], 0, actualSize);
    aNumValues = 1;
    NS_RELEASE(textWidget);
    status = PR_TRUE;
  }
  NS_IF_RELEASE(widget);
  return status;
}

