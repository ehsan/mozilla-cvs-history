/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
 * Source date: 9 Apr 1997 21:45:12 GMT
 * netscape/fonts/cdlm module C stub file
 * Generated by jmc version 1.8 -- DO NOT EDIT
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "xp_mem.h"

/* Include the implementation-specific header: */
#include "Pcdlm.h"

/* Include other interface headers: */

/*******************************************************************************
 * cdlm Methods
 ******************************************************************************/

#ifndef OVERRIDE_cdlm_getInterface
JMC_PUBLIC_API(void*)
_cdlm_getInterface(struct cdlm* self, jint op, const JMCInterfaceID* iid, JMCException* *exc)
{
	if (memcmp(iid, &cdlm_ID, sizeof(JMCInterfaceID)) == 0)
		return cdlmImpl2cdlm(cdlm2cdlmImpl(self));
	return _cdlm_getBackwardCompatibleInterface(self, iid, exc);
}
#endif

#ifndef OVERRIDE_cdlm_addRef
JMC_PUBLIC_API(void)
_cdlm_addRef(struct cdlm* self, jint op, JMCException* *exc)
{
	cdlmImplHeader* impl = (cdlmImplHeader*)cdlm2cdlmImpl(self);
	impl->refcount++;
}
#endif

#ifndef OVERRIDE_cdlm_release
JMC_PUBLIC_API(void)
_cdlm_release(struct cdlm* self, jint op, JMCException* *exc)
{
	cdlmImplHeader* impl = (cdlmImplHeader*)cdlm2cdlmImpl(self);
	if (--impl->refcount == 0) {
		cdlm_finalize(self, exc);
	}
}
#endif

#ifndef OVERRIDE_cdlm_hashCode
JMC_PUBLIC_API(jint)
_cdlm_hashCode(struct cdlm* self, jint op, JMCException* *exc)
{
	return (jint)self;
}
#endif

#ifndef OVERRIDE_cdlm_equals
JMC_PUBLIC_API(jbool)
_cdlm_equals(struct cdlm* self, jint op, void* obj, JMCException* *exc)
{
	return self == obj;
}
#endif

#ifndef OVERRIDE_cdlm_clone
JMC_PUBLIC_API(void*)
_cdlm_clone(struct cdlm* self, jint op, JMCException* *exc)
{
	cdlmImpl* impl = cdlm2cdlmImpl(self);
	cdlmImpl* newImpl = (cdlmImpl*)malloc(sizeof(cdlmImpl));
	if (newImpl == NULL) return NULL;
	memcpy(newImpl, impl, sizeof(cdlmImpl));
	((cdlmImplHeader*)newImpl)->refcount = 1;
	return newImpl;
}
#endif

#ifndef OVERRIDE_cdlm_toString
JMC_PUBLIC_API(const char*)
_cdlm_toString(struct cdlm* self, jint op, JMCException* *exc)
{
	return NULL;
}
#endif

#ifndef OVERRIDE_cdlm_finalize
JMC_PUBLIC_API(void)
_cdlm_finalize(struct cdlm* self, jint op, JMCException* *exc)
{
	/* Override this method and add your own finalization here. */
	XP_FREEIF(self);
}
#endif

/*******************************************************************************
 * Jump Tables
 ******************************************************************************/

const struct cdlmInterface cdlmVtable = {
	_cdlm_getInterface,
	_cdlm_addRef,
	_cdlm_release,
	_cdlm_hashCode,
	_cdlm_equals,
	_cdlm_clone,
	_cdlm_toString,
	_cdlm_finalize,
	_cdlm_SupportsInterface,
	_cdlm_CreateObject,
	_cdlm_OnUnload
};

/*******************************************************************************
 * Factory Operations
 ******************************************************************************/

JMC_PUBLIC_API(cdlm*)
cdlmFactory_Create(JMCException* *exception)
{
	cdlmImplHeader* impl = (cdlmImplHeader*)XP_NEW_ZAP(cdlmImpl);
	cdlm* self;
	if (impl == NULL) {
		JMC_EXCEPTION(exception, JMCEXCEPTION_OUT_OF_MEMORY);
		return NULL;
	}
	self = cdlmImpl2cdlm(impl);
	impl->vtablecdlm = &cdlmVtable;
	impl->refcount = 1;
	_cdlm_init(self, exception);
	if (JMC_EXCEPTION_RETURNED(exception)) {
		XP_FREE(impl);
		return NULL;
	}
	return self;
}

