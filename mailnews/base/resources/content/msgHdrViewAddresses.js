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

/////////////////////////////////////////////////////////////////////
// If we want to do any special processing for each email address
// we show in the msg header view overlay, we can isolate it in this file.
// It is included by msgHdrViewOverlay.
//////////////////////////////////////////////////////////////////////

// AddExtraAddressProcessing --> If you want to add any extra
// customizations for a given address you can insert it here.
// emailAddress --> is the email address in question
// addressButton --> the titledbutton in the UI that we have created
// to represent this email address. this is probably what you want
// to modify / poke. 
function AddExtraAddressProcessing(emailAddress, addressButton)
{}

// NotifyClearAddresses --> use to clear any observers on the email
// addresses that maybe in the hdr view overlay. Each time a new
// message is loaded in message pane, we'll call this function...
function NotifyClearAddresses()
{
}