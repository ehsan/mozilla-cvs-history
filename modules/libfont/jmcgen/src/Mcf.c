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
 * Source date: 9 Apr 1997 21:45:13 GMT
 * netscape/fonts/cf module C stub file
 * Generated by jmc version 1.8 -- DO NOT EDIT
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "xp_mem.h"

/* Include the implementation-specific header: */
#include "Pcf.h"

/* Include other interface headers: */

/*******************************************************************************
 * cf Methods
 ******************************************************************************/

#ifndef OVERRIDE_cf_getInterface
JMC_PUBLIC_API(void*)
_cf_getInterface(struct cf* self, jint op, const JMCInterfaceID* iid, JMCException* *exc)
{
	if (memcmp(iid, &cf_ID, sizeof(JMCInterfaceID)) == 0)
		return cfImpl2cf(cf2cfImpl(self));
	return _cf_getBackwardCompatibleInterface(self, iid, exc);
}
#endif

#ifndef OVERRIDE_cf_addRef
JMC_PUBLIC_API(void)
_cf_addRef(struct cf* self, jint op, JMCException* *exc)
{
	cfImplHeader* impl = (cfImplHeader*)cf2cfImpl(self);
	impl->refcount++;
}
#endif

#ifndef OVERRIDE_cf_release
JMC_PUBLIC_API(void)
_cf_release(struct cf* self, jint op, JMCException* *exc)
{
	cfImplHeader* impl = (cfImplHeader*)cf2cfImpl(self);
	if (--impl->refcount == 0) {
		cf_finalize(self, exc);
	}
}
#endif

#ifndef OVERRIDE_cf_hashCode
JMC_PUBLIC_API(jint)
_cf_hashCode(struct cf* self, jint op, JMCException* *exc)
{
	return (jint)self;
}
#endif

#ifndef OVERRIDE_cf_equals
JMC_PUBLIC_API(jbool)
_cf_equals(struct cf* self, jint op, void* obj, JMCException* *exc)
{
	return self == obj;
}
#endif

#ifndef OVERRIDE_cf_clone
JMC_PUBLIC_API(void*)
_cf_clone(struct cf* self, jint op, JMCException* *exc)
{
	cfImpl* impl = cf2cfImpl(self);
	cfImpl* newImpl = (cfImpl*)malloc(sizeof(cfImpl));
	if (newImpl == NULL) return NULL;
	memcpy(newImpl, impl, sizeof(cfImpl));
	((cfImplHeader*)newImpl)->refcount = 1;
	return newImpl;
}
#endif

#ifndef OVERRIDE_cf_toString
JMC_PUBLIC_API(const char*)
_cf_toString(struct cf* self, jint op, JMCException* *exc)
{
	return NULL;
}
#endif

#ifndef OVERRIDE_cf_finalize
JMC_PUBLIC_API(void)
_cf_finalize(struct cf* self, jint op, JMCException* *exc)
{
	/* Override this method and add your own finalization here. */
	XP_FREEIF(self);
}
#endif

/*******************************************************************************
 * Jump Tables
 ******************************************************************************/

const struct cfInterface cfVtable = {
	_cf_getInterface,
	_cf_addRef,
	_cf_release,
	_cf_hashCode,
	_cf_equals,
	_cf_clone,
	_cf_toString,
	_cf_finalize,
	_cf_GetState,
	_cf_EnumerateSizes,
	_cf_GetRenderableFont,
	_cf_GetMatchInfo,
	_cf_GetRcMajorType,
	_cf_GetRcMinorType
};

/*******************************************************************************
 * Factory Operations
 ******************************************************************************/

JMC_PUBLIC_API(cf*)
cfFactory_Create(JMCException* *exception, struct nfrc* a)
{
	cfImplHeader* impl = (cfImplHeader*)XP_NEW_ZAP(cfImpl);
	cf* self;
	if (impl == NULL) {
		JMC_EXCEPTION(exception, JMCEXCEPTION_OUT_OF_MEMORY);
		return NULL;
	}
	self = cfImpl2cf(impl);
	impl->vtablecf = &cfVtable;
	impl->refcount = 1;
	_cf_init(self, exception, a);
	if (JMC_EXCEPTION_RETURNED(exception)) {
		XP_FREE(impl);
		return NULL;
	}
	return self;
}

