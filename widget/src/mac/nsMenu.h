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

#ifndef nsMenu_h__
#define nsMenu_h__

#include "nsIMenu.h"
#include "nsVoidArray.h"
#include "nsIMenuListener.h"

#include <Menus.h>
#include <UnicodeConverter.h>

class nsIMenuBar;
class nsIMenuListener;


// temporary hack to get apple menu -- sfraser, approved saari
#define APPLE_MENU_HACK		1

#ifdef APPLE_MENU_HACK
extern const PRInt16 kMacMenuID;
extern const PRInt16 kAppleMenuID;
#endif /* APPLE_MENU_HACK */

//static PRInt16      mMacMenuIDCount;		// use GetUniqueMenuID()
 extern PRInt16 mMacMenuIDCount;// = kMacMenuID;

class nsMenu : public nsIMenu, public nsIMenuListener
{

public:
  nsMenu();
  virtual ~nsMenu();

  NS_DECL_ISUPPORTS
  
  // nsIMenuListener methods
  nsEventStatus MenuItemSelected(const nsMenuEvent & aMenuEvent); 
  nsEventStatus MenuSelected(const nsMenuEvent & aMenuEvent); 
  nsEventStatus MenuDeselected(const nsMenuEvent & aMenuEvent); 
  nsEventStatus MenuConstruct(
    const nsMenuEvent & aMenuEvent,
    nsIWidget         * aParentWindow, 
    void              * menuNode,
	void              * aWebShell);
  nsEventStatus MenuDestruct(const nsMenuEvent & aMenuEvent);
  
  // nsIMenu Methods
  NS_IMETHOD Create(nsISupports * aParent, const nsString &aLabel);
  NS_IMETHOD GetParent(nsISupports *&aParent);
  NS_IMETHOD GetLabel(nsString &aText);
  NS_IMETHOD SetLabel(const nsString &aText);
  NS_IMETHOD GetAccessKey(nsString &aText);
  NS_IMETHOD SetAccessKey(const nsString &aText);
  NS_IMETHOD AddItem(nsISupports* aText);
  NS_IMETHOD AddSeparator();
  NS_IMETHOD GetItemCount(PRUint32 &aCount);
  NS_IMETHOD GetItemAt(const PRUint32 aPos, nsISupports *& aMenuItem);
  NS_IMETHOD InsertItemAt(const PRUint32 aPos, nsISupports * aMenuItem);
  NS_IMETHOD RemoveItem(const PRUint32 aPos);
  NS_IMETHOD RemoveAll();
  NS_IMETHOD GetNativeData(void** aData);
  NS_IMETHOD SetNativeData(void* aData);
  NS_IMETHOD AddMenuListener(nsIMenuListener * aMenuListener);
  NS_IMETHOD RemoveMenuListener(nsIMenuListener * aMenuListener);
  NS_IMETHOD SetDOMNode(nsIDOMNode * aMenuNode);
  NS_IMETHOD SetDOMElement(nsIDOMElement * aMenuElement);
  NS_IMETHOD SetWebShell(nsIWebShell * aWebShell);
  
  // 
  NS_IMETHOD AddMenuItem(nsIMenuItem * aMenuItem);
  NS_IMETHOD AddMenu(nsIMenu * aMenu);

  // MacSpecific
  static PRInt16	GetUniqueMenuID() {
  						if (mMacMenuIDCount == 32767)
  							mMacMenuIDCount = 256;
  						return mMacMenuIDCount++;
  						}

protected:
  nsString     mLabel;
  PRUint32     mNumMenuItems;
  nsVoidArray  mMenuItemVoidArray;

  nsIMenu    * mMenuParent;
  nsIMenuBar * mMenuBarParent;

  nsIDOMNode    * mDOMNode;
  nsIDOMElement * mDOMElement;
  nsIWebShell   * mWebShell;
  bool            mConstructed;

  // MacSpecific
  PRInt16			   mMacMenuID;
  MenuHandle           mMacMenuHandle;
  nsIMenuListener *    mListener;
  UnicodeToTextRunInfo mUnicodeTextRunConverter;
  PRBool               mIsHelpMenu;

void LoadMenuItem(
  nsIMenu *    pParentMenu,
  nsIDOMElement * menuitemElement,
  nsIDOMNode *    menuitemNode,
  unsigned short  menuitemIndex,
  nsIWebShell *   aWebShell);
  
void LoadSubMenu(
  nsIMenu *       pParentMenu,
  nsIDOMElement * menuElement,
  nsIDOMNode *    menuNode);
  
  nsEventStatus HelpMenuConstruct(
    const nsMenuEvent & aMenuEvent,
    nsIWidget         * aParentWindow, 
    void              * menuNode,
	void              * aWebShell);
	
void NSStringSetMenuItemText(MenuHandle macMenuHandle, short menuItem, nsString& nsString);
MenuHandle NSStringNewMenu(short menuID, nsString& menuTitle);
MenuHandle NSStringNewChildMenu(short menuID, nsString& menuTitle);

private:
  
};


#endif // nsMenu_h__
