/* -*- Mode: java; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
 * Copyright (C) 1997-1999 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

package org.mozilla.javascript;

/**
 * This is the default error reporter for JavaScript.
 *
 * @author Norris Boyd
 */
class DefaultErrorReporter implements ErrorReporter {

    public void warning(String message, String sourceName, int line,
                        String lineSource, int lineOffset)
    {
        // do nothing
    }

    public void error(String message, String sourceName, int line,
                      String lineSource, int lineOffset)
    {
        throw new EvaluatorException(message);
    }

    public EvaluatorException runtimeError(String message, String sourceName,
                                           int line, String lineSource, 
                                           int lineOffset)
    {
        return new EvaluatorException(message);
    }
}
