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

#include "nsViewerApp.h"
#include "nsBrowserWindow.h"
#include "nsIImageManager.h"
#include "nsIWidget.h"
#include <stdlib.h>
#include "resources.h"

#include <ToolUtils.h>			// MacOS includes
#include <Menus.h>
#include <Windows.h>
#include <Devices.h>
#include <Resources.h>
#include <Dialogs.h>

#include <PP_Messages.h>		// for PP standard menu commands
#include "nsMacMessagePump.h"	// for the windowless menu event handler

#if DEBUG
#include "macstdlibextras.h"
#endif

enum
{
	menu_First = 128,
	menu_Apple = menu_First,
	menu_File,
	menu_Edit,
	menu_Sample,
	menu_Debug,
	menu_Tools,
	menu_Last = menu_Tools,

	submenu_Print = 16,
	submenu_CompatibilityMode = 32,
	
	cmd_Sample0					= 1000,
	cmd_PrintOneColumn	= 2000,
	cmd_Find						= 3000,
	
	cmd_DebugMode				= 4000,
	cmd_ReflowTest,
	cmd_DumpContents,
	cmd_DumpFrames,
	cmd_DumpViews,
	cmd_DumpStyleSheets,
	cmd_DumpStyleContexts,
	cmd_ShowContentSize,
	cmd_ShowFrameSize,
	cmd_ShowStyleSize,
	cmd_DebugSave,
	cmd_DebugToggleSelection,
	cmd_DebugRobot,
	cmd_ShowContentQuality,
	
	cmd_Compatibility_NavQuirks	= 4200,
	cmd_Compatibility_Standard,
	
	cmd_JSConsole				= 5000,
	cmd_EditorMode,
	cmd_Top100,
	cmd_TableInspector,
	cmd_ImageInspector
};


static nsNativeViewerApp* gTheApp;


#pragma mark -
//----------------------------------------------------------------------

nsNativeViewerApp::nsNativeViewerApp()
{
	nsMacMessagePump::SetWindowlessMenuEventHandler(DispatchMenuItemWithoutWindow);
}

nsNativeViewerApp::~nsNativeViewerApp()
{
}

int
nsNativeViewerApp::Run()
{
  OpenWindow();
  mAppShell->Run();
  return 0;
}

void nsNativeViewerApp::DispatchMenuItemWithoutWindow(PRInt32 menuResult)
{
	long menuID = HiWord(menuResult);
	long menuItem = LoWord(menuResult);
	switch (menuID)
	{
		case menu_Apple:
			switch (menuItem)
			{
				case cmd_About:
					::Alert(128, nil);
					break;
				default:
					Str255 daName;
					GetMenuItemText(GetMenuHandle(menu_Apple), menuItem, daName);
					OpenDeskAcc(daName);
					break;
			}
			break;

		case menu_File:
			
			switch (menuItem)
			{
				case cmd_New:
					gTheApp->OpenWindow();
					break;
				case cmd_Open:
					nsBrowserWindow * newWindow;
					gTheApp->OpenWindow(0, newWindow);
					newWindow->DoFileOpen();
					break;
				case cmd_Quit:
					gTheApp->Exit();
					break;
			}
			break;
		}
}

#pragma mark -
//----------------------------------------------------------------------

nsNativeBrowserWindow::nsNativeBrowserWindow()
{
}

nsNativeBrowserWindow::~nsNativeBrowserWindow()
{
}

nsresult
nsNativeBrowserWindow::CreateMenuBar(PRInt32 aWidth)
{
	for (int i = menu_First; i <= menu_Last; i++)
	{
		InsertMenu(GetMenu(i), 0);
	}
	InsertMenu(GetMenu(submenu_Print), -1);
	InsertMenu(GetMenu(submenu_CompatibilityMode), -1);
	AppendResMenu(GetMenuHandle(menu_Apple), 'DRVR');
	DrawMenuBar();
	return NS_OK;
}

nsEventStatus
nsNativeBrowserWindow::DispatchMenuItem(PRInt32 aID)
{
	PRInt32 xpID = 0;
	long menuID = HiWord(aID);
	long menuItem = LoWord(aID);
	
	switch (menuID)
	{
		case menu_Apple:
			switch (menuItem)
			{
				case cmd_About:
					::Alert(128, nil);
					break;
				default:
					Str255 daName;
					GetMenuItemText(GetMenuHandle(menu_Apple), menuItem, daName);
					OpenDeskAcc(daName);
					break;
			}
			break;

		case menu_File:
			switch (menuItem)
			{
				case cmd_New:		xpID = VIEWER_WINDOW_OPEN;		break;
				case cmd_Open:		xpID = VIEWER_FILE_OPEN;		break;
				case cmd_Close:
					  WindowPtr whichwindow = FrontWindow();
				      nsIWidget* raptorWindow = *(nsIWidget**)::GetWRefCon(whichwindow);
				      raptorWindow->Destroy();
					break;
				case 2000:		xpID = VIEW_SOURCE;				break;
				case 2001:		xpID = VIEWER_TREEVIEW;			break;
				case 2002:		xpID = VIEWER_TOOLBARDEMO;
				case cmd_Save:		/*n.a.*/						break;
				case cmd_SaveAs:	/*n.a.*/						break;
				case cmd_Revert:	/*n.a.*/						break;
				case cmd_PageSetup:	/*n.a.*/						break;
				case cmd_Print:		xpID = VIEWER_PRINT;	break;
				case cmd_Quit:		xpID = VIEWER_EXIT;				break;
			}
			break;

		case menu_Edit:
			switch (menuItem)
			{
				case cmd_Undo:		/*n.a.*/						break;
				case cmd_Cut:		xpID = VIEWER_EDIT_CUT;			break;
				case cmd_Copy:		xpID = VIEWER_EDIT_COPY;		break;
				case cmd_Paste:		xpID = VIEWER_EDIT_PASTE;		break;
				case cmd_Clear:		/*n.a.*/						break;
				case cmd_SelectAll:	xpID = VIEWER_EDIT_SELECTALL;	break;
				case cmd_Find:		xpID = VIEWER_EDIT_FINDINPAGE;	break;
				case cmd_Preferences:	xpID = VIEWER_PREFS;		break;
			}
			break;

		case menu_Sample:
			xpID = VIEWER_DEMO0 + menuItem - cmd_Sample0;
			break;

		case menu_Debug:
			switch (menuItem)
			{
				case cmd_DebugMode:					xpID = VIEWER_VISUAL_DEBUGGING;			break;
				case cmd_ReflowTest:				xpID = VIEWER_REFLOW_TEST;					break;

				case cmd_DumpContents:			xpID = VIEWER_DUMP_CONTENT;					break;
				case cmd_DumpFrames:				xpID = VIEWER_DUMP_FRAMES;					break;
				case cmd_DumpViews:					xpID = VIEWER_DUMP_VIEWS;						break;

				case cmd_DumpStyleSheets:		xpID = VIEWER_DUMP_STYLE_SHEETS;		break;
				case cmd_DumpStyleContexts:	xpID = VIEWER_DUMP_STYLE_CONTEXTS;	break;

				case cmd_ShowContentSize:		xpID = VIEWER_SHOW_CONTENT_SIZE;		break;
				case cmd_ShowFrameSize:			xpID = VIEWER_SHOW_FRAME_SIZE;			break;
				case cmd_ShowStyleSize:			xpID = VIEWER_SHOW_STYLE_SIZE;			break;
				
				case cmd_DebugSave:							xpID = VIEWER_DEBUGSAVE;						break;
				case cmd_DebugToggleSelection:	xpID = VIEWER_TOGGLE_SELECTION;			break;
				case cmd_DebugRobot:						xpID = VIEWER_DEBUGROBOT;						break;
				case cmd_ShowContentQuality:		xpID =VIEWER_SHOW_CONTENT_QUALITY;	break;
			}
			break;
			
		case menu_Tools:
			switch (menuItem)
			{
				case cmd_JSConsole:					xpID = JS_CONSOLE;								break;
				case cmd_EditorMode:				xpID = EDITOR_MODE;								break;
				case cmd_Top100:						xpID = VIEWER_TOP100;							break;
				case cmd_TableInspector:		xpID = VIEWER_TABLE_INSPECTOR;		break;
				case cmd_ImageInspector:		xpID = VIEWER_IMAGE_INSPECTOR;		break;
			}
			break;
			
		case submenu_Print:
			xpID = VIEWER_ONE_COLUMN + menuItem - cmd_PrintOneColumn;
			break;
			
		case submenu_CompatibilityMode:
			switch (menuItem)
			{
				case cmd_Compatibility_NavQuirks:		xpID = VIEWER_NAV_QUIRKS_MODE;	break;
				case cmd_Compatibility_Standard:		xpID = VIEWER_STANDARD_MODE;		break;
			}
			break;
	}

	// Dispatch xp menu items
	if (xpID != 0)
		return nsBrowserWindow::DispatchMenuItem(xpID);
	else
		return nsEventStatus_eIgnore;
}

#pragma mark -
//----------------------------------------------------------------------
int main(int argc, char **argv)
{

#if DEBUG
	// Set up the console
	InitializeSIOUX(false);
#endif	// DEBUG

  // Hack to get il_ss set so it doesn't fail in xpcompat.c
  nsIImageManager *manager;
  NS_NewImageManager(&manager);

  gTheApp = new nsNativeViewerApp();
  NS_ADDREF(gTheApp);
  gTheApp->Initialize(argc, argv);
  gTheApp->Run();

  return 0;
}
