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
 * The Original Code is mozilla.org code.
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

#include "nsTextHelper.h"
#include "nsToolkit.h"
#include "nsColor.h"
#include "nsGUIEvent.h"
#include "nsString.h"
#include "nsStringUtil.h"
#include <os2.h>


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
  ::WinSendMsg(mWnd, EM_SETTEXTLIMIT, (MPARAM)(INT)aChars, 0); 
  return NS_OK;
}

NS_METHOD  nsTextHelper::GetText(nsString& aTextBuffer, PRUint32 aBufferSize, PRUint32& aActualSize) {

  int length = WinQueryWindowTextLength(mWnd);
  int bufLength = length + 1;
  NS_ALLOC_CHAR_BUF(buf, 512, bufLength);
  int charsCopied = WinQueryWindowText(mWnd, bufLength, buf);
  aTextBuffer.SetLength(0);
  aTextBuffer.AppendWithConversion(buf);
  NS_FREE_CHAR_BUF(buf);
  aActualSize = charsCopied;
  return NS_OK;
}

NS_METHOD  nsTextHelper::SetText(const nsString &aText, PRUint32& aActualSize)
{ 
  mText = aText;

  WinSetWindowText(mWnd, NS_LossyConvertUCS2toASCII(aText).get());
  aActualSize = aText.Length();
  return NS_OK;
}

NS_METHOD  nsTextHelper::InsertText(const nsString &aText, PRUint32 aStartPos, PRUint32 aEndPos, PRUint32& aActualSize)
{ 
  nsString currentText;

  PRUint32 actualSize;
  GetText(currentText, 256, actualSize);
  nsString newText(aText);
  currentText.Insert(newText.get(), aStartPos, aText.Length());
  SetText(currentText,actualSize);
  aActualSize = aText.Length();

  mText = currentText;

  return NS_OK;
}
NS_METHOD  nsTextHelper::RemoveText()
{
  WinSetWindowText(mWnd, "");
  return NS_OK;
}
NS_METHOD  nsTextHelper::SetPassword(PRBool aIsPassword)
{
  mIsPassword = aIsPassword;
  return NS_OK;
}
NS_METHOD nsTextHelper::SetReadOnly(PRBool aReadOnlyFlag, PRBool& aOldFlag)
{
  aOldFlag = mIsReadOnly;
  mIsReadOnly = aReadOnlyFlag;
  // Update the widget
  ::WinSendMsg(mWnd, EM_SETREADONLY, (MPARAM) (BOOL)aReadOnlyFlag, (MPARAM)0);
  return NS_OK;
}
  
NS_METHOD nsTextHelper::SelectAll()
{
  ::WinSendMsg(mWnd, EM_SETSEL, (MPARAM) 0, (MPARAM)-1);
  return NS_OK;
}

NS_METHOD  nsTextHelper::SetSelection(PRUint32 aStartSel, PRUint32 aEndSel)
{
  ::WinSendMsg(mWnd, EM_SETSEL, (MPARAM) (INT)aStartSel, (MPARAM)(INT) (ULONG*)aEndSel); 
  return NS_OK;
}


NS_METHOD  nsTextHelper::GetSelection(PRUint32 *aStartSel, PRUint32 *aEndSel)
{
  ::WinSendMsg(mWnd, EM_QUERYSEL, (MPARAM) (ULONG*)aStartSel, (MPARAM) (ULONG*)aEndSel); 
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

nsTextHelper::nsTextHelper() : nsWindow(), nsITextWidget()
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
// return the window class name and initialize the class if needed
//
//-------------------------------------------------------------------------
PCSZ nsTextHelper::WindowClass()
{
    return WC_ENTRYFIELD_STRING;
}

//-------------------------------------------------------------------------
//
// return window styles
//
//-------------------------------------------------------------------------
ULONG nsTextHelper::WindowStyle()
{
    ULONG style =  WS_CLIPSIBLINGS | ES_MARGIN | ES_AUTOSCROLL;
    if (mIsPassword)
      style = style | ES_UNREADABLE;

    if (mIsReadOnly)
      style = style | ES_READONLY;
  
    return style; 
}

//-------------------------------------------------------------------------
//
// return window extended styles
//
//-------------------------------------------------------------------------

ULONG nsTextHelper::WindowExStyle()
{
    return 0;
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


