/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
// vim:set ts=2 sts=2 sw=2 et cin:
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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Pierre Phaneuf <pp@ludusdesign.com>
 *   Jacek Piskozub <piskozub@iopan.gda.pl>
 *   Leon Sha <leon.sha@sun.com>
 *   Roland Mainz <roland.mainz@informatik.med.uni-giessen.de>
 *   Robert O'Callahan <roc+moz@cs.cmu.edu>
 *   Christian Biesinger <cbiesinger@web.de>
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
#include "nscore.h"
#include "nsCOMPtr.h"
#include "nsAutoPtr.h"
#include "nsHTMLParts.h"
#include "nsHTMLContainerFrame.h"
#include "nsPresContext.h"
#include "nsIPresShell.h"
#include "nsStyleSet.h"
#include "nsWidgetsCID.h"
#include "nsIView.h"
#include "nsIViewManager.h"
#include "nsIDOMKeyListener.h"
#include "nsIPluginHost.h"
#include "nsplugin.h"
#include "nsString.h"
#include "nsReadableUtils.h"
#include "prmem.h"
#include "nsHTMLAtoms.h"
#include "nsIDocument.h"
#include "nsIHTMLDocument.h"
#include "nsINodeInfo.h"
#include "nsIURL.h"
#include "nsNetUtil.h"
#include "nsIPluginInstanceOwner.h"
#include "nsISupportsArray.h"
#include "plstr.h"
#include "nsILinkHandler.h"
#ifdef OJI
#include "nsIJVMPluginTagInfo.h"
#endif
#include "nsINameSpaceManager.h"
#include "nsIEventListener.h"
#include "nsIScrollableView.h"
#include "nsIScrollPositionListener.h"
#include "nsIStringStream.h" // for NS_NewCharInputStream
#include "nsITimer.h"
#include "nsLayoutAtoms.h"
#include "nsIDocShellTreeItem.h"
#include "nsIDocShellTreeOwner.h"
#include "nsIWebNavigation.h"
#include "nsIWebNavigationInfo.h"
#include "nsDocShellCID.h"
#include "nsIWebBrowserChrome.h"
#include "nsIDOMElement.h"
#include "nsIDOMNodeList.h"
#include "nsIDOMHTMLObjectElement.h"
#include "nsIDOMHTMLEmbedElement.h"
#include "nsIDOMHTMLAppletElement.h"
#include "nsIDOMElementCSSInlineStyle.h"
#include "nsIDOMCSSStyleDeclaration.h"
#include "nsIDOMCSS2Properties.h"
#include "nsContentPolicyUtils.h"
#include "nsIDOMWindow.h"
#include "nsIDOMDocumentEvent.h"
#include "nsIDOMMouseListener.h"
#include "nsIDOMMouseMotionListener.h"
#include "nsIDOMFocusListener.h"
#include "nsIDOMContextMenuListener.h"
#include "nsIDOMDragListener.h"
#include "nsIDOMEventReceiver.h"
#include "nsIDOMNSEvent.h"
#include "nsITextContent.h"
#include "nsIPrivateDOMEvent.h"
#include "nsIDocumentEncoder.h"
#include "nsXPIDLString.h"
#include "nsIDOMRange.h"
#include "nsIPluginWidget.h"
#include "nsGUIEvent.h"
#include "nsIRenderingContext.h"
#include "npapi.h"
#include "nsGfxCIID.h"
#include "nsUnicharUtils.h"
#include "nsTransform2D.h"
#include "nsIImageLoadingContent.h"
#include "nsIObjectLoadingContent.h"
#include "nsPIDOMWindow.h"
#include "nsContentUtils.h"
#include "nsIStringBundle.h"
#include "nsAttrName.h"

// headers for plugin scriptability
#include "nsIScriptGlobalObject.h"
#include "nsIScriptContext.h"
#include "nsIXPConnect.h"
#include "nsIXPCScriptable.h"
#include "nsIClassInfo.h"
#include "jsapi.h"

#include "nsCSSPseudoClasses.h"

// XXX For temporary paint code
#include "nsStyleContext.h"

// For mime types
#include "nsMimeTypes.h"
#include "nsIMIMEService.h"
#include "nsCExternalHandlerService.h"
#include "imgILoader.h"

#include "nsObjectFrame.h"
#include "nsIObjectFrame.h"
#include "nsPluginNativeWindow.h"
#include "nsPIPluginHost.h"
#include "nsIPluginDocument.h"

// accessibility support
#ifdef ACCESSIBILITY
#include "nsIAccessibilityService.h"
#endif

#ifdef MOZ_LOGGING
#define FORCE_PR_LOG 1 /* Allow logging in the release build */
#endif /* MOZ_LOGGING */
#include "prlog.h"

#include <errno.h>

#include "nsContentCID.h"
static NS_DEFINE_CID(kRangeCID, NS_RANGE_CID);

/* X headers suck */
#ifdef KeyPress
#undef KeyPress
#endif

#ifdef XP_WIN
#include <wtypes.h>
#include <winuser.h>
#endif

#ifdef CreateEvent // Thank you MS.
#undef CreateEvent
#endif

#ifdef PR_LOGGING 
static PRLogModuleInfo *nsObjectFrameLM = PR_NewLogModule("nsObjectFrame");
#endif /* PR_LOGGING */

// True if the default plugin is disabled. Initialize to non-boolean
// value so that we know if we've checked the pref or not.
static PRBool sDefaultPluginDisabled = 0xffffffff;

// special class for handeling DOM context menu events because for
// some reason it starves other mouse events if implemented on the
// same class
class nsPluginDOMContextMenuListener : public nsIDOMContextMenuListener,
                                       public nsIEventListener
{
public:
  nsPluginDOMContextMenuListener();
  virtual ~nsPluginDOMContextMenuListener();

  NS_DECL_ISUPPORTS

  NS_IMETHOD ContextMenu(nsIDOMEvent* aContextMenuEvent);
  
  nsresult Init(nsObjectFrame *aFrame);
  nsresult Destroy(nsObjectFrame *aFrame);
  
  NS_IMETHOD HandleEvent(nsIDOMEvent* aEvent)
  {
    return NS_OK;
  }
  nsEventStatus ProcessEvent(const nsGUIEvent& anEvent)
  {
    return nsEventStatus_eConsumeNoDefault;
  }
};


class nsPluginInstanceOwner : public nsIPluginInstanceOwner,
                              public nsIPluginTagInfo2,
#ifdef OJI
                              public nsIJVMPluginTagInfo,
#endif
                              public nsIEventListener,
                              public nsITimerCallback,
                              public nsIDOMMouseListener,
                              public nsIDOMMouseMotionListener,
                              public nsIDOMKeyListener,
                              public nsIDOMFocusListener,
                              public nsIScrollPositionListener,
                              public nsIDOMDragListener

{
public:
  nsPluginInstanceOwner();
  virtual ~nsPluginInstanceOwner();

  NS_DECL_ISUPPORTS

  //nsIPluginInstanceOwner interface

  NS_IMETHOD SetInstance(nsIPluginInstance *aInstance);

  NS_IMETHOD GetInstance(nsIPluginInstance *&aInstance);

  NS_IMETHOD GetWindow(nsPluginWindow *&aWindow);

  NS_IMETHOD GetMode(nsPluginMode *aMode);

  NS_IMETHOD CreateWidget(void);

  NS_IMETHOD GetURL(const char *aURL, const char *aTarget, void *aPostData, 
                    PRUint32 aPostDataLen, void *aHeadersData, 
                    PRUint32 aHeadersDataLen, PRBool isFile = PR_FALSE);

  NS_IMETHOD ShowStatus(const char *aStatusMsg);

  NS_IMETHOD ShowStatus(const PRUnichar *aStatusMsg);
  
  NS_IMETHOD GetDocument(nsIDocument* *aDocument);

  NS_IMETHOD InvalidateRect(nsPluginRect *invalidRect);

  NS_IMETHOD InvalidateRegion(nsPluginRegion invalidRegion);

  NS_IMETHOD ForceRedraw();

  NS_IMETHOD GetValue(nsPluginInstancePeerVariable variable, void *value);

  NS_IMETHOD PluginNotAvailable(const char *aMimeType);

  //nsIPluginTagInfo interface

  NS_IMETHOD GetAttributes(PRUint16& n, const char*const*& names,
                           const char*const*& values);

  NS_IMETHOD GetAttribute(const char* name, const char* *result);

  //nsIPluginTagInfo2 interface

  NS_IMETHOD GetTagType(nsPluginTagType *result);

  NS_IMETHOD GetTagText(const char* *result);

  NS_IMETHOD GetParameters(PRUint16& n, const char*const*& names, const char*const*& values);

  NS_IMETHOD GetParameter(const char* name, const char* *result);
  
  NS_IMETHOD GetDocumentBase(const char* *result);
  
  NS_IMETHOD GetDocumentEncoding(const char* *result);
  
  NS_IMETHOD GetAlignment(const char* *result);
  
  NS_IMETHOD GetWidth(PRUint32 *result);
  
  NS_IMETHOD GetHeight(PRUint32 *result);

  NS_IMETHOD GetBorderVertSpace(PRUint32 *result);
  
  NS_IMETHOD GetBorderHorizSpace(PRUint32 *result);

  NS_IMETHOD GetUniqueID(PRUint32 *result);

  NS_IMETHOD GetDOMElement(nsIDOMElement* *result);

#ifdef OJI
  //nsIJVMPluginTagInfo interface

  NS_IMETHOD GetCode(const char* *result);

  NS_IMETHOD GetCodeBase(const char* *result);

  NS_IMETHOD GetArchive(const char* *result);

  NS_IMETHOD GetName(const char* *result);

  NS_IMETHOD GetMayScript(PRBool *result);

#endif /* OJI */

 /** nsIDOMMouseListener interfaces 
  * @see nsIDOMMouseListener
  */
  NS_IMETHOD MouseDown(nsIDOMEvent* aMouseEvent);
  NS_IMETHOD MouseUp(nsIDOMEvent* aMouseEvent);
  NS_IMETHOD MouseClick(nsIDOMEvent* aMouseEvent);
  NS_IMETHOD MouseDblClick(nsIDOMEvent* aMouseEvent);
  NS_IMETHOD MouseOver(nsIDOMEvent* aMouseEvent);
  NS_IMETHOD MouseOut(nsIDOMEvent* aMouseEvent);
  NS_IMETHOD HandleEvent(nsIDOMEvent* aEvent);     
  /* END interfaces from nsIDOMMouseListener*/

  // nsIDOMMouseListener intefaces
  NS_IMETHOD MouseMove(nsIDOMEvent* aMouseEvent);
  NS_IMETHOD DragMove(nsIDOMEvent* aMouseEvent) { return NS_OK; }

  // nsIDOMKeyListener interfaces
  NS_IMETHOD KeyDown(nsIDOMEvent* aKeyEvent);
  NS_IMETHOD KeyUp(nsIDOMEvent* aKeyEvent);
  NS_IMETHOD KeyPress(nsIDOMEvent* aKeyEvent);
  // end nsIDOMKeyListener interfaces

  // nsIDOMFocuListener interfaces
  NS_IMETHOD Focus(nsIDOMEvent * aFocusEvent);
  NS_IMETHOD Blur(nsIDOMEvent * aFocusEvent);
  
  // nsIDOMDragListener interfaces
  NS_IMETHOD DragEnter(nsIDOMEvent* aMouseEvent);
  NS_IMETHOD DragOver(nsIDOMEvent* aMouseEvent);
  NS_IMETHOD DragExit(nsIDOMEvent* aMouseEvent);
  NS_IMETHOD DragDrop(nsIDOMEvent* aMouseEvent);
  NS_IMETHOD DragGesture(nsIDOMEvent* aMouseEvent);
  

  nsresult Destroy();  

  //nsIEventListener interface
  nsEventStatus ProcessEvent(const nsGUIEvent & anEvent);
  
  void Paint(const nsRect& aDirtyRect, PRUint32 ndc = 0);

  // nsITimerCallback interface
  NS_DECL_NSITIMERCALLBACK
  
  void CancelTimer();
  void StartTimer();

  // nsIScrollPositionListener interface
  NS_IMETHOD ScrollPositionWillChange(nsIScrollableView* aScrollable, nscoord aX, nscoord aY);
  NS_IMETHOD ScrollPositionDidChange(nsIScrollableView* aScrollable, nscoord aX, nscoord aY);

  //locals

  NS_IMETHOD Init(nsPresContext *aPresContext, nsObjectFrame *aFrame);

  nsPluginPort* GetPluginPort();
  void ReleasePluginPort(nsPluginPort * pluginPort);

  void SetPluginHost(nsIPluginHost* aHost);

#ifdef XP_MACOSX
  nsPluginPort* FixUpPluginWindow(PRInt32 inPaintState);
  void GUItoMacEvent(const nsGUIEvent& anEvent, EventRecord* origEvent, EventRecord& aMacEvent);
#endif

private:
  nsPluginNativeWindow       *mPluginWindow;
  nsCOMPtr<nsIPluginInstance> mInstance;
  nsObjectFrame              *mOwner;
  nsCString                   mDocumentBase;
  char                       *mTagText;
  nsCOMPtr<nsIWidget>         mWidget;
  nsCOMPtr<nsITimer>          mPluginTimer;
  nsCOMPtr<nsIPluginHost>     mPluginHost;
  PRPackedBool                mContentFocused;
  PRPackedBool                mWidgetVisible;    // used on Mac to store our widget's visible state
  PRUint16          mNumCachedAttrs;
  PRUint16          mNumCachedParams;
  char              **mCachedAttrParamNames;
  char              **mCachedAttrParamValues;
  
  nsPluginDOMContextMenuListener * mCXMenuListener;  // pointer to wrapper for nsIDOMContextMenuListener
  
  nsresult DispatchKeyToPlugin(nsIDOMEvent* aKeyEvent);
  nsresult DispatchMouseToPlugin(nsIDOMEvent* aMouseEvent);
  nsresult DispatchFocusToPlugin(nsIDOMEvent* aFocusEvent);

  nsresult EnsureCachedAttrParamArrays();
};

#if defined(XP_WIN) || (defined(DO_DIRTY_INTERSECT) && defined(XP_MACOSX))
static void ConvertTwipsToPixels(nsPresContext& aPresContext, nsRect& aTwipsRect, nsRect& aPixelRect);
#endif

  // Mac specific code to fix up port position and clip during paint
#ifdef XP_MACOSX

#ifdef DO_DIRTY_INTERSECT
  // convert relative coordinates to absolute
  static void ConvertRelativeToWindowAbsolute(nsIFrame* aFrame, nsPoint& aRel, nsPoint& aAbs, nsIWidget *&aContainerWidget);
#endif

  enum { ePluginPaintIgnore, ePluginPaintEnable, ePluginPaintDisable };

#endif // XP_MACOSX

nsObjectFrame::~nsObjectFrame()
{
}

NS_IMETHODIMP
nsObjectFrame::QueryInterface(const nsIID& aIID, void** aInstancePtr)
{
  NS_ENSURE_ARG_POINTER(aInstancePtr);
  *aInstancePtr = nsnull;

#ifdef DEBUG
  if (aIID.Equals(NS_GET_IID(nsIFrameDebug))) {
    *aInstancePtr = NS_STATIC_CAST(nsIFrameDebug*,this);
    return NS_OK;
  }
#endif

  if (aIID.Equals(NS_GET_IID(nsIObjectFrame))) {
    *aInstancePtr = NS_STATIC_CAST(nsIObjectFrame*,this);
    return NS_OK;
  } else if (aIID.Equals(NS_GET_IID(nsIFrame))) {
    *aInstancePtr = NS_STATIC_CAST(nsIFrame*,this);
    return NS_OK;
  } else if (aIID.Equals(NS_GET_IID(nsISupports))) {
    *aInstancePtr = NS_STATIC_CAST(nsIObjectFrame*,this);
    return NS_OK;
  }
  return NS_NOINTERFACE;
}

NS_IMETHODIMP_(nsrefcnt) nsObjectFrame::AddRef(void)
{
  NS_WARNING("not supported for frames");
  return 1;
}

NS_IMETHODIMP_(nsrefcnt) nsObjectFrame::Release(void)
{
  NS_WARNING("not supported for frames");
  return 1;
}

#ifdef ACCESSIBILITY
NS_IMETHODIMP nsObjectFrame::GetAccessible(nsIAccessible** aAccessible)
{
  nsCOMPtr<nsIAccessibilityService> accService = do_GetService("@mozilla.org/accessibilityService;1");

  if (accService) {
    return accService->CreateHTMLObjectFrameAccessible(this, aAccessible);
  }

  return NS_ERROR_FAILURE;
}

#ifdef XP_WIN
NS_IMETHODIMP nsObjectFrame::GetPluginPort(HWND *aPort)
{
  *aPort = (HWND) mInstanceOwner->GetPluginPort();
  return NS_OK;
}
#endif
#endif


static NS_DEFINE_CID(kWidgetCID, NS_CHILD_CID);
static NS_DEFINE_CID(kCAppShellCID, NS_APPSHELL_CID);
static NS_DEFINE_CID(kCPluginManagerCID, NS_PLUGINMANAGER_CID);

PRIntn
nsObjectFrame::GetSkipSides() const
{
  return 0;
}

// #define DO_DIRTY_INTERSECT 1   // enable dirty rect intersection during paint
NS_IMETHODIMP
nsObjectFrame::SetInitialChildList(nsPresContext*  aPresContext,
                                   nsIAtom*        aListName,
                                   nsIFrame*       aChildList)
{
  // we don't want to call this if it is already set (image)
  nsresult rv = NS_OK;
  if (mFrames.IsEmpty())
    rv = nsObjectFrameSuper::SetInitialChildList(aPresContext, aListName, aChildList);
  return rv;
}

static void
FirePluginNotFoundEvent(nsIContent *aTarget)
{
  nsContentUtils::DispatchTrustedEvent(aTarget->GetDocument(), aTarget,
                                       NS_LITERAL_STRING("PluginNotFound"),
                                       PR_TRUE, PR_TRUE);
}

NS_IMETHODIMP 
nsObjectFrame::Init(nsPresContext*   aPresContext,
                    nsIContent*      aContent,
                    nsIFrame*        aParent,
                    nsStyleContext*  aContext,
                    nsIFrame*        aPrevInFlow)
{
#ifdef DEBUG
  mInstantiating = PR_FALSE;
#endif

  if (sDefaultPluginDisabled == (PRBool)0xffffffff) {
    sDefaultPluginDisabled =
      nsContentUtils::GetBoolPref("plugin.default_plugin_disabled");
  }

  nsresult rv = nsObjectFrameSuper::Init(aPresContext, aContent, aParent,
                                         aContext, aPrevInFlow);
  nsCOMPtr<nsIObjectLoadingContent> objContent(do_QueryInterface(mContent));
  NS_ASSERTION(objContent, "Why not an object loading content?");
  objContent->HasNewFrame(this);
  return rv;
}

NS_IMETHODIMP
nsObjectFrame::Destroy(nsPresContext* aPresContext)
{
  NS_ASSERTION(!mInstantiating, "about to crash due to bug 136927");

  // we need to finish with the plugin before native window is destroyed
  // doing this in the destructor is too late.
  StopPlugin();
  
  return nsObjectFrameSuper::Destroy(aPresContext);
}

nsIAtom*
nsObjectFrame::GetType() const
{
  return nsLayoutAtoms::objectFrame; 
}

#ifdef DEBUG
NS_IMETHODIMP
nsObjectFrame::GetFrameName(nsAString& aResult) const
{
  return MakeFrameName(NS_LITERAL_STRING("ObjectFrame"), aResult);
}
#endif

nsresult
nsObjectFrame::CreateWidgetForView(nsIView* aView)
{
  // Bug 179822: Create widget and allow non-unicode SubClass
  nsWidgetInitData initData;
  initData.mUnicode = PR_FALSE;
  return aView->CreateWidget(kWidgetCID, &initData);
}

PRBool
nsObjectFrame::IsLeaf() const
{
  // We're actually a leaf.  We inherit from nsContainerFrame for
  // convenience for now, but we construct our own kids and the frame
  // constructor shouldn't be messing with them.
  // XXXbz ideally, we wouldn't have this child frame thing at all.
  return PR_TRUE;
}

nsresult
nsObjectFrame::CreateWidget(nscoord aWidth,
                            nscoord aHeight,
                            PRBool  aViewOnly)
{
  nsIView* view = GetView();
  NS_ASSERTION(view, "Object frames must have views");  
  if (!view) {
    return NS_OK;       //XXX why OK? MMP
  }

  nsIViewManager* viewMan = view->GetViewManager();
  // mark the view as hidden since we don't know the (x,y) until Paint
  // XXX is the above comment correct?
  viewMan->SetViewVisibility(view, nsViewVisibility_kHide);

  // Turn off double buffering on the Mac. This depends on bug 49743 and partially
  // fixes 32327, 19931 and 51787
#ifdef XP_MACOSX
  PRBool doubleBuffer =
    nsContentUtils::GetBoolPref("plugin.enable_double_buffer");
  
  viewMan->AllowDoubleBuffering(doubleBuffer);
#endif
  
  //this is ugly. it was ripped off from didreflow(). MMP
  // Position and size view relative to its parent, not relative to our
  // parent frame (our parent frame may not have a view).
  
  nsIView* parentWithView;
  nsPoint origin;
  nsRect r(0, 0, mRect.width, mRect.height);

  GetOffsetFromView(origin, &parentWithView);
  viewMan->ResizeView(view, r);
  viewMan->MoveViewTo(view, origin.x, origin.y);

  if (!aViewOnly && !view->HasWidget()) {
    nsresult rv = CreateWidgetForView(view);
    if (NS_FAILED(rv)) {
      return NS_OK;       //XXX why OK? MMP
    }
  }

  {
    // Here we set the background color for this widget because some plugins will use 
    // the child window background color when painting. If it's not set, it may default to gray
    // Sometimes, a frame doesn't have a background color or is transparent. In this
    // case, walk up the frame tree until we do find a frame with a background color
    for (nsIFrame* frame = this; frame; frame = frame->GetParent()) {
      const nsStyleBackground* background = frame->GetStyleBackground();
      if (!background->IsTransparent()) {  // make sure we got an actual color
        nsIWidget* win = view->GetWidget();
        if (win)
          win->SetBackgroundColor(background->mBackgroundColor);
        break;
      }
    }

  }

  viewMan->SetViewVisibility(view, nsViewVisibility_kShow);

  return NS_OK;
}

#define EMBED_DEF_WIDTH 240
#define EMBED_DEF_HEIGHT 200

void
nsObjectFrame::GetDesiredSize(nsPresContext* aPresContext,
                              const nsHTMLReflowState& aReflowState,
                              nsHTMLReflowMetrics& aMetrics)
{
  // By default, we have no area
  aMetrics.width = 0;
  aMetrics.height = 0;
  aMetrics.ascent = 0;
  aMetrics.descent = 0;

  if (IsHidden(PR_FALSE)) {
    if (aMetrics.mComputeMEW) {
      aMetrics.mMaxElementWidth = 0;
    }      
    return;
  }
  
  aMetrics.width = aReflowState.mComputedWidth;
  aMetrics.height = aReflowState.mComputedHeight;

  // for EMBED and APPLET, default to 240x200 for compatibility
  nsIAtom *atom = mContent->Tag();
  if (atom == nsHTMLAtoms::applet || atom == nsHTMLAtoms::embed) {
    float p2t = aPresContext->ScaledPixelsToTwips();
    if (aMetrics.width == NS_UNCONSTRAINEDSIZE) {
      aMetrics.width = PR_MIN(PR_MAX(NSIntPixelsToTwips(EMBED_DEF_WIDTH, p2t),
                                     aReflowState.mComputedMinWidth),
                              aReflowState.mComputedMaxWidth);
    }
    if (aMetrics.height == NS_UNCONSTRAINEDSIZE) {
      aMetrics.height = PR_MIN(PR_MAX(NSIntPixelsToTwips(EMBED_DEF_HEIGHT, p2t),
                                      aReflowState.mComputedMinHeight),
                               aReflowState.mComputedMaxHeight);
    }

#if defined (MOZ_WIDGET_GTK) || defined (MOZ_WIDGET_GTK2) || defined (MOZ_WIDGET_XLIB)  
    // We need to make sure that the size of the object frame does not
    // exceed the maximum size of X coordinates.  See bug #225357 for
    // more information.  In theory Gtk2 can handle large coordinates,
    // but underlying plugins can't.
    aMetrics.height = PR_MIN(NSIntPixelsToTwips(PR_INT16_MAX, p2t), aMetrics.height);
    aMetrics.width = PR_MIN(NSIntPixelsToTwips(PR_INT16_MAX, p2t), aMetrics.width);
#endif
  }

  // At this point, the width has an unconstrained value only if we have
  // nothing to go on (no width set, no information from the plugin, nothing).
  // Make up a number.
  if (aMetrics.width == NS_UNCONSTRAINEDSIZE) {
    aMetrics.width =
      (aReflowState.mComputedMinWidth != NS_UNCONSTRAINEDSIZE) ?
        aReflowState.mComputedMinWidth : 0;
  }

  // At this point, the height has an unconstrained value only in two cases:
  // a) We are in standards mode with percent heights and parent is auto-height
  // b) We have no height information at all.
  // In either case, we have to make up a number.
  if (aMetrics.height == NS_UNCONSTRAINEDSIZE) {
    aMetrics.height =
      (aReflowState.mComputedMinHeight != NS_UNCONSTRAINEDSIZE) ?
        aReflowState.mComputedMinHeight : 0;
  }

  // XXXbz don't add in the border and padding, because we screw up our
  // plugin's size and positioning if we do...  Eventually we _do_ want to
  // paint borders, though!  At that point, we will need to adjust the desired
  // size either here or in Reflow....  Further, we will need to fix Paint() to
  // call the superclass in all cases.
  
  // ascent
  aMetrics.ascent = aMetrics.height;

  if (aMetrics.mComputeMEW) {
    aMetrics.mMaxElementWidth = aMetrics.width;
  }
}

NS_IMETHODIMP
nsObjectFrame::Reflow(nsPresContext*           aPresContext,
                      nsHTMLReflowMetrics&     aMetrics,
                      const nsHTMLReflowState& aReflowState,
                      nsReflowStatus&          aStatus)
{
  DO_GLOBAL_REFLOW_COUNT("nsObjectFrame", aReflowState.reason);
  DISPLAY_REFLOW(aPresContext, this, aReflowState, aMetrics, aStatus);
  nsresult rv = NS_OK;

  // Get our desired size
  GetDesiredSize(aPresContext, aReflowState, aMetrics);

  // delay plugin instantiation until all children have
  // arrived. Otherwise there may be PARAMs or other stuff that the
  // plugin needs to see that haven't arrived yet.
  if (!GetContent()->IsDoneAddingChildren()) {
    aStatus = NS_FRAME_COMPLETE;
    return rv;
  }

  // if we are printing or print previewing, bail for now
  if (aPresContext->Medium() == nsLayoutAtoms::print) {
    aStatus = NS_FRAME_COMPLETE;
    return rv;
  }


  FixupWindow(nsSize(aMetrics.width, aMetrics.height));

  aStatus = NS_FRAME_COMPLETE;

  NS_FRAME_SET_TRUNCATION(aStatus, aReflowState, aMetrics);
  return rv;
}

nsresult
nsObjectFrame::InstantiatePlugin(nsIPluginHost* aPluginHost, 
                                 const char* aMimeType,
                                 nsIURI* aURI)
{

  mFullURL = aURI;

#ifdef DEBUG
  mInstantiating = PR_TRUE;
#endif

  nsCOMPtr<nsIDocument> doc;
  nsresult rv = mInstanceOwner->GetDocument(getter_AddRefs(doc));
  nsCOMPtr<nsIPluginDocument> pDoc (do_QueryInterface(doc));

  if (pDoc) {  /* full-page mode */
    nsCOMPtr<nsIStreamListener> stream;
    rv = aPluginHost->InstantiateFullPagePlugin(aMimeType, aURI,
          /* resulting stream listener */       *getter_AddRefs(stream),
                                                mInstanceOwner);
    if (NS_SUCCEEDED(rv))
      pDoc->SetStreamListener(stream);
  } else {   /* embedded mode */
    rv = aPluginHost->InstantiateEmbeddedPlugin(aMimeType, aURI,
                                                mInstanceOwner);
  }
#ifdef DEBUG
  mInstantiating = PR_FALSE;
#endif

  // XXX having to do this sucks. it'd be better to move the code from DidReflow
  // to FixupWindow or something.
  nsIPresShell *shell = GetPresContext()->GetPresShell();
  shell->AppendReflowCommand(this, eReflowType_StyleChanged, nsnull);
  return rv;
}

void
nsObjectFrame::FixupWindow(const nsSize& aSize)
{
  nsPresContext* presContext = GetPresContext();

  float t2p = presContext->TwipsToPixels();

  if (!mInstanceOwner)
    return;

  nsPluginWindow  *window;
  mInstanceOwner->GetWindow(window);

  NS_ENSURE_TRUE(window, /**/);

  nsPoint origin;
  nsIView *parentWithView;
  GetOffsetFromView(origin, &parentWithView);
  window->x = NSTwipsToIntPixels(origin.x, t2p);
  window->y = NSTwipsToIntPixels(origin.y, t2p);
  window->width = NSTwipsToIntPixels(aSize.width, t2p);
  window->height = NSTwipsToIntPixels(aSize.height, t2p);

  // on the Mac we need to set the clipRect to { 0, 0, 0, 0 } for now. This will keep
  // us from drawing on screen until the widget is properly positioned, which will not
  // happen until we have finished the reflow process.
  window->clipRect.top = 0;
  window->clipRect.left = 0;
#ifdef XP_MACOSX
  window->clipRect.bottom = 0;
  window->clipRect.right = 0;
#else
  window->clipRect.bottom = NSTwipsToIntPixels(aSize.height, t2p);
  window->clipRect.right = NSTwipsToIntPixels(aSize.width, t2p);
#endif
}

PRBool
nsObjectFrame::IsFocusable(PRInt32 *aTabIndex, PRBool aWithMouse)
{
  if (aTabIndex)
    *aTabIndex = -1;
  return nsObjectFrameSuper::IsFocusable(aTabIndex, aWithMouse);
}

PRBool
nsObjectFrame::IsHidden(PRBool aCheckVisibilityStyle) const
{
  if (aCheckVisibilityStyle) {
    if (!GetStyleVisibility()->IsVisibleOrCollapsed())
      return PR_TRUE;    
  }

  // only <embed> tags support the HIDDEN attribute
  if (mContent->Tag() == nsHTMLAtoms::embed) {
    // Yes, these are really the kooky ways that you could tell 4.x
    // not to hide the <embed> once you'd put the 'hidden' attribute
    // on the tag...

    // HIDDEN w/ no attributes gets translated as we are hidden for
    // compatibility w/ 4.x and IE so we don't create a non-painting
    // widget in layout. See bug 188959.
    nsAutoString hidden;
    if (mContent->GetAttr(kNameSpaceID_None, nsHTMLAtoms::hidden, hidden) &&
       (hidden.IsEmpty() ||
        !hidden.LowerCaseEqualsLiteral("false") &&
        !hidden.LowerCaseEqualsLiteral("no") &&
        !hidden.LowerCaseEqualsLiteral("off"))) {
      return PR_TRUE;
    }
  }

  return PR_FALSE;
}

nsPoint nsObjectFrame::GetWindowOriginInPixels(PRBool aWindowless)
{
  nsIView * parentWithView;
  nsPoint origin(0,0);

  GetOffsetFromView(origin, &parentWithView);

  // if it's windowless, let's make sure we have our origin set right
  // it may need to be corrected, like after scrolling
  if (aWindowless && parentWithView) {
    nsIViewManager* parentVM = parentWithView->GetViewManager();

    // Walk up all the views and add up their positions. This will give us our
    // absolute position which is what we want to give the plugin
    nsIView* theView = parentWithView;
    while (theView) {
      if (theView->GetViewManager() != parentVM)
        break;

      origin += theView->GetPosition();
      theView = theView->GetParent();
    }  
  }

  float t2p;
  t2p = GetPresContext()->TwipsToPixels();
  origin.x = NSTwipsToIntPixels(origin.x, t2p);
  origin.y = NSTwipsToIntPixels(origin.y, t2p);

  return origin;
}

NS_IMETHODIMP
nsObjectFrame::DidReflow(nsPresContext*            aPresContext,
                         const nsHTMLReflowState*  aReflowState,
                         nsDidReflowStatus         aStatus)
{
  nsresult rv = nsObjectFrameSuper::DidReflow(aPresContext, aReflowState, aStatus);

  // The view is created hidden; once we have reflowed it and it has been
  // positioned then we show it.
  if (aStatus != NS_FRAME_REFLOW_FINISHED) 
    return rv;

  PRBool bHidden = IsHidden();

  if (HasView()) {
    nsIView* view = GetView();
    nsIViewManager* vm = view->GetViewManager();
    if (vm)
      vm->SetViewVisibility(view, bHidden ? nsViewVisibility_kHide : nsViewVisibility_kShow);
  }

  nsPluginWindow *win = nsnull;
 
  nsCOMPtr<nsIPluginInstance> pi; 
  if (!mInstanceOwner ||
      NS_FAILED(rv = mInstanceOwner->GetInstance(*getter_AddRefs(pi))) ||
      !pi ||
      NS_FAILED(rv = mInstanceOwner->GetWindow(win)) || 
      !win)
    return rv;

  nsPluginNativeWindow *window = (nsPluginNativeWindow *)win;

#ifdef XP_MACOSX
  mInstanceOwner->FixUpPluginWindow(ePluginPaintDisable);
#endif

  if (bHidden)
    return rv;

  PRBool windowless = (window->type == nsPluginWindowType_Drawable);

  nsPoint origin = GetWindowOriginInPixels(windowless);

  window->x = origin.x;
  window->y = origin.y;

  // refresh the plugin port as well
  window->window = mInstanceOwner->GetPluginPort();

  // this will call pi->SetWindow and take care of window subclassing
  // if needed, see bug 132759
  window->CallSetWindow(pi);

  mInstanceOwner->ReleasePluginPort((nsPluginPort *)window->window);

  return rv;
}

NS_IMETHODIMP
nsObjectFrame::Paint(nsPresContext*       aPresContext,
                     nsIRenderingContext& aRenderingContext,
                     const nsRect&        aDirtyRect,
                     nsFramePaintLayer    aWhichLayer,
                     PRUint32             aFlags)
{
  if (!GetStyleVisibility()->IsVisibleOrCollapsed())
    return NS_OK;
  
  // If we are painting in Print Preview do nothing....
  if (aPresContext->Type() == nsPresContext::eContext_PrintPreview) {
    return NS_OK;
  }

  // determine if we are printing
  if (aPresContext->Type() == nsPresContext::eContext_Print) {
    // UNIX Plugins can't PP at this time, so draw an empty box
    // we only want to print on the content layer pass
    if (eFramePaintLayer_Content != aWhichLayer)
      return NS_OK;

    // if we are printing, we need to get the correct nsIPluginInstance
    // for THIS content node in order to call ->Print() on the right plugin

    // first, we need to get the document
    nsCOMPtr<nsIDocument> doc = mContent->GetDocument();
    NS_ENSURE_TRUE(doc, NS_ERROR_NULL_POINTER);

    // now we need to get the shell for the screen
    // XXX assuming that the shell at zero will always be the screen one
    nsIPresShell *shell = doc->GetShellAt(0);
    NS_ENSURE_TRUE(shell, NS_ERROR_NULL_POINTER);

    // then the shell can give us the screen frame for this content node
    nsIFrame* frame = shell->GetPrimaryFrameFor(mContent);
    NS_ENSURE_TRUE(frame, NS_ERROR_NULL_POINTER);

    // make sure this is REALLY an nsIObjectFrame
    // we may need to go through the children to get it
    nsIObjectFrame* objectFrame = nsnull;
    CallQueryInterface(frame,&objectFrame);
    if (!objectFrame)
      objectFrame = GetNextObjectFrame(aPresContext,frame);
    NS_ENSURE_TRUE(objectFrame,NS_ERROR_FAILURE);

    // finally we can get our plugin instance
    nsCOMPtr<nsIPluginInstance> pi;
    if (NS_FAILED(objectFrame->GetPluginInstance(*getter_AddRefs(pi))) || !pi)
      return NS_ERROR_FAILURE;

    // now we need to setup the correct location for printing
    nsresult rv;
    nsPluginWindow    window;
    nsPoint           origin;
    float             t2p;
    window.window =   nsnull;

    // prepare embedded mode printing struct
    nsPluginPrint npprint;
    npprint.mode = nsPluginMode_Embedded;

    // we need to find out if we are windowless or not
    PRBool windowless = PR_FALSE;
    pi->GetValue(nsPluginInstanceVariable_WindowlessBool, (void *)&windowless);
    window.type  =  windowless ? nsPluginWindowType_Drawable : nsPluginWindowType_Window;

    // Get the offset of the DC
    nsTransform2D* rcTransform;
    aRenderingContext.GetCurrentTransform(rcTransform);
    rcTransform->GetTranslationCoord(&origin.x, &origin.y);
    
    // Get the conversion factor between pixels and twips
    t2p = aPresContext->TwipsToPixels();

    // set it all up
    // XXX is windowless different?
    window.x = origin.x;
    window.y = origin.y;
    window.width = NSToCoordRound(mRect.width * t2p);
    window.height= NSToCoordRound(mRect.height * t2p);
    window.clipRect.bottom = 0; window.clipRect.top = 0;
    window.clipRect.left = 0; window.clipRect.right = 0;
    
// XXX platform specific printing code
#if defined(XP_UNIX) && !defined(XP_MACOSX)
    /* UNIX does things completely differently:
     * We call the plugin and it sends generated PostScript data into a
     * file handle we provide. If the plugin returns with success we embed
     * this PostScript code fragment into the PostScript job we send to the
     * printer.
     */

    PR_LOG(nsObjectFrameLM, PR_LOG_DEBUG, ("nsObjectFrame::Paint() start for X11 platforms\n"));
           
    FILE *plugintmpfile = tmpfile();
    if (!plugintmpfile) {
      PR_LOG(nsObjectFrameLM, PR_LOG_DEBUG, ("error: could not open tmp. file, errno=%d\n", errno));
      return NS_ERROR_FAILURE;
    }
 
    /* Send off print info to plugin */
    NPPrintCallbackStruct npPrintInfo;
    npPrintInfo.type = NP_PRINT;
    npPrintInfo.fp   = plugintmpfile;
    npprint.print.embedPrint.platformPrint = (void *)&npPrintInfo;
    /* aDirtyRect contains the right information for ps print */
    window.x =   aDirtyRect.x;
    window.y =   aDirtyRect.y;
    window.width =   aDirtyRect.width;
    window.height =   aDirtyRect.height;
    npprint.print.embedPrint.window        = window;
    rv = pi->Print(&npprint);
    if (NS_FAILED(rv)) {
      PR_LOG(nsObjectFrameLM, PR_LOG_DEBUG, ("error: plugin returned failure %lx\n", (long)rv));
      fclose(plugintmpfile);
      return rv;
    }

    /* Send data to printer */
    rv = aRenderingContext.RenderEPS(aDirtyRect, plugintmpfile);

    fclose(plugintmpfile);

    PR_LOG(nsObjectFrameLM, PR_LOG_DEBUG, ("plugin printing done, return code is %lx\n", (long)rv));

#else  // Windows and non-UNIX, non-Mac(Classic) cases

    // we need the native printer device context to pass to plugin
    // On Windows, this will be the HDC
    void* dc;
    dc = aRenderingContext.GetNativeGraphicData(nsIRenderingContext::NATIVE_WINDOWS_DC);

    npprint.print.embedPrint.platformPrint = dc;
    npprint.print.embedPrint.window = window;
    // send off print info to plugin
    rv = pi->Print(&npprint);

#endif

    // XXX Nav 4.x always sent a SetWindow call after print. Should we do the same?
    nsDidReflowStatus status = NS_FRAME_REFLOW_FINISHED; // should we use a special status?
    frame->DidReflow(shell->GetPresContext(),
                     nsnull, status);  // DidReflow will take care of it

    return rv;  // done with printing
  }

// Screen painting code
#ifdef XP_MACOSX
  // delegate all painting to the plugin instance.
  if ((NS_FRAME_PAINT_LAYER_FOREGROUND == aWhichLayer) && mInstanceOwner)
      mInstanceOwner->Paint(aDirtyRect);
#elif defined (XP_WIN) || defined(XP_OS2)
  if (NS_FRAME_PAINT_LAYER_FOREGROUND == aWhichLayer) {
    nsCOMPtr<nsIPluginInstance> inst;
    GetPluginInstance(*getter_AddRefs(inst));
    if (inst) {
      // Look if it's windowless
      nsPluginWindow * window;
      mInstanceOwner->GetWindow(window);

      if (window->type == nsPluginWindowType_Drawable) {
        // check if we need to call SetWindow with updated parameters
        PRBool doupdatewindow = PR_FALSE;

        // check if we need to update hdc
        void* hdc;
        hdc = aRenderingContext.GetNativeGraphicData(nsIRenderingContext::NATIVE_WINDOWS_DC);
        if (NS_REINTERPRET_CAST(PRUint32, window->window) != (PRUint32)(HDC)hdc) {
          window->window = NS_REINTERPRET_CAST(nsPluginPort*, hdc);
          doupdatewindow = PR_TRUE;
        }

        /*
         * Layout now has an optimized way of painting. Now we always get
         * a new drawing surface, sized to be just what's needed. Windowless
         * plugins need a transform applied to their origin so they paint
         * in the right place. Since |SetWindow| is no longer being used
         * to tell the plugin where it is, we dispatch a NPWindow through
         * |HandleEvent| to tell the plugin when its window moved
         */

        // Get the offset of the DC
        nsTransform2D* rcTransform;
        aRenderingContext.GetCurrentTransform(rcTransform);
        nsPoint origin;
        rcTransform->GetTranslationCoord(&origin.x, &origin.y);

        if ((window->x != origin.x) || (window->y != origin.y)) {
          window->x = origin.x;
          window->y = origin.y;
          doupdatewindow = PR_TRUE;
        }

        // if our location or visible area has changed, we need to tell the plugin
        if (doupdatewindow) {
#ifdef XP_WIN    // Windowless plugins on windows need a special event to update their location, see bug 135737

          // first, lets find out how big the window is, in pixels
          nsIViewManager* vm = aPresContext->GetViewManager();
            if (vm) {
              nsIView* view;
              vm->GetRootView(view);
              if (view) {
                nsIWidget* win = view->GetWidget();
                if (win) {
                  nsRect visibleRect;
                  win->GetBounds(visibleRect);         
                    
                  // next, get our plugin's rect so we can intersect it with the visible rect so we
                  // can tell the plugin where and how much to paint
                  NS_ASSERTION(window->type == nsPluginWindowType_Drawable,
                               "What happened to our window type?");
                  origin = GetWindowOriginInPixels(PR_TRUE);
                  nsRect winlessRect = nsRect(origin, nsSize(window->width, window->height));
                  winlessRect.IntersectRect(winlessRect, visibleRect);

                  // now check our cached window and only update plugin if something has changed
                  if (mWindowlessRect != winlessRect) {
                    mWindowlessRect = winlessRect;

                    WINDOWPOS winpos;
                    memset(&winpos, 0, sizeof(winpos));
                    winpos.x = mWindowlessRect.x;
                    winpos.y = mWindowlessRect.y;
                    winpos.cx = mWindowlessRect.width;
                    winpos.cy = mWindowlessRect.height;

                    // finally, update the plugin by sending it a WM_WINDOWPOSCHANGED event
                    nsPluginEvent pluginEvent;
                    pluginEvent.event = WM_WINDOWPOSCHANGED;
                    pluginEvent.wParam = 0;
                    pluginEvent.lParam = (uint32)&winpos;
                    PRBool eventHandled = PR_FALSE;

                    inst->HandleEvent(&pluginEvent, &eventHandled);
                  }
                }
              }
            }
#endif

          inst->SetWindow(window);        
        }

        mInstanceOwner->Paint(aDirtyRect, (PRUint32)(HDC)hdc);
      }
    }
  }
#endif
  DO_GLOBAL_REFLOW_COUNT_DSP("nsObjectFrame", &aRenderingContext);
  return NS_OK;
}

NS_IMETHODIMP
nsObjectFrame::HandleEvent(nsPresContext* aPresContext,
                           nsGUIEvent*     anEvent,
                           nsEventStatus*  anEventStatus)
{
  NS_ENSURE_ARG_POINTER(anEventStatus);
  nsresult rv = NS_OK;

  //FIX FOR CRASHING WHEN NO INSTANCE OWVER
  if (!mInstanceOwner)
    return NS_ERROR_NULL_POINTER;

  if (anEvent->message == NS_PLUGIN_ACTIVATE) {
    nsIContent* content = GetContent();
    if (content) {
      content->SetFocus(aPresContext);
      return rv;
    }
  }

#ifdef XP_WIN
  rv = nsObjectFrameSuper::HandleEvent(aPresContext, anEvent, anEventStatus);
  return rv;
#endif

  switch (anEvent->message) {
  case NS_DESTROY:
    mInstanceOwner->CancelTimer();
    break;
  case NS_GOTFOCUS:
  case NS_LOSTFOCUS:
    *anEventStatus = mInstanceOwner->ProcessEvent(*anEvent);
    break;
    
  default:
    // instead of using an event listener, we can dispatch events to plugins directly.
    rv = nsObjectFrameSuper::HandleEvent(aPresContext, anEvent, anEventStatus);
  }

  return rv;
}

nsresult nsObjectFrame::GetPluginInstance(nsIPluginInstance*& aPluginInstance)
{
  aPluginInstance = nsnull;

  if (mInstanceOwner == nsnull)
    return NS_ERROR_NULL_POINTER;
  
  return mInstanceOwner->GetInstance(aPluginInstance);
}

nsresult
nsObjectFrame::PrepareInstanceOwner()
{
  // First, have to stop any possibly running plugins.
  StopPlugin();

  NS_ASSERTION(!mInstanceOwner, "Must not have an instance owner here");

  mInstanceOwner = new nsPluginInstanceOwner();
  if (!mInstanceOwner)
    return NS_ERROR_OUT_OF_MEMORY;

  NS_ADDREF(mInstanceOwner);
  mInstanceOwner->Init(GetPresContext(), this);
  return NS_OK;
}

nsresult
nsObjectFrame::Instantiate(nsIChannel* aChannel, nsIStreamListener** aStreamListener)
{
  nsresult rv = PrepareInstanceOwner();
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIPluginHost> pluginHost(do_GetService(kCPluginManagerCID, &rv));
  if (NS_FAILED(rv))
    return rv;
  mInstanceOwner->SetPluginHost(pluginHost);

  // This must be done before instantiating the plugin
  FixupWindow(mRect.Size());

  aChannel->GetURI(getter_AddRefs(mFullURL));

  rv = pluginHost->InstantiatePluginForChannel(aChannel, mInstanceOwner, aStreamListener);

  // XXX having to do this sucks. it'd be better to move the code from DidReflow
  // to FixupWindow.
  nsIPresShell *shell = GetPresContext()->GetPresShell();
  shell->AppendReflowCommand(this, eReflowType_StyleChanged, nsnull);
  return rv;
}

nsresult
nsObjectFrame::Instantiate(const char* aMimeType, nsIURI* aURI)
{
  NS_ASSERTION(aMimeType || aURI, "Need a type or a URI!");
  nsresult rv = PrepareInstanceOwner();
  NS_ENSURE_SUCCESS(rv, rv);

  // This must be done before instantiating the plugin
  FixupWindow(mRect.Size());

  // get the nsIPluginHost service
  nsCOMPtr<nsIPluginHost> pluginHost(do_GetService(kCPluginManagerCID, &rv));
  if (NS_FAILED(rv))
    return rv;
  mInstanceOwner->SetPluginHost(pluginHost);

  rv = InstantiatePlugin(pluginHost, aMimeType, aURI);

  // finish up
  if (NS_SUCCEEDED(rv)) {
    nsCOMPtr<nsIPluginInstance> inst;
    mInstanceOwner->GetInstance(*getter_AddRefs(inst));
    if (inst) {
      // The plugin may have set up new interfaces; we need to mess with our JS
      // wrapper.  Note that we DO NOT want to call this if there is no plugin
      // instance!  That would just reenter Instantiate(), trying to create
      // said plugin instance.
      NotifyContentObjectWrapper();
    }
  }

  return rv;
}

void
nsObjectFrame::StopPlugin()
{
  if (mInstanceOwner != nsnull) {
    nsCOMPtr<nsIPluginInstance> inst;
    mInstanceOwner->GetInstance(*getter_AddRefs(inst));
    if (inst) {
      nsPluginWindow *win;
      mInstanceOwner->GetWindow(win);
      nsPluginNativeWindow *window = (nsPluginNativeWindow *)win;
      nsCOMPtr<nsIPluginInstance> nullinst;

      PRBool doCache = PR_TRUE;
      PRBool doCallSetWindowAfterDestroy = PR_FALSE;

      // first, determine if the plugin wants to be cached
      inst->GetValue(nsPluginInstanceVariable_DoCacheBool, 
                     (void *) &doCache);
      if (!doCache) {
        // then determine if the plugin wants Destroy to be called after
        // Set Window.  This is for bug 50547.
        inst->GetValue(nsPluginInstanceVariable_CallSetWindowAfterDestroyBool, 
                       (void *) &doCallSetWindowAfterDestroy);
        if (doCallSetWindowAfterDestroy) {
          inst->Stop();
          inst->Destroy();
          
          if (window) 
            window->CallSetWindow(nullinst);
          else 
            inst->SetWindow(nsnull);
        }
        else {
          if (window) 
            window->CallSetWindow(nullinst);
          else 
            inst->SetWindow(nsnull);

          inst->Stop();
          inst->Destroy();
        }
      }
      else {
        if (window) 
          window->CallSetWindow(nullinst);
        else 
          inst->SetWindow(nsnull);

        inst->Stop();
      }

      nsCOMPtr<nsIPluginHost> pluginHost = do_GetService(kCPluginManagerCID);
      if (pluginHost)
        pluginHost->StopPluginInstance(inst);

      // the frame is going away along with its widget
      // so tell the window to forget its widget too
      if (window)
        window->SetPluginWidget(nsnull);
    }

    mInstanceOwner->Destroy();
    NS_RELEASE(mInstanceOwner);
  }
}

void
nsObjectFrame::NotifyContentObjectWrapper()
{
  nsCOMPtr<nsIDocument> doc = mContent->GetDocument();
  if (!doc)
    return;

  nsIScriptGlobalObject *sgo = doc->GetScriptGlobalObject();
  if (!sgo)
    return;

  nsIScriptContext *scx = sgo->GetContext();
  if (!scx)
    return;

  JSContext *cx = (JSContext *)scx->GetNativeContext();

  nsCOMPtr<nsIXPConnectWrappedNative> wrapper;
  nsContentUtils::XPConnect()->
    GetWrappedNativeOfNativeObject(cx, sgo->GetGlobalJSObject(), mContent,
                                   NS_GET_IID(nsISupports),
                                   getter_AddRefs(wrapper));

  if (!wrapper) {
    // Nothing to do here if there's no wrapper for mContent
    return;
  }

  nsCOMPtr<nsIClassInfo> ci(do_QueryInterface(mContent));
  if (!ci)
    return;

  nsCOMPtr<nsISupports> s;
  ci->GetHelperForLanguage(nsIProgrammingLanguage::JAVASCRIPT,
                           getter_AddRefs(s));

  nsCOMPtr<nsIXPCScriptable> helper(do_QueryInterface(s));

  if (!helper) {
    // There's nothing we can do if there's no helper
    return;
  }

  JSObject *obj = nsnull;
  nsresult rv = wrapper->GetJSObject(&obj);
  if (NS_FAILED(rv))
    return;

  // Abuse the scriptable helper to trigger prototype setup for the
  // wrapper for mContent so that this plugin becomes part of the DOM
  // object.
  helper->PostCreate(wrapper, cx, obj);
}

// static
nsIObjectFrame *
nsObjectFrame::GetNextObjectFrame(nsPresContext* aPresContext, nsIFrame* aRoot)
{
  nsIFrame* child = aRoot->GetFirstChild(nsnull);

  while (child) {
    nsIObjectFrame* outFrame = nsnull;
    CallQueryInterface(child, &outFrame);
    if (outFrame) {
      nsCOMPtr<nsIPluginInstance> pi;
      outFrame->GetPluginInstance(*getter_AddRefs(pi));  // make sure we have a REAL plugin
      if (pi)
        return outFrame;
    }

    outFrame = GetNextObjectFrame(aPresContext, child);
    if (outFrame)
      return outFrame;
    child = child->GetNextSibling();
  }

  return nsnull;
}

nsIFrame*
NS_NewObjectFrame(nsIPresShell* aPresShell)
{
  return new (aPresShell) nsObjectFrame;
}


// nsPluginDOMContextMenuListener class implementation

nsPluginDOMContextMenuListener::nsPluginDOMContextMenuListener()
{
}

nsPluginDOMContextMenuListener::~nsPluginDOMContextMenuListener()
{
}

NS_IMPL_ISUPPORTS2(nsPluginDOMContextMenuListener, nsIDOMContextMenuListener, nsIEventListener)

NS_IMETHODIMP
nsPluginDOMContextMenuListener::ContextMenu(nsIDOMEvent* aContextMenuEvent)
{
  aContextMenuEvent->PreventDefault(); // consume event

  return NS_OK;
}

nsresult nsPluginDOMContextMenuListener::Init(nsObjectFrame *aFrame)
{
  nsIContent* content = aFrame->GetContent();

  // Register context menu listener
  if (content) {
    nsCOMPtr<nsIDOMEventTarget> receiver(do_QueryInterface(content));
    if (receiver) {
      nsCOMPtr<nsIDOMContextMenuListener> cxMenuListener;
      QueryInterface(NS_GET_IID(nsIDOMContextMenuListener), getter_AddRefs(cxMenuListener));
      if (cxMenuListener) {
        receiver->AddEventListener(NS_LITERAL_STRING("contextmenu"), cxMenuListener, PR_TRUE);
        return NS_OK;
      }
    }
  }

  return NS_ERROR_NO_INTERFACE;
}

nsresult nsPluginDOMContextMenuListener::Destroy(nsObjectFrame *aFrame)
{
  nsIContent* content = aFrame->GetContent();

  // Unregister context menu listener
  if (content) {
    nsCOMPtr<nsIDOMEventTarget> receiver(do_QueryInterface(content));
    if (receiver) {
      nsCOMPtr<nsIDOMContextMenuListener> cxMenuListener;
      QueryInterface(NS_GET_IID(nsIDOMContextMenuListener), getter_AddRefs(cxMenuListener));
      if (cxMenuListener) { 
        receiver->RemoveEventListener(NS_LITERAL_STRING("contextmenu"), cxMenuListener, PR_TRUE);
      }
      else NS_ASSERTION(PR_FALSE, "Unable to remove event listener for plugin");
    }
    else NS_ASSERTION(PR_FALSE, "plugin was not an event listener");
  }
  else NS_ASSERTION(PR_FALSE, "plugin had no content");

  return NS_OK;
}

//plugin instance owner

nsPluginInstanceOwner::nsPluginInstanceOwner()
{
  // create nsPluginNativeWindow object, it is derived from nsPluginWindow
  // struct and allows to manipulate native window procedure
  nsCOMPtr<nsIPluginHost> ph = do_GetService(kCPluginManagerCID);
  nsCOMPtr<nsPIPluginHost> pph(do_QueryInterface(ph));
  if (pph)
    pph->NewPluginNativeWindow(&mPluginWindow);
  else
    mPluginWindow = nsnull;

  mOwner = nsnull;
  mTagText = nsnull;
  mContentFocused = PR_FALSE;
  mWidgetVisible = PR_TRUE;
  mNumCachedAttrs = 0;
  mNumCachedParams = 0;
  mCachedAttrParamNames = nsnull;
  mCachedAttrParamValues = nsnull;
}

nsPluginInstanceOwner::~nsPluginInstanceOwner()
{
  PRInt32 cnt;

  // shut off the timer.
  if (mPluginTimer != nsnull) {
    CancelTimer();
  }

  mOwner = nsnull;

  for (cnt = 0; cnt < (mNumCachedAttrs + 1 + mNumCachedParams); cnt++) {
    if (mCachedAttrParamNames && mCachedAttrParamNames[cnt]) {
      PR_Free(mCachedAttrParamNames[cnt]);
      mCachedAttrParamNames[cnt] = nsnull;
    }

    if (mCachedAttrParamValues && mCachedAttrParamValues[cnt]) {
      PR_Free(mCachedAttrParamValues[cnt]);
      mCachedAttrParamValues[cnt] = nsnull;
    }
  }

  if (mCachedAttrParamNames) {
    PR_Free(mCachedAttrParamNames);
    mCachedAttrParamNames = nsnull;
  }

  if (mCachedAttrParamValues) {
    PR_Free(mCachedAttrParamValues);
    mCachedAttrParamValues = nsnull;
  }

  if (mTagText) {
    nsCRT::free(mTagText);
    mTagText = nsnull;
  }

#if defined(XP_UNIX) && !defined(XP_MACOSX)
  // the mem for this struct is allocated
  // by PR_MALLOC in ns4xPluginInstance.cpp:ns4xPluginInstance::SetWindow()
  if (mPluginWindow && mPluginWindow->ws_info) {
    PR_Free(mPluginWindow->ws_info);
    mPluginWindow->ws_info = nsnull;
  }
#endif

  // clean up plugin native window object
  nsCOMPtr<nsIPluginHost> ph = do_GetService(kCPluginManagerCID);
  nsCOMPtr<nsPIPluginHost> pph(do_QueryInterface(ph));
  if (pph) {
    pph->DeletePluginNativeWindow(mPluginWindow);
    mPluginWindow = nsnull;
  }
}

/*
 * nsISupports Implementation
 */

NS_IMPL_ADDREF(nsPluginInstanceOwner)
NS_IMPL_RELEASE(nsPluginInstanceOwner)

NS_INTERFACE_MAP_BEGIN(nsPluginInstanceOwner)
  NS_INTERFACE_MAP_ENTRY(nsIPluginInstanceOwner)
  NS_INTERFACE_MAP_ENTRY(nsIPluginTagInfo)
  NS_INTERFACE_MAP_ENTRY(nsIPluginTagInfo2)
#ifdef OJI
  NS_INTERFACE_MAP_ENTRY(nsIJVMPluginTagInfo)
#endif
  NS_INTERFACE_MAP_ENTRY(nsIEventListener)
  NS_INTERFACE_MAP_ENTRY(nsITimerCallback)
  NS_INTERFACE_MAP_ENTRY(nsIDOMMouseListener)
  NS_INTERFACE_MAP_ENTRY(nsIDOMMouseMotionListener)
  NS_INTERFACE_MAP_ENTRY(nsIDOMKeyListener)
  NS_INTERFACE_MAP_ENTRY(nsIDOMFocusListener)
  NS_INTERFACE_MAP_ENTRY(nsIScrollPositionListener)
  NS_INTERFACE_MAP_ENTRY(nsIDOMDragListener)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsIDOMEventListener, nsIDOMMouseListener)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIPluginInstanceOwner)
NS_INTERFACE_MAP_END

NS_IMETHODIMP nsPluginInstanceOwner::SetInstance(nsIPluginInstance *aInstance)
{
  mInstance = aInstance;

  return NS_OK;
}

NS_IMETHODIMP nsPluginInstanceOwner::GetWindow(nsPluginWindow *&aWindow)
{
  NS_ASSERTION(mPluginWindow, "the plugin window object being returned is null");
  aWindow = mPluginWindow;
  return NS_OK;
}

NS_IMETHODIMP nsPluginInstanceOwner::GetMode(nsPluginMode *aMode)
{
  nsCOMPtr<nsIDocument> doc;
  nsresult rv = GetDocument(getter_AddRefs(doc));
  nsCOMPtr<nsIPluginDocument> pDoc (do_QueryInterface(doc));

  if (pDoc) {
    *aMode = nsPluginMode_Full;
  } else {
    *aMode = nsPluginMode_Embedded;
  }

  return rv;
}

NS_IMETHODIMP nsPluginInstanceOwner::GetAttributes(PRUint16& n,
                                                     const char*const*& names,
                                                     const char*const*& values)
{
  nsresult rv = EnsureCachedAttrParamArrays();
  NS_ENSURE_SUCCESS(rv, rv);

  n = mNumCachedAttrs;
  names  = (const char **)mCachedAttrParamNames;
  values = (const char **)mCachedAttrParamValues;

  return rv;
}

NS_IMETHODIMP nsPluginInstanceOwner::GetAttribute(const char* name, const char* *result)
{
  NS_ENSURE_ARG_POINTER(name);
  NS_ENSURE_ARG_POINTER(result);
  
  nsresult rv = EnsureCachedAttrParamArrays();
  NS_ENSURE_SUCCESS(rv, rv);

  *result = nsnull;

  for (int i = 0; i < mNumCachedAttrs; i++) {
    if (0 == PL_strcasecmp(mCachedAttrParamNames[i], name)) {
      *result = mCachedAttrParamValues[i];
      return NS_OK;
    }
  }

  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsPluginInstanceOwner::GetDOMElement(nsIDOMElement* *result)
{
  NS_ENSURE_ARG_POINTER(result);
  nsresult rv = NS_ERROR_FAILURE;

  *result = nsnull;

  if (nsnull != mOwner) {
    nsIContent* cont = mOwner->GetContent();
    if (cont) {
      rv = CallQueryInterface(cont, result);
    }
  }

  return rv;
}

NS_IMETHODIMP nsPluginInstanceOwner::GetInstance(nsIPluginInstance *&aInstance)
{
  NS_IF_ADDREF(aInstance = mInstance);

  return NS_OK;
}

NS_IMETHODIMP nsPluginInstanceOwner::GetURL(const char *aURL, const char *aTarget, void *aPostData, PRUint32 aPostDataLen, void *aHeadersData, 
                                            PRUint32 aHeadersDataLen, PRBool isFile)
{
  NS_ENSURE_TRUE(mOwner,NS_ERROR_NULL_POINTER);

  // the container of the pres context will give us the link handler
  nsCOMPtr<nsISupports> container = mOwner->GetPresContext()->GetContainer();
  NS_ENSURE_TRUE(container,NS_ERROR_FAILURE);
  nsCOMPtr<nsILinkHandler> lh = do_QueryInterface(container);
  NS_ENSURE_TRUE(lh, NS_ERROR_FAILURE);

  nsAutoString  unitarget; unitarget.AssignASCII(aTarget); // XXX could this be nonascii?

  nsCOMPtr<nsIURI> baseURL;
  nsCOMPtr<nsIDocument> doc;
  nsresult rv = GetDocument(getter_AddRefs(doc));
  if (NS_SUCCEEDED(rv) && doc) {
    // XXX should this really be the document base URL?  Or the
    // content's base URL?
    baseURL = doc->GetBaseURI();  // gets the document's url
  } else {
    baseURL = mOwner->GetFullURL(); // gets the plugin's content url
  }

  // Create an absolute URL
  nsCOMPtr<nsIURI> uri;
  rv = NS_NewURI(getter_AddRefs(uri), aURL, baseURL);

  NS_ENSURE_SUCCESS(rv, NS_ERROR_FAILURE);
  nsIContent* content = mOwner->GetContent();
  NS_ENSURE_TRUE(content, NS_ERROR_FAILURE);

  nsCOMPtr<nsIInputStream> postDataStream;
  nsCOMPtr<nsIInputStream> headersDataStream;

  // deal with post data, either in a file or raw data, and any headers
  if (aPostData) {

    rv = NS_NewPluginPostDataStream(getter_AddRefs(postDataStream), (const char *)aPostData, aPostDataLen, isFile);

    NS_ASSERTION(NS_SUCCEEDED(rv),"failed in creating plugin post data stream");
    if (NS_FAILED(rv))
      return rv;

    if (aHeadersData) {
      rv = NS_NewPluginPostDataStream(getter_AddRefs(headersDataStream), 
                                      (const char *) aHeadersData, 
                                      aHeadersDataLen,
                                      PR_FALSE,
                                      PR_TRUE);  // last arg says we are headers, no /r/n/r/n fixup!

      NS_ASSERTION(NS_SUCCEEDED(rv),"failed in creating plugin header data stream");
      if (NS_FAILED(rv))
        return rv;
    }
  }

  PRInt32 blockPopups =
    nsContentUtils::GetIntPref("privacy.popups.disable_from_plugins");
  nsAutoPopupStatePusher popupStatePusher((PopupControlState)blockPopups);

  rv = lh->OnLinkClick(content, eLinkVerb_Replace, 
                       uri, unitarget.get(), 
                       postDataStream, headersDataStream);

  return rv;
}

NS_IMETHODIMP nsPluginInstanceOwner::ShowStatus(const char *aStatusMsg)
{
  nsresult  rv = NS_ERROR_FAILURE;
  
  rv = this->ShowStatus(NS_ConvertUTF8toUCS2(aStatusMsg).get());
  
  return rv;
}

NS_IMETHODIMP nsPluginInstanceOwner::ShowStatus(const PRUnichar *aStatusMsg)
{
  nsresult  rv = NS_ERROR_FAILURE;

  if (!mOwner) {
    return rv;
  }
  nsCOMPtr<nsISupports> cont = mOwner->GetPresContext()->GetContainer();
  if (!cont) {
    return NS_OK;
  }

  nsCOMPtr<nsIDocShellTreeItem> docShellItem(do_QueryInterface(cont, &rv));
  if (NS_FAILED(rv) || !docShellItem) {
    return rv;
  }

  nsCOMPtr<nsIDocShellTreeOwner> treeOwner;
  rv = docShellItem->GetTreeOwner(getter_AddRefs(treeOwner));
  if (NS_FAILED(rv) || !treeOwner) {
    return rv;
  }

  nsCOMPtr<nsIWebBrowserChrome> browserChrome(do_GetInterface(treeOwner, &rv));
  if (NS_FAILED(rv) || !browserChrome) {
    return rv;
  }
  rv = browserChrome->SetStatus(nsIWebBrowserChrome::STATUS_SCRIPT, 
                                aStatusMsg);

  return rv;
}

NS_IMETHODIMP nsPluginInstanceOwner::GetDocument(nsIDocument* *aDocument)
{
  if (!aDocument)
    return NS_ERROR_NULL_POINTER;

  *aDocument = nsnull;
  if (mOwner) {
    nsIPresShell *shell = mOwner->GetPresContext()->GetPresShell();
    if (shell)
      NS_IF_ADDREF(*aDocument = shell->GetDocument());
  }
  return NS_OK;
}

NS_IMETHODIMP nsPluginInstanceOwner::InvalidateRect(nsPluginRect *invalidRect)
{
  nsresult rv = NS_ERROR_FAILURE;

  if (mOwner && invalidRect) {
    //no reference count on view
    nsIView* view = mOwner->GetView();

    if (view) {
      float ptot;
      ptot = mOwner->GetPresContext()->PixelsToTwips();

      nsRect rect((int)(ptot * invalidRect->left),
            (int)(ptot * invalidRect->top),
            (int)(ptot * (invalidRect->right - invalidRect->left)),
            (int)(ptot * (invalidRect->bottom - invalidRect->top)));

      //set flags to not do a synchronous update, force update does the redraw
      view->GetViewManager()->UpdateView(view, rect, NS_VMREFRESH_NO_SYNC);
    }
  }

  return rv;
}

NS_IMETHODIMP nsPluginInstanceOwner::InvalidateRegion(nsPluginRegion invalidRegion)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP nsPluginInstanceOwner::ForceRedraw()
{
  nsIView* view = mOwner->GetView();
  if (view) {
    return view->GetViewManager()->Composite();
  }

  return NS_OK;
}

NS_IMETHODIMP nsPluginInstanceOwner::GetValue(nsPluginInstancePeerVariable variable, void *value)
{
  nsresult rv = NS_ERROR_FAILURE;

  switch(variable) {
    case nsPluginInstancePeerVariable_NetscapeWindow:
    {      
      // get the document's widget from the view manager
      // get the view manager from the pres shell, not from the view!
      // we may not have a view if we are hidden
      if (mOwner) {
        nsIViewManager* vm = mOwner->GetPresContext()->GetViewManager();
          if (vm) {
            nsCOMPtr<nsIWidget> widget;
            rv = vm->GetWidget(getter_AddRefs(widget));            
            if (widget) {

              void** pvalue = (void**)value;
              *pvalue = (void*)widget->GetNativeData(NS_NATIVE_WINDOW);

            } else NS_ERROR("couldn't get doc's widget in getting doc's window handle");
          } else NS_ERROR("couldn't get view manager in getting doc's window handle");
      } else NS_ERROR("plugin owner has no owner in getting doc's window handle");

      break;
    }
  }

  return rv;
}

NS_IMETHODIMP
nsPluginInstanceOwner::PluginNotAvailable(const char *aMimeType)
{
  return NS_OK;
}

NS_IMETHODIMP nsPluginInstanceOwner::GetTagType(nsPluginTagType *result)
{
  NS_ENSURE_ARG_POINTER(result);
  nsresult rv = NS_ERROR_FAILURE;

  *result = nsPluginTagType_Unknown;

  if (mOwner) {
    nsIContent* cont = mOwner->GetContent();
    if (cont) {
      nsIAtom *atom = cont->Tag();

      if (atom == nsHTMLAtoms::applet)
        *result = nsPluginTagType_Applet;
      else if (atom == nsHTMLAtoms::embed)
        *result = nsPluginTagType_Embed;
      else if (atom == nsHTMLAtoms::object)
        *result = nsPluginTagType_Object;

      rv = NS_OK;
    }
  }

  return rv;
}

NS_IMETHODIMP nsPluginInstanceOwner::GetTagText(const char* *result)
{
    NS_ENSURE_ARG_POINTER(result);
    if (nsnull == mTagText) {
        nsresult rv;
        nsIContent* content = mOwner->GetContent();

        nsCOMPtr<nsIDOMNode> node(do_QueryInterface(content, &rv));
        if (NS_FAILED(rv))
            return rv;
        nsCOMPtr<nsIDocument> document;
        rv = GetDocument(getter_AddRefs(document));
        if (NS_FAILED(rv))
            return rv;
        nsCOMPtr<nsIDocumentEncoder> docEncoder(do_CreateInstance(NS_DOC_ENCODER_CONTRACTID_BASE "text/html", &rv));
        if (NS_FAILED(rv))
            return rv;
        rv = docEncoder->Init(document, NS_LITERAL_STRING("text/html"), nsIDocumentEncoder::OutputEncodeBasicEntities);
        if (NS_FAILED(rv))
            return rv;

        nsCOMPtr<nsIDOMRange> range(do_CreateInstance(kRangeCID,&rv));
        if (NS_FAILED(rv))
            return rv;

        rv = range->SelectNode(node);
        if (NS_FAILED(rv))
            return rv;

        docEncoder->SetRange(range);
        nsString elementHTML;
        rv = docEncoder->EncodeToString(elementHTML);
        if (NS_FAILED(rv))
            return rv;

        mTagText = ToNewUTF8String(elementHTML);
        if (!mTagText)
            return NS_ERROR_OUT_OF_MEMORY;
    }
    *result = mTagText;
    return NS_OK;
}

NS_IMETHODIMP nsPluginInstanceOwner::GetParameters(PRUint16& n, const char*const*& names, const char*const*& values)
{
  nsresult rv = EnsureCachedAttrParamArrays();
  NS_ENSURE_SUCCESS(rv, rv);

  n = mNumCachedParams;
  if (n) {
    names  = (const char **)(mCachedAttrParamNames + mNumCachedAttrs + 1);
    values = (const char **)(mCachedAttrParamValues + mNumCachedAttrs + 1);
  } else
    names = values = nsnull;

  return rv;
}

NS_IMETHODIMP nsPluginInstanceOwner::GetParameter(const char* name, const char* *result)
{
  NS_ENSURE_ARG_POINTER(name);
  NS_ENSURE_ARG_POINTER(result);
  
  nsresult rv = EnsureCachedAttrParamArrays();
  NS_ENSURE_SUCCESS(rv, rv);

  *result = nsnull;

  for (int i = mNumCachedAttrs + 1; i < (mNumCachedParams + 1 + mNumCachedAttrs); i++) {
    if (0 == PL_strcasecmp(mCachedAttrParamNames[i], name)) {
      *result = mCachedAttrParamValues[i];
      return NS_OK;
    }
  }

  return NS_ERROR_FAILURE;
}
  
NS_IMETHODIMP nsPluginInstanceOwner::GetDocumentBase(const char* *result)
{
  NS_ENSURE_ARG_POINTER(result);
  nsresult rv = NS_OK;
  if (mDocumentBase.IsEmpty()) {
    if (!mOwner) {
      *result = nsnull;
      return NS_ERROR_FAILURE;
    }

    nsIPresShell* shell = mOwner->GetPresContext()->PresShell();
    rv = NS_ERROR_FAILURE;
    if (shell) {
      nsIDocument* doc = shell->GetDocument();
      if (doc) {
        rv = doc->GetBaseURI()->GetSpec(mDocumentBase);
      }
    }
  }
  if (NS_SUCCEEDED(rv))
    *result = ToNewCString(mDocumentBase);
  return rv;
}

static nsHashtable *gCharsetMap = nsnull;
typedef struct {
    char mozName[16];
    char javaName[12];
} moz2javaCharset;

/* XXX If you add any strings longer than
 *  {"x-mac-cyrillic",  "MacCyrillic"},
 *  {"x-mac-ukrainian", "MacUkraine"},
 * to the following array then you MUST update the
 * sizes of the arrays in the moz2javaCharset struct
 */

static const moz2javaCharset charsets[] = 
{
    {"windows-1252",    "Cp1252"},
    {"IBM850",          "Cp850"},
    {"IBM852",          "Cp852"},
    {"IBM855",          "Cp855"},
    {"IBM857",          "Cp857"},
    {"IBM828",          "Cp862"},
    {"IBM864",          "Cp864"},
    {"IBM866",          "Cp866"},
    {"windows-1250",    "Cp1250"},
    {"windows-1251",    "Cp1251"},
    {"windows-1253",    "Cp1253"},
    {"windows-1254",    "Cp1254"},
    {"windows-1255",    "Cp1255"},
    {"windows-1256",    "Cp1256"},
    {"windows-1257",    "Cp1257"},
    {"windows-1258",    "Cp1258"},
    {"EUC-JP",          "EUC_JP"},
    {"EUC-KR",          "EUC_KR"},
    {"x-euc-tw",        "EUC_TW"},
    {"gb18030",         "GB18030"},
    {"x-gbk",           "GBK"},
    {"ISO-2022-JP",     "ISO2022JP"},
    {"ISO-2022-KR",     "ISO2022KR"},
    {"ISO-8859-2",      "ISO8859_2"},
    {"ISO-8859-3",      "ISO8859_3"},
    {"ISO-8859-4",      "ISO8859_4"},
    {"ISO-8859-5",      "ISO8859_5"},
    {"ISO-8859-6",      "ISO8859_6"},
    {"ISO-8859-7",      "ISO8859_7"},
    {"ISO-8859-8",      "ISO8859_8"},
    {"ISO-8859-9",      "ISO8859_9"},
    {"ISO-8859-13",     "ISO8859_13"},
    {"x-johab",         "Johab"},
    {"KOI8-R",          "KOI8_R"},
    {"TIS-620",         "MS874"},
    {"windows-936",     "MS936"},
    {"x-windows-949",   "MS949"},
    {"x-mac-arabic",    "MacArabic"},
    {"x-mac-croatian",  "MacCroatia"},
    {"x-mac-cyrillic",  "MacCyrillic"},
    {"x-mac-greek",     "MacGreek"},
    {"x-mac-hebrew",    "MacHebrew"},
    {"x-mac-icelandic", "MacIceland"},
    {"x-mac-roman",     "MacRoman"},
    {"x-mac-romanian",  "MacRomania"},
    {"x-mac-ukrainian", "MacUkraine"},
    {"Shift_JIS",       "SJIS"},
    {"TIS-620",         "TIS620"}
};
  
NS_IMETHODIMP nsPluginInstanceOwner::GetDocumentEncoding(const char* *result)
{
  NS_ENSURE_ARG_POINTER(result);
  *result = nsnull;

  nsresult rv;
  nsCOMPtr<nsIDocument> doc;
  rv = GetDocument(getter_AddRefs(doc));
  NS_ASSERTION(NS_SUCCEEDED(rv), "failed to get document");
  if (NS_FAILED(rv))
    return rv;

  const nsACString &charset = doc->GetDocumentCharacterSet();

  if (charset.IsEmpty())
    return NS_OK;

  // common charsets and those not requiring conversion first
  if (charset.EqualsLiteral("us-ascii")) {
    *result = PL_strdup("US_ASCII");
  } else if (charset.EqualsLiteral("ISO-8859-1") ||
      !nsCRT::strncmp(PromiseFlatCString(charset).get(), "UTF", 3)) {
    *result = ToNewCString(charset);
  } else {
    if (!gCharsetMap) {
      gCharsetMap = new nsHashtable(sizeof(charsets)/sizeof(moz2javaCharset));
      if (!gCharsetMap)
        return NS_ERROR_OUT_OF_MEMORY;

      for (PRUint16 i = 0; i < sizeof(charsets)/sizeof(moz2javaCharset); i++) {
        nsCStringKey key(charsets[i].mozName);
        gCharsetMap->Put(&key, (void *)(charsets[i].javaName));
      }
    }
    nsCStringKey mozKey(charset);
    // if found mapping, return it; otherwise return original charset
    char *mapping = (char *)gCharsetMap->Get(&mozKey);
    *result = mapping ? PL_strdup(mapping) : ToNewCString(charset);
  }

  return (*result) ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
}

NS_IMETHODIMP nsPluginInstanceOwner::GetAlignment(const char* *result)
{
  return GetAttribute("ALIGN", result);
}
  
NS_IMETHODIMP nsPluginInstanceOwner::GetWidth(PRUint32 *result)
{
  NS_ENSURE_ARG_POINTER(result);

  NS_ENSURE_TRUE(mPluginWindow, NS_ERROR_NULL_POINTER);

  *result = mPluginWindow->width;

  return NS_OK;
}
  
NS_IMETHODIMP nsPluginInstanceOwner::GetHeight(PRUint32 *result)
{
  NS_ENSURE_ARG_POINTER(result);

  NS_ENSURE_TRUE(mPluginWindow, NS_ERROR_NULL_POINTER);

  *result = mPluginWindow->height;

  return NS_OK;
}

  
NS_IMETHODIMP nsPluginInstanceOwner::GetBorderVertSpace(PRUint32 *result)
{
  nsresult    rv;
  const char  *vspace;

  rv = GetAttribute("VSPACE", &vspace);

  if (NS_OK == rv) {
    if (*result != 0)
      *result = (PRUint32)atol(vspace);
    else
      *result = 0;
  }
  else
    *result = 0;

  return rv;
}
  
NS_IMETHODIMP nsPluginInstanceOwner::GetBorderHorizSpace(PRUint32 *result)
{
  nsresult    rv;
  const char  *hspace;

  rv = GetAttribute("HSPACE", &hspace);

  if (NS_OK == rv) {
    if (*result != 0)
      *result = (PRUint32)atol(hspace);
    else
      *result = 0;
  }
  else
    *result = 0;

  return rv;
}

NS_IMETHODIMP nsPluginInstanceOwner::GetUniqueID(PRUint32 *result)
{
  NS_ENSURE_ARG_POINTER(result);
  *result = NS_PTR_TO_INT32(mOwner);
  return NS_OK;
}

#ifdef OJI
NS_IMETHODIMP nsPluginInstanceOwner::GetCode(const char* *result)
{
  nsresult rv;
  nsPluginTagType tagType;  
  NS_ENSURE_SUCCESS(rv = GetTagType(&tagType), rv);

  rv = NS_ERROR_FAILURE;
  if (nsPluginTagType_Object != tagType)
    rv = GetAttribute("CODE", result);
  if (NS_FAILED(rv))
    rv = GetParameter("CODE", result);

  return rv;
}

NS_IMETHODIMP nsPluginInstanceOwner::GetCodeBase(const char* *result)
{
  nsresult rv;
  if (NS_FAILED(rv = GetAttribute("CODEBASE", result)))
    rv = GetParameter("CODEBASE", result);
  return rv;
}

NS_IMETHODIMP nsPluginInstanceOwner::GetArchive(const char* *result)
{
  nsresult rv;
  if (NS_FAILED(rv = GetAttribute("ARCHIVE", result)))
    rv = GetParameter("ARCHIVE", result);
  return rv;
}

NS_IMETHODIMP nsPluginInstanceOwner::GetName(const char* *result)
{
  nsresult rv;
  nsPluginTagType tagType;  
  NS_ENSURE_SUCCESS(rv = GetTagType(&tagType), rv);

  rv = NS_ERROR_FAILURE;
  if (nsPluginTagType_Object != tagType)
    rv = GetAttribute("NAME", result);
  if (NS_FAILED(rv))
    rv = GetParameter("NAME", result);

  return rv;
}

NS_IMETHODIMP nsPluginInstanceOwner::GetMayScript(PRBool *result)
{
  NS_ENSURE_ARG_POINTER(result);
  nsPluginTagType tagType;  
  NS_ENSURE_SUCCESS(GetTagType(&tagType), NS_ERROR_FAILURE);

  const char* unused;
  if (nsPluginTagType_Object == tagType)
    *result = NS_SUCCEEDED(GetParameter("MAYSCRIPT", &unused)); 
  else
    *result = NS_SUCCEEDED(GetAttribute("MAYSCRIPT", &unused));

  return NS_OK;
}
#endif /* OJI */

// Little helper function to resolve relative URL in
// |value| for certain inputs of |name|
void nsObjectFrame::FixUpURLS(const nsString &name, nsAString &value)
{
  if (name.LowerCaseEqualsLiteral("pluginurl") ||
      name.LowerCaseEqualsLiteral("pluginspage")) {        
    
    nsCOMPtr<nsIURI> baseURI = mContent->GetBaseURI();
    nsAutoString newURL;
    NS_MakeAbsoluteURI(newURL, value, baseURI);
    if (!newURL.IsEmpty())
      value = newURL;
  }
}

// Cache the attributes and/or parameters of our tag into a single set
// of arrays to be compatible with 4.x. The attributes go first,
// followed by a PARAM/null and then any PARAM tags. Also, hold the
// cached array around for the duration of the life of the instance
// because 4.x did. See bug 111008.

nsresult nsPluginInstanceOwner::EnsureCachedAttrParamArrays()
{
  if (mCachedAttrParamValues)
    return NS_OK;

  NS_PRECONDITION(((mNumCachedAttrs + mNumCachedParams) == 0) &&
                  !mCachedAttrParamNames,
                  "re-cache of attrs/params not implemented! use the DOM "
                  "node directy instead");
  NS_ENSURE_TRUE(mOwner, NS_ERROR_NULL_POINTER);

  // first, we need to find out how much we need to allocate for our
  // arrays count up attributes
  mNumCachedAttrs = 0;

  nsIContent* content = mOwner->GetContent();
  nsresult rv = NS_OK;
  NS_ENSURE_TRUE(content, rv);

  PRUint32 cattrs = content->GetAttrCount();

  if (cattrs < 0x0000FFFF) {
    // unsigned 32 bits to unsigned 16 bits conversion
    mNumCachedAttrs = NS_STATIC_CAST(PRUint16, cattrs);
  } else {
    mNumCachedAttrs = 0xFFFE;  // minus one in case we add an extra "src" entry below
  }

  // now, we need to find all the PARAM tags that are children of us
  // however, be carefull NOT to include any PARAMs that don't have us
  // as a direct parent. For nested object (or applet) tags, be sure
  // to only round up the param tags that coorespond with THIS
  // instance. And also, weed out any bogus tags that may get in the
  // way, see bug 39609. Then, with any param tag that meet our
  // qualification, temporarly cache them in an nsISupportsArray until
  // we can figure out what size to make our fixed char* array.

  mNumCachedParams = 0;
  nsCOMPtr<nsISupportsArray> ourParams;
  rv = NS_NewISupportsArray(getter_AddRefs(ourParams));
  NS_ENSURE_SUCCESS(rv, rv);
 
  // use the DOM to get us ALL our dependent PARAM tags, even if not
  // ours
  nsCOMPtr<nsIDOMElement> mydomElement = do_QueryInterface(content);
  NS_ENSURE_TRUE(mydomElement, NS_ERROR_NO_INTERFACE);

  nsCOMPtr<nsIDOMNodeList> allParams; 

  nsINodeInfo *ni = content->NodeInfo();

  // Making DOM method calls can cause our frame to go away, which
  // might kill us...
  nsCOMPtr<nsIPluginInstanceOwner> kungFuDeathGrip(this);
  
  if (ni->NamespaceEquals(kNameSpaceID_XHTML)) {
    // For XHTML elements we need to take the namespace URI into
    // account when looking for param elements.

    NS_NAMED_LITERAL_STRING(xhtml_ns, "http://www.w3.org/1999/xhtml");

    mydomElement->GetElementsByTagNameNS(xhtml_ns, NS_LITERAL_STRING("param"),
                                         getter_AddRefs(allParams));
  } else {
    // If content is not XHTML, it must be HTML, no need to worry
    // about namespaces then...

    mydomElement->GetElementsByTagName(NS_LITERAL_STRING("param"),
                                       getter_AddRefs(allParams));
  }    

  if (allParams) {
    PRUint32 numAllParams; 
    allParams->GetLength(&numAllParams);
    // loop through every so called dependent PARAM tag to check if it
    // "belongs" to us

    for (PRUint32 i = 0; i < numAllParams; i++) {
      nsCOMPtr<nsIDOMNode> pnode;
      allParams->Item(i, getter_AddRefs(pnode));

      nsCOMPtr<nsIDOMElement> domelement = do_QueryInterface(pnode);
      if (domelement) {
        // let's NOT count up param tags that don't have a name attribute
        nsAutoString name;
        domelement->GetAttribute(NS_LITERAL_STRING("name"), name);
        if (!name.IsEmpty()) {
          nsCOMPtr<nsIDOMNode> parent;
          nsCOMPtr<nsIDOMHTMLObjectElement> domobject;
          nsCOMPtr<nsIDOMHTMLAppletElement> domapplet;
          pnode->GetParentNode(getter_AddRefs(parent));
          // walk up the parents of this PARAM until we find an object
          // (or applet) tag

          while (!(domobject || domapplet) && parent) {
            domobject = do_QueryInterface(parent);
            domapplet = do_QueryInterface(parent);
            nsCOMPtr<nsIDOMNode> temp;
            parent->GetParentNode(getter_AddRefs(temp));
            parent = temp;
          }

          if (domapplet || domobject) {
            if (domapplet)
              parent = do_QueryInterface(domapplet);
            else
              parent = do_QueryInterface(domobject);

            // now check to see if this PARAM's parent is us. if so,
            // cache it for later

            nsCOMPtr<nsIDOMNode> mydomNode = do_QueryInterface(mydomElement);
            if (parent == mydomNode) {
              ourParams->AppendElement(pnode);
            }
          }
        }
      }
    }
  }

  // We're done with DOM method calls now; make sure we still have a frame.
  NS_ENSURE_TRUE(mOwner, NS_ERROR_OUT_OF_MEMORY);

  PRUint32 cparams;
  ourParams->Count(&cparams); // unsigned 32 bits to unsigned 16 bits conversion
  if (cparams < 0x0000FFFF)
    mNumCachedParams = NS_STATIC_CAST(PRUint16, cparams);
  else 
    mNumCachedParams = 0xFFFF;

  // Some plugins were never written to understand the "data" attribute of the OBJECT tag.
  // Real and WMP will not play unless they find a "src" attribute, see bug 152334.
  // Nav 4.x would simply replace the "data" with "src". Because some plugins correctly
  // look for "data", lets instead copy the "data" attribute and add another entry
  // to the bottom of the array if there isn't already a "src" specified.
  PRInt16 numRealAttrs = mNumCachedAttrs;
  nsAutoString data;
  if (content->Tag() == nsHTMLAtoms::object
    && !content->HasAttr(kNameSpaceID_None, nsHTMLAtoms::src)
    && content->GetAttr(kNameSpaceID_None, nsHTMLAtoms::data, data)) {
      mNumCachedAttrs++;
  }

  // now lets make the arrays
  mCachedAttrParamNames  = (char **)PR_Calloc(sizeof(char *) * (mNumCachedAttrs + 1 + mNumCachedParams), 1);
  NS_ENSURE_TRUE(mCachedAttrParamNames,  NS_ERROR_OUT_OF_MEMORY);
  mCachedAttrParamValues = (char **)PR_Calloc(sizeof(char *) * (mNumCachedAttrs + 1 + mNumCachedParams), 1);
  NS_ENSURE_TRUE(mCachedAttrParamValues, NS_ERROR_OUT_OF_MEMORY);

  // let's fill in our attributes
  PRInt16 c = 0;

  // Some plugins (eg Flash, see bug 234675.) are actually sensitive to the
  // attribute order.  So we want to make sure we give the plugin the
  // attributes in the order they came in in the source, to be compatible with
  // other browsers.  Now in HTML, the storage order is the reverse of the
  // source order, while in XML and XHTML it's the same as the source order
  // (see the AddAttributes functions in the HTML and XML content sinks).
  PRInt16 start, end, increment;
  if (content->IsContentOfType(nsIContent::eHTML) &&
      content->NodeInfo()->NamespaceEquals(kNameSpaceID_None)) {
    // HTML.  Walk attributes in reverse order.
    start = numRealAttrs - 1;
    end = -1;
    increment = -1;
  } else {
    // XHTML or XML.  Walk attributes in forward order.
    start = 0;
    end = numRealAttrs;
    increment = 1;
  }
  for (PRInt16 index = start; index != end; index += increment) {
    const nsAttrName* attrName = content->GetAttrNameAt(index);
    nsIAtom* atom = attrName->LocalName();
    nsAutoString value;
    content->GetAttr(attrName->NamespaceID(), atom, value);
    nsAutoString name;
    atom->ToString(name);

    mOwner->FixUpURLS(name, value);

    mCachedAttrParamNames [c] = ToNewUTF8String(name);
    mCachedAttrParamValues[c] = ToNewUTF8String(value);
    c++;
  }

  // if the conditions above were met, copy the "data" attribute to a "src" array entry
  if (data.Length()) {
    mCachedAttrParamNames [mNumCachedAttrs-1] = ToNewUTF8String(NS_LITERAL_STRING("SRC"));
    mCachedAttrParamValues[mNumCachedAttrs-1] = ToNewUTF8String(data);
  }

  // add our PARAM and null separator
  mCachedAttrParamNames [mNumCachedAttrs] = ToNewUTF8String(NS_LITERAL_STRING("PARAM"));
  mCachedAttrParamValues[mNumCachedAttrs] = nsnull;

  // now fill in the PARAM name/value pairs from the cached DOM nodes
  c = 0;
  for (PRInt16 idx = 0; idx < mNumCachedParams; idx++) {
    nsCOMPtr<nsIDOMElement> param = do_QueryElementAt(ourParams, idx);
    if (param) {
     nsAutoString name;
     nsAutoString value;
     param->GetAttribute(NS_LITERAL_STRING("name"), name); // check for empty done above
     param->GetAttribute(NS_LITERAL_STRING("value"), value);

     mOwner->FixUpURLS(name, value);

     /*
      * According to the HTML 4.01 spec, at
      * http://www.w3.org/TR/html4/types.html#type-cdata
      * ''User agents may ignore leading and trailing
      * white space in CDATA attribute values (e.g., "
      * myval " may be interpreted as "myval"). Authors
      * should not declare attribute values with
      * leading or trailing white space.''
      * However, do not trim consecutive spaces as in bug 122119
      */            
     name.Trim(" \n\r\t\b", PR_TRUE, PR_TRUE, PR_FALSE);
     value.Trim(" \n\r\t\b", PR_TRUE, PR_TRUE, PR_FALSE);
     mCachedAttrParamNames [mNumCachedAttrs + 1 + c] = ToNewUTF8String(name);
     mCachedAttrParamValues[mNumCachedAttrs + 1 + c] = ToNewUTF8String(value);
     c++;                                                      // rules!
    }
  }

  return NS_OK;
}


// Here's where we forward events to plugins.

#ifdef XP_MACOSX

static void InitializeEventRecord(EventRecord* event)
{
    memset(event, 0, sizeof(EventRecord));
    ::GetGlobalMouse(&event->where);
    event->when = ::TickCount();
    event->modifiers = ::GetCurrentKeyModifiers();
}

void nsPluginInstanceOwner::GUItoMacEvent(const nsGUIEvent& anEvent, EventRecord* origEvent, EventRecord& aMacEvent)
{
  nsPresContext* presContext = mOwner ? mOwner->GetPresContext() : nsnull;
  InitializeEventRecord(&aMacEvent);
  switch (anEvent.message) {
    case NS_FOCUS_EVENT_START:   // this is the same as NS_FOCUS_CONTENT
        aMacEvent.what = nsPluginEventType_GetFocusEvent;
        if (presContext) {
            nsIContent* content = mOwner->GetContent();
            if (content)
                content->SetFocus(presContext);
        }
        break;

    case NS_BLUR_CONTENT:
        aMacEvent.what = nsPluginEventType_LoseFocusEvent;
        if (presContext) {
            nsIContent* content = mOwner->GetContent();
            if (content)
                content->RemoveFocus(presContext);
        }
        break;

    case NS_MOUSE_MOVE:
    case NS_MOUSE_ENTER:
        if (origEvent)
          aMacEvent = *origEvent;
        aMacEvent.what = nsPluginEventType_AdjustCursorEvent;
        break;
  }
}

#endif

nsresult nsPluginInstanceOwner::ScrollPositionWillChange(nsIScrollableView* aScrollable, nscoord aX, nscoord aY)
{
#ifdef XP_MACOSX
    CancelTimer();

    if (mInstance) {
        nsCOMPtr<nsIPluginWidget> pluginWidget = do_QueryInterface(mWidget);
        if (pluginWidget && NS_SUCCEEDED(pluginWidget->StartDrawPlugin())) {
            EventRecord scrollEvent;
            InitializeEventRecord(&scrollEvent);
            scrollEvent.what = nsPluginEventType_ScrollingBeginsEvent;
    
            nsPluginPort* pluginPort = FixUpPluginWindow(ePluginPaintDisable);
            if (pluginPort) {
              nsPluginEvent pluginEvent = { &scrollEvent, nsPluginPlatformWindowRef(GetWindowFromPort(pluginPort->port)) };
            
              PRBool eventHandled = PR_FALSE;
              mInstance->HandleEvent(&pluginEvent, &eventHandled);
            }
            pluginWidget->EndDrawPlugin();
        }
    }
#endif
    return NS_OK;
}

nsresult nsPluginInstanceOwner::ScrollPositionDidChange(nsIScrollableView* aScrollable, nscoord aX, nscoord aY)
{
#ifdef XP_MACOSX
    if (mInstance) {
      nsCOMPtr<nsIPluginWidget> pluginWidget = do_QueryInterface(mWidget);
      if (pluginWidget && NS_SUCCEEDED(pluginWidget->StartDrawPlugin())) {
        EventRecord scrollEvent;
        InitializeEventRecord(&scrollEvent);
        scrollEvent.what = nsPluginEventType_ScrollingEndsEvent;
  
        nsPluginPort* pluginPort = FixUpPluginWindow(ePluginPaintEnable);
        if (pluginPort) {
          nsPluginEvent pluginEvent =
            { &scrollEvent,
              nsPluginPlatformWindowRef(GetWindowFromPort(pluginPort->port))
            };

          PRBool eventHandled = PR_FALSE;
          mInstance->HandleEvent(&pluginEvent, &eventHandled);
        }
        pluginWidget->EndDrawPlugin();
      }

      // FIXME - Only invalidate the newly revealed amount.
      // XXX necessary?
      if (mWidget)
        mWidget->Invalidate(PR_TRUE);
    }
#endif

    StartTimer();
    return NS_OK;
}

/*=============== nsIFocusListener ======================*/
nsresult nsPluginInstanceOwner::Focus(nsIDOMEvent * aFocusEvent)
{
  mContentFocused = PR_TRUE;
  return DispatchFocusToPlugin(aFocusEvent);
}

nsresult nsPluginInstanceOwner::Blur(nsIDOMEvent * aFocusEvent)
{
  mContentFocused = PR_FALSE;
  return DispatchFocusToPlugin(aFocusEvent);
}

nsresult nsPluginInstanceOwner::DispatchFocusToPlugin(nsIDOMEvent* aFocusEvent)
{
#ifndef XP_MACOSX
  if (!mPluginWindow || nsPluginWindowType_Window == mPluginWindow->type) {
    // continue only for cases without child window
    return aFocusEvent->PreventDefault(); // consume event
  }
#endif

  nsCOMPtr<nsIPrivateDOMEvent> privateEvent(do_QueryInterface(aFocusEvent));
  if (privateEvent) {
    nsEvent * theEvent;
    privateEvent->GetInternalNSEvent(&theEvent);
    if (theEvent) {
      // we only care about the message in ProcessEvent
      nsGUIEvent focusEvent(NS_IS_TRUSTED_EVENT(theEvent), theEvent->message,
                            nsnull);
      nsEventStatus rv = ProcessEvent(focusEvent);
      if (nsEventStatus_eConsumeNoDefault == rv) {
        aFocusEvent->PreventDefault();

        nsCOMPtr<nsIDOMNSEvent> nsevent(do_QueryInterface(aFocusEvent));

        if (nsevent) {
          nsevent->PreventBubble();
        }
      }
    }
    else NS_ASSERTION(PR_FALSE, "nsPluginInstanceOwner::DispatchFocusToPlugin failed, focusEvent null");   
  }
  else NS_ASSERTION(PR_FALSE, "nsPluginInstanceOwner::DispatchFocusToPlugin failed, privateEvent null");   
  
  return NS_OK;
}    

/*=============== nsIDOMDragListener ======================*/
nsresult nsPluginInstanceOwner::DragEnter(nsIDOMEvent* aMouseEvent)
{
  if (mInstance) {
    // Let the plugin handle drag events.
    aMouseEvent->PreventDefault();
    nsCOMPtr<nsIDOMNSEvent> nsevent(do_QueryInterface(aMouseEvent));
    if (nsevent) {
      nsevent->PreventBubble();
    }
  }

  return NS_OK;
}

nsresult nsPluginInstanceOwner::DragOver(nsIDOMEvent* aMouseEvent)
{
  if (mInstance) {
    // Let the plugin handle drag events.
    aMouseEvent->PreventDefault();
    nsCOMPtr<nsIDOMNSEvent> nsevent(do_QueryInterface(aMouseEvent));
    if (nsevent) {
      nsevent->PreventBubble();
    }
  }

  return NS_OK;
}

nsresult nsPluginInstanceOwner::DragExit(nsIDOMEvent* aMouseEvent)
{
  if (mInstance) {
    // Let the plugin handle drag events.
    aMouseEvent->PreventDefault();
    nsCOMPtr<nsIDOMNSEvent> nsevent(do_QueryInterface(aMouseEvent));
    if (nsevent) {
      nsevent->PreventBubble();
    }
  }

  return NS_OK;
}

nsresult nsPluginInstanceOwner::DragDrop(nsIDOMEvent* aMouseEvent)
{
  if (mInstance) {
    // Let the plugin handle drag events.
    aMouseEvent->PreventDefault();
    nsCOMPtr<nsIDOMNSEvent> nsevent(do_QueryInterface(aMouseEvent));
    if (nsevent) {
      nsevent->PreventBubble();
    }
  }

  return NS_OK;
}

nsresult nsPluginInstanceOwner::DragGesture(nsIDOMEvent* aMouseEvent)
{
  if (mInstance) {
    // Let the plugin handle drag events.
    aMouseEvent->PreventDefault();
    nsCOMPtr<nsIDOMNSEvent> nsevent(do_QueryInterface(aMouseEvent));
    if (nsevent) {
      nsevent->PreventBubble();
    }
  }

  return NS_OK;
}



/*=============== nsIKeyListener ======================*/
nsresult nsPluginInstanceOwner::KeyDown(nsIDOMEvent* aKeyEvent)
{
  return DispatchKeyToPlugin(aKeyEvent);
}

nsresult nsPluginInstanceOwner::KeyUp(nsIDOMEvent* aKeyEvent)
{
  return DispatchKeyToPlugin(aKeyEvent);
}

nsresult nsPluginInstanceOwner::KeyPress(nsIDOMEvent* aKeyEvent)
{
#ifdef XP_MACOSX // send KeyPress events only on Mac

  // KeyPress events are really synthesized keyDown events.
  // Here we check the native message of the event so that
  // we won't send the plugin two keyDown events.
  nsCOMPtr<nsIPrivateDOMEvent> privateEvent(do_QueryInterface(aKeyEvent));
  if (privateEvent) {
    nsEvent *theEvent;
    privateEvent->GetInternalNSEvent(&theEvent);
    const nsGUIEvent *guiEvent = (nsGUIEvent*)theEvent;
    const EventRecord *ev = (EventRecord*)(guiEvent->nativeMsg); 
    if (guiEvent &&
        guiEvent->message == NS_KEY_PRESS &&
        ev &&
        ev->what == keyDown)
      return aKeyEvent->PreventDefault(); // consume event
  }

  // Nasty hack to avoid recursive event dispatching with Java. Java can
  // dispatch key events to a TSM handler, which comes back and calls 
  // [ChildView insertText:] on the cocoa widget, which sends a key
  // event back down.
  static PRBool sInKeyDispatch = PR_FALSE;
  
  if (sInKeyDispatch)
    return aKeyEvent->PreventDefault(); // consume event

  sInKeyDispatch = PR_TRUE;
  nsresult rv =  DispatchKeyToPlugin(aKeyEvent);
  sInKeyDispatch = PR_FALSE;
  return rv;
#else
  if (mInstance) {
    // If this event is going to the plugin, we want to kill it.
    // Not actually sending keypress to the plugin, since we didn't before.
    aKeyEvent->PreventDefault();
    nsCOMPtr<nsIDOMNSEvent> nsevent(do_QueryInterface(aKeyEvent));
    if (nsevent) {
      nsevent->PreventBubble();
    }
  }
  return NS_OK;
#endif
}

nsresult nsPluginInstanceOwner::DispatchKeyToPlugin(nsIDOMEvent* aKeyEvent)
{
#ifndef XP_MACOSX
  if (!mPluginWindow || nsPluginWindowType_Window == mPluginWindow->type)
    return aKeyEvent->PreventDefault(); // consume event
  // continue only for cases without child window
#endif

  if (mInstance) {
    nsCOMPtr<nsIPrivateDOMEvent> privateEvent(do_QueryInterface(aKeyEvent));
    if (privateEvent) {
      nsKeyEvent* keyEvent = nsnull;
      privateEvent->GetInternalNSEvent((nsEvent**)&keyEvent);
      if (keyEvent) {
        nsEventStatus rv = ProcessEvent(*keyEvent);
        if (nsEventStatus_eConsumeNoDefault == rv) {
          aKeyEvent->PreventDefault();

          nsCOMPtr<nsIDOMNSEvent> nsevent(do_QueryInterface(aKeyEvent));

          if (nsevent) {
            nsevent->PreventBubble();
          }
        }
      }
      else NS_ASSERTION(PR_FALSE, "nsPluginInstanceOwner::DispatchKeyToPlugin failed, keyEvent null");   
    }
    else NS_ASSERTION(PR_FALSE, "nsPluginInstanceOwner::DispatchKeyToPlugin failed, privateEvent null");   
  }

  return NS_OK;
}    

/*=============== nsIMouseMotionListener ======================*/

nsresult
nsPluginInstanceOwner::MouseMove(nsIDOMEvent* aMouseEvent)
{
#if !defined(XP_MACOSX)
  if (!mPluginWindow || nsPluginWindowType_Window == mPluginWindow->type)
    return aMouseEvent->PreventDefault(); // consume event
  // continue only for cases without child window
#endif

  // don't send mouse events if we are hiddden
  if (!mWidgetVisible)
    return NS_OK;

  nsCOMPtr<nsIPrivateDOMEvent> privateEvent(do_QueryInterface(aMouseEvent));
  if (privateEvent) {
    nsMouseEvent* mouseEvent = nsnull;
    privateEvent->GetInternalNSEvent((nsEvent**)&mouseEvent);
    if (mouseEvent) {
      nsEventStatus rv = ProcessEvent(*mouseEvent);
      if (nsEventStatus_eConsumeNoDefault == rv) {
        return aMouseEvent->PreventDefault(); // consume event
      }
    }
    else NS_ASSERTION(PR_FALSE, "nsPluginInstanceOwner::MouseMove failed, mouseEvent null");   
  }
  else NS_ASSERTION(PR_FALSE, "nsPluginInstanceOwner::MouseMove failed, privateEvent null");   
  
  return NS_OK;
}

/*=============== nsIMouseListener ======================*/

nsresult
nsPluginInstanceOwner::MouseDown(nsIDOMEvent* aMouseEvent)
{
#if !defined(XP_MACOSX)
  if (!mPluginWindow || nsPluginWindowType_Window == mPluginWindow->type)
    return aMouseEvent->PreventDefault(); // consume event
  // continue only for cases without child window
#endif

  // if the plugin is windowless, we need to set focus ourselves
  // otherwise, we might not get key events
  if (mOwner && mPluginWindow &&
      mPluginWindow->type == nsPluginWindowType_Drawable) {
    nsIContent* content = mOwner->GetContent();
    if (content)
      content->SetFocus(mOwner->GetPresContext());
  }

  nsCOMPtr<nsIPrivateDOMEvent> privateEvent(do_QueryInterface(aMouseEvent));
  if (privateEvent) {
    nsMouseEvent* mouseEvent = nsnull;
    privateEvent->GetInternalNSEvent((nsEvent**)&mouseEvent);
    if (mouseEvent) {
      nsEventStatus rv = ProcessEvent(*mouseEvent);
      if (nsEventStatus_eConsumeNoDefault == rv) {
        return aMouseEvent->PreventDefault(); // consume event
      }
    }
    else NS_ASSERTION(PR_FALSE, "nsPluginInstanceOwner::MouseDown failed, mouseEvent null");   
  }
  else NS_ASSERTION(PR_FALSE, "nsPluginInstanceOwner::MouseDown failed, privateEvent null");   
  
  return NS_OK;
}

nsresult
nsPluginInstanceOwner::MouseUp(nsIDOMEvent* aMouseEvent)
{
  return DispatchMouseToPlugin(aMouseEvent);
}

nsresult
nsPluginInstanceOwner::MouseClick(nsIDOMEvent* aMouseEvent)
{
  return DispatchMouseToPlugin(aMouseEvent);
}

nsresult
nsPluginInstanceOwner::MouseDblClick(nsIDOMEvent* aMouseEvent)
{
  return DispatchMouseToPlugin(aMouseEvent);
}

nsresult
nsPluginInstanceOwner::MouseOver(nsIDOMEvent* aMouseEvent)
{
  return DispatchMouseToPlugin(aMouseEvent);
}

nsresult
nsPluginInstanceOwner::MouseOut(nsIDOMEvent* aMouseEvent)
{
  return DispatchMouseToPlugin(aMouseEvent);
}

nsresult nsPluginInstanceOwner::DispatchMouseToPlugin(nsIDOMEvent* aMouseEvent)
{
#if !defined(XP_MACOSX)
  if (!mPluginWindow || nsPluginWindowType_Window == mPluginWindow->type)
    return aMouseEvent->PreventDefault(); // consume event
  // continue only for cases without child window
#endif

  // don't send mouse events if we are hiddden
  if (!mWidgetVisible)
    return NS_OK;

  nsCOMPtr<nsIPrivateDOMEvent> privateEvent(do_QueryInterface(aMouseEvent));
  if (privateEvent) {
    nsMouseEvent* mouseEvent = nsnull;
    privateEvent->GetInternalNSEvent((nsEvent**)&mouseEvent);
    if (mouseEvent) {
      nsEventStatus rv = ProcessEvent(*mouseEvent);
      if (nsEventStatus_eConsumeNoDefault == rv) {
        aMouseEvent->PreventDefault();

        nsCOMPtr<nsIDOMNSEvent> nsevent(do_QueryInterface(aMouseEvent));

        if (nsevent) {
          nsevent->PreventBubble();
        }
      }
    }
    else NS_ASSERTION(PR_FALSE, "nsPluginInstanceOwner::DispatchMouseToPlugin failed, mouseEvent null");   
  }
  else NS_ASSERTION(PR_FALSE, "nsPluginInstanceOwner::DispatchMouseToPlugin failed, privateEvent null");   
  
  return NS_OK;
}

nsresult
nsPluginInstanceOwner::HandleEvent(nsIDOMEvent* aEvent)
{
  return NS_OK;
}


nsEventStatus nsPluginInstanceOwner::ProcessEvent(const nsGUIEvent& anEvent)
{
  // printf("nsGUIEvent.message: %d\n", anEvent.message);
  nsEventStatus rv = nsEventStatus_eIgnore;
  if (!mInstance)   // if mInstance is null, we shouldn't be here
    return rv;

#ifdef XP_MACOSX
  // check for null mWidget
  if (mWidget) {
    nsCOMPtr<nsIPluginWidget> pluginWidget = do_QueryInterface(mWidget);
    if (pluginWidget && NS_SUCCEEDED(pluginWidget->StartDrawPlugin())) {
      EventRecord macEvent;
      EventRecord* event = (EventRecord*)anEvent.nativeMsg;
      if ((event == NULL) || (event->what == nullEvent)  || 
          (anEvent.message == NS_FOCUS_CONTENT)          || 
          (anEvent.message == NS_BLUR_CONTENT)           || 
          (anEvent.message == NS_MOUSE_MOVE)             ||
          (anEvent.message == NS_MOUSE_ENTER)) {
        GUItoMacEvent(anEvent, event, macEvent);
        event = &macEvent;
      }

#ifdef MOZ_WIDGET_COCOA
      if (anEvent.message == NS_FOCUS_CONTENT)
      {
        // Work around an issue in the Flash plugin, which can cache a pointer
        // to a doomed TSM document (one that belongs to a NSTSMInputContext)
        // and try to activate it after it has been deleted. See bug 183313.
        ::DeactivateTSMDocument(::TSMGetActiveDocument());
      }
#endif
      
      nsPluginPort* pluginPort = FixUpPluginWindow(ePluginPaintIgnore);
      PRBool eventHandled = PR_FALSE;
      if (pluginPort) {
        nsPluginEvent pluginEvent =
          {
            event,
            nsPluginPlatformWindowRef(GetWindowFromPort(pluginPort->port))
          };

        mInstance->HandleEvent(&pluginEvent, &eventHandled);
      }

      if (eventHandled && !(anEvent.message == NS_MOUSE_LEFT_BUTTON_DOWN &&
                            !mContentFocused))
        rv = nsEventStatus_eConsumeNoDefault;

      pluginWidget->EndDrawPlugin();
    }
  }
#endif

#ifdef XP_WIN
  // this code supports windowless plugins
  nsPluginEvent * pPluginEvent = (nsPluginEvent *)anEvent.nativeMsg;
  // we can get synthetic events from the nsEventStateManager... these
  // have no nativeMsg
  nsPluginEvent pluginEvent;
  if (!pPluginEvent) {
    switch (anEvent.message) {
      case NS_FOCUS_CONTENT:
        pluginEvent.event = WM_SETFOCUS;
        pluginEvent.wParam = 0;
        pluginEvent.lParam = 0;
        pPluginEvent = &pluginEvent;
        break;
      case NS_BLUR_CONTENT:
        pluginEvent.event = WM_KILLFOCUS;
        pluginEvent.wParam = 0;
        pluginEvent.lParam = 0;
        pPluginEvent = &pluginEvent;
        break;
    }
  }

  if (pPluginEvent) {
    PRBool eventHandled = PR_FALSE;
    mInstance->HandleEvent(pPluginEvent, &eventHandled);
    if (eventHandled)
      rv = nsEventStatus_eConsumeNoDefault;
  }
#endif

  return rv;
}

nsresult
nsPluginInstanceOwner::Destroy()
{
  nsIContent* content = mOwner->GetContent();

  // stop the timer explicitly to reduce reference count.
  CancelTimer();

  // unregister context menu listener
  if (mCXMenuListener) {
    mCXMenuListener->Destroy(mOwner);    
    NS_RELEASE(mCXMenuListener);
  }

  nsCOMPtr<nsIDOMEventReceiver> receiver(do_QueryInterface(content));
  if (receiver) {

    nsCOMPtr<nsIDOMEventListener> listener;
    QueryInterface(NS_GET_IID(nsIDOMEventListener), getter_AddRefs(listener));

    // Unregister focus event listener
    receiver->RemoveEventListenerByIID(listener, NS_GET_IID(nsIDOMFocusListener));

    // Unregister mouse event listener
    receiver->RemoveEventListenerByIID(listener, NS_GET_IID(nsIDOMMouseListener));

    // now for the mouse motion listener
    receiver->RemoveEventListenerByIID(listener, NS_GET_IID(nsIDOMMouseMotionListener));

    // Unregister key event listener;
    receiver->RemoveEventListener(NS_LITERAL_STRING("keypress"), listener, PR_TRUE);
    receiver->RemoveEventListener(NS_LITERAL_STRING("keydown"), listener, PR_TRUE);
    receiver->RemoveEventListener(NS_LITERAL_STRING("keyup"), listener, PR_TRUE);

    // Unregister drag event listener;
    receiver->RemoveEventListener(NS_LITERAL_STRING("dragdrop"), listener, PR_TRUE);
    receiver->RemoveEventListener(NS_LITERAL_STRING("dragover"), listener, PR_TRUE);
    receiver->RemoveEventListener(NS_LITERAL_STRING("dragexit"), listener, PR_TRUE);
    receiver->RemoveEventListener(NS_LITERAL_STRING("dragenter"), listener, PR_TRUE);
    receiver->RemoveEventListener(NS_LITERAL_STRING("draggesture"), listener, PR_TRUE);
  }

  // Unregister scroll position listener
  nsIFrame* parentWithView = mOwner->GetAncestorWithView();
  nsIView* curView = parentWithView ? parentWithView->GetView() : nsnull;
  while (curView) {
    nsIScrollableView* scrollingView = curView->ToScrollableView();
    if (scrollingView)
      scrollingView->RemoveScrollPositionListener((nsIScrollPositionListener *)this);
    
    curView = curView->GetParent();
  }

  mOwner = nsnull; // break relationship between frame and plugin instance owner

  return NS_OK;
}

// Paints are handled differently, so we just simulate an update event.

void nsPluginInstanceOwner::Paint(const nsRect& aDirtyRect, PRUint32 ndc)
{
  if (!mInstance || !mOwner)
    return;
 
#ifdef XP_MACOSX
#ifdef DO_DIRTY_INTERSECT   // aDirtyRect isn't always correct, see bug 56128
  nsPoint rel(aDirtyRect.x, aDirtyRect.y);
  nsPoint abs(0,0);
  nsCOMPtr<nsIWidget> containerWidget;

  // Convert dirty rect relative coordinates to absolute and also get the containerWidget
  ConvertRelativeToWindowAbsolute(mOwner, rel, abs, *getter_AddRefs(containerWidget));

  nsRect absDirtyRect = nsRect(abs.x, abs.y, aDirtyRect.width, aDirtyRect.height);

  // Convert to absolute pixel values for the dirty rect
  nsRect absDirtyRectInPixels;
  ConvertTwipsToPixels(*mOwner->GetPresContext(), absDirtyRect,
                       absDirtyRectInPixels);
#endif

  nsCOMPtr<nsIPluginWidget> pluginWidget = do_QueryInterface(mWidget);
  if (pluginWidget && NS_SUCCEEDED(pluginWidget->StartDrawPlugin())) {
    nsPluginPort* pluginPort = FixUpPluginWindow(ePluginPaintEnable);
    if (pluginPort) {
      WindowRef  nativeWindowRef = ::GetWindowFromPort(pluginPort->port);
      
      EventRecord updateEvent;
      InitializeEventRecord(&updateEvent);
      updateEvent.what = updateEvt;
      updateEvent.message = UInt32(nativeWindowRef);
    
      nsPluginEvent pluginEvent = { &updateEvent, nsPluginPlatformWindowRef(nativeWindowRef) };
      PRBool eventHandled = PR_FALSE;
      mInstance->HandleEvent(&pluginEvent, &eventHandled);
    }
    pluginWidget->EndDrawPlugin();
  }
#endif

#ifdef XP_WIN
  nsPluginWindow * window;
  GetWindow(window);
  nsRect relDirtyRect = nsRect(aDirtyRect.x, aDirtyRect.y, aDirtyRect.width, aDirtyRect.height);
  nsRect relDirtyRectInPixels;
  ConvertTwipsToPixels(*mOwner->GetPresContext(), relDirtyRect,
                       relDirtyRectInPixels);

  // we got dirty rectangle in relative window coordinates, but we
  // need it in absolute units and in the (left, top, right, bottom) form
  RECT drc;
  drc.left   = relDirtyRectInPixels.x + window->x;
  drc.top    = relDirtyRectInPixels.y + window->y;
  drc.right  = drc.left + relDirtyRectInPixels.width;
  drc.bottom = drc.top + relDirtyRectInPixels.height;

  nsPluginEvent pluginEvent;
  pluginEvent.event = WM_PAINT;
  pluginEvent.wParam = (uint32)ndc;
  pluginEvent.lParam = (uint32)&drc;
  PRBool eventHandled = PR_FALSE;
  mInstance->HandleEvent(&pluginEvent, &eventHandled);
#endif
}

// Here's how we give idle time to plugins.

NS_IMETHODIMP nsPluginInstanceOwner::Notify(nsITimer* /* timer */)
{
#ifdef XP_MACOSX
    // validate the plugin clipping information by syncing the plugin window info to
    // reflect the current widget location. This makes sure that everything is updated
    // correctly in the event of scrolling in the window.
    if (mInstance) {
        nsCOMPtr<nsIPluginWidget> pluginWidget = do_QueryInterface(mWidget);
        if (pluginWidget && NS_SUCCEEDED(pluginWidget->StartDrawPlugin())) {
            nsPluginPort* pluginPort = FixUpPluginWindow(ePluginPaintIgnore);
            if (pluginPort) {
                EventRecord idleEvent;
                InitializeEventRecord(&idleEvent);
                idleEvent.what = nullEvent;
                    
                // give a bogus 'where' field of our null event when hidden, so Flash
                // won't respond to mouse moves in other tabs, see bug 120875
                if (!mWidgetVisible)
                    idleEvent.where.h = idleEvent.where.v = 20000;
    
                nsPluginEvent pluginEvent = { &idleEvent, nsPluginPlatformWindowRef(GetWindowFromPort(pluginPort->port)) };
    
                PRBool eventHandled = PR_FALSE;
                mInstance->HandleEvent(&pluginEvent, &eventHandled);
            }
            
            pluginWidget->EndDrawPlugin();
       }
    }
#endif
    return NS_OK;
}

void nsPluginInstanceOwner::StartTimer()
{
#ifdef XP_MACOSX
    nsresult rv;

    // start a periodic timer to provide null events to the plugin instance.
    if (!mPluginTimer) {
      mPluginTimer = do_CreateInstance("@mozilla.org/timer;1", &rv);
      if (NS_SUCCEEDED(rv))
        rv = mPluginTimer->InitWithCallback(this, 1020 / 60, nsITimer::TYPE_REPEATING_SLACK);
    }
#endif
}

void nsPluginInstanceOwner::CancelTimer()
{
    if (mPluginTimer) {
        mPluginTimer->Cancel();
        mPluginTimer = nsnull;
    }
}

NS_IMETHODIMP nsPluginInstanceOwner::Init(nsPresContext* aPresContext, nsObjectFrame *aFrame)
{
  //do not addref to avoid circular refs. MMP
  mOwner = aFrame;
  
  nsIContent* content = mOwner->GetContent();
  
  // Some plugins require a specific sequence of shutdown and startup when
  // a page is reloaded. Shutdown happens usually when the last instance
  // is destroyed. Here we make sure the plugin instance in the old
  // document is destroyed before we try to create the new one.
  aPresContext->EnsureVisible(PR_TRUE);

  // register context menu listener
  mCXMenuListener = new nsPluginDOMContextMenuListener();
  if (mCXMenuListener) {    
    NS_ADDREF(mCXMenuListener);    
    mCXMenuListener->Init(aFrame);
  }

  nsCOMPtr<nsIDOMEventReceiver> receiver(do_QueryInterface(content));
  if (receiver) {

    nsCOMPtr<nsIDOMEventListener> listener;
    QueryInterface(NS_GET_IID(nsIDOMEventListener), getter_AddRefs(listener));

    // Register focus listener
    receiver->AddEventListenerByIID(listener, NS_GET_IID(nsIDOMFocusListener));

    // Register mouse listener
    receiver->AddEventListenerByIID(listener, NS_GET_IID(nsIDOMMouseListener));

    // now do the mouse motion listener
    receiver->AddEventListenerByIID(listener, NS_GET_IID(nsIDOMMouseMotionListener));

    // Register key listener
    receiver->AddEventListener(NS_LITERAL_STRING("keypress"), listener, PR_TRUE);
    receiver->AddEventListener(NS_LITERAL_STRING("keydown"), listener, PR_TRUE);
    receiver->AddEventListener(NS_LITERAL_STRING("keyup"), listener, PR_TRUE);

    // Register drag listener
    receiver->AddEventListener(NS_LITERAL_STRING("dragdrop"), listener, PR_TRUE);
    receiver->AddEventListener(NS_LITERAL_STRING("dragover"), listener, PR_TRUE);
    receiver->AddEventListener(NS_LITERAL_STRING("dragexit"), listener, PR_TRUE);
    receiver->AddEventListener(NS_LITERAL_STRING("dragenter"), listener, PR_TRUE);
    receiver->AddEventListener(NS_LITERAL_STRING("draggesture"), listener, PR_TRUE);
  }
  
  // Register scroll position listener
  // We need to register a scroll pos listener on every scrollable
  // view up to the top
  nsIFrame* parentWithView = mOwner->GetAncestorWithView();
  nsIView* curView = parentWithView ? parentWithView->GetView() : nsnull;
  while (curView) {
    nsIScrollableView* scrollingView = curView->ToScrollableView();
    if (scrollingView)
      scrollingView->AddScrollPositionListener((nsIScrollPositionListener *)this);
    
    curView = curView->GetParent();
  }

  return NS_OK; 
}

nsPluginPort* nsPluginInstanceOwner::GetPluginPort()
{
//!!! Port must be released for windowless plugins on Windows, because it is HDC !!!

  nsPluginPort* result = NULL;
  if (mWidget) {
#ifdef XP_WIN
    if (mPluginWindow && mPluginWindow->type == nsPluginWindowType_Drawable)
      result = (nsPluginPort*) mWidget->GetNativeData(NS_NATIVE_GRAPHIC);
    else
#endif
      result = (nsPluginPort*) mWidget->GetNativeData(NS_NATIVE_PLUGIN_PORT);
    }
    return result;
}

void nsPluginInstanceOwner::ReleasePluginPort(nsPluginPort * pluginPort)
{
#ifdef XP_WIN
  if (mWidget && mPluginWindow &&
      mPluginWindow->type == nsPluginWindowType_Drawable) {
    mWidget->FreeNativeData((HDC)pluginPort, NS_NATIVE_GRAPHIC);
  }
#endif
}

NS_IMETHODIMP nsPluginInstanceOwner::CreateWidget(void)
{
  NS_ENSURE_TRUE(mPluginWindow, NS_ERROR_NULL_POINTER);

  nsIView   *view;
  nsresult  rv = NS_ERROR_FAILURE;

  if (mOwner) {
    // Create view if necessary

    view = mOwner->GetView();

    if (!view || !mWidget) {
      PRBool windowless = PR_FALSE;

      mInstance->GetValue(nsPluginInstanceVariable_WindowlessBool,
                          (void *)&windowless);

      // always create widgets in Twips, not pixels
      float p2t = mOwner->GetPresContext()->ScaledPixelsToTwips();
      rv = mOwner->CreateWidget(NSIntPixelsToTwips(mPluginWindow->width, p2t),
                                NSIntPixelsToTwips(mPluginWindow->height, p2t),
                                windowless);
      if (NS_OK == rv) {
        view = mOwner->GetView();

        if (view) {
          mWidget = view->GetWidget();
          PRBool fTransparent = PR_FALSE;
          mInstance->GetValue(nsPluginInstanceVariable_TransparentBool,
                              (void *)&fTransparent);
          
          view->GetViewManager()->SetViewContentTransparency(view,
                                                             fTransparent);
        }

        if (PR_TRUE == windowless) {
          mPluginWindow->type = nsPluginWindowType_Drawable;

          // this needs to be a HDC according to the spec, but I do
          // not see the right way to release it so let's postpone
          // passing HDC till paint event when it is really
          // needed. Change spec?
          mPluginWindow->window = nsnull;
        } else if (mWidget) {
          mWidget->Resize(mPluginWindow->width, mPluginWindow->height,
                          PR_FALSE);

          // mPluginWindow->type is used in |GetPluginPort| so it must
          // be initilized first
          mPluginWindow->type = nsPluginWindowType_Window;
          mPluginWindow->window = GetPluginPort();

          // start the idle timer.
          StartTimer();

          // tell the plugin window about the widget
          mPluginWindow->SetPluginWidget(mWidget);
        }
      }
    }
  }

  return rv;
}

void nsPluginInstanceOwner::SetPluginHost(nsIPluginHost* aHost)
{
  mPluginHost = aHost;
}

#if defined(XP_WIN) || (defined(DO_DIRTY_INTERSECT) && defined(XP_MACOSX))
// convert frame coordinates from twips to pixels
static void ConvertTwipsToPixels(nsPresContext& aPresContext, nsRect& aTwipsRect, nsRect& aPixelRect)
{
  float t2p;
  t2p = aPresContext.TwipsToPixels();
  aPixelRect.x = NSTwipsToIntPixels(aTwipsRect.x, t2p);
  aPixelRect.y = NSTwipsToIntPixels(aTwipsRect.y, t2p);
  aPixelRect.width = NSTwipsToIntPixels(aTwipsRect.width, t2p);
  aPixelRect.height = NSTwipsToIntPixels(aTwipsRect.height, t2p);
}
#endif

  // Mac specific code to fix up the port location and clipping region
#ifdef XP_MACOSX

#ifdef DO_DIRTY_INTERSECT
// Convert from a frame relative coordinate to a coordinate relative to its
// containing window
static void ConvertRelativeToWindowAbsolute(nsIFrame*   aFrame,
                                            nsPoint&    aRel, 
                                            nsPoint&    aAbs,
                                            nsIWidget*& aContainerWidget)
{
  // See if this frame has a view
  nsIView *view = aFrame->GetView();
  if (!view) {
    aAbs.x = 0;
    aAbs.y = 0;
    // Calculate frames offset from its nearest view
    aFrame->GetOffsetFromView(aAbs, &view);
  } else {
    // Store frames offset from its view.
    aAbs = aFrame->GetPosition();
  }

  NS_ASSERTION(view, "the object frame does not have a view");
  if (view) {
    // Calculate the view's offset from its nearest widget
    nsPoint viewOffset;
    aContainerWidget = view->GetNearestWidget(&viewOffset);
    NS_IF_ADDREF(aContainerWidget);
    aAbs += viewOffset;
  }

  // Add relative coordinate to the absolute coordinate that has been calculated
  aAbs += aRel;
}
#endif // DO_DIRTY_INTERSECT

nsPluginPort* nsPluginInstanceOwner::FixUpPluginWindow(PRInt32 inPaintState)
{
  if (!mWidget || !mPluginWindow)
    return nsnull;

  nsPluginPort* pluginPort = GetPluginPort(); 

  if (!pluginPort)
    return nsnull;

  // first, check our view for CSS visibility style
  PRBool isVisible =
    mOwner->GetView()->GetVisibility() == nsViewVisibility_kShow;

  nsCOMPtr<nsIPluginWidget> pluginWidget = do_QueryInterface(mWidget);
  
  nsPoint pluginOrigin;
  nsRect widgetClip;
  PRBool widgetVisible;
  pluginWidget->GetPluginClipRect(widgetClip, pluginOrigin, /* out */ widgetVisible);
  
  // printf("GetPluginClipRect returning visible %d\n", widgetVisible);

  isVisible &= widgetVisible;
  if (!isVisible)
    widgetClip.Empty();
  
  // set the port coordinates
  mPluginWindow->x = -pluginPort->portx;
  mPluginWindow->y = -pluginPort->porty;

  nsPluginRect oldClipRect = mPluginWindow->clipRect;
  
  // fix up the clipping region
  mPluginWindow->clipRect.top    = widgetClip.y;
  mPluginWindow->clipRect.left   = widgetClip.x;

  mWidgetVisible = isVisible;

  if (!mWidgetVisible || inPaintState == ePluginPaintDisable) {
    mPluginWindow->clipRect.bottom = mPluginWindow->clipRect.top;
    mPluginWindow->clipRect.right  = mPluginWindow->clipRect.left;
    // pluginPort = nsnull; // don't uncomment this
  }
  else if (inPaintState == ePluginPaintEnable)
  {
    mPluginWindow->clipRect.bottom = mPluginWindow->clipRect.top + widgetClip.height;
    mPluginWindow->clipRect.right  = mPluginWindow->clipRect.left + widgetClip.width; 
  }

  // if the clip rect changed, call SetWindow()
  // (RealPlayer needs this to draw correctly)
  if (mPluginWindow->clipRect.left    != oldClipRect.left   ||
      mPluginWindow->clipRect.top     != oldClipRect.top    ||
      mPluginWindow->clipRect.right   != oldClipRect.right  ||
      mPluginWindow->clipRect.bottom  != oldClipRect.bottom)
  {  
    mInstance->SetWindow(mPluginWindow);
  }

  return pluginPort;
}

#endif // XP_MACOSX

