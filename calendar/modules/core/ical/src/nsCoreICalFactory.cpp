/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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

#include "nscore.h"
#include "nsIFactory.h"
#include "nsISupports.h"
#include "nsCoreCIID.h"

#include "nsCalendar.h"
#include "nsCalVEvent.h"
#include "nsCalDateTimeProperty.h"
#include "nsCalStringProperty.h"
#include "nsCalIntegerProperty.h"
#include "nsCalAttendeeProperty.h"
#include "nscalcoreicalCIID.h"

static NS_DEFINE_IID(kISupportsIID,      NS_ISUPPORTS_IID);
static NS_DEFINE_IID(kIFactoryIID,       NS_IFACTORY_IID);
static NS_DEFINE_IID(kCCalendarCID, NS_CALICALENDARVCALENDAR_CID);
static NS_DEFINE_IID(kCCalVEventCID, NS_CALICALENDARVEVENT_CID);
static NS_DEFINE_IID(kCCalDateTimePropertyCID, NS_CALDATETIMEPROPERTY_CID);
static NS_DEFINE_IID(kCCalStringPropertyCID, NS_CALSTRINGPROPERTY_CID);
static NS_DEFINE_IID(kCCalIntegerPropertyCID, NS_CALINTEGERPROPERTY_CID);
static NS_DEFINE_IID(kCCalAttendeePropertyCID, NS_CALATTENDEEPROPERTY_CID);

class nsCoreICalFactory : public nsIFactory
{   
  public:   
    // nsISupports methods   
    NS_IMETHOD QueryInterface(const nsIID &aIID,    
                              void **aResult);   
    NS_IMETHOD_(nsrefcnt) AddRef(void);   
    NS_IMETHOD_(nsrefcnt) Release(void);   

    // nsIFactory methods   
    NS_IMETHOD CreateInstance(nsISupports *aOuter,   
                              const nsIID &aIID,   
                              void **aResult);   

    NS_IMETHOD LockFactory(PRBool aLock);   

    nsCoreICalFactory(const nsCID &aClass);   
    ~nsCoreICalFactory();   

  private:   
    nsrefcnt  mRefCnt;   
    nsCID     mClassID;
};   

nsCoreICalFactory::nsCoreICalFactory(const nsCID &aClass)   
{   
  mRefCnt = 0;
  mClassID = aClass;
}   

nsCoreICalFactory::~nsCoreICalFactory()   
{   
  NS_ASSERTION(mRefCnt == 0, "non-zero refcnt at destruction");   
}   

nsresult nsCoreICalFactory::QueryInterface(const nsIID &aIID,   
                                      void **aResult)   
{   
  if (aResult == NULL) {   
    return NS_ERROR_NULL_POINTER;   
  }   

  // Always NULL result, in case of failure   
  *aResult = NULL;   

  if (aIID.Equals(kISupportsIID)) {   
    *aResult = (void *)(nsISupports*)this;   
  } else if (aIID.Equals(kIFactoryIID)) {   
    *aResult = (void *)(nsIFactory*)this;   
  }   

  if (*aResult == NULL) {   
    return NS_NOINTERFACE;   
  }   

  AddRef(); // Increase reference count for caller   
  return NS_OK;   
}   

nsrefcnt nsCoreICalFactory::AddRef()   
{   
  return ++mRefCnt;   
}   

nsrefcnt nsCoreICalFactory::Release()   
{   
  if (--mRefCnt == 0) {   
    delete this;   
    return 0; // Don't access mRefCnt after deleting!   
  }   
  return mRefCnt;   
}  

nsresult nsCoreICalFactory::CreateInstance(nsISupports *aOuter,  
                                      const nsIID &aIID,  
                                      void **aResult)  
{  
  if (aResult == NULL) {  
    return NS_ERROR_NULL_POINTER;  
  }  

  *aResult = NULL;  
  
  nsISupports *inst = nsnull;

  if (mClassID.Equals(kCCalendarCID)) {
    inst = (nsISupports *)(nsICalendar *)new nsCalendar();
  }
  else if (mClassID.Equals(kCCalDateTimePropertyCID)) {
    inst = (nsISupports *)(nsICalProperty *)new nsCalDateTimeProperty();
  }
  else if (mClassID.Equals(kCCalStringPropertyCID)) {
    inst = (nsISupports *)(nsICalProperty *)new nsCalStringProperty();
  }
  else if (mClassID.Equals(kCCalAttendeePropertyCID)) {
    inst = (nsISupports *)(nsICalProperty *)new nsCalAttendeeProperty();
  }
  else if (mClassID.Equals(kCCalIntegerPropertyCID)) {
    inst = (nsISupports *)(nsICalProperty *)new nsCalIntegerProperty();
  }
  else if (mClassID.Equals(kCCalVEventCID)) {
    inst = (nsISupports *)(nsICalVEvent *)new nsCalVEvent();
  }

  if (inst == NULL) {  
    return NS_ERROR_OUT_OF_MEMORY;  
  }  

  nsresult res = inst->QueryInterface(aIID, aResult);

  if (res != NS_OK) {  
    // We didn't get the right interface, so clean up  
    delete inst;  
  }  

  return res;  
}  

nsresult nsCoreICalFactory::LockFactory(PRBool aLock)  
{  
  // Not implemented in simplest case.  
  return NS_OK;
}  

// return the proper factory to the caller
extern "C" NS_EXPORT nsresult NSGetFactory(const nsCID &aClass, nsIFactory **aFactory)
{
  if (nsnull == aFactory) {
    return NS_ERROR_NULL_POINTER;
  }

  *aFactory = new nsCoreICalFactory(aClass);

  if (nsnull == aFactory) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  return (*aFactory)->QueryInterface(kIFactoryIID, (void**)aFactory);
}

