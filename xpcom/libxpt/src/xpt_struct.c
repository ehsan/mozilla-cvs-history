/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* Implementation of XDR routines for typelib structures. */

#include "xpt_xdr.h"
#include "xpt_struct.h"

PRBool
XPT_DoHeader(XPTCursor *cursor, XPTHeader **headerp)
{
    XPTMode mode = cursor->state->mode;
    XPTCursor my_cursor;
    XPTHeader *header;

    if (mode == XPT_DECODE) {
        if !((*headerp = header = PR_NEWZAP(XPTHedaer)))
	    goto error;
    } else {
        header = *headerp;
    }
    
    for (i=0, i<16; i++) {
        if (!XPT_Do8(&my_cursor, &header->magic[i]))
            goto error;
    }
    
    if(!XPT_Do8(&my_cursor, &header->major_version) ||
       !XPT_Do8(&my_cursor, &header->minor_version) ||
       !XPT_Do16(&my_cursor, &header->num_interfaces) ||
       !XPT_Do32(&my_cursor, &header->file_length) ||
       !XPT_DoInterfaceDirectoryEntry(&my_cursor, 
                                      &header->interface_directory) ||
       !XPT_Do8(&my_cursor, &header->data_pool) ||
       !XPT_DoAnnotation(&my_cursor, &header->annotations))
        { 
            goto error;
        }
    
    return PR_TRUE;
    
 error:
    if (mode == XPT_DECODE) {
        PR_FREE(header);
        *headerp = 0;
    }    
    return PR_FALSE;
}   

/* InterfaceDirectoryEntry records go in the header */
PRBool
XPT_DoInterfaceDirectoryEntry(XPTCursor *cursor,
                               XPTInterfaceDirectoryEntry **idep)
{    
    XPTMode mode = cursor->state->mode;
    XPTCursor my_cursor;
    XPTInterfaceDirectoryEntry *ide;

    /*
     * if we were a ``normal'' function, we'd have to get our offset
     * with something like
     *    offset = XPT_GetOffset(cursor->state, XPT_DATA);
     * and then write/read it into/from our _caller_'s cursor:
     *    XPT_Do32(cursor, &offset).
     * In the decode case below, we would want to check for already-restored
     * data at this offset, and create+register if none was found:
     *    ide = XPT_GetAddrForOffset(cursor->state, offset);
     *    if (!ide) {
     *      -- none registered! --
     *        ide = PR_NEW(XPTInterfaceDirectoryEntry);
     *        if (!ide) return PR_FALSE;
     *        XPT_SetAddrForOffset(cursor->state, offset, (void *)ide);
     *        *idep = ide;
     *    } else {
     *        -- found it! --
     *        *idep = ide;
     *        return PR_TRUE;
     *    }
     * I think I'll write a macro that does the right thing here, because
     * it'll appear at the beginning of just about every such function.
     *
     */

    if (mode == XPT_DECODE) {
        if !((*idep = ide = PR_NEWZAP(XPTInterfaceDirectoryEntry)))
            goto error;
    } else {
        ide = *idep;
    }    

    /* create a cursor, reserving XPT_IDE_SIZE bytes in the encode case */
    if (!XPT_CreateCursor(cursor->state, XPT_HEADER, XPT_IDE_SIZE,
                          &my_cursor) ||
        
        /* write the IID in our cursor space */
        !XPT_DoIID(&my_cursor, &ide->iid) ||

        /* write the string in the data pool, and the offset in our
           cursor space */
        !XPT_DoCString(&my_cursor, &ide->name) ||

        /* write the InterfaceDescriptor in the data pool, and the offset
           in our cursor space */
        !XPT_DoInterfaceDescriptor(&my_cursor, &ide->interface_descriptor)) 
        {
            goto error;
        }
    
    return PR_TRUE;

 error:
    if (mode == XPT_DECODE) {
        PR_FREE(ide);
        *idep = 0;
    }
    return PR_FALSE;
}

PRBool
XPT_DoInterfaceDescriptor(XPTCursor *cursor, XPTInterfaceDescriptor **idp)
{
    XPTMode mode = cursor->state->mode;
    XPTInterfaceDescriptor id;
    XPTCursor my_cursor;

    if (mode == XPT_DECODE)
        id = PR_NEWZAP(XPTInterfaceDescriptor);
    else
        id = *idp;
    
    if(!XPT_CreateCursor(cursor->state, XPT_DATA, XPT_ID_SIZE, &my_cursor) ||
       !XPT_DoInterfaceDirectoryEntry(&my_cursor, &id->parent_interface) ||
       !XPT_Do16(&my_cursor, &id->num_methods))
        goto error;
    
    for (i = 0; i < id->num_methods; i++) {
        if (!XPT_DoMethodDescriptor(&my_cursor, &id->method_descriptors[i]))
            goto error;
    }
    
    if (!XPT_Do16(&my_cursor, &id->num_constants))
        goto error;

    for (i = 0; i < id->num_constants; i++) {
        if (!XPT_DoConstDescriptor(&my_cursor, &id->constant_descriptors[i]))
            goto error;
    }

    return PR_TRUE;

 error:
    if (mode == XPT_DECODE) {
        PR_FREE(id);
        *idp = 0;
    }
    return PR_FALSE;
}

PRBool
XPT_DoConstDescriptor(XPTCursor *cursor, XPTConstDescriptor **cdp)
{
    XPTMode mode = cursor->state->mode;
    XPTConstDescriptor cd;
    XPTCursor my_cursor;

    if (mode == XPT_DECODE)
        cd = PR_NEWZAP(XPTConstDescriptor);
    else
        cd = *cdp;

    if (!XPT_CreateCursor(cursor->state, XPT_DATA, XPT_CD_SIZE, &my_cursor) ||
        !XPT_DoCString(&my_cursor, &cd->name) ||
        !XPT_DoTypeDescriptor(&my_cursor, &cd->type))        
        {
            goto error;
        }
    switch(cd->type->prefix->tag) {
    case '0':
        XPT_Do8(&my_cursor, &cd->value->i8);
        break;
    case '1':
        XPT_Do16(&my_cursor, &cd->value->i16);
        break;
    case '2':
        XPT_Do32(&my_cursor, &cd->value->i32);
        break;
    case '3':
        XPT_Do64(&my_cursor, &cd->value->i64);
        break;
    case '4':
        XPT_Do8(&my_cursor, &cd->value->ui8);
        break;
    case '5':
        XPT_Do16(&my_cursor, &cd->value->ui16);
        break;
    case '6':
        XPT_Do32(&my_cursor, &cd->value->ui32);
        break;
    case '7':
        XPT_Do64(&my_cursor, &cd->value->ui64);
        break;
    case '11':
        XPT_Do8(&my_cursor, &cd->value->ch);
        break;
    case '12':
        XPT_Do16(&my_cursor, &cd->value->wch);
        break;
    case '15':
        if (cd->type->prefix->is_pointer == 1) {
            XPT_DoString(&my_cursor, &cd->value->string);
            break;
        }
        goto error;
    default:
            goto error;
    }

    return PR_TRUE;

 error:
    if (mode == XPT_DECODE) {
        PR_FREE(cd);
        *cdp = 0;
    }
    return PR_FALSE;
}

PRBool
XPT_DoMethodDescriptor(XPTCursor *cursor, XPTMethodDescriptor **mdp)
{
    XPTMode mode = cursor->state->mode;
    XPTConstDescriptor md;
    XPTCursor my_cursor;
    uintn scratch;

    if (mode == XPT_DECODE)
        md = PR_NEWZAP(XPTMethodDescriptor);
    else
        md = *mdp;

    if (!XPT_CreateCursor(cursor->state, XPT_DATA, XPT_MD_SIZE, &my_cursor) ||
        !XPT_DO_BITS(&my_cursor, &md->is_getter, 1, scratch) ||
        !XPT_DO_BITS(&my_cursor, &md->is_setter, 1, scratch) ||
        !XPT_DO_BITS(&my_cursor, &md->is_varargs, 1, scratch) ||
        !XPT_DO_BITS(&my_cursor, &md->is_constructor, 1, scratch) ||
        !XPT_DO_BITS(&my_cursor, &md->reserved, 4, scratch) ||
        !XPT_DoCString(&my_cursor, &md->name) ||
        !XPT_Do8(&my_cursor, &md->num_args) ||
        !XPT_DoParamDescriptor(&my_cursor, &md->params) ||
        !XPT_DoParamDescriptor(&my_cursor, &md->result))
        {
            goto error;
        }

    return PR_TRUE;
    
 error:
    if (mode == XPT_DECODE) {
        PR_FREE(md);
        *mdp = 0;
    }
    return PR_FALSE;
}

PRBool
XPT_DoParamDescriptor(XPTCursor *cursor, XPTParamDescriptor **pdp)
{
    XPTMode mode = cursor->state->mode;
    XPTParamDescriptor pd;
    XPTCursor my_cursor;
    uintn scratch;

    if (mode == XPT_DECODE)
        pd = PR_NEWZAP(XPTParamDescriptor);
    else
        pd = *pdp;

    if (!XPT_CreateCursor(cursor->state, XPT_DATA, XPT_PD_SIZE, &my_cursor) ||
        !XPT_DO_BITS(&my_cursor, &pd->in, 1, scratch) ||
        !XPT_DO_BITS(&my_cursor, &pd->out, 1, scratch) ||
        !XPT_DO_BITS(&my_cursor, &pd->retval, 1, scratch) ||
        !XPT_DO_BITS(&my_cursor, &pd->reserved, 5, scratch) ||
        !XPT_DoTypeDescriptor(&my_cursor, &pd->type))    
        {
            goto error;
        }
        
    return PR_TRUE;

 error:
    if (mode == XPT_DECODE) {
        PR_FREE(pd);
        *pdp = 0;
    }
    return PR_FALSE;
}

PRBool
XPT_DoTypeDescriptorPrefix(XPTCursor *cursor, XPTTypeDescriptorPrefix **tdpp)
{
    XPTMode mode = cursor->state->mode;
    XPTTypeDescriptorPrefix tdp;
    XPTCursor my_cursor;
    uintn scratch;
    
    if (mode == XPT_DECODE)
        tdp = PR_NEWZAP(XPTTypeDescriptorPrefix);
    else
        tdp = *tdpp;
    
    if (!XPT_CreateCursor(cursor->state, XPT_DATA, XPT_TDP_SIZE, 
                          &my_cursor) ||
        !XPT_DO_BITS(&my_cursor, &tdp->is_pointer, 1, scratch) ||
        !XPT_DO_BITS(&my_cursor, &tdp->is_unique_pointer, 1, scratch) ||
        !XPT_DO_BITS(&my_cursor, &tdp->is_reference, 1, scratch) ||
        !XPT_DO_BITS(&my_cursor, &tdp->tag, 5, scratch))
        { 
            goto error;
        }

    return PR_TRUE;

 error:
    if (mode == XPT_DECODE) {
        PR_FREE(tdp);
        *tdpp = 0;
    }
    return PR_FALSE;
}

PRBool
XPT_DoInterfaceTypeDescriptor(XPTCursor *cursor, 
                              XPTInterfaceTypeDescriptor **itdp)
{
    XPTMode mode = cursor->state->mode;
    XPTIntefaceTypeDescriptor itd;
    XPTCursor my_cursor;
    
    if (mode == XPT_DECODE)
        itd = PR_NEWZAP(XPTInterfaceTypeDescriptor);
    else
        itd = *itdp;
    
    if (!XPT_CreateCursor(cursor->state, XPT_DATA, XPT_ITD_SIZE, 
                          &my_cursor) ||
        !XPT_DoInterfaceDirectoryEntry(&my_cursor, &itd->interface))
        {
            goto error;
        }
    
    return PR_TRUE;
    
 error:
    if (mode == XPT_DECODE) {
        PR_FREE(itd);
        *itdp = 0;
    }
    return PR_FALSE;    
}

PRBool
XPT_DoInterfaceIsTypeDescriptor(XPTCursor *cursor, 
                                XPTInterfaceIsTypeDescriptor **iitdp)
{
    XPTMode mode = cursor->state->mode;
    XPTIntefaceTypeDescriptor iitd;
    XPTCursor my_cursor;
    
    if (mode == XPT_DECODE)
        iitd = PR_NEWZAP(XPTInterfaceIsTypeDescriptor);
    else
        iitd = *iitdp;
    
    if (!XPT_CreateCursor(cursor->state, XPT_DATA, XPT_IITD_SIZE, 
                         &my_cursor) ||
        !XPT_Do8(&my_cursor, &iitd->argnum))
        {
            goto error;
        }
        
    return PR_TRUE;
    
 error:
    if (mode == XPT_DECODE) {
        PR_FREE(iitd);
        *iitdp = 0;
    }
    return PR_FALSE;    
}

PRBool
XPT_DoTypeDescriptor(XPTCursor *cursor, XPTSimpleTypeDescriptor **tdp)
{
    XPTMode mode = cursor->state->mode;
    XPTSimpleTypeDescriptor td;
    XPTCursor my_cursor;
    
    if (mode == XPT_DECODE)
        td = PR_NEWZAP(XPTTypeDescriptor);
    else
        td = *tdp;
    
    if (!XPT_CreateCursor(cursor->state, XPT_DATA, XPT_TD_SIZE, &my_cursor) ||
        !XPT_DoTypeDescriptorPrefix(&my_cursor, &td->prefix))
        {
            goto error;
        }
    
    if (td->prefix->tag == 16) {
        if (!XPT_DoInterfaceTypeDescriptor(&my_cursor, &td->type->interface))
            goto error;
    } else {
        if (td->prefix->tag == 17) {
            if (!XPT_DoInterfaceIsTypeDescriptor(&my_cursor, 
                                               &td->type->interface_is))
                goto error;
        }
    }
   
    return PR_TRUE;
    
 error:
    if (mode == XPT_DECODE) {
        PR_FREE(td);
        *tdp = 0;
    }
    return PR_FALSE;
}


PRBool
XPT_DoAnnotationPrefix(XPTCursor *cursor, XPTAnnotationPrefix **app)
{
    XPTMode mode = cursor->state->mode;
    XPTAnnotationPrefix ap;
    XPTCursor my_cursor;
    uintn scratch;
    
    if (mode == XPT_DECODE)
        ap = PR_NEWZAP(XPTAnnotationPrefix);
    else
        ap = *app;
    
    if (!XPT_CreateCursor(cursor->state, XPT_HEADER, XPT_AP_SIZE, 
                          &my_cursor) ||
        !XPT_DO_BITS(&my_cursor, &ap->is_last, 1, scratch) ||
        !XPT_DO_BITS(&my_cursor, &ap->tag, 7, scratch))
        {
            goto error;
        }

    return PR_TRUE;
    
 error:
    if (mode == XPT_DECODE) {
        PR_FREE(ap);
        *app = 0;
    }
    return PR_FALSE;    
}

PRBool
XPT_DoPrivateAnnotation(XPTCursor *cursor, XPTPrivateAnnotation **pap)
{
    XPTMode mode = cursor->state->mode;
    XPTPrivateAnnotation pa;
    XPTCursor my_cursor;
    
    if (mode == XPT_DECODE)
        pa = PR_NEWZAP(XPTPrivateAnnotation);
    else
        pa = *pap;
    
    if (!XPT_CreateCursor(cursor->state, XPT_HEADER, XPT_AP_SIZE, 
                          &my_cursor) ||
        !XPT_DoString(&my_cursor, &pa->creator) ||
        !XPT_DoString(&my_cursor, &pa->private_data))
        {
            goto error;
        }
    
    return PR_TRUE;

 error:
    if (mode == XPT_DECODE) {
        PR_FREE(pa);
        *pap = 0;
    }
    return PR_FALSE;
}

PRBool
XPT_DoAnnotation(XPTCursor *cursor, XPTAnnotation **ap)
{
    XPTMode mode = cursor->state->mode;
    XPTAnnotation a;
    XPTCursor my_cursor;
    
    if (mode == XPT_DECODE)
        a = PR_NEWZAP(XPTPrivateAnnotation);
    else
        a = *ap;
    
    if (!XPT_CreateCursor(cursor->state, XPT_HEADER, XPT_A_SIZE, 
                          &my_cursor) ||
        !XPT_DoAnnotationPrefix(&my_cursor, &a->prefix))
        {
            goto error;
        }
    
    if (a->prefix->tag == 1) {
        if (!XPT_DoPrivateAnnotation(&my_cursor, &a->private))
            goto error;
    }
    
    return PR_TRUE;

 error:
    if (mode == XPT_DECODE) {
        PR_FREE(a);
        *ap = 0;
    }
    return PR_FALSE;
}




