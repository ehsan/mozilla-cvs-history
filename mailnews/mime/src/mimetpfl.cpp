/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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

#include "mimetpfl.h"
#include "mimebuf.h"
#include "prmem.h"
#include "plstr.h"
#include "nsMimeTransition.h"
#include "nsMimeURLUtils.h"
#include "nsCRT.h"
#include "nsMimeStringResources.h"

#define MIME_SUPERCLASS mimeInlineTextClass
MimeDefClass(MimeInlineTextPlainFlowed, MimeInlineTextPlainFlowedClass,
			 mimeInlineTextPlainFlowedClass, &MIME_SUPERCLASS);

static int MimeInlineTextPlainFlowed_parse_begin (MimeObject *);
static int MimeInlineTextPlainFlowed_parse_line (char *, PRInt32, MimeObject *);
static int MimeInlineTextPlainFlowed_parse_eof (MimeObject *, PRBool);

static MimeInlineTextPlainFlowedExData *MimeInlineTextPlainFlowedExDataList = 0;



static int
MimeInlineTextPlainFlowedClassInitialize(MimeInlineTextPlainFlowedClass *clazz)
{
  MimeObjectClass *oclass = (MimeObjectClass *) clazz;
  NS_ASSERTION(!oclass->class_initialized, "class not initialized");
  oclass->parse_begin = MimeInlineTextPlainFlowed_parse_begin;
  oclass->parse_line  = MimeInlineTextPlainFlowed_parse_line;
  oclass->parse_eof   = MimeInlineTextPlainFlowed_parse_eof;
  
  return 0;
}

static int
MimeInlineTextPlainFlowed_parse_begin (MimeObject *obj)
{
  int status = ((MimeObjectClass*)&MIME_SUPERCLASS)->parse_begin(obj);
  if (status < 0) return status;

  char s[] = "";  
  status =  MimeObject_write(obj, s, 0, PR_TRUE); /* force out any separators... */
  if(status<0) return status;

  /* This memory is freed when parse_eof is called. So it better be! */
  struct MimeInlineTextPlainFlowedExData *exdata =
    (MimeInlineTextPlainFlowedExData *)PR_MALLOC(sizeof(struct MimeInlineTextPlainFlowedExData));
  if(!exdata) return MIME_OUT_OF_MEMORY;
  exdata->ownerobj = obj;
  exdata->inflow=0;
  exdata->quotelevel=0;
  exdata->next = MimeInlineTextPlainFlowedExDataList;
  MimeInlineTextPlainFlowedExDataList = exdata;
  
  return 0;

  /* FROM text/plain  
  int status = 0;

  status = ((MimeObjectClass*)&MIME_SUPERCLASS)->parse_begin(obj);
  if (status < 0) return status;

  if (!obj->output_p) return 0;

  if (obj->options &&
	  obj->options->write_html_p &&
	  obj->options->output_fn)
	{
	  char* strs[4];
	  char* s;
	  strs[0] = "<PRE>";
	  strs[1] = "<PRE VARIABLE>";
	  strs[2] = "< RE WRAP>";
	  strs[3] = "<PRE VARIABLE WRAP>";
	  s = nsCRT::strdup(strs[(obj->options->variable_width_plaintext_p ? 1 : 0) +
						(obj->options->wrap_long_lines_p ? 2 : 0)]);
	  if (!s) return MIME_OUT_OF_MEMORY;
	  status = MimeObject_write(obj, s, nsCRT::strlen(s), PR_FALSE);
	  PR_Free(s);
	  if (status < 0) return status;
  */
	  /* text/plain objects always have separators before and after them.
		 Note that this is not the case for text/enriched objects. */
  /*	  status = MimeObject_write_separator(obj);
	  if (status < 0) return status;
	}

  return 0;
  */
}

static int
MimeInlineTextPlainFlowed_parse_eof (MimeObject *obj, PRBool abort_p)
{
  int status;
  if (obj->closed_p) return 0;
  
  /* Run parent method first, to flush out any buffered data. */
  status = ((MimeObjectClass*)&MIME_SUPERCLASS)->parse_eof(obj, abort_p);
  if (status < 0) return status;

  if (!obj->output_p) return 0;


  struct MimeInlineTextPlainFlowedExData *exdata;
  struct MimeInlineTextPlainFlowedExData *prevexdata;
  exdata = MimeInlineTextPlainFlowedExDataList;
  prevexdata = MimeInlineTextPlainFlowedExDataList;
  while(exdata && (exdata->ownerobj != obj)) {
    prevexdata = exdata;
    exdata = exdata->next;
  }
  NS_ASSERTION(exdata, "The extra data has disappeared!");

  if(exdata == MimeInlineTextPlainFlowedExDataList) {
    // No previous.
    MimeInlineTextPlainFlowedExDataList = exdata->next;
  } else {
    prevexdata->next = exdata->next;
  }

  uint32 quotelevel = exdata->quotelevel;
  
  PR_Free(exdata);

  while(quotelevel>0) {
    // Write </blockquote>
    status = MimeObject_write(obj, "</blockquote>", 13, PR_FALSE);
    if(status<0) {
      return status;
    }
    
    quotelevel--;
  }

  
  /*  if (obj->options &&
	  obj->options->write_html_p &&
	  obj->options->output_fn &&
	  !abort_p)
	{
	  char s[] = "</PRE>";
	  status = MimeObject_write(obj, s, nsCRT::strlen(s), PR_FALSE);
	  if (status < 0) return status;
  */
	  /* text/plain objects always have separators before and after them.
		 Note that this is not the case for text/enriched objects.
	   */
  /*	  status = MimeObject_write_separator(obj);
	  if (status < 0) return status;
	}
  */
  return 0;
}


static int
MimeInlineTextPlainFlowed_parse_line (char *line, PRInt32 length, MimeObject *obj)
{
  int status;
  
  struct MimeInlineTextPlainFlowedExData *exdata;
  exdata = MimeInlineTextPlainFlowedExDataList;
  while(exdata && (exdata->ownerobj != obj)) {
    exdata = exdata->next;
  }
    
  NS_ASSERTION(exdata, "The extra data has disappeared!");

#ifdef DEBUG_bratell
  fprintf(stderr,"*IN*");
  for(int32 lineindex = 0; lineindex < length; lineindex++) {
    switch(line[lineindex]) {
    case ' ': fprintf(stderr, "_", line[lineindex]);break;
    case '\n': fprintf(stderr, "\\n", line[lineindex]);break;
    case '\r': fprintf(stderr, "\\r", line[lineindex]);break;
    default:  fprintf(stderr, "%c", line[lineindex]);break;
    }
  }
  fprintf(stderr,"$\n");
#endif

  NS_ASSERTION(length > 0, "zero length");
  if (length <= 0) return 0;

  // Every line ends in a '\r' and a '\n' according to the rfc822 format.
  // The line is flowed if the the third to last char is a ' '
  
  NS_ASSERTION(length>1, "Line should be at least a \"\\r\\n\"");
  NS_ASSERTION('\n' == line[length-1], "Corrupt line end (#1)");
  NS_ASSERTION('\r' == line[length-2], "Corrupt line end (#2)");

  
  int32 flowed = (length>=2 && (' ' == line[length-3]));
  
  // Grows the buffer if needed for this line
  // calculate needed buffersize. Use the linelength
  // and then double it to compensate for text converted
  // to links. Then add 15 for each '>' (blockquote) and
  // 20 for each ':' and '@'. (overhead in conversion to
  // links). Also add 5 for every '\r' or \n' and 7 for each
  // space in case they have to be replaced by &nbsp;
  int32 buffersizeneeded = length * 2 + 15*exdata->quotelevel;
  for(int32 i=0; i<length; i++) {
    switch(line[i]) {
    case '>': buffersizeneeded += 25; break; // '>' -> '<blockquote type=cite>'
    case '<': buffersizeneeded += 5; break; // '<' -> '&lt;'
    case ':': buffersizeneeded += 20; break;
    case '@': buffersizeneeded += 20; break;
    case '\r': buffersizeneeded += 5; break;
    case '\n': buffersizeneeded += 5; break;
    case ' ': buffersizeneeded += 7; break; // Not very good for other charsets
    case '\t': buffersizeneeded += 30; break;
    default: break; // Nothing
    }
  }
  buffersizeneeded += 30; // For possible <nobr> ... </nobr>
  status = MimeObject_grow_obuffer (obj, buffersizeneeded);
  if (status < 0) return status;

  char *templine = (char *)PR_MALLOC(buffersizeneeded);
  if(!templine) 
    return MIME_OUT_OF_MEMORY;
  
  /* Copy `line' to `templine', quoting HTML along the way.
	 Note: this function does no charset conversion; that has already
	 been done.
   */
  templine[0] = '\0';
  //  *obj->obuffer = 0;

  nsMimeURLUtils myUtil;

  uint32 linequotelevel = 0;
  char *linep = line;
  // Space stuffed?
  if(' ' == *linep) {
    linep++;
  } else {
    // count '>':s before the first non-'>'
    while('>' == *linep) {
      linep++;
      linequotelevel++;
    }
    // Space stuffed?
    if(' ' == *linep) {
      linep++;
    }
  }
  
  // If we have been told not to mess with this text, then don't do this search!
  PRBool skipScanning = (obj->options && obj->options->force_user_charset);

  if (!skipScanning)
  {
    // Convert mail addresses and web addresses to links. Strip out the part
    // with the quotes in the beginning.
    status = myUtil.ScanForURLs(linep, length-linequotelevel, templine, buffersizeneeded,
                  (obj->options ?
					      obj->options->dont_touch_citations_p : PR_FALSE));
  }  
  else
  {
    nsCRT::memcpy(templine, line, length);
    templine[length] = '\0';
    status = NS_OK;
  }

  if (status != NS_OK) {
    PR_Free(templine);
    return status;
  }

  //  NS_ASSERTION(*line == 0 || *obj->obuffer, "have line or buffer");
  NS_ASSERTION(*line == 0 || *templine, "have line or buffer");

  char *templinep = templine;
  char *outlinep = obj->obuffer;


  /* Correct number of blockquotes */
  int32 quoteleveldiff=linequotelevel - exdata->quotelevel;
  if((quoteleveldiff != 0) && flowed && exdata->inflow) {
    // From RFC 2646 4.5
    // The receiver SHOULD handle this error by using the 'quote-depth-wins' rule,
    // which is to ignore the flowed indicator and treat the line as fixed.  That
    // is, the change in quote depth ends the paragraph.

    // We get that behaviour by just going on.
  }
  while(quoteleveldiff>0) {
    quoteleveldiff--;
    /* Output <blockquote> */
    *outlinep='<'; outlinep++;
    *outlinep='b'; outlinep++;
    *outlinep='l'; outlinep++;
    *outlinep='o'; outlinep++;
    *outlinep='c'; outlinep++;
    *outlinep='k'; outlinep++;
    *outlinep='q'; outlinep++;
    *outlinep='u'; outlinep++;
    *outlinep='o'; outlinep++;
    *outlinep='t'; outlinep++;
    *outlinep='e'; outlinep++;
    *outlinep=' '; outlinep++;
    *outlinep='t'; outlinep++;
    *outlinep='y'; outlinep++;
    *outlinep='p'; outlinep++;
    *outlinep='e'; outlinep++;
    *outlinep='='; outlinep++;
    *outlinep='c'; outlinep++;
    *outlinep='i'; outlinep++;
    *outlinep='t'; outlinep++;
    *outlinep='e'; outlinep++;
    *outlinep='>'; outlinep++;
  }
  while(quoteleveldiff<0) {
    quoteleveldiff++;
    /* Output </blockquote> */
    *outlinep='<'; outlinep++;
    *outlinep='/'; outlinep++;
    *outlinep='b'; outlinep++;
    *outlinep='l'; outlinep++;
    *outlinep='o'; outlinep++;
    *outlinep='c'; outlinep++;
    *outlinep='k'; outlinep++;
    *outlinep='q'; outlinep++;
    *outlinep='u'; outlinep++;
    *outlinep='o'; outlinep++;
    *outlinep='t'; outlinep++;
    *outlinep='e'; outlinep++;
    *outlinep='>'; outlinep++;
  }
  exdata->quotelevel = linequotelevel;
  

  if(flowed) {
    int32 dashdashspace; // Check for RFC 2646 4.3
    if((0==linequotelevel) && !exdata->inflow){
      // possible
      dashdashspace = 0;
    } else {
      dashdashspace = -1;
    }
    while(*templinep && ('\r' != *templinep)) {
      // Check RFC 2646 "4.3. Usenet Signature Convention": "-- "+CRLF is
      // not a flowed line
      if(dashdashspace>=0) {
        switch(dashdashspace) {
        case 0: // First and second '-'
        case 1:
          if('-' == *templinep)
            dashdashspace++;
          else
            dashdashspace=-1;
          break;
        case 2:
          if(' ' == *templinep)
            dashdashspace++;
          else
            dashdashspace=-1;
          break;
        default:
          dashdashspace = -1;
        }
      } // end if(dashdashspace...)

      // Output the same character

      *outlinep=*templinep;
      outlinep++;
      templinep++;
      
    }
    if(3 == dashdashspace) {
      // "-- " is a fixed line. 
      *outlinep='<'; outlinep++;
      *outlinep='b'; outlinep++;
      *outlinep='r'; outlinep++;
      *outlinep='>'; outlinep++;
    } 

    exdata->inflow=1;
  } else {
    // Fixed
    uint32 intag = 0; 

    /*    // Output the stripped '>':s
    while(linequotelevel) {
      // output '>'
      *outlinep='&'; outlinep++;
      *outlinep='g'; outlinep++;
      *outlinep='t'; outlinep++;
      *outlinep=';'; outlinep++;
      linequotelevel--;
    }
    */
    
    while(*templinep && (*templinep != '\r')) {
      if('<' == *templinep) intag = 1;
      if(!intag) {
        if((' ' == *templinep) && !exdata->inflow) {
          // To not get any line wraps
          *outlinep='&'; outlinep++;
          *outlinep='n'; outlinep++;
          *outlinep='b'; outlinep++;
          *outlinep='s'; outlinep++;
          *outlinep='p'; outlinep++;
          *outlinep=';'; outlinep++;
          templinep++;
        } else if(('\t' == *templinep) && !exdata->inflow) {
          // To not get any line wraps
          // Output 4 spaces
          for(int spaces=0; spaces<4; spaces++) {
            *outlinep='&'; outlinep++;
            *outlinep='n'; outlinep++;
            *outlinep='b'; outlinep++;
            *outlinep='s'; outlinep++;
            *outlinep='p'; outlinep++;
            *outlinep=';'; outlinep++;
          }
          templinep++;
        } else {
          *outlinep = *templinep;
          outlinep++;
          templinep++;
        } 
      } else {
        // In tag. Don't change anything
        *outlinep = *templinep;
        outlinep++;
        templinep++;
      }
    }
    
    *outlinep='<'; outlinep++;
    *outlinep='b'; outlinep++;
    *outlinep='r'; outlinep++;
    *outlinep='>'; outlinep++;

    exdata->inflow = 0;
  } // End Fixed line

  *outlinep='\r'; outlinep++;
  *outlinep='\n'; outlinep++;
  *outlinep='\0'; outlinep++;

  PR_Free(templine);
  
  
  
#ifdef DEBUG_bratell
  fprintf(stderr,"*OUT*");
  for(uint32 lineindex2 = 0; lineindex2 < nsCRT::strlen(obj->obuffer); lineindex2++) {
    switch(obj->obuffer[lineindex2]) {
    case ' ': fprintf(stderr, "_", obj->obuffer[lineindex2]);break;
    case '\n': fprintf(stderr, "\\n", obj->obuffer[lineindex2]);break;
    case '\r': fprintf(stderr, "\\r", obj->obuffer[lineindex2]);break;
      //    case ' ': fprintf(stderr, "%c", obj->obuffer[lineindex2]);break;
      //    case ' ': fprintf(stderr, "%c", obj->obuffer[lineindex2]);break;
    default:  fprintf(stderr, "%c", obj->obuffer[lineindex2]);break;
    }
  }
  fprintf(stderr,"$\n");
#endif

  // Calculate linelength as
  // <pointer to the next free char>-<pointer to the beginning>-1
  // '-1' for the terminating '\0'
  return MimeObject_write(obj, obj->obuffer, outlinep-obj->obuffer-1, PR_TRUE);
}
