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

#ifndef nsRadioButton_h__
#define nsRadioButton_h__

#include "nsdefs.h"
#include "nsWindow.h"
#include "nsSwitchToUIThread.h"
#include "nsString.h"

#include "nsIRadioButton.h"

#include <RadioButton.h>

/**
 * Native Win32 Radio button wrapper
 */

class nsRadioButton : public nsWindow,
                      public nsIRadioButton
{

public:
                            nsRadioButton();
    virtual                 ~nsRadioButton();

     // nsISupports
    NS_IMETHOD QueryInterface(REFNSIID aIID, void** aInstancePtr);                           
    NS_IMETHOD_(nsrefcnt) AddRef(void);                                       
    NS_IMETHOD_(nsrefcnt) Release(void);          

    // nsIRadioButton part
    NS_IMETHOD              SetLabel(const nsString& aText);
    NS_IMETHOD              GetLabel(nsString& aBuffer);
    NS_IMETHOD              SetState(const PRBool aState);
    NS_IMETHOD              GetState(PRBool& aState);

    NS_IMETHOD Paint(nsIRenderingContext& aRenderingContext,
                     const nsRect& aDirtyRect);

    virtual PRBool          OnMove(PRInt32 aX, PRInt32 aY);
    virtual PRBool          OnPaint(nsRect &r);
    virtual PRBool          OnResize(nsRect &aWindowRect);


protected:
    PRBool                  fState;
	virtual BView *CreateBeOSView();
	BRadioButton	*mRadioButton;
};

//
// A BRadioButton subclass
//
class nsRadioButtonBeOS : public BRadioButton, public nsIWidgetStore {
  public:
    nsRadioButtonBeOS( nsIWidget *aWidgetWindow, BRect aFrame, const char *aName,
        const char *aLabel, BMessage *aMessage, uint32 aResizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP, 
        uint32 aFlags = B_WILL_DRAW | B_NAVIGABLE );
};

#endif // nsRadioButton_h__
