/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */

var appCore = null;

function Startup()
{
  // Create the browser instance component.
  createBrowserInstance();

	window._content.appCore= appCore;
  if (appCore == null) {
    // Give up.
    window.close();
  }
  // Initialize browser instance..
  appCore.setWebShellWindow(window);


  gURLBar = document.getElementById("urlbar");
}

function Shutdown()
{
  // Close the app core.
  if ( appCore )
    appCore.close();
}

function createBrowserInstance()
{
  appCore = Components
              .classes[ "component://netscape/appshell/component/browser/instance" ]
                .createInstance( Components.interfaces.nsIBrowserInstance );
  if ( !appCore ) {
      alert( "Error creating browser instance\n" );
  }
}

function BrowserLoadURL()
{
  try {
    appCore.loadUrl(gURLBar.value);
  }
  catch(e) {
  }
}

function BrowserBack()
{
  appCore.back();
}

function BrowserForward()
{
  appCore.forward();
}

function BrowserStop()
{
  appCore.stop();
}

function BrowserReload()
{
  appCore.reload(0);
}

