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

#include "nsString.h"
#include "nsINameSpaceManager.h"
#include "nsXULAtoms.h"

static const char kXULNameSpace[] = "http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul";

// XXX make this be autogenerated. doh!

PRInt32  nsXULAtoms::nameSpaceID;
nsIAtom* nsXULAtoms::button;
nsIAtom* nsXULAtoms::checkbox;
nsIAtom* nsXULAtoms::tristatecheckbox;
nsIAtom* nsXULAtoms::radio;
nsIAtom* nsXULAtoms::text;
nsIAtom* nsXULAtoms::toolbar;
nsIAtom* nsXULAtoms::toolbox;

nsIAtom* nsXULAtoms::tree;
nsIAtom* nsXULAtoms::treecaption;
nsIAtom* nsXULAtoms::treehead;
nsIAtom* nsXULAtoms::treebody;
nsIAtom* nsXULAtoms::treeitem;
nsIAtom* nsXULAtoms::treecell;
nsIAtom* nsXULAtoms::treechildren;
nsIAtom* nsXULAtoms::treeindentation;
nsIAtom* nsXULAtoms::treeallowevents;
nsIAtom* nsXULAtoms::treecol;
nsIAtom* nsXULAtoms::treecolgroup;

nsIAtom* nsXULAtoms::progressmeter;
nsIAtom* nsXULAtoms::titledbutton;
nsIAtom* nsXULAtoms::mode;

nsIAtom* nsXULAtoms::box;
nsIAtom* nsXULAtoms::flex;

nsIAtom* nsXULAtoms::widget;
nsIAtom* nsXULAtoms::window;

static nsrefcnt gRefCnt;
static nsINameSpaceManager* gNameSpaceManager;

void nsXULAtoms::AddrefAtoms() {

  if (gRefCnt == 0) {
    /* XUL Atoms registers the XUL name space ID because it's a convenient
       place to do this, if you don't want a permanent, "well-known" ID.
    */
    if (NS_SUCCEEDED(NS_NewNameSpaceManager(&gNameSpaceManager)))
//    gNameSpaceManager->CreateRootNameSpace(namespace);
      gNameSpaceManager->RegisterNameSpace(kXULNameSpace, nameSpaceID);
    else
      NS_ASSERTION(0, "failed to create xul atoms namespace manager");

    // now register the atoms
    button = NS_NewAtom("button");
    checkbox = NS_NewAtom("checkbox");
    tristatecheckbox = NS_NewAtom("tristatecheckbox");
    radio = NS_NewAtom("radio");
    text = NS_NewAtom("text");
    toolbar = NS_NewAtom("toolbar");
    toolbox = NS_NewAtom("toolbox");

    tree = NS_NewAtom("tree");
	treecaption = NS_NewAtom("treecaption");
	treehead = NS_NewAtom("treehead");
	treebody = NS_NewAtom("treebody");
	treecell = NS_NewAtom("treecell");
	treeitem = NS_NewAtom("treeitem");
	treechildren = NS_NewAtom("treechildren");
	treeindentation = NS_NewAtom("treeindentation");
    treeallowevents = NS_NewAtom("treeallowevents");
    treecol = NS_NewAtom("treecol");
	treecolgroup = NS_NewAtom("treecolgroup");

	progressmeter = NS_NewAtom("progressmeter");
	titledbutton = NS_NewAtom("titledbutton");

	mode = NS_NewAtom("mode");
	box = NS_NewAtom("box");
	flex = NS_NewAtom("flex");

    widget = NS_NewAtom("widget");
    window = NS_NewAtom("window");
  }
  ++gRefCnt;
}

void nsXULAtoms::ReleaseAtoms() {

  NS_PRECONDITION(gRefCnt != 0, "bad release of xul atoms");
  if (--gRefCnt == 0) {
    NS_RELEASE(button);
    NS_RELEASE(checkbox);
    NS_RELEASE(tristatecheckbox);
    NS_RELEASE(radio);
    NS_RELEASE(text);
    NS_RELEASE(toolbar);
    NS_RELEASE(toolbox);

    NS_RELEASE(tree);
	NS_RELEASE(treecaption);
	NS_RELEASE(treehead);
	NS_RELEASE(treebody);
	NS_RELEASE(treecell);
	NS_RELEASE(treeitem);
	NS_RELEASE(treechildren);
	NS_RELEASE(treeindentation);
    NS_RELEASE(treeallowevents);
	NS_RELEASE(treecol);
	NS_RELEASE(treecolgroup);

    NS_RELEASE(progressmeter);
	NS_RELEASE(mode);

	NS_RELEASE(box);
	NS_RELEASE(flex);


	NS_RELEASE(widget);
    NS_RELEASE(window);
    NS_IF_RELEASE(gNameSpaceManager);
  }
}
