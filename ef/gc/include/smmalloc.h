/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; -*-  
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

/*******************************************************************************
                            S P O R T   M O D E L    
                              _____
                         ____/_____\____
                        /__o____o____o__\        __
                        \_______________/       (@@)/
                         /\_____|_____/\       x~[]~
             ~~~~~~~~~~~/~~~~~~~|~~~~~~~\~~~~~~~~/\~~~~~~~~~

    Advanced Technology Garbage Collector
    Copyright (c) 1997 Netscape Communications, Inc. All rights reserved.
    Recovered by: Warren Harris
*******************************************************************************/

#ifndef __SMMALLOC__
#define __SMMALLOC__

#include "sm.h"
#include "smpage.h"

SM_BEGIN_EXTERN_C

/*******************************************************************************
 * Malloc Pools
 ******************************************************************************/

typedef struct SMPool {
    SMPageDesc*         unfilledPages;
    SMPageDesc*         sweepPage;
    SMSmallObjCount     sweepIndex;
    PRLock*             lock;
} SMPool;

typedef struct SMPool {
    SMPageMgr           pageMgr;
    PRLock*             lock[SM_ALLOC_BUCKETS];
    struct SMFree*      freeList[SM_ALLOC_BUCKETS];
} SMPool;

SM_EXTERN(PRStatus)
SM_InitPool(SMPool* pool, SMPageMgr* pageMgr);

SM_EXTERN(void)
SM_FiniPool(SMPool* pool);

extern SMSmallObjSize  sm_ObjectSize[];
extern SMSmallObjCount sm_ObjectsPerPage[];

/******************************************************************************/

SM_END_EXTERN_C

#endif /* __SMMALLOC__ */
/******************************************************************************/
