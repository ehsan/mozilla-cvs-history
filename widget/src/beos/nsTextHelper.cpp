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

#include "nsTextHelper.h"
#include "nsToolkit.h"
#include "nsColor.h"
#include "nsGUIEvent.h"
#include "nsString.h"
#include "nsStringUtil.h"
#include <TextView.h>

NS_METHOD nsTextHelper::PreCreateWidget(nsWidgetInitData *aInitData)
{
  if (nsnull != aInitData) {
    nsTextWidgetInitData* data = (nsTextWidgetInitData *) aInitData;
    mIsPassword = data->mIsPassword;
    mIsReadOnly = data->mIsReadOnly;
  }
  return NS_OK;
}

NS_METHOD nsTextHelper::SetMaxTextLength(PRUint32 aChars)
{
	if(mTextView && mTextView->LockLooper())
	{
		mTextView->SetMaxBytes(aChars);
		mTextView->UnlockLooper();
	}
	return NS_OK;
}

NS_METHOD  nsTextHelper::GetText(nsString& aTextBuffer, PRUint32 aBufferSize, PRUint32& aActualSize)
{
	if(mTextView && mTextView->LockLooper())
	{
		aTextBuffer.SetLength(0);
		aTextBuffer.Append(mTextView->Text());
		aActualSize = strlen(mTextView->Text());
		mTextView->UnlockLooper();
	}
	return NS_OK;
}

NS_METHOD  nsTextHelper::SetText(const nsString &aText, PRUint32& aActualSize)
{ 
	mText = aText;
	
	const char *text;
	text = aText.ToNewCString();
	if(mTextView && mTextView->LockLooper())
	{
		mTextView->SetText(text);
		mTextView->UnlockLooper();
	}
	delete [] text;
	
	aActualSize = aText.Length();
	return NS_OK;
}

NS_METHOD  nsTextHelper::InsertText(const nsString &aText, PRUint32 aStartPos, PRUint32 aEndPos, PRUint32& aActualSize)
{
	const char *text;
	text = aText.ToNewCString();
	if(mTextView)
	{
		if(mTextView->LockLooper())
		{
			mTextView->Insert(aStartPos, text, aActualSize);
			mTextView->UnlockLooper();
		}
		else
			mTextView->Insert(aStartPos, text, aActualSize);
	}
	delete [] text;
	mText.Insert(aText, aStartPos, aText.Length());
	return NS_OK;
}

NS_METHOD  nsTextHelper::RemoveText()
{
	if(mTextView && mTextView->LockLooper())
	{
		mTextView->Delete(0, mTextView->TextLength() - 1);
		mTextView->UnlockLooper();
	}
	return NS_OK;
}

NS_METHOD  nsTextHelper::SetPassword(PRBool aIsPassword)
{
  mIsPassword = aIsPassword;
if(mIsPassword) printf("nsTextHelper::SetPassword not implemented\n");
  return NS_OK;
}

NS_METHOD nsTextHelper::SetReadOnly(PRBool aReadOnlyFlag, PRBool& aOldFlag)
{
	aOldFlag = mIsReadOnly;
	mIsReadOnly = aReadOnlyFlag;

	// Update the widget
	if(mTextView && mTextView->LockLooper())
	{
		mTextView->MakeEditable(false);
		mTextView->UnlockLooper();
	}

	return NS_OK;
}
  
NS_METHOD nsTextHelper::SelectAll()
{
	if(mTextView && mTextView->LockLooper())
	{
		mTextView->SelectAll();
		mTextView->UnlockLooper();
	}
	return NS_OK;
}

NS_METHOD  nsTextHelper::SetSelection(PRUint32 aStartSel, PRUint32 aEndSel)
{
	if(mTextView && mTextView->LockLooper())
	{
		mTextView->Select(aStartSel, aEndSel);
		mTextView->UnlockLooper();
	}
	return NS_OK;
}


NS_METHOD  nsTextHelper::GetSelection(PRUint32 *aStartSel, PRUint32 *aEndSel)
{
	if(mTextView && mTextView->LockLooper())
	{
		mTextView->GetSelection((int32 *)aStartSel, (int32 *)aEndSel);
		mTextView->UnlockLooper();
	}
	return NS_OK;
}

NS_METHOD  nsTextHelper::SetCaretPosition(PRUint32 aPosition)
{
  SetSelection(aPosition, aPosition);
  return NS_OK;
}

NS_METHOD  nsTextHelper::GetCaretPosition(PRUint32& aPos)
{
  PRUint32 start;
  PRUint32 end;
  GetSelection(&start, &end);
  if (start == end) {
    aPos = start;
  }
  else {
    aPos =  PRUint32(-1);/* XXX is this right??? scary cast! */
  }
  return NS_OK;
}

//-------------------------------------------------------------------------
//
// nsTextHelper constructor
//
//-------------------------------------------------------------------------

nsTextHelper::nsTextHelper() : nsWindow(), nsITextAreaWidget(), nsITextWidget()
{
  mIsReadOnly = PR_FALSE;
  mIsPassword = PR_FALSE;
}

//-------------------------------------------------------------------------
//
// nsTextHelper destructor
//
//-------------------------------------------------------------------------
nsTextHelper::~nsTextHelper()
{
}

//-------------------------------------------------------------------------
//
// Clear window before paint
//
//-------------------------------------------------------------------------

PRBool nsTextHelper::AutoErase()
{
  return(PR_TRUE);
}


TextFrameBeOS::TextFrameBeOS(BTextView *child, BRect frame, const char *name, uint32 resizingmode, uint32 flags)
 : BView(frame, name, resizingmode, flags | B_FRAME_EVENTS), tv(child)
{
	SetViewColor(0, 0, 0);
	AddChild(tv);
}

void TextFrameBeOS::FrameResized(float width, float height)
{
	tv->MoveTo(1, 1);
	tv->ResizeTo(width - 2, height - 2);
	BRect r = Bounds();
	r.InsetBy(1, 1);
	tv->SetTextRect(r);
	BView::FrameResized(width, height);
}
