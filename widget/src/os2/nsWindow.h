/*
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is the Mozilla OS/2 libraries.
 *
 * The Initial Developer of the Original Code is John Fairhurst,
 * <john_fairhurst@iname.com>.  Portions created by John Fairhurst are
 * Copyright (C) 1999 John Fairhurst. All Rights Reserved.
 *
 * Contributor(s): 
 * This Original Code has been modified by IBM Corporation.
 * Modifications made by IBM described herein are
 * Copyright (c) International Business Machines
 * Corporation, 2000
 *
 * Modifications to Mozilla code or documentation
 * identified per MPL Section 3.3
 *
 * Date             Modified by     Description of modification
 * 03/23/2000       IBM Corp.      Added InvalidateRegion method.
 * 04/12/2000       IBM Corp.      Changed params on DispatchMouseEvent to match Windows..
 * 04/14/2000       IBM Corp.      Declared EventIsInsideWindow for CaptureRollupEvents
 *
 */

#ifndef _nswindow_h
#define _nswindow_h

#include "nsWidgetDefs.h"
#include "nsBaseWidget.h"
#include "nsToolkit.h"

class nsIMenuBar;
class nsContextMenu;

// Base widget class.
// This is abstract.  Controls (labels, radio buttons, listboxen) derive
// from here.  A thing called a child window derives from here, and the
// frame window class derives from the child.  The content-displaying
// classes are off on their own branch to avoid creating a palette for
// every window we create.  This may turn out to be what's required, in
// which case the paint & palette code from nsChildWindow needs to be
// munged in here.  nsFrameWindow is separate because work needs to be done
// there to decide whether methods apply to frame or client.

class nsWindow : public nsBaseWidget,
                 public nsSwitchToPMThread
{
 public:
   // Scaffolding
   nsWindow();
   virtual ~nsWindow();

   // nsIWidget

   // Creation from native (eh?) or widget parent, destroy
   NS_IMETHOD Create( nsIWidget *aParent,
                      const nsRect &aRect,
                      EVENT_CALLBACK aHandleEventFunction,
                      nsIDeviceContext *aContext,
                      nsIAppShell *aAppShell = nsnull,
                      nsIToolkit *aToolkit = nsnull,
                      nsWidgetInitData *aInitData = nsnull);
   NS_IMETHOD Create( nsNativeWidget aParent,
                      const nsRect &aRect,
                      EVENT_CALLBACK aHandleEventFunction,
                      nsIDeviceContext *aContext,
                      nsIAppShell *aAppShell = nsnull,
                      nsIToolkit *aToolkit = nsnull,
                      nsWidgetInitData *aInitData = nsnull);
   NS_IMETHOD Destroy(); // call before releasing

   // Hierarchy: only interested in widget children (it seems)
   virtual nsIWidget *GetParent();

   // Strangely misplaced menubar methods
   NS_IMETHOD         SetMenuBar( nsIMenuBar *aMenuBar);
   NS_IMETHOD         ShowMenuBar( PRBool bShow);

   // Physical properties
   NS_IMETHOD Show( PRBool bState);
   NS_IMETHOD Move( PRInt32 aX, PRInt32 aY);
   NS_IMETHOD Resize( PRInt32 aWidth,
                      PRInt32 aHeight,
                      PRBool   aRepaint);
   NS_IMETHOD Resize( PRInt32 aX,
                      PRInt32 aY,
                      PRInt32 aWidth,
                      PRInt32 aHeight,
                      PRBool   aRepaint);
   NS_IMETHOD GetClientBounds( nsRect &aRect);
   NS_IMETHOD GetBorderSize( PRInt32 &aWidth, PRInt32 &aHeight);
   NS_IMETHOD Enable( PRBool bState);
   NS_IMETHOD SetFocus();
   NS_IMETHOD IsVisible( PRBool &aState);

   NS_IMETHOD ModalEventFilter( PRBool aRealEvent, void *aEvent,
                                PRBool *aForWindow );

   NS_IMETHOD GetPreferredSize( PRInt32 &aWidth, PRInt32 &aHeight);
   NS_IMETHOD SetPreferredSize( PRInt32 aWidth, PRInt32 aHeight);

   NS_IMETHOD BeginResizingChildren();
   NS_IMETHOD EndResizingChildren();
   NS_IMETHOD WidgetToScreen( const nsRect &aOldRect, nsRect &aNewRect);
   NS_IMETHOD ScreenToWidget( const nsRect &aOldRect, nsRect &aNewRect);
   NS_IMETHOD DispatchEvent( struct nsGUIEvent *event, nsEventStatus &aStatus);
   NS_IMETHOD CaptureRollupEvents(nsIRollupListener * aListener, PRBool aDoCapture, PRBool aConsumeRollupEvent);

   // Widget appearance
   virtual nscolor         GetForegroundColor();
   NS_IMETHOD              SetForegroundColor( const nscolor &aColor);
   virtual nscolor         GetBackgroundColor();
   NS_IMETHOD              SetBackgroundColor( const nscolor &aColor);
   virtual nsIFontMetrics *GetFont();
   NS_IMETHOD              SetFont( const nsFont &aFont);
   NS_IMETHOD              SetColorMap( nsColorMap *aColorMap);
   NS_IMETHOD              SetCursor( nsCursor aCursor);
   NS_IMETHOD              SetTitle( const nsString& aTitle); 
   NS_IMETHOD              Invalidate( PRBool aIsSynchronous);
   NS_IMETHOD              Invalidate( const nsRect & aRect, PRBool aIsSynchronous);
   NS_IMETHOD              InvalidateRegion(const nsIRegion *aRegion, PRBool aIsSynchronous);
   NS_IMETHOD              Update();
   NS_IMETHOD              Scroll( PRInt32 aDx, PRInt32 aDy, nsRect *aClipRect);
   NS_IMETHOD              Paint( nsIRenderingContext& aRenderingContext,
                                  const nsRect& aDirtyRect);

   // Tooltips
   NS_IMETHOD SetTooltips( PRUint32 aNumberOfTips, nsRect *aTooltipAreas[]);   
   NS_IMETHOD RemoveTooltips();
   NS_IMETHOD UpdateTooltips( nsRect* aNewTips[]);

   // Get a HWND or a HPS.
   virtual void  *GetNativeData( PRUint32 aDataType);
   virtual void   FreeNativeData( void *aDatum, PRUint32 aDataType);

   // nsSwitchToPMThread interface
   NS_IMETHOD CallMethod( MethodInfo *info);

   // PM methods which need to be public (menus, etc)
   ULONG  GetNextID()    { return mNextID++; }
   USHORT GetNextCmdID() { return mNextCmdID++; }
   void   NS2PM_PARENT( POINTL &ptl);
   void   NS2PM( POINTL &ptl);
   void   SetContextMenu( nsContextMenu *aMenu);

 protected:
   static  nsWindow*   gCurrentWindow;
   static  PRBool      EventIsInsideWindow(nsWindow* aWindow); 
   // nsWindow methods subclasses must provide for creation to work
   virtual PCSZ  WindowClass() = 0;
   virtual ULONG WindowStyle() = 0;

   // hooks subclasses may wish to override!
   virtual void     PostCreateWidget()            {}
   virtual PRInt32  GetHeight( PRInt32 aHeight)   { return aHeight; }
   virtual PRInt32  GetClientHeight()             { return mBounds.height; }
   virtual ULONG    GetSWPFlags( ULONG flags)     { return flags; }
   virtual HWND     GetMainWindow() const         { return mWnd; }
   virtual void     SetupForPrint( HWND /*hwnd*/) {}

   // Useful functions for subclasses to use, threaded as necessary.
   virtual nsresult GetWindowText( nsString &str, PRUint32 *rc);
   virtual void     AddToStyle( ULONG style);
   virtual void     RemoveFromStyle( ULONG style);
   virtual void     GetStyle( ULONG &out);
   virtual nscolor  QueryPresParam( ULONG ppID);
   virtual void     SetPresParam( ULONG ppID, const nscolor &c);
   // return true if deferred
   virtual BOOL     SetWindowPos( HWND hwndInsertBehind, long x, long y,
                                  long cx, long cy, unsigned long flags);

   // Message handlers - may wish to override.  Default implementation for
   // palette, control, paint & scroll is to do nothing.

   // Return whether message has been processed.
   virtual PRBool ProcessMessage( ULONG m, MPARAM p1, MPARAM p2, MRESULT &r);
   virtual PRBool OnPaint();
   virtual void   OnDestroy();
   virtual PRBool OnReposition( PSWP pNewSwp);
   virtual PRBool OnResize( PRInt32 aX, PRInt32 aY);
   virtual PRBool OnMove( PRInt32 aX, PRInt32 aY);
   virtual PRBool OnKey( MPARAM mp1, MPARAM mp2);
   virtual PRBool OnRealizePalette();
   virtual PRBool OnScroll( MPARAM mp1, MPARAM mp2);
   virtual PRBool OnControl( MPARAM mp1, MPARAM mp2);
   virtual PRBool OnMenuClick( USHORT aCmd);
   virtual PRBool OnActivateMenu( HWND aMenu, BOOL aActivate);
   // called after param has been set...
   virtual PRBool OnPresParamChanged( MPARAM mp1, MPARAM mp2);
   virtual PRBool OnDragOver( MPARAM mp1, MPARAM mp2, MRESULT &mr);
   virtual PRBool OnDragLeave( MPARAM mp1, MPARAM mp2);
   virtual PRBool OnDrop( MPARAM mp1, MPARAM mp2);

   // PM data members
   HWND      mWnd;            // window handle
   PFNWP     mFnWP;           // previous window procedure
   nsWindow *mParent;         // parent widget
   ULONG     mNextID;         // next child window id
   USHORT    mNextCmdID;      // next WM_COMMAND id for menus
   PSWP      mSWPs;           // SWPs for deferred window positioning
   ULONG     mlHave, mlUsed;  // description of mSWPs array
   HPOINTER  mPointer;        // current PM pointer
   HPS       mPS;             // cache PS for window
   ULONG     mPSRefs;         // number of refs to cache ps
   BOOL      mDragInside;     // track draginside state
   VDKEY     mDeadKey;        // dead key from previous keyevent
   BOOL      mHaveDeadKey;    // is mDeadKey valid [0 may be a valid dead key, for all I know]
   HWND      mHackDestroyWnd; // access GetMainWindow() window from destructor
   QMSG      mQmsg;

   HWND      GetParentHWND() const;
   HWND      GetHWND() const   { return mWnd; }
   PFNWP     GetPrevWP() const { return mFnWP; }

   // nglayout data members
   PRInt32        mPreferredHeight;
   PRInt32        mPreferredWidth;
   nsToolkit     *mOS2Toolkit;
   nsFont        *mFont;
   nsIMenuBar    *mMenuBar;
   nsContextMenu *mActiveMenu; // record this so we can send it events

   // State of the window, used to emulate windows better...
   enum nsWindowState
   {
      nsWindowState_ePrecreate,       // default state; Create() not called
      nsWindowState_eInCreate,        // processing Create() method
      nsWindowState_eLive,            // active, existing window
      nsWindowState_eDoingDelete,     // object destructor running
      nsWindowState_eDead             // window destroyed
   } mWindowState;

   // Implementation ------------------------------
   void DoCreate( HWND hwndP, nsWindow *wndP, const nsRect &rect,
                  EVENT_CALLBACK aHandleEventFunction,
                  nsIDeviceContext *aContext, nsIAppShell *aAppShell,
                  nsIToolkit *aToolkit, nsWidgetInitData *aInitData);

   virtual void RealDoCreate( HWND hwndP, nsWindow *aParent,
                              const nsRect &aRect,
                              EVENT_CALLBACK aHandleEventFunction,
                              nsIDeviceContext *aContext,
                              nsIAppShell *aAppShell,
                              nsWidgetInitData *aInitData,
                              HWND hwndOwner = 0);

   virtual void SubclassWindow( PRBool bState);

   PRBool  ConvertStatus( nsEventStatus aStatus);
   void    InitEvent( nsGUIEvent &event, PRUint32 aEventType, nsPoint *pt = 0);
   PRBool  DispatchEventInternal( struct nsGUIEvent *event);
   PRBool  DispatchStandardEvent( PRUint32 aMsg, PRUint8 aStructType = NS_GUI_EVENT);
   virtual PRBool DispatchMouseEvent( PRUint32 aEventType, MPARAM mp1, MPARAM mp2);
   virtual PRBool DispatchResizeEvent( PRInt32 aClientX, PRInt32 aClientY);
   void    DeferPosition( HWND, HWND, long, long, long, long, ULONG);

   // Enumeration of the methods which are accessable on the PM thread
   enum {
      W_CREATE,
      W_DESTROY,
      W_SET_FOCUS,
      W_UPDATE_WINDOW,
      W_SET_TITLE,
      W_GET_TITLE
   };
   friend MRESULT EXPENTRY fnwpNSWindow( HWND, ULONG, MPARAM, MPARAM);
   friend MRESULT EXPENTRY fnwpFrame( HWND, ULONG, MPARAM, MPARAM);
};

#define PM2NS_PARENT NS2PM_PARENT
#define PM2NS NS2PM

extern PRUint32 WMChar2KeyCode( MPARAM mp1, MPARAM mp2);

extern nsWindow *NS_HWNDToWindow( HWND hwnd);

#define NSCANVASCLASS "WarpzillaCanvas"

#if 0

// Need to do this because the cross-platform widgets (toolbars) assume
// that the name of the NS_CHILD_CID is ChildWindow and that it gets
// defined in "nsWindow.h".
//
// However, if we've included this header *from nsCanvas.h*, then we
// get a lovely circular dependency, and so special-case this.
//
// Yes, I suppose I'm just being perverse by having three separate classes
// here, but I just baulk at naming a class 'ChildWindow'.
//
// (and don't tell me there's a JLib class called JMother.  Believe me,
//  I know, and I regret it at least twice a week...)
//
#ifndef _nscanvas_h
#include "nsCanvas.h"
typedef nsCanvas ChildWindow;
#endif

#endif

#endif
