/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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

#ifndef _nsMotifMenu_h_
#define _nsMotifMenu_h_

#include "nscore.h"

#include <qpopupmenu.h>
#include <qmenubar.h>

class nsBrowserWindow;

class nsMenuEventHandler : public QObject
{
    Q_OBJECT
public:
    nsMenuEventHandler(nsBrowserWindow * window);

public slots:
    void MenuItemActivated(int id);

private:
    nsBrowserWindow * mWindow;
};

void CreateViewerMenus(QWidget *aParent, 
                       void * data, 
                       PRInt32 * aMenuBarHeight);
void InsertMenuItem(QPopupMenu * popup, 
                    const char * string, 
                    QObject * receiver, 
                    int id);

#endif // _nsMotifMenu_h_
