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
#ifndef nsViewer_h___
#define nsViewer_h___

#include "nsIWebWidget.h"
#include "nsIDocumentObserver.h"
#include "nsDocLoader.h"
#include "nsIAppShell.h"

#define WIDGET_DLL "raptorwidget.dll"
#define GFXWIN_DLL "raptorgfxwin.dll"
#define VIEW_DLL   "raptorview.dll"

class DocObserver : public nsIDocumentObserver {
public:
  DocObserver(nsIWebWidget* aWebWidget) {
    NS_INIT_REFCNT();
    mWebWidget = aWebWidget;
    NS_ADDREF(aWebWidget);
  }

  NS_DECL_ISUPPORTS;

  NS_IMETHOD SetTitle(const nsString& aTitle);

  virtual void BeginUpdate() { }
  virtual void EndUpdate() { }
  virtual void ContentChanged(nsIContent* aContent,
                              nsISupports* aSubContent) {}
  virtual void ContentAppended(nsIContent* aContainer) { }
  virtual void ContentInserted(nsIContent* aContainer,
                               nsIContent* aChild,
                               PRInt32 aIndexInContainer) { }
  virtual void ContentReplaced(nsIContent* aContainer,
                               nsIContent* aOldChild,
                               nsIContent* aNewChild,
                               PRInt32 aIndexInContainer) { }
  virtual void ContentWillBeRemoved(nsIContent* aContainer,
                                    nsIContent* aChild,
                                    PRInt32 aIndexInContainer) { }
  virtual void ContentHasBeenRemoved(nsIContent* aContainer,
                                     nsIContent* aChild,
                                     PRInt32 aIndexInContainer) { }
  virtual void StyleSheetAdded(nsIStyleSheet* aStyleSheet) { }


protected:
  ~DocObserver() {
    NS_RELEASE(mWebWidget);
  }

  nsIWebWidget* mWebWidget;
};

struct WindowData {
  nsIWebWidget* ww;
  DocObserver* observer;
  nsIWidget *windowWidget;

  WindowData() {
    ww = nsnull;
  }
};

class nsViewer : public nsDispatchListener {
  public:
    virtual void AddMenu(nsIWidget* aMainWindow);
    virtual void ShowConsole(WindowData* aWindata);
    virtual void CloseConsole();
    virtual void DoDebugRobot(WindowData* aWindata);
    virtual void CopySelection(WindowData* aWindata);
    virtual nsresult Run();
    virtual void Destroy(WindowData* wd);
    virtual void Stop();
    virtual void AfterDispatch();
    virtual void CrtSetDebug(PRUint32 aNewFlags);

    virtual nsDocLoader* SetupViewer(nsIWidget **aMainWindow);
    virtual void CleanupViewer(nsDocLoader* aDl);
    virtual nsEventStatus DispatchMenuItem(nsGUIEvent *aEvent);
    virtual nsresult ShowPrintPreview(nsIWebWidget* web, PRIntn aColumns);
    virtual WindowData* CreateTopLevel(const char* title, int aWidth, int aHeight);
    virtual void AddTestDocs(nsDocLoader* aDocLoader);
    virtual void AddTestDocsFromFile(nsDocLoader* aDocLoader, char *aFileName);
    virtual void DestroyAllWindows();
    virtual struct WindowData* FindWindowData(nsIWidget* aWidget);
    virtual PRBool GetFileNameFromFileSelector(nsIWidget* aParentWindow, nsString* aFileName);
    virtual void OpenHTMLFile(WindowData* wd);
    virtual void SelectAll(WindowData* wd);
    virtual void ProcessArguments(int argc, char **argv);
};

  // Set the single viewer.
extern void SetViewer(nsViewer* aViewer);


#endif