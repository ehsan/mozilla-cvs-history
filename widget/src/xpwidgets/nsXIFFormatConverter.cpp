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

#include "nsString.h"
#include "nsISupportsArray.h"
#include "nsRepository.h"
#include "nsCOMPtr.h"
#include "nsISupportsPrimitives.h"
#include "nsXPIDLString.h"

#include "nsITransferable.h" // for mime defs, this is BAD


// These are temporary
#if defined(XP_UNIX) || defined(XP_MAC) || defined(XP_BEOS)
#include <strstream.h>
#endif

#ifdef XP_PC
#include <strstrea.h>
#endif

// XIF convertor stuff
#include "nsIParser.h"
#include "nsParserCIID.h"
#include "nsHTMLContentSinkStream.h"
#include "nsHTMLToTXTSinkStream.h"
#include "nsXIFDTD.h"
#include "nsIStringStream.h"

#include "nsString.h"
#include "nsWidgetsCID.h"
#include "nsXIFFormatConverter.h"


// unicode conversion
#define NS_IMPL_IDS
  #include "nsIPlatformCharset.h"
#undef NS_IMPL_IDS
#include "nsISaveAsCharset.h"
  

static NS_DEFINE_IID(kCParserCID, NS_PARSER_IID);  // don't panic. NS_PARSER_IID just has the wrong name.

NS_IMPL_ADDREF(nsXIFFormatConverter)
NS_IMPL_RELEASE(nsXIFFormatConverter)
NS_IMPL_QUERY_INTERFACE1(nsXIFFormatConverter, nsIFormatConverter)


//-------------------------------------------------------------------------
//
// XIFFormatConverter constructor
//
//-------------------------------------------------------------------------
nsXIFFormatConverter::nsXIFFormatConverter()
{
  NS_INIT_REFCNT();
}

//-------------------------------------------------------------------------
//
// XIFFormatConverter destructor
//
//-------------------------------------------------------------------------
nsXIFFormatConverter::~nsXIFFormatConverter()
{
}


//
// GetInputDataFlavors
//
// Creates a new list and returns the list of all the flavors this converter
// knows how to import. In this case, it's just XIF.
//
// Flavors (strings) are wrapped in a primitive object so that JavaScript can
// access them easily via XPConnect.
//
NS_IMETHODIMP
nsXIFFormatConverter::GetInputDataFlavors(nsISupportsArray **_retval)
{
  if ( !_retval )
    return NS_ERROR_INVALID_ARG;
  
  nsresult rv = NS_NewISupportsArray ( _retval );  // addrefs for us
  if ( NS_SUCCEEDED(rv) )
    rv = AddFlavorToList ( *_retval, kXIFMime );
  
  return rv;
  
} // GetInputDataFlavors


//
// GetOutputDataFlavors
//
// Creates a new list and returns the list of all the flavors this converter
// knows how to export (convert). In this case, it's all sorts of things that XIF can be
// converted to.
//
// Flavors (strings) are wrapped in a primitive object so that JavaScript can
// access them easily via XPConnect.
//
NS_IMETHODIMP
nsXIFFormatConverter::GetOutputDataFlavors(nsISupportsArray **_retval)
{
  if ( !_retval )
    return NS_ERROR_INVALID_ARG;
  
  nsresult rv = NS_NewISupportsArray ( _retval );  // addrefs for us
  if ( NS_SUCCEEDED(rv) ) {
    rv = AddFlavorToList ( *_retval, kHTMLMime );
    if ( NS_FAILED(rv) )
      return rv;
    rv = AddFlavorToList ( *_retval, kAOLMailMime );
    if ( NS_FAILED(rv) )
      return rv;
    rv = AddFlavorToList ( *_retval, kUnicodeMime );
    if ( NS_FAILED(rv) )
      return rv;
    rv = AddFlavorToList ( *_retval, kTextMime );
    if ( NS_FAILED(rv) )
      return rv;
  }
  return rv;

} // GetOutputDataFlavors


//
// AddFlavorToList
//
// Convenience routine for adding a flavor wrapped in an nsISupportsString object
// to a list
//
nsresult
nsXIFFormatConverter :: AddFlavorToList ( nsISupportsArray* inList, const char* inFlavor )
{
  nsCOMPtr<nsISupportsString> dataFlavor;
  nsresult rv = nsComponentManager::CreateInstance(NS_SUPPORTS_STRING_PROGID, nsnull, 
                                                    NS_GET_IID(nsISupportsString), getter_AddRefs(dataFlavor));
  if ( dataFlavor ) {
    dataFlavor->SetData ( NS_CONST_CAST(char*, inFlavor) );
    // add to list as an nsISupports so the correct interface gets the addref
    // in AppendElement()
    nsCOMPtr<nsISupports> genericFlavor ( do_QueryInterface(dataFlavor) );
    inList->AppendElement ( genericFlavor);
  }
  return rv;

} // AddFlavorToList


//
// CanConvert
//
// Determines if we support the given conversion. Currently, this method only
// converts from XIF to others.
//
NS_IMETHODIMP
nsXIFFormatConverter::CanConvert(const char *aFromDataFlavor, const char *aToDataFlavor, PRBool *_retval)
{
  if ( !_retval )
    return NS_ERROR_INVALID_ARG;
  
  *_retval = PR_FALSE;
  nsAutoString fromFlavor ( aFromDataFlavor );
  if ( fromFlavor.Equals(kXIFMime) ) {
    nsAutoString toFlavor ( aToDataFlavor );
    if ( toFlavor.Equals(kTextMime) )
      *_retval = PR_TRUE;
    else if ( toFlavor.Equals(kHTMLMime) )
      *_retval = PR_TRUE;
    else if ( toFlavor.Equals(kUnicodeMime) )
      *_retval = PR_TRUE;
    else if ( toFlavor.Equals(kAOLMailMime) )
      *_retval = PR_TRUE;
  }
  return NS_OK;

} // CanConvert



//
// Convert
//
// Convert data from one flavor to another. The data is wrapped in primitive objects so that it is
// accessable from JS. Currently, this only accepts XIF input, so anything else is invalid.
//
//XXX This method copies the data WAAAAY too many time for my liking. Grrrrrr. Mostly it's because
//XXX we _must_ put things into nsStrings so that the parser will accept it. Lame lame lame lame. We
//XXX also can't just get raw unicode out of the nsString, so we have to allocate heap to get
//XXX unicode out of the string. Lame lame lame.
//
NS_IMETHODIMP
nsXIFFormatConverter::Convert(const char *aFromDataFlavor, nsISupports *aFromData, PRUint32 aDataLen, 
                               const char *aToDataFlavor, nsISupports **aToData, PRUint32 *aDataToLen)
{
  if ( !aToData || !aDataToLen )
    return NS_ERROR_INVALID_ARG;

  nsresult rv = NS_OK;
  
  nsCAutoString fromFlavor ( aFromDataFlavor );
  if ( fromFlavor.Equals(kXIFMime) ) {
    nsCAutoString toFlavor ( aToDataFlavor );

    // XIF on clipboard is going to always be double byte so it will be in a primitive
    // class of nsISupportsWString. Also, since the data is in two byte chunks the 
    // length represents the length in 1-byte chars, so we need to divide by two.
    nsCOMPtr<nsISupportsWString> dataWrapper0 ( do_QueryInterface(aFromData) );
    if ( dataWrapper0 ) {
      nsXPIDLString data;
      dataWrapper0->ToString ( getter_Copies(data) );  //��� COPY #1
      if ( data ) {
        PRUnichar* castedData = NS_CONST_CAST(PRUnichar*, NS_STATIC_CAST(const PRUnichar*, data));
        nsAutoString dataStr ( CBufDescriptor(castedData, PR_TRUE, aDataLen) );  //��� try not to copy the data
        nsAutoString outStr;

        if ( toFlavor.Equals(kTextMime) ) {
          if ( NS_SUCCEEDED(ConvertFromXIFToText(dataStr, outStr)) ) {  //��� shouldn't copy
            nsCOMPtr<nsISupportsString> dataWrapper;
            nsComponentManager::CreateInstance(NS_SUPPORTS_STRING_PROGID, nsnull, 
                                                NS_GET_IID(nsISupportsString), getter_AddRefs(dataWrapper) );
            if ( dataWrapper ) {
              char* holderBecauseNSStringIsLame = outStr.ToNewCString();  //��� COPY #2
              dataWrapper->SetData ( holderBecauseNSStringIsLame );       //��� COPY #3
              nsCOMPtr<nsISupports> genericDataWrapper ( do_QueryInterface(dataWrapper) );
              *aToData = genericDataWrapper;
              NS_ADDREF(*aToData);
              *aDataToLen = outStr.Length();
              nsCRT::free(holderBecauseNSStringIsLame);
            }
          }
        } // if plain text
        else if ( toFlavor.Equals(kHTMLMime) || toFlavor.Equals(kUnicodeMime) ) {
          nsresult res;
          if (toFlavor.Equals(kHTMLMime))
            res = ConvertFromXIFToHTML(dataStr, outStr);
          else
            res = ConvertFromXIFToText(dataStr, outStr);
          if ( NS_SUCCEEDED(res) ) {
            nsCOMPtr<nsISupportsWString> dataWrapper;
            nsComponentManager::CreateInstance(NS_SUPPORTS_WSTRING_PROGID, nsnull, 
                                                NS_GET_IID(nsISupportsWString), getter_AddRefs(dataWrapper) );
            if ( dataWrapper ) {
              dataWrapper->SetData ( NS_CONST_CAST(PRUnichar*,outStr.GetUnicode()) );  //��� COPY #2
              nsCOMPtr<nsISupports> genericDataWrapper ( do_QueryInterface(dataWrapper) );
              *aToData = genericDataWrapper;
              NS_ADDREF(*aToData);
              *aDataToLen = outStr.Length() * 2;
            }
          }
        } // else if HTML
        else if ( toFlavor.Equals(kAOLMailMime) ) {
          if ( NS_SUCCEEDED(ConvertFromXIFToAOLMail(dataStr, outStr)) ) {  //��� COPY #2
            nsCOMPtr<nsISupportsWString> dataWrapper;
            nsComponentManager::CreateInstance(NS_SUPPORTS_WSTRING_PROGID, nsnull, 
                                                NS_GET_IID(nsISupportsWString), getter_AddRefs(dataWrapper) );
            if ( dataWrapper ) {
              dataWrapper->SetData ( NS_CONST_CAST(PRUnichar*,outStr.GetUnicode()) );  //��� COPY #3
              nsCOMPtr<nsISupports> genericDataWrapper ( do_QueryInterface(dataWrapper) );
              *aToData = genericDataWrapper;
              NS_ADDREF(*aToData);
              *aDataToLen = outStr.Length() * 2;
            }
          }
        } // else if AOL mail
        else {
          *aToData = nsnull;
          *aDataToLen = 0;
          rv = NS_ERROR_FAILURE;      
        }
      }
    }
    
  } // if we got xif mime
  else
    rv = NS_ERROR_FAILURE;      
    
  return rv;
  
} // Convert



//
// ConvertFromXIFToText
//
// Takes XIF and converts it to plain text using the correct charset for the platform/OS/language.
//
NS_IMETHODIMP
nsXIFFormatConverter::ConvertFromXIFToText(const nsAutoString & aFromStr, nsAutoString & aToStr)
{
  // Figure out the correct charset we need to use. We are guaranteed that this does not change
  // so we cache it.
  static nsAutoString platformCharset;
  static PRBool hasDeterminedCharset = PR_FALSE;
  if ( !hasDeterminedCharset ) {
    nsCOMPtr <nsIPlatformCharset> platformCharsetService;
    nsresult res = nsComponentManager::CreateInstance(NS_PLATFORMCHARSET_PROGID, nsnull, 
                                                       NS_GET_IID(nsIPlatformCharset), 
                                                       getter_AddRefs(platformCharsetService));
    if (NS_SUCCEEDED(res))
      res = platformCharsetService->GetCharset(kPlatformCharsetSel_PlainTextInClipboard, platformCharset);
    if (NS_FAILED(res))
      platformCharset.SetString("ISO-8859-1");
      
    hasDeterminedCharset = PR_TRUE;
  }
  
  // create the parser to do the conversion.
  aToStr = "";
  nsCOMPtr<nsIParser> parser;
  nsresult rv = nsComponentManager::CreateInstance(kCParserCID, nsnull, NS_GET_IID(nsIParser),
                                                     getter_AddRefs(parser));
  if ( !parser )
    return rv;

  // create a string stream to hold the converted text in the appropriate charset. The stream
  // owns the char buffer it creates, so we don't have to worry about it.
  nsCOMPtr<nsISupports> stream;
  char* buffer = nsnull;
  rv = NS_NewCharOutputStream ( getter_AddRefs(stream), &buffer );   // owns |buffer|
  if ( !stream )    
    return rv;
  nsCOMPtr<nsIOutputStream> outStream ( do_QueryInterface(stream) );
  
  // convert it!
  nsCOMPtr<nsIHTMLContentSink> sink;
  rv = NS_New_HTMLToTXT_SinkStream(getter_AddRefs(sink),outStream,&platformCharset);
  if ( sink ) {
    parser->SetContentSink(sink);
	
    nsCOMPtr<nsIDTD> dtd;
    rv = NS_NewXIFDTD(getter_AddRefs(dtd));
    if ( dtd ) {
      parser->RegisterDTD(dtd);
      parser->Parse(aFromStr, 0, "text/xif",PR_FALSE,PR_TRUE);           
    }
  }
  
  // assign the data back into our out param.
  aToStr = buffer;

  return NS_OK;
}

/**
  * 
  *
  */
NS_IMETHODIMP
nsXIFFormatConverter::ConvertFromXIFToHTML(const nsAutoString & aFromStr, nsAutoString & aToStr)
{
  aToStr = "";
  nsCOMPtr<nsIParser> parser;
  nsresult rv = nsComponentManager::CreateInstance(kCParserCID, 
                                             nsnull, 
                                             nsIParser::GetIID(), 
                                             getter_AddRefs(parser));
  if ( !parser )
    return rv;

  nsCOMPtr<nsIHTMLContentSink> sink;
  rv = NS_New_HTML_ContentSinkStream(getter_AddRefs(sink),&aToStr,0);
  if ( sink ) {
    parser->SetContentSink(sink);
	
    nsCOMPtr<nsIDTD> dtd;
    rv = NS_NewXIFDTD(getter_AddRefs(dtd));
    if ( dtd ) {
      parser->RegisterDTD(dtd);
      parser->Parse(aFromStr, 0, "text/xif",PR_FALSE,PR_TRUE);           
    }
  }
  return NS_OK;
}

/**
  * 
  *
  */
NS_IMETHODIMP
nsXIFFormatConverter::ConvertFromXIFToAOLMail(const nsAutoString & aFromStr, nsAutoString & aToStr)
{
  nsAutoString html;
  if (NS_OK == ConvertFromXIFToHTML(aFromStr, html)) {
    aToStr = "<HTML>";
    aToStr.Append(html);
    aToStr.Append("</HTML>");
  }
  return NS_OK;
}

