/* 
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is the Netscape security libraries.
 * 
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are 
 * Copyright (C) 1994-2000 Netscape Communications Corporation.  All
 * Rights Reserved.
 * 
 * Contributor(s):
 * 
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU General Public License Version 2 or later (the
 * "GPL"), in which case the provisions of the GPL are applicable 
 * instead of those above.  If you wish to allow use of your 
 * version of this file only under the terms of the GPL and not to
 * allow others to use your version of this file under the MPL,
 * indicate your decision by deleting the provisions above and
 * replace them with the notice and other provisions required by
 * the GPL.  If you do not delete the provisions above, a recipient
 * may use your version of this file under either the MPL or the
 * GPL.
 */

/*
 * This file is in part derived from a file "pkcs11t.h" made available
 * by RSA Security at ftp://ftp.rsasecurity.com/pub/pkcs/pkcs-11/pkcs11t.h
 */

#ifndef NSSCKP_H
#define NSSCKP_H

#ifdef DEBUG
static const char NSSCKP_CVS_ID[] = "@(#) $RCSfile: nssckp.h,v $ $Revision: 1.1 $ $Date: 2000/03/31 19:43:33 $ $Name:  $";
#endif /* DEBUG */

#endif /* NSSCKP_H */

/*
 * These platform-dependent packing rules are required by all PKCS#11
 * modules, to be binary compatible.  These rules have been placed in 
 * separate header files (nssckp.h to enable the packing, nsscku.h to 
 * disable) for consistancy.  These files can be included many times,
 * so the bodies should *NOT* be in the multiple-inclusion-preventing
 * #ifndef/#endif area above.
 */

/*
 * WIN32 is defined (when appropriate) in NSPR's prcpucfg.h.
 */

#ifdef WIN32
#pragma warning(disable:4103)
#pragma pack(push, cryptoki, 1)
#endif /* WIN32 */

/* End of nssckp.h */
