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

package com.netscape.jsdebugging.api.local;

import com.netscape.jsdebugging.api.*;

/**
 * InterruptHook must be subinterfaceed to respond to hooks
 * at a particular program instruction.
 */
public class InterruptHookLocal extends netscape.jsdebug.InterruptHook
{
    public InterruptHookLocal( InterruptHook hook )
    {
        _hook = hook;
    }

    public void aboutToExecute(netscape.jsdebug.ThreadStateBase debug, 
                               netscape.jsdebug.PC pc)
    {
//        System.out.println( "InterruptHookLocal called..." );
        try
        {
            JSThreadStateLocal ts = new JSThreadStateLocal(debug);
            JSPCLocal jspc = (JSPCLocal) ts.getCurrentFrame().getPC();
            ts.aboutToCallHook();
            _hook.aboutToExecute(ts, jspc);
            ts.returnedFromCallHook();
        }
        catch( Exception e )
        {
            e.printStackTrace();
            // eat it.
        }
    }

    public InterruptHook getWrappedHook() {return _hook;}

    private InterruptHook _hook;
}
