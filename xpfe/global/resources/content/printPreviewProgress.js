/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Communicator client code, 
 * released March 31, 1998. 
 *
 * The Initial Developer of the Original Code is Netscape Communications 
 * Corporation.  Portions created by Netscape are 
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 *
 * Contributors:
 *     Rod Spears <rods@netscape.com>
 */

// dialog is just an array we'll use to store various properties from the dialog document...
var dialog;

// the printProgress is a nsIPrintProgress object
var printProgress = null; 

// random global variables...
var targetFile;

var progressParams = null;

// all progress notifications are done through the nsIWebProgressListener implementation...
var progressListener = {
    onStateChange: function(aWebProgress, aRequest, aStateFlags, aStatus)
    {
      
      if (aStateFlags & Components.interfaces.nsIWebProgressListener.STATE_STOP)
      {
        window.close();
      }
    },
    
    onProgressChange: function(aWebProgress, aRequest, aCurSelfProgress, aMaxSelfProgress, aCurTotalProgress, aMaxTotalProgress)
    {
      if (progressParams)
      {
        dialog.title.crop = progressParams.docTitle ? "end" : "center";
        dialog.title.value = progressParams.docTitle || progressParams.docURL;
      }
    },

	  onLocationChange: function(aWebProgress, aRequest, aLocation)
    {
      // we can ignore this notification
    },

    onStatusChange: function(aWebProgress, aRequest, aStatus, aMessage)
    {
      if (aMessage != "")
        dialog.title.setAttribute("value", aMessage);
    },

    onSecurityChange: function(aWebProgress, aRequest, state)
    {
      // we can ignore this notification
    },

    QueryInterface : function(iid)
    {
     if (iid.equals(Components.interfaces.nsIWebProgressListener) || iid.equals(Components.interfaces.nsISupportsWeakReference))
      return this;
     
     throw Components.results.NS_NOINTERFACE;
    }
};

function onLoad() {
    // Set global variables.
    printProgress = window.arguments[0];

    if ( !printProgress ) {
        dump( "Invalid argument to printPreviewProgress.xul\n" );
        window.close()
        return;
    }

    dialog          = new Object;
    dialog.strings  = new Array;
    dialog.title      = document.getElementById("dialog.title");
    dialog.titleLabel = document.getElementById("dialog.titleLabel");

    if (window.arguments[1]) {
      progressParams = window.arguments[1].QueryInterface(Components.interfaces.nsIPrintProgressParams)
      if (progressParams) {
        dialog.title.crop = progressParams.docTitle ? "end" : "center";
        dialog.title.value = progressParams.docTitle || progressParams.docURL;
      }
    }

    // set our web progress listener on the helper app launcher
    printProgress.registerListener(progressListener);
    moveToAlertPosition();

    //We need to delay the set title else dom will overwrite it
    window.setTimeout(doneIniting, 100);
}

function onUnload() 
{
  if (printProgress)
  {
   try 
   {
     printProgress.unregisterListener(progressListener);
     printProgress = null;
   }
    
   catch( exception ) {}
  }
}

function getString( stringId ) {
   // Check if we've fetched this string already.
   if (!(stringId in dialog.strings)) {
      // Try to get it.
      var elem = document.getElementById( "dialog.strings."+stringId );
      try {
        if ( elem
           &&
           elem.childNodes
           &&
           elem.childNodes[0]
           &&
           elem.childNodes[0].nodeValue ) {
         dialog.strings[ stringId ] = elem.childNodes[0].nodeValue;
        } else {
          // If unable to fetch string, use an empty string.
          dialog.strings[ stringId ] = "";
        }
      } catch (e) { dialog.strings[ stringId ] = ""; }
   }
   return dialog.strings[ stringId ];
}

// If the user presses cancel, tell the app launcher and close the dialog...
function onCancel () 
{
  // Cancel app launcher.
   try 
   {
     printProgress.processCanceledByUser = true;
   }
   catch( exception ) {return true;}
    
  // don't Close up dialog by returning false, the backend will close the dialog when everything will be aborted.
  return false;
}

function doneIniting() 
{
  // called by function timeout in onLoad
  printProgress.doneIniting();
}
