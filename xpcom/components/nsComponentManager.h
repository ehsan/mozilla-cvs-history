/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

#ifndef nsComponentManager_h__
#define nsComponentManager_h__

#include "nsIComponentManager.h"
#include "nsIRegistry.h"
#include "nsHashtable.h"
#include "prtime.h"
#include "prmon.h"

class nsFactoryEntry;
class nsDll;

// Registry Factory creation function defined in nsRegistry.cpp
// We hook into this function locally to create and register the registry
// Since noone outside xpcom needs to know about this and nsRegistry.cpp
// does not have a local include file, we are putting this definition
// here rather than in nsIRegistry.h
extern "C" NS_EXPORT nsresult NS_RegistryGetFactory(nsIFactory** aFactory);

////////////////////////////////////////////////////////////////////////////////

class nsComponentManagerImpl : public nsIComponentManager {
public:
    NS_DECL_ISUPPORTS

    // nsIComponentManager methods:
    NS_IMETHOD FindFactory(const nsCID &aClass,
                           nsIFactory **aFactory);

    // Finds a class ID for a specific Program ID
    NS_IMETHOD ProgIDToCLSID(const char *aProgID,
                             nsCID *aClass);
  
    // Finds a Program ID for a specific class ID
    // caller frees the result with delete[]
    NS_IMETHOD CLSIDToProgID(nsCID *aClass,
                             char* *aClassName,
                             char* *aProgID);
  
    // Creates a class instance for a specific class ID
    NS_IMETHOD CreateInstance(const nsCID &aClass, 
                              nsISupports *aDelegate,
                              const nsIID &aIID,
                              void **aResult);

    // Convenience routine, creates a class instance for a specific ProgID
    NS_IMETHOD CreateInstance(const char *aProgID,
                              nsISupports *aDelegate,
                              const nsIID &aIID,
                              void **aResult);

    // Manually registry a factory for a class
    NS_IMETHOD RegisterFactory(const nsCID &aClass,
                               const char *aClassName,
                               const char *aProgID,
                               nsIFactory *aFactory,
                               PRBool aReplace);

    // Manually register a dynamically loaded component.
    NS_IMETHOD RegisterComponent(const nsCID &aClass,
                                 const char *aClassName,
                                 const char *aProgID,
                                 const char *aLibrary,
                                 PRBool aReplace,
                                 PRBool aPersist);

    // Manually unregister a factory for a class
    NS_IMETHOD UnregisterFactory(const nsCID &aClass,
                                 nsIFactory *aFactory);

    // Manually unregister a dynamically loaded component
    NS_IMETHOD UnregisterComponent(const nsCID &aClass,
                                   const char *aLibrary);

    // Unload dynamically loaded factories that are not in use
    NS_IMETHOD FreeLibraries(void);

    //////////////////////////////////////////////////////////////////////////////
    // DLL registration support
    // Autoregistration will try only files with these extensions.
    // All extensions are case insensitive.
    // ".dll",    // Windows
    // ".dso",    // Unix
    // ".so",     // Unix
    // ".sl",     // Unix: HP
    // ".shlb",	// Mac
    // ".dlm",    // new for all platforms
    //
    // Directory and fullname are what NSPR will accept. For eg.
    //	MAC		/Hard drive/mozilla/dist/bin
    // 	WIN		y:\Hard drive\mozilla\dist\bin (or) y:/Hard drive/mozilla/dist/bin
    //	UNIX	/Hard drive/mozilla/dist/bin
    //
    NS_IMETHOD AutoRegister(RegistrationTime when, const char* directory);
    NS_IMETHOD AutoRegisterComponent(RegistrationTime when, const char *fullname);

    // nsComponentManagerImpl methods:
    nsComponentManagerImpl();
    virtual ~nsComponentManagerImpl();

    static nsComponentManagerImpl* gComponentManager;
    nsresult Init(void);

protected:
    nsresult LoadFactory(nsFactoryEntry *aEntry, nsIFactory **aFactory);

    nsresult SyncComponentsInDir(RegistrationTime when, const char *directory);
    nsresult SelfRegisterDll(nsDll *dll);
    nsresult SelfUnregisterDll(nsDll *dll);
    nsresult HashProgID(const char *aprogID, const nsCID &aClass);

    // The following functions are the only ones that operate on the persistent
    // registry
    nsresult PlatformInit(void);
    nsresult PlatformVersionCheck();
    nsresult PlatformCreateDll(const char *fullname, nsDll* *result);
    nsresult PlatformMarkNoComponents(nsDll *dll);
    nsresult PlatformRegister(const char *cidString, const char *className, const char *progID, nsDll *dll);
    nsresult PlatformUnregister(const char *cidString, const char *aLibrary);
    nsresult PlatformFind(const nsCID &aCID, nsFactoryEntry* *result);
    nsresult PlatformProgIDToCLSID(const char *aProgID, nsCID *aClass);
    nsresult PlatformCLSIDToProgID(nsCID *aClass, char* *aClassName, char* *aProgID);
    void     PlatformGetFileInfo(nsIRegistry::Key Key,PRTime *lastModifiedTime,PRUint32 *fileSize);

protected:
    nsHashtable*     mFactories;
    nsHashtable*     mProgIDs;
    PRMonitor*       mMon;
    nsHashtable*     mDllStore;
    nsIRegistry*     mRegistry;
    nsIRegistry::Key mXPCOMKey;
    nsIRegistry::Key mClassesKey;
    nsIRegistry::Key mCLSIDKey;
};

#define NS_MAX_FILENAME_LEN	1024

#define NS_ERROR_IS_DIR NS_ERROR_GENERATE_FAILURE(NS_ERROR_MODULE_XPCOM, 24)

#ifdef XP_UNIX
/* The default registry on the unix system is $HOME/.mozilla/registry per
 * vr_findGlobalRegName(). vr_findRegFile() will create the registry file
 * if it doesn't exist. But it wont create directories.
 *
 * Hence we need to create the directory if it doesn't exist already.
 *
 * Why create it here as opposed to the app ?
 * ------------------------------------------
 * The app cannot create the directory in main() as most of the registry
 * and initialization happens due to use of static variables.
 * And we dont want to be dependent on the order in which
 * these static stuff happen.
 *
 * Permission for the $HOME/.mozilla will be Read,Write,Execute
 * for user only. Nothing to group and others.
 */
#define NS_MOZILLA_DIR_NAME		".mozilla"
#define NS_MOZILLA_DIR_PERMISSION	00700
#endif /* XP_UNIX */

/**
 * When using the registry we put a version number in it.
 * If the version number that is in the registry doesn't match
 * the following, we ignore the registry. This lets news versions
 * of the software deal with old formats of registry and not
 *
 * alpha0.20 : First time we did versioning
 * alpha0.30 : Changing autoreg to begin registration from ./components on unix
 * alpha0.40 : repository -> component manager
 * alpha0.50 : using nsIRegistry
 * alpha0.60 : xpcom 2.0 landing
 */
#define NS_XPCOM_COMPONENT_MANAGER_VERSION_STRING "alpha0.60"

////////////////////////////////////////////////////////////////////////////////
/**
 * Class: nsFactoryEntry()
 *
 * There are two types of FactoryEntries.
 *
 * 1. {CID, dll} mapping.
 *		Factory is a consequence of the dll. These can be either session
 *		specific or persistent based on whether we write this
 *		to the registry or not.
 *
 * 2. {CID, factory} mapping
 *		These are strictly session specific and in memory only.
 */

class nsFactoryEntry {
public:
    nsFactoryEntry();
    nsFactoryEntry(const nsCID &aClass, nsIFactory *aFactory);
    ~nsFactoryEntry();

    nsresult Init(nsHashtable* dllHashtable, const nsCID &aClass, const char *aLibrary,
                  PRTime lastModTime, PRUint32 fileSize);

    nsCID cid;
    nsIFactory *factory;

    // DO NOT DELETE THIS. Many nsFactoryEntry(s) could be sharing the same Dll.
    // This gets deleted from the dllStore going away.
    nsDll *dll;	
};

////////////////////////////////////////////////////////////////////////////////

#endif // nsComponentManager_h__
