# -*- Mode: Java; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
# The Original Code is Thunderbird RSS Subscription Manager
#
# The Initial Developer of the Original Code is
# The Mozilla Foundation.
# Portions created by the Initial Developer are Copyright (C) 2005
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#  Scott MacGregor <mscott@mozilla.org>
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
# ***** END LICENSE BLOCK ******

const MSG_FOLDER_FLAG_TRASH = 0x0100;
const IPS = Components.interfaces.nsIPromptService;
const nsIDragService = Components.interfaces.nsIDragService;
const kRowIndexUndefined = -1;

var gFeedSubscriptionsWindow = {
  mFeedContainers   : [],
  mTree             : null,
  mBundle           : null,
  mRSSServer        : null,

  init: function ()
  {  
    // extract the server argument
    if (window.arguments[0].server)
      this.mRSSServer = window.arguments[0].server;
    
    var docshell = window.QueryInterface(Components.interfaces.nsIInterfaceRequestor)
                        .getInterface(Components.interfaces.nsIWebNavigation)
                        .QueryInterface(Components.interfaces.nsIDocShell);        
    docshell.allowAuth = true;

    this.mTree = document.getElementById("rssSubscriptionsList");
    this.mBundle = document.getElementById("bundle_newsblog");
   
    this.loadSubscriptions();
    this.mTree.treeBoxObject.view = this.mView;
    if (this.mView.rowCount > 0) 
      this.mTree.view.selection.select(0);
  },
  
  uninit: function ()
  {
    var dismissDialog = true;

    // if we are in the middle of subscribing to a feed, inform the user that 
    // dismissing the dialog right now will abort the feed subscription.
    // cheat and look at the disabled state of the add button to determine if we are in the middle of a new subscription
    if (document.getElementById('addFeed').getAttribute('disabled'))
    {
      var promptService = Components.classes["@mozilla.org/embedcomp/prompt-service;1"].getService(IPS);
      var newsBlogBundle = document.getElementById("bundle_newsblog");
      dismissDialog = !(promptService.confirmEx(window, newsBlogBundle.getString('subscribe-cancelSubscriptionTitle'), 
                                       newsBlogBundle.getString('subscribe-cancelSubscription'), 
                                       (IPS.BUTTON_TITLE_YES * IPS.BUTTON_POS_0) + (IPS.BUTTON_TITLE_NO * IPS.BUTTON_POS_1),
                                       null, null, null, null, { }));    
    }  
    return dismissDialog;
  },
  
  mView: 
  {
    mRowCount   : 0,

    get rowCount() 
    { 
      return this.mRowCount; 
    },
    
    getItemAtIndex: function (aIndex)
    {
      return gFeedSubscriptionsWindow.mFeedContainers[aIndex];
    },

    removeItemAtIndex: function (aIndex, aCount)
    {
      var itemToRemove = this.getItemAtIndex(aIndex);
      if (!itemToRemove) 
        return;

      var parentIndex = this.getParentIndex(aIndex);
      if (parentIndex != kRowIndexUndefined)
      {
        var parent = this.getItemAtIndex(parentIndex);
        if (parent)
        {
          for (var index = 0; index < parent.children.length; index++)
            if (parent.children[index] == itemToRemove)
            {
              parent.children.splice(index, 1);
              break;
            }
        }
      }

      // now remove it from our view
      gFeedSubscriptionsWindow.mFeedContainers.splice(aIndex, 1);

      // now invalidate the correct tree rows
      var tbo = gFeedSubscriptionsWindow.mTree.treeBoxObject;

      this.mRowCount--;
      tbo.rowCountChanged(aIndex, -1);

      // now update the selection position
      if (aIndex < gFeedSubscriptionsWindow.mFeedContainers.length)
        this.selection.select(aIndex);
      else 
        this.selection.clearSelection();

      // now refocus the tree
      gFeedSubscriptionsWindow.mTree.focus();
    },
    
    getCellText: function (aIndex, aColumn)
    {
      var item = this.getItemAtIndex(aIndex);
      if (!item) 
        return "";
      else if (aColumn.id == "folderNameCol")
        return item.name;
    },

    _selection: null, 
    get selection () { return this._selection; },
    set selection (val) { this._selection = val; return val; },
    getRowProperties: function (aIndex, aProperties) {},
    getCellProperties: function (aIndex, aColumn, aProperties) {},
    getColumnProperties: function (aColumn, aProperties) {},

    isContainer: function (aIndex)
    {
      var item = this.getItemAtIndex(aIndex);
      return item ? item.container : false;
    },

    isContainerOpen: function (aIndex) 
    { 
      var item = this.getItemAtIndex(aIndex);
      return item ? item.open : false;
    },

    isContainerEmpty: function (aIndex) 
    { 
      var item = this.getItemAtIndex(aIndex);
      if (!item) 
        return false;
      return item.children.length == 0;
    },

    isSeparator: function (aIndex) { return false; },    
    isSorted: function (aIndex) { return false; },    
    
    canDrop: function (aIndex, aOrientation) 
    { 
      var dropResult = this.extractDragData();
      return (aOrientation == Components.interfaces.nsITreeView.DROP_ON) && 
                              dropResult.canDrop && (dropResult.url || (dropResult.index != kRowIndexUndefined)); 
    },
    
    mDropUrl: "",
    mDropFolderUrl: "",
    drop: function (aIndex, aOrientation) 
    {  
      var results = this.extractDragData();
      if (!results.canDrop)
        return;

      if (results.url)
      {
        var folderItem = this.getItemAtIndex(aIndex);
        // don't freeze the app that initiaed the drop just because we are in a loop waiting for the user
        // to dimisss the add feed dialog....
        this.mDropUrl = results.url;
        this.mDropFolderUrl = folderItem.url;
        setTimeout(processDrop, 0);
      } 
      else if (results.index != kRowIndexUndefined)
        gFeedSubscriptionsWindow.moveFeed(results.index, aIndex);
    },
    
    //  helper function for drag and drop
    extractDragData: function()
    {
      var canDrop = false;
      var urlToDrop;
      var sourceIndex = kRowIndexUndefined;
      var dragService = Components.classes["@mozilla.org/widget/dragservice;1"].getService().QueryInterface(nsIDragService);
      var dragSession = dragService.getCurrentSession();

      var transfer = Components.classes["@mozilla.org/widget/transferable;1"].createInstance(Components.interfaces.nsITransferable);
      transfer.addDataFlavor("text/x-moz-url");
      transfer.addDataFlavor("text/x-moz-feed-index");
    
      dragSession.getData (transfer, 0);
      var dataObj = new Object();
      var flavor = new Object();
      var len = new Object();

      try {
        transfer.getAnyTransferData(flavor, dataObj, len);   
      } catch (ex) { return { canDrop: false, url: "" }; }

      if (dataObj.value)
      {
        dataObj = dataObj.value.QueryInterface(Components.interfaces.nsISupportsString);      
        sourceUri = dataObj.data.substring(0, len.value); // pull the URL out of the data object
     
        if (flavor.value == 'text/x-moz-url')
        {
          var uri = Components.classes["@mozilla.org/network/standard-url;1"].createInstance(Components.interfaces.nsIURI);
          uri.spec = sourceUri.split("\n")[0];
            
          if (uri.schemeIs("http") || uri.schemeIs("https"))
          {
            urlToDrop = uri.spec;
            canDrop = true;
          }
        } 
        else if (flavor.value == 'text/x-moz-feed-index')
        {
          sourceIndex = parseInt(sourceUri);
          canDrop = true;
        }
      }  // if dataObj.value

      return { canDrop: canDrop, url: urlToDrop, index: sourceIndex };
    },

    getParentIndex: function (aIndex) 
    {
      var item = this.getItemAtIndex(aIndex);

      if (item)
      {
        for (var index = aIndex; index >= 0; index--)
          if (gFeedSubscriptionsWindow.mFeedContainers[index].level <  item.level)
            return index;
      }
   
      return kRowIndexUndefined;
    },    
    hasNextSibling: function (aParentIndex, aIndex) 
    { 
      var item = this.getItemAtIndex(aIndex);
      if (item) 
      {
        // if the next node in the view has the same level as us, then we must have a next sibling...
        if (aIndex + 1 < gFeedSubscriptionsWindow.mFeedContainers.length )
          return this.getItemAtIndex(aIndex + 1).level == item.level;
      }

      return false;
    },
    hasPreviousSibling: function (aIndex)
    {
      var item = this.getItemAtIndex(aIndex);
      if (item && aIndex)
        return this.getItemAtIndex(aIndex - 1).level == item.level;
      else
        return false;      
    },
    getLevel: function (aIndex) 
    {
      var item = this.getItemAtIndex(aIndex);
      if (!item) 
        return 0;
      return item.level;
    },
    getImageSrc: function (aIndex, aColumn) {},    
    getProgressMode: function (aIndex, aColumn) {},    
    getCellValue: function (aIndex, aColumn) {},
    setTree: function (aTree) {},    
    toggleOpenState: function (aIndex) 
    {
      var item = this.getItemAtIndex(aIndex);
      if (!item) return;
      var multiplier = item.open ? -1 : 1;
      var delta = multiplier * item.children.length;
      this.mRowCount += delta;

      if (multiplier < 0)
        gFeedSubscriptionsWindow.mFeedContainers.splice(aIndex + 1, item.children.length);
      else
        for (var i = 0; i < item.children.length; i++)
          gFeedSubscriptionsWindow.mFeedContainers.splice(aIndex + 1 + i, 0, item.children[i]);

      // add or remove the children from our view
      item.open = !item.open;
      gFeedSubscriptionsWindow.mTree.treeBoxObject.rowCountChanged(aIndex + 1, delta);
    },    
    cycleHeader: function (aColumn) {},    
    selectionChanged: function () {},    
    cycleCell: function (aIndex, aColumn) {},    
    isEditable: function (aIndex, aColumn) 
    { 
      return false; 
    },
    setCellValue: function (aIndex, aColumn, aValue) {},    
    setCellText: function (aIndex, aColumn, aValue) {},    
    performAction: function (aAction) {},  
    performActionOnRow: function (aAction, aIndex) {},    
    performActionOnCell: function (aAction, aindex, aColumn) {}
  },
  
  makeFolderObject: function (aFolder, aCurrentLevel)
  {
    var folderObject =  { children : [],
                          name     : aFolder.prettiestName,
                          level    : aCurrentLevel,
                          url      : aFolder.QueryInterface(Components.interfaces.nsIRDFResource).Value,
                          open     : false,
                          container: true };

    // if a feed has any sub folders, we should add them to the list of children
    if (aFolder.hasSubFolders)
    {
      var folderEnumerator = aFolder.GetSubFolders();
      var done = false;

      while (!done) 
      {
        var folder = folderEnumerator.currentItem().QueryInterface(Components.interfaces.nsIMsgFolder);
        folderObject.children.push(this.makeFolderObject(folder, aCurrentLevel + 1));

        try {
          folderEnumerator.next();
        } 
        catch (ex)
        {
          done = true;
        }        
      }
    }

    var msgdb = aFolder.QueryInterface(Components.interfaces.nsIMsgFolder).getMsgDatabase(null);
    var folderInfo = msgdb.dBFolderInfo;
    var feedurls = folderInfo.getCharPtrProperty("feedUrl");
    var feedUrlArray = feedurls.split("|"); 

    for (url in feedUrlArray)
    {
      if (!feedUrlArray[url])
        continue;
      var feedResource  = rdf.GetResource(feedUrlArray[url]);
      var feed = new Feed(feedResource, this.mRSSServer);

      // Special case, if a folder only has a single feed associated with it, then just use the feed
      // in the view and don't show the folder at all. 
//      if (feedUrlArray.length <= 2 && !aFolder.hasSubFolders) // Note: split always adds an empty element to the array...
//        this.mFeedContainers[aCurrentLength] = this.makeFeedObject(feed, aCurrentLevel);
//      else // now add any feed urls for the folder
        folderObject.children.push(this.makeFeedObject(feed, aCurrentLevel + 1));           
    }

    return folderObject;
  },

  makeFeedObject: function (aFeed, aLevel)
  {
    // look inside the data source for the feed properties
    var feed = { children    : [],
                 name        : aFeed.title,
                 url         : aFeed.url,
                 level       : aLevel,
                 open        : false,
                 container   : false };
    return feed;
  },
  
  loadSubscriptions: function () 
  {
    // put together an array of folders
    var numFolders = 0;
    this.mFeedContainers = [];

    if (this.mRSSServer.rootFolder.hasSubFolders)
    {
      var folderEnumerator = this.mRSSServer.rootFolder.GetSubFolders();
      var done = false;

      while (!done) 
      {
        var folder = folderEnumerator.currentItem().QueryInterface(Components.interfaces.nsIMsgFolder);
        if (folder && !folder.getFlag(MSG_FOLDER_FLAG_TRASH)) 
        {
          this.mFeedContainers.push(this.makeFolderObject(folder, 0));
          numFolders++;
        }

        try {
          folderEnumerator.next();
        } 
        catch (ex)
        {
          done = true;
        }        
      }
    }
    this.mView.mRowCount = numFolders;
  },
  
  updateFeedData: function (aItem)
  {
    var ids = ['nameLabel', 'nameValue', 'locationLabel', 'locationValue'];
    if (aItem && !aItem.container) 
    {
      // set the feed location and title info
      document.getElementById('nameValue').value = aItem.name;
      document.getElementById('locationValue').value = aItem.url;
    }
    else 
    {
      var noneSelected = this.mBundle.getString("subscribe-noFeedSelected");
      document.getElementById('nameValue').value = noneSelected;
      document.getElementById('locationValue').value = "";
    }

    for (i = 0; i < ids.length; ++i)
      document.getElementById(ids[i]).disabled = !aItem || aItem.container;
  },
    
  onRSSFeedSelected: function () 
  {
    var properties, item;
    var seln = this.mTree.view.selection;
    item = this.mView.getItemAtIndex(seln.currentIndex);
      
    this.updateFeedData(item);
        
    document.getElementById("removeFeed").disabled = !item || item.container;
  },
  
  removeFeed: function () 
  { 
    var seln = this.mView.selection;
    if (seln.count != 1) return;

    var itemToRemove = this.mView.getItemAtIndex(seln.currentIndex);

    if (!itemToRemove)
      return;

    // ask the user if he really wants to unsubscribe from the feed
    var promptService = Components.classes["@mozilla.org/embedcomp/prompt-service;1"].getService(IPS);
    var abortRemoval = promptService.confirmEx(window, this.mBundle.getString('subsribe-confirmFeedDeletionTitle'), 
                                                  this.mBundle.getFormattedString('subsribe-confirmFeedDeletion', [itemToRemove.name], 1), 
                                                  (IPS.BUTTON_TITLE_YES * IPS.BUTTON_POS_0) + (IPS.BUTTON_TITLE_NO * IPS.BUTTON_POS_1),
                                                  null, null, null, null, { });
    if (abortRemoval)
      return;

    var resource = rdf.GetResource(itemToRemove.url);
    var feed = new Feed(resource);
    var ds = getSubscriptionsDS(this.mRSSServer);

    if (feed && ds)
    {
      // remove the feed from the subscriptions ds
      var feeds = getSubscriptionsList(this.mRSSServer);
      var index = feeds.IndexOf(resource);
      if (index != kRowIndexUndefined)
        feeds.RemoveElementAt(index, false);

      // remove the feed property string from the folder data base
      var currentFolder = ds.GetTarget(resource, FZ_DESTFOLDER, true);
      if (currentFolder) 
      {
        var currentFolderURI = currentFolder.QueryInterface(Components.interfaces.nsIRDFResource).Value;
        currentFolder = rdf.GetResource(currentFolderURI).QueryInterface(Components.interfaces.nsIMsgFolder);
    
        var feedUrl = ds.GetTarget(resource, DC_IDENTIFIER, true);    
        ds.Unassert(resource, DC_IDENTIFIER, feedUrl, true);

        feedUrl = feedUrl ? feedUrl.QueryInterface(Components.interfaces.nsIRDFLiteral).Value : "";

        updateFolderFeedUrl(currentFolder, feedUrl, true); // remove the old url
      }

      // Remove all assertions about the feed from the subscriptions database.
      removeAssertions(ds, resource);
      ds.QueryInterface(Components.interfaces.nsIRDFRemoteDataSource).Flush(); // flush any changes

      // Remove all assertions about items in the feed from the items database.
      var itemds = getItemsDS(this.mRSSServer);
      feed.invalidateItems();
      feed.removeInvalidItems();
      itemds.QueryInterface(Components.interfaces.nsIRDFRemoteDataSource).Flush(); // flush any changes
    }

    // Now that we have removed the feed from the datasource, it is time to update our
    // view layer. Start by removing the child from its parent folder object
    this.mView.removeItemAtIndex(seln.currentIndex);

    // If we don't have any more subscriptions pointing into
    // this folder, then I think we should offer to delete it...
    // Cheat and look at the feed url property to see if anyone else is still using the feed...
    // you could also accomplish this by looking at some properties in the data source...

//    var msgdb = currentFolder.QueryInterface(Components.interfaces.nsIMsgFolder).getMsgDatabase(null);
//   var folderInfo = msgdb.dBFolderInfo;
//    var oldFeedUrl = folderInfo.getCharPtrProperty("feedUrl");

//    if (!oldFeedUrl) // no more feeds pointing to the folder?
//    {
//      try {
//        var openerResource = this.mRSSServer.rootMsgFolder.QueryInterface(Components.interfaces.nsIRDFResource);
//        var folderResource = currentFolder.QueryInterface(Components.interfaces.nsIRDFResource);
//        window.opener.messenger.DeleteFolders(window.opener.GetFolderDatasource(), openerResource, folderResource);
//      } catch (e) { }
//    }
  },

  // aRootFolderURI --> optional argument. The folder to initially create the new feed under.
  addFeed: function(aFeedLocation, aRootFolderURI)
  {
    var userAddedFeed = false; 
    var defaultQuickMode = this.mRSSServer.getBoolAttribute('quickMode');
    var feedProperties = { feedName: "", feedLocation: aFeedLocation, 
                           serverURI: this.mRSSServer.serverURI, 
                           serverPrettyName: this.mRSSServer.prettyName,  
                           folderURI: aRootFolderURI, 
                           quickMode: this.mRSSServer.getBoolAttribute('quickMode'), 
                           newFeed: true,
                           result: userAddedFeed};

    feedProperties = openFeedEditor(feedProperties);

    // if the user hit cancel, exit without doing anything
    if (!feedProperties.result)
      return;
  
    if (!feedProperties.feedLocation)
        return;

    // before we go any further, make sure the user is not already subscribed to this feed.
    if (feedAlreadyExists(feedProperties.feedLocation, this.mRSSServer))
    {
      var promptService = Components.classes["@mozilla.org/embedcomp/prompt-service;1"].getService(IPS);
      promptService.alert(window, null, this.mBundle.getString("subscribe-feedAlreadySubscribed"));            
      return;
    }

    var itemResource = rdf.GetResource(feedProperties.feedLocation);
    feed = new Feed(itemResource);

    // if the user specified a specific folder to add the feed too, then set it here
    if (feedProperties.folderURI)
    {
      var folderResource = rdf.GetResource(feedProperties.folderURI);   
      if (folderResource)
      {
        var folder = folderResource.QueryInterface(Components.interfaces.nsIMsgFolder);
        if (folder && !folder.isServer)
          feed.folder = folder;
      }
    }

    // set the server for the feed
    feed.server = this.mRSSServer;
    feed.quickMode = feedProperties.quickMode;

    // update status text
    updateStatusItem('statusText', document.getElementById("bundle_newsblog").getString('subscribe-validating'));
    updateStatusItem('progressMeter', 0);

    // validate the feed and download the articles
    // we used to pass false which caused us to skip parsing then we'd 
    // turn around and download the feed again so we could actually parse the items...
    // But now that this operation is asynch, just kick it off once...if we change this back
    // modify feedDownloadCallback.downloaded to parse the feed...
    // Also, disable the Add button while we are subscribing.
    document.getElementById('addFeed').setAttribute('disabled', 'true');
    feed.download(true, this.mFeedDownloadCallback);
  },

  // moves the feed located at aOldFeedIndex to a child of aNewParentIndex
  moveFeed: function(aOldFeedIndex, aNewParentIndex)
  {
    // if the new parent is the same as the current parent, then do nothing
    if (this.mView.getParentIndex(aOldFeedIndex) == aNewParentIndex)
      return;

    var currentItem = this.mView.getItemAtIndex(aOldFeedIndex);
    var currentParentItem = this.mView.getItemAtIndex(this.mView.getParentIndex(aOldFeedIndex));
    var currentParentResource = rdf.GetResource(currentParentItem.url);

    var newParentItem = this.mView.getItemAtIndex(aNewParentIndex);
    var newParentResource = rdf.GetResource(newParentItem.url);

    var ds = getSubscriptionsDS(this.mRSSServer);
    var resource = rdf.GetResource(currentItem.url);
    var currentFolder = currentParentResource.QueryInterface(Components.interfaces.nsIMsgFolder);

    // unassert the older URI, add an assertion for the new parent URI...
    ds.Change(resource, FZ_DESTFOLDER, currentParentResource, newParentResource);

    // we need to update the feed url attributes on the databases for each folder
    updateFolderFeedUrl(currentParentResource.QueryInterface(Components.interfaces.nsIMsgFolder), 
                        currentItem.url, true); // remove our feed url property from the current folder
    updateFolderFeedUrl(newParentResource.QueryInterface(Components.interfaces.nsIMsgFolder), 
                        currentItem.url, false);       // add our feed url property to the new folder


    // Finally, update our view layer
    this.mView.removeItemAtIndex(aOldFeedIndex, 1);
    if (aNewParentIndex > aOldFeedIndex)
      aNewParentIndex--;
    
    currentItem.level = newParentItem.level + 1;
    newParentItem.children.push(currentItem);
    var indexOfNewItem = aNewParentIndex + newParentItem.children.length;;

    if (!newParentItem.open) // force open the container
      this.mView.toggleOpenState(aNewParentIndex);
    else
    {
      this.mFeedContainers.splice(indexOfNewItem, 0, currentItem);
      this.mView.mRowCount++;
      this.mTree.treeBoxObject.rowCountChanged(indexOfNewItem, 1);
    }

    gFeedSubscriptionsWindow.mTree.view.selection.select(indexOfNewItem)
  },

  beginDrag: function (aEvent)
  {
    // get the selected feed article (if there is one)
    var seln = this.mView.selection;
    if (seln.count != 1) 
      return;

    // only initiate a drag if the item is a feed (i.e. ignore folders/containers)
    var item = this.mView.getItemAtIndex(seln.currentIndex);
    if (!item || item.container)
      return;

    var transfer = Components.classes["@mozilla.org/widget/transferable;1"].createInstance(Components.interfaces.nsITransferable); 
    var transArray = Components.classes["@mozilla.org/supports-array;1"].createInstance(Components.interfaces.nsISupportsArray);
    var dragData = Components.classes["@mozilla.org/supports-string;1"].createInstance(Components.interfaces.nsISupportsString);

    transfer.addDataFlavor("text/x-moz-feed-index"); // i made this flavor type up
    dragData.data = seln.currentIndex.toString();

    transfer.setTransferData ( "text/x-moz-feed-index", dragData, seln.currentIndex.toString() * 2 );  // doublebyte byte data
    transArray.AppendElement(transfer.QueryInterface(Components.interfaces.nsISupports));

    var dragService = Components.classes["@mozilla.org/widget/dragservice;1"].getService().QueryInterface(nsIDragService);
    dragService.invokeDragSession ( aEvent.target, transArray, null, nsIDragService.DRAGDROP_ACTION_MOVE);
  },

  mFeedDownloadCallback:
  {
    downloaded: function(feed, aErrorCode)
    {
      // feed is null if our attempt to parse the feed failed
      if (aErrorCode == kNewsBlogSuccess)
      {
        updateStatusItem('progressMeter', 100);

        // if we get here...we should always have a folder by now...either
        // in feed.folder or FeedItems created the folder for us....
        var folder = feed.folder ? feed.folder : gFeedSubscriptionsWindow.mRSSServer.rootMsgFolder.getChildNamed(feed.name);

        updateFolderFeedUrl(folder, feed.url, false);

        // add feed just adds the feed we have validated and downloaded to our datasource
        // it also flushes the subscription datasource
        addFeed(feed.url, feed.name, folder); 

        // now add the feed to our view
        gFeedSubscriptionsWindow.loadSubscriptions();
        gFeedSubscriptionsWindow.mTree.treeBoxObject.invalidate();
        if (gFeedSubscriptionsWindow.mView.rowCount > 0) 
          gFeedSubscriptionsWindow.mTree.view.selection.select(0);
      } 
      else if (aErrorCode == kNewsBlogInvalidFeed) //  the feed was bad...
        window.alert(gFeedSubscriptionsWindow.mBundle.getFormattedString('newsblog-invalidFeed', [feed.url]));
      else if (aErrorCode == kNewsBlogRequestFailure) 
        window.alert(gFeedSubscriptionsWindow.mBundle.getFormattedString('newsblog-networkError', [feed.url]));

      // re-enable the add button now that we are done subscribing
      document.getElementById('addFeed').removeAttribute('disabled');

      // our operation is done...clear out the status text and progressmeter
      setTimeout(clearStatusInfo, 1000);
    },

    // this gets called after the RSS parser finishes storing a feed item to disk
    // aCurrentFeedItems is an integer corresponding to how many feed items have been downloaded so far
    // aMaxFeedItems is an integer corresponding to the total number of feed items to download
    onFeedItemStored: function (feed, aCurrentFeedItems, aMaxFeedItems)
    { 
      updateStatusItem('statusText', gFeedSubscriptionsWindow.mBundle.getFormattedString("subscribe-fetchingFeedItems", 
                                                                                         [aCurrentFeedItems, aMaxFeedItems]));
      this.onProgress(feed, aCurrentFeedItems, aMaxFeedItems);
    },

    onProgress: function(feed, aProgress, aProgressMax)
    {
      updateStatusItem('progressMeter', (aProgress * 100) / aProgressMax);
    },
  }
};

// opens the feed properties dialog
function openFeedEditor(aFeedProperties)
{
  window.openDialog('chrome://messenger-newsblog/content/feed-properties.xul', 'feedproperties', 'modal,titlebar,chrome,center', aFeedProperties);
  return aFeedProperties;
} 

function processDrop()
{
  gFeedSubscriptionsWindow.addFeed(gFeedSubscriptionsWindow.mView.mDropUrl, gFeedSubscriptionsWindow.mView.mDropFolderUrl);
}

// status helper routines

function updateStatusItem(aID, aValue)
{
  var el = document.getElementById(aID);
  if (el.getAttribute('collapsed'))
    el.removeAttribute('collapsed');

  el.value = aValue;
}

function clearStatusInfo()
{
  document.getElementById('statusText').value = "";
  document.getElementById('progressMeter').collapsed = true;
}

function doEdit() {
    // XXX There should be some way of correlating feed RDF resources
    // with their corresponding Feed objects.  Perhaps in the end much
    // of this code could hang off methods of the Feed object.
    var ds = getSubscriptionsDS(this.mRSSServer);
    var tree = document.getElementById('subscriptions');
    var item = tree.view.getItemAtIndex(tree.view.selection.currentIndex);
    var resource = rdf.GetResource(item.id);
    var old_url = ds.GetTarget(resource, DC_IDENTIFIER, true);
    old_url = old_url ? old_url.QueryInterface(Components.interfaces.nsIRDFLiteral).Value : "";
    var feed = new Feed(resource);

    var currentFolder = ds.GetTarget(resource, FZ_DESTFOLDER, true);
    var currentFolderURI = currentFolder.QueryInterface(Components.interfaces.nsIRDFResource).Value;

    currentFolder = rdf.GetResource(currentFolderURI).QueryInterface(Components.interfaces.nsIMsgFolder);
   
    var userModifiedFeed = false; 
    var feedProperties = { feedLocation: old_url, serverURI: this.mRSSServer.serverURI, 
                           serverPrettyName: this.mRSSServer.prettyName, folderURI: currentFolderURI, 
                           quickMode: feed.quickMode, result: userModifiedFeed};

    feedProperties = openFeedEditor(feedProperties);
    if (!feedProperties.result) // did the user cancel?
        return;

    // did the user change the folder URI for storing the feed?
    if (feedProperties.folderURI && feedProperties.folderURI != currentFolderURI)
    {
      // unassert the older URI, add an assertion for the new URI...
      ds.Change(resource, FZ_DESTFOLDER, currentFolder, rdf.GetResource(feedProperties.folderURI));

      // we need to update the feed url attributes on the databases for each folder
      var folderResource = rdf.GetResource(feedProperties.folderURI);   
      var newFolder = folderResource.QueryInterface(Components.interfaces.nsIMsgFolder);
      currentFolder = rdf.GetResource(currentFolderURI).QueryInterface(Components.interfaces.nsIMsgFolder);

      updateFolderFeedUrl(currentFolder, old_url, true); // remove our feed url property from the current folder
      updateFolderFeedUrl(newFolder, feedProperties.feedLocation, false); // add our feed url property to the new folder

      currentFolder = newFolder; // the folder has changed
    }

    // check to see if the location changed
    if (feedProperties.feedLocation && feedProperties.feedLocation != old_url)
    {
      ds.Change(resource, DC_IDENTIFIER, rdf.GetLiteral(old_url), rdf.GetLiteral(feedProperties.feedLocation));
      // now update our feed url property on the destination folder
      updateFolderFeedUrl(currentFolder, old_url, false); // remove the old url
      updateFolderFeedUrl(currentFolder, feedProperties.feedLocation, true);  // add the new one
    }

    // check to see if the quickMode value changed
    if (feed.quickMode != feedProperties.quickMode)
      feed.quickMode = feedProperties.quickMode;

   ds.QueryInterface(Components.interfaces.nsIRDFRemoteDataSource).Flush(); // flush any changes
}
