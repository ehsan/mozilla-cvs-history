/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "MPL"); you may not use this file except in
 * compliance with the MPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of the Original Code is Alexander. Portions
 * created by Alexander Larsson are Copyright (C) 1999
 * Alexander Larsson. All Rights Reserved. 
 */
#include "GtkMozillaContainer.h"

#include "nsRepository.h"
#include "nsIWebShell.h"
#include "nsIURL.h"
#ifdef NECKO
#include "nsIIOService.h"
static NS_DEFINE_CID(kIOServiceCID, NS_IOSERVICE_CID);
#endif // NECKO
#include "nsFileSpec.h"
#include "nsIDocumentLoader.h"
#include "nsIContentViewer.h"
#include "prprf.h"

static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);
static NS_DEFINE_IID(kIWebShellIID, NS_IWEB_SHELL_IID);
static NS_DEFINE_IID(kWebShellCID, NS_WEB_SHELL_CID);
static NS_DEFINE_IID(kIWebShellContainerIID, NS_IWEB_SHELL_CONTAINER_IID);
static NS_DEFINE_IID(kIDocumentLoaderFactoryIID,   NS_IDOCUMENTLOADERFACTORY_IID);

GtkMozillaContainer::GtkMozillaContainer(GtkMozilla *moz)
{
  mWebShell = nsnull;
  width = height = 0;
  mStream = nsnull;
  
  mozilla = moz;
  gtk_widget_set_app_paintable(GTK_WIDGET(moz), PR_TRUE);
}


GtkMozillaContainer::~GtkMozillaContainer(void)
{
  NS_IF_RELEASE(mWebShell);
}

void
GtkMozillaContainer::Show()
{
  GtkAllocation *alloc = &GTK_WIDGET(mozilla)->allocation;
    
  nsresult rv = nsRepository::CreateInstance(kWebShellCID, nsnull,
					     kIWebShellIID,
					     (void**)&mWebShell);

  if (NS_FAILED(rv) || !mWebShell) {
    printf("Cannot create WebShell!");
    return;
  }
  
  if (mozilla) {
    width = alloc->width;
    height = alloc->height;

    //printf("Init, size: %d, %d\n", width, height);
    mWebShell->Init((nsNativeWidget *)mozilla,
		    0, 0,
		    width, height);
    
    mWebShell->SetContainer(this);
  }
  if (mWebShell) {
    mWebShell->Show();
  }
}

void
GtkMozillaContainer::Resize(gint w, gint h)
{
  int new_size;
  //  printf("GtkMozillaContainer::Resize called width: %d, %d\n", w, h);
  new_size = ((width != w) || (height != h));
  if (new_size && mWebShell) {
    width = w;
    height = h;
    //printf("GtkMozillaContainer::Resize setting to: %d, %d\n", width, height);
    gtk_layout_set_size(GTK_LAYOUT(mozilla), width, height);
    mWebShell->SetBounds(0, 0, width, height);
  }
}

void
GtkMozillaContainer::LoadURL(const gchar *url)
{
  PRUnichar *u_url;
  int len, i;
  len = strlen(url);
  
  u_url = new (PRUnichar)[len+1];
  for (i=0;i<len;i++)
    u_url[i] = (PRUnichar) url[i];
  u_url[len] = 0;
  mWebShell->LoadURL(u_url);
  delete [] u_url;
}

void
GtkMozillaContainer::Stop()
{
  mWebShell->Stop();
}

void
GtkMozillaContainer::Reload(GtkMozillaReloadType type)
{
 mWebShell->Reload((nsURLReloadType)type);
}

gint
GtkMozillaContainer::Back()
{
  return NS_SUCCEEDED(mWebShell->Back());
}

gint
GtkMozillaContainer::CanBack()
{
 return mWebShell->CanBack()==NS_OK;
}

gint
GtkMozillaContainer::Forward()
{
 return NS_SUCCEEDED(mWebShell->Forward());
}

gint
GtkMozillaContainer::CanForward()
{
  nsresult rv = mWebShell->CanForward();
  return mWebShell->CanForward()==NS_OK;
}

gint
GtkMozillaContainer::GoTo(gint history_index)
{
  return NS_SUCCEEDED(mWebShell->GoTo((PRInt32)history_index));
}

gint
GtkMozillaContainer::GetHistoryLength()
{
  PRInt32 Result;
  
  if (NS_SUCCEEDED(mWebShell->GetHistoryLength(Result)))
    return (gint) Result;
  else
    return 0;
}

gint
GtkMozillaContainer::GetHistoryIndex()
{
  PRInt32 Result;
  
  if (NS_SUCCEEDED(mWebShell->GetHistoryIndex(Result)))
    return (gint) Result;
  else
    return 0;
}


NS_IMETHODIMP
GtkMozillaContainer::QueryInterface(REFNSIID aIID, void** aInstancePtr)
{
  if (NULL == aInstancePtr) {
    return NS_ERROR_NULL_POINTER;
  }

  nsISupports *ifp = nsnull;

  if (aIID.Equals(kIWebShellContainerIID)) {
    ifp = (nsIWebShellContainer*)this;
  } else if(aIID.Equals(kISupportsIID)) {
    ifp = this;
  } else {
    *aInstancePtr = 0;
   
    return NS_NOINTERFACE;
  }

  *aInstancePtr = (void *)ifp;

  NS_ADDREF(ifp);
  
  return NS_OK;
}


nsrefcnt
GtkMozillaContainer::AddRef()
{
  // This object is not reference counted so we just return a number > 0
  return 3;
}


nsrefcnt
GtkMozillaContainer::Release()
{
  // This object is not reference counted so we just return a number > 0
  return 2;
}

static char *simple_unicode_to_char(const PRUnichar* aURL)
{
  int i;
  int len=0;
  const PRUnichar* ptr;
  char *str;

  ptr=aURL;
  while (*ptr++)
    len++;

  str = (char *)malloc(len+1);
  if (str==NULL)
    return NULL;
  
  for (i=0;i<len;i++) {
    if (aURL[i]<127)
      str[i] = (char) aURL[i]&0xff;
    else
      str[i] = '_';
  }
  str[len] = 0;
  return str;
}

NS_IMETHODIMP
GtkMozillaContainer::WillLoadURL(nsIWebShell* aShell,
                                 const PRUnichar* aURL,
                                 nsLoadType aReason)
{
  gint result = 1;
  char *url = simple_unicode_to_char(aURL);
  gtk_signal_emit_by_name(GTK_OBJECT(mozilla), "will_load_url",
			  url, (gint) aReason, &result);
  if (url)
    free(url);
  
  return (result)?NS_OK:NS_ERROR_FAILURE;
}


NS_IMETHODIMP
GtkMozillaContainer::BeginLoadURL(nsIWebShell* aShell, const PRUnichar* aURL)
{
  char *url = simple_unicode_to_char(aURL);
  gtk_signal_emit_by_name(GTK_OBJECT(mozilla), "begin_load_url", url);
  if (url)
    free(url);
  return NS_OK;
}

NS_IMETHODIMP
GtkMozillaContainer::EndLoadURL(nsIWebShell* aShell,
                                const PRUnichar* aURL, PRInt32 aStatus)
{
  char *url = simple_unicode_to_char(aURL);
  gtk_signal_emit_by_name(GTK_OBJECT(mozilla), "end_load_url",
			  url, (gint) aStatus);
  if (url)
    free(url);
  return NS_OK;
}


nsresult
GtkMozillaContainer::CreateContentViewer(nsIURI* aURL, 
                                         const char* aContentType, 
                                         const char *aCommand,
                                         nsIContentViewerContainer* aContainer,
                                         nsISupports* aExtraInfo,
                                         nsIStreamListener** aDocListenerResult,
                                         nsIContentViewer** aDocViewerResult)
{
    // Lookup class-id for the command plus content-type combination
    nsCID cid;
    char id[500];
    PR_snprintf(id, sizeof(id),
                NS_DOCUMENT_LOADER_FACTORY_PROGID_PREFIX "%s/%s",
                aCommand, aContentType);
    nsresult rv = nsComponentManager::ProgIDToCLSID(id, &cid);
    if (NS_FAILED(rv)) {
        return rv;
    }

    // Create an instance of the document-loader-factory object
    nsIDocumentLoaderFactory* factory;
    rv = nsComponentManager::CreateInstance(cid, (nsISupports *)nsnull,
                                            kIDocumentLoaderFactoryIID, 
                                            (void **)&factory);
    if (NS_FAILED(rv)) {
        return rv;
    }

    // Now create an instance of the content viewer
    rv = factory->CreateInstance(aURL, aContentType, aCommand, aContainer,
                                 aExtraInfo, aDocListenerResult,
                                 aDocViewerResult);
    NS_RELEASE(factory);
    return rv;
}


NS_IMETHODIMP
GtkMozillaContainer::ProgressLoadURL(nsIWebShell* aShell, const PRUnichar* aURL, PRInt32 aProgress,
                                     PRInt32 aProgressMax)
{
  printf("Progress: %d (0..%d)\n", aProgress, aProgressMax);
  return NS_OK;
}

NS_IMETHODIMP
GtkMozillaContainer::NewWebShell(PRUint32 aChromeMask, PRBool aVisible, nsIWebShell *&aNewWebShell)
{
  printf("NewWebShell\n");
  aNewWebShell = nsnull;
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
GtkMozillaContainer::FindWebShellWithName(const PRUnichar* aName, nsIWebShell*& aResult)
{
  printf("FindWebShellWithName\n");
  aResult = nsnull;
  if (NS_OK == mWebShell->FindChildWithName(aName, aResult)) {
    if (nsnull != aResult) {
      return NS_OK;
    }
  }
  return NS_OK;
}

NS_IMETHODIMP
GtkMozillaContainer::ContentShellAdded(nsIWebShell* aChildShell, nsIContent* frameNode)
{
  printf("ContentShellAdded\n");
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
GtkMozillaContainer::CreatePopup(nsIDOMElement* aElement, nsIDOMElement* aPopupContent, 
                                 PRInt32 aXPos, PRInt32 aYPos, 
                                 const nsString& aPopupType, 
                                 const nsString& anAnchorAlignment,
                                 const nsString& aPopupAlignment,
                                 nsIDOMWindow* aWindow)
{
  printf("CreatePopup\n");
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
GtkMozillaContainer::CanCreateNewWebShell(PRBool& aResult)
{
  printf("CanCreateNewWebShell\n");
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
GtkMozillaContainer::SetNewWebShellInfo(const nsString& aName, const nsString& anURL, 
                              nsIWebShell* aOpenerShell, PRUint32 aChromeMask,
                              nsIWebShell** aNewShell, nsIWebShell** anInnerShell)
{
  printf("SetNewWebShellInfo\n");
  return NS_ERROR_FAILURE;
}
  
NS_IMETHODIMP
GtkMozillaContainer::ChildShellAdded(nsIWebShell** aChildShell, nsIContent* frameNode)
{
  printf("ChildShellAdded\n");
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
GtkMozillaContainer::FocusAvailable(nsIWebShell* aFocusedWebShell, PRBool& aFocusTaken)
{
  printf("FocusAvailable\n");
  return NS_ERROR_FAILURE;
}

gint
GtkMozillaContainer::StartStream(const char *base_url, const char *action,
                                 const char *content_type)
{
  nsresult rv = NS_OK;
  nsString url_str(base_url);
  nsIURI* url = nsnull;
  nsIContentViewer* viewer = nsnull;
  nsIStreamListener* listener = nsnull;

#ifndef NECKO  
  rv = NS_NewURL(&url, url_str, NULL, mWebShell);
#else
  NS_WITH_SERVICE(nsIIOService, service, kIOServiceCID, &rv);
  if (NS_FAILED(rv)) return rv;

  nsIURI *uri = nsnull;
  const char *uriStr = url_str.GetBuffer();
  rv = service->NewURI(uriStr, nsnull, &uri);
  if (NS_FAILED(rv)) return rv;

  rv = uri->QueryInterface(nsIURI::GetIID(), (void**)&url);
  NS_RELEASE(uri);
#endif // NECKO

  if (NS_FAILED(rv)) {
    goto done;
  }

  rv = CreateContentViewer(url,
                           content_type, 
                           action, 
                           mWebShell,
                           nsnull,
                           &listener, 
                           &viewer);
  
  if (NS_FAILED(rv)) {
    printf("GtkMozillaContainer: Unable to create ContentViewer for action=%s, content-type=%s\n", action, content_type);
    goto done;
  }

  rv = viewer->SetContainer((nsIContentViewerContainer*)mWebShell);
  if (NS_FAILED(rv)) {
    goto done;
  }

  rv = mWebShell->Embed(viewer, action, nsnull);
  if (NS_FAILED(rv)) {
    goto done;
  }

  /*
   * Pass the OnStartBinding(...) notification out to the document 
   * IStreamListener.
   */
  rv = listener->OnStartBinding(url, content_type);
  if (NS_FAILED(rv)) {
    goto done;
  }

  mStream = new GtkMozillaInputStream();
  mStreamURL = url;
  mListener = listener;
  
 done:
  NS_IF_RELEASE(viewer);

  if (NS_SUCCEEDED(rv))
    return 0;
  else
    return -1;
}

gint
GtkMozillaContainer::WriteStream(const char *data, gint len)
{
  nsresult rv = NS_OK;
  PRUint32 Count;
  
  mStream->Fill(data, len);
    
  rv = mListener->OnDataAvailable(mStreamURL, mStream, len);
  if (NS_FAILED(rv))
    return 0;
  
  rv = mListener->OnProgress(mStreamURL, len, len+1);
  if (NS_FAILED(rv))
    return 0;

  mStream->FillResult(&Count);
  
  return (gint) Count;
}

void
GtkMozillaContainer::EndStream(void)
{
  nsresult rv = NS_OK;
  
  mStream->Fill(NULL, 0);
    
  rv = mListener->OnDataAvailable(mStreamURL, mStream, 0);
  if (NS_FAILED(rv))
    return;
  
  rv = mListener->OnStopBinding(mStreamURL, NS_OK, NULL);
  if (NS_FAILED(rv))
    return;
  
  rv = mListener->OnProgress(mStreamURL, 10, 10);
  if (NS_FAILED(rv))
    return;
  
  NS_IF_RELEASE(mStreamURL);
  NS_IF_RELEASE(mListener);
  NS_IF_RELEASE(mStream);
}
