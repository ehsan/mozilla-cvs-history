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
 * The Original Code is Camino code.
 *
 * The Initial Developer of the Original Code is
 * Sean Murphy.
 * Portions created by the Initial Developer are Copyright (C) 2008
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Sean Murphy <murph@seanmurph.com>
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

#ifndef CHSafeBrowsingAboutModule_h__
#define CHSafeBrowsingAboutModule_h__

#include "nsIAboutModule.h"

//
// CHSafeBrowsingAboutModule
//
// An nsIAboutModule for the "about:safebrowsingblocked" error page.
//
class CHSafeBrowsingAboutModule : public nsIAboutModule
{
 public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIABOUTMODULE

  CHSafeBrowsingAboutModule() {}
  virtual ~CHSafeBrowsingAboutModule() {}

  static NS_METHOD CreateSafeBrowsingAboutModule(nsISupports *aOuter, REFNSIID aIID, void **aResult);

 private:
  nsresult GetBlockedPageSource(nsACString &result);
};

/* EDF643A9-8B38-472C-92A0-B6 3B EF B3 07 69 */
#define CH_SAFEBROWSING_ABOUT_MODULE_CID \
{ 0xEDF643A9, 0xB38, 0x472C, \
{ 0x92, 0xA0, 0xB6, 0x3B, 0xEF, 0xB3, 0x07, 0x69}}

#endif // CHSafeBrowsingAboutModule_h__
