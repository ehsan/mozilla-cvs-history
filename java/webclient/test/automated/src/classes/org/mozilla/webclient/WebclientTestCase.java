/*
 * $Id: WebclientTestCase.java,v 1.1 2002/10/01 00:39:28 edburns%acm.org Exp $
 */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * 
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Sun
 * Microsystems, Inc. Portions created by Sun are
 * Copyright (C) 1999 Sun Microsystems, Inc. All
 * Rights Reserved.
 *
 * Contributor(s): Ed Burns &lt;edburns@acm.org&gt;
 */

package org.mozilla.webclient;

// WebclientTestCase.java

import java.util.ArrayList;

import java.io.IOException;

import org.mozilla.util.Assert;
import org.mozilla.util.ParameterCheck;

import junit.framework.TestCase;

/**
 *
 *  <B>WebclientTestCase</B> is a class ...
 *
 * <B>Lifetime And Scope</B> <P>
 *
 * @version $Id: WebclientTestCase.java,v 1.1 2002/10/01 00:39:28 edburns%acm.org Exp $
 * 
 * @see	Blah
 * @see	Bloo
 *
 */

public abstract class WebclientTestCase extends TestCase
{
//
// Protected Constants
//

public static final String WEBCLIENTSTUB_LOG_MODULE = "webclientstub";
public static final String WEBCLIENT_LOG_MODULE = "webclient";
public static final String OUTPUT_FILE_ROOT = "./build.test/";

//
// Class Variables
//

//
// Instance Variables
//

// Attribute Instance Variables

// Relationship Instance Variables

//
// Constructors and Initializers    
//

public WebclientTestCase()
{
    super("WebclientTestCase");
}

public WebclientTestCase(String name)
{
    super(name);
}

//
// Class methods
//

//
// Methods From TestCase
//

public void setUp()
{
    verifyPreconditions();
}

//
// General Methods
//

/**

* assertTrue that the string logModuleName is a correct log module
* string as specified in pr_log.h, and that its value is at least n.

*/

protected void verifyLogModuleValueIsAtLeastN(String logModuleName, int n)
{
    int i = 0;
    String logModuleValue = null;
    assertTrue(null != (logModuleValue = 
			System.getProperty("NSPR_LOG_MODULES")));
    
    assertTrue(-1 != 
	       (i = logModuleValue.indexOf(logModuleName + ":")));
    try {
	i = Integer.
	    valueOf(logModuleValue.substring(i + logModuleName.length() + 1,
					     i + logModuleName.length() + 2)).
	    intValue();
	assertTrue(i >= n); 
    }
    catch (Throwable e) {
	e.printStackTrace();
	assertTrue(false);
    }
	
}

/**

* assertTrue that NSPR_LOG_FILE is set.

*/

protected String verifyOutputFileIsSet()
{
    String logFileValue = null;

    assertTrue(null != (logFileValue = 
			System.getProperty("NSPR_LOG_FILE")));
    return logFileValue;
    
}

/**

* This implementation checks that the proper environment vars are set.

*/

protected void verifyPreconditions()
{
    String nsprLogModules = null;

    // make sure we have at least PR_LOG_DEBUG set
    verifyLogModuleValueIsAtLeastN(WEBCLIENTSTUB_LOG_MODULE, 4);
    verifyLogModuleValueIsAtLeastN(WEBCLIENT_LOG_MODULE, 4);
    if (sendOutputToFile()) {
	verifyOutputFileIsSet();
    }
}

public boolean verifyExpectedOutput()
{
    boolean result = false;
    CompareFiles cf = new CompareFiles();
    String errorMessage = null;
    String outputFileName = null;
    String correctFileName = null;
    
    // If this testcase doesn't participate in file comparison
    if (!this.sendOutputToFile() && 
	(null == this.getExpectedOutputFilename())) {
	return true;
    }
    
    if (this.sendOutputToFile() ) {
        outputFileName = verifyOutputFileIsSet();
    } 
    correctFileName = OUTPUT_FILE_ROOT + this.getExpectedOutputFilename();
    
    errorMessage = "File Comparison failed: diff -u " + outputFileName + " " + 
        correctFileName;
    
    ArrayList ignoreList = null;
    String [] ignore = null;
    
    if (null != (ignore = this.getLinesToIgnore())) {
	ignoreList = new ArrayList();
	for (int i = 0; i < ignore.length; i++) {
	    ignoreList.add(ignore[i]);
	}
    }
    
    try {
	result = cf.filesIdentical(outputFileName, correctFileName,ignoreList,
				   getIgnorePrefix()); 
    }
    catch (IOException e) {
	System.out.println(e.getMessage());
	e.printStackTrace();
    }

    if (!result) {
	System.out.println(errorMessage);
    }
    System.out.println("VERIFY:"+result); 
    return result;
}

/**

* @return the name of the expected output filename for this testcase.

*/

public String getExpectedOutputFilename() { return null; }

public String [] getLinesToIgnore() { return null; }

public int getIgnorePrefix() { return 15; } // 15 is the ignore prefix
					    // for output from PR_LOG

public boolean sendOutputToFile() { return false; }



} // end of class WebclientTestCase
