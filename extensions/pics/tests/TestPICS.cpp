#define NS_IMPL_IDS
#include "nsISupports.h"
#include "nsRepository.h"
#include "nsIServiceManager.h"
#include "nsIPICS.h"
#include "nsIPref.h"
#include "nsIParser.h"
#include "nsParserCIID.h"

#ifndef NECKO
#include "nsINetService.h"
#else
#include "nsIIOService.h"
#endif // NECKO

#include "nsIObserverService.h"
#include "nsString.h"
#include "prmem.h"
#include "plstr.h"
#include "nspics.h"

#define PICS_DLL   "pics.dll"
#define PREF_DLL   "xppref32.dll"
#define BASE_DLL   "raptorbase.dll"
#define RAPTORHTMLPARS_DLL "raptorhtmlpars.dll"
#define NETLIB_DLL "netlib.dll"




static NS_DEFINE_IID(kIPICSIID, NS_IPICS_IID);
static NS_DEFINE_IID(kPICSCID, NS_PICS_CID);

static NS_DEFINE_IID(kIPrefIID, NS_IPREF_IID);
static NS_DEFINE_CID(kPrefCID, NS_PREF_CID);

#ifndef NECKO
static NS_DEFINE_IID(kNetServiceCID, NS_NETSERVICE_CID);
#else
static NS_DEFINE_IID(kIOServiceCID, NS_IOSERVICE_CID);
#endif // NECKO


static NS_DEFINE_IID(kCParserIID, NS_IPARSER_IID);
static NS_DEFINE_IID(kCParserCID, NS_PARSER_IID);


PRInt32 main(PRInt32 argc, char *argv[])
{

	nsIPICS *pics = NULL;

  nsresult res;

  nsIObserverService* anObserverService;
  nsString  aTopic("tagobserver");

	
	res = nsRepository::CreateInstance(kPICSCID,
												NULL,
												kIPICSIID,
												(void **) &pics);

	char *fileBuf2;

	char fileBuf[10000] = 
			"(PICS-1.1 \"http://www.gcf.org/v2.5\" labels by \"John Doe\" for \"http://www.w3.org/DSig/DsigProj.html\" ratings (suds 0.5))";

	char fileBuf1[10000] = 
			"(PICS-1.1 \"http://www.gcf.org/v2.5\" l r (suds 0.5))";

  
	fileBuf2 = (char *)PR_Malloc(1000);
	PL_strcpy(fileBuf2, "(PICS-1.1 \"http://www.gcf.org/v2.5\" \n by \"John Doe\" labels for \"http://www.w3.org/DSig/DsigProj.html\" ratings (suds 0.5))");


	char fileBuf3[10000] = 
			"(PICS-1.1 l r (suds 0.5))";

	char fileBuf4[1000] = "(PICS-1.1 \"http://www.classify.org/safesurf/\" l gen true for \"http://www.w3.org\"     by \"philipd@w3.org\" r (ss~~000 1 ss~~100 1))";

	char fileBuf5[1000] = "(PICS-1.1 \"http://www.rsac.org/ratingsv01.html\" l gen true comment    \"RSACi North America Server\" by \"philipd@w3.org\" for \"http://www.w3.org\"     on \"1996.04.16T08:15-0500\" r (n 0 s 0 v 0 l 0))";

	char fileBuf6[1000] = "(PICS-1.1 \"http://www.gcf.org/v2.5\"  l r (suds 0.5 density 0 color/hue 1) r (subject 2 density 1 color/hue 1))";

	char fileBuf7[1000] = "(PICS-1.1 \"http://www.gcf.org/v2.5\" by \"John Doe\" labels on \"1994.11.05T08:15-0500\" until \"1995.12.31T23:59-0000\" for \"http://w3.org/PICS/Overview.html\" ratings (suds 0.5 density 0 color/hue 1) for \"http://w3.org/PICS/Underview.html\" by \"Jane Doe\" ratings (subject 2 density 1 color/hue 1))";
    
  char fileBuf8[1000] = "(PICS-1.1 \"\"http://www.rsac.org/ratingsv01.html\"\" l gen true r (n 0 s 0 v 0 l 0)";
  char fileBuf9[1000] = "(PICS-1.1 \"http://www.rsac.org/ratingsv01.html\" l gen true r (n 0 s 0 v 0 l 0)";
  char fileBuf10[1000] = "\"(PICS-1.1)";

  char fileBuf11[1000] =   "(PICS-1.1 \"http://www.rsac.org/ratingsv01.html\" l gen true comment \"RSACi North America Server\" by \"philipd@w3.org\" for \"http://www.w3.org\" on \"1996.04.16T08:15-0500\" r (n 0 s 0 v 0 l 0))";
	if (res == NS_OK) {
    // Load preferences
    nsComponentManager::RegisterComponent(kPrefCID, NULL, NULL, PREF_DLL, PR_FALSE, PR_FALSE);

#ifndef NECKO
    nsComponentManager::RegisterComponent(kNetServiceCID, NULL, NULL, NETLIB_DLL, PR_FALSE, PR_FALSE);
#else
    nsComponentManager::RegisterComponent(kIOServiceCID, NULL, NULL, NETLIB_DLL, PR_FALSE, PR_FALSE);
#endif // NECKO

    nsComponentManager::RegisterComponent(kCParserCID, NULL, NULL, RAPTORHTMLPARS_DLL, PR_FALSE, PR_FALSE);

    res = nsServiceManager::GetService(NS_OBSERVERSERVICE_PROGID, 
                                nsIObserverService::GetIID(), 
                                (nsISupports **)&anObserverService);
    



    pics->ProcessPICSLabel(fileBuf11);
	}
     
	PR_Free(fileBuf2);
	return 0;
}

