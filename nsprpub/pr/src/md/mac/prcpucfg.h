/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "NPL"); you may not use this file except in
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

#ifndef nspr_cpucfg___
#define nspr_cpucfg___

#ifndef XP_MAC
#define XP_MAC
#endif

#undef  IS_LITTLE_ENDIAN
#define IS_BIG_ENDIAN 1

#define PR_BYTES_PER_BYTE   1L
#define PR_BYTES_PER_SHORT  2L
#define PR_BYTES_PER_INT    4L
#define PR_BYTES_PER_INT64  8L
#define PR_BYTES_PER_LONG   4L
#define PR_BYTES_PER_FLOAT  4L
#define PR_BYTES_PER_DOUBLE 8L
#define PR_BYTES_PER_WORD   4L
#define PR_BYTES_PER_DWORD  8L

#define PR_BITS_PER_BYTE    8L
#define PR_BITS_PER_SHORT   16L
#define PR_BITS_PER_INT     32L
#define PR_BITS_PER_INT64   64L
#define PR_BITS_PER_LONG    32L
#define PR_BITS_PER_FLOAT   32L
#define PR_BITS_PER_DOUBLE  64L
#define PR_BITS_PER_WORD    32L

#define PR_BITS_PER_BYTE_LOG2   3L
#define PR_BITS_PER_SHORT_LOG2  4L
#define PR_BITS_PER_INT_LOG2    5L
#define PR_BITS_PER_INT64_LOG2  6L
#define PR_BITS_PER_LONG_LOG2   5L
#define PR_BITS_PER_FLOAT_LOG2  5L
#define PR_BITS_PER_DOUBLE_LOG2 6L
#define PR_BITS_PER_WORD_LOG2   5L

#define PR_ALIGN_OF_SHORT   2L
#define PR_ALIGN_OF_INT     4L
#define PR_ALIGN_OF_LONG    4L
#define PR_ALIGN_OF_INT64   2L
#define PR_ALIGN_OF_FLOAT   4L
#define PR_ALIGN_OF_DOUBLE  4L
#define PR_ALIGN_OF_POINTER 4L
#define PR_ALIGN_OF_WORD    4L

#define PR_BYTES_PER_WORD_LOG2   2L
#define PR_BYTES_PER_DWORD_LOG2  3L
#define PR_WORDS_PER_DWORD_LOG2  1L

#define _PR_LOCAL_THREADS_ONLY

#ifndef NO_NSPR_10_SUPPORT
#define BYTES_PER_BYTE		PR_BYTES_PER_BYTE
#define BYTES_PER_SHORT 	PR_BYTES_PER_SHORT
#define BYTES_PER_INT 		PR_BYTES_PER_INT
#define BYTES_PER_INT64		PR_BYTES_PER_INT64
#define BYTES_PER_LONG		PR_BYTES_PER_LONG
#define BYTES_PER_FLOAT		PR_BYTES_PER_FLOAT
#define BYTES_PER_DOUBLE	PR_BYTES_PER_DOUBLE
#define BYTES_PER_WORD		PR_BYTES_PER_WORD
#define BYTES_PER_DWORD		PR_BYTES_PER_DWORD

#define BITS_PER_BYTE		PR_BITS_PER_BYTE
#define BITS_PER_SHORT		PR_BITS_PER_SHORT
#define BITS_PER_INT		PR_BITS_PER_INT
#define BITS_PER_INT64		PR_BITS_PER_INT64
#define BITS_PER_LONG		PR_BITS_PER_LONG
#define BITS_PER_FLOAT		PR_BITS_PER_FLOAT
#define BITS_PER_DOUBLE		PR_BITS_PER_DOUBLE
#define BITS_PER_WORD		PR_BITS_PER_WORD

#define BITS_PER_BYTE_LOG2	PR_BITS_PER_BYTE_LOG2
#define BITS_PER_SHORT_LOG2	PR_BITS_PER_SHORT_LOG2
#define BITS_PER_INT_LOG2	PR_BITS_PER_INT_LOG2
#define BITS_PER_INT64_LOG2	PR_BITS_PER_INT64_LOG2
#define BITS_PER_LONG_LOG2	PR_BITS_PER_LONG_LOG2
#define BITS_PER_FLOAT_LOG2	PR_BITS_PER_FLOAT_LOG2
#define BITS_PER_DOUBLE_LOG2 	PR_BITS_PER_DOUBLE_LOG2
#define BITS_PER_WORD_LOG2		PR_BITS_PER_WORD_LOG2

#define ALIGN_OF_SHORT		PR_ALIGN_OF_SHORT
#define ALIGN_OF_INT		PR_ALIGN_OF_INT
#define ALIGN_OF_LONG		PR_ALIGN_OF_LONG
#define ALIGN_OF_INT64		PR_ALIGN_OF_INT64
#define ALIGN_OF_FLOAT		PR_ALIGN_OF_FLOAT
#define ALIGN_OF_DOUBLE		PR_ALIGN_OF_DOUBLE
#define ALIGN_OF_POINTER	PR_ALIGN_OF_POINTER
#define ALIGN_OF_WORD		PR_ALIGN_OF_WORD

#define BYTES_PER_WORD_LOG2		PR_BYTES_PER_WORD_LOG2
#define BYTES_PER_DWORD_LOG2	PR_BYTES_PER_DWORD_LOG2
#define WORDS_PER_DWORD_LOG2	PR_WORDS_PER_DWORD_LOG2
#endif NO_NSPR_10_SUPPORT

#endif /* nspr_cpucfg___ */
