/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
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

#include "nsIUnixToolkitService.h"
#include "nsUnixToolkitService.h"

NS_GENERIC_FACTORY_CONSTRUCTOR(nsUnixToolkitService)

static nsModuleComponentInfo components[] =
{
  { "Unix WIndow Service",
    NS_UNIX_TOOLKIT_SERVICE_CID,
    NS_UNIX_TOOLKIT_SERVICE_PROGID,
    nsUnixToolkitServiceConstructor }
};

NS_IMPL_NSGETMODULE("nsToolkitModule", components)
