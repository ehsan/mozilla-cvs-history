/* 
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
 * The Original Code is the Netscape Security Services for Java.
 * 
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are 
 * Copyright (C) 1998-2000 Netscape Communications Corporation.  All
 * Rights Reserved.
 * 
 * Contributor(s):
 * 
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU General Public License Version 2 or later (the
 * "GPL"), in which case the provisions of the GPL are applicable 
 * instead of those above.  If you wish to allow use of your 
 * version of this file only under the terms of the GPL and not to
 * allow others to use your version of this file under the MPL,
 * indicate your decision by deleting the provisions above and
 * replace them with the notice and other provisions required by
 * the GPL.  If you do not delete the provisions above, a recipient
 * may use your version of this file under either the MPL or the
 * GPL.
 */
package org.mozilla.jss;

/**
 * A class for closing databases. Since closing the databases is
 * very dangerous and breaks the JSS model, it may only be done from  
 * special applications. This class should be subclasses by
 * authorized subclasses.  It cannot be instantiated itself.
 */
public abstract class DatabaseCloser {

    private static final String authorizedClosers[] =
        {   "org.mozilla.certsetup.apps.CertSetup$DatabaseCloser",
            "org.mozilla.jss.CloseDBs"                                  };

    /**
     * Creates a new DatabaseCloser.  This should only be called
     * from an authorized subclass.  This class cannot itself be
     * instantiated.
     *
     * @throws Exception If the instantiation is not a valid subclass.
     */
    public DatabaseCloser() throws Exception {
        Class clazz = this.getClass();
        String name = clazz.getName();
        boolean approved = false;
        for(int i=0; i < authorizedClosers.length; i++) {
            if( name.equals( authorizedClosers[i] ) ) {
                approved = true;
                break;
            }
        }
        if(!approved) {
            throw new Exception();
        }
    }

    /**
     * Closes the certificate and key databases.  This is extremely
     * dangerous.
     */
    protected native void closeDatabases();
}
