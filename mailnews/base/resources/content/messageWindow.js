/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 * 
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation. Portions created by Netscape are
 * Copyright (C) 1998-1999 Netscape Communications Corporation. All
 * Rights Reserved.
 */

/* This is where functions related to the standalone message window are kept */

/* globals for a particular window */

var compositeDataSourceProgID        = datasourceProgIDPrefix + "composite-datasource";

var gCompositeDataSource;
var gCurrentMessageUri;
var gCurrentFolderUri;

var gCurrentMessageIsDeleted = false;

// the folderListener object
var folderListener = {
    OnItemAdded: function(parentItem, item, view) {},

	OnItemRemoved: function(parentItem, item, view)
	{
		var parentFolderResource = parentItem.QueryInterface(Components.interfaces.nsIRDFResource);
		if(!parentFolderResource)
			return;

		var parentURI = parentFolderResource.Value;
		if(parentURI != gCurrentFolderUri)
			return;

		var deletedMessageResource = item.QueryInterface(Components.interfaces.nsIRDFResource);
		var deletedUri = deletedMessageResource.Value;

		//If the deleted message is our message then we know we're about to be deleted.
		if(deletedUri == gCurrentMessageUri)
		{
			gCurrentMessageIsDeleted = true;
		}

	},

	OnItemPropertyChanged: function(item, property, oldValue, newValue) {},

	OnItemIntPropertyChanged: function(item, property, oldValue, newValue)
	{
	},

	OnItemBoolPropertyChanged: function(item, property, oldValue, newValue) {},

    OnItemUnicharPropertyChanged: function(item, property, oldValue, newValue){},
	OnItemPropertyFlagChanged: function(item, property, oldFlag, newFlag) {},

    OnItemEvent: function(folder, event) {
		if (event.GetUnicode() == "DeleteOrMoveMsgCompleted") {
			HandleDeleteOrMoveMsgCompleted(folder);
		}     
    }
}

function HandleDeleteOrMoveMsgCompleted(folder)
{
	dump("In HandleDeleteOrMoveMsgCompleted\n");
	var folderResource = folder.QueryInterface(Components.interfaces.nsIRDFResource);
	if(!folderResource)
		return;

	var folderUri = folderResource.Value;
	if((folderUri == gCurrentFolderUri) && gCurrentMessageIsDeleted)
	{
		//If we knew we were going to be deleted and the deletion has finished, close the window.
		gCurrentMessageIsDeleted = false;
		//Use timeout to make sure all folder listeners get event before removing them.  Messes up
		//folder listener iterator if we don't do this.
		setTimeout("window.close();",0);

	}
}

function OnLoadMessageWindow()
{
	HideMenus();
	CreateMailWindowGlobals();
	CreateMessageWindowGlobals();
	verifyAccounts();

	InitMsgWindow();

	messenger.SetWindow(window, msgWindow);
	InitializeDataSources();
	// FIX ME - later we will be able to use onload from the overlay
	OnLoadMsgHeaderPane();

    try {
        mailSession.AddFolderListener(folderListener);
	} catch (ex) {
        dump("Error adding to session\n");
    }

	if(window.arguments && window.arguments.length == 2)
	{
		if(window.arguments[0])
		{
			gCurrentMessageUri = window.arguments[0];
		}
		else
		{
			gCurrentMessageUri = null;
		}

		if(window.arguments[1])
		{
			gCurrentFolderUri = window.arguments[1];
		}
		else
		{
			gCurrentFolderUri = null;
		}
	}	

  setTimeout("OpenURL(gCurrentMessageUri);", 0);
  SetupCommandUpdateHandlers();

}

function HideMenus()
{
	var message_menuitem=document.getElementById('menu_showMessage');
	if(message_menuitem)
		message_menuitem.setAttribute("hidden", "true");

	var expandOrCollapseMenu = document.getElementById('menu_expandOrCollapse');
	if(expandOrCollapseMenu)
		expandOrCollapseMenu.setAttribute("hidden", "true");

	var renameFolderMenu = document.getElementById('menu_renameFolder');
	if(renameFolderMenu)
		renameFolderMenu.setAttribute("hidden", "true");

	var viewMessagesMenu = document.getElementById('viewMessagesMenu');
	if(viewMessagesMenu)
		viewMessagesMenu.setAttribute("hidden", "true");
}

function OnUnloadMessageWindow()
{
	OnMailWindowUnload();
}

function CreateMessageWindowGlobals()
{
	gCompositeDataSource = Components.classes[compositeDataSourceProgID].createInstance();
	gCompositeDataSource = gCompositeDataSource.QueryInterface(Components.interfaces.nsIRDFCompositeDataSource);

}

function InitializeDataSources()
{
	AddDataSources();
	//Now add datasources to composite datasource
	gCompositeDataSource.AddDataSource(accountManagerDataSource);
    gCompositeDataSource.AddDataSource(folderDataSource);
	gCompositeDataSource.AddDataSource(messageDataSource);
}

function GetSelectedMsgFolders()
{
	var folderArray = new Array(1);
	var msgFolder = GetLoadedMsgFolder();
	if(msgFolder)
	{
		folderArray[0] = msgFolder;	
	}
	return folderArray;
}

function GetSelectedMessages()
{
	var messageArray = new Array(1);
	var message = GetLoadedMessage();
	if(message)
	{
		messageArray[0] = message;	
	}
	return messageArray;
}

function GetLoadedMsgFolder()
{
	var folderResource = RDF.GetResource(gCurrentFolderUri);
	if(folderResource)
	{
		var msgFolder = folderResource.QueryInterface(Components.interfaces.nsIMsgFolder);
		return msgFolder;
	}
	return null;
}

function GetLoadedMessage()
{
	var messageResource = RDF.GetResource(gCurrentMessageUri);
	if(messageResource)
	{
		var message = messageResource.QueryInterface(Components.interfaces.nsIMessage);
		return message;
	}
	return null;

}

function GetCompositeDataSource(command)
{
	return gCompositeDataSource;	
}

//Sets the next message after a delete.  If useSelection is true then use the
//current selection to determine this.  Otherwise use messagesToCheck which will
//be an array of nsIMessage's.
function SetNextMessageAfterDelete(messagesToCheck, useSelection)
{
	gCurrentMessageIsDeleted = true;
}

function SelectFolder(folderUri)
{
	gCurrentFolderUri = folderUri;
}

function SelectMessage(messageUri)
{
	gCurrentMessageUri = messageUri;
	OpenURL(gCurrentMessageUri);
}

function ReloadMessage()
{
	OpenURL(gCurrentMessageUri);
}

// MessageWindowController object (handles commands when one of the trees does not have focus)
var MessageWindowController =
{
   supportsCommand: function(command)
	{

		switch ( command )
		{
			case "cmd_delete":
			case "button_delete":
			case "cmd_shiftDelete":
				return true;
			default:
				return false;
		}
	},

	isCommandEnabled: function(command)
	{
		switch ( command )
		{
			case "cmd_delete":
			case "button_delete":
			case "cmd_shiftDelete":
				if ( command == "cmd_delete")
				{
					goSetMenuValue(command, 'valueMessage');
				}
				return ( gCurrentMessageUri != null);
			default:
				return false;
		}
	},

	doCommand: function(command)
	{
   		//dump("MessageWindowController.doCommand(" + command + ")\n");

		switch ( command )
		{
			case "cmd_delete":
				MsgDeleteMessage(false, false);
				break;
			case "cmd_shiftDelete":
				MsgDeleteMessage(true, false);
				break;
			case "button_delete":
				MsgDeleteMessage(false, true);
				break;
		}
	},
	
	onEvent: function(event)
	{
	}
};


function CommandUpdate_Mail()
{
	goUpdateCommand('cmd_delete');
	goUpdateCommand('button_delete');
	goUpdateCommand('cmd_shiftDelete');
}

function SetupCommandUpdateHandlers()
{
	top.controllers.insertControllerAt(0, MessageWindowController);
}

function CommandUpdate_UndoRedo()
{

}
