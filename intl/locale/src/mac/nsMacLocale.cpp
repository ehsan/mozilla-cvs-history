/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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

#include "nsISupports.h"
#include "nscore.h"
#include "nsString.h"
#include "nsILocale.h"
#include "nsMacLocale.h"
#include "nsLocaleCID.h"
#include "prprf.h"
#include <script.h>

NS_DEFINE_IID(kIMacLocaleIID, NS_IMACLOCALE_IID);
NS_DEFINE_IID(kMacLocaleCID, NS_MACLOCALE_CID);

struct iso_map
{
	char*	iso_code;
	short	mac_lang_code;
	short	mac_script_code;

};
typedef struct iso_map iso_map;

iso_map lang_list[] = {
	{ "sq", langAlbanian, smRoman },
	{ "am", langAmharic, smEthiopic	},
	{ "ar", langArabic, smArabic },
	{ "hy", langArmenian, smArmenian},
	{ "as", langAssamese, smBengali },
	{ "ay", langAymara, smRoman},
	{ "eu", langBasque, smRoman},
	{ "bn", langBengali, smBengali },
	{ "dz", langDzongkha, smTibetan },
	{ "br", langBreton, smRoman },
	{ "bg", langBulgarian, smCyrillic },
	{ "my", langBurmese, smBurmese },
	{ "km", langKhmer, smKhmer },
	{ "ca", langCatalan, smRoman },
	{ "zh", langTradChinese, smTradChinese },
	{ "hr", langCroatian, smRoman },
	{ "cs", langCzech, smCentralEuroRoman },
	{ "da", langDanish, smRoman },
	{ "nl", langDutch, smRoman },
	{ "en", langEnglish, smRoman },
	{ "eo", langEsperanto, smRoman },
	{ "et", langEstonian, smCentralEuroRoman},
	{ "fo", langFaeroese, smRoman },
	{ "fa", langFarsi, smArabic },
	{ "fi", langFinnish, smRoman },
	{ "fr", langFrench, smRoman },
	{ "ka", langGeorgian, smGeorgian },
	{ "de", langGerman, smRoman },
	{ "el", langGreek, smGreek },
	{ "gn", langGuarani, smRoman },
	{ "gu", langGujarati, smGujarati },
	{ "he", langHebrew, smHebrew },
	{ "iw", langHebrew, smHebrew },
	{ "hu", langHungarian, smCentralEuroRoman }, 
	{ "is", langIcelandic, smRoman },
	{ "in", langIndonesian, smRoman },
	{ "id", langIndonesian,  smRoman },
	{ "iu", langInuktitut, smEthiopic },
	{ "ga", langIrish, smRoman }, 
	{ "it", langItalian, smRoman },
	{ "ja", langJapanese, smJapanese },
	{ "jw", langJavaneseRom, smRoman },
	{ "kn", langKannada, smKannada },
	{ "ks", langKashmiri, smArabic },
	{ "kk", langKazakh, smCyrillic },
	{ "ky", langKirghiz, smCyrillic },
	{ "ko", langKorean, smKorean },
	{ "ku", langKurdish, smArabic },
	{ "lo", langLao, smLao },
	{ "la", langLatin, smRoman },
	{ "lv", langLatvian, smCentralEuroRoman },
	{ "lt", langLithuanian, smCentralEuroRoman },
	{ "mk", langMacedonian, smCyrillic },
	{ "mg", langMalagasy, smRoman },
	{ "ml", langMalayalam, smMalayalam },
	{ "mt", langMaltese, smRoman },
	{ "mr", langMarathi, smDevanagari },
	{ "mo", langMoldavian, smCyrillic },
	{ "ne", langNepali, smDevanagari },
	{ "no", langNorwegian, smRoman },
	{ "or", langOriya, smOriya },
	{ "om", langOromo, smEthiopic },
	{ "ps", langPashto, smArabic },
	{ "pl", langPolish, smCentralEuroRoman },
	{ "pt", langPortuguese, smRoman },
	{ "pa", langPunjabi, smGurmukhi },
	{ "ro", langRomanian, smRoman },
	{ "ru", langRussian, smCyrillic },
	{ "sa", langSanskrit, smDevanagari },
	{ "sr", langSerbian, smCyrillic },
	{ "sd", langSindhi, smArabic },
	{ "si", langSinhalese, smSinhalese },
	{ "sk", langSlovak, smCentralEuroRoman },
	{ "sl", langSlovenian, smRoman },
	{ "so", langSomali, smRoman },
	{ "es", langSpanish, smRoman },
	{ "su", langSundaneseRom, smRoman },
	{ "sw", langSwahili, smRoman },
	{ "sv", langSwedish, smRoman }, 
	{ "tl", langTagalog, smRoman },
	{ "tg", langTajiki, smCyrillic },
	{ "ta", langTamil, smTamil },
	{ "tt", langTatar, smCyrillic },
	{ "te", langTelugu, smTelugu },
	{ "th", langThai, smThai },
	{ "bo", langTibetan, smTibetan },
	{ "ti", langTigrinya, smEthiopic },
	{ "tr", langTurkish, smRoman },
	{ "tk", langTurkmen, smCyrillic },
	{ "ug", langUighur, smCyrillic },
	{ "uk", langUkrainian, smCyrillic },
	{ "ur", langUrdu, smArabic },
	{ "uz", langUzbek, smCyrillic },
	{ "vi", langVietnamese, smVietnamese },
	{ "cy", langWelsh, smRoman },
	{ "ji", langYiddish, smHebrew },
	{ "yi", langYiddish, smHebrew },
	{ "", 0, 0}
};

	

/* nsMacLocale ISupports */
NS_IMPL_ISUPPORTS(nsMacLocale,kIMacLocaleIID)

nsMacLocale::nsMacLocale(void)
{
  NS_INIT_REFCNT();
}

nsMacLocale::~nsMacLocale(void)
{

}

NS_IMETHODIMP 
nsMacLocale::GetPlatformLocale(const nsString* locale,short* scriptCode, short* langCode)
{
  	char  country_code[3];
  	char  lang_code[3];
  	char  region_code[3];
	char* xp_locale = locale->ToNewCString();
	int	i;
	
	if (xp_locale != nsnull) {
    	if (!ParseLocaleString(xp_locale,lang_code,country_code,region_code,'-')) {
      		*scriptCode = smRoman;
      		*langCode = langEnglish;
      		delete [] xp_locale;
      		return NS_ERROR_FAILURE;
   		}
	
		for(i=0;strlen(lang_list[i].iso_code)!=0;i++) {
			if (strcmp(lang_list[i].iso_code,lang_code)==0) {
				*scriptCode = lang_list[i].mac_script_code;
				*langCode = lang_list[i].mac_lang_code;
				delete [] xp_locale;
				return NS_OK;
			}
		}
	}
	
	return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
nsMacLocale::GetXPLocale(short scriptCode, short langCode, nsString* locale)
{

	int i;
	nsString* temp;
	
	for(i=0;strlen(lang_list[i].iso_code)!=0;i++) {
		if (langCode==lang_list[i].mac_lang_code) {
			temp = new nsString(lang_list[i].iso_code);
			*locale = *temp;
			delete temp;
			return NS_OK;
		}
	}
	return NS_ERROR_FAILURE;

}

//
// returns PR_FALSE/PR_TRUE depending on if it was of the form LL-CC-RR
PRBool
nsMacLocale::ParseLocaleString(const char* locale_string, char* language, char* country, char* region, char separator)
{
	size_t		len;

	len = strlen(locale_string);
	if (len==0 || (len!=2 && len!=5 && len!=8))
		return PR_FALSE;
	
	if (len==2) {
		language[0]=locale_string[0];
		language[1]=locale_string[1];
		language[2]=0;
		country[0]=0;
		region[0]=0;
	} else if (len==5) {
		language[0]=locale_string[0];
		language[1]=locale_string[1];
		language[2]=0;
		country[0]=locale_string[3];
		country[1]=locale_string[4];
		country[2]=0;
		region[0]=0;
		if (locale_string[2]!=separator) return PR_FALSE;
	} else if (len==8) {
		language[0]=locale_string[0];
		language[1]=locale_string[1];
		language[2]=0;
		country[0]=locale_string[3];
		country[1]=locale_string[4];
		country[2]=0;
		region[0]=locale_string[6];
		region[1]=locale_string[7];
		region[2]=0;
		if (locale_string[2]!=separator || locale_string[5]!=separator) return PR_FALSE;
	} else {
		return PR_FALSE;
	}

	return PR_TRUE;
}
