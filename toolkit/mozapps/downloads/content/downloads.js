///////////////////////////////////////////////////////////////////////////////
// Globals

const kObserverServiceProgID = "@mozilla.org/observer-service;1";
const NC_NS = "http://home.netscape.com/NC-rdf#";

var gDownloadManager  = null;
var gDownloadListener = null;
var gDownloadsView    = null;
var gUserInterfered   = false;
var gActiveDownloads  = [];

///////////////////////////////////////////////////////////////////////////////
// Utility Functions 
function setRDFProperty(aID, aProperty, aValue)
{
  var rdf = Components.classes["@mozilla.org/rdf/rdf-service;1"].getService(Components.interfaces.nsIRDFService);

  var db = gDownloadManager.datasource;
  var propertyArc = rdf.GetResource(NC_NS + aProperty);
  
  var res = rdf.GetResource(aID);
  var node = db.GetTarget(res, propertyArc, true);
  if (node)
    db.Change(res, propertyArc, node, rdf.GetLiteral(aValue));
  else
    db.Assert(res, propertyArc, rdf.GetLiteral(aValue), true);
}

function getRDFProperty(aID, aProperty)
{
  var rdf = Components.classes["@mozilla.org/rdf/rdf-service;1"].getService(Components.interfaces.nsIRDFService);

  var db = gDownloadManager.datasource;
  var propertyArc = rdf.GetResource(NC_NS + aProperty);
  
  var res = rdf.GetResource(aID);
  var node = db.GetTarget(res, propertyArc, true);
  if (!node) return "";
  try {
    node = node.QueryInterface(Components.interfaces.nsIRDFLiteral);
    return node.Value;
  }
  catch (e) {
    try {
      node = node.QueryInterface(Components.interfaces.nsIRDFInt);
      return node.Value;
    }
    catch (e) {
      node = node.QueryInterface(Components.interfaces.nsIRDFResource);
      return node.Value;
    }
  }
  return "";
}

function fireEventForElement(aElement, aEventType)
{
  var e = document.createEvent("Events");
  e.initEvent("download-" + aEventType, false, true);
  
  aElement.dispatchEvent(e);
}

///////////////////////////////////////////////////////////////////////////////
// Start/Stop Observers
function downloadCompleted(aDownload) 
{
  // Wrap this in try...catch since this can be called while shutting down... 
  // it doesn't really matter if it fails then since well.. we're shutting down
  // and there's no UI to update!
  try {
    var rdf = Components.classes["@mozilla.org/rdf/rdf-service;1"].getService(Components.interfaces.nsIRDFService);
    var rdfc = Components.classes["@mozilla.org/rdf/container;1"].createInstance(Components.interfaces.nsIRDFContainer);

    var db = gDownloadManager.datasource;
    
    rdfc.Init(db, rdf.GetResource("NC:DownloadsRoot"));

    var id = aDownload.target.persistentDescriptor;
    var dlRes = rdf.GetUnicodeResource(id);
  
    var insertIndex = gDownloadManager.activeDownloadCount + 1;
    // Don't bother inserting the item into the same place!
    if (insertIndex != rdfc.IndexOf(dlRes)) {
      rdfc.RemoveElement(dlRes, true);
      if (insertIndex == rdfc.GetCount() || insertIndex < 1) 
        rdfc.AppendElement(dlRes);
      else
        rdfc.InsertElementAt(dlRes, insertIndex, true);      
    }
        
    // Remove the download from our book-keeping list and if the count
    // falls to zero, update the title here, since we won't be getting
    // any more progress notifications in which to do it. 
    for (var i = 0; i < gActiveDownloads.length; ++i) {
      if (gActiveDownloads[i] == aDownload) {
        gActiveDownloads.splice(i, 1);
        break;
      }
    }
    if (gActiveDownloads.length == 0)
      window.title = document.documentElement.getAttribute("statictitle");    
  }
  catch (e) {
  }
}

function autoClose(aDownload)
{
  if (gDownloadManager.activeDownloadCount == 0) {
    // For the moment, just use the simple heuristic that if this window was
    // opened by the download process, rather than by the user, it should auto-close
    // if the pref is set that way. If the user opened it themselves, it should
    // not close until they explicitly close it. 
    // We may like to revisit this in a bit more detail later, perhaps we want
    // to keep it up if the user messes with it in a significant way.
    var pref = Components.classes["@mozilla.org/preferences-service;1"]
                        .getService(Components.interfaces.nsIPrefBranch);
    var autoClose = pref.getBoolPref("browser.download.closeWhenDone")
    if (autoClose && (!window.opener || window.opener.location.href == window.location.href))
      window.close();
  }
}

var gDownloadObserver = {
  observe: function (aSubject, aTopic, aState) 
  {
    var dl = aSubject.QueryInterface(Components.interfaces.nsIDownload);
    switch (aTopic) {
    case "dl-done":
      downloadCompleted(dl);
      
      autoClose(dl);      
      break;
    case "dl-failed":
    case "dl-cancel":
      downloadCompleted(dl);
      break;
    case "dl-start":
      // Add this download to the percentage average tally
      gActiveDownloads.push(dl);
      
      break;
      
    }
  }
};

///////////////////////////////////////////////////////////////////////////////
// Download Event Handlers

function onDownloadCancel(aEvent)
{
  gDownloadManager.cancelDownload(aEvent.target.id);

  setRDFProperty(aEvent.target.id, "DownloadAnimated", "false");

  // XXXben - 
  // If we got here because we resumed the download, we weren't using a temp file
  // because we used saveURL instead. (this is because the proper download mechanism
  // employed by the helper app service isn't fully accessible yet... should be fixed...
  // talk to bz...)
  // the upshot is we have to delete the file if it exists. 
  var f = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
  f.initWithPath(aEvent.target.id);
  if (f.exists()) 
    f.remove(true);

  gDownloadViewController.onCommandUpdate();
}

function onDownloadPause(aEvent)
{
  var uri = aEvent.target.id;
  gDownloadManager.pauseDownload(uri);
  setRDFProperty(uri, "DownloadStatus", aEvent.target.getAttribute("status-internal"));
  setRDFProperty(uri, "ProgressPercent", aEvent.target.getAttribute("progress"));
}

function onDownloadResume(aEvent)
{
  gDownloadManager.resumeDownload(aEvent.target.id);
}

function onDownloadRemove(aEvent)
{
  if (aEvent.target.removable) {
    gDownloadManager.removeDownload(aEvent.target.id);
    
    gDownloadViewController.onCommandUpdate();
  }
}

function onDownloadShow(aEvent)
{
  var f = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
  f.initWithPath(aEvent.target.id);

  if (f.exists()) {
#ifdef XP_UNIX
    // on unix, open a browser window rooted at the parent
    var parent = f.parent;
    if (parent) {
      var pref = Components.classes["@mozilla.org/preferences-service;1"]
                          .getService(Components.interfaces.nsIPrefBranch);
      var browserURL = pref.getCharPref("browser.chromeURL");                          
      window.openDialog(browserURL, "_blank", "chrome,all,dialog=no", parent.path);
    }
#else
    f.reveal();
#endif
  }
}

function onDownloadOpen(aEvent)
{
  var download = aEvent.target;
  if (download.localName == "download") {
    if (download.openable) {
      var f = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
      f.initWithPath(aEvent.target.id);

      if (f.exists()) {
        // XXXben security check!  
        f.launch();
      }
    }
    else if(download.canceledOrFailed) {
      // If the user canceled this download, double clicking tries again. 
      fireEventForElement(download, "retry")
    }
  }
}

function onDownloadOpenWith(aEvent)
{
}

function onDownloadProperties(aEvent)
{
  window.openDialog("chrome://mozapps/content/downloads/downloadProperties.xul",
                    "_blank", "modal,centerscreen,chrome,resizable=no", aEvent.target.id);
}

function onDownloadAnimated(aEvent)
{
  gDownloadViewController.onCommandUpdate();    

  setRDFProperty(aEvent.target.id, "DownloadAnimated", "true");
}

function onDownloadRetry(aEvent)
{
  var download = aEvent.target;
  if (download.localName == "download") {
    var src = getRDFProperty(download.id, "URL");
    saveURL(src, download.getAttribute("target"), null, true, true);
  }
  
  gDownloadViewController.onCommandUpdate();
}

// This is called by the progress listener. We don't actually use the event
// system here to minimize time wastage. 
var gLastComputedMean = 0;
function onUpdateProgress()
{
  var numActiveDownloads = gActiveDownloads.length;
  if (numActiveDownloads == 0) {
    window.title = document.documentElement.getAttribute("statictitle");
    return;
  }
    
  var mean = 0;
  for (var i = 0; i < numActiveDownloads; ++i) {
    var dl = gActiveDownloads[i];
    var progress = dl.percentComplete;
    if (progress < 100)
      mean += progress;
  }

  mean = Math.round(mean / numActiveDownloads);
  if (mean != gLastComputedMean) {
    gLastComputedMean = mean;
    var strings = document.getElementById("downloadStrings");
    
    var title;
    if (numActiveDownloads > 1)
      title = strings.getFormattedString("downloadsTitleMultiple", [mean, numActiveDownloads]);
    else
      title = strings.getFormattedString("downloadsTitle", [mean]);

    window.title = title;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Startup, Shutdown
function Startup() 
{
  gDownloadsView = document.getElementById("downloadView");

  const dlmgrContractID = "@mozilla.org/download-manager;1";
  const dlmgrIID = Components.interfaces.nsIDownloadManager;
  gDownloadManager = Components.classes[dlmgrContractID].getService(dlmgrIID);
  
  // The DownloadProgressListener (DownloadProgressListener.js) handles progress
  // notifications. 
  var downloadStrings = document.getElementById("downloadStrings");
  gDownloadListener = new DownloadProgressListener(document, downloadStrings);
  gDownloadManager.listener = gDownloadListener;

  // The active downloads list is created by the front end only when the download starts,  
  // so we need to pre-populate it with any downloads that were already going. 
  var activeDownloads = gDownloadManager.activeDownloads;
  var count = activeDownloads.Count();
  for (var i = 0; i < count; ++i)
    gActiveDownloads.push(activeDownloads.QueryElementAt(i, Components.interfaces.nsIDownload));

  // Handlers for events generated by the UI (downloads, list view)
  gDownloadsView.addEventListener("download-cancel",      onDownloadCancel,     false);
  gDownloadsView.addEventListener("download-pause",       onDownloadPause,      false);
  gDownloadsView.addEventListener("download-resume",      onDownloadResume,     false);
  gDownloadsView.addEventListener("download-remove",      onDownloadRemove,     false);
  gDownloadsView.addEventListener("download-show",        onDownloadShow,       false);
  gDownloadsView.addEventListener("download-open",        onDownloadOpen,       false);
  gDownloadsView.addEventListener("download-retry",       onDownloadRetry,      false);
  gDownloadsView.addEventListener("download-animated",    onDownloadAnimated,   false);
  gDownloadsView.addEventListener("download-properties",  onDownloadProperties, false);
  gDownloadsView.addEventListener("dblclick",             onDownloadOpen,       false);

  // Handlers for events generated by the Download Manager (download events)
  var observerService = Components.classes[kObserverServiceProgID]
                                  .getService(Components.interfaces.nsIObserverService);
  observerService.addObserver(gDownloadObserver, "dl-done",   false);
  observerService.addObserver(gDownloadObserver, "dl-cancel", false);
  observerService.addObserver(gDownloadObserver, "dl-failed", false);  
  observerService.addObserver(gDownloadObserver, "dl-start", false);  

  // This is for the "Clean Up" button, which requires there to be
  // non-active downloads before it can be enabled. 
  gDownloadsView.controllers.appendController(gDownloadViewController);

  // Finally, update the UI. 
  gDownloadsView.database.AddDataSource(gDownloadManager.datasource);
  gDownloadsView.builder.rebuild();
  
  gDownloadsView.focus();
}

function Shutdown() 
{
  gDownloadManager.listener = null;

  // Assert the current progress for all the downloads in case the window is reopened
  gDownloadManager.saveState();

  var observerService = Components.classes[kObserverServiceProgID]
                                  .getService(Components.interfaces.nsIObserverService);
  observerService.removeObserver(gDownloadObserver, "dl-done");
  observerService.removeObserver(gDownloadObserver, "dl-cancel");
  observerService.removeObserver(gDownloadObserver, "dl-failed");  
  observerService.removeObserver(gDownloadObserver, "dl-start");  
}

///////////////////////////////////////////////////////////////////////////////
// View Context Menus
var gContextMenus = [ 
  ["menuitem_pause", "menuitem_cancel", "menuseparator_properties", "menuitem_properties"],
  ["menuitem_open", "menuitem_show", "menuitem_remove", "menuseparator_properties", "menuitem_properties"],
  ["menuitem_retry", "menuitem_remove", "menuseparator_properties", "menuitem_properties"],
  ["menuitem_retry", "menuitem_remove", "menuseparator_properties", "menuitem_properties"],
  ["menuitem_resume", "menuitem_cancel", "menuseparator_properties", "menuitem_properties"]
];

function buildContextMenu(aEvent)
{
  if (aEvent.target.id != "downloadContextMenu")
    return;
    
  var popup = document.getElementById("downloadContextMenu");
  while (popup.hasChildNodes())
    popup.removeChild(popup.firstChild);
  
  if (gDownloadsView.selected) {
    var idx = parseInt(gDownloadsView.selected.getAttribute("state"));
    if (idx < 0)
      idx = 0;
    
    var menus = gContextMenus[idx];
    for (var i = 0; i < menus.length; ++i)
      popup.appendChild(document.getElementById(menus[i]).cloneNode(true));
    
    return true;
  }
  
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// Drag and Drop

var gDownloadDNDObserver =
{
  onDragOver: function (aEvent, aFlavour, aDragSession)
  {
    aDragSession.canDrop = true;
  },
  
  onDrop: function(aEvent, aXferData, aDragSession)
  {
    var split = aXferData.data.split("\n");
    var url = split[0];
    if (url != aXferData.data) {  //do nothing, not a valid URL
      var name = split[1];
      saveURL(url, name, null, true, true);
    }
  },
  _flavourSet: null,  
  getSupportedFlavours: function ()
  {
    if (!this._flavourSet) {
      this._flavourSet = new FlavourSet();
      this._flavourSet.appendFlavour("text/x-moz-url");
      this._flavourSet.appendFlavour("text/unicode");
    }
    return this._flavourSet;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Command Updating and Command Handlers

var gDownloadViewController = {
  supportsCommand: function (aCommand)
  {
    return aCommand == "cmd_cleanUp";
  },
  
  isCommandEnabled: function (aCommand)
  {
    if (aCommand == "cmd_cleanUp") 
      return gDownloadManager.canCleanUp;
    return false;
  },
  
  doCommand: function (aCommand)
  {
    if (aCommand == "cmd_cleanUp" && this.isCommandEnabled(aCommand)) {
      gDownloadManager.cleanUp();
      this.onCommandUpdate();
    }
  },  
  
  onCommandUpdate: function ()
  {
    var command = "cmd_cleanUp";
    var enabled = this.isCommandEnabled(command);
    goSetCommandEnabled(command, enabled);
  }
};

function onDownloadShowOptions()
{
  var windowManager = Components.classes['@mozilla.org/appshell/window-mediator;1'].getService();
  var windowManagerInterface = windowManager.QueryInterface(Components.interfaces.nsIWindowMediator);
  var topWindow = windowManagerInterface.getMostRecentWindow("Browser:Options");
  if (topWindow) {
    topWindow.focus();
    topWindow.switchPage("catDownloadsButton");
  }
  else
    openDialog("chrome://browser/content/pref/pref.xul", "PrefWindow",
              "chrome,titlebar,resizable,modal", "catDownloadsButton");
}


# -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
# The Original Code is Mozilla.org Code.
# 
# The Initial Developer of the Original Code is
# Doron Rosenberg.
# Portions created by the Initial Developer are Copyright (C) 2001
# the Initial Developer. All Rights Reserved.
# 
# Contributor(s):
#   Blake Ross <blakeross@telocity.com> (Original Author) 
#   Ben Goodger <ben@bengoodger.com> (v2.0) 
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

 
