/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
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


#include "nsProxyEvent.h"
#include "nsProxyObjectManager.h"
#include "nsProxyEventPrivate.h"

#include "nsIProxyCreateInstance.h"

#include "nsRepository.h"
#include "nsIServiceManager.h"
#include "nsCOMPtr.h"

#include "nsIAppShellComponentImpl.h"
#include "nsIEventQueueService.h"



static NS_DEFINE_IID(kIFactoryIID, NS_IFACTORY_IID);
static NS_DEFINE_IID( kAppShellServiceCID, NS_APPSHELL_SERVICE_CID );
static NS_DEFINE_IID( kEventQueueServiceCID, NS_EVENTQUEUESERVICE_CID );


/***************************************************************************/
/* nsProxyCreateInstance                                                   */
/* This private class will allow us to create Instances on another thread  */
/***************************************************************************/
class nsProxyCreateInstance : public nsIProxyCreateInstance
{
    NS_DECL_ISUPPORTS
    NS_IMETHOD CreateInstanceByIID(const nsIID & cid, nsISupports *aOuter, const nsIID & iid, void * *result);
    NS_IMETHOD CreateInstanceByProgID(const char *aProgID, nsISupports *aOuter, const nsIID & iid, void * *result);

    nsProxyCreateInstance();
    virtual ~nsProxyCreateInstance();

};

nsProxyCreateInstance::nsProxyCreateInstance()
{
    NS_INIT_REFCNT();
    NS_ADDREF_THIS();
}

nsProxyCreateInstance::~nsProxyCreateInstance()
{
}
static NS_DEFINE_IID(kIProxyCreateInstanceIID,NS_IPROXYCREATEINSTANCE_IID);
NS_IMPL_ISUPPORTS(nsProxyCreateInstance, kIProxyCreateInstanceIID);

NS_IMETHODIMP nsProxyCreateInstance::CreateInstanceByIID(const nsIID & cid, nsISupports *aOuter, const nsIID & iid, void * *result)
{
    return nsComponentManager::CreateInstance(  cid, 
                                                aOuter,
                                                iid,
                                                result);
}


NS_IMETHODIMP nsProxyCreateInstance::CreateInstanceByProgID(const char *aProgID, nsISupports *aOuter, const nsIID & iid, void * *result)
{
    return nsComponentManager::CreateInstance(  aProgID, 
                                                aOuter,
                                                iid,
                                                result);
}











/////////////////////////////////////////////////////////////////////////
// nsProxyObjectManager
/////////////////////////////////////////////////////////////////////////

nsProxyObjectManager* nsProxyObjectManager::mInstance = nsnull;

static NS_DEFINE_IID(kIProxyEventManager, NS_IPROXYEVENT_MANAGER_IID);
NS_IMPL_ISUPPORTS(nsProxyObjectManager, kIProxyEventManager)

nsProxyObjectManager::nsProxyObjectManager()
{
    NS_INIT_REFCNT();

    mProxyClassMap = new nsHashtable(256, PR_TRUE);
    mProxyObjectMap = new nsHashtable(256, PR_TRUE);
}

nsProxyObjectManager::~nsProxyObjectManager()
{
    delete mProxyClassMap;
    delete mProxyObjectMap;
}

nsProxyObjectManager *
nsProxyObjectManager::GetInstance()
{
    if (mInstance == NULL) 
    {
        mInstance = new nsProxyObjectManager();
    }
    return mInstance;
}


// Helpers
NS_IMETHODIMP 
nsProxyObjectManager::Create(nsISupports* outer, const nsIID& aIID, void* *aInstancePtr)
{
    nsProxyObjectManager *proxyObjectManager = GetInstance();

    if (proxyObjectManager == NULL)
        return NS_ERROR_OUT_OF_MEMORY;

    return proxyObjectManager->QueryInterface(aIID, aInstancePtr);
}


NS_IMETHODIMP 
nsProxyObjectManager::GetProxyObject(nsIEventQueue *destQueue, REFNSIID aIID, nsISupports* aObj, ProxyType proxyType, void** aProxyObject)
{
    nsIEventQueue *postQ = destQueue;

    *aProxyObject = nsnull;

    if (postQ == nsnull)
    {
        // Get app shell service.
        nsIAppShellService *appShell;
        nsresult rv = nsServiceManager::GetService( kAppShellServiceCID,
                                                    nsIAppShellService::GetIID(),
                                                    (nsISupports**)&appShell );

        if ( NS_FAILED( rv ) )
            return NS_ERROR_UNEXPECTED;

        PRThread *aThread;
        rv = appShell->GetPrimordialThread(&aThread);
        
        if ( NS_FAILED( rv ) )
        {
            NS_RELEASE(appShell);
            return NS_ERROR_UNEXPECTED;
        }

        nsIEventQueueService *eventQService;

        rv = nsServiceManager::GetService( kEventQueueServiceCID,
                                           nsIEventQueueService::GetIID(),
                                           (nsISupports**)&eventQService );

        if ( NS_FAILED( rv ) )
        {
            NS_RELEASE(appShell);
            return NS_ERROR_UNEXPECTED;
        }
        
        rv = eventQService->GetThreadEventQueue(aThread, &postQ);
        
        if ( NS_FAILED( rv ) )
        {
            NS_RELEASE(appShell);
            NS_RELEASE(eventQService);
            return NS_ERROR_UNEXPECTED;
        }
    
        NS_RELEASE(appShell);
        NS_RELEASE(eventQService);
    
    }

    // check to see if proxy is there or not.
    *aProxyObject = nsProxyEventObject::GetNewOrUsedProxy(postQ, proxyType, aObj, aIID);
    if (*aProxyObject != nsnull)
    {
        return NS_OK;
    }
    
    return NS_ERROR_NO_INTERFACE; //fix error code?
}   


NS_IMETHODIMP 
nsProxyObjectManager::GetProxyObject(nsIEventQueue *destQueue, 
                                     const nsCID &aClass, 
                                     nsISupports *aDelegate, 
                                     const nsIID &aIID, 
                                     ProxyType proxyType, 
                                     void** aProxyObject)
{
    *aProxyObject = nsnull;
    
    // 1. Create a proxy for creating an instance on another thread.

    nsIProxyCreateInstance* ciProxy = nsnull;
    nsProxyCreateInstance* ciObject = new nsProxyCreateInstance();
    
    if (ciObject == nsnull)
        return NS_ERROR_NULL_POINTER;

    nsresult rv = GetProxyObject(destQueue, nsIProxyCreateInstance::GetIID(), ciObject, PROXY_SYNC, (void**)&ciProxy);
    
    if (NS_FAILED(rv))
    {
        delete ciObject;
        return rv;
    }
        
    // 2. now create a new instance of the request object via our proxy.

    nsISupports* aObj;

    rv = ciProxy->CreateInstanceByIID(aClass, 
                                      aDelegate, 
                                      aIID, 
                                      (void**)&aObj);

    
    // 3.  Delete the create instance proxy and its real object.
    
    delete ciObject;
    NS_RELEASE(ciProxy);


    // 4.  Check to see if creating the requested instance failed.
    if ( NS_FAILED(rv))
    {
        return rv;
    }

    // 5.  Now create a proxy object for the requested object.

    rv = GetProxyObject(destQueue, aIID, aObj, proxyType, aProxyObject);

    
    // 6. release ownership of aObj so that aProxyObject owns it.
    
    NS_RELEASE(aObj);

    // 7. return the error returned from GetProxyObject.  Either way, we our out of here.

    return rv;   
}

nsHashtable* 
nsProxyObjectManager::GetRealObjectToProxyObjectMap()
{
    NS_VERIFY(mProxyObjectMap, "no hashtable");
    return mProxyObjectMap;
}   

nsHashtable*
nsProxyObjectManager::GetIIDToProxyClassMap()
{
    NS_VERIFY(mProxyClassMap, "no hashtable");
    return mProxyClassMap;
}   



/////////////////////////////////////////////////////////////////////////
// nsProxyEventFactory
/////////////////////////////////////////////////////////////////////////
nsProxyEventFactory::nsProxyEventFactory(void)
{
    NS_INIT_REFCNT();
}

nsProxyEventFactory::~nsProxyEventFactory(void)
{
}

NS_IMPL_ISUPPORTS(nsProxyEventFactory,kIFactoryIID)

NS_IMETHODIMP
nsProxyEventFactory::CreateInstance(nsISupports *aOuter, REFNSIID aIID, void **aResult)
{
    if (aResult == NULL)
    {
        return NS_ERROR_NULL_POINTER;
    }

    *aResult = NULL;

    nsProxyObjectManager *inst = nsProxyObjectManager::GetInstance();

    if (inst == NULL)
        return NS_ERROR_OUT_OF_MEMORY;

    nsresult result =  inst->QueryInterface(aIID, aResult);

    if (NS_FAILED(result)) 
    {
        *aResult = NULL;
    }

    NS_ADDREF(inst);  // Are we sure that we need to addref???

    return result;

}

NS_IMETHODIMP
nsProxyEventFactory::LockFactory(PRBool aLock)
{
// not implemented.
    return NS_ERROR_NOT_IMPLEMENTED;
}




////////////////////////////////////////////////////////////////////////////////
// DLL Entry Points:
////////////////////////////////////////////////////////////////////////////////
static NS_DEFINE_IID(kProxyObjectManagerCID, NS_PROXYEVENT_MANAGER_CID);
static NS_DEFINE_CID(kComponentManagerCID, NS_COMPONENTMANAGER_CID);

extern "C" NS_EXPORT PRBool
NSCanUnload(nsISupports* aServMgr)
{
    return 0;
}

extern "C" NS_EXPORT nsresult
NSRegisterSelf(nsISupports* aServMgr, const char *path)
{
    nsresult rv;

    nsCOMPtr<nsIServiceManager> servMgr(do_QueryInterface(aServMgr, &rv));
    if (NS_FAILED(rv)) return rv;

    nsIComponentManager* compMgr;
    rv = servMgr->GetService(kComponentManagerCID, 
                             nsIComponentManager::GetIID(), 
                             (nsISupports**)&compMgr);
    if (NS_FAILED(rv)) return rv;

#ifdef NS_DEBUG
    printf("*** nsProxyObjectManager is being registered.  Hold on to your seat...\n");
#endif

    rv = compMgr->RegisterComponent(kProxyObjectManagerCID, NULL, NULL, path, PR_TRUE, PR_TRUE);
    if (NS_FAILED(rv)) goto done;
  
  done:
    (void)servMgr->ReleaseService(kComponentManagerCID, compMgr);
    return rv;
}

extern "C" NS_EXPORT nsresult
NSUnregisterSelf(nsISupports* aServMgr, const char *path)
{
    nsresult rv;

    nsCOMPtr<nsIServiceManager> servMgr(do_QueryInterface(aServMgr, &rv));
    if (NS_FAILED(rv)) return rv;

    nsIComponentManager* compMgr;
    rv = servMgr->GetService(kComponentManagerCID, 
                             nsIComponentManager::GetIID(), 
                             (nsISupports**)&compMgr);
    if (NS_FAILED(rv)) return rv;

#ifdef NS_DEBUG
    printf("*** nsProxyObjectManager is being unregistered.  Na na na na hey hey\n");
#endif
    
    rv = compMgr->UnregisterComponent(kProxyObjectManagerCID, path);
    if (NS_FAILED(rv)) goto done;

  done:
    (void)servMgr->ReleaseService(kComponentManagerCID, compMgr);
    return rv;
}



extern "C" NS_EXPORT nsresult
NSGetFactory(nsISupports* aServMgr,
             const nsCID &aClass,
             const char *aClassName,
             const char *aProgID,
             nsIFactory **aFactory)
{
    if (aFactory == NULL)
    {
        return NS_ERROR_NULL_POINTER;
    }

    *aFactory = NULL;
    nsISupports *inst;

    
    if (aClass.Equals(kProxyObjectManagerCID) )
        inst = new nsProxyEventFactory();
    else
        return NS_ERROR_ILLEGAL_VALUE;

    if (inst == NULL)
        return NS_ERROR_OUT_OF_MEMORY;

    nsresult res = inst->QueryInterface(kIFactoryIID, (void**) aFactory);

    if (NS_FAILED(res)) 
    {   
        delete inst;
    }

    return res;
}
