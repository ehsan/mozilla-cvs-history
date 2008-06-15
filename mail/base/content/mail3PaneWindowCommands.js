# -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1/GPL 2.0/LGPL 2.1
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Original Code is Mozilla Communicator client code, released
# March 31, 1998.
#
# The Initial Developer of the Original Code is
# Netscape Communications Corporation.
# Portions created by the Initial Developer are Copyright (C) 1998-2000
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#   Jan Varga <varga@nixcorp.com>
#   H�kan Waara (hwaara@chello.se)
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 2 or later (the "GPL"), or
# the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
# in which case the provisions of the GPL or the LGPL are applicable instead
# of those above. If you wish to allow use of your version of this file only
# under the terms of either the GPL or the LGPL, and not to allow others to
# use your version of this file under the terms of the MPL, indicate your
# decision by deleting the provisions above and replace them with the notice
# and other provisions required by the GPL or the LGPL. If you do not delete
# the provisions above, a recipient may use your version of this file under
# the terms of any one of the MPL, the GPL or the LGPL.
#
# ***** END LICENSE BLOCK *****

var gMessengerBundle = document.getElementById("bundle_messenger");

// Controller object for folder pane
var FolderPaneController =
{
   supportsCommand: function(command)
  {
    switch ( command )
    {
      case "cmd_delete":
      case "button_delete":
      case "button_compact":
      //case "cmd_selectAll": the folder pane currently only handles single selection
      case "cmd_cut":
      case "cmd_copy":
      case "cmd_paste":
        return true;

      default:
        return false;
    }
  },

  isCommandEnabled: function(command)
  {
    switch ( command )
    {
      case "cmd_cut":
      case "cmd_copy":
      case "cmd_paste":
        return false;
      case "cmd_delete":
      case "button_delete":
      case "button_compact":
      if ( command == "cmd_delete" )
        goSetMenuValue(command, 'valueFolder');
      var folderTree = GetFolderTree();
      var startIndex = {};
      var endIndex = {};
      folderTree.view.selection.getRangeAt(0, startIndex, endIndex);
      if (startIndex.value >= 0) {
        var canDeleteThisFolder;
        var specialFolder = null;
        var isServer = null;
        var serverType = null;
        try {
          var folderResource = GetFolderResource(folderTree, startIndex.value);
          specialFolder = GetFolderAttribute(folderTree, folderResource, "SpecialFolder");
          isServer = GetFolderAttribute(folderTree, folderResource, "IsServer") == "true";
          serverType = GetFolderAttribute(folderTree, folderResource, "ServerType");
          if (serverType == "nntp") {
             if ( command == "cmd_delete" ) {
                goSetMenuValue(command, 'valueNewsgroup');
                goSetAccessKey(command, 'valueNewsgroupAccessKey');
            }
          }
        }
        catch (ex) {
          //dump("specialFolder failure: " + ex + "\n");
        }
        if (specialFolder == "Inbox" || specialFolder == "Trash" || specialFolder == "Drafts" ||
            specialFolder == "Sent" || specialFolder == "Templates" || specialFolder == "Unsent Messages" ||
            (specialFolder == "Junk" && !CanRenameDeleteJunkMail(GetSelectedFolderURI())) || isServer)
          canDeleteThisFolder = false;
        else
          canDeleteThisFolder = true;
        return (command != "button_compact") ?
          canDeleteThisFolder && isCommandEnabled(command) :
          !isServer && IsCompactFolderEnabled();
      }
      else
        return false;
      default:
        return false;
    }
  },

  doCommand: function(command)
  {
    // if the user invoked a key short cut then it is possible that we got here for a command which is
    // really disabled. kick out if the command should be disabled.
    if (!this.isCommandEnabled(command)) return;

    switch ( command )
    {
      case "cmd_delete":
      case "button_delete":
        MsgDeleteFolder();
        break;
      case "button_compact":
        MsgCompactFolder(false);
        break;
    }
  },

  onEvent: function(event)
  {
  }
};

// DefaultController object (handles commands when one of the trees does not have focus)
var DefaultController =
{
   supportsCommand: function(command)
  {

    switch ( command )
    {
      case "cmd_createFilterFromPopup":
      case "cmd_close":
      case "cmd_reply":
      case "button_reply":
      case "cmd_replySender":
      case "cmd_replyGroup":
      case "cmd_replyall":
      case "button_replyall":
      case "cmd_forward":
      case "button_forward":
      case "cmd_forwardInline":
      case "cmd_forwardAttachment":
      case "cmd_editAsNew":
      case "cmd_createFilterFromMenu":
      case "cmd_delete":
      case "button_delete":
      case "button_junk":
      case "cmd_shiftDelete":
      case "cmd_nextMsg":
      case "button_next":
      case "button_previous":
      case "cmd_nextUnreadMsg":
      case "cmd_nextFlaggedMsg":
      case "cmd_nextUnreadThread":
      case "cmd_previousMsg":
      case "cmd_previousUnreadMsg":
      case "cmd_previousFlaggedMsg":
      case "button_goForward":
      case "button_goBack":
      case "cmd_goForward":
      case "cmd_goBack":
      case "cmd_goStartPage":
      case "cmd_viewAllMsgs":
      case "cmd_viewUnreadMsgs":
      case "cmd_viewThreadsWithUnread":
      case "cmd_viewWatchedThreadsWithUnread":
      case "cmd_viewIgnoredThreads":
      case "cmd_undo":
      case "cmd_redo":
      case "cmd_expandAllThreads":
      case "cmd_collapseAllThreads":
      case "cmd_renameFolder":
      case "cmd_sendUnsentMsgs":
      case "cmd_openMessage":
      case "button_print":
      case "cmd_print":
      case "cmd_printpreview":
      case "cmd_printSetup":
      case "cmd_saveAsFile":
      case "cmd_saveAsTemplate":
      case "cmd_properties":
      case "cmd_viewPageSource":
      case "cmd_setFolderCharset":
      case "cmd_reload":
      case "button_getNewMessages":
      case "cmd_getNewMessages":
      case "cmd_getMsgsForAuthAccounts":
      case "cmd_getNextNMessages":
      case "cmd_find":
      case "cmd_findAgain":
      case "cmd_findPrevious":
      case "cmd_search":
      case "button_mark":
      case "cmd_tag":
      case "cmd_markAsRead":
      case "cmd_markAllRead":
      case "cmd_markThreadAsRead":
      case "cmd_markReadByDate":
      case "cmd_markAsFlagged":
      case "cmd_markAsJunk":
      case "cmd_markAsNotJunk":
      case "cmd_recalculateJunkScore":
      case "cmd_applyFilters":
      case "cmd_runJunkControls":
      case "cmd_deleteJunk":
      case "button_file":
      case "cmd_file":
      case "cmd_emptyTrash":
      case "cmd_compactFolder":
      case "button_compact":
      case "cmd_settingsOffline":
      case "cmd_close":
      case "cmd_selectAll":
      case "cmd_selectThread":
      case "cmd_moveToFolderAgain":
        return true;
      case "cmd_downloadFlagged":
      case "cmd_downloadSelected":
      case "cmd_synchronizeOffline":
        return MailOfflineMgr.isOnline();

      case "cmd_watchThread":
      case "cmd_killThread":
        return(isNewsURI(GetFirstSelectedMessage()));

      default:
        return false;
    }
  },

  isCommandEnabled: function(command)
  {
    var enabled = new Object();
    enabled.value = false;
    var checkStatus = new Object();

    switch ( command )
    {
      case "cmd_delete":
        UpdateDeleteCommand();
        // fall through
      case "button_delete":
        if (gDBView)
          gDBView.getCommandStatus(nsMsgViewCommandType.deleteMsg, enabled, checkStatus);
        return enabled.value;
      case "cmd_shiftDelete":
        if (gDBView)
          gDBView.getCommandStatus(nsMsgViewCommandType.deleteNoTrash, enabled, checkStatus);
        return enabled.value;
      case "button_junk":
        UpdateJunkToolbarButton();
        if (gDBView)
          gDBView.getCommandStatus(nsMsgViewCommandType.junk, enabled, checkStatus);
        return enabled.value;
      case "cmd_killThread":
        return GetNumSelectedMessages() > 0;
      case "cmd_watchThread":
        if (gDBView)
          gDBView.getCommandStatus(nsMsgViewCommandType.toggleThreadWatched, enabled, checkStatus);
        return enabled.value;
      case "cmd_createFilterFromPopup":
      case "cmd_createFilterFromMenu":
        var loadedFolder = GetLoadedMsgFolder();
        if (!(loadedFolder && loadedFolder.server.canHaveFilters))
          return false;   // else fall thru
      case "cmd_saveAsFile":
      case "cmd_saveAsTemplate":
        if (GetNumSelectedMessages() > 1)
          return false;   // else fall thru
      case "cmd_reply":
      case "button_reply":
      case "cmd_replySender":
      case "cmd_replyGroup":
      case "cmd_replyall":
      case "button_replyall":
      case "cmd_forward":
      case "button_forward":
      case "cmd_forwardInline":
      case "cmd_forwardAttachment":
      case "cmd_editAsNew":
      case "cmd_openMessage":
      case "button_print":
      case "cmd_print":
      case "cmd_viewPageSource":
      case "cmd_reload":
        if (GetNumSelectedMessages() > 0)
        {
          if (gDBView)
          {
            gDBView.getCommandStatus(nsMsgViewCommandType.cmdRequiringMsgBody, enabled, checkStatus);
            return enabled.value;
          }
        }
        return false;
      case "cmd_printpreview":
        if ( GetNumSelectedMessages() == 1 && gDBView)
        {
           gDBView.getCommandStatus(nsMsgViewCommandType.cmdRequiringMsgBody, enabled, checkStatus);
           return enabled.value;
        }
        return false;
      case "cmd_printSetup":
        return true;
      case "cmd_markAsFlagged":
      case "button_file":
      case "cmd_file":
        return (GetNumSelectedMessages() > 0 );
      case "cmd_markAsJunk":
      case "cmd_markAsNotJunk":
        // can't do news on junk yet.
        return (GetNumSelectedMessages() > 0 && !isNewsURI(GetFirstSelectedMessage()));
      case "cmd_recalculateJunkScore":
        if (GetNumSelectedMessages() > 0)
          gDBView.getCommandStatus(nsMsgViewCommandType.runJunkControls, enabled, checkStatus);
        return enabled.value;
      case "cmd_applyFilters":
        if (gDBView)
          gDBView.getCommandStatus(nsMsgViewCommandType.applyFilters, enabled, checkStatus);
        return enabled.value;
      case "cmd_runJunkControls":
        if (gDBView)
          gDBView.getCommandStatus(nsMsgViewCommandType.runJunkControls, enabled, checkStatus);
        return enabled.value;
      case "cmd_deleteJunk":
        if (gDBView)
          gDBView.getCommandStatus(nsMsgViewCommandType.deleteJunk, enabled, checkStatus);
        return enabled.value;
      case "button_mark":
      case "cmd_tag":
      case "cmd_markAsRead":
      case "cmd_markThreadAsRead":
        return GetNumSelectedMessages() > 0;
      case "button_previous":
      case "button_next":
        return IsViewNavigationItemEnabled();
      case "cmd_nextMsg":
      case "cmd_nextUnreadMsg":
      case "cmd_nextUnreadThread":
      case "cmd_previousMsg":
      case "cmd_previousUnreadMsg":
        return IsViewNavigationItemEnabled();
      case "button_goForward":
      case "button_goBack":
      case "cmd_goForward":
      case "cmd_goBack":
        if (gDBView)
          enabled.value = gDBView.navigateStatus((command == "cmd_goBack" || command == "button_goBack") ? nsMsgNavigationType.back : nsMsgNavigationType.forward);
        return enabled.value;
      case "cmd_goStartPage":
        return pref.getBoolPref("mailnews.start_page.enabled") && !IsMessagePaneCollapsed();
      case "cmd_markAllRead":
      case "cmd_markReadByDate":
        return IsFolderSelected();
      case "cmd_find":
      case "cmd_findAgain":
      case "cmd_findPrevious":
        return IsMessageDisplayedInMessagePane();
        break;
      case "cmd_search":
        return IsCanSearchMessagesEnabled();
      case "cmd_selectAll":
        return gDBView != null;
      // these are enabled on when we are in threaded mode
      case "cmd_selectThread":
        if (GetNumSelectedMessages() <= 0) return false;
      case "cmd_expandAllThreads":
      case "cmd_collapseAllThreads":
        if (!gDBView || !gDBView.supportsThreading)
          return false;
        return (gDBView.viewFlags & nsMsgViewFlagsType.kThreadedDisplay);
      case "cmd_nextFlaggedMsg":
      case "cmd_previousFlaggedMsg":
        return IsViewNavigationItemEnabled();
      case "cmd_viewAllMsgs":
      case "cmd_viewUnreadMsgs":
      case "cmd_viewThreadsWithUnread":
      case "cmd_viewWatchedThreadsWithUnread":
      case "cmd_viewIgnoredThreads":
        return gDBView;
      case "cmd_stop":
        return true;
      case "cmd_undo":
      case "cmd_redo":
          return SetupUndoRedoCommand(command);
      case "cmd_renameFolder":
        return IsRenameFolderEnabled();
      case "cmd_sendUnsentMsgs":
        return IsSendUnsentMsgsEnabled(null);
      case "cmd_properties":
        return IsPropertiesEnabled(command);
      case "button_getNewMessages":
      case "cmd_getNewMessages":
      case "cmd_getMsgsForAuthAccounts":
        return IsGetNewMessagesEnabled();
      case "cmd_getNextNMessages":
        return IsGetNextNMessagesEnabled();
      case "cmd_emptyTrash":
        return IsEmptyTrashEnabled();
      case "button_compact":
      case "cmd_compactFolder":
        return IsCompactFolderEnabled();
      case "cmd_setFolderCharset":
        return IsFolderCharsetEnabled();
      case "cmd_close":
        return true;
      case "cmd_downloadFlagged":
        return(IsFolderSelected() && MailOfflineMgr.isOnline());
      case "cmd_downloadSelected":
        return (IsFolderSelected() && MailOfflineMgr.isOnline() && GetNumSelectedMessages() > 0);
      case "cmd_synchronizeOffline":
        return MailOfflineMgr.isOnline() && IsAccountOfflineEnabled();
      case "cmd_settingsOffline":
        return IsAccountOfflineEnabled();
      case "cmd_moveToFolderAgain":
        return (pref.getCharPref("mail.last_msg_movecopy_target_uri") && GetNumSelectedMessages() > 0);
      default:
        return false;
    }
    return false;
  },

  doCommand: function(command)
  {
    // if the user invoked a key short cut then it is possible that we got here for a command which is
    // really disabled. kick out if the command should be disabled.
    if (!this.isCommandEnabled(command)) return;

    switch ( command )
    {
      case "cmd_close":
        CloseMailWindow();
        break;
      case "button_getNewMessages":
      case "cmd_getNewMessages":
        MsgGetMessage();
        break;
      case "cmd_getMsgsForAuthAccounts":
        MsgGetMessagesForAllAuthenticatedAccounts();
        break;
      case "cmd_getNextNMessages":
        MsgGetNextNMessages();
        break;
      case "cmd_reply":
        MsgReplyMessage(null);
        break;
      case "cmd_replySender":
        MsgReplySender(null);
        break;
      case "cmd_replyGroup":
        MsgReplyGroup(null);
        break;
      case "cmd_replyall":
        MsgReplyToAllMessage(null);
        break;
      case "cmd_forward":
        MsgForwardMessage(null);
        break;
      case "cmd_forwardInline":
        MsgForwardAsInline(null);
        break;
      case "cmd_forwardAttachment":
        MsgForwardAsAttachment(null);
        break;
      case "cmd_editAsNew":
        MsgEditMessageAsNew();
        break;
      case "cmd_createFilterFromMenu":
        MsgCreateFilter();
        break;
      case "cmd_createFilterFromPopup":
        break;// This does nothing because the createfilter is invoked from the popupnode oncommand.
      case "button_delete":
      case "cmd_delete":
         // if the user deletes a message before its mark as read timer goes off, we should mark it as read
         // this ensures that we clear the biff indicator from the system tray when the user deletes the new message
        if (gMarkViewedMessageAsReadTimer)
          MarkCurrentMessageAsRead();
        SetNextMessageAfterDelete();
        gDBView.doCommand(nsMsgViewCommandType.deleteMsg);
        break;
      case "cmd_shiftDelete":
        if (gMarkViewedMessageAsReadTimer)
          MarkCurrentMessageAsRead();
        SetNextMessageAfterDelete();
        gDBView.doCommand(nsMsgViewCommandType.deleteNoTrash);
        break;
      case "cmd_killThread":
        /* kill thread kills the thread and then does a next unread */
        GoNextMessage(nsMsgNavigationType.toggleThreadKilled, true);
        break;
      case "cmd_watchThread":
        gDBView.doCommand(nsMsgViewCommandType.toggleThreadWatched);
        break;
      case "button_next":
      case "cmd_nextUnreadMsg":
        MsgNextUnreadMessage();
        break;
      case "cmd_nextUnreadThread":
        MsgNextUnreadThread();
        break;
      case "cmd_nextMsg":
        MsgNextMessage();
        break;
      case "cmd_nextFlaggedMsg":
        MsgNextFlaggedMessage();
        break;
      case "cmd_previousMsg":
        MsgPreviousMessage();
        break;
      case "button_previous":
      case "cmd_previousUnreadMsg":
        MsgPreviousUnreadMessage();
        break;
      case "cmd_previousFlaggedMsg":
        MsgPreviousFlaggedMessage();
        break;
      case "button_goForward":
      case "cmd_goForward":
        MsgGoForward();
        break;
      case "button_goBack":
      case "cmd_goBack":
        MsgGoBack();
        break;
      case "cmd_goStartPage":
        HideMessageHeaderPane();
        loadStartPage();
        break;
      case "cmd_viewAllMsgs":
      case "cmd_viewThreadsWithUnread":
      case "cmd_viewWatchedThreadsWithUnread":
      case "cmd_viewUnreadMsgs":
      case "cmd_viewIgnoredThreads":
        SwitchView(command);
        break;
      case "cmd_undo":
        messenger.undo(msgWindow);
        break;
      case "cmd_redo":
        messenger.redo(msgWindow);
        break;
      case "cmd_expandAllThreads":
        gDBView.doCommand(nsMsgViewCommandType.expandAll);
        break;
      case "cmd_collapseAllThreads":
        gDBView.doCommand(nsMsgViewCommandType.collapseAll);
        break;
      case "cmd_renameFolder":
        MsgRenameFolder();
        return;
      case "cmd_sendUnsentMsgs":
        MsgSendUnsentMsgs();
        return;
      case "cmd_openMessage":
        MsgOpenSelectedMessages();
        return;
      case "cmd_printSetup":
        PrintUtils.showPageSetup();
        return;
      case "cmd_print":
        PrintEnginePrint();
        return;
      case "cmd_printpreview":
        PrintEnginePrintPreview();
        return;
      case "cmd_saveAsFile":
        MsgSaveAsFile();
        return;
      case "cmd_saveAsTemplate":
        MsgSaveAsTemplate();
        return;
      case "cmd_viewPageSource":
        MsgViewPageSource();
        return;
      case "cmd_setFolderCharset":
        MsgFolderProperties();
        return;
      case "cmd_reload":
        MsgReload();
        return;
      case "cmd_find":
        // make sure the message pane has focus before we start a find since we only support searching
        // within the message body. Do it here and not in MsgFind() which can be called from standalone where we don't want to set focus
        SetFocusMessagePane();
        MsgFind();
        return;
      case "cmd_findAgain":
        // make sure the message pane has focus before we start a find since we only support searching
        // within the message body. Do it here and not in MsgFind() which can be called from standalone where we don't want to set focus
        SetFocusMessagePane();
        MsgFindAgain(false);
        return;
      case "cmd_findPrevious":
        // make sure the message pane has focus before we start a find since we only support searching
        // within the message body. Do it here and not in MsgFind() which can be called from standalone where we don't want to set focus
        SetFocusMessagePane();
        MsgFindAgain(true);
        return;
      case "cmd_markReadByDate":
        MsgMarkReadByDate();
        return;
      case "cmd_properties":
        MsgFolderProperties();
        return;
      case "cmd_search":
        MsgSearchMessages();
        return;
      case "button_mark":
      case "cmd_markAsRead":
        MsgMarkMsgAsRead(null);
        return;
      case "cmd_markThreadAsRead":
        MsgMarkThreadAsRead();
        return;
      case "cmd_markAllRead":
        gDBView.doCommand(nsMsgViewCommandType.markAllRead);
        return;
      case "button_junk":
        MsgJunk();
        return;
      case "cmd_stop":
        MsgStop();
        return;
      case "cmd_markAsFlagged":
        MsgMarkAsFlagged(null);
        return;
      case "cmd_markAsJunk":
        JunkSelectedMessages(true);
        return;
      case "cmd_markAsNotJunk":
        JunkSelectedMessages(false);
        return;
      case "cmd_recalculateJunkScore":
        analyzeMessagesForJunk();
        return;
      case "cmd_applyFilters":
        MsgApplyFilters(null);
        return;
      case "cmd_runJunkControls":
        filterFolderForJunk();
        return;
      case "cmd_deleteJunk":
        deleteJunkInFolder();
        return;
      case "cmd_emptyTrash":
        MsgEmptyTrash();
        return;
      case "cmd_compactFolder":
        MsgCompactFolder(true);
        return;
      case "button_compact":
        MsgCompactFolder(false);
        return;
      case "cmd_downloadFlagged":
          MsgDownloadFlagged();
          break;
      case "cmd_downloadSelected":
          MsgDownloadSelected();
          break;
      case "cmd_synchronizeOffline":
          MsgSynchronizeOffline();
          break;
      case "cmd_settingsOffline":
          MailOfflineMgr.openOfflineAccountSettings();
          break;
      case "cmd_moveToFolderAgain":
          var folderId = pref.getCharPref("mail.last_msg_movecopy_target_uri");
          if (pref.getBoolPref("mail.last_msg_movecopy_was_move"))
            MsgMoveMessage(GetMsgFolderFromUri(folderId));
          else
            MsgCopyMessage(GetMsgFolderFromUri(folderId));
          break;
      case "cmd_selectAll":
          // move the focus so the user can delete the newly selected messages, not the folder
          SetFocusThreadPane();
          // if in threaded mode, the view will expand all before selecting all
          gDBView.doCommand(nsMsgViewCommandType.selectAll)
          if (gDBView.numSelected != 1) {
              setTitleFromFolder(gDBView.msgFolder,null);
              ClearMessagePane();
          }
          break;
      case "cmd_selectThread":
          gDBView.doCommand(nsMsgViewCommandType.selectThread);
          break;
    }
  },

  onEvent: function(event)
  {
    // on blur events set the menu item texts back to the normal values
    if ( event == 'blur' )
    {
      goSetMenuValue('cmd_undo', 'valueDefault');
      goSetMenuValue('cmd_redo', 'valueDefault');
    }
  }
};

function GetNumSelectedMessages()
{
    try {
        return gDBView.numSelected;
    }
    catch (ex) {
        return 0;
    }
}

var gLastFocusedElement=null;

function FocusRingUpdate_Mail()
{
  // WhichPaneHasFocus() uses on top.document.commandDispatcher.focusedElement
  // to determine which pane has focus
  // if the focusedElement is null, we're here on a blur.
  // nsFocusController::Blur() calls nsFocusController::SetFocusedElement(null),
  // which will update any commands listening for "focus".
  // we really only care about nsFocusController::Focus() happens,
  // which calls nsFocusController::SetFocusedElement(element)
  var currentFocusedElement = WhichPaneHasFocus();

  if (currentFocusedElement != gLastFocusedElement) {
    if (currentFocusedElement)
      currentFocusedElement.setAttribute("focusring", "true");

    if (gLastFocusedElement)
      gLastFocusedElement.removeAttribute("focusring");

    gLastFocusedElement = currentFocusedElement;

    // since we just changed the pane with focus we need to update the toolbar to reflect this
    // XXX TODO
    // can we optimize
    // and just update cmd_delete and button_delete?
    UpdateMailToolbar("focus");
  }
}

function WhichPaneHasFocus()
{
  var threadTree = GetThreadTree();
  var folderTree = GetFolderTree();
  var messagePane = GetMessagePane();

  if (top.document.commandDispatcher.focusedWindow == GetMessagePaneFrame())
    return messagePane;

  var currentNode = top.document.commandDispatcher.focusedElement;
  while (currentNode) {
    if (currentNode === threadTree ||
        currentNode === folderTree ||
        currentNode === messagePane)
      return currentNode;

    currentNode = currentNode.parentNode;
  }

  return null;
}

function SetupCommandUpdateHandlers()
{
  // folder pane
  var widget = GetFolderTree();
  if ( widget )
    widget.controllers.appendController(FolderPaneController);

  top.controllers.insertControllerAt(0, DefaultController);
}

function UnloadCommandUpdateHandlers()
{
  top.controllers.removeController(DefaultController);
}

function IsSendUnsentMsgsEnabled(folderResource)
{
  var identity;

  if (messenger.sendingUnsentMsgs) // if we're currently sending unsent msgs, disable this cmd.
    return false;
  try {
    if (folderResource) {
      // if folderResource is non-null, it is
      // resource for the "Unsent Messages" folder
      // we're here because we've done a right click on the "Unsent Messages"
      // folder (context menu)
      var msgFolder = folderResource.QueryInterface(Components.interfaces.nsIMsgFolder);
      return (msgFolder.getTotalMessages(false) > 0);
    }
    else {
      var folders = GetSelectedMsgFolders();
      if (folders.length > 0) {
        identity = getIdentityForServer(folders[0].server);
      }
    }
  }
  catch (ex) {
    dump("ex = " + ex + "\n");
    identity = null;
  }

  try {
    if (!identity) {
      var am = Components.classes["@mozilla.org/messenger/account-manager;1"].getService(Components.interfaces.nsIMsgAccountManager);
      identity = am.defaultAccount.defaultIdentity;
    }

    var msgSendlater = Components.classes["@mozilla.org/messengercompose/sendlater;1"].getService(Components.interfaces.nsIMsgSendLater);
    var unsentMsgsFolder = msgSendlater.getUnsentMessagesFolder(identity);
    return (unsentMsgsFolder.getTotalMessages(false) > 0);
  }
  catch (ex) {
    dump("ex = " + ex + "\n");
  }
  return false;
}

function IsRenameFolderEnabled()
{
    var folderTree = GetFolderTree();
    var selection = folderTree.view.selection;
    if (selection.count == 1)
    {
        var startIndex = {};
        var endIndex = {};
        selection.getRangeAt(0, startIndex, endIndex);
        var folderResource = GetFolderResource(folderTree, startIndex.value);
        var canRename = GetFolderAttribute(folderTree, folderResource, "CanRename") == "true";
        return canRename && isCommandEnabled("cmd_renameFolder");
    }
    else
        return false;
}

function IsCanSearchMessagesEnabled()
{
  var folderURI = GetSelectedFolderURI();
  if (!folderURI)
    return false;
  var server = GetServer(folderURI);
  return server.canSearchMessages;
}
function IsFolderCharsetEnabled()
{
  return IsFolderSelected();
}

function IsPropertiesEnabled(command)
{
   try
   {
      var folderTree = GetFolderTree();
      var folderResource = GetSelectedFolderResource();

      // when servers are selected
      // it should be "Edit | Properties..."
      if (GetFolderAttribute(folderTree, folderResource, "IsServer") == "true")
        goSetMenuValue(command, "valueGeneric");
      else
        goSetMenuValue(command, isNewsURI(folderResource.Value) ? "valueNewsgroup" : "valueFolder");
   }
   catch (ex)
   {
      // properties menu failure
   }

   var selection = folderTree.view.selection;
   return (selection.count == 1);
}

function IsViewNavigationItemEnabled()
{
  return IsFolderSelected();
}

function IsFolderSelected()
{
    var folderTree = GetFolderTree();
    var selection = folderTree.view.selection;
    if (selection.count == 1)
    {
        var startIndex = {};
        var endIndex = {};
        selection.getRangeAt(0, startIndex, endIndex);
        var folderResource = GetFolderResource(folderTree, startIndex.value);
        return GetFolderAttribute(folderTree, folderResource, "IsServer") != "true";
    }
    else
        return false;
}

function IsMessageDisplayedInMessagePane()
{
  return (!IsMessagePaneCollapsed() && (GetNumSelectedMessages() > 0));
}

function MsgDeleteFolder()
{
    var folderTree = GetFolderTree();
    var selectedFolders = GetSelectedMsgFolders();
    for (var i = 0; i < selectedFolders.length; i++)
    {
        var selectedFolder = selectedFolders[i];
        var folderResource = selectedFolder.QueryInterface(Components.interfaces.nsIRDFResource);
        var specialFolder = GetFolderAttribute(folderTree, folderResource, "SpecialFolder");
        if (specialFolder != "Inbox" && specialFolder != "Trash")
        {
            var folder = selectedFolder.QueryInterface(Components.interfaces.nsIMsgFolder);
            if (folder.flags & MSG_FOLDER_FLAG_VIRTUAL)
            {
                if (gCurrentVirtualFolderUri == folderResource.Value)
                  gCurrentVirtualFolderUri = null;
                var array = Components.classes["@mozilla.org/array;1"]
                                      .createInstance(Components.interfaces.nsIMutableArray);
                array.appendElement(folder, false);
                folder.parent.deleteSubFolders(array, msgWindow);
                continue;
            }
            var protocolInfo = Components.classes["@mozilla.org/messenger/protocol/info;1?type=" + selectedFolder.server.type].getService(Components.interfaces.nsIMsgProtocolInfo);

            // do not allow deletion of special folders on imap accounts
            if ((specialFolder == "Sent" ||
                specialFolder == "Drafts" ||
                specialFolder == "Templates" ||
                (specialFolder == "Junk" && !CanRenameDeleteJunkMail(GetSelectedFolderURI()))) &&
                !protocolInfo.specialFoldersDeletionAllowed)
            {
                var errorMessage = gMessengerBundle.getFormattedString("specialFolderDeletionErr",
                                                    [specialFolder]);
                var specialFolderDeletionErrTitle = gMessengerBundle.getString("specialFolderDeletionErrTitle");
                var promptService = Components.classes["@mozilla.org/embedcomp/prompt-service;1"]
                                              .getService(Components.interfaces.nsIPromptService);

                promptService.alert(window, specialFolderDeletionErrTitle, errorMessage);
                continue;
            }
            else if (isNewsURI(folderResource.Value))
            {
                var unsubscribe = ConfirmUnsubscribe(selectedFolder);
                if (unsubscribe)
                    UnSubscribe(selectedFolder);
            }
            else
            {
                var array = Components.classes["@mozilla.org/array;1"]
                                      .createInstance(Components.interfaces.nsIMutableArray);
                array.appendElement(selectedFolder, false);
                selectedFolder.parent.deleteSubFolders(array, msgWindow);
            }
        }
    }
}

function SetFocusThreadPaneIfNotOnMessagePane()
{
  var focusedElement = WhichPaneHasFocus();

  if((focusedElement != GetThreadTree()) &&
     (focusedElement != GetMessagePane()))
     SetFocusThreadPane();
}

// 3pane related commands.  Need to go in own file.  Putting here for the moment.
function MsgNextMessage()
{
  GoNextMessage(nsMsgNavigationType.nextMessage, false );
}

function MsgNextUnreadMessage()
{
  GoNextMessage(nsMsgNavigationType.nextUnreadMessage, true);
}
function MsgNextFlaggedMessage()
{
  GoNextMessage(nsMsgNavigationType.nextFlagged, true);
}

function MsgNextUnreadThread()
{
  GoNextMessage(nsMsgNavigationType.nextUnreadThread, true);
}

function MsgPreviousMessage()
{
    GoNextMessage(nsMsgNavigationType.previousMessage, false);
}

function MsgPreviousUnreadMessage()
{
  GoNextMessage(nsMsgNavigationType.previousUnreadMessage, true);
}

function MsgPreviousFlaggedMessage()
{
  GoNextMessage(nsMsgNavigationType.previousFlagged, true);
}

function MsgGoForward()
{
  GoNextMessage(nsMsgNavigationType.forward, true);
}

function MsgGoBack()
{
  GoNextMessage(nsMsgNavigationType.back, true);
}

function SwitchPaneFocus(event)
{
  var folderTree = GetFolderTree();
  var threadTree = GetThreadTree();
  var messagePane = GetMessagePane();

  var focusedElement = WhichPaneHasFocus();
  if (focusedElement == null)       // focus not on one of the main three panes (probably toolbar)
    focusedElement = threadTree;    // treat as if on thread tree

  if (event && event.shiftKey)
  {
    // Reverse traversal: Message -> Thread -> Folder -> Message
    if (focusedElement == threadTree && !IsFolderPaneCollapsed())
      folderTree.focus();
    else if (focusedElement != messagePane && !IsMessagePaneCollapsed())
      SetFocusMessagePane();
    else
      threadTree.focus();
  }
  else
  {
    // Forward traversal: Folder -> Thread -> Message -> Folder
    if (focusedElement == threadTree && !IsMessagePaneCollapsed())
      SetFocusMessagePane();
    else if (focusedElement != folderTree && !IsFolderPaneCollapsed())
      folderTree.focus();
    else
      threadTree.focus();
  }
}

function SetFocusThreadPane()
{
    var threadTree = GetThreadTree();
    threadTree.focus();
}

function SetFocusMessagePane()
{
  // messagePaneFrame.focus() fails to blur the currently focused element
  document.commandDispatcher.advanceFocusIntoSubtree(GetMessagePane());
}

function isCommandEnabled(cmd)
{
  var selectedFolders = GetSelectedMsgFolders();
  var numFolders = selectedFolders.length;
  if(numFolders !=1)
    return false;

  var folder = selectedFolders[0];
  if (!folder)
    return false;
  else
    return folder.isCommandEnabled(cmd);

}

//
// This function checks if the configured junk mail can be renamed or deleted.
//
function CanRenameDeleteJunkMail(aFolderUri)
{
  if (!aFolderUri)
    return false;

  // Go through junk mail settings for all servers and see if the folder is set/used by anyone.
  try
  {
    var allServers = accountManager.allServers;

    for (var i=0;i<allServers.Count();i++)
    {
      var currentServer = allServers.GetElementAt(i).QueryInterface(Components.interfaces.nsIMsgIncomingServer);
      var settings = currentServer.spamSettings;
      // If junk mail control or move junk mail to folder option is disabled then
      // allow the folder to be removed/renamed since the folder is not used in this case.
      if (!settings.level || !settings.moveOnSpam)
        continue;
      if (settings.spamFolderURI == aFolderUri)
        return false;
    }
  }
  catch(ex)
  {
      dump("Can't get all servers\n");
  }
  return true;
}
