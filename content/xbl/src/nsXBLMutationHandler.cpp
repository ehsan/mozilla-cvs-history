/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
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
 * Original Author: David W. Hyatt (hyatt@netscape.com)
 *
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "nsCOMPtr.h"
#include "nsXBLPrototypeHandler.h"
#include "nsXBLMutationHandler.h"
#include "nsXBLAtoms.h"
#include "nsIContent.h"
#include "nsIScriptContext.h"
#include "nsIScriptGlobalObject.h"
#include "nsIDocument.h"
#include "nsIDOMDocument.h"
#include "nsIEventListenerManager.h"
#include "nsIDOMEventReceiver.h"
#include "nsXBLBinding.h"
#include "nsIPrivateDOMEvent.h"
#include "nsIDOMWindowInternal.h"
#include "nsIServiceManager.h"
#include "nsIURI.h"
#include "nsXPIDLString.h"

nsXBLMutationHandler::nsXBLMutationHandler(nsIDOMEventReceiver* aReceiver,
                                           nsXBLPrototypeHandler* aHandler)
  : nsXBLEventHandler(aReceiver, aHandler)
{
}

nsXBLMutationHandler::~nsXBLMutationHandler()
{
}

NS_IMPL_ISUPPORTS_INHERITED1(nsXBLMutationHandler, nsXBLEventHandler, nsIDOMMutationListener)

nsresult nsXBLMutationHandler::SubtreeModified(nsIDOMEvent* aEvent)
{
  return DoGeneric(nsXBLAtoms::DOMSubtreeModified, aEvent);
}

nsresult nsXBLMutationHandler::AttrModified(nsIDOMEvent* aEvent)
{
  return DoGeneric(nsXBLAtoms::DOMAttrModified, aEvent);
}

nsresult nsXBLMutationHandler::CharacterDataModified(nsIDOMEvent* aEvent)
{
  return DoGeneric(nsXBLAtoms::DOMCharacterDataModified, aEvent);
}

nsresult nsXBLMutationHandler::NodeInserted(nsIDOMEvent* aEvent)
{
  return DoGeneric(nsXBLAtoms::DOMNodeInserted, aEvent);
}

nsresult nsXBLMutationHandler::NodeRemoved(nsIDOMEvent* aEvent)
{
  return DoGeneric(nsXBLAtoms::DOMNodeRemoved, aEvent);
}

nsresult nsXBLMutationHandler::NodeInsertedIntoDocument(nsIDOMEvent* aEvent)
{
  return DoGeneric(nsXBLAtoms::DOMNodeInsertedIntoDocument, aEvent);
}

nsresult nsXBLMutationHandler::NodeRemovedFromDocument(nsIDOMEvent* aEvent)
{
  return DoGeneric(nsXBLAtoms::DOMNodeRemovedFromDocument, aEvent);
}

///////////////////////////////////////////////////////////////////////////////////

nsresult
NS_NewXBLMutationHandler(nsIDOMEventReceiver* aRec,
                         nsXBLPrototypeHandler* aHandler,
                         nsXBLMutationHandler** aResult)
{
  *aResult = new nsXBLMutationHandler(aRec, aHandler);
  if (!*aResult)
    return NS_ERROR_OUT_OF_MEMORY;
  NS_ADDREF(*aResult);
  return NS_OK;
}
