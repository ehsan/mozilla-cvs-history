/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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

/* Invoke tests xptcall. */

#include <stdio.h>
#include "xptcall.h"
#include "prlong.h"

// {AAC1FB90-E099-11d2-984E-006008962422}
#define INVOKETESTTARGET_IID \
{ 0xaac1fb90, 0xe099, 0x11d2, \
  { 0x98, 0x4e, 0x0, 0x60, 0x8, 0x96, 0x24, 0x22 } }


class InvokeTestTargetInterface : public nsISupports
{
public:
    NS_IMETHOD AddTwoInts(PRInt32 p1, PRInt32 p2, PRInt32* retval) = 0;
    NS_IMETHOD MultTwoInts(PRInt32 p1, PRInt32 p2, PRInt32* retval) = 0;
    NS_IMETHOD AddTwoLLs(PRInt64 p1, PRInt64 p2, PRInt64* retval) = 0;
    NS_IMETHOD MultTwoLLs(PRInt64 p1, PRInt64 p2, PRInt64* retval) = 0;
};

class InvokeTestTarget : public InvokeTestTargetInterface
{
public:
    NS_DECL_ISUPPORTS
    NS_IMETHOD AddTwoInts(PRInt32 p1, PRInt32 p2, PRInt32* retval);
    NS_IMETHOD MultTwoInts(PRInt32 p1, PRInt32 p2, PRInt32* retval);
    NS_IMETHOD AddTwoLLs(PRInt64 p1, PRInt64 p2, PRInt64* retval);
    NS_IMETHOD MultTwoLLs(PRInt64 p1, PRInt64 p2, PRInt64* retval);

    InvokeTestTarget();
};

static NS_DEFINE_IID(kInvokeTestTargetIID, INVOKETESTTARGET_IID);
NS_IMPL_ISUPPORTS(InvokeTestTarget, kInvokeTestTargetIID);

InvokeTestTarget::InvokeTestTarget()
{
    NS_INIT_REFCNT();
    NS_ADDREF_THIS();
}

NS_IMETHODIMP 
InvokeTestTarget::AddTwoInts(PRInt32 p1, PRInt32 p2, PRInt32* retval)
{
    *retval = p1 + p2;
    return NS_OK;
}        

NS_IMETHODIMP 
InvokeTestTarget::MultTwoInts(PRInt32 p1, PRInt32 p2, PRInt32* retval)
{
    *retval = p1 * p2;
    return NS_OK;
}        

NS_IMETHODIMP 
InvokeTestTarget::AddTwoLLs(PRInt64 p1, PRInt64 p2, PRInt64* retval)
{
    *retval = p1 + p2;
    return NS_OK;
}

NS_IMETHODIMP 
InvokeTestTarget::MultTwoLLs(PRInt64 p1, PRInt64 p2, PRInt64* retval)
{
    *retval = p1 * p2;
    return NS_OK;
}

int main()
{
    InvokeTestTarget *test = new InvokeTestTarget();

    PRInt32 out;
    PRInt64 out64;
    printf("calling direct:\n");
    if(NS_SUCCEEDED(test->AddTwoInts(1,1,&out)))
        printf("\t1 + 1 = %d\n", out);
    else
        printf("\tFAILED");
    if(NS_SUCCEEDED(test->AddTwoLLs(LL_INIT(0,1),LL_INIT(0,1),&out64)))
        printf("\t1L + 1L = %d\n", (int)out64);
    else
        printf("\tFAILED");
    if(NS_SUCCEEDED(test->MultTwoInts(2,2,&out)))
        printf("\t2 * 2 = %d\n", out);
    else
        printf("\tFAILED");
    if(NS_SUCCEEDED(test->MultTwoLLs(LL_INIT(0,2),LL_INIT(0,2),&out64)))
        printf("\t2L * 2L = %d\n", (int)out64);
    else
        printf("\tFAILED");

    nsXPTCVariant var[3];

    printf("calling via invoke:\n");

    var[0].val.i32 = 1;
    var[0].type = nsXPTType::T_I32;
    var[0].flags = 0;

    var[1].val.i32 = 1;
    var[1].type = nsXPTType::T_I32;
    var[1].flags = 0;

    var[2].val.i32 = 0;
    var[2].type = nsXPTType::T_I32;
    var[2].flags = nsXPTCVariant::PTR_IS_DATA;
    var[2].ptr = &var[2].val.i32;

    if(NS_SUCCEEDED(XPTC_InvokeByIndex(test, 3, 3, var)))
        printf("\t1 + 1 = %d\n", var[2].val.i32);
    else
        printf("\tFAILED");

    LL_I2L(var[0].val.i64, 1);
    var[0].type = nsXPTType::T_I64;
    var[0].flags = 0;

    LL_I2L(var[1].val.i64, 1);
    var[1].type = nsXPTType::T_I64;
    var[1].flags = 0;

    LL_I2L(var[2].val.i64, 0);
    var[2].type = nsXPTType::T_I64;
    var[2].flags = nsXPTCVariant::PTR_IS_DATA;
    var[2].ptr = &var[2].val.i64;

    if(NS_SUCCEEDED(XPTC_InvokeByIndex(test, 5, 3, var)))
        printf("\t1L + 1L = %d\n", (int)var[2].val.i64);
    else
        printf("\tFAILED");

    var[0].val.i32 = 2;
    var[0].type = nsXPTType::T_I32;
    var[0].flags = 0;

    var[1].val.i32 = 2;
    var[1].type = nsXPTType::T_I32;
    var[1].flags = 0;

    var[2].val.i32 = 0;
    var[2].type = nsXPTType::T_I32;
    var[2].flags = nsXPTCVariant::PTR_IS_DATA;
    var[2].ptr = &var[2].val.i32;

    if(NS_SUCCEEDED(XPTC_InvokeByIndex(test, 4, 3, var)))
        printf("\t2 * 2 = %d\n", var[2].val.i32);
    else
        printf("\tFAILED");

    LL_I2L(var[0].val.i64,2);
    var[0].type = nsXPTType::T_I64;
    var[0].flags = 0;

    LL_I2L(var[1].val.i64,2);
    var[1].type = nsXPTType::T_I64;
    var[1].flags = 0;

    LL_I2L(var[2].val.i64,0);
    var[2].type = nsXPTType::T_I64;
    var[2].flags = nsXPTCVariant::PTR_IS_DATA;
    var[2].ptr = &var[2].val.i64;

    if(NS_SUCCEEDED(XPTC_InvokeByIndex(test, 6, 3, var)))
        printf("\t2L * 2L = %d\n", (int)var[2].val.i64);
    else
        printf("\tFAILED");

    return 0;
}

