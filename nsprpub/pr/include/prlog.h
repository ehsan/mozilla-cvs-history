/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
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

#ifndef prlog_h___
#define prlog_h___

#include "prtypes.h"

PR_BEGIN_EXTERN_C

/*
**
** Define in your environment a NSPR_LOG_MODULES variable. The value of
** this variable has the form:
**
**     <moduleName>:<value>[, <moduleName>:<value>]*
**
** where moduleName is one of named modules that support debugging (see
** the header file for a particular module for more specific
** information).  Value is one of the enum PRLogModuleLevel's legal
** values.
**
** Special modules exist for controlling the logging facility:
**    sync        -- do unbuffered logging
**    bufsize:size    -- use a buffer of "size" bytes
**
** Define in your environment NSPR_LOG_FILE to specify the log file to
** use unless the default of "stderr" is acceptable.
**
** To put log messages in your programs, use the PR_LOG macro:
**
**     PR_LOG(<module>, <level>, (<printfString>, <args>*));
**
** Where <module> is the address of a PRLogModuleInfo structure, and
** <level> is one of the following levels:
*/

typedef enum PRLogModuleLevel {
    PR_LOG_NONE = 0,                /* nothing */
    PR_LOG_ALWAYS = 1,              /* always printed */
    PR_LOG_ERROR = 2,               /* error messages */
    PR_LOG_WARNING = 3,             /* warning messages */
    PR_LOG_DEBUG = 4,               /* debug messages */

    PR_LOG_NOTICE = PR_LOG_DEBUG,   /* notice messages */
    PR_LOG_WARN = PR_LOG_WARNING,   /* warning messages */
    PR_LOG_MIN = PR_LOG_DEBUG,      /* minimal debugging messages */
    PR_LOG_MAX = PR_LOG_DEBUG       /* maximal debugging messages */
} PRLogModuleLevel;

/*
** One of these structures is created for each module that uses logging.
**    "name" is the name of the module
**    "level" is the debugging level selected for that module
*/
typedef struct PRLogModuleInfo {
    const char *name;
    PRLogModuleLevel level;
    struct PRLogModuleInfo *next;
} PRLogModuleInfo;

/*
** Create a new log module.
*/
PR_EXTERN(PRLogModuleInfo*) PR_NewLogModule(const char *name);

/*
** Set the file to use for logging. Returns PR_FALSE if the file cannot
** be created
*/
PR_EXTERN(PRBool) PR_SetLogFile(const char *name);

/*
** Set the size of the logging buffer. If "buffer_size" is zero then the
** logging becomes "synchronous" (or unbuffered).
*/
PR_EXTERN(void) PR_SetLogBuffering(PRIntn buffer_size);

/*
** Print a string to the log. "fmt" is a PR_snprintf format type. All
** messages printed to the log are preceeded by the name of the thread
** and a time stamp. Also, the routine provides a missing newline if one
** is not provided.
*/
PR_EXTERN(void) PR_LogPrint(const char *fmt, ...);

/*
** Flush the log to its file.
*/
PR_EXTERN(void) PR_LogFlush(void);

/*
** Windoze 16 can't support a large static string space for all of the
** various debugging strings so logging is not enabled for it.
*/
#if (defined(DEBUG) || defined(FORCE_PR_LOG)) && !defined(WIN16)
#define PR_LOGGING 1

#define PR_LOG_TEST(_module,_level) \
    ((_module)->level >= (_level))

/*
** Log something.
**    "module" is the address of a PRLogModuleInfo structure
**    "level" is the desired logging level
**    "args" is a variable length list of arguments to print, in the following
**       format:  ("printf style format string", ...)
*/
#define PR_LOG(_module,_level,_args)     \
    PR_BEGIN_MACRO             \
      if (PR_LOG_TEST(_module,_level)) { \
      PR_LogPrint _args;         \
      }                     \
    PR_END_MACRO

#else /* (defined(DEBUG) || defined(FORCE_PR_LOG)) && !defined(WIN16) */

#undef PR_LOGGING
#define PR_LOG_TEST(module,level) 0
#define PR_LOG(module,level,args)

#endif /* (defined(DEBUG) || defined(FORCE_PR_LOG)) && !defined(WIN16) */

#ifndef NO_NSPR_10_SUPPORT

#ifdef PR_LOGGING
#define PR_LOG_BEGIN    PR_LOG
#define PR_LOG_END      PR_LOG
#define PR_LOG_DEFINE   PR_NewLogModule
#else
#define PR_LOG_BEGIN(module,level,args)
#define PR_LOG_END(module,level,args)
#define PR_LOG_DEFINE(_name)    NULL
#endif /* PR_LOGGING */

#endif /* NO_NSPR_10_SUPPORT */

#if defined(DEBUG)

PR_EXTERN(void) PR_Assert(const char *s, const char *file, PRIntn ln);
#define PR_ASSERT(_expr) \
    ((_expr)?((void)0):PR_Assert(# _expr,__FILE__,__LINE__))

#define PR_NOT_REACHED(_reasonStr) \
    PR_Assert(_reasonStr,__FILE__,__LINE__)

#else

#define PR_ASSERT(expr) ((void) 0)
#define PR_NOT_REACHED(reasonStr)

#endif /* defined(DEBUG) */

PR_END_EXTERN_C

#endif /* prlog_h___ */
