/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is 
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Michael Lowe <michael.lowe@bigfoot.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or 
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#ifndef nsITimerQueue_h___
#define nsITimerQueue_h___

#include "nscore.h"
#include "nsISupports.h"

class nsTimer;

/// Interface IID for nsITimerQueue
#define NS_ITIMERQUEUE_IID         \
{ 0x7d1f1070, 0x1dd2, 0x11b2,  \
{ 0xa5, 0x18, 0xe5, 0x2b, 0x26, 0xa2, 0x48, 0x7b } }


class nsITimerQueue : public nsISupports {
public:
  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ITIMERQUEUE_IID)

  NS_IMETHOD_(void) AddReadyQueue(nsITimer* timer)=0;
  NS_IMETHOD_(PRBool) IsTimerInQueue(nsITimer* timer)=0;
  NS_IMETHOD_(PRBool) HasReadyTimers(PRUint32 minTimerPriority)=0;
  NS_IMETHOD_(void) FireNextReadyTimer(PRUint32 minTimerPriority)=0;
};

#endif // nsITimerQueue_h___
