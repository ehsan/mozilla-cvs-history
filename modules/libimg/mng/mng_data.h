/* ************************************************************************** */
/* *             For conditions of distribution and use,                    * */
/* *                see copyright notice in libmng.h                        * */
/* ************************************************************************** */
/* *                                                                        * */
/* * project   : libmng                                                     * */
/* * file      : mng_data.h                copyright (c) 2000 G.Juyn        * */
/* * version   : 0.5.3                                                      * */
/* *                                                                        * */
/* * purpose   : main data structure definition                             * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* * web       : http://www.3-t.com                                         * */
/* * email     : mailto:info@3-t.com                                        * */
/* *                                                                        * */
/* * comment   : Definition of the library main data structure              * */
/* *                                                                        * */
/* * changes   : 0.5.1 - 05/04/2000 - G.Juyn                                * */
/* *             - added CRC table to main structure (for thread-safety)    * */
/* *             0.5.1 - 05/06/2000 - G.Juyn                                * */
/* *             - added iPLTEentries for checking hIST-length              * */
/* *             0.5.1 - 05/08/2000 - G.Juyn                                * */
/* *             - changed palette definition to exported palette-type      * */
/* *             - removed frozen indicator                                 * */
/* *             - added create/write indicators                            * */
/* *             - changed strict-ANSI stuff                                * */
/* *             0.5.1 - 05/13/2000 - G.Juyn                                * */
/* *             - added eMNGma hack (will be removed in 1.0.0 !!!)         * */
/* *             - added TERM animation object pointer (easier reference)   * */
/* *             - added saved-data structure for SAVE/SEEK processing      * */
/* *                                                                        * */
/* *             0.5.2 - 05/18/2000 - G.Juyn                                * */
/* *             - added fields for JNG support (IJG-based)                 * */
/* *             0.5.2 - 05/24/2000 - G.Juyn                                * */
/* *             - changed global tRNS definition                           * */
/* *             0.5.2 - 05/30/2000 - G.Juyn                                * */
/* *             - added delta-image fields                                 * */
/* *             0.5.2 - 06/01/2000 - G.Juyn                                * */
/* *             - added internal delta-image processing callbacks          * */
/* *             0.5.2 - 06/02/2000 - G.Juyn                                * */
/* *             - changed SWAP_ENDIAN to BIGENDIAN_SUPPORTED               * */
/* *               (contributed by Tim Rowley)                              * */
/* *             - added getalphaline callback for RGB8_A8 canvasstyle      * */
/* *             0.5.2 - 06/06/2000 - G.Juyn                                * */
/* *             - added parameter for delayed buffer-processing            * */
/* *                                                                        * */
/* *             0.5.3 - 06/16/2000 - G.Juyn                                * */
/* *             - added update-region parms for refresh calback            * */
/* *             - added Needrefresh parameter                              * */
/* *             0.5.3 - 06/17/2000 - G.Juyn                                * */
/* *             - added Deltaimmediate parm for faster delta-processing    * */
/* *             0.5.3 - 06/21/2000 - G.Juyn                                * */
/* *             - added Speed parameter to facilitate testing              * */
/* *             - added Imagelevel parameter for processtext callback      * */
/* *                                                                        * */
/* ************************************************************************** */

#if defined(__BORLANDC__) && defined(MNG_STRICT_ANSI)
#pragma option -A                      /* force ANSI-C */
#endif

#ifndef _mng_data_h_
#define _mng_data_h_

/* ************************************************************************** */

#define MNG_MAGIC 0x52530a0aL

/* ************************************************************************** */
/* *                                                                        * */
/* * Internal structures                                                    * */
/* *                                                                        * */
/* ************************************************************************** */

typedef mng_palette8 mng_rgbpaltab;

/* ************************************************************************** */
/* *                                                                        * */
/* * The saved_data structure                                               * */
/* *                                                                        * */
/* * This contains the saved data after a SAVE chunk has been processed.    * */
/* * The data is saved from the main data structure during SAVE processing, * */
/* * and restored to the main data structure during SEEK processing.        * */
/* *                                                                        * */
/* ************************************************************************** */

typedef struct mng_savedata_struct {

#if defined(MNG_SUPPORT_READ) || defined(MNG_SUPPORT_WRITE)
           mng_bool          bHasglobalPLTE;     /* global PLTE chunk processed */
           mng_bool          bHasglobalTRNS;     /* global tRNS chunk processed */
           mng_bool          bHasglobalGAMA;     /* global gAMA chunk processed */
           mng_bool          bHasglobalCHRM;     /* global cHRM chunk processed */
           mng_bool          bHasglobalSRGB;     /* global sRGB chunk processed */
           mng_bool          bHasglobalICCP;     /* global iCCP chunk processed */
           mng_bool          bHasglobalBKGD;     /* global bKGD chunk processed */
#endif /* MNG_SUPPORT_READ || MNG_SUPPORT_WRITE */

#ifdef MNG_SUPPORT_DISPLAY
           mng_uint16        iDEFIobjectid;      /* DEFI fields */
           mng_uint8         iDEFIdonotshow;
           mng_uint8         iDEFIconcrete;
           mng_bool          bDEFIhasloca;
           mng_int32         iDEFIlocax;
           mng_int32         iDEFIlocay;
           mng_bool          bDEFIhasclip;
           mng_int32         iDEFIclipl;
           mng_int32         iDEFIclipr;
           mng_int32         iDEFIclipt;
           mng_int32         iDEFIclipb;

           mng_uint16        iBACKred;           /* BACK fields */
           mng_uint16        iBACKgreen;
           mng_uint16        iBACKblue;
           mng_uint8         iBACKmandatory;
           mng_uint16        iBACKimageid;
           mng_uint8         iBACKtile;

           mng_uint8         iFRAMmode;          /* FRAM fields (global) */
           mng_uint32        iFRAMdelay;
           mng_uint32        iFRAMtimeout;
           mng_bool          bFRAMclipping;
           mng_int32         iFRAMclipl;
           mng_int32         iFRAMclipr;
           mng_int32         iFRAMclipt;
           mng_int32         iFRAMclipb;

           mng_uint32        iGlobalPLTEcount;   /* global PLTE fields */
           mng_rgbpaltab     aGlobalPLTEentries;

           mng_uint32        iGlobalTRNSrawlen;  /* global tRNS fields */
           mng_uint8arr      aGlobalTRNSrawdata;

           mng_uint32        iGlobalGamma;       /* global gAMA fields */

           mng_uint32        iGlobalWhitepointx; /* global cHRM fields */
           mng_uint32        iGlobalWhitepointy;
           mng_uint32        iGlobalPrimaryredx;
           mng_uint32        iGlobalPrimaryredy;
           mng_uint32        iGlobalPrimarygreenx;
           mng_uint32        iGlobalPrimarygreeny;
           mng_uint32        iGlobalPrimarybluex;
           mng_uint32        iGlobalPrimarybluey;

           mng_uint8         iGlobalRendintent;  /* global sRGB fields */

           mng_uint32        iGlobalProfilesize; /* global iCCP fields */
           mng_ptr           pGlobalProfile;

           mng_uint16        iGlobalBKGDred;     /* global bKGD fields */
           mng_uint16        iGlobalBKGDgreen;
           mng_uint16        iGlobalBKGDblue;
#endif /* MNG_SUPPORT_DISPLAY */

        } mng_savedata;

typedef mng_savedata * mng_savedatap;

/* ************************************************************************** */
/* *                                                                        * */
/* * The main libmng data structure                                         * */
/* *                                                                        * */
/* * The handle used in all functions points to this structure which        * */
/* * contains all volatile data necessary to process the network graphic.   * */
/* *                                                                        * */
/* ************************************************************************** */

typedef struct mng_data_struct {

           mng_uint32        iMagic;             /* magic number to validate
                                                    a given handle */     
           mng_int32         iUserdata;          /* application workdata */

           mng_imgtype       eSigtype;           /* image information */
           mng_imgtype       eImagetype;         /* initially zeroed */
           mng_uint32        iWidth;             /* filled after header is processed */
           mng_uint32        iHeight;
           mng_uint32        iTicks;             /* these only after MHDR */
           mng_uint32        iLayercount;
           mng_uint32        iFramecount;
           mng_uint32        iPlaytime;
           mng_uint32        iSimplicity;

           mng_uint32        iImagelevel;        /* level an image inside a stream */

           mng_uint32        iCanvasstyle;       /* layout of the drawing-canvas */
           mng_uint32        iBkgdstyle;         /* layout of the background-canvas */

           mng_int8          iMagnify;           /* magnification factor (not used yet) */
           mng_uint32        iOffsetx;           /* x-offset for extremely large image */
           mng_uint32        iOffsety;           /* y-offset for extremely large image */
           mng_uint32        iCanvaswidth;       /* real canvas size */
           mng_uint32        iCanvasheight;      /* must be set by processheader callback */

           mng_uint16        iBGred;             /* default background color */
           mng_uint16        iBGgreen;           /* initially "black" */
           mng_uint16        iBGblue;

           mng_bool          bIssRGB;            /* indicates sRGB system */

#ifdef MNG_FULL_CMS                              /* little CMS variables */
           mng_cmsprof       hProf1;             /* image input profile */
           mng_cmsprof       hProf2;             /* default output profile */
           mng_cmsprof       hProf3;             /* default sRGB profile */
           mng_cmstrans      hTrans;             /* current transformation handle */
#endif

           mng_float         dViewgamma;         /* gamma calculation variables */
           mng_float         dDisplaygamma;      /* initially set for sRGB conditions */
           mng_float         dDfltimggamma;

           mng_bool          bStorechunks;       /* switch for storing chunkdata */

           mng_speedtype     iSpeed;             /* speed-modifier for animations */

           mng_uint32        iMaxwidth;          /* maximum canvas size */
           mng_uint32        iMaxheight;         /* initially set to 1024 x 1024 */

           mng_int32         iErrorcode;         /* error reporting fields */
           mng_int8          iSeverity;
           mng_int32         iErrorx1;
           mng_int32         iErrorx2;
           mng_pchar         zErrortext;

           mng_memalloc      fMemalloc;          /* callback pointers */
           mng_memfree       fMemfree;           /* initially nulled */
           mng_openstream    fOpenstream;
           mng_closestream   fClosestream;
           mng_readdata      fReaddata;
           mng_writedata     fWritedata;
           mng_errorproc     fErrorproc;
           mng_traceproc     fTraceproc;
           mng_processheader fProcessheader;
           mng_processtext   fProcesstext;
           mng_getcanvasline fGetcanvasline;
           mng_getbkgdline   fGetbkgdline;
           mng_getalphaline  fGetalphaline;
           mng_refresh       fRefresh;
           mng_gettickcount  fGettickcount;
           mng_settimer      fSettimer;
           mng_processgamma  fProcessgamma;
           mng_processchroma fProcesschroma;
           mng_processsrgb   fProcesssrgb;
           mng_processiccp   fProcessiccp;
           mng_processarow   fProcessarow;

#if defined(MNG_SUPPORT_READ) || defined(MNG_SUPPORT_WRITE)
           mng_bool          bPreDraft48;        /* flags ancient style draft */

           mng_chunkid       iChunkname;         /* read/write-state variables */
           mng_uint32        iChunkseq;
           mng_chunkp        pFirstchunk;        /* double-linked list of */
           mng_chunkp        pLastchunk;         /* stored chunk-structures */

           mng_bool          bHasheader;         /* first header chunk processed */
           mng_bool          bHasMHDR;           /* inside a MHDR-MEND sequence */
           mng_bool          bHasIHDR;           /* inside a IHDR-IEND sequence */
           mng_bool          bHasBASI;           /* inside a BASI-IEND sequence */
           mng_bool          bHasDHDR;           /* inside a DHDR-IEND sequence */
#ifdef MNG_INCLUDE_JNG
           mng_bool          bHasJHDR;           /* inside a JHDR-IEND sequence */
           mng_bool          bHasJSEP;           /* passed the JSEP separator */
           mng_bool          bHasJDAT;           /* at least 1 JDAT processed */
#endif
           mng_bool          bHasPLTE;           /* PLTE chunk processed */
           mng_bool          bHasTRNS;           /* tRNS chunk processed */
           mng_bool          bHasGAMA;           /* gAMA chunk processed */
           mng_bool          bHasCHRM;           /* cHRM chunk processed */
           mng_bool          bHasSRGB;           /* sRGB chunk processed */
           mng_bool          bHasICCP;           /* iCCP chunk processed */
           mng_bool          bHasBKGD;           /* bKGD chunk processed */
           mng_bool          bHasIDAT;           /* at least 1 IDAT processed */
           
           mng_bool          bHasSAVE;           /* SAVE chunk processed */
           mng_bool          bHasBACK;           /* BACK chunk processed */
           mng_bool          bHasFRAM;           /* FRAM chunk processed */
           mng_bool          bHasTERM;           /* TERM chunk processed */
           mng_bool          bHasLOOP;           /* at least 1 LOOP open */

           mng_bool          bHasglobalPLTE;     /* global PLTE chunk processed */
           mng_bool          bHasglobalTRNS;     /* global tRNS chunk processed */
           mng_bool          bHasglobalGAMA;     /* global gAMA chunk processed */
           mng_bool          bHasglobalCHRM;     /* global cHRM chunk processed */
           mng_bool          bHasglobalSRGB;     /* global sRGB chunk processed */
           mng_bool          bHasglobalICCP;     /* global iCCP chunk processed */
           mng_bool          bHasglobalBKGD;     /* global bKGD chunk processed */

           mng_uint32        iDatawidth;         /* IHDR/BASI/DHDR fields */
           mng_uint32        iDataheight;        /* valid if inside IHDR-IEND, */
           mng_uint8         iBitdepth;          /* BASI-IEND or DHDR-IEND */
           mng_uint8         iColortype;
           mng_uint8         iCompression;
           mng_uint8         iFilter;
           mng_uint8         iInterlace;

           mng_uint32        iPLTEcount;         /* PLTE fields */

           mng_bool          bEMNGMAhack;        /* TODO: to be removed in 1.0.0 !!! */

#ifdef MNG_INCLUDE_JNG
           mng_uint8         iJHDRcolortype;     /* JHDR fields */
           mng_uint8         iJHDRimgbitdepth;   /* valid if inside JHDR-IEND */
           mng_uint8         iJHDRimgcompression;
           mng_uint8         iJHDRimginterlace;
           mng_uint8         iJHDRalphabitdepth;
           mng_uint8         iJHDRalphacompression;
           mng_uint8         iJHDRalphafilter;
           mng_uint8         iJHDRalphainterlace;
#endif

#endif /* MNG_SUPPORT_READ || MNG_SUPPORT_WRITE */

#ifdef MNG_SUPPORT_READ
           mng_bool          bReading;           /* read processing variables */
           mng_bool          bHavesig;
           mng_bool          bEOF;
           mng_uint32        iReadbufsize;
           mng_uint8p        pReadbuf;
#endif /* MNG_SUPPORT_READ */

#ifdef MNG_SUPPORT_WRITE
           mng_bool          bCreating;          /* create/write processing variables */
           mng_bool          bWriting;
           mng_chunkid       iFirstchunkadded;           
           mng_uint32        iWritebufsize;
           mng_uint8p        pWritebuf;
#endif

#ifdef MNG_SUPPORT_DISPLAY
           mng_bool          bDisplaying;        /* display-state variables */
           mng_uint32        iFrameseq;
           mng_uint32        iLayerseq;
           mng_uint32        iFrametime;         /* millisecs */

           mng_uint32        iRuntime;           /* millisecs since start */
           mng_uint32        iStarttime;         /* tickcount at start */
           mng_uint32        iEndtime;           /* tickcount at end */
           mng_bool          bRunning;           /* animation is active */
           mng_bool          bTimerset;          /* the timer has been set;
                                                    we're expecting a call to
                                                    mng_display_resume! */
           mng_uint8         iBreakpoint;        /* indicates at which point the
                                                    flow was broken to run the timer */
           mng_bool          bSuspended;         /* input-reading has been suspended;
                                                    we're expecting a call to
                                                    mng_read_resume! */
           mng_uint8         iSuspendpoint;      /* indicates at which point the flow
                                                    was broken to suspend input-reading */
           mng_bool          bNeedrefresh;       /* indicates screen-refresh is needed */   
           mng_objectp       pCurrentobj;        /* current "object" */
           mng_objectp       pCurraniobj;        /* current animation object
                                                    "to be"/"being" processed */
           mng_objectp       pTermaniobj;        /* TERM animation object */
           mng_objectp       pObjzero;           /* "on-the-fly" image (object = 0) */
           mng_objectp       pLastclone;         /* last clone */
           mng_objectp       pStoreobj;          /* current store object for row routines */
           mng_objectp       pStorebuf;          /* current store object-buffer for row routines */
           mng_objectp       pRetrieveobj;       /* current retrieve object for row routines */
           mng_savedatap     pSavedata;          /* pointer to saved data (after SAVE) */

           mng_uint32        iUpdateleft;        /* update region for refresh */
           mng_uint32        iUpdateright;
           mng_uint32        iUpdatetop;
           mng_uint32        iUpdatebottom;

           mng_int8          iPass;              /* current interlacing pass;
                                                    negative value means no interlace */
           mng_int32         iRow;               /* current row counter */
           mng_int32         iRowinc;            /* row increment for this pass */
           mng_int32         iCol;               /* current starting column */
           mng_int32         iColinc;            /* column increment for this pass */
           mng_int32         iRowsamples;        /* nr. of samples in current workrow */
           mng_int32         iSamplemul;         /* needed to calculate rowsize */
           mng_int32         iSampleofs;            /* from rowsamples */
           mng_int32         iSamplediv;
           mng_int32         iRowsize;           /* size of actual data in work row */
           mng_int32         iRowmax;            /* maximum size of data in work row */
           mng_uint8p        pWorkrow;           /* working row of pixel-data */
           mng_uint8p        pPrevrow;           /* previous row of pixel-data */
           mng_uint8p        pRGBArow;           /* intermediate row of RGBA8 or RGBA16 data */
           mng_bool          bIsRGBA16;          /* indicates intermediate row is RGBA16 */
           mng_bool          bIsOpaque;          /* indicates intermediate row is fully opaque */
           mng_int32         iFilterbpp;         /* bpp index for filtering routines */

           mng_int32         iSourcel;           /* variables for showing objects */
           mng_int32         iSourcer;
           mng_int32         iSourcet;
           mng_int32         iSourceb;
           mng_int32         iDestl;
           mng_int32         iDestr;
           mng_int32         iDestt;
           mng_int32         iDestb;

           mng_objectp       pFirstimgobj;       /* double-linked list of */
           mng_objectp       pLastimgobj;        /* image-object structures */
           mng_objectp       pFirstaniobj;       /* double-linked list of */
           mng_objectp       pLastaniobj;        /* animation-object structures */

#if defined(MNG_GAMMA_ONLY) || defined(MNG_FULL_CMS)
           mng_uint8         aGammatab[256];     /* precomputed gamma lookup table */
           mng_float         dLastgamma;         /* last gamma used to compute table */
#endif

           mng_ptr           fDisplayrow;        /* internal callback to display an
                                                    uncompressed/unfiltered/
                                                    color-corrected row */
           mng_ptr           fRestbkgdrow;       /* internal callback for restore-
                                                    background processing of a row */
           mng_ptr           fCorrectrow;        /* internal callback to color-correct an
                                                    uncompressed/unfiltered row */
           mng_ptr           fRetrieverow;       /* internal callback to retrieve an
                                                    uncompressed/unfiltered row of data */
           mng_ptr           fStorerow;          /* internal callback to store an
                                                    uncompressed/unfiltered row of data */
           mng_ptr           fProcessrow;        /* internal callback to process an
                                                    uncompressed row of data */
           mng_ptr           fInitrowproc;       /* internal callback to initialize
                                                    the row processing */

           mng_uint16        iDEFIobjectid;      /* DEFI fields */
           mng_uint8         iDEFIdonotshow;
           mng_uint8         iDEFIconcrete;
           mng_bool          bDEFIhasloca;
           mng_int32         iDEFIlocax;
           mng_int32         iDEFIlocay;
           mng_bool          bDEFIhasclip;
           mng_int32         iDEFIclipl;
           mng_int32         iDEFIclipr;
           mng_int32         iDEFIclipt;
           mng_int32         iDEFIclipb;

           mng_uint16        iBACKred;           /* BACK fields */
           mng_uint16        iBACKgreen;
           mng_uint16        iBACKblue;
           mng_uint8         iBACKmandatory;
           mng_uint16        iBACKimageid;
           mng_uint8         iBACKtile;

           mng_bool          bHasnextBACK;       /* next frame BACK fields */
           mng_uint16        iNextBACKred;
           mng_uint16        iNextBACKgreen;
           mng_uint16        iNextBACKblue;
           mng_uint8         iNextBACKmandatory;
           mng_uint16        iNextBACKimageid;
           mng_uint8         iNextBACKtile;

           mng_uint8         iFRAMmode;          /* FRAM fields (global) */
           mng_uint32        iFRAMdelay;
           mng_uint32        iFRAMtimeout;
           mng_bool          bFRAMclipping;
           mng_int32         iFRAMclipl;
           mng_int32         iFRAMclipr;
           mng_int32         iFRAMclipt;
           mng_int32         iFRAMclipb;

           mng_uint8         iFramemode;         /* current subframe variables */
           mng_uint32        iFramedelay;
           mng_uint32        iFrametimeout;
           mng_bool          bFrameclipping;
           mng_int32         iFrameclipl;
           mng_int32         iFrameclipr;
           mng_int32         iFrameclipt;
           mng_int32         iFrameclipb;

           mng_uint32        iNextdelay;         /* delay for *after* next image */

           mng_uint8         iSHOWmode;          /* SAVE fields */
           mng_uint16        iSHOWfromid;
           mng_uint16        iSHOWtoid;
           mng_uint16        iSHOWnextid;
           mng_int16         iSHOWskip;

           mng_uint32        iGlobalPLTEcount;   /* global PLTE fields */
           mng_rgbpaltab     aGlobalPLTEentries;

           mng_uint32        iGlobalTRNSrawlen;  /* global tRNS fields */
           mng_uint8arr      aGlobalTRNSrawdata;

           mng_uint32        iGlobalGamma;       /* global gAMA fields */

           mng_uint32        iGlobalWhitepointx; /* global cHRM fields */
           mng_uint32        iGlobalWhitepointy;
           mng_uint32        iGlobalPrimaryredx;
           mng_uint32        iGlobalPrimaryredy;
           mng_uint32        iGlobalPrimarygreenx;
           mng_uint32        iGlobalPrimarygreeny;
           mng_uint32        iGlobalPrimarybluex;
           mng_uint32        iGlobalPrimarybluey;

           mng_uint8         iGlobalRendintent;  /* global sRGB fields */

           mng_uint32        iGlobalProfilesize; /* global iCCP fields */
           mng_ptr           pGlobalProfile;

           mng_uint16        iGlobalBKGDred;     /* global bKGD fields */
           mng_uint16        iGlobalBKGDgreen;
           mng_uint16        iGlobalBKGDblue;

           mng_ptr           pDeltaImage;        /* delta-image fields */
           mng_uint8         iDeltaImagetype;
           mng_uint8         iDeltatype;
           mng_uint32        iDeltaBlockwidth;
           mng_uint32        iDeltaBlockheight;
           mng_uint32        iDeltaBlockx;
           mng_uint32        iDeltaBlocky;
           mng_bool          bDeltaimmediate;

           mng_ptr           fDeltagetrow;       /* internal delta-proc callbacks */
           mng_ptr           fDeltaaddrow;
           mng_ptr           fDeltareplacerow;
           mng_ptr           fDeltaputrow;

#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_INCLUDE_ZLIB
           z_stream          sZlib;              /* zlib (de)compression variables */

           mng_int32         iZlevel;            /* zlib compression parameters */
           mng_int32         iZmethod;
           mng_int32         iZwindowbits;
           mng_int32         iZmemlevel;
           mng_int32         iZstrategy;

           mng_uint32        iMaxIDAT;           /* maximum size of IDAT data */

           mng_bool          bInflating;         /* indicates "inflate" in progress */
           mng_bool          bDeflating;         /* indicates "deflate" in progress */
#endif /* MNG_INCLUDE_ZLIB */

#ifdef MNG_INCLUDE_JNG
           mngjpeg_dctmethod eJPEGdctmethod;     /* IJG compression variables */
           mng_int32         iJPEGquality;
           mng_int32         iJPEGsmoothing;
           mng_bool          bJPEGcompressprogr;
           mng_bool          bJPEGcompressopt;

           mng_uint32        iMaxJDAT;           /* maximum size of JDAT data */

           mngjpeg_compp     pJPEGcinfo;         /* compression structure */
           mngjpeg_errorp    pJPEGcerr;          /* error-manager compress */
           mngjpeg_decompp   pJPEGdinfo;         /* decompression structure */
           mngjpeg_errorp    pJPEGderr;          /* error-manager decompress */
           mngjpeg_sourcep   pJPEGdsrc;          /* source-manager decompress */

           mng_uint8p        pJPEGbuf;           /* buffer for JPEG (de)compression */
           mng_uint32        iJPEGbufmax;        /* allocated space for buffer */
           mng_uint8p        pJPEGcurrent;       /* current pointer into buffer */
           mng_uint32        iJPEGbufremain;     /* remaining bytes in buffer */
           mng_uint32        iJPEGtoskip;        /* bytes to skip on next input-block */

           mng_uint8p        pJPEGrow;           /* buffer for a JPEG row of samples */
           mng_uint32        iJPEGrowlen;

           mng_bool          bJPEGcompress;      /* indicates "compress" initialized */

           mng_bool          bJPEGdecompress;    /* indicates "decompress" ininitialized */
           mng_bool          bJPEGhasheader;     /* indicates "readheader" succeeded */
           mng_bool          bJPEGdecostarted;   /* indicates "decompress" started */
           mng_bool          bJPEGscanstarted;   /* indicates "first scan" started */
           mng_bool          bJPEGprogressive;   /* indicates a progressive image */

           mng_ptr           fStorerow2;         /* internal callback to store an
                                                    uncompressed/unfiltered row of JPEG-data */
           mng_uint32        iJPEGrow;           /* row-number for current JPEG row */
           mng_uint32        iJPEGalpharow;      /* nr. of rows filled with alpha */
           mng_uint32        iJPEGrgbrow;        /* nr. of rows filled with 'color'-info */
           mng_uint32        iJPEGdisprow;       /* nr. of rows already displayed "on-the-fly" */

#if defined(MNG_USE_SETJMP) && defined (MNG_INCLUDE_IJG6B)
           jmp_buf           sErrorbuf;          /* setjmp/longjmp buffer (error-recovery) */
#endif
           
#endif /* MNG_INCLUDE_JNG */

           mng_uint32        aCRCtable [256];    /* CRC prefab table */
           mng_bool          bCRCcomputed;       /* "has been build" indicator */

        } mng_data;

typedef mng_data * mng_datap;

/* ************************************************************************** */
/* *                                                                        * */
/* * Internal Callback-Function prototypes                                  * */
/* *                                                                        * */
/* ************************************************************************** */

typedef mng_retcode(*mng_displayrow)  (mng_datap pData);
typedef mng_retcode(*mng_restbkgdrow) (mng_datap pData);
typedef mng_retcode(*mng_correctrow)  (mng_datap pData);
typedef mng_retcode(*mng_retrieverow) (mng_datap pData);
typedef mng_retcode(*mng_storerow)    (mng_datap pData);
typedef mng_retcode(*mng_processrow)  (mng_datap pData);
typedef mng_retcode(*mng_initrowproc) (mng_datap pData);

/* ************************************************************************** */
/* *                                                                        * */
/* * Routines for swapping byte-order from and to graphic files             * */
/* * (This code is adapted from the libpng package)                         * */
/* *                                                                        * */
/* ************************************************************************** */

#ifndef MNG_BIGENDIAN_SUPPORTED
mng_uint32 mng_get_uint32 (mng_uint8p pBuf);
mng_int32  mng_get_int32  (mng_uint8p pBuf);
mng_uint16 mng_get_uint16 (mng_uint8p pBuf);
void       mng_put_uint32 (mng_uint8p pBuf,
                           mng_uint32 i);
void       mng_put_int32  (mng_uint8p pBuf,
                           mng_int32  i);
void       mng_put_uint16 (mng_uint8p pBuf,
                           mng_uint16 i);
#else /* MNG_BIGENDIAN_SUPPORTED */
#define mng_get_uint32(P)   *(mng_uint32p)(P)
#define mng_get_int32(P)    *(mng_int32p)(P)
#define mng_get_uint16(P)   *(mng_uint16p)(P)
#define mng_put_uint32(P,I) *(mng_uint32p)(P) = (I)
#define mng_put_int32(P,I)  *(mng_int32p)(P) = (I)
#define mng_put_uint16(P,I) *(mng_uint16p)(P) = (I)
#endif /* MNG_BIGENDIAN_SUPPORTED */

/* ************************************************************************** */
/* *                                                                        * */
/* * Some handy(?) macro definitions                                        * */
/* *                                                                        * */
/* ************************************************************************** */

#define MAX_COORD(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN_COORD(a, b)  (((a) < (b)) ? (a) : (b))

/* ************************************************************************** */

#endif /* _mng_data_h_ */

/* ************************************************************************** */
/* * end of file                                                            * */
/* ************************************************************************** */
