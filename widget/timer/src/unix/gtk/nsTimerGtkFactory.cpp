/* -*- Mode: C++; tab-width: 2; indentT-tabs-mode: nil; c-basic-offset: 2 -*-
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

#include "nsIGenericFactory.h"
#include "nsIModule.h"
#include "nsCOMPtr.h"

#include "nsUnixTimerCIID.h"
#include "nsTimerGtk.h"

NS_GENERIC_FACTORY_CONSTRUCTOR(nsTimerGtk)

static nsModuleComponentInfo components[] =
{
  { "GTK timer",
    NS_TIMER_GTK_CID,
    "component://netscape/timer/unix/gtk", 
    nsTimerGtkConstructor }
};

NS_IMPL_NSGETMODULE("nsGtkTimerModule", components)
