/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Christopher Blizzard <blizzard@mozilla.org>.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "nsIGenericFactory.h"

// generic factory constructors

/*
NS_GENERIC_FACTORY_CONSTRUCTOR(nsFontMetrics)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsDeviceContext)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsDeviceContextSpec)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsDeviceContextSpecFactory)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsRenderingContext)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsImage)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsBlender)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsRegion)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsFontEnumerator)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsFontList)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsScreenManager)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsPrintOptions)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsPrinterEnumerator)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsScriptableRegion)
*/

static nsModuleComponentInfo components[] =
{
};

PR_STATIC_CALLBACK(void)
nsGfxGtk2ModuleDtor(nsIModule *self)
{
}

NS_IMPL_NSGETMODULE_WITH_DTOR(nsGfxGtk2Module, components, nsGfxGtk2ModuleDtor)
