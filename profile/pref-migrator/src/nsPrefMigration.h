/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are Copyright (C) 1998
 * Netscape Communications Corporation.  All Rights Reserved.
 */

#ifndef nsPrefMigration_h___
#define nsPrefMigration_h___


#include "nscore.h"
#include "nsIFactory.h"
#include "nsISupports.h"
#include "nsFileSpec.h"
#include "nsIPref.h"
#include "nsIServiceManager.h"
#include "nsICommonDialogs.h"
#include "nsCOMPtr.h"
#include "nsIDOMWindow.h"
#include "nsIWebShellWindow.h"
#include "nsIFileSpec.h"
#include "nsPrefMigrationCIDs.h"
#include "nsIPrefMigration.h"
#include "nsVoidArray.h"

#ifdef XP_MAC
#define IMAP_MAIL_FILTER_FILE_NAME_FORMAT_IN_4x "%s Rules" 
#endif

class nsPrefMigration: public nsIPrefMigration
{
    public:
      NS_DEFINE_STATIC_CID_ACCESSOR(NS_PREFMIGRATION_CID) 

      static nsPrefMigration *GetInstance();

      nsPrefMigration();
      virtual ~nsPrefMigration();

      NS_DECL_ISUPPORTS

      NS_DECL_NSIPREFMIGRATION

      // todo try to move this to private.  We need this because we need to call this
      // from a thread.

      nsVoidArray mProfilesToMigrate;
      nsresult ProcessPrefsCallback(const char* oldProfilePathStr, const char * newProfilePathStr);
      void WaitForThread();
    
    private:
      
      static nsPrefMigration* mInstance;
	
	  nsresult ConvertPersistentStringToFileSpec(const char *str, nsIFileSpec *path);
      nsresult CreateNewUser5Tree(nsIFileSpec* oldProfilePath, 
                                  nsIFileSpec* newProfilePath);

      nsresult GetDirFromPref(nsIFileSpec* oldProfilePath,
                              nsIFileSpec* newProfilePath, 
                              const char* newDirName,
                              char* pref, 
                              nsIFileSpec* newPath, 
                              nsIFileSpec* oldPath);

      nsresult GetSizes(nsFileSpec inputPath,
                        PRBool readSubdirs,
                        PRUint32* sizeTotal);

      nsresult GetDriveName(nsFileSpec inputPath,
                            char** driveName);

      nsresult CheckForSpace(nsFileSpec newProfilePath, 
                             PRFloat64 requiredSpace);

      nsresult DoTheCopy(nsIFileSpec *oldPath, 
                         nsIFileSpec *newPath,
                         PRBool readSubdirs); 

      nsresult DoTheCopyAndRename(nsIFileSpec *oldPath, 
                              nsIFileSpec *newPath,
                              PRBool readSubdirs,
                              PRBool needToRenameFiles,
                              const char *oldName,
                              const char *newName); 

      nsresult DoSpecialUpdates(nsIFileSpec * profilePath);
      nsresult Rename4xFileAfterMigration(nsIFileSpec *profilePath, const char *oldFileName, const char *newFileName);
#ifdef IMAP_MAIL_FILTER_FILE_NAME_FORMAT_IN_4x
      nsresult RenameAndMove4xImapFilterFile(nsIFileSpec *profilePath, const char *hostname);
      nsresult RenameAndMove4xImapFilterFiles(nsIFileSpec *profilePath);
#endif /* IMAP_MAIL_FILTER_FILE_NAME_FORMAT_IN_4x */
      nsresult RenameAndMove4xPopFilterFile(nsIFileSpec *profilePath);
  
      nsresult SetPremigratedFilePref(const char *pref_name, nsIFileSpec *filePath);
      
      
      nsIPref* m_prefs;
      nsresult getPrefService();
      nsCOMPtr<nsIFileSpec> m_prefsFile; 
      nsCOMPtr<nsIDOMWindow> m_parentWindow;
      nsIDOMWindow *m_progressWindow;
      nsCOMPtr<nsIWebShellWindow>  mPMProgressWindow;
  

      
};

#endif

