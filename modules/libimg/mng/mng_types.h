/* ************************************************************************** */
/* *             For conditions of distribution and use,                    * */
/* *                see copyright notice in libmng.h                        * */
/* ************************************************************************** */
/* *                                                                        * */
/* * project   : libmng                                                     * */
/* * file      : mng_types.h               copyright (c) 2000 G.Juyn        * */
/* * version   : 0.5.3                                                      * */
/* *                                                                        * */
/* * purpose   : type specifications                                        * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* * web       : http://www.3-t.com                                         * */
/* * email     : mailto:info@3-t.com                                        * */
/* *                                                                        * */
/* * comment   : Specification of the types used by the library             * */
/* *             Creates platform-independant structure                     * */
/* *                                                                        * */
/* * changes   : 0.5.1 - 05/06/2000 - G.Juyn                                * */
/* *             - added iteratechunk callback definition                   * */
/* *             0.5.1 - 05/08/2000 - G.Juyn                                * */
/* *             - improved definitions for DLL support                     * */
/* *             - added 8-bit palette definition                           * */
/* *             - added general array definitions                          * */
/* *             - added MNG_NULL definition                                * */
/* *             - changed strict-ANSI stuff                                * */
/* *             0.5.1 - 05/11/2000 - G.Juyn                                * */
/* *             - changed most callback prototypes to allow the app        * */
/* *               to report errors during callback processing              * */
/* *             0.5.1 - 05/16/2000 - G.Juyn                                * */
/* *             - moved standard header includes into this file            * */
/* *               (stdlib/mem for mem-mngmt & math for fp gamma-calc)      * */
/* *                                                                        * */
/* *             0.5.2 - 05/18/2000 - G.Juyn                                * */
/* *             - B003 - fixed problem with <mem.h> being proprietary      * */
/* *               to Borland platform                                      * */
/* *             - added helper definitions for JNG (IJG-based)             * */
/* *             - fixed support for IJGSRC6B                               * */
/* *             0.5.2 - 05/24/2000 - G.Juyn                                * */
/* *             - added default IJG compression parameters and such        * */
/* *             0.5.2 - 05/31/2000 - G.Juyn                                * */
/* *             - fixed inclusion for memcpy (contributed by Tim Rowley)   * */
/* *             - added mng_int32p (contributed by Tim Rowley)             * */
/* *             0.5.2 - 06/02/2000 - G.Juyn                                * */
/* *             - removed SWAP_ENDIAN reference (contributed by Tim Rowley)* */
/* *             - added getalphaline callback for RGB8_A8 canvasstyle      * */
/* *                                                                        * */
/* *             0.5.3 - 06/21/2000 - G.Juyn                                * */
/* *             - added speedtype to facilitate testing                    * */
/* *                                                                        * */
/* ************************************************************************** */

#ifndef _mng_types_h_
#define _mng_types_h_

/* ************************************************************************** */

#ifdef __BORLANDC__
#pragma option -AT                     /* turn off strict ANSI-C for the moment */
#endif

#ifndef WIN32
#if defined(_WIN32) || defined(__WIN32__) || defined(_Windows) || defined(_WINDOWS)
#define WIN32                          /* gather them into a single define */
#endif
#endif

/* ************************************************************************** */
/* *                                                                        * */
/* * Here's where the external & standard libs are embedded                 * */
/* *                                                                        * */
/* * (it can be a bit of a pain in the lower-back to get them to work       * */
/* *  together)                                                             * */
/* *                                                                        * */
/* ************************************************************************** */

#ifdef MNG_INCLUDE_ZLIB                /* zlib by Mark Adler & Jean-loup Gailly */
#include "zlib.h"
#endif

#ifdef MNG_INCLUDE_LCMS                /* little cms by Marti Maria */
#undef FAR                             /* possibly defined by zlib */
#include "lcms.h"
#endif /* MNG_INCLUDE_LCMS */

#ifdef MNG_INCLUDE_IJG6B               /* IJG's jpgsrc6b */
#include <stdio.h>
#ifdef MNG_USE_SETJMP
#include <setjmp.h>                    /* needed for error-recovery (blergh) */
#else
#ifdef WIN32
#define USE_WINDOWS_MESSAGEBOX         /* display a messagebox under Windoze */
#endif
#endif /* MNG_USE_SETJMP */
#undef FAR                             /* possibly defined by zlib or lcms */
#include "jpeglib.h"                   /* all that for JPEG support  :-) */
#endif /* MNG_INCLUDE_IJG6B */

#ifdef MNG_INTERNAL_MEMMNGMT
#include <stdlib.h>                    /* "calloc" & "free" */
#endif

#ifdef WIN32
/* B003 */
#if defined __BORLANDC__
#include <mem.h>                       /* defines "memcpy" for BCB */
#else
#include <memory.h>                    /* defines "memcpy" for other win32 platforms */
#endif
/* B003 */
#else
#ifdef BSD
#include <strings.h>                   /* defines "memcpy" for BSD (?) */
#else
#include <string.h>                    /* defines "memcpy" for all others (???) */
#endif
#endif

#if defined(MNG_FULL_CMS) || defined(MNG_GAMMA_ONLY)
#include <math.h>                      /* fp gamma-calculation */
#endif

/* ************************************************************************** */
/* *                                                                        * */
/* * Platform-dependant stuff                                               * */
/* *                                                                        * */
/* ************************************************************************** */

/* TODO: this may require some elaboration for other platforms;
   only works with BCB for now */

#ifndef MNG_DLL
#if defined(MNG_BUILD_DLL) || defined(MNG_USE_DLL)
#define MNG_DLL
#endif
#endif

#if defined(MNG_DLL) && defined(WIN32) /* setup DLL calling conventions */ 
#define MNG_DECL __stdcall
#if defined(MNG_BUILD_DLL)
#define MNG_EXT __declspec(dllexport)
#elif defined(MNG_USE_DLL)
#define MNG_EXT __declspec(dllimport)
#else
#define MNG_EXT
#endif
#ifdef MNG_STRICT_ANSI
#undef MNG_STRICT_ANSI                 /* can't do strict-ANSI with this DLL-stuff */
#endif
#else
#define MNG_DECL                       /* dummies for non-DLL */
#define MNG_EXT
#endif /* MNG_DLL && WIN32 */

#if defined(__BORLANDC__) && defined(MNG_STRICT_ANSI)
#pragma option -A                      /* now force ANSI-C from here on */
#endif

/* ************************************************************************** */

typedef signed   long  mng_int32;                /* basic integers */
typedef unsigned long  mng_uint32;
typedef signed   short mng_int16;
typedef unsigned short mng_uint16;
typedef signed   char  mng_int8;
typedef unsigned char  mng_uint8;

typedef double         mng_float;                /* basic float */

typedef char *         mng_pchar;                /* string */
typedef void *         mng_ptr;                  /* generic pointer */

/* ************************************************************************** */
/* *                                                                        * */
/* * Platform-independant from here                                         * */
/* *                                                                        * */
/* ************************************************************************** */

typedef mng_uint32 *   mng_uint32p;              /* pointer to unsigned longs */
typedef mng_int32 *    mng_int32p;               /* pointer to longs */
typedef mng_uint16 *   mng_uint16p;              /* pointer to unsigned words */
typedef mng_uint8 *    mng_uint8p;               /* pointer to unsigned bytes */

typedef mng_int8       mng_bool;                 /* booleans */

typedef mng_int32      mng_handle;               /* generic handle */
typedef mng_int32      mng_retcode;              /* generic return code */
typedef mng_int32      mng_chunkid;              /* 4-byte chunkname identifier */
typedef mng_ptr        mng_chunkp;               /* pointer to a chunk-structure */
typedef mng_ptr        mng_objectp;              /* pointer to an object-structure */

typedef mng_chunkid *  mng_chunkidp;             /* pointer to chunkid */

typedef struct {                                 /* 8-bit palette element */
          mng_uint8 iRed;
          mng_uint8 iGreen;
          mng_uint8 iBlue;
        } mng_palette8e;
typedef mng_palette8e mng_palette8[256];         /* 8-bit palette */

typedef mng_uint8     mng_uint8arr[256];         /* generic arrays */
typedef mng_uint8     mng_uint8arr4[4];
typedef mng_uint16    mng_uint16arr[256];
typedef mng_uint32    mng_uint32arr2[2];

/* ************************************************************************** */

#define MNG_FALSE 0
#define MNG_TRUE  1
#define MNG_NULL  0

/* ************************************************************************** */

#ifdef MNG_INCLUDE_ZLIB

/* size of temporary zlib buffer for deflate processing */
#define MNG_ZLIB_MAXBUF     8192

/* default zlib compression parameters for deflateinit2 */
#define MNG_ZLIB_LEVEL      9                    /* level */
#define MNG_ZLIB_METHOD     Z_DEFLATED           /* method */
#define MNG_ZLIB_WINDOWBITS 15                   /* window size */
#define MNG_ZLIB_MEMLEVEL   9                    /* memory level */
#define MNG_ZLIB_STRATEGY   Z_DEFAULT_STRATEGY   /* strategy */

#define MNG_MAX_IDAT_SIZE   4096                 /* maximum size of IDAT data */

#endif /* MNG_INCLUDE_ZLIB */

/* ************************************************************************** */

#ifdef MNG_INCLUDE_JNG

#ifdef MNG_INCLUDE_IJG6B                         /* IJG helper defs */
typedef struct jpeg_compress_struct   mngjpeg_comp;
typedef struct jpeg_decompress_struct mngjpeg_decomp;
typedef struct jpeg_error_mgr         mngjpeg_error;
typedef struct jpeg_source_mgr        mngjpeg_source;

typedef mngjpeg_comp   * mngjpeg_compp;
typedef mngjpeg_decomp * mngjpeg_decompp;
typedef mngjpeg_error  * mngjpeg_errorp;
typedef mngjpeg_source * mngjpeg_sourcep;

typedef J_DCT_METHOD     mngjpeg_dctmethod;

/* default IJG parameters for compression */
#define MNG_JPEG_DCT         JDCT_DEFAULT        /* DCT algorithm (JDCT_ISLOW) */
#define MNG_JPEG_QUALITY     100                 /* quality 0..100; 100=best */
#define MNG_JPEG_SMOOTHING   0                   /* default no smoothing */
#define MNG_JPEG_PROGRESSIVE MNG_FALSE           /* default is just baseline */
#define MNG_JPEG_OPTIMIZED   MNG_FALSE           /* default is not optimized */
#endif /* MNG_INCLUDE_IJG6B */

#define MNG_JPEG_MAXBUF      65500               /* max size of temp JPEG buffer */
#define MNG_MAX_JDAT_SIZE    4096                /* maximum size of JDAT data */

#endif /* MNG_INCLUDE_JNG */

/* ************************************************************************** */

#ifdef MNG_INCLUDE_LCMS
typedef cmsHPROFILE         mng_cmsprof;         /* little CMS helper defs */
typedef cmsHTRANSFORM       mng_cmstrans;
typedef cmsCIExyY           mng_CIExyY;
typedef cmsCIExyYTRIPLE     mng_CIExyYTRIPLE;
typedef LPGAMMATABLE        mng_gammatabp;
#endif /* MNG_INCLUDE_LCMS */

/* ************************************************************************** */

                                       /* enumeration of known graphics types */
enum mng_imgtypes {mng_it_unknown, mng_it_png, mng_it_mng, mng_it_jng};
typedef enum mng_imgtypes mng_imgtype;

                                       /* enumeration of animation speed-types */
enum mng_speedtypes {mng_st_normal, mng_st_fast, mng_st_slow, mng_st_slowest};
typedef enum mng_speedtypes mng_speedtype;

/* ************************************************************************** */

                                       /* memory management callbacks */
typedef mng_ptr    MNG_DECL (*mng_memalloc)      (mng_uint32  iLen);
typedef void       MNG_DECL (*mng_memfree)       (mng_ptr     iPtr,
                                                  mng_uint32  iLen);

                                       /* I/O management callbacks */
typedef mng_bool   MNG_DECL (*mng_openstream)    (mng_handle  hHandle);
typedef mng_bool   MNG_DECL (*mng_closestream)   (mng_handle  hHandle);
typedef mng_bool   MNG_DECL (*mng_readdata)      (mng_handle  hHandle,
                                                  mng_ptr     pBuf,
                                                  mng_uint32  iBuflen,
                                                  mng_uint32p pRead);
typedef mng_bool   MNG_DECL (*mng_writedata)     (mng_handle  hHandle,
                                                  mng_ptr     pBuf,
                                                  mng_uint32  iBuflen,
                                                  mng_uint32p pWritten);

                                       /* error & trace processing callbacks */
typedef mng_bool   MNG_DECL (*mng_errorproc)     (mng_handle  hHandle,
                                                  mng_int32   iErrorcode,
                                                  mng_int8    iSeverity,
                                                  mng_chunkid iChunkname,
                                                  mng_uint32  iChunkseq,
                                                  mng_int32   iExtra1,
                                                  mng_int32   iExtra2,
                                                  mng_pchar   zErrortext);
typedef mng_bool   MNG_DECL (*mng_traceproc)     (mng_handle  hHandle,
                                                  mng_int32   iFuncnr,
                                                  mng_int32   iFuncseq,
                                                  mng_pchar   zFuncname);

                                       /* read processing callbacks */
typedef mng_bool   MNG_DECL (*mng_processheader) (mng_handle  hHandle,
                                                  mng_uint32  iWidth,
                                                  mng_uint32  iHeight);
typedef mng_bool   MNG_DECL (*mng_processtext)   (mng_handle  hHandle,
                                                  mng_uint8   iType,
                                                  mng_pchar   zKeyword,
                                                  mng_pchar   zText,
                                                  mng_pchar   zLanguage,
                                                  mng_pchar   zTranslation);

                                       /* display processing callbacks */
typedef mng_ptr    MNG_DECL (*mng_getcanvasline) (mng_handle  hHandle,
                                                  mng_uint32  iLinenr);
typedef mng_ptr    MNG_DECL (*mng_getbkgdline)   (mng_handle  hHandle,
                                                  mng_uint32  iLinenr);
typedef mng_ptr    MNG_DECL (*mng_getalphaline)  (mng_handle  hHandle,
                                                  mng_uint32  iLinenr);
typedef mng_bool   MNG_DECL (*mng_refresh)       (mng_handle  hHandle,
                                                  mng_uint32  iTop,
                                                  mng_uint32  iLeft,
                                                  mng_uint32  iBottom,
                                                  mng_uint32  iRight);

                                       /* timer management callbacks */
typedef mng_uint32 MNG_DECL (*mng_gettickcount)  (mng_handle  hHandle);
typedef mng_bool   MNG_DECL (*mng_settimer)      (mng_handle  hHandle,
                                                  mng_uint32  iMsecs);

                                       /* color management callbacks */
typedef mng_bool   MNG_DECL (*mng_processgamma)  (mng_handle  hHandle,
                                                  mng_uint32  iGamma);
typedef mng_bool   MNG_DECL (*mng_processchroma) (mng_handle  hHandle,
                                                  mng_uint32  iWhitepointx,
                                                  mng_uint32  iWhitepointy,
                                                  mng_uint32  iRedx,
                                                  mng_uint32  iRedy,
                                                  mng_uint32  iGreenx,
                                                  mng_uint32  iGreeny,
                                                  mng_uint32  iBluex,
                                                  mng_uint32  iBluey);
typedef mng_bool   MNG_DECL (*mng_processsrgb)   (mng_handle  hHandle,
                                                  mng_uint8   iRenderingintent);
typedef mng_bool   MNG_DECL (*mng_processiccp)   (mng_handle  hHandle,
                                                  mng_uint32  iProfilesize,
                                                  mng_ptr     pProfile);
typedef mng_bool   MNG_DECL (*mng_processarow)   (mng_handle  hHandle,
                                                  mng_uint32  iRowsamples,
                                                  mng_bool    bIsRGBA16,
                                                  mng_ptr     pRow);

                                       /* chunk access callback(s) */
typedef mng_bool   MNG_DECL (*mng_iteratechunk)  (mng_handle  hHandle,
                                                  mng_handle  hChunk,
                                                  mng_chunkid iChunkid,
                                                  mng_uint32  iChunkseq);

/* ************************************************************************** */

#endif /* _mng_types_h_ */

/* ************************************************************************** */
/* * end of file                                                            * */
/* ************************************************************************** */

