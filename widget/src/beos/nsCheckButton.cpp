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

#include "nsCheckButton.h"
#include "nsToolkit.h"
#include "nsColor.h"
#include "nsGUIEvent.h"
#include "nsString.h"
#include "nsStringUtil.h"

#include "nsILookAndFeel.h"
#include "nsWidgetsCID.h"
#include "nsIComponentManager.h"

#include "nsIDeviceContext.h"

NS_IMPL_ADDREF(nsCheckButton)
NS_IMPL_RELEASE(nsCheckButton)

//-------------------------------------------------------------------------
//
// nsCheckButton constructor
//
//-------------------------------------------------------------------------
nsCheckButton::nsCheckButton() : nsWindow() , nsICheckButton(),
  mState(PR_FALSE)
{
  NS_INIT_REFCNT();
}


//-------------------------------------------------------------------------
//
// nsCheckButton destructor
//
//-------------------------------------------------------------------------
nsCheckButton::~nsCheckButton()
{
}


/**
 * Implement the standard QueryInterface for NS_IWIDGET_IID and NS_ISUPPORTS_IID
 * @modify gpk 8/4/98
 * @param aIID The name of the class implementing the method
 * @param _classiiddef The name of the #define symbol that defines the IID
 * for the class (e.g. NS_ISUPPORTS_IID)
 * 
*/ 
nsresult nsCheckButton::QueryInterface(const nsIID& aIID, void** aInstancePtr)
{
    if (NULL == aInstancePtr) {
        return NS_ERROR_NULL_POINTER;
    }

    static NS_DEFINE_IID(kICheckButtonIID, NS_ICHECKBUTTON_IID);
    if (aIID.Equals(kICheckButtonIID)) {
        *aInstancePtr = (void*) ((nsICheckButton*)this);
        NS_ADDREF_THIS();
        return NS_OK;
    }
    return nsWindow::QueryInterface(aIID,aInstancePtr);
}


//-------------------------------------------------------------------------
//
// Set this button label
//
//-------------------------------------------------------------------------
NS_METHOD nsCheckButton::SetState(const PRBool aState)
{
	mState = aState;
	if(mCheckBox && mCheckBox->LockLooper())
	{
		mCheckBox->SetValue(aState ? 1 : 0);
		mCheckBox->UnlockLooper();
	}
	return NS_OK;
}

//-------------------------------------------------------------------------
//
// Set this button label
//
//-------------------------------------------------------------------------
NS_METHOD nsCheckButton::GetState(PRBool& aState)
{
	aState = mState;
	if(mCheckBox && mCheckBox->LockLooper())
	{
		aState = mCheckBox->Value() ? PR_TRUE : PR_FALSE;
		mCheckBox->UnlockLooper();
	}
	return NS_OK;
}

//-------------------------------------------------------------------------
//
// Set this button label
//
//-------------------------------------------------------------------------
NS_METHOD nsCheckButton::SetLabel(const nsString& aText)
{
	char label[256];
	aText.ToCString(label, 256);
	label[255] = '\0';
	if(mCheckBox && mCheckBox->LockLooper())
	{
		mCheckBox->SetLabel(label);
		mCheckBox->UnlockLooper();
	}
	return NS_OK;
}


//-------------------------------------------------------------------------
//
// Get this button label
//
//-------------------------------------------------------------------------
NS_METHOD nsCheckButton::GetLabel(nsString& aBuffer)
{
	if(mCheckBox && mCheckBox->LockLooper())
	{
		aBuffer.SetLength(0);
		aBuffer.Append(mCheckBox->Label());
		mCheckBox->UnlockLooper();
	}
	return NS_OK;
}

//-------------------------------------------------------------------------
//
// move, paint, resizes message - ignore
//
//-------------------------------------------------------------------------
PRBool nsCheckButton::OnMove(PRInt32, PRInt32)
{
  return PR_FALSE;
}

PRBool nsCheckButton::OnPaint(nsRect &r)
{
    return PR_FALSE;
}

PRBool nsCheckButton::OnResize(nsRect &aWindowRect)
{
    return PR_FALSE;
}


/**
 * Renders the CheckButton for Printing
 *
 **/
NS_METHOD nsCheckButton::Paint(nsIRenderingContext& aRenderingContext,
                               const nsRect&        aDirtyRect)
{
  nsRect rect;
  float  appUnits;
  float  scale;
  nsIDeviceContext * context;
  aRenderingContext.GetDeviceContext(context);

  context->GetCanonicalPixelScale(scale);
  context->GetDevUnitsToAppUnits(appUnits);

  GetBoundsAppUnits(rect, appUnits);

  nscoord one   = nscoord(PRFloat64(rect.height) * 1.0/20.0);
  nscoord three = nscoord(PRFloat64(rect.width)  * 3.0/20.0);
  nscoord five  = nscoord(PRFloat64(rect.width)  * 5.0/20.0);
  nscoord six   = nscoord(PRFloat64(rect.height) * 5.0/20.0);
  nscoord eight = nscoord(PRFloat64(rect.height) * 7.0/20.0);
  nscoord nine  = nscoord(PRFloat64(rect.width)  * 9.0/20.0);
  nscoord ten   = nscoord(PRFloat64(rect.height) * 9.0/20.0);

  rect.x      += three;
  rect.y      += nscoord(PRFloat64(rect.height) * 3.5 /20.0);
  rect.width  = nscoord(PRFloat64(rect.width) * 12.0/20.0);
  rect.height = nscoord(PRFloat64(rect.height) * 12.0/20.0);

  aRenderingContext.SetColor(NS_RGB(0,0,0));

  nscoord onePixel  = nscoord((appUnits+0.6F));
  DrawScaledRect(aRenderingContext, rect, scale, appUnits);
  nscoord x = rect.x;
  nscoord y = rect.y;

  if (mState) {
    nscoord inc   = nscoord(PRFloat64(rect.height) *   0.75/20.0);
    nscoord yy = 0;
    for (nscoord i=0;i<4;i++) {
      DrawScaledLine(aRenderingContext, x+three, y+eight+yy,  x+five, y+ten+yy, scale, appUnits, PR_FALSE); // top
      DrawScaledLine(aRenderingContext, x+five,  y+ten+yy,    x+nine, y+six+yy, scale, appUnits, PR_FALSE); // top
      //aRenderingContext.DrawLine(x+three, y+eight+yy,  x+five, y+ten+yy);
      //aRenderingContext.DrawLine(x+five,  y+ten+yy,    x+nine, y+six+yy);
      yy += nscoord(scale);
    }
  }

  NS_RELEASE(context);
  return NS_OK;
}

BView *nsCheckButton::CreateBeOSView()
{
	return mCheckBox = new nsCheckBoxBeOS(this, BRect(0, 0, 0, 0), "", "", NULL);
}

//-------------------------------------------------------------------------
// Sub-class of BeOS CheckBox
//-------------------------------------------------------------------------
nsCheckBoxBeOS::nsCheckBoxBeOS( nsIWidget *aWidgetWindow, BRect aFrame, 
    const char *aName, const char *aLabel, BMessage *aMessage,
    uint32 aResizingMode, uint32 aFlags )
  : BCheckBox( aFrame, aName, aLabel, aMessage, aResizingMode, aFlags ),
    nsIWidgetStore( aWidgetWindow )
{
}
