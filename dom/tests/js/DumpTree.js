/* -*- Mode: js; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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


//
// travers the html tree and dump out the type of element
//
function traverse(node, indent)
{
    dump("\n")
    indent += "  "
    var type = node.nodeType;

    // if it's an element dump the tag and recurse the children
    if (type == Node.ELEMENT_NODE) {

        dump(indent + node.tagName)

        // go through the children
        if (node.hasChildNodes()) {
            var children = node.childNodes;
            var length = children.length;
            var count = 0;
            while(count < length) {
                child = children[count]
                traverse(child, indent)
                count++
            }
        }
    }
    // it's just text, no tag, dump "Text"
    else if (type == Node.TEXT_NODE) {
        dump(indent + "Text")
    }
}

var node = document.documentElement

traverse(node, "")
dump("\n")

  