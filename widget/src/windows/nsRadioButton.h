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
    NS_DECL_ISUPPORTS

    // nsIRadioButton part
    NS_IMETHOD              SetLabel(const nsString& aText);
    NS_IMETHOD              GetLabel(nsString& aBuffer);
    NS_IMETHOD              SetState(const PRBool aState);
    NS_IMETHOD              GetState(PRBool& aState);

    virtual PRBool          OnMove(PRInt32 aX, PRInt32 aY);
    virtual PRBool          OnPaint();
    virtual PRBool          OnResize(nsRect &aWindowRect);
    virtual void            GetBounds(nsRect &aRect);


protected:
    PRBool                  fState;

    virtual LPCTSTR         WindowClass();
    virtual DWORD           WindowStyle();
    virtual DWORD           WindowExStyle();

};

#endif // nsRadioButton_h__
