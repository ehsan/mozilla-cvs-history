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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is 
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * Original Author: David W. Hyatt (hyatt@netscape.com)
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

class nsIAtom;
class nsICSSStyleSheet;
class nsIRDFService;
class nsIRDFDataSource;
class nsIRDFResource;
class nsIRDFNode;
class nsISimpleEnumerator;
class nsSupportsHashtable;
class nsIRDFContainer;
class nsIRDFContainerUtils;
class nsIDOMWindowInternal;
class nsIDocument;
class nsILocalFile;
class nsIProperties;

#include "nsIChromeRegistry.h"
#include "nsIXULOverlayProvider.h"
#include "nsIProtocolHandler.h"
#include "nsIRDFCompositeDataSource.h"
#include "nsIObserver.h"
#include "nsWeakReference.h"
#include "nsString.h"
#include "nsIZipReader.h"
#include "nsCOMArray.h"
#include "nsInterfaceHashtable.h"
     
// for component registration
// {47049e42-1d87-482a-984d-56ae185e367a}
#define NS_CHROMEREGISTRY_CID \
{ 0x47049e42, 0x1d87, 0x482a, { 0x98, 0x4d, 0x56, 0xae, 0x18, 0x5e, 0x36, 0x7a } }

class nsChromeRegistry : public nsIXULChromeRegistry,
                         public nsIXULOverlayProvider,
                         public nsIProtocolHandler,
                         public nsIObserver,
                         public nsSupportsWeakReference
{
public:
  NS_DECL_ISUPPORTS

  // nsIChromeRegistry methods:
  NS_DECL_NSICHROMEREGISTRY
  NS_DECL_NSIXULCHROMEREGISTRY
  NS_DECL_NSIXULOVERLAYPROVIDER

  // nsIProtocolHandler
  NS_DECL_NSIPROTOCOLHANDLER

  NS_DECL_NSIOBSERVER

  // nsChromeRegistry methods:
  nsChromeRegistry();
  virtual ~nsChromeRegistry();

  nsresult Init();

public:
  static nsresult FollowArc(nsIRDFDataSource *aDataSource,
                            nsACString& aResult,
                            nsIRDFResource* aChromeResource,
                            nsIRDFResource* aProperty);

  static nsresult UpdateArc(nsIRDFDataSource *aDataSource, nsIRDFResource* aSource, nsIRDFResource* aProperty, 
                            nsIRDFNode *aTarget, PRBool aRemove);

protected:
  nsresult GetDynamicDataSource(nsIURI *aChromeURL, PRBool aIsOverlay, PRBool aUseProfile, PRBool aCreateDS, nsIRDFDataSource **aResult);
  nsresult GetURIList(nsIRDFDataSource *aDS, nsIRDFResource *aResource, nsCOMArray<nsIURI>& aArray);
  nsresult GetDynamicInfo(nsIURI *aChromeURL, PRBool aIsOverlay, nsISimpleEnumerator **aResult);

  PRBool   IsOverlayAllowed(nsIURI *aChromeURI);

  nsresult GetResource(const nsACString& aChromeType, nsIRDFResource** aResult);
  
  nsresult UpdateDynamicDataSource(nsIRDFDataSource *aDataSource,
                                   nsIRDFResource *aResource,
                                   PRBool aIsOverlay, PRBool
                                   aUseProfile, PRBool aRemove);
  nsresult UpdateDynamicDataSources(nsIRDFDataSource *aDataSource,
                                    PRBool aIsOverlay,
                                    PRBool aUseProfile, PRBool
                                    aRemove);
  nsresult WriteInfoToDataSource(const char *aDocURI,
                                 const PRUnichar *aOverlayURI,
                                 PRBool aIsOverlay, PRBool
                                 aUseProfile, PRBool aRemove);
 
  nsresult LoadStyleSheetWithURL(nsIURI* aURL, nsICSSStyleSheet** aSheet);

  nsresult LoadInstallDataSource();
  nsresult LoadProfileDataSource();

  void FlushSkinCaches();
  void FlushAllCaches();

private:
  nsresult LoadDataSource(const nsACString &aFileName,
                          nsIRDFDataSource **aResult,
                          PRBool aUseProfileDirOnly = PR_FALSE,
                          const char *aProfilePath = nsnull);

  static nsresult GetProfileRoot(nsACString& aFileURL);
  static nsresult GetInstallRoot(nsIFile** aFileURL);

  nsresult RefreshWindow(nsIDOMWindowInternal* aWindow);

  nsresult GetArcs(nsIRDFDataSource* aDataSource,
                   const nsACString& aType,
                   nsISimpleEnumerator** aResult);

  nsresult AddToCompositeDataSource(PRBool aUseProfile);
  
  nsresult GetBaseURL(const nsACString& aPackage,
                      const nsACString& aProvider, 
                      nsACString& aBaseURL);

  nsresult FindProvider(const nsACString& aPackage,
                        const nsACString& aProvider,
                        nsCOMPtr<nsIRDFResource> &aProviderResource,
                        nsCOMPtr<nsIRDFResource> &aPackageResource);

  nsresult TrySubProvider(const nsACString& aPackage,
                          nsIRDFResource* aProviderResource,
                          nsCOMPtr<nsIRDFResource> &aSelectedProvider);

  nsresult FindSubProvider(const nsACString& aPackage,
                           const nsACString& aProvider,
                           nsCOMPtr<nsIRDFResource> &aSelectedProvider);

  nsresult InstallProvider(const nsACString& aProviderType,
                             const nsACString& aBaseURL,
                             PRBool aUseProfile, PRBool aAllowScripts, PRBool aRemove);
  nsresult UninstallProvider(const nsACString& aProviderType, const nsACString& aProviderName, PRBool aUseProfile);
  nsresult UninstallFromDynamicDataSource(const nsACString& aPackageName,
                                          PRBool aIsOverlay, PRBool aUseProfile);

  NS_HIDDEN_(nsresult) RealCheckForNewChrome();
  NS_HIDDEN_(nsresult) ProcessNewChromeFile(nsILocalFile *aListFile);
  NS_HIDDEN_(nsresult) ProcessNewChromeBuffer(char *aBuffer, PRInt32 aLength);

  PRBool GetProviderCount(const nsACString& aProviderType, nsIRDFDataSource* aDataSource);

protected:
  nsCString mProfileRoot;
  nsCString mInstallRoot;

  nsCOMPtr<nsIRDFCompositeDataSource> mChromeDataSource;
  nsCOMPtr<nsIRDFDataSource> mInstallDirChromeDataSource;
  nsCOMPtr<nsIRDFDataSource> mUIDataSource;

  nsSupportsHashtable* mDataSourceTable;
  nsIRDFService* mRDFService;
  nsIRDFContainerUtils* mRDFContainerUtils;

  nsCString mSelectedLocale;
  nsCString mSelectedSkin;

  nsInterfaceHashtable<nsCStringHashKey, nsIRDFResource> mSelectedLocales;
  nsInterfaceHashtable<nsCStringHashKey, nsIRDFResource> mSelectedSkins;

  // Resources
  nsCOMPtr<nsIRDFResource> mBaseURL;
  nsCOMPtr<nsIRDFResource> mPackages;
  nsCOMPtr<nsIRDFResource> mPackage;
  nsCOMPtr<nsIRDFResource> mName;
  nsCOMPtr<nsIRDFResource> mImage;
  nsCOMPtr<nsIRDFResource> mLocType;
  nsCOMPtr<nsIRDFResource> mAllowScripts;
  nsCOMPtr<nsIRDFResource> mHasOverlays;
  nsCOMPtr<nsIRDFResource> mHasStylesheets;
  nsCOMPtr<nsIRDFResource> mDisabled;
  nsCOMPtr<nsIRDFResource> mPlatformPackage;

  // useful atoms - these are static atoms, so don't use nsCOMPtr
  static nsIAtom* sCPrefix;            // "c"
  
  PRPackedBool mInstallInitialized;
  PRPackedBool mProfileInitialized;
  
  // Boolean that indicates we should batch flushes of the main
  // chrome.rdf file.
  PRPackedBool mBatchInstallFlushes;
};
