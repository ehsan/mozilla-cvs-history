/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
/* 
   BrowserFrame.cpp -- class definition for the browser frame class
   Created: Spence Murray <spence@netscape.com>, 17-Oct-96.
 */


#include "rosetta.h"
#include "BrowserFrame.h"
#include "BrowserView.h"
#include "HistoryMenu.h"
#include "BackForwardMenu.h"
#include "Command.h"
#include "MozillaApp.h"
#include "ViewGlue.h"
#include "RDFToolbox.h"
#include "NavCenterFrame.h"
#include "Logo.h"
#include "xpassert.h"
#include "csid.h"

#ifdef ENDER
#include "EditorFrame.h"
#include "EditorView.h"
#include "EditorToolbar.h"
#endif

#include "DtWidgets/ComboBox.h"

#include <Xfe/Xfe.h>

#ifdef DEBUG_spence
#define D(x) x
#else
#define D(x)
#endif

extern "C" {
  void fe_set_scrolled_default_size(MWContext *context);
  void fe_home_cb (Widget widget, XtPointer closure, XtPointer call_data);
  Boolean plonk (MWContext *context);
  Boolean plonk_cancelled (void);
  URL_Struct *fe_GetBrowserStartupUrlStruct();
}


// Browser Encoding Menu Spec - no longer shared between Browsers, and Mail/News
MenuSpec XFE_BrowserFrame::encoding_menu_spec[] = {
	{ xfeCmdChangeDocumentEncoding,	TOGGLEBUTTON, NULL, "EncodingRadioGroup", False, (void*)CS_LATIN1 },
	MENU_SEPARATOR,
	{ xfeCmdChangeDocumentEncoding,	TOGGLEBUTTON, NULL, "EncodingRadioGroup", False, (void*)CS_LATIN2 },
	{ xfeCmdChangeDocumentEncoding,	TOGGLEBUTTON, NULL, "EncodingRadioGroup", False, (void*)CS_CP_1250 },
	MENU_SEPARATOR,
	{ xfeCmdChangeDocumentEncoding,	TOGGLEBUTTON, NULL, "EncodingRadioGroup", False, (void*)CS_EUCJP_AUTO },
	{ xfeCmdChangeDocumentEncoding,	TOGGLEBUTTON, NULL, "EncodingRadioGroup", False, (void*)CS_SJIS },
	{ xfeCmdChangeDocumentEncoding,	TOGGLEBUTTON, NULL, "EncodingRadioGroup", False, (void*)CS_EUCJP },
	MENU_SEPARATOR,
	{ xfeCmdChangeDocumentEncoding,	TOGGLEBUTTON, NULL, "EncodingRadioGroup", False, (void*)CS_BIG5 },
	{ xfeCmdChangeDocumentEncoding,	TOGGLEBUTTON, NULL, "EncodingRadioGroup", False, (void*)CS_CNS_8BIT },
	MENU_SEPARATOR,
	{ xfeCmdChangeDocumentEncoding,	TOGGLEBUTTON, NULL, "EncodingRadioGroup", False, (void*)CS_GB_8BIT },
	MENU_SEPARATOR,
	{ xfeCmdChangeDocumentEncoding,	TOGGLEBUTTON, NULL, "EncodingRadioGroup", False, (void*)CS_KSC_8BIT_AUTO },
	{ xfeCmdChangeDocumentEncoding,	TOGGLEBUTTON, NULL, "EncodingRadioGroup", False, (void*)CS_KSC_8BIT },
	{ xfeCmdChangeDocumentEncoding,	TOGGLEBUTTON, NULL, "EncodingRadioGroup", False, (void*)CS_2022_KR },
	MENU_SEPARATOR,
	{ xfeCmdChangeDocumentEncoding,	TOGGLEBUTTON, NULL, "EncodingRadioGroup", False, (void*)CS_8859_5 },
	{ xfeCmdChangeDocumentEncoding,	TOGGLEBUTTON, NULL, "EncodingRadioGroup", False, (void*)CS_KOI8_R },
	{ xfeCmdChangeDocumentEncoding,	TOGGLEBUTTON, NULL, "EncodingRadioGroup", False, (void*)CS_CP_1251 },
	MENU_SEPARATOR,
	{ xfeCmdChangeDocumentEncoding,	TOGGLEBUTTON, NULL, "EncodingRadioGroup", False, (void*)CS_KOI8_U },
	MENU_SEPARATOR,
	{ xfeCmdChangeDocumentEncoding,	TOGGLEBUTTON, NULL, "EncodingRadioGroup", False, (void*)CS_ARMSCII8 },
	MENU_SEPARATOR,
	{ xfeCmdChangeDocumentEncoding,	TOGGLEBUTTON, NULL, "EncodingRadioGroup", False, (void*)CS_8859_7 },
	{ xfeCmdChangeDocumentEncoding,	TOGGLEBUTTON, NULL, "EncodingRadioGroup", False, (void*)CS_CP_1253 },
	MENU_SEPARATOR,
	{ xfeCmdChangeDocumentEncoding,	TOGGLEBUTTON, NULL, "EncodingRadioGroup", False, (void*)CS_8859_9 },
	MENU_SEPARATOR,
	{ xfeCmdChangeDocumentEncoding,	TOGGLEBUTTON, NULL, "EncodingRadioGroup", False, (void*)CS_TIS620 },
	MENU_SEPARATOR,
	{ xfeCmdChangeDocumentEncoding,	TOGGLEBUTTON, NULL, "EncodingRadioGroup", False, (void*)CS_UTF8 },
	{ xfeCmdChangeDocumentEncoding,	TOGGLEBUTTON, NULL, "EncodingRadioGroup", False, (void*)CS_UTF7 },
	MENU_SEPARATOR,
	{ xfeCmdChangeDocumentEncoding,	TOGGLEBUTTON, NULL, "EncodingRadioGroup", False, (void*)CS_USRDEF2 },
	MENU_SEPARATOR,
	{ xfeCmdSetDefaultDocumentEncoding,	PUSHBUTTON },
	{ NULL }
};
MenuSpec XFE_BrowserFrame::file_menu_spec[] = {
#if (defined(MOZ_MAIL_NEWS) || defined(EDITOR))
  { "newSubmenu",		CASCADEBUTTON,
	(MenuSpec*)&XFE_Frame::new_menu_spec },
#else
  { xfeCmdOpenBrowser,	PUSHBUTTON },
#endif
  { xfeCmdOpenPage,		PUSHBUTTON },
  MENU_SEPARATOR,
  { xfeCmdSaveAs,		PUSHBUTTON },
  { xfeCmdSaveFrameAs,  PUSHBUTTON },
  MENU_SEPARATOR,
  { xfeCmdSendPage,		PUSHBUTTON },
  { xfeCmdSendLink,     PUSHBUTTON },
  MENU_SEPARATOR,
#ifdef EDITOR
  { xfeCmdEditPage,		PUSHBUTTON },
  { xfeCmdEditFrame,	PUSHBUTTON },
#endif
  { xfeCmdUploadFile,		PUSHBUTTON },
  //MENU_SEPARATOR,
  //{ xfeCmdGoOffline,            PUSHBUTTON },
  MENU_SEPARATOR,
  //{ xfeCmdPrintSetup,		PUSHBUTTON },
  //{ xfeCmdPrintPreview,		PUSHBUTTON },
  { xfeCmdPrint,		PUSHBUTTON },
  MENU_SEPARATOR,
  { xfeCmdClose,		PUSHBUTTON },
  { xfeCmdExit,			PUSHBUTTON },
  { NULL }
};

MenuSpec XFE_BrowserFrame::edit_menu_spec[] = {
  { xfeCmdUndo,			PUSHBUTTON },
  { xfeCmdRedo,			PUSHBUTTON },
  MENU_SEPARATOR,
  { xfeCmdCut,			PUSHBUTTON },
  { xfeCmdCopy,			PUSHBUTTON },
  { xfeCmdPaste,		PUSHBUTTON },
  //xxxDelete
  { xfeCmdSelectAll,		PUSHBUTTON },
  MENU_SEPARATOR,
  { xfeCmdFindInObject,		PUSHBUTTON },
  { xfeCmdFindAgain,		PUSHBUTTON },
  { xfeCmdSearch,	        PUSHBUTTON },
#ifdef MOZ_MAIL_NEWS
  { xfeCmdSearchAddress,	PUSHBUTTON },
  MENU_SEPARATOR,
#endif
  { xfeCmdEditToolbars,	PUSHBUTTON },
  { xfeCmdEditPreferences,	PUSHBUTTON },
  { NULL }
};

MenuSpec XFE_BrowserFrame::view_menu_spec[] = {
  { xfeCmdToggleNavigationToolbar ,PUSHBUTTON },
  { xfeCmdToggleLocationToolbar,  PUSHBUTTON },
  { xfeCmdToggleNavCenter, PUSHBUTTON},
  MENU_SEPARATOR,
  { xfeCmdIncreaseFont,		PUSHBUTTON },
  { xfeCmdDecreaseFont,		PUSHBUTTON },
  MENU_SEPARATOR,
  { xfeCmdReload,           PUSHBUTTON },
  { xfeCmdShowImages,		PUSHBUTTON },
  { xfeCmdRefresh,          PUSHBUTTON },
  { xfeCmdStopLoading,		PUSHBUTTON },
  MENU_SEPARATOR,
  { xfeCmdViewPageSource,	PUSHBUTTON },
  { xfeCmdViewPageInfo,		PUSHBUTTON },
  { xfeCmdPageServices,     PUSHBUTTON },
  MENU_SEPARATOR,
  { "encodingSubmenu",		CASCADEBUTTON,
	(MenuSpec*)&XFE_BrowserFrame::encoding_menu_spec },
  { NULL }
};

MenuSpec XFE_BrowserFrame::go_menu_spec[] = {
  { xfeCmdBack,			PUSHBUTTON },
  { xfeCmdForward,		PUSHBUTTON },
  { xfeCmdHome,			PUSHBUTTON },
  MENU_SEPARATOR,
  { "historyPlaceHolder",	DYNA_MENUITEMS, NULL, NULL, False, NULL, XFE_HistoryMenu::generate },
  { NULL }
};

MenuSpec XFE_BrowserFrame::menu_bar_spec[] = {
  { xfeMenuFile, 	CASCADEBUTTON, (MenuSpec*)&XFE_BrowserFrame::file_menu_spec },
  { xfeMenuEdit, 	CASCADEBUTTON, (MenuSpec*)&XFE_BrowserFrame::edit_menu_spec },
  { xfeMenuView, 	CASCADEBUTTON, (MenuSpec*)&XFE_BrowserFrame::view_menu_spec },
  { xfeMenuGo,		CASCADEBUTTON, (MenuSpec*)&XFE_BrowserFrame::go_menu_spec },
  { "bookmarksSubmenu",	CASCADEBUTTON, XFE_Frame::bookmark_submenu_spec },
  { xfeMenuWindow, 	CASCADEBUTTON, (MenuSpec*)&XFE_Frame::window_menu_spec },
  { xfeMenuHelp, 	CASCADEBUTTON, (MenuSpec*)&XFE_Frame::help_menu_spec },
  { NULL }
};

#ifdef ENDER
static ToolbarSpec alignment_menu_spec[] = {
	{ xfeCmdSetAlignmentStyleLeft,	 PUSHBUTTON, &ed_left_group },
	{ xfeCmdSetAlignmentStyleCenter, PUSHBUTTON, &ed_center_group },
	{ xfeCmdSetAlignmentStyleRight,	 PUSHBUTTON, &ed_right_group },
	{ NULL }
};

static ToolbarSpec goodies_menu_spec[] = {
	{ xfeCmdInsertLink,           PUSHBUTTON, &ed_link_group   },
	{ xfeCmdInsertTarget,         PUSHBUTTON, &ed_target_group },
	{ xfeCmdInsertImage,          PUSHBUTTON, &ed_image_group  },
	{ xfeCmdInsertHorizontalLine, PUSHBUTTON, &ed_hrule_group  },
	{ xfeCmdInsertTable,          PUSHBUTTON, &ed_table_group  },
	{ NULL }
};

static ToolbarSpec editor_style_toolbar_spec[] = {

	{ xfeCmdSetParagraphStyle, COMBOBOX },
	{ xfeCmdSetFontFace,       COMBOBOX },
	{ xfeCmdSetFontSize,       COMBOBOX },
	{ xfeCmdSetFontColor,      COMBOBOX },
	TOOLBAR_SEPARATOR,

	{ xfeCmdToggleCharacterStyleBold,	   TOGGLEBUTTON, &ed_bold_group },
	{ xfeCmdToggleCharacterStyleItalic,	   TOGGLEBUTTON, &ed_italic_group },
	{ xfeCmdToggleCharacterStyleUnderline, TOGGLEBUTTON, &ed_underline_group },
//	{ xfeCmdClearAllStyles,                PUSHBUTTON  , &ed_clear_group },
	TOOLBAR_SEPARATOR,

	{ xfeCmdInsertBulletedList,	TOGGLEBUTTON, &ed_bullet_group },
	{ xfeCmdInsertNumberedList,	TOGGLEBUTTON, &ed_number_group },
	TOOLBAR_SEPARATOR,

	{ xfeCmdOutdent,	PUSHBUTTON, &ed_outdent_group },
	{ xfeCmdIndent,	PUSHBUTTON, &ed_indent_group },
	{ xfeCmdSetAlignmentStyle, CASCADEBUTTON, &ed_left_group, 0, 0, 0,
	  (MenuSpec*)&alignment_menu_spec },
	{ "editorGoodiesMenu", CASCADEBUTTON, &ed_insert_group, 0, 0, 0,
	  (MenuSpec*)&goodies_menu_spec },
	{ NULL }
};
#endif /* ENDER */

static XFE_CommandList* my_commands;

XFE_Command*
XFE_BrowserFrame::getCommand(CommandType cmd)
{
	return findCommand(my_commands, cmd);
}

XFE_BrowserFrame::XFE_BrowserFrame(Widget toplevel,
								   XFE_Frame *parent_frame,
								   Chrome *chromespec) :
	XFE_Frame("Navigator", 
			  toplevel, 
			  parent_frame, 
			  FRAME_BROWSER, 
			  chromespec, 
			  True),
	m_rdfToolbars(NULL),
#ifdef ENDER
	m_editorStyleToolbar(NULL),
#endif
	m_browserDropSite(NULL)
{
  geometryPrefName = "browser";

  if (parent_frame)
    fe_copy_context_settings(m_context, parent_frame->getContext());
#ifdef notyet

  HG01092
  
#endif /* notyet */

  // Browser view holds the NavCenter view and the HTML view
  XFE_BrowserView * browserView =
      new XFE_BrowserView(this, getChromeParent(), NULL, m_context); 

  m_rdfToolbars = new XFE_RDFToolbox(this, m_toolbox);

  // add notification now 'cuz frame->getURL might not get called and
  // fe_SetURLString will break.
  registerInterest(
        XFE_HTMLView::newURLLoading, 
        this,
		(XFE_FunctionNotification)newPageLoading_cb);

  m_notification_added = True;

  XFE_MozillaApp::theApp()->registerInterest(
        XFE_MozillaApp::updateToolbarAppearance,
        this,
        (XFE_FunctionNotification)updateToolbarAppearance_cb);

  /*
  XtVaSetValues(browserView->getBaseWidget(),
		XmNleftAttachment, XmATTACH_FORM,
		XmNtopAttachment, XmATTACH_FORM,
		XmNrightAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		NULL);
        */
  
  // register drop site on HTMLView
  m_browserDropSite=new XFE_BrowserDrop((browserView->getHTMLView())->getBaseWidget(),this);  
  m_browserDropSite->enable();
     
  setView(browserView);   
  setMenubar(menu_bar_spec);

  if (fe_globalPrefs.autoload_images_p) 
  {
  }

  fe_set_scrolled_default_size(m_context);

//  htmlview->show();

  respectChrome(chromespec);

  HG72711
  m_dashboard->setShowStatusBar(True);
  m_dashboard->setShowProgressBar(True);

  // If a chromespec is given and all 3 toolbars are off, then we need
  // to unmanage the toolbox.  Otherwise the Frame's expect geometry 
  // will be wrong.
  if (chromespec
      && !chromespec->show_button_bar
      && !chromespec->show_url_bar
      && !chromespec->show_directory_buttons)
  {
      if (m_toolbox)
          m_toolbox->hide();
  }

  // Configure the toolbox for the first time
  configureToolbox();

#ifdef DEBUG_radha
  printf("Created the BrowserFrame\n");
#endif
}

XFE_BrowserFrame::~XFE_BrowserFrame()
{
    if (m_browserDropSite)
        delete m_browserDropSite;

    XFE_MozillaApp::theApp()->unregisterInterest(XFE_MozillaApp::updateToolbarAppearance,
                                                 this,
                                                 (XFE_FunctionNotification)updateToolbarAppearance_cb);
}

void
XFE_BrowserFrame::updateToolbar()
{
}

#ifdef ENDER
//////////////////////////////////////////////////////////////////////////
void
XFE_BrowserFrame::createEditorToolbar()
{
	XP_ASSERT( m_editorStyleToolbar == NULL );
	XP_ASSERT( m_toolbox != NULL );

	// Create the editor toolbars needed for embedded composer
	m_editorStyleToolbar = 
		new XFE_EditorToolbar(this,
							  m_toolbox,
							  "editorFormattingToolbar",
							  (ToolbarSpec*)&editor_style_toolbar_spec,
							  True);
	
	// Need to register commands which the editor toolbars will need:
	registerCommand(my_commands, new SetFontColorCommand(0));
}
//////////////////////////////////////////////////////////////////////////
void
XFE_BrowserFrame::showEditorToolbar(XFE_View* view)
{
	// Create the editor style toolbar only once
	if (!m_editorStyleToolbar)
	{
		createEditorToolbar();
	}

	XP_ASSERT( m_editorStyleToolbar != NULL );

	if (m_editorStyleToolbar)
	{
		if (view)
		{
			m_editorStyleToolbar->setCommandDispatcher(view);
		}

		m_editorStyleToolbar->update();

		// Update the toolbar to reflect the current view and cursor location:
		m_editorStyleToolbar->updateCommand(0);

		m_editorStyleToolbar->show();
	}
}
//////////////////////////////////////////////////////////////////////////
void
XFE_BrowserFrame::hideEditorToolbar()
{
	if (m_editorStyleToolbar)
	{
		m_editorStyleToolbar->hide();
	}
}
#endif /* ENDER */

//////////////////////////////////////////////////////////////////////////
//
// isCommandEnabled()
//
//////////////////////////////////////////////////////////////////////////
XP_Bool
XFE_BrowserFrame::isCommandEnabled(CommandType			cmd,
								   void *				calldata,
								   XFE_CommandInfo *	/* info */)
{
	if (cmd == xfeCmdToggleLocationToolbar
		|| cmd == xfeCmdToggleNavCenter
        || cmd == xfeCmdEditToolbars
		)
	{
		return True;
	}
	else
	{
		return XFE_Frame::isCommandEnabled(cmd, calldata);
	}
}
//////////////////////////////////////////////////////////////////////////
//
// doCommand()
//
//////////////////////////////////////////////////////////////////////////
void
XFE_BrowserFrame::doCommand(CommandType			cmd,
                            void *				calldata,
							XFE_CommandInfo *	info)
{
	if (cmd == xfeCmdToggleLocationToolbar)
    {
		return;
    }
	else if (cmd == xfeCmdToggleNavCenter)
    {
		if (((XFE_BrowserView*)m_view)->isNavCenterShown())
		{
			((XFE_BrowserView *)m_view)->hideNavCenter();
		}
		else
		{
			((XFE_BrowserView *) m_view)->showNavCenter();
		}
    }
	else if (cmd == xfeCmdClearUrlBar)
	{
//		XP_ASSERT( m_urlBar != NULL );
		
// 		if (m_urlBar == NULL)
// 		{
			XBell(XtDisplay(m_widget),100);

// 			return;
// 		}

// 		m_urlBar->clearText();
	}
    else if (cmd == xfeCmdEditToolbars)
    {
        XFE_NavCenterFrame::editToolbars(m_widget, this);

        return;
    }
	else
	{
		XFE_Frame::doCommand(cmd, calldata, info);
	}
}
//////////////////////////////////////////////////////////////////////////
//
// handlesCommand()
//
//////////////////////////////////////////////////////////////////////////
XP_Bool
XFE_BrowserFrame::handlesCommand(CommandType		cmd,
								 void *				calldata,
								 XFE_CommandInfo *	/* info */)
{
	if (cmd == xfeCmdToggleLocationToolbar
		|| cmd == xfeCmdToggleNavCenter
        || cmd == xfeCmdEditToolbars
        )
	{
		return True;
	}
	else
	{
		return XFE_Frame::handlesCommand(cmd, calldata);
	}
}
//////////////////////////////////////////////////////////////////////////
//
// commandToString()
//
//////////////////////////////////////////////////////////////////////////
char *
XFE_BrowserFrame::commandToString(CommandType		cmd,
								  void *			calldata,
								  XFE_CommandInfo *	info)
{
	if (cmd == xfeCmdToggleNavCenter)
    {
        char *res = NULL;
		
        if (((XFE_BrowserView *)m_view)->isNavCenterShown())
            res = "hideNavCenterCmdString";
        else
            res = "showNavCenterCmdString";
		
        return stringFromResource(res);
    }
	else
    {
		return XFE_Frame::commandToString(cmd, calldata, info);
    }
}

int
XFE_BrowserFrame::getURL(URL_Struct *url)
{
/*    XFE_HTMLView *hview = (XFE_HTMLView*)m_view;  */

  XFE_BrowserView * browserview = (XFE_BrowserView *)m_view;   

  // we can't conditionally register here - otherwise fe_SetURLString
  // won't work. we now do it in the constructor.
  //  if (!m_notification_added)
  //	  {
  //		  m_notification_added = True;
  //
  //		  registerInterest(XFE_HTMLView::newURLLoading, 
  //						   this,
  //						   (XFE_FunctionNotification)newPageLoading_cb);
  //	  }

  // set the url property
  storeProperty (m_context, "_MOZILLA_URL", 
				 url ? (const unsigned char *) url->address : (const unsigned char *)"");

/*      return hview->getURL(url, skip_get_url);  */
     return (browserview->getHTMLView())->getURL(url);  
}

void
XFE_BrowserFrame::queryChrome(Chrome * chrome)
{
  if (!chrome)
	return;
  XFE_Frame::queryChrome(chrome);
  chrome->show_url_bar           = False;
  chrome->show_directory_buttons = False;
}

void
XFE_BrowserFrame::respectChrome(Chrome * chrome)
{
  if (!chrome)
	return;

//  XFE_Frame::respectChrome(chrome);
  
  // Chain respectChrome() _AFTER_ doing urlbar and personal toolbar, 
  // so that the toolbox can be properly configured by the super class.
  XFE_Frame::respectChrome(chrome);
}


XFE_CALLBACK_DEFN(XFE_BrowserFrame, navigateToURL)(XFE_NotificationCenter*, void*, void* callData)
{
#if 0
	int status;
	URL_Struct *url_struct = (URL_Struct*)callData;
	
	// update _MOZILLA_URL property
	if (url_struct->address) {
	  storeProperty (m_context, "_MOZILLA_URL", 
		       (const unsigned char *) url_struct->address);
	}
	status = getURL(url_struct);
	
	if (status >= 0)
    {
		if (url_struct && url_struct->address)
		{
			m_urlBar->recordURL(url_struct);
		}
    }
#endif
}

XFE_CALLBACK_DEFN(XFE_BrowserFrame, newPageLoading)
	(XFE_NotificationCenter*, void*, void* callData)
{
#if 0
	URL_Struct *url = (URL_Struct*)callData;
	
	// update _MOZILLA_URL property
	if (url->address) {
	  storeProperty (m_context, "_MOZILLA_URL", 
		       (const unsigned char *) url->address);
	}

	m_urlBar->setURLString(url);

#ifdef ENDER
    hideEditorToolbar();
#endif /* ENDER */
#endif
}

XFE_CALLBACK_DEFN(XFE_BrowserFrame, updateToolbarAppearance)(XFE_NotificationCenter */*obj*/, 
									   void */*clientData*/, 
									   void */*callData*/)
{
  updateToolbar();
}

/*static*/
void
XFE_BrowserFrame::bringToFrontOrMakeNew(Widget toplevel)
{
  // This follows the "Do as I mean" Taskbar spec
  //   if 0 browsers, make a new one.
  //   if 1 browser and it is most recent, bring up new one
  //      otherwise bring up the one that exists
  //   if >1 browsers, cycle through them by
  //      bringing the oldest to the front

  XP_List *browserList =
    XFE_MozillaApp::theApp()->getFrameList(FRAME_BROWSER);

  int browserCount = XP_ListCount(browserList);

  struct fe_MWContext_cons *cons = fe_all_MWContexts;

  MWContext *leastRecentContext = NULL;

  if (browserCount == 0 ||
      (browserCount == 1 &&
       cons && cons->context->type == MWContextBrowser))
    {
      fe_showBrowser(toplevel, NULL, NULL,
                     fe_GetBrowserStartupUrlStruct());
    }
  else
    {
      for (; cons; cons = cons->next)
        {
          if (cons->context->type == MWContextBrowser
              && !cons->context->is_grid_cell)
            leastRecentContext = cons->context;
        }

      XFE_Frame *leastRecentFrame = ViewGlue_getFrame(leastRecentContext);

      if (leastRecentFrame)
        {
          leastRecentFrame->show();

          // Reorder fe_all_MWContexts to reflect the raise
          fe_UserActivity (leastRecentContext);
        }
      else  // We shouldn't get here, but just in case...
        {
          fe_showBrowser(toplevel, NULL, NULL, 
                         fe_GetBrowserStartupUrlStruct());
        }
    }
}

extern "C" MWContext *
fe_reuseBrowser(MWContext * context, URL_Struct *url)
{
	if (!context)
	{
		return fe_showBrowser(FE_GetToplevelWidget(), NULL, NULL, url);
	}

	Widget toplevel = XtParent(CONTEXT_WIDGET(context));
	
	if (context->type != MWContextBrowser)
    {
        context = fe_FindNonCustomBrowserContext(context);
    }

	MWContext * top_context = XP_GetNonGridContext(context);

	if (top_context && top_context->type == MWContextBrowser)
    {
		// NOTE:  if someone else is calling us make sure the browser
		//        window get's raised to the front...
		//

		// Popup the shell first, so that we gurantee its realized 
		//
		XtPopup(CONTEXT_WIDGET(top_context),XtGrabNone);
		
		// Force the window to the front and de-iconify if needed 
		//
		XMapRaised(XtDisplay(CONTEXT_WIDGET(top_context)),
				   XtWindow(CONTEXT_WIDGET(top_context)));

		fe_GetURL (top_context, url, FALSE);

		return top_context;
    }
	else
    {
		return fe_showBrowser(toplevel, NULL, NULL, url);
    }
}

extern "C" MWContext *
fe_showBrowser(Widget toplevel, XFE_Frame *parent_frame, Chrome *chromespec, URL_Struct *url)
{
  // not a static global, since we can have multiple browsers.
	XFE_BrowserFrame *theFrame;
	MWContext *theContext = NULL;
	
	D( printf("in showBrowser()\n"); );
	
	theFrame = new XFE_BrowserFrame(toplevel, parent_frame, chromespec);
	
	theFrame->show();
	
	theContext = theFrame->getContext();

    if ((!chromespec || chromespec->type != MWContextHTMLHelp)
        && plonk(theContext))
		{
			url = 0;
			
			if (!fe_contextIsValid(theContext)) return NULL;
		}

	if (!fe_VendorAnim)
	  if (NET_CheckForTimeBomb (theContext))
	    url = 0;

        if (url == NULL) {
		    if (!plonk_cancelled())
				theFrame->getURL(url);
			//fe_home_cb(toplevel, theFrame->getContext(), NULL);
            //else
			// do nothing - assume the plonk canceller is loading a page.
			// (i.e. XFE_BrowserDrop after a desktop file was dropped.)
        }
        else {
            theFrame->getURL(url);
        }
        
	// hang properties for the browser window
  
	theFrame->storeProperty (theContext,
		       (char *) "_MOZILLA_VERSION",
		       (const unsigned char *) fe_version);

	D( printf("leaving showBrowser()\n"); );

	return theContext;
}

/* A wrapper of XFE_showBrowser() above
 */
extern "C" MWContext *
XFE_showBrowser(Widget toplevel, URL_Struct *url)
{
	return fe_showBrowser(toplevel, NULL, NULL, url);
}

//
//    Yet another wrapper. Defined in xfe.h
//
extern "C" MWContext*
fe_BrowserGetURL(MWContext* context, char* address)
{
	return fe_reuseBrowser(context,
						   NET_CreateURLStruct(address, NET_DONT_RELOAD));
}

//////////////////////////////////////////////////////////////////////////
//
// Toolbox methods
//
//////////////////////////////////////////////////////////////////////////
void
XFE_BrowserFrame::toolboxItemSnap(XFE_ToolboxItem * item)
{
}
//////////////////////////////////////////////////////////////////////////
void
XFE_BrowserFrame::toolboxItemClose(XFE_ToolboxItem * item)
{
	XP_ASSERT( item != NULL );
}
//////////////////////////////////////////////////////////////////////////
void
XFE_BrowserFrame::toolboxItemOpen(XFE_ToolboxItem * item)
{
	XP_ASSERT( item != NULL );
}
//////////////////////////////////////////////////////////////////////////
void
XFE_BrowserFrame::toolboxItemChangeShowing(XFE_ToolboxItem * item)
{
	XP_ASSERT( item != NULL );
}
//////////////////////////////////////////////////////////////////////////
void
XFE_BrowserFrame::configureToolbox()
{
	// If a the frame was constructed with a chromespec, then we ignore
	// all the preference magic.
	if (m_chromespec_provided)
	{
		return;
	}

	// Make sure the toolbox is alive
	if (!m_toolbox || (m_toolbox && !m_toolbox->isAlive()))
	{
		return;
	}

//	printf("configureToolbox(%s)\n",XtName(m_widget));
}
//////////////////////////////////////////////////////////////////////////
