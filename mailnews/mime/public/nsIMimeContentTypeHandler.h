/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
 
/*
 * This interface is implemented by content type handlers that will be
 * called upon by libmime to process various attachments types. The primary
 * purpose of these handlers will be to represent the attached data in a
 * viewable HTML format that is useful for the user
 *
 * Note: These will all register by their content type prefixed by the
 *       following:  mimecth:text/vcard
 * 
 *       libmime will then use nsComponentManager::ProgIDToCLSID() to 
 *       locate the appropriate Content Type handler
 */
#ifndef nsIMimeContentTypeHandler_h_
#define nsIMimeContentTypeHandler_h_

typedef struct {
  PRBool      force_inline_display;
} contentTypeHandlerInitStruct;

#include "prtypes.h"
#include "nsISupports.h"
#include "mimecth.h"

// {20DABD99-F8B5-11d2-8EE0-00A024A7D144}
#define NS_IMIME_CONTENT_TYPE_HANDLER_IID \
      { 0x20dabd99, 0xf8b5, 0x11d2,   \
      { 0x8e, 0xe0, 0x0, 0xa0, 0x24, 0xa7, 0xd1, 0x44 } }

// {20DABDA1-F8B5-11d2-8EE0-00A024A7D144}
#define NS_VCARD_CONTENT_TYPE_HANDLER_CID \
      { 0x20dabda1, 0xf8b5, 0x11d2, \
      { 0x8e, 0xe0, 0x0, 0xa0, 0x24, 0xa7, 0xd1, 0x44 } }

#define NS_CALENDAR_CONTENT_TYPE_HANDLER_CID \
      { 0x20dabdac, 0xf8b5, 0x11d2, \
      { 0x0b, 0xe0, 0x0, 0xa0, 0x24, 0xa7, 0xd1, 0x44 } }

#define NS_SMIME_CONTENT_TYPE_HANDLER_CID \
      { 0x20dabdac, 0xf8b5, 0x11d2, \
      { 0xFF, 0xe0, 0x0, 0xa0, 0x24, 0xa7, 0xd1, 0x44 } }

#define NS_SIGNED_CONTENT_TYPE_HANDLER_CID \
      { 0x20dabdac, 0xf8b5, 0x11d2, \
      { 0xFF, 0xe0, 0x0, 0xaf, 0x19, 0xa7, 0xd1, 0x44 } }

class nsIMimeContentTypeHandler : public nsISupports {
public: 
  static const nsIID& GetIID() { static nsIID iid = NS_IMIME_CONTENT_TYPE_HANDLER_IID; return iid; }

  NS_IMETHOD    GetContentType(char **contentType) = 0;

  NS_IMETHOD    CreateContentTypeHandlerClass(const char *content_type, 
                                              contentTypeHandlerInitStruct *initStruct, 
                                              MimeObjectClass **objClass) = 0;
}; 

#endif /* nsIMimeContentTypeHandler_h_ */
