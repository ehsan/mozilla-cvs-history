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
 * The Original Code is the Mozilla browser.
 * 
 * The Initial Developer of the Original Code is Netscape
 * Communications, Inc.  Portions created by Netscape are
 * Copyright (C) 1999, Mozilla.  All Rights Reserved.
 * 
 * Contributor(s):
 *   Scott MacGregor <mscott@netscape.com>
 */

#ifndef nsExternalHelperAppService_h__
#define nsExternalHelperAppService_h__

#ifdef MOZ_LOGGING
#define FORCE_PR_LOG
#endif
#include "prlog.h"
#include "prtime.h"

#include "nsIExternalHelperAppService.h"
#include "nsIExternalProtocolService.h"
#include "nsIURIContentListener.h"
#include "nsIWebProgressListener.h"
#include "nsIHelperAppLauncherDialog.h"

#include "nsIMIMEInfo.h"
#include "nsIMIMEService.h"
#include "nsIStreamListener.h"
#include "nsIFile.h"
#include "nsIFileStreams.h"
#include "nsIOutputStream.h"
#include "nsString.h"
#include "nsIInterfaceRequestor.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsILocalFile.h"
#include "nsIChannel.h"

#include "nsIRDFDataSource.h"
#include "nsIRDFResource.h"
#include "nsCOMPtr.h"
#include "nsIObserver.h"
#include "nsCOMArray.h"
#include "nsWeakReference.h"

class nsExternalAppHandler;
class nsIMIMEInfo;
class nsIRDFService;
class nsIDownload;

/**
 * The helper app service. Responsible for handling content that Mozilla
 * itself can not handle
 */
class nsExternalHelperAppService
: public nsIExternalHelperAppService,
  public nsPIExternalAppLauncher,
  public nsIExternalProtocolService,
  public nsIMIMEService,
  public nsIObserver,
  public nsSupportsWeakReference
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIEXTERNALHELPERAPPSERVICE
  NS_DECL_NSPIEXTERNALAPPLAUNCHER
  NS_DECL_NSIEXTERNALPROTOCOLSERVICE
  NS_DECL_NSIMIMESERVICE
  NS_DECL_NSIOBSERVER

  nsExternalHelperAppService();
  virtual ~nsExternalHelperAppService();
  /**
   * Initializes the RDF datasource from the profile.
   * @retval NS_OK Loading was successful
   * @retval errorcode Loading failed
   * @see mOverRideDataSource
   */
  nsresult InitDataSource();
  /**
   * Initializes internal state. Will be called automatically when
   * this service is first instantiated.
   */
  nsresult Init();

  /**
   * Create an external app handler and binds it with a mime info object which
   * represents how we want to dispose of this content.
   * CreateNewExternalHandler is implemented only by the base class.
   * @param aMIMEInfo      MIMEInfo object, representing the type of the
   *                       content that should be handled
   * @param aFileExtension The extension we need to append to our temp file,
   *                       INCLUDING the ".". e.g. .mp3
   * @param aWindowContext Window context, as passed to DoContent
   */
  nsExternalAppHandler * CreateNewExternalHandler(nsIMIMEInfo * aMIMEInfo,
                                                  const char * aFileExtension,
                                                  nsISupports * aWindowContext);
 
  /**
   * Given a content type, look up the user override information to see if
   * we have a mime info object representing this content type. The user
   * over ride information is contained in a in memory data source.
   */
  nsresult GetMIMEInfoForMimeTypeFromDS(const char * aContentType,
                                        nsIMIMEInfo ** aMIMEInfo);
  
  /**
   * Given an extension, look up the user override information to see if we
   * have a mime info object representing this extension. The user over ride
   * information is contained in a in memory data source.
   */
  nsresult GetMIMEInfoForExtensionFromDS(const char * aFileExtension,
                                         nsIMIMEInfo ** aMIMEInfo);

  /**
   * Given a mimetype and an extension, looks up a mime info from the OS.
   * The mime type is given preference. This function follows the same rules
   * as nsIMIMEService::GetFromTypeAndExtension.
   * This is supposed to be overridden by the platform-specific
   * nsOSHelperAppService!
   */
  virtual already_AddRefed<nsIMIMEInfo> GetMIMEInfoFromOS(const char * aMIMEType,
                                                          const char * aFileExt);

  /**
   * Given a string identifying an application, create an nsIFile representing
   * it. This function should look in $PATH for the application.
   * GetFileTokenForPath must be implemented by each platform. 
   * @param platformAppPath A platform specific path to an application that we
   *                        got out of the rdf data source. This can be a mac
   *                        file spec, a unix path or a windows path depending
   *                        on the platform
   * @param aFile           [out] An nsIFile representation of that platform
   *                        application path.
   */
  virtual nsresult GetFileTokenForPath(const PRUnichar * platformAppPath,
                                       nsIFile ** aFile) = 0;

  /**
   * Helper routine used to test whether a given mime type is in our
   * mimeTypes.rdf data source
   */
  PRBool MIMETypeIsInDataSource(const char * aContentType);

protected:
  /**
   * Pointer to the datasource that contains the user override information.
   * @see InitDataSource
   */
  nsCOMPtr<nsIRDFDataSource> mOverRideDataSource;

	nsCOMPtr<nsIRDFResource> kNC_Description;
	nsCOMPtr<nsIRDFResource> kNC_Value;
	nsCOMPtr<nsIRDFResource> kNC_FileExtensions;
  nsCOMPtr<nsIRDFResource> kNC_Path;
  nsCOMPtr<nsIRDFResource> kNC_UseSystemDefault;
  nsCOMPtr<nsIRDFResource> kNC_SaveToDisk;
  nsCOMPtr<nsIRDFResource> kNC_AlwaysAsk;
  nsCOMPtr<nsIRDFResource> kNC_HandleInternal;
  nsCOMPtr<nsIRDFResource> kNC_PrettyName;

  /**
   * Whether mOverRideDataSource is initialized
   */
  PRBool mDataSourceInitialized;

  /**
   * Helper routines for digesting the data source and filling in a mime info
   * object for a given content type inside that data source.
   */
  nsresult FillTopLevelProperties(const char * aContentType,
                                  nsIRDFResource * aContentTypeNodeResource, 
                                  nsIRDFService * aRDFService,
                                  nsIMIMEInfo * aMIMEInfo);
  /**
   * @see FillTopLevelProperties
   */
  nsresult FillContentHandlerProperties(const char * aContentType,
                                        nsIRDFResource * aContentTypeNodeResource,
                                        nsIRDFService * aRDFService,
                                        nsIMIMEInfo * aMIMEInfo);

  /**
   * A small helper function which gets the target for a given source and
   * property. QIs to a literal and returns a CONST ptr to the string value
   * of that target
   */
  nsresult FillLiteralValueFromTarget(nsIRDFResource * aSource,
                                      nsIRDFResource * aProperty,
                                      const PRUnichar ** aLiteralValue);

  /**
   * Searches the "extra" array of MIMEInfo objects for an object
   * with a specific type.
   * @param aContentType The type to search for.
   * @param aMIMEInfo    [out] The mime info, if found
   */
  virtual nsresult GetMIMEInfoForMimeTypeFromExtras(const char * aContentType,
                                                    nsIMIMEInfo ** aMIMEInfo);
  /**
   * Searches the "extra" array of MIMEInfo objects for an object
   * with a specific extension.
   * @see GetMIMEInfoForMimeTypeFromExtras
   */
  virtual nsresult GetMIMEInfoForExtensionFromExtras(const char * aExtension,
                                                     nsIMIMEInfo ** aMIMEInfo);

protected:
#ifdef PR_LOGGING
  /**
   * NSPR Logging Module. Usage: set NSPR_LOG_MODULES=HelperAppService:level,
   * where level should be 2 for errors, 3 for debug messages from the cross-
   * platform nsExternalHelperAppService, and 4 for os-specific debug messages.
   */
  static PRLogModuleInfo* mLog;

  // friend, so that it can access the nspr log module
  friend class nsExternalAppHandler;
#endif
  /**
   * Functions related to the tempory file cleanup service provided by
   * nsExternalHelperAppService
   */
  nsresult ExpungeTemporaryFiles();
  /**
   * Array for the files that should be deleted
   */
  nsCOMArray<nsILocalFile> mTemporaryFilesList;
};

/**
 * We need to read the data out of the incoming stream into a buffer which we
 * can then use to write the data into the output stream representing the
 * temp file.
 */
#define DATA_BUFFER_SIZE (4096*2) 

/**
 * An external app handler is just a small little class that presents itself as
 * a nsIStreamListener. It saves the incoming data into a temp file. The handler
 * is bound to an application when it is created. When it receives an
 * OnStopRequest it launches the application using the temp file it has
 * stored the data into.  We create a handler every time we have to process
 * data using a helper app.
 */
class nsExternalAppHandler : public nsIStreamListener,
                             public nsIHelperAppLauncher,
                             public nsIURIContentListener,
                             public nsIInterfaceRequestor,
                             public nsIObserver
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISTREAMLISTENER
  NS_DECL_NSIREQUESTOBSERVER
  NS_DECL_NSIHELPERAPPLAUNCHER
  NS_DECL_NSIURICONTENTLISTENER
  NS_DECL_NSIINTERFACEREQUESTOR
  NS_DECL_NSIOBSERVER

  nsExternalAppHandler();
  virtual ~nsExternalAppHandler();

  virtual nsresult Init(nsIMIMEInfo * aMIMEInfo, const char * aFileExtension,
                        nsISupports * aWindowContext,
                        nsExternalHelperAppService *aHelperAppService);

protected:
  nsCOMPtr<nsIFile> mTempFile;
  nsCOMPtr<nsIURI> mSourceUrl;
  nsString mTempFileExtension;
  nsCOMPtr<nsIMIMEInfo> mMimeInfo;
  nsCOMPtr<nsIOutputStream> mOutStream; /**< output stream to the temp file */
  nsCOMPtr<nsISupports> mWindowContext; 
  /**
   * The following field is set if we were processing an http channel that had
   * a content disposition header which specified the SUGGESTED file name we
   * should present to the user in the save to disk dialog. 
   */
  nsString mSuggestedFileName;

  /**
   * The canceled flag is set if the user canceled the launching of this
   * application before we finished saving the data to a temp file.
   */
  PRPackedBool mCanceled;

  /**
   * have we received information from the user about how they want to
   * dispose of this content
   */
  PRPackedBool mReceivedDispositionInfo;
  PRPackedBool mStopRequestIssued; 
  PRPackedBool mProgressListenerInitialized;
  /// This is set when handling something with "Content-Disposition: attachment"
  PRPackedBool mHandlingAttachment;
  PRTime mTimeDownloadStarted;
  PRInt32 mContentLength;
  PRInt32 mProgress; /**< Number of bytes received (for sending progress notifications). */

  /**
   * When we are told to save the temp file to disk (in a more permament
   * location) before we are done writing the content to a temp file, then
   * we need to remember the final destination until we are ready to use it.
   */
  nsCOMPtr<nsIFile> mFinalFileDestination;

  char * mDataBuffer;

  nsresult SetUpTempFile(nsIChannel * aChannel);
  /**
   * When we download a helper app, we are going to retarget all load
   * notifications into our own docloader and load group instead of
   * using the window which initiated the load....RetargetLoadNotifications
   * contains that information...
   */
  nsresult RetargetLoadNotifications(nsIRequest *request); 
  /**
   * If the user tells us how they want to dispose of the content and
   * we still haven't finished downloading while they were deciding,
   * then create a progress listener of some kind so they know
   * what's going on...
   */
  nsresult CreateProgressListener();
  nsresult PromptForSaveToFile(nsILocalFile ** aNewFile,
                               const nsAFlatString &aDefaultFile,
                               const nsAFlatString &aDefaultFileExt);
  /**
   * If the passed in channel is an nsIHTTPChannel, we'll attempt to extract
   * a suggested file name from the content disposition header...
   */
  void ExtractSuggestedFileNameFromChannel(nsIChannel * aChannel);
  /**
   * After we're done prompting the user for any information, if the original
   * channel had a refresh url associated with it (which might point to a
   * "thank you for downloading" kind of page, then process that....It is safe
   * to invoke this method multiple times. We'll clear mOriginalChannel after
   * it's called and this ensures we won't call it again....
   */
  void ProcessAnyRefreshTags();

  /** 
   * An internal method used to actually move the temp file to the final
   * destination once we done receiving data AND have showed the progress dialog
   */
  nsresult MoveFile(nsIFile * aNewFileLocation);
  /**
   * An internal method used to actually launch a helper app given the temp file
   * once we are done receiving data AND have showed the progress dialog.
   */
  nsresult OpenWithApplication(nsIFile * aApplication);
  
  /**
   * Helper routine which peaks at the mime action specified by mMimeInfo
   * and calls either MoveFile or OpenWithApplication
   */
  nsresult ExecuteDesiredAction();
  /**
   * Helper routine that searches a pref string for a given mime type
   */
  PRBool GetNeverAskFlagFromPref(const char * prefName, const char * aContentType);

  /**
   * Initialize an nsIDownload object for use as a progress object
   */
  nsresult InitializeDownload(nsIDownload*);
  
  /**
   * Helper routine to ensure mSuggestedFileName is "correct";
   * the base class implementation ensures that mSuggestedFileName has
   * mTempFileExtension as extension;
   */
  virtual void EnsureSuggestedFileName();

  typedef enum { kReadError, kWriteError, kLaunchError } ErrorType;
  /**
   * Utility function to send proper error notification to web progress listener
   */
  void SendStatusChange(ErrorType type, nsresult aStatus, nsIRequest *aRequest, const nsAFlatString &path);
  
  nsCOMPtr<nsISupports>           mLoadCookie;    /**< load cookie used by the uri loader when we fetch the url */
  nsCOMPtr<nsIWebProgressListener> mWebProgressListener;
  nsCOMPtr<nsIChannel> mOriginalChannel; /**< in the case of a redirect, this will be the pre-redirect channel. */
  nsCOMPtr<nsIHelperAppLauncherDialog> mDialog;
  nsExternalHelperAppService *mHelperAppService;
};

#endif // nsExternalHelperAppService_h__
