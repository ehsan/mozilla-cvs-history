/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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

package com.netscape.jsdebugging.apitests.testing.desc;

import com.netscape.jsdebugging.apitests.xml.*;
import com.netscape.jsdebugging.api.*;

/**
 * Describes a SourceLocation object.
 *
 * @author Alex Rakhlin
 */
public class DescSourceLocation extends DescriptorSerializable {

    public DescSourceLocation (XMLWriter xmlw, DescriptorManager dmgr){
        super (Tags.source_location_tag, xmlw, dmgr);
    }


    public void description (Object obj){
        if (obj != null){
            JSSourceLocation loc = (JSSourceLocation) obj;
            _xmlw.tag (Tags.lineno_tag, loc.getLine());
            _xmlw.tag (Tags.url_tag, loc.getURL());
        }
        else {
            _xmlw.tag (Tags.lineno_tag, -1);
            _xmlw.tag (Tags.url_tag, "");
        }
    }
}