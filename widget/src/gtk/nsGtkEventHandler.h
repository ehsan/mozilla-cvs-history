/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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

#ifndef __nsXtEventHandler_h      
#define __nsXtEventHandler_h

#include "Xm/Xm.h"

class nsIWidget;
class nsIMenuItem;

void nsGtkWidget_KeyPressMask_EventHandler(Widget w, XtPointer p, XEvent * event, Boolean * b);
void nsGtkWidget_KeyReleaseMask_EventHandler(Widget w, XtPointer p, XEvent * event, Boolean * b);
void nsGtkWidget_ExposureMask_EventHandler(Widget w, XtPointer p, XEvent * event, Boolean * b);
void nsGtkWidget_ButtonPressMask_EventHandler(Widget w,XtPointer p, XEvent * event, Boolean * b);
void nsGtkWidget_ButtonReleaseMask_EventHandler(Widget w, XtPointer p, XEvent * event, Boolean * b);
void nsGtkWidget_ButtonMotionMask_EventHandler(Widget w, XtPointer p, XEvent * event, Boolean * b);
void nsGtkWidget_MotionMask_EventHandler(Widget w, XtPointer p, XEvent * event, Boolean * b);
void nsGtkWidget_EnterMask_EventHandler(Widget w, XtPointer p, XEvent * event, Boolean * b);
void nsGtkWidget_LeaveMask_EventHandler(Widget w, XtPointer p, XEvent * event, Boolean * b);
//----------------------------------------------------

void nsGtkWidget_FSBCancel_Callback(Widget w, XtPointer p, XtPointer call_data);
void nsGtkWidget_FSBOk_Callback(Widget w, XtPointer p, XtPointer call_data);

//----------------------------------------------------
void nsGtkWidget_Focus_Callback(Widget w, XtPointer p, XtPointer call_data);
void nsGtkWidget_Scrollbar_Callback(Widget w, XtPointer p, XtPointer call_data);
void nsGtkWidget_Toggle_Callback(Widget w, XtPointer p, XtPointer call_data);
void nsGtkWidget_Toggle_ArmCallback(Widget w, XtPointer p, XtPointer call_data);
void nsGtkWidget_Toggle_DisArmCallback(Widget w, XtPointer p, XtPointer call_data);
void nsGtkWidget_RadioButton_ArmCallback(Widget w, XtPointer p, XtPointer call_data);
void nsGtkWidget_RadioButton_DisArmCallback(Widget w, XtPointer p, XtPointer call_data);

void nsGtkWidget_Text_Callback(Widget w, XtPointer p, XtPointer call_data);
void nsGtkWidget_Resize_Callback(Widget w, XtPointer p, XtPointer call_data);
void nsGtkWidget_Expose_Callback(Widget w, XtPointer p, XtPointer call_data);

void nsGtkWidget_Refresh_Callback(XtPointer call_data);

void nsGtkWidget_ResetResize_Callback(XtPointer call_data);

void nsGtkWidget_Menu_Callback(Widget w, XtPointer p, XtPointer call_data);

#endif  // __nsXtEventHandler.h





