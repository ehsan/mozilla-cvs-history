/*
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

/**
 *  File Name:          regress-7703.js
 *  Reference:          "http://bugzilla.mozilla.org/show_bug.cgi?id=7703";
 *  Description:        See the text of the bugnumber above
 */

    var SECTION = "js1_2";       // provide a document reference (ie, ECMA section)
    var VERSION = "JS1_2"; // Version of JavaScript or ECMA
    var TITLE   = "Regression test for bugzilla # 7703";       // Provide ECMA section title or a description
    var BUGNUMBER = "http://bugzilla.mozilla.org/show_bug.cgi?id=7703";     // Provide URL to bugsplat or bugzilla report

    startTest();               // leave this alone

    /*
     * Calls to AddTestCase here. AddTestCase is a function that is defined
     * in shell.js and takes three arguments:
     * - a string representation of what is being tested
     * - the expected result
     * - the actual result
     *
     * For example, a test might look like this:
     *
     * var zip = /[\d]{5}$/;
     *
     * AddTestCase(
     * "zip = /[\d]{5}$/; \"PO Box 12345 Boston, MA 02134\".match(zip)",   // description of the test
     *  "02134",                                                           // expected result
     *  "PO Box 12345 Boston, MA 02134".match(zip) );                      // actual result
     *
     */

    types = [];
    function inspect(object) {
        for (prop in object) {
            var x = object[prop];
            types[types.length] = (typeof x);
        }
    }

    var o = {a: 1, b: 2};

    AddTestCase( "inspect(o),length", 2,         types.length );
    AddTestCase( "inspect(o)[1]",      "number", types[1] );
    AddTestCase( "inspect(o)[2]",      "number", types[2] );

    function inspect_again(object) {
        for (prop in object) {
            types[types.length] = (typeof object[prop]);
        }
    }

    AddTestCase( "inspect_again(o),length", 2,         types.length );
    AddTestCase( "inspect_again(o)[1]",      "number", types[1] );
    AddTestCase( "inspect_again(o)[2]",      "number", types[2] );


    test();       // leave this alone.  this executes the test cases and
                  // displays results.
