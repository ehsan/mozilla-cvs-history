/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
 * Contributor(s): ruslan
 */

#include "nsHTTPCompressConv.h"
#include "nsIAllocator.h"
#include "plstr.h"
#include "prlog.h"
#include "nsIChannel.h"
#include "nsCOMPtr.h"
#include "nsIByteArrayInputStream.h"
#include "nsIStringStream.h"

// nsISupports implementation
NS_IMPL_ISUPPORTS2 (nsHTTPCompressConv, nsIStreamConverter, nsIStreamListener);

// nsFTPDirListingConv methods
nsHTTPCompressConv::nsHTTPCompressConv ()
    :   mListener (nsnull),
        mInpBuffer  (NULL), mInpBufferLen (0),
        mOutBuffer  (NULL), mOutBufferLen (0),
        mMode (HTTP_COMPRESS_IDENTITY),
        mCheckHeaderDone (PR_FALSE),
        mGzipStreamInitialized (PR_FALSE), mGzipStreamEnded (PR_FALSE),
        hMode (0), mLen (0), mSkipCount (0), mFlags (0)
{
    NS_INIT_ISUPPORTS ();
}

nsHTTPCompressConv::~nsHTTPCompressConv ()
{
    NS_IF_RELEASE(mListener);

	if (mInpBuffer != NULL)
		nsAllocator::Free (mInpBuffer);

    if (mOutBuffer != NULL)
		nsAllocator::Free (mOutBuffer);
}

NS_IMETHODIMP
nsHTTPCompressConv::AsyncConvertData (
							const PRUnichar *aFromType, 
							const PRUnichar *aToType, 
							nsIStreamListener *aListener, 
							nsISupports *aCtxt)
{
	nsString2 from (aFromType);
	nsString2 to   ( aToType );

	char * fromStr = from.ToNewCString ();
	char *   toStr =   to.ToNewCString ();

	if (!PL_strncasecmp (fromStr, HTTP_COMPRESS_TYPE  , strlen (HTTP_COMPRESS_TYPE  ))
        ||
        !PL_strncasecmp (fromStr, HTTP_X_COMPRESS_TYPE, strlen (HTTP_X_COMPRESS_TYPE)))
        mMode = HTTP_COMPRESS_COMPRESS;
    else
	if (!PL_strncasecmp (fromStr, HTTP_GZIP_TYPE   , strlen (HTTP_COMPRESS_TYPE))
        ||
        !PL_strncasecmp (fromStr, HTTP_X_GZIP_TYPE , strlen (HTTP_X_GZIP_TYPE)))
        mMode = HTTP_COMPRESS_GZIP;
    else
	if (!PL_strncasecmp (fromStr, HTTP_DEFLATE_TYPE, strlen (HTTP_DEFLATE_TYPE)))
        mMode = HTTP_COMPRESS_DEFLATE;

	nsAllocator::Free (fromStr);
	nsAllocator::Free (  toStr);

    // hook ourself up with the receiving listener. 
    mListener = aListener;
    NS_ADDREF (mListener);

    mAsyncConvContext = aCtxt;
	
    return NS_OK; 
} 

NS_IMETHODIMP
nsHTTPCompressConv::OnStartRequest (nsIChannel *aChannel, nsISupports *aContext)
{
    return mListener -> OnStartRequest (aChannel, aContext);
} 

NS_IMETHODIMP
nsHTTPCompressConv::OnStopRequest  (nsIChannel *aChannel, nsISupports *aContext, nsresult status, const PRUnichar *errorMsg)
{
    return mListener -> OnStopRequest  (aChannel, aContext, status, errorMsg);
} 

NS_IMETHODIMP
nsHTTPCompressConv::OnDataAvailable ( 
							  nsIChannel *aChannel, 
							  nsISupports *aContext, 
							  nsIInputStream *iStr, 
							  PRUint32 aSourceOffset, 
							  PRUint32 aCount)
{
    nsresult rv = NS_ERROR_FAILURE;
	PRUint32 streamLen;

    rv = iStr -> Available (&streamLen);
    if (NS_FAILED (rv))
		return rv;

	if (streamLen == 0 || mGzipStreamEnded)
		return NS_OK;

    switch (mMode)
    {
        case HTTP_COMPRESS_GZIP    :
            streamLen = check_header (iStr, streamLen, &rv);

            if (rv != NS_OK)
                return rv;

            if (streamLen == 0)
                return NS_OK;

        case HTTP_COMPRESS_COMPRESS:
        case HTTP_COMPRESS_DEFLATE :

            if (mInpBuffer != NULL && streamLen > mInpBufferLen)
            {
                mInpBuffer = (unsigned char *) nsAllocator::Realloc (mInpBuffer, mInpBufferLen = streamLen);
               
                if (mOutBufferLen < streamLen * 2)
                    mInpBuffer = (unsigned char *) nsAllocator::Realloc (mOutBuffer, mInpBufferLen = streamLen * 3);

                if (mInpBuffer == NULL || mOutBuffer == NULL)
                    return NS_ERROR_OUT_OF_MEMORY;
            }

            if (mInpBuffer == NULL)
                mInpBuffer = (unsigned char *) nsAllocator::Alloc (mInpBufferLen = streamLen);

            if (mOutBuffer == NULL)
                mOutBuffer = (unsigned char *) nsAllocator::Alloc (mOutBufferLen = streamLen * 3);

            if (mInpBuffer == NULL || mOutBuffer == NULL)
                return NS_ERROR_OUT_OF_MEMORY;

            iStr -> Read ((char *)mInpBuffer, streamLen, &rv);

            if (NS_FAILED (rv))
                return rv;

            if (mMode == HTTP_COMPRESS_COMPRESS)
            {
                unsigned long uLen = mOutBufferLen;
                int code = uncompress (mOutBuffer, &uLen, mInpBuffer, mInpBufferLen);
                if (code == Z_BUF_ERROR)
                {
                    mOutBuffer = (unsigned char *) nsAllocator::Realloc (mOutBuffer, mOutBufferLen *= 3);
                    if (mOutBuffer == NULL)
                        return NS_ERROR_OUT_OF_MEMORY;
                    
                    code = uncompress (mOutBuffer, &uLen, mInpBuffer, mInpBufferLen);
                }
                if (code != Z_OK)
                    return NS_ERROR_FAILURE;

                rv = do_OnDataAvailable (aChannel, aContext, aSourceOffset, (char *)mOutBuffer, (PRUint32) uLen);
                if (NS_FAILED (rv))
                    return rv;

            }
            else
            {
                if (!mGzipStreamInitialized)
                {
                    memset (&d_stream, 0, sizeof (d_stream));
                
                    if (inflateInit (&d_stream) != Z_OK)
                        return NS_ERROR_FAILURE;

                    mGzipStreamInitialized = PR_TRUE;
                }

                d_stream.next_in  = mInpBuffer;
                d_stream.avail_in = (uInt)streamLen;

                for ( ; ; )
                {
                    d_stream.next_out  = mOutBuffer;
                    d_stream.avail_out = (uInt)mOutBufferLen;
 
                    int code = inflate  (&d_stream, Z_NO_FLUSH);
                    unsigned bytesWritten = (uInt)mOutBufferLen - d_stream.avail_out;

                    if (code == Z_STREAM_END)
                    {
                        if (bytesWritten)
                        {
                            rv = do_OnDataAvailable (aChannel, aContext, aSourceOffset, (char *)mOutBuffer, bytesWritten);
                            if (NS_FAILED (rv))
                                return rv;
                        }
                        
                        inflateEnd (&d_stream);
                        mGzipStreamEnded = PR_TRUE;
                        break;
                    }
                    else
                    if (code == Z_OK)
                    {
                        if (bytesWritten)
                        {
                            rv = do_OnDataAvailable (aChannel, aContext, aSourceOffset, (char *)mOutBuffer, bytesWritten);
                            if (NS_FAILED (rv))
                                return rv;
                        }
                    }
                    else
                    if (code == Z_BUF_ERROR)
                    {
                        if (bytesWritten)
                        {
                            rv = do_OnDataAvailable (aChannel, aContext, aSourceOffset, (char *)mOutBuffer, bytesWritten);
                            if (NS_FAILED (rv))
                                return rv;
                        }
                        break;
                    }
                    else
                        return NS_ERROR_FAILURE;
                } /* for */
            } /* gzip */
            break;

        default: 
            rv = mListener -> OnDataAvailable (aChannel, aContext, iStr, aSourceOffset, aCount);
			if (NS_FAILED (rv))
			    return rv;
    } /* switch */

	return NS_OK;
} /* OnDataAvailable */


// XXX/ruslan: need to implement this too

NS_IMETHODIMP
nsHTTPCompressConv::Convert (
						  nsIInputStream *aFromStream, 
						  const PRUnichar *aFromType, 
						  const PRUnichar *aToType, 
						  nsISupports *aCtxt, 
						  nsIInputStream **_retval)
{ 
    return NS_ERROR_NOT_IMPLEMENTED;
} 

nsresult
NS_NewHTTPCompressConv (nsHTTPCompressConv ** aHTTPCompressConv)
{
    NS_PRECONDITION(aHTTPCompressConv != nsnull, "null ptr");

    if (! aHTTPCompressConv)
        return NS_ERROR_NULL_POINTER;

    *aHTTPCompressConv = new nsHTTPCompressConv ();

    if (! *aHTTPCompressConv)
        return NS_ERROR_OUT_OF_MEMORY;

    NS_ADDREF(*aHTTPCompressConv);
    return NS_OK;
}

nsresult
nsHTTPCompressConv::do_OnDataAvailable (nsIChannel *aChannel, nsISupports *aContext, PRUint32 aSourceOffset, char *buffer, PRUint32 aCount)
{
    nsresult rv;

    nsIInputStream * convertedStream = nsnull;
	nsIByteArrayInputStream * convertedStreamSup = nsnull;

	rv = NS_NewByteArrayInputStream (&convertedStreamSup, buffer, aCount);
	if (NS_FAILED (rv)) 
	    return rv;

    rv = convertedStreamSup -> QueryInterface (NS_GET_IID (nsIInputStream), (void**)&convertedStream);
	NS_RELEASE (convertedStreamSup);
 
	if (NS_FAILED (rv))
	    return rv;

	rv = mListener -> OnDataAvailable (aChannel, aContext, convertedStream, aSourceOffset, aCount);

    return rv;
}

#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define RESERVED     0xE0 /* bits 5..7: reserved */

static unsigned gz_magic[2] = {0x1f, 0x8b}; /* gzip magic header */

PRUint32
nsHTTPCompressConv::check_header (nsIInputStream *iStr, PRUint32 streamLen, nsresult *rs)
{
    nsresult rv;
    enum  { GZIP_INIT = 0, GZIP_OS, GZIP_EXTRA0, GZIP_EXTRA1, GZIP_EXTRA2, GZIP_ORIG, GZIP_COMMENT, GZIP_CRC };
    char c;

    *rs = NS_OK;

    if (mCheckHeaderDone)
        return streamLen;

    while (streamLen)
    {
        switch (hMode)
        {
            case GZIP_INIT:
                iStr -> Read (&c, 1, &rv);
                streamLen--;
                
                if (mSkipCount == 0 && ((unsigned)c & 0377) != gz_magic[0])
                {
                    *rs = NS_ERROR_FAILURE;
                    return 0;
                }

                if (mSkipCount == 1 && ((unsigned)c & 0377) != gz_magic[1])
                {
                    *rs = NS_ERROR_FAILURE;
                    return 0;
                }

                if (mSkipCount == 2 && ((unsigned)c & 0377) != Z_DEFLATED)
                {
                    *rs = NS_ERROR_FAILURE;
                    return 0;
                }

                mSkipCount++;
                if (mSkipCount == 4)
                {
                    mFlags = (unsigned) c & 0377;
                    if (mFlags & RESERVED)
                    {
                        *rs = NS_ERROR_FAILURE;
                        return 0;
                    }
                    hMode = GZIP_OS;
                    mSkipCount = 0;
                }
                break;

            case GZIP_OS  :
                iStr -> Read (&c, 1, &rv);
                streamLen--;
                mSkipCount++;

                if (mSkipCount == 6)
                    hMode = GZIP_EXTRA0;
                break;
        
            case GZIP_EXTRA0:
                if (mFlags & EXTRA_FIELD)
                {
                    iStr -> Read (&c, 1, &rv);
                    streamLen--;
                    mLen = (uInt) c & 0377;
                    hMode = GZIP_EXTRA1;
                }
                else
                    hMode = GZIP_ORIG;
                break;

            case GZIP_EXTRA1:
                iStr -> Read (&c, 1, &rv);
                streamLen--;
                mLen = ((uInt) c & 0377) << 8;
                mSkipCount = 0;
                hMode = GZIP_EXTRA2;
                break;

            case GZIP_EXTRA2:
                if (mSkipCount == mLen)
                    hMode = GZIP_ORIG;
                else
                {
                    iStr -> Read (&c, 1, &rv);
                    streamLen--;
                    mSkipCount++;
                }
                break;

            case GZIP_ORIG:
                if (mFlags & ORIG_NAME)
                {
                    iStr -> Read (&c, 1, &rv);
                    streamLen--;
                    if (c == 0)
                        hMode = GZIP_COMMENT;
                }
                else
                    hMode = GZIP_COMMENT;
                break;

            case GZIP_COMMENT:
                if (mFlags & GZIP_COMMENT)
                {
                    iStr -> Read (&c, 1, &rv);
                    streamLen--;
                    if (c == 0)
                    {
                        hMode = GZIP_CRC;
                        mSkipCount = 0;
                    }
                }
                else
                {
                    hMode = GZIP_CRC;
                    mSkipCount = 0;
                }
                break;

           case GZIP_CRC:
                if (mFlags & HEAD_CRC)
                {
                    iStr -> Read (&c, 1, &rv);
                    streamLen--;
                    mSkipCount++;
                    if (mSkipCount == 2)
                    {
                        mCheckHeaderDone = PR_TRUE;
                        return streamLen;
                    }
                }
                else
                {
                    mCheckHeaderDone = PR_TRUE;
                    return streamLen;
                }
            break;
        }
    }
    return streamLen;
}
