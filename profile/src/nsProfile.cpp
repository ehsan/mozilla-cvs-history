/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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

#include "nsProfile.h"
#include "nsIPref.h"

#include "pratom.h"
#include "prmem.h"
#include "plstr.h"
#include "prenv.h"

#include "nsIFactory.h"
#include "nsIComponentManager.h"
#include "nsIEnumerator.h"
#include "nsXPIDLString.h"
#include "nsIFileSpec.h"
#include "nsIFileLocator.h"
#include "nsFileLocations.h"
#include "nsEscape.h"
#include "nsIURL.h"

#include "nsIAppShellService.h"
#include "nsAppShellCIDs.h"
#include "prprf.h"

#include "nsIIOService.h"
#include "nsNetUtil.h"
#include "nsPrefMigration.h"
#include "nsIPrefMigration.h"
#include "nsPrefMigrationCIDs.h"
#include "nsFileStream.h"
#include "nsSpecialSystemDirectory.h"
#include "nsIPrompt.h"
#include "nsIStreamListener.h"
#include "nsIServiceManager.h"
#include "nsCOMPtr.h"
#include "nsIBookmarksService.h"
#include "nsIModule.h"
#include "nsIGenericFactory.h"
#include "nsICookieService.h"
#include "nsICategoryManager.h"
#include "nsISupportsPrimitives.h"
#include "nsAppDirectoryServiceDefs.h"

#include "nsIChromeRegistry.h" // chromeReg

// Interfaces Needed
#include "nsIDocShell.h"
#include "nsIWebBrowserChrome.h"

#include "nsIScriptGlobalObject.h"
#include "nsIBaseWindow.h"
#include "nsICommonDialogs.h"
#include "nsIDOMWindowInternal.h"
#include "nsIWindowMediator.h"

#if defined (XP_UNIX)
#elif defined (XP_MAC)
#define OLD_REGISTRY_FILE_NAME "Netscape Registry"
#elif defined (XP_BEOS)
#else /* assume XP_PC */
#ifndef XP_OS2
#include <direct.h>
#endif
#define OLD_REGISTRY_FILE_NAME "nsreg.dat"
#endif /* XP_UNIX */

// hack for copying panels.rdf and localstore.rdf into migrated profile dir
#define PANELS_RDF_FILE                "panels.rdf"
#define LOCALSTORE_RDF_FILE                "localstore.rdf"

// A default profile name, in case automigration 4x profile fails
#define DEFAULT_PROFILE_NAME           "default"

#define PROFILE_SELECTION_URL          "chrome://communicator/content/profile/profileSelection.xul"
#define PROFILE_SELECTION_CMD_LINE_ARG "-SelectProfile"
#define PROFILE_MANAGER_URL            "chrome://communicator/content/profile/profileSelection.xul?manage=true"
#define PROFILE_MANAGER_CMD_LINE_ARG   "-ProfileManager"
#define PROFILE_WIZARD_URL             "chrome://communicator/content/profile/createProfileWizard.xul"
#define CONFIRM_AUTOMIGRATE_URL        "chrome://communicator/content/profile/confirmMigration.xul"
#define PROFILE_WIZARD_CMD_LINE_ARG    "-ProfileWizard"
#define INSTALLER_CMD_LINE_ARG         "-installer"
#define CREATE_PROFILE_CMD_LINE_ARG    "-CreateProfile"
#define PROFILE_CMD_LINE_ARG "-P"   

#define PREF_CONFIRM_AUTOMIGRATION     "profile.confirm_automigration"
#define SHRIMP_PREF                    "shrimp.startup.enable"

#if defined (XP_MAC)
#define CHROME_STYLE nsIWebBrowserChrome::CHROME_WINDOW_BORDERS | nsIWebBrowserChrome::CHROME_WINDOW_CLOSE | nsIWebBrowserChrome::CHROME_CENTER_SCREEN
#else /* the rest */
#define CHROME_STYLE nsIWebBrowserChrome::CHROME_ALL | nsIWebBrowserChrome::CHROME_CENTER_SCREEN
#endif 

// we want everyone to have the debugging info to the console for now
// to help track down profile manager problems
// when we ship, we'll turn this off
#define DEBUG_profile 1
#undef DEBUG_profile_verbose

// ProfileAccess varaible (gProfileDataAccess) to access registry operations
// gDataAccessInstCount is used to keep track of instance count to activate
// destructor at the right time (count == 0)
static nsProfileAccess*    gProfileDataAccess = nsnull;
static PRInt32          gInstanceCount = 0;
static PRBool           mCurrentProfileAvailable = PR_FALSE;

// Atoms for file locations
static nsIAtom* sApp_PrefsDirectory50         = nsnull;
static nsIAtom* sApp_PreferencesFile50        = nsnull;
static nsIAtom* sApp_UserProfileDirectory50   = nsnull;
static nsIAtom* sApp_UserChromeDirectory      = nsnull;
static nsIAtom* sApp_LocalStore50             = nsnull;
static nsIAtom* sApp_History50                = nsnull;
static nsIAtom* sApp_UsersPanels50            = nsnull;
static nsIAtom* sApp_UsersMimeTypes50         = nsnull;
static nsIAtom* sApp_BookmarksFile50          = nsnull;
static nsIAtom* sApp_SearchFile50             = nsnull;
static nsIAtom* sApp_MailDirectory50          = nsnull;
static nsIAtom* sApp_ImapMailDirectory50      = nsnull;
static nsIAtom* sApp_NewsDirectory50          = nsnull;
static nsIAtom* sApp_MessengerFolderCache50   = nsnull;

// IID and CIDs of all the services needed
static NS_DEFINE_CID(kIProfileIID, NS_IPROFILE_IID);
static NS_DEFINE_CID(kBookmarksCID, NS_BOOKMARKS_SERVICE_CID);      
static NS_DEFINE_CID(kComponentManagerCID, NS_COMPONENTMANAGER_CID);
static NS_DEFINE_CID(kFileLocatorCID, NS_FILELOCATOR_CID);
static NS_DEFINE_CID(kRegistryCID, NS_REGISTRY_CID);
static NS_DEFINE_CID(kPrefCID, NS_PREF_CID);
static NS_DEFINE_CID(kAppShellServiceCID, NS_APPSHELL_SERVICE_CID);
static NS_DEFINE_IID(kIFactoryIID,  NS_IFACTORY_IID);
static NS_DEFINE_IID(kIIOServiceIID, NS_IIOSERVICE_IID);
static NS_DEFINE_CID(kIOServiceCID, NS_IOSERVICE_CID);
static NS_DEFINE_CID(kPrefMigrationCID, NS_PREFMIGRATION_CID);
static NS_DEFINE_CID(kPrefConverterCID, NS_PREFCONVERTER_CID);
static NS_DEFINE_IID(kCookieServiceCID, NS_COOKIESERVICE_CID);
static NS_DEFINE_CID(kDialogParamBlockCID, NS_DialogParamBlock_CID);
static NS_DEFINE_CID(kWindowMediatorCID, NS_WINDOWMEDIATOR_CID);

static NS_DEFINE_CID(kChromeRegistryCID,    NS_CHROMEREGISTRY_CID);

static
nsresult GetStringFromSpec(nsFileSpec inSpec, char **string)
{
    nsresult rv;
    nsCOMPtr<nsIFileSpec> spec;

    rv = NS_NewFileSpecWithSpec(inSpec, getter_AddRefs(spec));
    if (NS_SUCCEEDED(rv)) {
        rv = spec->GetPersistentDescriptorString(string);
        if (NS_SUCCEEDED(rv)) {
            return NS_OK;
        }
        else {
            nsCRT::free(*string);
            return rv;
        }
    } 
    else {
        *string = nsnull;
        return rv;
    }
}

/*
 * Constructor/Destructor
 */
nsProfile::nsProfile()
{
    NS_INIT_REFCNT();
    mAutomigrate = PR_FALSE;
    mOutofDiskSpace = PR_FALSE;
    mDiskSpaceErrorQuitCalled = PR_FALSE;

    if (gInstanceCount++ == 0) {
        
        gProfileDataAccess = new nsProfileAccess();
        
       // Make our directory atoms
        
       // Preferences:
         sApp_PrefsDirectory50         = NS_NewAtom(NS_APP_PREFS_50_DIR);
         sApp_PreferencesFile50        = NS_NewAtom(NS_APP_PREFS_50_FILE);
        
       // Profile:
         sApp_UserProfileDirectory50   = NS_NewAtom(NS_APP_USER_PROFILE_50_DIR);
        
       // Application Directories:
         sApp_UserChromeDirectory      = NS_NewAtom(NS_APP_USER_CHROME_DIR);
         
       // Aplication Files:
         sApp_LocalStore50             = NS_NewAtom(NS_APP_LOCALSTORE_50_FILE);
         sApp_History50                = NS_NewAtom(NS_APP_HISTORY_50_FILE);
         sApp_UsersPanels50            = NS_NewAtom(NS_APP_USER_PANELS_50_FILE);
         sApp_UsersMimeTypes50         = NS_NewAtom(NS_APP_USER_MIMETYPES_50_FILE);
         
       // Bookmarks:
         sApp_BookmarksFile50          = NS_NewAtom(NS_APP_BOOKMARKS_50_FILE);
         
       // Search
         sApp_SearchFile50             = NS_NewAtom(NS_APP_SEARCH_50_FILE);
         
       // MailNews
         sApp_MailDirectory50          = NS_NewAtom(NS_APP_MAIL_50_DIR);
         sApp_ImapMailDirectory50      = NS_NewAtom(NS_APP_IMAP_MAIL_50_DIR);
         sApp_NewsDirectory50          = NS_NewAtom(NS_APP_NEWS_50_DIR);
         sApp_MessengerFolderCache50   = NS_NewAtom(NS_APP_MESSENGER_FOLDER_CACHE_50_DIR);
         
         nsresult rv;
         NS_WITH_SERVICE(nsIDirectoryService, directoryService, NS_DIRECTORY_SERVICE_PROGID, &rv);
         if (NS_SUCCEEDED(rv))
            directoryService->RegisterProvider(this);
    }
}

nsProfile::~nsProfile() 
{
#if defined(DEBUG_profile_verbose)
    printf("~nsProfile \n");
#endif

   if (--gInstanceCount == 0) {
        
      delete gProfileDataAccess;

      NS_IF_RELEASE(sApp_PrefsDirectory50);
      NS_IF_RELEASE(sApp_PreferencesFile50);
      NS_IF_RELEASE(sApp_UserProfileDirectory50);
      NS_IF_RELEASE(sApp_UserChromeDirectory);
      NS_IF_RELEASE(sApp_LocalStore50);
      NS_IF_RELEASE(sApp_History50);
      NS_IF_RELEASE(sApp_UsersPanels50);
      NS_IF_RELEASE(sApp_UsersMimeTypes50);
      NS_IF_RELEASE(sApp_BookmarksFile50);
      NS_IF_RELEASE(sApp_SearchFile50);
      NS_IF_RELEASE(sApp_MailDirectory50);
      NS_IF_RELEASE(sApp_ImapMailDirectory50);
      NS_IF_RELEASE(sApp_NewsDirectory50);
      NS_IF_RELEASE(sApp_MessengerFolderCache50);
    }
}

/*
 * nsISupports Implementation
 */
NS_IMPL_THREADSAFE_ADDREF(nsProfile)
NS_IMPL_THREADSAFE_RELEASE(nsProfile)

NS_INTERFACE_MAP_BEGIN(nsProfile)
    NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIProfile)
    NS_INTERFACE_MAP_ENTRY(nsIProfile)
    NS_INTERFACE_MAP_ENTRY(nsIDirectoryServiceProvider)
NS_INTERFACE_MAP_END

/*
 * nsIProfile Implementation
 */
NS_IMETHODIMP nsProfile::Startup(const PRUnichar *filename)
{
    return NS_OK;
}

NS_IMETHODIMP
nsProfile::GetAutomigrate(PRBool *aAutomigrate)
{
    NS_ENSURE_ARG_POINTER(aAutomigrate);

    *aAutomigrate = mAutomigrate;
    return NS_OK;
}

NS_IMETHODIMP
nsProfile::SetAutomigrate(PRBool aAutomigrate)
{
    mAutomigrate = aAutomigrate;
    return NS_OK;
}

NS_IMETHODIMP
nsProfile::StartupWithArgs(nsICmdLineService *cmdLineArgs)
{
    nsresult rv;

    // initializations for profile manager
    PRBool profileDirSet = PR_FALSE;
    nsCString profileURLStr("");

#ifdef DEBUG_profile_verbose
    printf("Profile Manager : Profile Wizard and Manager activites : Begin\n");
#endif

    if (cmdLineArgs)
        rv = ProcessArgs(cmdLineArgs, &profileDirSet, profileURLStr);

    // This boolean is set only when an automigrated user runs out of disk space
    // and chooses to cancel further operations from the dialogs presented...
    if (mDiskSpaceErrorQuitCalled)
        return NS_ERROR_FAILURE;

    if (!profileDirSet) {
        rv = LoadDefaultProfileDir(profileURLStr);

        if (NS_FAILED(rv)) return rv;
    }

#ifdef DEBUG_profile_verbose
    printf("Profile Manager : Profile Wizard and Manager activites : End\n");
#endif

    return NS_OK;
}


nsresult
nsProfile::LoadDefaultProfileDir(nsCString & profileURLStr)
{
    nsresult rv;
    nsCOMPtr<nsIURI> profileURL;
    PRInt32 numProfiles=0;
  
    NS_WITH_SERVICE(nsIPref, prefs, kPrefCID, &rv);
    if (NS_FAILED(rv)) return rv;

    GetProfileCount(&numProfiles);
    /*
     * Create the Application Shell instance...
     */
    NS_WITH_SERVICE(nsIAppShellService, profAppShell,
                          kAppShellServiceCID, &rv);
    if (NS_FAILED(rv))
        return rv;

    PRBool shrimpPrefEnabled = PR_FALSE;
    prefs->GetBoolPref(SHRIMP_PREF, &shrimpPrefEnabled);

    if (shrimpPrefEnabled)
        profileURLStr = "";

    if (profileURLStr.Length() == 0)
    {
        // This means that there was no command-line argument to force
        // profile UI to come up. But we need the UI anyway if there
        // are no profiles yet, or if there is more than one.
        if (numProfiles == 0)
        {
            rv = CreateDefaultProfile();
            if (NS_FAILED(rv)) return rv;
                
            GetProfileCount(&numProfiles);
            profileURLStr = "";

            mCurrentProfileAvailable = PR_TRUE;
            // Need to load new profile prefs.
            rv = LoadNewProfilePrefs();
        }
        else if (numProfiles > 1)
            profileURLStr = PROFILE_SELECTION_URL;
    }

    if ((profileURLStr.Length() != 0) && !(shrimpPrefEnabled))
    {
        rv = NS_NewURI(getter_AddRefs(profileURL), (const char *)profileURLStr);

        if (NS_FAILED(rv)) {
            return rv;
        } 

        nsCOMPtr<nsIXULWindow> profWindow;
        rv = profAppShell->CreateTopLevelWindow(nsnull, profileURL,
                                                PR_TRUE, PR_TRUE, CHROME_STYLE,
                                                NS_SIZETOCONTENT,           // width 
                                                NS_SIZETOCONTENT,           // height
                                                getter_AddRefs(profWindow));

        if (NS_FAILED(rv)) return rv;

        /*
         * Start up the main event loop...
         */    
        rv = profAppShell->Run();
    }

    PRBool confirmAutomigration = PR_FALSE;
    if (NS_SUCCEEDED(rv) && prefs) {
        rv = prefs->GetBoolPref(PREF_CONFIRM_AUTOMIGRATION, &confirmAutomigration);
        if (NS_FAILED(rv)) confirmAutomigration = PR_FALSE;
    }
    
    if (confirmAutomigration) {
        if (profileURLStr == (const char*)CONFIRM_AUTOMIGRATE_URL) {
            PRBool automigrate = PR_FALSE;
            rv = GetAutomigrate(&automigrate);
            if (NS_SUCCEEDED(rv) && automigrate) {
                AutoMigrate();
            }
            else {
                // the user hit cancel.
                // so they don't want to automatically migrate
                // so call this again with the profile manager ui
                nsCString profileManagerUrl(PROFILE_MANAGER_URL);
                rv = LoadDefaultProfileDir(profileManagerUrl);
                return rv;
            }
        }
    }

    // if we get here, and we don't have a current profile, 
    // return a failure so we will exit
    // this can happen, if the user hits Exit in the profile manager dialog
    nsXPIDLString currentProfileStr;
    rv = GetCurrentProfile(getter_Copies(currentProfileStr));

    if (NS_FAILED(rv) || (*(const PRUnichar*)currentProfileStr == 0)) {
        return NS_ERROR_FAILURE;
    }
    mCurrentProfileAvailable = PR_TRUE;

    // Now we have the right profile, read the user-specific prefs.
    rv = prefs->ReadUserPrefs();
    if (NS_FAILED(rv)) return rv;

    NS_WITH_SERVICE(nsICategoryManager, catman, NS_CATEGORYMANAGER_PROGID, &rv);

    if(NS_SUCCEEDED(rv) && catman) 
    {
        nsCOMPtr<nsISimpleEnumerator> enumItem;
        rv = catman->EnumerateCategory(NS_PROFILE_STARTUP_CATEGORY, getter_AddRefs(enumItem));
        if(NS_SUCCEEDED(rv) && enumItem) 
        {
           while (PR_TRUE) 
           {
               nsCOMPtr<nsISupportsString> progid;

               rv = enumItem->GetNext(getter_AddRefs(progid));
               if (NS_FAILED(rv) || !progid) break;

               nsXPIDLCString progidString;
               progid->ToString (getter_Copies(progidString));
        
               nsCOMPtr <nsIProfileStartupListener> listener = do_CreateInstance(progidString, &rv);
        
               if (listener) 
                   listener->OnProfileStartup(currentProfileStr);
           }
        }
    }

    PRBool prefs_converted = PR_FALSE;
    rv = prefs->GetBoolPref("prefs.converted-to-utf8",&prefs_converted);
    if(NS_FAILED(rv)) return rv;

    if (!prefs_converted) 
    {
        nsCOMPtr <nsIPrefConverter> pPrefConverter = do_CreateInstance(kPrefConverterCID, &rv);
        if (NS_FAILED(rv)) return rv;
        if (!pPrefConverter) return NS_ERROR_FAILURE;

        rv = pPrefConverter->ConvertPrefsToUTF8();
    }
    return rv;
}

nsresult 
nsProfile::AutoMigrate()
{
    nsresult rv = NS_OK;
    // automatically migrate the one 4.x profile
    rv = MigrateAllProfiles();

    // Create a default profile if automigration failed for reasons
    // other than out of disk space case...
    if (NS_FAILED(rv) && !mOutofDiskSpace) 
    {
#ifdef DEBUG_profile
        printf("AutoMigration failed. Let's create a default 5.0 profile.\n");
#endif
        
        rv = CreateDefaultProfile();
        if (NS_FAILED(rv)) return rv;
    }   

    gProfileDataAccess->mProfileDataChanged = PR_TRUE;
    gProfileDataAccess->UpdateRegistry(nsnull);

    return rv;
}

nsresult
nsProfile::ProcessArgs(nsICmdLineService *cmdLineArgs,
                       PRBool* profileDirSet,
                       nsCString & profileURLStr)
{
    NS_ASSERTION(cmdLineArgs, "Invalid cmdLineArgs");   
    NS_ASSERTION(profileDirSet, "Invalid profileDirSet");   

    nsresult rv;
    nsXPIDLCString cmdResult;
    nsFileSpec currProfileDirSpec;

#ifdef DEBUG_profile_verbose
    printf("Profile Manager : Command Line Options : Begin\n");
#endif
 
    // check for command line arguments for profile manager
    //    
    // -P command line option works this way:
    // apprunner -P profilename 
    // runs the app using the profile <profilename> 
    // remembers profile for next time
    rv = cmdLineArgs->GetCmdLineValue(PROFILE_CMD_LINE_ARG, getter_Copies(cmdResult));
    if (NS_SUCCEEDED(rv))
    {
        if (cmdResult) {
            nsAutoString currProfileName; currProfileName.AssignWithConversion(cmdResult);

#ifdef DEBUG_profile
            printf("ProfileName : %s\n", (const char*)cmdResult);
#endif /* DEBUG_profile */
            PRBool exists;
            rv = ProfileExists(currProfileName.GetUnicode(), &exists);
            if (NS_FAILED(rv)) return rv;            
            
            if (!exists) {
                PRInt32 num5xProfiles = 0;
                PRInt32 num4xProfiles = 0;

                GetProfileCount(&num5xProfiles);
                Get4xProfileCount(&num4xProfiles);

                if (num5xProfiles == 0 && num4xProfiles == 0) {
                    profileURLStr = PROFILE_WIZARD_URL;
                }
                else if (num5xProfiles > 0) {
                    profileURLStr = PROFILE_SELECTION_URL;
                }
                else if (num4xProfiles > 0) {
                    profileURLStr = PROFILE_MANAGER_URL;
                }
                *profileDirSet = PR_FALSE;
            }
            else {
                rv = GetProfileDir(currProfileName.GetUnicode(), &currProfileDirSpec);
                if (NS_SUCCEEDED(rv)){
                    *profileDirSet = PR_TRUE;
                    mCurrentProfileAvailable = PR_TRUE;

                    // Need to load new profile prefs.
                    rv = LoadNewProfilePrefs();
                }
            }
        }
    }

    // -CreateProfile command line option works this way:
    // apprunner -CreateProfile profilename 
    // creates a new profile named <profilename> and sets the directory to your CWD 
    // runs app using that profile 
    // remembers profile for next time 
    //                         - OR -
    // apprunner -CreateProfile "profilename profiledir" 
    // creates a new profile named <profilename> and sets the directory to <profiledir> 
    // runs app using that profile 
    // remembers profile for next time

    rv = cmdLineArgs->GetCmdLineValue(CREATE_PROFILE_CMD_LINE_ARG, getter_Copies(cmdResult));
    if (NS_SUCCEEDED(rv))
    {
        if (cmdResult) {
            nsAutoString currProfileName; currProfileName.AssignWithConversion(strtok(NS_CONST_CAST(char*,(const char*)cmdResult), " "));
            nsAutoString currProfileDirString; currProfileDirString.AssignWithConversion(strtok(NULL, " "));
        
            if (!currProfileDirString.IsEmpty()) {
                currProfileDirSpec = currProfileDirString;
            }
            else {
                // No directory name provided. Get File Locator
                NS_WITH_SERVICE(nsIFileLocator, locator, kFileLocatorCID, &rv);
                if (NS_FAILED(rv) || !locator)
                    return NS_ERROR_FAILURE;
        
                // Get current profile, make the new one a sibling...
                nsCOMPtr <nsIFileSpec> spec;
                rv = locator->GetFileLocation(
                                 nsSpecialFileSpec::App_DefaultUserProfileRoot50, 
                                 getter_AddRefs(spec));

                if (NS_FAILED(rv) || !spec)
                    return NS_ERROR_FAILURE;
                spec->GetFileSpec(&currProfileDirSpec);

                rv = locator->ForgetProfileDir();
            }
#ifdef DEBUG_profile_verbose
            printf("profileName & profileDir are: %s\n", (const char*)cmdResult);
#endif /* DEBUG_profile */

            nsAutoString currProfileDir; currProfileDir.AssignWithConversion(currProfileDirSpec.GetNativePathCString());
            rv = CreateNewProfile(currProfileName.GetUnicode(), currProfileDir.GetUnicode(), nsnull, PR_TRUE);
            if (NS_SUCCEEDED(rv)) {
                *profileDirSet = PR_TRUE;
                mCurrentProfileAvailable = PR_TRUE;

                // Need to load new profile prefs.
                rv = LoadNewProfilePrefs();
                gProfileDataAccess->mProfileDataChanged = PR_TRUE;
                gProfileDataAccess->UpdateRegistry(nsnull);
            }
            rv = ForgetCurrentProfile();
            if (NS_FAILED(rv)) return rv;
        }
    }

    // Start Profile Manager
    rv = cmdLineArgs->GetCmdLineValue(PROFILE_MANAGER_CMD_LINE_ARG, getter_Copies(cmdResult));
    if (NS_SUCCEEDED(rv))
    {        
        if (cmdResult) {
            profileURLStr = PROFILE_MANAGER_URL;
        }
    }
    
    // Start Profile Selection
    rv = cmdLineArgs->GetCmdLineValue(PROFILE_SELECTION_CMD_LINE_ARG, getter_Copies(cmdResult));
    if (NS_SUCCEEDED(rv))
    {        
        if (cmdResult) {
            profileURLStr = PROFILE_SELECTION_URL;
        }
    }
    
    
    // Start Profile Wizard
    rv = cmdLineArgs->GetCmdLineValue(PROFILE_WIZARD_CMD_LINE_ARG, getter_Copies(cmdResult));
    if (NS_SUCCEEDED(rv))
    {        
        if (cmdResult) {
            profileURLStr = PROFILE_WIZARD_URL;
        }
    }

    // Start Migaration activity
    rv = cmdLineArgs->GetCmdLineValue(INSTALLER_CMD_LINE_ARG, getter_Copies(cmdResult));
    if (NS_SUCCEEDED(rv))
    {        
        if (cmdResult) {
            rv = MigrateProfileInfo();
            if (NS_FAILED(rv)) return rv;

            PRInt32 num4xProfiles = 0;
            rv = Get4xProfileCount(&num4xProfiles);
            if (NS_FAILED(rv)) return rv;
            
            PRInt32 numProfiles = 0;
            GetProfileCount(&numProfiles);
            if (num4xProfiles == 0 && numProfiles == 0) {
                // Let us create a default 5.0 profile
                CreateDefaultProfile();
                if (NS_FAILED(rv)) return rv;
            }
            else if (num4xProfiles == 0 && numProfiles == 1) {
                profileURLStr = "";
            }
            else if (num4xProfiles == 1 && numProfiles == 0) {
                PRBool confirmAutomigration = PR_FALSE;
                NS_WITH_SERVICE(nsIPref, prefs, kPrefCID, &rv)
                if (NS_SUCCEEDED(rv) && prefs) {
                    rv = prefs->GetBoolPref(PREF_CONFIRM_AUTOMIGRATION, 
                                            &confirmAutomigration);
                    if (NS_FAILED(rv)) confirmAutomigration = PR_FALSE;
                }
                if (confirmAutomigration) {
                    profileURLStr = CONFIRM_AUTOMIGRATE_URL;
                }
                else {
                    AutoMigrate();
                }
            }
            else if (numProfiles > 1)
            {
                profileURLStr = PROFILE_SELECTION_URL;
            }
            else {
                // show the profile manager
                profileURLStr = PROFILE_MANAGER_URL;
            }
        }
    }

#ifdef DEBUG_profile_verbose
    printf("Profile Manager : Command Line Options : End\n");
#endif

    return NS_OK;
}


/*
 * Getters
 */

// Gets the profiles directory for a given profile
// Sets the given profile to be a current profile
NS_IMETHODIMP nsProfile::GetProfileDir(const PRUnichar *profileName, nsFileSpec* profileDir)
{
    NS_ENSURE_ARG_POINTER(profileName);   
    NS_ENSURE_ARG_POINTER(profileDir);

    nsresult rv = NS_OK;

#if defined(DEBUG_profile_verbose)
    printf("ProfileManager : GetProfileDir\n");
#endif

    ProfileStruct    *aProfile;

    rv = gProfileDataAccess->GetValue(profileName, &aProfile);
    if (NS_FAILED(rv)) return rv;

	if (aProfile == nsnull)
		return NS_ERROR_FAILURE;

    nsCOMPtr<nsIFileSpec>spec;
    rv = NS_NewFileSpec(getter_AddRefs(spec));
    if (NS_FAILED(rv)) return rv;

    nsCAutoString profileLocation;
    profileLocation.AssignWithConversion(aProfile->profileLocation);
    rv = spec->SetPersistentDescriptorString(profileLocation.GetBuffer());
    if (NS_FAILED(rv)) return rv;

    rv = spec->GetFileSpec(profileDir);
    if (NS_FAILED(rv)) return rv;
                           
    // Set this to be a current profile only if it is a 5.0 profile
    if (aProfile->isMigrated.EqualsWithConversion(REGISTRY_YES_STRING))
    {
        gProfileDataAccess->SetCurrentProfile(profileName);
                                  
        nsFileSpec tmpFileSpec(*profileDir);
        
        PRBool inTrash = PR_FALSE;
        
#ifdef XP_MAC
        nsSpecialSystemDirectory trashFolder(nsSpecialSystemDirectory::Mac_TrashDirectory);
        inTrash = tmpFileSpec.IsChildOf(trashFolder);
#endif  
        
        if (inTrash || !tmpFileSpec.Exists()) 
        {
            // Get profile defaults folder..
            NS_WITH_SERVICE(nsIFileLocator, locator, kFileLocatorCID, &rv);
                                       
            if (NS_FAILED(rv) || !locator)
                return NS_ERROR_FAILURE;
                                        
            nsCOMPtr <nsIFileSpec> profDefaultsDir;

            rv = locator->GetFileLocation(
                             nsSpecialFileSpec::App_ProfileDefaultsFolder50, 
                             getter_AddRefs(profDefaultsDir));
            if (NS_FAILED(rv) || !profDefaultsDir)
                return NS_ERROR_FAILURE;
                                        
            nsFileSpec defaultsDirSpec;
            profDefaultsDir->GetFileSpec(&defaultsDirSpec);

#ifndef XP_MAC
            nsFilePath(tmpFileSpec.GetNativePathCString(), PR_TRUE);
#else
            // Build new profile folder. Update Registry entries.
            // Return new folder.
            
            // Get the default location for user profiles
            nsCOMPtr <nsIFileSpec> defaultRoot;
            rv = locator->GetFileLocation(
                             nsSpecialFileSpec::App_DefaultUserProfileRoot50, 
                             getter_AddRefs(defaultRoot));
        
            if (NS_FAILED(rv) || !defaultRoot)
                return NS_ERROR_FAILURE;

            defaultRoot->GetFileSpec(&tmpFileSpec);
            if (!tmpFileSpec.Exists())
                tmpFileSpec.CreateDirectory();

            // append profile name
            tmpFileSpec += NS_ConvertUCS2toUTF8(profileName);

            // Create New Directory. PersistentDescriptor needs an existing object.
            if (!tmpFileSpec.Exists())
                tmpFileSpec.CreateDirectory();

            // Get persistent string for profile directory            
            nsXPIDLCString profileDirString;
            nsCOMPtr<nsIFileSpec>dirSpec;
            rv = NS_NewFileSpecWithSpec(tmpFileSpec, getter_AddRefs(dirSpec));
            if (NS_SUCCEEDED(rv)) {
                rv = dirSpec->GetPersistentDescriptorString(getter_Copies(profileDirString));
            }
            if (NS_FAILED(rv)) return rv;
            
            // Update profile struct entries with new value.
            nsAutoString profileLoc; 
            profileLoc.AssignWithConversion(profileDirString);
            
            aProfile->profileLocation = profileLoc;
            gProfileDataAccess->SetValue(aProfile);

            // Return new file spec. 
            nsCOMPtr<nsIFileSpec>newSpec;
            rv = NS_NewFileSpec(getter_AddRefs(newSpec));
            if (NS_FAILED(rv)) return rv;
            rv = newSpec->SetPersistentDescriptorString(profileDirString);
            if (NS_FAILED(rv)) return rv;

            rv = newSpec->GetFileSpec(profileDir);
            if (NS_FAILED(rv)) return rv;
#endif
                                        
            // Copy contents from defaults folder.
            if (defaultsDirSpec.Exists())
                defaultsDirSpec.RecursiveCopy(tmpFileSpec);
        }
    }

	delete aProfile;
    return rv;
}

NS_IMETHODIMP nsProfile::GetDefaultProfileParentDir(nsIFileSpec **aDefaultProfileDir)
{
    NS_ENSURE_ARG_POINTER(aDefaultProfileDir);

    nsresult rv;

    NS_WITH_SERVICE(nsIFileLocator, locator, kFileLocatorCID, &rv);
    if (NS_FAILED(rv) || !locator) return NS_ERROR_FAILURE;

    rv = locator->GetFileLocation(
                     nsSpecialFileSpec::App_DefaultUserProfileRoot50, 
                     aDefaultProfileDir);
    if (NS_FAILED(rv) || !aDefaultProfileDir || !*aDefaultProfileDir) 
        return NS_ERROR_FAILURE;

    return NS_OK;
}

// Gets the number of profiles
// Location: Common/Profiles
NS_IMETHODIMP nsProfile::GetProfileCount(PRInt32 *numProfiles)
{
    NS_ENSURE_ARG_POINTER(numProfiles);

    *numProfiles = 0;

    gProfileDataAccess->GetNumProfiles(numProfiles);
    return NS_OK;
}


// If only a single profile exists
// return the name of the single profile.
// Otherwise it return the name of the first valid profile.
NS_IMETHODIMP nsProfile::GetFirstProfile(PRUnichar **profileName)
{
    NS_ENSURE_ARG_POINTER(profileName);

    gProfileDataAccess->GetFirstProfile(profileName);
    gProfileDataAccess->SetCurrentProfile(*profileName);

    return NS_OK;
}


// Returns the name of the current profile i.e., the last used profile
NS_IMETHODIMP
nsProfile::GetCurrentProfile(PRUnichar **profileName)
{
    NS_ENSURE_ARG_POINTER(profileName);

    gProfileDataAccess->GetCurrentProfile(profileName);
    return NS_OK;
}

// Returns the name of the current profile directory
NS_IMETHODIMP nsProfile::GetCurrentProfileDir(nsFileSpec* profileDir)
{
    NS_ENSURE_ARG_POINTER(profileDir);

    nsresult rv = NS_OK;

    nsXPIDLString profileName;

    GetCurrentProfile(getter_Copies(profileName));

    rv = GetProfileDir(profileName, profileDir);
    if (NS_FAILED(rv)) return rv;

    return rv;
}


/*
 * Setters
 */

// Sets the current profile directory
NS_IMETHODIMP nsProfile::SetProfileDir(const PRUnichar *profileName, nsFileSpec& profileDir)
{
    NS_ENSURE_ARG_POINTER(profileName);   

    nsresult rv = NS_OK;
 
    // Create a tmp Filespec and create a directory if required
    nsFileSpec tmpDir(profileDir);
                    
    if (!profileDir.Exists())
    {
        // nsPersistentFileDescriptor requires an existing
        // object. Make it first.
        tmpDir.CreateDirectory();
    }
                    
    nsXPIDLCString profileDirString;
                    
    nsCOMPtr<nsIFileSpec>spec;
    rv = NS_NewFileSpecWithSpec(profileDir, getter_AddRefs(spec));
    if (NS_SUCCEEDED(rv)) {
        rv = spec->GetPersistentDescriptorString(getter_Copies(profileDirString));
    }
    if (NS_FAILED(rv)) return rv;

    // Do I need to check for NS_ERROR_OUT_OF_MEMORY when I do a new on a class?
    ProfileStruct* aProfile = new ProfileStruct();

    nsAutoString profileLocation; profileLocation.AssignWithConversion(profileDirString);


    aProfile->profileName     = profileName;
    aProfile->profileLocation = profileLocation;
    aProfile->isMigrated.AssignWithConversion(REGISTRY_YES_STRING);


    rv = CreateUserDirectories(tmpDir);
    if (NS_FAILED(rv)) {
        delete aProfile;
        return rv;
    }

    gProfileDataAccess->SetValue(aProfile);
                           
    gProfileDataAccess->SetCurrentProfile(profileName);
    
    delete aProfile;

    return rv;
}


// Creates a new profile
NS_IMETHODIMP 
nsProfile::CreateNewProfile(const PRUnichar* profileName, 
                            const PRUnichar* nativeProfileDir,
                            const PRUnichar* langcode,
                            PRBool useExistingDir)
{
    NS_ENSURE_ARG_POINTER(profileName);   

    nsresult rv = NS_OK;

#if defined(DEBUG_profile)
    {
      printf("ProfileManager : CreateNewProfile\n");

      nsCAutoString temp1; temp1.AssignWithConversion(profileName);
      printf("Profile Name: %s\n", NS_STATIC_CAST(const char*, temp1));

      nsCAutoString temp2; temp2.AssignWithConversion(nativeProfileDir);
      printf("Profile Dir: %s\n", NS_STATIC_CAST(const char*, temp2));
    }
#endif

    NS_WITH_SERVICE(nsIFileLocator, locator, kFileLocatorCID, &rv);

    if (NS_FAILED(rv) || !locator)
        return NS_ERROR_FAILURE;

    if (!profileName) return NS_ERROR_FAILURE;

    nsFileSpec dirSpec;
    
    if (!nativeProfileDir || !*nativeProfileDir)
    {
        // They didn't specify a directory path...
        nsCOMPtr <nsIFileSpec> defaultRoot;
        rv = locator->GetFileLocation(
                         nsSpecialFileSpec::App_DefaultUserProfileRoot50, 
                         getter_AddRefs(defaultRoot));
        
        if (NS_FAILED(rv) || !defaultRoot)
            return NS_ERROR_FAILURE;

        defaultRoot->GetFileSpec(&dirSpec);
        if (!dirSpec.Exists())
            dirSpec.CreateDirectory();

        // append profile name
        dirSpec += NS_ConvertUCS2toUTF8(profileName);

        // Make profile directory unique only when the user 
        // decides to not use an already existing profile directory
        if (!useExistingDir)
            dirSpec.MakeUnique();
    }
    else {
        dirSpec = NS_ConvertUCS2toUTF8(nativeProfileDir);

        // this prevents people from choosing there profile directory
        // or another directory, and remove it when they delete the profile.
        // append profile name
        dirSpec += NS_ConvertUCS2toUTF8(profileName);

        // Make profile directory unique only when the user 
        // decides to not use an already existing profile directory
        if (!useExistingDir)
            dirSpec.MakeUnique();
    }

#if defined(DEBUG_profile_verbose)
    printf("before SetProfileDir\n");
#endif

    if (!dirSpec.Exists())
    {
        dirSpec.CreateDirectory();
        useExistingDir = PR_FALSE;
    }

    // Set the directory value and add the entry to the registry tree.
    // Creates required user directories.
    rv = SetProfileDir(profileName, dirSpec);

#if defined(DEBUG_profile_verbose)
    printf("after SetProfileDir\n");
#endif

    // Get profile defaults folder..
    nsCOMPtr <nsIFileSpec> profDefaultsDir;
    rv = locator->GetFileLocation(
                     nsSpecialFileSpec::App_ProfileDefaultsFolder50_nloc, 
                     getter_AddRefs(profDefaultsDir));
        
    if (NS_FAILED(rv) || !profDefaultsDir)
    {
        return NS_ERROR_FAILURE;
    }

    nsFileSpec defaultsDirSpec;

    profDefaultsDir->GetFileSpec(&defaultsDirSpec);

    if (langcode) {
        // caller prefers locale subdir
        nsFileSpec tmpdir; 
        tmpdir = defaultsDirSpec;
        tmpdir += NS_ConvertUCS2toUTF8(langcode);

        if (tmpdir.Exists())
            defaultsDirSpec = tmpdir;

        nsCOMPtr<nsIChromeRegistry> chromeRegistry = do_GetService(kChromeRegistryCID, &rv);
        if (NS_SUCCEEDED(rv)) {
            nsFileURL fileURL(dirSpec);
            const char* fileStr = fileURL.GetURLString();
            rv = chromeRegistry->SelectLocaleForProfile(langcode, 
                                                        NS_ConvertUTF8toUCS2(fileStr));
        }
    }
    // Copy contents from defaults folder.
    if (defaultsDirSpec.Exists() && (!useExistingDir))
    {
        defaultsDirSpec.RecursiveCopy(dirSpec);
    }

    gProfileDataAccess->mNumProfiles++;
    gProfileDataAccess->mProfileDataChanged = PR_TRUE;
    gProfileDataAccess->UpdateRegistry(nsnull);
    return NS_OK;
}

// Create required user directories like ImapMail, Mail, News, Cache etc.
nsresult nsProfile::CreateUserDirectories(const nsFileSpec& profileDir)
{
    nsresult rv = NS_OK;

#if defined(DEBUG_profile_verbose)
    printf("ProfileManager : CreateUserDirectories\n");
#endif

    nsFileSpec tmpDir;
    
    tmpDir = profileDir;
    tmpDir += NEW_NEWS_DIR_NAME;

    if (!tmpDir.Exists())
        tmpDir.CreateDirectory();

    tmpDir = profileDir;
    tmpDir += NEW_IMAPMAIL_DIR_NAME;

    if (!tmpDir.Exists())
        tmpDir.CreateDirectory();
    
    tmpDir = profileDir;
    tmpDir += NEW_MAIL_DIR_NAME;

    if (!tmpDir.Exists())
        tmpDir.CreateDirectory();
    
    tmpDir = profileDir;
    tmpDir += "Cache";

    if (!tmpDir.Exists())
        tmpDir.CreateDirectory();

    return rv;
}


// Delete all user directories associated with the a profile
// A FileSpec of the profile's directory is taken as input param
nsresult nsProfile::DeleteUserDirectories(const nsFileSpec& profileDir)
{
    nsresult rv = NS_OK;

#if defined(DEBUG_profile_verbose)
    printf("ProfileManager : DeleteUserDirectories\n");
#endif

    if (profileDir.Exists())
        profileDir.Delete(PR_TRUE);
    
    return rv;
}

// Rename a old profile to new profile.
// Copies all the keys from old profile to new profile.
// Deletes the old profile from the registry
NS_IMETHODIMP 
nsProfile::RenameProfile(const PRUnichar* oldName, const PRUnichar* newName)
{
    NS_ENSURE_ARG_POINTER(oldName);   
    NS_ENSURE_ARG_POINTER(newName);   

    nsresult rv = NS_OK;

#if defined(DEBUG_profile)
    {
      printf("ProfileManager : Renaming profile\n");

      nsCAutoString temp1; temp1.AssignWithConversion(oldName);
      printf("Old name:  %s\n", NS_STATIC_CAST(const char*, temp1));

      nsCAutoString temp2; temp2.AssignWithConversion(newName);
      printf("New name:  %s\n", NS_STATIC_CAST(const char*, temp2));
    }
#endif

    PRBool exists;
    rv = ProfileExists(newName, &exists);
    if (NS_FAILED(rv)) return rv;
    
    // That profile already exists...
    if (exists) {
#if defined(DEBUG_profile)  
        printf("ProfileManager : Rename Operation failed : Profile exists. Provide a different new name for profile.\n");
#endif
        return NS_ERROR_FAILURE;
    }

    // Copy reg keys
    rv = CopyRegKey(oldName, newName);
    if (NS_FAILED(rv)) return rv;
     
    // Delete old profile entry
    rv = DeleteProfile(oldName, PR_FALSE /* don't delete files */);
    if (NS_FAILED(rv)) return rv;
     
	/* note, we do not rename the directory on disk to the new name
	 * this is on purpose.
     *
	 * we don't require the directory name to match the profile name, 
	 * but it usually does.  
	 * (the pairing of values occurs in the profile registry)
	 * 
	 * Imagine this scenario:
	 * 1) user creates a profile "foo" and the directory gets named "foo".
 	 * 2) user creates a profile "bar" and the directory gets named "bar"
	 * 3) user deletes the profile "foo", but chooses not to delete the files on disk.  (they are given this option when deleting a profile)
	 * 4) user renames "bar" profile to "foo", but still uses the directory named "bar" on disk
	 *
	 * bad things would happen if we tried to rename the directory
	 */
    
    /* profile is just replaced. But Keep up the count */
    gProfileDataAccess->mNumProfiles++;

    rv = ForgetCurrentProfile();
    if (NS_FAILED(rv)) return rv;

    gProfileDataAccess->mProfileDataChanged = PR_TRUE;
    gProfileDataAccess->UpdateRegistry(nsnull);

    return NS_OK;
}

// Copy old profile entries to the new profile
// In the process creates new profile subtree.
nsresult nsProfile::CopyRegKey(const PRUnichar *oldProfile, const PRUnichar *newProfile)
{
    NS_ENSURE_ARG_POINTER(oldProfile);   
    NS_ENSURE_ARG_POINTER(newProfile);   

    nsresult rv = NS_OK;

    ProfileStruct    *aProfile;

    rv = gProfileDataAccess->GetValue(oldProfile, &aProfile);
    if (NS_FAILED(rv)) return rv;

    aProfile->profileName        = newProfile;

    rv = gProfileDataAccess->SetValue(aProfile);

	delete aProfile;

    return rv;
}

NS_IMETHODIMP nsProfile::ForgetCurrentProfile()
{
    nsresult rv = NS_OK;

    // Remove the current profile subtree from the registry.
    PRUnichar tmp[] = { '\0' };

    gProfileDataAccess->SetCurrentProfile(tmp);
    if (NS_FAILED(rv)) return rv;  

    gProfileDataAccess->mForgetProfileCalled = PR_TRUE;
    
    NS_WITH_SERVICE(nsIFileLocator, locator, kFileLocatorCID, &rv);
    if (NS_FAILED(rv)) return rv;

    if (!locator) return NS_ERROR_FAILURE;

    rv = locator->ForgetProfileDir();
    if (NS_FAILED(rv)) return rv;  

    return rv;
}

// Delete a profile from the registry
// Not deleting the directories on the harddisk yet.
// 4.x kind of confirmation need to be implemented yet
NS_IMETHODIMP nsProfile::DeleteProfile(const PRUnichar* profileName, PRBool canDeleteFiles)
{
    NS_ENSURE_ARG_POINTER(profileName);   

    nsresult rv = NS_OK;
 
    rv = ForgetCurrentProfile();
    if (NS_FAILED(rv)) return rv;
    
    // If user asks for it, delete profile directory
    if (canDeleteFiles) {
        nsFileSpec profileDirSpec;
        
        rv = GetProfileDir(profileName, &profileDirSpec);
        if (NS_FAILED(rv)) return rv;
        
        rv = DeleteUserDirectories(profileDirSpec);
        if (NS_FAILED(rv)) return rv;
    }

    // Remove the subtree from the registry.
    gProfileDataAccess->RemoveSubTree(profileName);
    if (NS_FAILED(rv)) return rv;

    gProfileDataAccess->mProfileDataChanged = PR_TRUE;
    gProfileDataAccess->UpdateRegistry(nsnull);

    return rv;
}

// Get the list of all profiles
// Populate the input param.
// This method is written to support the core service
// call to get the names all profiles.
NS_IMETHODIMP nsProfile::GetProfileList(PRUnichar **profileListStr)
{
    NS_ENSURE_ARG_POINTER(profileListStr);

    gProfileDataAccess->GetProfileList(profileListStr);
    return NS_OK;
}


// launch the application with a profile of user's choice
// Prefs and FileLocation services are used here.
// FileLocation service to make ir forget about the global profile dir it had.
// Prefs service to kick off the startup to start the app with new profile's prefs.
NS_IMETHODIMP nsProfile::StartApprunner(const PRUnichar* profileName)
{
    NS_ENSURE_ARG_POINTER(profileName);   

    nsresult rv = NS_OK;

#if defined(DEBUG_profile)
    {
      printf("ProfileManager : StartApprunner\n");

      nsCAutoString temp; temp.AssignWithConversion(profileName);
      printf("profileName passed in: %s", NS_STATIC_CAST(const char*, temp));
    }
#endif

    gProfileDataAccess->SetCurrentProfile(profileName);
    mCurrentProfileAvailable = PR_TRUE;

    NS_WITH_SERVICE(nsIFileLocator, locator, kFileLocatorCID, &rv);
    if (NS_FAILED(rv)) return rv;

    if (!locator) return NS_ERROR_FAILURE;
    
    rv = locator->ForgetProfileDir();
    if (NS_FAILED(rv)) {
#ifdef DEBUG_profile
        printf("failed to forget the profile dir\n"); 
#endif /* DEBUG_profile */
        return rv;
    }

    // Update registry entries
    gProfileDataAccess->mProfileDataChanged = PR_TRUE;
    gProfileDataAccess->UpdateRegistry(nsnull);
    
    // Need to load new profile prefs.
    rv = LoadNewProfilePrefs();
    return rv;
}

NS_IMETHODIMP nsProfile::LoadNewProfilePrefs()
{
    nsresult rv;
    NS_WITH_SERVICE(nsIPref, prefs, kPrefCID, &rv);
    if (NS_FAILED(rv)) return rv;

    prefs->ResetPrefs();
    prefs->ReadUserPrefs();

    return NS_OK;
}


// Migrate profile information from the 4x registry to 5x registry.
NS_IMETHODIMP nsProfile::MigrateProfileInfo()
{
    nsresult rv = NS_OK;

#if defined(XP_PC) || defined(XP_MAC)

#if defined(DEBUG_profile_verbose)
    printf("Entered MigrateProfileInfo.\n");
#endif

    char oldRegFile[_MAX_LENGTH] = {'\0'};

#ifdef XP_PC
#ifdef XP_OS2
    nsSpecialSystemDirectory systemDir(nsSpecialSystemDirectory::OS2_OS2Directory);
#else
    // Registry file has been traditionally stored in the windows directory (XP_PC).
    nsSpecialSystemDirectory systemDir(nsSpecialSystemDirectory::Win_WindowsDirectory);
#endif
    
    // Append the name of the old registry to the path obtained.
    PL_strcpy(oldRegFile, systemDir.GetNativePathCString());
    PL_strcat(oldRegFile, OLD_REGISTRY_FILE_NAME);
#else /* XP_MAC */
    nsSpecialSystemDirectory regLocation(nsSpecialSystemDirectory::Mac_SystemDirectory);
    
    // Append the name of the old registry to the path obtained.
    regLocation += "Preferences";
    regLocation += OLD_REGISTRY_FILE_NAME;
    
    PL_strcpy(oldRegFile, regLocation.GetNativePathCString());
#endif /* XP_PC */

    rv = gProfileDataAccess->Get4xProfileInfo(oldRegFile);

#elif defined (XP_BEOS)
#else
    /* XP_UNIX */
    rv = gProfileDataAccess->Get4xProfileInfo(nsnull);
#endif /* XP_PC || XP_MAC */

    gProfileDataAccess->mProfileDataChanged = PR_TRUE;
    gProfileDataAccess->UpdateRegistry(nsnull);

	return rv;
}

nsresult
nsProfile::CopyDefaultFile(nsIFileSpec *profDefaultsDir, nsFileSpec& newProfDir, const char *fileName)
{
    nsFileSpec defaultsDirSpecFile;

    profDefaultsDir->GetFileSpec(&defaultsDirSpecFile);
    
    defaultsDirSpecFile += fileName;

    if (defaultsDirSpecFile.Exists())
    {
        defaultsDirSpecFile.CopyToDir(newProfDir);
    }

	return NS_OK;
}

// Migrate a selected profile
// Set the profile to the current profile....debatable.
// Calls PrefMigration service to do the Copy and Diverge
// of 4x Profile information
NS_IMETHODIMP 
nsProfile::MigrateProfile(const PRUnichar* profileName, PRBool showProgressAsModalWindow)
{
    NS_ENSURE_ARG_POINTER(profileName);   

    nsresult rv = NS_OK;

#if defined(DEBUG_profile)
    printf("Inside Migrate Profile routine.\n" );
#endif

    nsFileSpec oldProfDir;
    nsFileSpec newProfDir;

    rv = GetProfileDir(profileName, &oldProfDir);
    if (NS_FAILED(rv)) return rv;
    
    // Create new profile dir path
    NS_WITH_SERVICE(nsIFileLocator, locator, kFileLocatorCID, &rv);
    if (NS_FAILED(rv) || !locator) return NS_ERROR_FAILURE;
    
    // Get current profile, make the new one a sibling...
    nsCOMPtr<nsIFileSpec> newSpec;
    rv = locator->GetFileLocation(
                     nsSpecialFileSpec::App_DefaultUserProfileRoot50, 
                     getter_AddRefs(newSpec));
    if (!newSpec)
        return NS_ERROR_FAILURE;

    newSpec->GetFileSpec(&newProfDir);
    newProfDir += NS_ConvertUCS2toUTF8(profileName);
    newProfDir.MakeUnique();
    if (newProfDir.Exists()) {
#ifdef DEBUG_profile
        printf("directory already exists\n");
#endif
        return NS_ERROR_FAILURE;
    }

    // Call migration service to do the work.
    nsCOMPtr <nsIPrefMigration> pPrefMigrator;


    rv = nsComponentManager::CreateInstance(kPrefMigrationCID, 
                                            nsnull,
                                            NS_GET_IID(nsIPrefMigration),
                                            getter_AddRefs(pPrefMigrator));
    if (NS_FAILED(rv)) return rv;
    if (!pPrefMigrator) return NS_ERROR_FAILURE;
        
    nsXPIDLCString oldProfDirStr;
    nsXPIDLCString newProfDirStr;

    if (!newProfDir.Exists()) {
        newProfDir.CreateDirectory();
    }    

    rv = GetStringFromSpec(newProfDir, getter_Copies(newProfDirStr));
    if (NS_FAILED(rv)) return rv;
    
    if (!oldProfDir.Exists()) {
        return NS_ERROR_FAILURE;
    }
    rv = GetStringFromSpec(oldProfDir, getter_Copies(oldProfDirStr));
    if (NS_FAILED(rv)) return rv;

    // you can do this a bunch of times.
    rv = pPrefMigrator->AddProfilePaths(oldProfDirStr, newProfDirStr);  

    rv = pPrefMigrator->ProcessPrefs(showProgressAsModalWindow);
    if (NS_FAILED(rv)) return rv;

    // check for diskspace errors  
    nsresult errorCode;   
    errorCode = pPrefMigrator->GetError();

    // In either of the cases below we have to return error to make
    // app understand that migration has failed.
    if (errorCode == CREATE_NEW)
    {
        PRInt32 numProfiles = 0;
        ShowProfileWizard();

        // When the automigration process fails because of disk space error,
        // we present user a create profile wizard if the user chooses to create a 
        // a profile then. But then the user may click on cancel on that dialog...
        // So, if the user clicks on cancel, the number of profiles should be 
        // ZERO at the point for the user who failed to automigrate single 4x profile.
        // On such condition, set mDiskSpaceErrorQuitCalled to allow user to quit the app.
        // If the user is presented with profilemanager dialog with multiple 4x profiles
        // to migrate, value of mDiskSpaceErrorQuitCalled does not matter as it gets ignored..
        // If a single profile needs automigration and no confirmation 
        // is needed for that operation mAutomigrate is set to false. 
        if (!mAutomigrate)
        {
            GetProfileCount(&numProfiles);
            if (numProfiles == 0)
                mDiskSpaceErrorQuitCalled = PR_TRUE;
        }
        mOutofDiskSpace = PR_TRUE;
        return NS_ERROR_FAILURE;
    }
    else if (errorCode == CANCEL) 
    {
        // When the automigration process fails because of disk space error,
        // user may choose to simply quit the app from the dialog presented 
        // by pref-migrator. So, set mDiskSpaceErrorQuitCalled to allow user 
        // to quit the app in such a case.
        // If the user is presented with profilemanager dialog with multiple 4x profiles
        // to migrate, value of mDiskSpaceErrorQuitCalled does not matter as it gets ignored..
        // If a single profile needs automigration and no confirmation 
        // is needed for that operation mAutomigrate is set to false. 
        if (!mAutomigrate)
            mDiskSpaceErrorQuitCalled = PR_TRUE;

        ForgetCurrentProfile();
        mOutofDiskSpace = PR_TRUE;
        return NS_ERROR_FAILURE;
    }
    else if (errorCode != SUCCESS) 
    {
        return NS_ERROR_FAILURE;
    }

    // Copy the default 5.0 profile files into the migrated profile
    // Get profile defaults folder..
    nsCOMPtr <nsIFileSpec> profDefaultsDir;
    rv = locator->GetFileLocation(
                     nsSpecialFileSpec::App_ProfileDefaultsFolder50, 
                     getter_AddRefs(profDefaultsDir));
        
    if (NS_FAILED(rv) || !profDefaultsDir)
    {
        return NS_ERROR_FAILURE;
    }

    // Copy panels.rdf & localstore.rdf files
    // This is a hack. Once the localFileSpec implementation
    // is complete, this will be removed.
	rv = CopyDefaultFile(profDefaultsDir, newProfDir, LOCALSTORE_RDF_FILE);
	if (NS_FAILED(rv)) return rv;
	rv = CopyDefaultFile(profDefaultsDir, newProfDir, PANELS_RDF_FILE);
	if (NS_FAILED(rv)) return rv;
    // hack finish.
	
    rv = SetProfileDir(profileName, newProfDir);
    if (NS_FAILED(rv)) return rv;

    gProfileDataAccess->mNumProfiles++;
    gProfileDataAccess->mNumOldProfiles--;

    gProfileDataAccess->mProfileDataChanged = PR_TRUE;
    gProfileDataAccess->UpdateRegistry(nsnull);

    return rv;
}

nsresult
nsProfile::ShowProfileWizard(void)
{
    nsresult rv = NS_OK;
    PRBool hasParentWindow = PR_FALSE;
    nsCOMPtr<nsIDOMWindowInternal> PMDOMWindow;

    // Get the window mediator
    NS_WITH_SERVICE(nsIWindowMediator, windowMediator, kWindowMediatorCID, &rv);
    if (NS_SUCCEEDED(rv)) 
    {
        nsCOMPtr<nsISimpleEnumerator> windowEnumerator;

        if (NS_SUCCEEDED(windowMediator->GetEnumerator(nsnull, getter_AddRefs(windowEnumerator)))) 
        {
            // Get each dom window
            PRBool more;
            windowEnumerator->HasMoreElements(&more);
            while (more) 
            {
                nsCOMPtr<nsISupports> protoWindow;
                rv = windowEnumerator->GetNext(getter_AddRefs(protoWindow));
                if (NS_SUCCEEDED(rv) && protoWindow) 
                {
                    PMDOMWindow = do_QueryInterface(protoWindow);
                    if (PMDOMWindow) 
                    {
                        hasParentWindow = PR_TRUE;
                        break;
                    }
                }
                windowEnumerator->HasMoreElements(&more);
            }
        }
    }

    if (hasParentWindow)
    {
        // Get the script global object for the window
        nsCOMPtr<nsIScriptGlobalObject> sgo;
        sgo = do_QueryInterface(PMDOMWindow);
        if (!sgo) return NS_ERROR_FAILURE;

        // Get the script context from the global context
        nsCOMPtr<nsIScriptContext> scriptContext;
        sgo->GetContext( getter_AddRefs(scriptContext));
        if (!scriptContext) return NS_ERROR_FAILURE;

        // Get the JSContext from the script context
        JSContext* jsContext = (JSContext*)scriptContext->GetNativeContext();
        if (!jsContext) return NS_ERROR_FAILURE;

    
        //-----------------------------------------------------
        // Create the nsIDialogParamBlock to pass the trigger
        // list to the dialog
        //-----------------------------------------------------
        nsCOMPtr<nsIDialogParamBlock> ioParamBlock;
        rv = nsComponentManager::CreateInstance(kDialogParamBlockCID,
                                            nsnull,
                                            NS_GET_IID(nsIDialogParamBlock),
                                            getter_AddRefs(ioParamBlock));

    
        if ( NS_SUCCEEDED( rv ) ) 
            ioParamBlock->SetInt(0,4); // standard wizard buttons

 
        void* stackPtr;
        jsval *argv = JS_PushArguments( jsContext,
                                    &stackPtr,
                                    "sss%ip",
                                    PROFILE_WIZARD_URL,
                                    "_blank",
                                    "chrome,modal",
                                    (const nsIID*)(&NS_GET_IID(nsIDialogParamBlock)),
                                    (nsISupports*)ioParamBlock);

        if (argv)
        {
            nsCOMPtr<nsIDOMWindowInternal> newWindow;
            rv = PMDOMWindow->OpenDialog(jsContext,
                                         argv,
                                         4,
                                         getter_AddRefs(newWindow));
            if (NS_SUCCEEDED(rv))
            {
                JS_PopArguments( jsContext, stackPtr);
            }
            else
                return NS_ERROR_FAILURE;
        }
        else
            return NS_ERROR_FAILURE;     

    }
    else
    {
        // No parent window is available.
        // So, Create top level window with create profile wizard
        NS_WITH_SERVICE(nsIAppShellService, wizAppShell,
                          kAppShellServiceCID, &rv);
        if (NS_FAILED(rv)) return rv;

        nsCOMPtr<nsIURI> profURI;
        rv = NS_NewURI(getter_AddRefs(profURI), NS_LITERAL_CSTRING(PROFILE_WIZARD_URL));
        if (NS_FAILED(rv)) return rv;

        nsCOMPtr<nsIXULWindow> newWindow;
        rv = wizAppShell->CreateTopLevelWindow(nsnull, profURI,
                                                PR_TRUE, PR_TRUE, CHROME_STYLE,
                                                NS_SIZETOCONTENT,           // width 
                                                NS_SIZETOCONTENT,           // height
                                                getter_AddRefs(newWindow));

        if (NS_FAILED(rv)) return rv;

        /*
         * Bring up the wizard...
         */    
        rv = wizAppShell->Run();
    }
    return rv;

}

NS_IMETHODIMP nsProfile::ProfileExists(const PRUnichar *profileName, PRBool *exists)
{
    NS_ENSURE_ARG_POINTER(profileName); 
    NS_ENSURE_ARG_POINTER(exists);

    *exists = gProfileDataAccess->ProfileExists(profileName);
    return NS_OK;
}

NS_IMETHODIMP nsProfile::IsCurrentProfileAvailable(PRBool *available)
{
    NS_ENSURE_ARG_POINTER(available);

    *available = mCurrentProfileAvailable;
    return NS_OK;
}

// Gets the number of unmigrated 4x profiles
// Location: Common/Profiles
NS_IMETHODIMP nsProfile::Get4xProfileCount(PRInt32 *numProfiles)
{
    NS_ENSURE_ARG_POINTER(numProfiles);

    *numProfiles = 0;

    gProfileDataAccess->GetNum4xProfiles(numProfiles);
    return NS_OK;
}


// Migrates all unmigrated profiles
NS_IMETHODIMP nsProfile::MigrateAllProfiles()
{
    nsresult rv = NS_OK;
    for (PRInt32 i=0; i < gProfileDataAccess->mNumOldProfiles; i++)
    {
        ProfileStruct* profileItem = (ProfileStruct *)
                                     (gProfileDataAccess->m4xProfiles->ElementAt(i));
        rv = MigrateProfile(profileItem->profileName.GetUnicode(), 
                            PR_FALSE /* don't show progress as modal window */);
        if (NS_FAILED(rv)) return rv;
    }

    return rv;
}


nsresult nsProfile::RenameProfileDir(const PRUnichar* newProfileName) 
{
    NS_ASSERTION(newProfileName, "Invalid new profile name");      

    nsresult rv = NS_OK;
    nsFileSpec dirSpec;

    rv = GetProfileDir(newProfileName, &dirSpec);
    if (NS_FAILED(rv)) return rv;
    
    nsFileSpec renamedDirSpec = dirSpec;
    renamedDirSpec.SetLeafName(NS_ConvertUCS2toUTF8(newProfileName));
    renamedDirSpec.MakeUnique();

    // rename the directory
    rv = dirSpec.Rename(renamedDirSpec.GetLeafName());
    if (NS_FAILED(rv)) return rv;

    // update the registry
    rv = SetProfileDir(newProfileName, dirSpec);
    if (NS_FAILED(rv)) return rv;

    return NS_OK;
}

NS_IMETHODIMP nsProfile::CloneProfile(const PRUnichar* newProfile)
{
    NS_ENSURE_ARG_POINTER(newProfile);   

    nsresult rv = NS_OK;

#if defined(DEBUG_profile)
    printf("ProfileManager : CloneProfile\n");
#endif
    nsFileSpec currProfileDir;
    nsFileSpec newProfileDir;

    NS_WITH_SERVICE(nsIFileLocator, locator, kFileLocatorCID, &rv);
    if (NS_FAILED(rv) || !locator) return NS_ERROR_FAILURE;

    GetCurrentProfileDir(&currProfileDir);

    if (currProfileDir.Exists())
    {
        nsCOMPtr <nsIFileSpec> dirSpec;
        rv = locator->GetFileLocation(
                         nsSpecialFileSpec::App_DefaultUserProfileRoot50, 
                         getter_AddRefs(dirSpec));
        
        if (NS_FAILED(rv) || !dirSpec)
        return NS_ERROR_FAILURE;

        //Append profile name to form a directory name
        dirSpec->GetFileSpec(&newProfileDir);

        // TODO:
        // hash profileName  (will MakeUnique do that for us?)
        // don't allow special characters (like ..)
        // make acceptable length (will MakeUnique do that for us?)
        newProfileDir += NS_ConvertUCS2toUTF8(newProfile);
        newProfileDir.MakeUnique();

        if (newProfileDir.Exists()) {
#ifdef DEBUG_profile
            printf("directory already exists\n");
#endif
            return NS_ERROR_FAILURE;
        }

        currProfileDir.RecursiveCopy(newProfileDir);
        
        rv = SetProfileDir(newProfile, newProfileDir);
    }


#if defined(DEBUG_profile_verbose)
    {
      if (NS_SUCCEEDED(rv))
      printf("ProfileManager : Cloned CurrentProfile\n");

      nsCAutoString temp; temp.AssignWithConversion(newProfile);
      printf("The new profile is ->%s<-\n", NS_STATIC_CAST(const char*, temp));
    }
#endif

    gProfileDataAccess->mNumProfiles++;
    gProfileDataAccess->mProfileDataChanged = PR_TRUE;

    rv = locator->ForgetProfileDir();
    if (NS_FAILED(rv)) return rv;  

    return rv;
}

nsresult
nsProfile::CreateDefaultProfile(void)
{
    nsresult rv = NS_OK;

    nsFileSpec profileDirSpec;
    
    // Get the default user profiles folder
    NS_WITH_SERVICE(nsIFileLocator, locator, kFileLocatorCID, &rv);
    if (NS_FAILED(rv) || !locator)
        return NS_ERROR_FAILURE;

    nsCOMPtr <nsIFileSpec> spec;
    rv = locator->GetFileLocation(
                     nsSpecialFileSpec::App_DefaultUserProfileRoot50, 
                     getter_AddRefs(spec));
    if (NS_FAILED(rv) || !spec)
        return NS_ERROR_FAILURE;
    spec->GetFileSpec(&profileDirSpec);

    rv = locator->ForgetProfileDir();

    nsAutoString dirSpecStr; dirSpecStr.AssignWithConversion(profileDirSpec.GetNativePathCString());

    nsAutoString defaultProfileName; defaultProfileName.AssignWithConversion(DEFAULT_PROFILE_NAME);
    rv = CreateNewProfile(defaultProfileName.GetUnicode(), dirSpecStr.GetUnicode(), nsnull, PR_TRUE);

    return rv;
}

NS_IMETHODIMP 
nsProfile::UpdateRegistry(nsIFile* regName)
{
   nsresult rv = NS_OK;

   gProfileDataAccess->mProfileDataChanged = PR_TRUE;
   rv= gProfileDataAccess->UpdateRegistry(regName);

   return rv;
}

NS_IMETHODIMP 
nsProfile::SetRegStrings(const PRUnichar* profileName, 
                         const PRUnichar* regString,
                         const PRUnichar* regName,
                         const PRUnichar* regEmail,
                         const PRUnichar* regOption)
{
   nsresult rv = NS_OK;

   ProfileStruct*    aProfile;

   rv = gProfileDataAccess->GetValue(profileName, &aProfile);
   if (NS_FAILED(rv)) return rv;
   
   aProfile->NCHavePregInfo = regString;

   if (regName)    aProfile->NCProfileName   = regName;
   if (regEmail)   aProfile->NCEmailAddress  = regEmail;
   if (regOption)  aProfile->NCDeniedService = regOption;

   gProfileDataAccess->SetValue(aProfile);

   delete aProfile;

   return rv;
}

NS_IMETHODIMP 
nsProfile::IsRegStringSet(const PRUnichar *profileName, char **regString)
{
    NS_ENSURE_ARG_POINTER(profileName);   
    NS_ENSURE_ARG_POINTER(regString);

    gProfileDataAccess->CheckRegString(profileName, regString);
    return NS_OK;
}

/*
 * nsIDirectoryServiceProvider Implementation
 */
 
// File Name Defines
#if defined(XP_MAC)

#else

#endif

#define PREFS_FILE_50_NAME          "prefs.js"
#define USER_CHROME_DIR_50_NAME     "Chrome"
#define LOCAL_STORE_FILE_50_NAME    "localstore.rdf"
#define HISTORY_FILE_50_NAME        "history.dat"
#define PANELS_FILE_50_NAME         "panels.rdf"
#define MIME_TYPES_FILE_50_NAME     "mimeTypes.rdf"
#define BOOKMARKS_FILE_50_NAME      "bookmarks.html"
#define SEARCH_FILE_50_NAME         "search.rdf" 
#define MAIL_DIR_50_NAME            "Mail"
#define IMAP_MAIL_DIR_50_NAME       "ImapMail"
#define NEWS_DIR_50_NAME            "News"
#define MSG_FOLDER_CACHE_DIR_50_NAME "panacea.dat"

NS_IMETHODIMP
nsProfile::GetFile(const char *prop, PRBool *persistant, nsIFile **_retval)
{
    nsCOMPtr<nsILocalFile>  localFile;
    nsresult rv = NS_ERROR_FAILURE;

    *_retval = nsnull;
    
    // Set persistant to TRUE - When the day comes when we switch profiles, we'll just
    // Undefine our properties. This is more efficent because these files are accessed
    // a lot more frequently than the profile will change.
    
    *persistant = PR_TRUE;

    nsIAtom* inAtom = NS_NewAtom(prop);
    NS_ENSURE_TRUE(inAtom, NS_ERROR_OUT_OF_MEMORY);

    if (inAtom == sApp_PrefsDirectory50)
    {
        rv = CloneProfileDirectorySpec(getter_AddRefs(localFile));
    }
    else if (inAtom == sApp_PreferencesFile50)
    {
        rv = CloneProfileDirectorySpec(getter_AddRefs(localFile));
        if (NS_SUCCEEDED(rv))
            rv = localFile->Append(PREFS_FILE_50_NAME); // AppendRelativePath
    }
    else if (inAtom == sApp_UserProfileDirectory50)
    {
        rv = CloneProfileDirectorySpec(getter_AddRefs(localFile));
    }
    else if (inAtom == sApp_UserChromeDirectory)
    {
        rv = CloneProfileDirectorySpec(getter_AddRefs(localFile));
        if (NS_SUCCEEDED(rv))
            rv = localFile->Append(USER_CHROME_DIR_50_NAME);
    }
    else if (inAtom == sApp_LocalStore50)
    {
        rv = CloneProfileDirectorySpec(getter_AddRefs(localFile));
        if (NS_SUCCEEDED(rv))
            rv = localFile->Append(LOCAL_STORE_FILE_50_NAME);
    }
    else if (inAtom == sApp_History50)
    {
        rv = CloneProfileDirectorySpec(getter_AddRefs(localFile));
        if (NS_SUCCEEDED(rv))
            rv = localFile->Append(HISTORY_FILE_50_NAME);
    }
    else if (inAtom == sApp_UsersPanels50)
    {
        // Here we differ from nsFileLocator - It checks for the
        // existance of this file and if it does not exist, copies
        // it from the defaults folder to the profile folder. Since
        // WE set up any profile folder, we'll make sure it's copied then.
        
        rv = CloneProfileDirectorySpec(getter_AddRefs(localFile));
        if (NS_SUCCEEDED(rv))
            rv = localFile->Append(PANELS_FILE_50_NAME);
    }
    else if (inAtom == sApp_UsersMimeTypes50)
    {
        // Here we differ from nsFileLocator - It checks for the
        // existance of this file and if it does not exist, copies
        // it from the defaults folder to the profile folder. Since
        // WE set up any profile folder, we'll make sure it's copied then.
        
        rv = CloneProfileDirectorySpec(getter_AddRefs(localFile));
        if (NS_SUCCEEDED(rv))
            rv = localFile->Append(MIME_TYPES_FILE_50_NAME);
    }
    else if (inAtom == sApp_BookmarksFile50)
    {
        rv = CloneProfileDirectorySpec(getter_AddRefs(localFile));
        if (NS_SUCCEEDED(rv))
            rv = localFile->Append(BOOKMARKS_FILE_50_NAME);
    }
    else if (inAtom == sApp_SearchFile50)
    {
        // Here we differ from nsFileLocator - It checks for the
        // existance of this file and if it does not exist, copies
        // it from the defaults folder to the profile folder. Since
        // WE set up any profile folder, we'll make sure it's copied then.
        
        rv = CloneProfileDirectorySpec(getter_AddRefs(localFile));
        if (NS_SUCCEEDED(rv))
            rv = localFile->Append(SEARCH_FILE_50_NAME);
    }
    else if (inAtom == sApp_MailDirectory50)
    {
        rv = CloneProfileDirectorySpec(getter_AddRefs(localFile));
        if (NS_SUCCEEDED(rv))
            rv = localFile->Append(MAIL_DIR_50_NAME);
    }
    else if (inAtom == sApp_ImapMailDirectory50)
    {
        rv = CloneProfileDirectorySpec(getter_AddRefs(localFile));
        if (NS_SUCCEEDED(rv))
            rv = localFile->Append(IMAP_MAIL_DIR_50_NAME);
    }
    else if (inAtom == sApp_NewsDirectory50)
    {
        rv = CloneProfileDirectorySpec(getter_AddRefs(localFile));
        if (NS_SUCCEEDED(rv))
            rv = localFile->Append(NEWS_DIR_50_NAME);
    }
    else if (inAtom == sApp_MessengerFolderCache50)
    {
        rv = CloneProfileDirectorySpec(getter_AddRefs(localFile));
        if (NS_SUCCEEDED(rv))
            rv = localFile->Append(MSG_FOLDER_CACHE_DIR_50_NAME);
    }

    NS_RELEASE(inAtom);

    if (localFile && NS_SUCCEEDED(rv))
    	return localFile->QueryInterface(NS_GET_IID(nsIFile), (void**)_retval);
    	
    return rv;
}

nsresult nsProfile::CloneProfileDirectorySpec(nsILocalFile **aLocalFile)
{
    nsresult rv;    
    nsFileSpec currentDir;
    
    rv = GetCurrentProfileDir(&currentDir); // This should probably be cached...
    NS_ENSURE_SUCCESS(rv, rv);
    
#if defined(XP_MAC)
    nsCOMPtr<nsILocalFileMac> localFileMac;
    FSSpec  tempSpec = currentDir.GetFSSpec();
    rv = NS_NewLocalFileWithFSSpec(&tempSpec, PR_TRUE, getter_AddRefs(localFileMac));
    if (NS_SUCCEEDED(rv))
        rv = localFileMac->QueryInterface(NS_GET_IID(nsILocalFile), aLocalFile);
#else
    rv = NS_NewLocalFile(currentDir.GetNativePathCString(), PR_TRUE, aLocalFile);
#endif
    
    return rv;
}

