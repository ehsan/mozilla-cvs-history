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
 * Copyright (C) 1999 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

#include "msgCore.h"    // precompiled header...
#include "nsMsgImapCID.h"

#ifdef XP_PC
#include <windows.h>    // for InterlockedIncrement
#endif

#include "nsIEventQueueService.h"

#include "nsIURL.h"
#include "nsImapUrl.h"

#include "nsINetService.h"
#include "nsIMsgMailSession.h"
#include "nsIIMAPHostSessionList.h"
#include "nsIMAPGenericParser.h"
#include "nsString.h"
#include "prmem.h"
#include "plstr.h"
#include "prprf.h"
#include "nsCRT.h"
#include "nsCOMPtr.h"
#include "nsIImapIncomingServer.h"
#include "nsMsgBaseCID.h"

static NS_DEFINE_CID(kMsgMailSessionCID, NS_MSGMAILSESSION_CID);
static NS_DEFINE_CID(kCImapHostSessionListCID, NS_IIMAPHOSTSESSIONLIST_CID);

nsImapUrl::nsImapUrl()
{
	m_server = nsnull;
	
	m_listOfMessageIds = nsnull;
	m_sourceCanonicalFolderPathSubString = nsnull;
	m_destinationCanonicalFolderPathSubString = nsnull;
	m_listOfMessageIds = nsnull;
    m_tokenPlaceHolder = nsnull;
	m_idsAreUids = PR_FALSE;
	m_mimePartSelectorDetected = PR_FALSE;
	m_allowContentChange = PR_TRUE;	// assume we can do MPOD.
	m_validUrl = PR_TRUE;	// assume the best.
	m_flags = 0;
	m_userName = nsnull;
	m_onlineSubDirSeparator = '/'; 
}

nsresult nsImapUrl::Initialize(const char * aUserName)
{
	nsresult rv = NS_OK;
	if (aUserName)
		m_userName = PL_strdup(aUserName);
	else
		rv = NS_ERROR_NULL_POINTER;
	return rv;
}
 
nsImapUrl::~nsImapUrl()
{
	PR_FREEIF(m_listOfMessageIds);
	PR_FREEIF(m_userName);
}
  
NS_IMPL_ISUPPORTS_INHERITED(nsImapUrl, nsMsgMailNewsUrl, nsIImapUrl) 

////////////////////////////////////////////////////////////////////////////////////
// Begin nsIImapUrl specific support
////////////////////////////////////////////////////////////////////////////////////

NS_IMETHODIMP nsImapUrl::GetRequiredImapState(nsImapState * aImapUrlState)
{
	if (aImapUrlState)
	{
		// the imap action determines the state we must be in...check the 
		// the imap action.

		if (m_imapAction & 0x10000000)
			*aImapUrlState = nsImapSelectedState;
		else
			*aImapUrlState = nsImapAuthenticatedState;
	}

	return NS_OK;
}

NS_IMETHODIMP nsImapUrl::GetServer(nsIMsgIncomingServer **aServer)
{
    nsresult rv = NS_ERROR_NULL_POINTER;
	if (aServer) // valid argument to return result in?
	{
		*aServer = m_server;
		NS_IF_ADDREF(*aServer);
		if (m_server)
			rv = NS_OK;  // only return ok if we have non null server...
	} // if aMsgIdentity

	return rv;
}

NS_IMETHODIMP nsImapUrl::GetImapLog(nsIImapLog ** aImapLog)
{
	if (aImapLog)
	{
		*aImapLog = m_imapLog;
		NS_IF_ADDREF(*aImapLog );
	}

	return NS_OK;
}

NS_IMETHODIMP nsImapUrl::SetImapLog(nsIImapLog  * aImapLog)
{
	if (aImapLog)
		m_imapLog = dont_QueryInterface(aImapLog);
	return NS_OK;
}

NS_IMETHODIMP nsImapUrl::GetImapMailFolderSink(nsIImapMailFolderSink **
                                           aImapMailFolderSink)
{
	if (aImapMailFolderSink)
	{
		*aImapMailFolderSink = m_imapMailFolderSink;
		NS_IF_ADDREF(*aImapMailFolderSink);
	}

	return NS_OK;
}

NS_IMETHODIMP nsImapUrl::SetImapMailFolderSink(nsIImapMailFolderSink  * aImapMailFolderSink)
{
	if (aImapMailFolderSink)
		m_imapMailFolderSink = dont_QueryInterface(aImapMailFolderSink);

	return NS_OK;
}
 
NS_IMETHODIMP nsImapUrl::GetImapMessageSink(nsIImapMessageSink ** aImapMessageSink)
{
	if (aImapMessageSink)
	{
		*aImapMessageSink = m_imapMessageSink;
		NS_IF_ADDREF(*aImapMessageSink);
	}

	return NS_OK;
}

NS_IMETHODIMP nsImapUrl::SetImapMessageSink(nsIImapMessageSink  * aImapMessageSink)
{
	if (aImapMessageSink)
		m_imapMessageSink = dont_QueryInterface(aImapMessageSink);

	return NS_OK;
}

NS_IMETHODIMP nsImapUrl::GetImapExtensionSink(nsIImapExtensionSink ** aImapExtensionSink)
{
	if (aImapExtensionSink)
	{
		*aImapExtensionSink = m_imapExtensionSink;
		NS_IF_ADDREF(*aImapExtensionSink);
	}

	return NS_OK;
}

NS_IMETHODIMP nsImapUrl::SetImapExtensionSink(nsIImapExtensionSink  * aImapExtensionSink)
{
	if (aImapExtensionSink)
		m_imapExtensionSink = dont_QueryInterface(aImapExtensionSink);

	return NS_OK;
}

NS_IMETHODIMP nsImapUrl::GetImapMiscellaneousSink(nsIImapMiscellaneousSink **
                                              aImapMiscellaneousSink)
{
	if (aImapMiscellaneousSink)
	{
		*aImapMiscellaneousSink = m_imapMiscellaneousSink;
		NS_IF_ADDREF(*aImapMiscellaneousSink);
	}

	return NS_OK;
}

NS_IMETHODIMP nsImapUrl::SetImapMiscellaneousSink(nsIImapMiscellaneousSink  *
                                              aImapMiscellaneousSink)
{
	if (aImapMiscellaneousSink)
		m_imapMiscellaneousSink = dont_QueryInterface(aImapMiscellaneousSink);

	return NS_OK;
}

        
////////////////////////////////////////////////////////////////////////////////////
// End nsIImapUrl specific support
////////////////////////////////////////////////////////////////////////////////////

nsresult nsImapUrl::ParseUrl(const nsString& aSpec)
{
#ifdef DEBUG_mscott
	// mscott - i just added a new method for intialization, I'm adding a quick
	// check here to verify that initialize was called on this class...this is
	// really for debugging purposes so I can find out if I missed a spot where
	// I needed to initialize the url before using it.
	NS_ASSERTION(m_userName, "oops...looks like we didn't initialize the url.");
#endif

    // XXX hack!
    char* cSpec = aSpec.ToNewCString();

    NS_LOCK_INSTANCE();

    PR_FREEIF(m_protocol);
    PR_FREEIF(m_host);
    PR_FREEIF(m_search);
	PR_FREEIF(m_file);
    m_port = IMAP_PORT;

	// mscott -> eventually we'll replace all of this duplicate host and port parsing code with a url parser
	// class..this should come with N2 Landing...


    // The URL is considered absolute if and only if it begins with a
    // protocol spec. A protocol spec is an alphanumeric string of 1 or
    // more characters that is terminated with a colon.
    PRBool isAbsolute = PR_FALSE;
    char* cp = nsnull;
	char *imapPartOfUrl = nsnull;

    char* ap = cSpec;
    char ch;
    while (0 != (ch = *ap)) 
	{
        if (((ch >= 'a') && (ch <= 'z')) ||
            ((ch >= 'A') && (ch <= 'Z')) ||
            ((ch >= '0') && (ch <= '9'))) 
		{
            ap++;
            continue;
        }
        if ((ch == ':') && (ap - cSpec >= 2)) 
		{
            isAbsolute = PR_TRUE;
            cp = ap;
            break;
        }
        break;
    }

    PRInt32 slen = aSpec.Length();
    m_spec = (char *) PR_Malloc(slen + 1);
    aSpec.ToCString(m_spec, slen+1);

    // get protocol first
    PRInt32 plen = cp - cSpec;
    m_protocol = (char*) PR_Malloc(plen + 1);
    PL_strncpy(m_protocol, cSpec, plen);
    m_protocol[plen] = 0;
    cp++;                               // eat : in protocol
    
	// skip over one, two or three slashes
    if (*cp == '/') 
	{
		cp++;
        if (*cp == '/') 
		{
			cp++;
			if (*cp == '/') 
				cp++;
        }
	} 
	else 
	{
		delete [] cSpec;
		NS_UNLOCK_INSTANCE();
        return NS_ERROR_ILLEGAL_VALUE;
    }

	// Host name follows protocol for http style urls
	const char* cp0 = cp;
    cp = PL_strchr(cp0, '@');
    
    if (cp) {
        // we have a username between cp0 and cp
        m_userName = PL_strndup(cp0, (cp - cp0));
        cp0 = cp+1;
    }
    
	cp = PL_strpbrk(cp0, "/:");
	if (nsnull == cp) 
	{
		// There is only a host name
		PRInt32 hlen = PL_strlen(cp0);
        m_host = (char*) PR_Malloc(hlen + 1);
        PL_strcpy(m_host, cp0);
	}
    else {
		PRInt32 hlen = cp - cp0;
        m_host = (char*) PR_Malloc(hlen + 1);
        PL_strncpy(m_host, cp0, hlen);        
        m_host[hlen] = 0;

		if (':' == *cp) 
		{
			// We have a port number
            cp0 = cp+1;
            cp = PL_strchr(cp, '/');
            m_port = strtol(cp0, (char **)nsnull, 10 /* base 10 */);
        }
		imapPartOfUrl = cp + 1; // #### probably not quite right - should check for "/"??
        cp = PL_strchr(cp, '?');
        if (cp)
        {
            cp++;
            PRInt32 cplen = PL_strlen(cp);
            m_search = (char*) PR_Malloc(cplen+1);
            PL_strcpy(m_search, cp);
        }
	}

	if (imapPartOfUrl)
		m_file = PL_strdup(imapPartOfUrl);

	ParseImapPart(imapPartOfUrl);

    delete [] cSpec;

    if (m_host)
    {
        nsresult rv = NS_OK;
        NS_WITH_SERVICE(nsIMsgMailSession, session, kMsgMailSessionCID, &rv); 
        if (NS_FAILED(rv)) return rv;
        
        nsCOMPtr<nsIMsgAccountManager> accountManager;
        rv = session->GetAccountManager(getter_AddRefs(accountManager));
        if(NS_FAILED(rv)) return rv;
        
        nsCOMPtr<nsIMsgIncomingServer> server;
        rv = accountManager->FindServer(m_userName,
                                        m_host,
                                        "imap",
                                        getter_AddRefs(server));
        if (NS_FAILED(rv)) return rv;
        // can't do an addref because it's private to nsIURI, so use
        // do_QueryInterface instead
		m_server = do_QueryInterface(server);
    }

    NS_UNLOCK_INSTANCE();
    return NS_OK;
}

void nsImapUrl::ReconstructSpec(void)
{
    PR_FREEIF(m_spec);

    char portBuffer[10];
    if (0 != m_port)
        PR_snprintf(portBuffer, 10, ":%d", m_port);
    else
        portBuffer[0] = '\0';

    PRInt32 plen = PL_strlen(m_protocol) + PL_strlen(m_host) +
        PL_strlen(portBuffer) + 4;

	if (m_file)
		plen += 1 + PL_strlen(m_file);

    if (m_search)
        plen += 1 + PL_strlen(m_search);

    m_spec = (char *) PR_Malloc(plen + 1);
    PR_snprintf(m_spec, plen, "%s://%s%s", 
                m_protocol, ((nsnull != m_host) ? m_host : ""), portBuffer);

	if (m_file)
	{
		PL_strcat(m_spec, "/");
		PL_strcat(m_spec, m_file);
	}
    if (m_search) 
	{
        PL_strcat(m_spec, "?");
        PL_strcat(m_spec, m_search);
    }
}



NS_IMETHODIMP nsImapUrl::CreateSearchCriteriaString(nsString2 *aResult)
{
	if (nsnull == aResult || !m_searchCriteriaString) 
		return  NS_ERROR_NULL_POINTER;

    NS_LOCK_INSTANCE();
	aResult->Assign(m_searchCriteriaString);
    NS_UNLOCK_INSTANCE();
	return NS_OK;
}


NS_IMETHODIMP nsImapUrl::CreateListOfMessageIdsString(nsString2 *aResult) 
{
	if (nsnull == aResult || !m_listOfMessageIds) 
		return  NS_ERROR_NULL_POINTER;

	PRInt32 bytesToCopy = PL_strlen(m_listOfMessageIds);
    NS_LOCK_INSTANCE();

	// mime may have glommed a "&part=" for a part download
	// we return the entire message and let mime extract
	// the part. Pop and news work this way also.
	// this algorithm truncates the "&part" string.
	char *currentChar = m_listOfMessageIds;
	while (*currentChar && (*currentChar != '&'))
		currentChar++;
	if (*currentChar == '&')
		bytesToCopy = currentChar - m_listOfMessageIds;

	// we should also strip off anything after "/;section="
	// since that can specify an IMAP MIME part
	char *wherePart = PL_strstr(m_listOfMessageIds, "/;section=");
	if (wherePart)
		bytesToCopy = MIN(bytesToCopy, wherePart - m_listOfMessageIds);

	aResult->Assign(m_listOfMessageIds, bytesToCopy);

    NS_UNLOCK_INSTANCE();
	return NS_OK;
}
  
NS_IMETHODIMP nsImapUrl::GetImapPartToFetch(char **result) 
{
    NS_LOCK_INSTANCE();
	//  here's the old code:
#if 0
	char *wherepart = NULL, *rv = NULL;
	if (m_listOfMessageIds && (wherepart = PL_strstr(m_listOfMessageIds, "/;section=")) != NULL)
	{
		wherepart += 10; // XP_STRLEN("/;section=")
		if (wherepart)
		{
			char *wherelibmimepart = XP_STRSTR(wherepart, "&part=");
			int len = PL_strlen(m_listOfMessageIds), numCharsToCopy = 0;
			if (wherelibmimepart)
				numCharsToCopy = (wherelibmimepart - wherepart);
			else
				numCharsToCopy = PL_strlen(m_listOfMessageIds) - (wherepart - m_listOfMessageIds);
			if (numCharsToCopy)
			{
				rv = (char *) PR_Malloc(sizeof(char) * (numCharsToCopy + 1));
				if (rv)
				{
					XP_STRNCPY_SAFE(rv, wherepart, numCharsToCopy + 1);	// appends a \0
				}
			}
		}
	}
#endif // 0 
    NS_UNLOCK_INSTANCE();
    return NS_OK;

}

NS_IMETHODIMP nsImapUrl::GetOnlineSubDirSeparator(char* separator)
{
    if (separator)
    {
        *separator = m_onlineSubDirSeparator;
        return NS_OK;
    }
    else
    {
        return NS_ERROR_NULL_POINTER;
    }
}

NS_IMETHODIMP
nsImapUrl::SetOnlineSubDirSeparator(char onlineDirSeparator)
{
	m_onlineSubDirSeparator = onlineDirSeparator;
    return NS_OK;
}


NS_IMETHODIMP nsImapUrl::MessageIdsAreUids(PRBool *result)
{
    NS_LOCK_INSTANCE();
	*result = m_idsAreUids;
    NS_UNLOCK_INSTANCE();
    return NS_OK;
}

NS_IMETHODIMP 
nsImapUrl::GetChildDiscoveryDepth(PRInt32* result)
{
    NS_LOCK_INSTANCE();
    *result = m_discoveryDepth;
    NS_UNLOCK_INSTANCE();
    return NS_OK;
}

NS_IMETHODIMP nsImapUrl::GetMsgFlags(imapMessageFlagsType *result)	// kAddMsgFlags or kSubtractMsgFlags only
{
    NS_LOCK_INSTANCE();
	*result = m_flags;
    NS_UNLOCK_INSTANCE();
    return NS_OK;
}

void nsImapUrl::ParseImapPart(char *imapPartOfUrl)
{
	m_tokenPlaceHolder = imapPartOfUrl;
	m_urlidSubString = m_tokenPlaceHolder ? nsIMAPGenericParser::Imapstrtok_r(nil, IMAP_URL_TOKEN_SEPARATOR, &m_tokenPlaceHolder) : (char *)NULL;
	if (!m_urlidSubString)
	{
		m_validUrl = FALSE;
		return;
	}
	
	if (!PL_strcasecmp(m_urlidSubString, "fetch"))
	{
		m_imapAction   					 = nsImapMsgFetch;
		ParseUidChoice();
		ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
		ParseListofMessageIds();
	}
	else /* if (fInternal) no concept of internal - not sure there will be one */
	{
		if (!PL_strcasecmp(m_urlidSubString, "header"))
		{
			m_imapAction   					 = nsImapMsgHeader;
			ParseUidChoice();
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
			ParseListofMessageIds();
		}
		else if (!PL_strcasecmp(m_urlidSubString, "deletemsg"))
		{
			m_imapAction   					 = nsImapDeleteMsg;
			ParseUidChoice();
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
			ParseListofMessageIds();
		}
		else if (!PL_strcasecmp(m_urlidSubString, "uidexpunge"))
		{
			m_imapAction   					 = nsImapUidExpunge;
			ParseUidChoice();
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
			ParseListofMessageIds();
		}
		else if (!PL_strcasecmp(m_urlidSubString, "deleteallmsgs"))
		{
			m_imapAction   					 = nsImapDeleteAllMsgs;
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
		}
		else if (!PL_strcasecmp(m_urlidSubString, "addmsgflags"))
		{
			m_imapAction   					 = nsImapAddMsgFlags;
			ParseUidChoice();
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
			ParseListofMessageIds();
			ParseMsgFlags();
		}
		else if (!PL_strcasecmp(m_urlidSubString, "subtractmsgflags"))
		{
			m_imapAction   					 = nsImapSubtractMsgFlags;
			ParseUidChoice();
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
			ParseListofMessageIds();
			ParseMsgFlags();
		}
		else if (!PL_strcasecmp(m_urlidSubString, "setmsgflags"))
		{
			m_imapAction   					 = nsImapSetMsgFlags;
			ParseUidChoice();
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
			ParseListofMessageIds();
			ParseMsgFlags();
		}
		else if (!PL_strcasecmp(m_urlidSubString, "onlinecopy"))
		{
			m_imapAction   					 = nsImapOnlineCopy;
			ParseUidChoice();
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
			ParseListofMessageIds();
			ParseFolderPath(&m_destinationCanonicalFolderPathSubString);
		}
		else if (!PL_strcasecmp(m_urlidSubString, "onlinemove"))
		{
			m_imapAction   					 = nsImapOnlineMove;
			ParseUidChoice();
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
			ParseListofMessageIds();
			ParseFolderPath(&m_destinationCanonicalFolderPathSubString);
		}
		else if (!PL_strcasecmp(m_urlidSubString, "onlinetoofflinecopy"))
		{
			m_imapAction   					 = nsImapOnlineToOfflineCopy;
			ParseUidChoice();
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
			ParseListofMessageIds();
			ParseFolderPath(&m_destinationCanonicalFolderPathSubString);
		}
		else if (!PL_strcasecmp(m_urlidSubString, "onlinetoofflinemove"))
		{
			m_imapAction   					 = nsImapOnlineToOfflineMove;
			ParseUidChoice();
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
			ParseListofMessageIds();
			ParseFolderPath(&m_destinationCanonicalFolderPathSubString);
		}
		else if (!PL_strcasecmp(m_urlidSubString, "offlinetoonlinecopy"))
		{
			m_imapAction   					 = nsImapOfflineToOnlineMove;
			ParseFolderPath(&m_destinationCanonicalFolderPathSubString);
		}
		else if (!PL_strcasecmp(m_urlidSubString, "search"))
		{
			m_imapAction   					 = nsImapSearch;
			ParseUidChoice();
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
			ParseSearchCriteriaString();
		}
		else if (!PL_strcasecmp(m_urlidSubString, "test"))
		{
			m_imapAction   					 = nsImapTest;
		}
		else if (!PL_strcasecmp(m_urlidSubString, "select"))
		{
			m_imapAction   					 = nsImapSelectFolder;
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
			if (m_tokenPlaceHolder && *m_tokenPlaceHolder)
				ParseListofMessageIds();
			else
				m_listOfMessageIds = PL_strdup("");
		}
		else if (!PL_strcasecmp(m_urlidSubString, "liteselect"))
		{
			m_imapAction   					 = nsImapLiteSelectFolder;
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
		}
		else if (!PL_strcasecmp(m_urlidSubString, "selectnoop"))
		{
			m_imapAction   					 = nsImapSelectNoopFolder;
			m_listOfMessageIds = PL_strdup("");
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
		}
		else if (!PL_strcasecmp(m_urlidSubString, "expunge"))
		{
			m_imapAction   					 = nsImapExpungeFolder;
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
			m_listOfMessageIds = PL_strdup("");		// no ids to UNDO
		}
		else if (!PL_strcasecmp(m_urlidSubString, "create"))
		{
			m_imapAction   					 = nsImapCreateFolder;
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
		}
		else if (!PL_strcasecmp(m_urlidSubString, "discoverchildren"))
		{
			m_imapAction   					 = nsImapDiscoverChildrenUrl;
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
		}
		else if (!PL_strcasecmp(m_urlidSubString, "discoverlevelchildren"))
		{
			m_imapAction   					 = nsImapDiscoverLevelChildrenUrl;
			ParseChildDiscoveryDepth();
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
		}
		else if (!PL_strcasecmp(m_urlidSubString, "discoverallboxes"))
		{
			m_imapAction   					 = nsImapDiscoverAllBoxesUrl;
		}
		else if (!PL_strcasecmp(m_urlidSubString, "discoverallandsubscribedboxes"))
		{
			m_imapAction   					 = nsImapDiscoverAllAndSubscribedBoxesUrl;
		}
		else if (!PL_strcasecmp(m_urlidSubString, "delete"))
		{
			m_imapAction   					 = nsImapDeleteFolder;
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
		}
		else if (!PL_strcasecmp(m_urlidSubString, "rename"))
		{
			m_imapAction   					 = nsImapRenameFolder;
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
			ParseFolderPath(&m_destinationCanonicalFolderPathSubString);
		}
		else if (!PL_strcasecmp(m_urlidSubString, "movefolderhierarchy"))
		{
			m_imapAction   					 = nsImapMoveFolderHierarchy;
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
			if (m_tokenPlaceHolder && *m_tokenPlaceHolder)	// handle promote to root
				ParseFolderPath(&m_destinationCanonicalFolderPathSubString);
		}
		else if (!PL_strcasecmp(m_urlidSubString, "list"))
		{
			m_imapAction   					 = nsImapLsubFolders;
			ParseFolderPath(&m_destinationCanonicalFolderPathSubString);
		}
		else if (!PL_strcasecmp(m_urlidSubString, "biff"))
		{
			m_imapAction   					 = nsImapBiff;
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
			ParseListofMessageIds();
		}
		else if (!PL_strcasecmp(m_urlidSubString, "netscape"))
		{
			m_imapAction   					 = nsImapGetMailAccountUrl;
		}
		else if (!PL_strcasecmp(m_urlidSubString, "appendmsgfromfile"))
		{
			m_imapAction					 = nsImapAppendMsgFromFile;
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
		}
		else if (!PL_strcasecmp(m_urlidSubString, "appenddraftfromfile"))
		{
			m_imapAction					 = nsImapAppendMsgFromFile;
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
		}
		else if (!PL_strcasecmp(m_urlidSubString, "subscribe"))
		{
			m_imapAction					 = nsImapSubscribe;
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
		}
		else if (!PL_strcasecmp(m_urlidSubString, "unsubscribe"))
		{
			m_imapAction					 = nsImapUnsubscribe;
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
		}
		else if (!PL_strcasecmp(m_urlidSubString, "refreshacl"))
		{
			m_imapAction					= nsImapRefreshACL;
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
		}
		else if (!PL_strcasecmp(m_urlidSubString, "refreshfolderurls"))
		{
			m_imapAction					= nsImapRefreshFolderUrls;
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
		}
		else if (!PL_strcasecmp(m_urlidSubString, "refreshallacls"))
		{
			m_imapAction					= nsImapRefreshAllACLs;
		}
		else if (!PL_strcasecmp(m_urlidSubString, "listfolder"))
		{
			m_imapAction					= nsImapListFolder;
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
		}
		else if (!PL_strcasecmp(m_urlidSubString, "upgradetosubscription"))
		{
			m_imapAction					= nsImapUpgradeToSubscription;
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
		}
		else if (!PL_strcasecmp(m_urlidSubString, "folderstatus"))
		{
			m_imapAction					= nsImapFolderStatus;
			ParseFolderPath(&m_sourceCanonicalFolderPathSubString);
		}
		else
		{
			m_validUrl = PR_FALSE;	
		}
	}
}


// Returns NULL if nothing was done.
// Otherwise, returns a newly allocated name.
NS_IMETHODIMP nsImapUrl::AddOnlineDirectoryIfNecessary(const char *onlineMailboxName, char ** directory)
{
	nsresult result = NS_OK;
	char *rv = NULL;
#ifdef HAVE_PORT
	// If this host has an online server directory configured
	char *onlineDir = TIMAPHostInfo::GetOnlineDirForHost(GetUrlHost());
	if (onlineMailboxName && onlineDir)
	{
#ifdef DEBUG
		// This invariant should be maintained by libmsg when reading/writing the prefs.
		// We are only supporting online directories whose online delimiter is /
		// Therefore, the online directory must end in a slash.
		XP_ASSERT(onlineDir[XP_STRLEN(onlineDir) - 1] == '/');
#endif
		TIMAPNamespace *ns = TIMAPHostInfo::GetNamespaceForMailboxForHost(GetUrlHost(), onlineMailboxName);
		NS_ASSERTION(ns, "couldn't find namespace for host");
		if (ns && (PL_strlen(ns->GetPrefix()) == 0) && PL_strcasecmp(onlineMailboxName, "INBOX"))
		{
			// Also make sure that the first character in the mailbox name is not '/'.
			NS_ASSERTION(*onlineMailboxName != '/', "first char of onlinemailbox is //");

			// The namespace for this mailbox is the root ("").
			// Prepend the online server directory
			int finalLen = XP_STRLEN(onlineDir) + XP_STRLEN(onlineMailboxName) + 1;
			rv = (char *)XP_ALLOC(finalLen);
			if (rv)
			{
				XP_STRCPY(rv, onlineDir);
				XP_STRCAT(rv, onlineMailboxName);
			}
		}
	}
#endif // HAVE_PORT
	if (directory)
		*directory = rv;
	else
		PR_FREEIF(rv);
	return result;
}

// Converts from canonical format (hierarchy is indicated by '/' and all real slashes ('/') are escaped)
// to the real online name on the server.
NS_IMETHODIMP nsImapUrl::AllocateServerPath(const char * canonicalPath, char onlineDelimiter, char ** aAllocatedPath)
{
	nsresult retVal = NS_OK;
	char *rv = NULL;
	char delimiterToUse = onlineDelimiter;
	if (onlineDelimiter == kOnlineHierarchySeparatorUnknown)
		GetOnlineSubDirSeparator(&delimiterToUse);
	NS_ASSERTION(delimiterToUse != kOnlineHierarchySeparatorUnknown, "hierarchy separator unknown");
	if (canonicalPath)
		rv = ReplaceCharsInCopiedString(canonicalPath, '/', delimiterToUse);
	else
		rv = PL_strdup("");

	char *onlineNameAdded = nsnull;
	AddOnlineDirectoryIfNecessary(rv, &onlineNameAdded);
	if (onlineNameAdded)
	{
		PR_FREEIF(rv);
		rv = onlineNameAdded;
	}

	if (aAllocatedPath)
		*aAllocatedPath = rv;
	else
		PR_FREEIF(rv);

	return retVal;
}

// Converts the real online name on the server to canonical format:
// result is hierarchy is indicated by '/' and all real slashes ('/') are escaped.
// The caller has already converted m-utf-7 to 8 bit ascii, which is a problem.
NS_IMETHODIMP nsImapUrl::AllocateCanonicalPath(const char *serverPath, char onlineDelimiter, char **allocatedPath ) 
{
    nsresult rv = NS_ERROR_NULL_POINTER;
    char *canonicalPath = nsnull;
	char delimiterToUse = onlineDelimiter;
    const char* hostName = nsnull;
    char* userName = nsnull;
    nsString aString;
	char *currentPath = (char *) serverPath;
    char *onlineDir = nsnull;

	NS_LOCK_INSTANCE();

    NS_WITH_SERVICE(nsIImapHostSessionList, hostSessionList,
                    kCImapHostSessionListCID, &rv);    

    *allocatedPath = nsnull;

	if (onlineDelimiter == kOnlineHierarchySeparatorUnknown ||
		onlineDelimiter == 0)
		GetOnlineSubDirSeparator(&delimiterToUse);

	NS_ASSERTION (serverPath, "Oops... null serverPath");

	if (!serverPath || NS_FAILED(rv))
		goto done;

    GetHost(&hostName);
    m_server->GetUsername(&userName);

    hostSessionList->GetOnlineDirForHost(hostName, userName, aString); 
    // First we have to check to see if we should strip off an online server
    // subdirectory 
	// If this host has an online server directory configured
	onlineDir = aString.Length() > 0? aString.ToNewCString(): nsnull;

	if (currentPath && onlineDir)
	{
#ifdef DEBUG
		// This invariant should be maintained by libmsg when reading/writing the prefs.
		// We are only supporting online directories whose online delimiter is /
		// Therefore, the online directory must end in a slash.
		NS_ASSERTION (onlineDir[PL_strlen(onlineDir) - 1] == '/', 
                      "Oops... online dir not end in a slash");
#endif

		// By definition, the online dir must be at the root.
		int len = PL_strlen(onlineDir);
		if (!PL_strncmp(onlineDir, currentPath, len))
		{
			// This online path begins with the server sub directory
			currentPath += len;

			// This might occur, but it's most likely something not good.
			// Basically, it means we're doing something on the online sub directory itself.
			NS_ASSERTION (*currentPath, "Oops ... null currentPath");
			// Also make sure that the first character in the mailbox name is not '/'.
			NS_ASSERTION (*currentPath != '/', 
                          "Oops ... currentPath starts with a slash");
		}
	}

	if (!currentPath)
		goto done;

	// Now, start the conversion to canonical form.
	canonicalPath = ReplaceCharsInCopiedString(currentPath, delimiterToUse ,
                                               '/');
	
	// eat any escape characters for escaped dir separators
	if (canonicalPath)
	{
		char *currentEscapeSequence = PL_strstr(canonicalPath, "\\/");
		while (currentEscapeSequence)
		{
			PL_strcpy(currentEscapeSequence, currentEscapeSequence+1);
			currentEscapeSequence = PL_strstr(currentEscapeSequence+1, "\\/");
		}
        *allocatedPath = canonicalPath;
	}

done:
    PR_FREEIF(userName);
    PR_FREEIF(onlineDir);

    NS_UNLOCK_INSTANCE();
    return rv;
}

NS_IMETHODIMP  nsImapUrl::CreateServerSourceFolderPathString(char **result)
{
	if (!result)
	    return NS_ERROR_NULL_POINTER;
	NS_LOCK_INSTANCE();
	AllocateServerPath(m_sourceCanonicalFolderPathSubString, kOnlineHierarchySeparatorUnknown, result);

    NS_UNLOCK_INSTANCE();
    return NS_OK;
}

NS_IMETHODIMP nsImapUrl::CreateCanonicalSourceFolderPathString(char **result)
{
	if (!result)
	    return NS_ERROR_NULL_POINTER;
	NS_LOCK_INSTANCE();
	*result = PL_strdup(m_sourceCanonicalFolderPathSubString ? m_sourceCanonicalFolderPathSubString : "");
    NS_UNLOCK_INSTANCE();
	return (*result) ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
}

NS_IMETHODIMP nsImapUrl::CreateServerDestinationFolderPathString(char **result)
{
	nsresult rv = NS_OK;
	if (!result)
	    return NS_ERROR_NULL_POINTER;
	NS_LOCK_INSTANCE();
	// its possible for the destination folder path to be the root
	if (!m_destinationCanonicalFolderPathSubString)
		*result = PL_strdup("");
	else
		rv = AllocateServerPath(m_destinationCanonicalFolderPathSubString, kOnlineHierarchySeparatorUnknown, result);
    NS_UNLOCK_INSTANCE();
	return (*result) ? rv : NS_ERROR_OUT_OF_MEMORY;
}


// for enabling or disabling mime parts on demand. Setting this to TRUE says we
// can use mime parts on demand, if we chose.
NS_IMETHODIMP nsImapUrl::SetAllowContentChange(PRBool allowContentChange)
{
	NS_LOCK_INSTANCE();
	m_allowContentChange = allowContentChange;
    NS_UNLOCK_INSTANCE();
	return NS_OK;
}

NS_IMETHODIMP nsImapUrl::GetAllowContentChange(PRBool *result)
{
	if (!result)
	    return NS_ERROR_NULL_POINTER;
	NS_LOCK_INSTANCE();
	*result = m_allowContentChange;
    NS_UNLOCK_INSTANCE();
	return NS_OK;
}


char *nsImapUrl::ReplaceCharsInCopiedString(const char *stringToCopy, char oldChar, char newChar)
{	
	char oldCharString[2];
	*oldCharString = oldChar;
	*(oldCharString+1) = 0;
	
	char *translatedString = PL_strdup(stringToCopy);
	char *currentSeparator = PL_strstr(translatedString, oldCharString);
	
	while(currentSeparator)
	{
		*currentSeparator = newChar;
		currentSeparator = PL_strstr(currentSeparator+1, oldCharString);
	}

	return translatedString;
}


////////////////////////////////////////////////////////////////////////////////////
// End of functions which should be made obsolete after modifying nsIURI
////////////////////////////////////////////////////////////////////////////////////

void nsImapUrl::ParseFolderPath(char **resultingCanonicalPath)
{
	char *resultPath = m_tokenPlaceHolder ? nsIMAPGenericParser::Imapstrtok_r(nil, IMAP_URL_TOKEN_SEPARATOR, &m_tokenPlaceHolder) : (char *)NULL;
	
	if (!resultPath)
	{
		m_validUrl = PR_FALSE;
		return;
	}

	*resultingCanonicalPath = PL_strdup(resultPath);
	// The delimiter will be set for a given URL, but will not be statically available
	// from an arbitrary URL.  It is the creator's responsibility to fill in the correct
	// delimiter from the folder's namespace when creating the URL.
	char dirSeparator = *(*resultingCanonicalPath)++;
	if (dirSeparator != kOnlineHierarchySeparatorUnknown)
		SetOnlineSubDirSeparator( dirSeparator);
	
	// if dirSeparator == kOnlineHierarchySeparatorUnknown, then this must be a create
	// of a top level imap box.  If there is an online subdir, we will automatically
	// use its separator.  If there is not an online subdir, we don't need a separator.
	
}


void nsImapUrl::ParseSearchCriteriaString()
{
	m_searchCriteriaString = m_tokenPlaceHolder ? nsIMAPGenericParser::Imapstrtok_r(nil, IMAP_URL_TOKEN_SEPARATOR, &m_tokenPlaceHolder) : (char *)NULL;
	if (!m_searchCriteriaString)
		m_validUrl = FALSE;
}


void nsImapUrl::ParseChildDiscoveryDepth()
{
	char *discoveryDepth = m_tokenPlaceHolder ? nsIMAPGenericParser::Imapstrtok_r(nil, IMAP_URL_TOKEN_SEPARATOR, &m_tokenPlaceHolder) : (char *)NULL;
	if (!discoveryDepth)
	{
		m_validUrl = PR_FALSE;
		m_discoveryDepth = 0;
		return;
	}
	m_discoveryDepth = atoi(discoveryDepth);
}

void nsImapUrl::ParseUidChoice()
{
	char *uidChoiceString = m_tokenPlaceHolder ? nsIMAPGenericParser::Imapstrtok_r(nil, IMAP_URL_TOKEN_SEPARATOR, &m_tokenPlaceHolder) : (char *)NULL;
	if (!uidChoiceString)
		m_validUrl = FALSE;
	else
		m_idsAreUids = PL_strcmp(uidChoiceString, "UID") == 0;
}

void nsImapUrl::ParseMsgFlags()
{
	char *flagsPtr = m_tokenPlaceHolder ? nsIMAPGenericParser::Imapstrtok_r(nil, IMAP_URL_TOKEN_SEPARATOR, &m_tokenPlaceHolder) : (char *)NULL;
	if (flagsPtr)
	{
		// the url is encodes the flags byte as ascii 
		int intFlags = atoi(flagsPtr);
		m_flags = (imapMessageFlagsType) intFlags;	// cast here 
	}
	else
		m_flags = 0;
}

void nsImapUrl::ParseListofMessageIds()
{
	m_listOfMessageIds = m_tokenPlaceHolder ? nsIMAPGenericParser::Imapstrtok_r(nil, IMAP_URL_TOKEN_SEPARATOR, &m_tokenPlaceHolder) : (char *)NULL;
	if (!m_listOfMessageIds)
		m_validUrl = PR_FALSE;
	else
	{
		m_listOfMessageIds = PL_strdup(m_listOfMessageIds);
		m_mimePartSelectorDetected = PL_strstr(m_listOfMessageIds, "&part=") != 0;
	}
}

