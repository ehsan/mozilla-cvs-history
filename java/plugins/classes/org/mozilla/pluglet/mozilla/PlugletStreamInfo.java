/* 
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Initial Developer of the Original Code is Sun Microsystems,
 * Inc. Portions created by Sun are Copyright (C) 1999 Sun Microsystems,
 * Inc. All Rights Reserved. 
 */
package org.mozilla.pluglet.mozilla;

public interface PlugletStreamInfo {
    /**
     * Returns the MIME type.
     */
    public String getContentType();
    public boolean isSeekable();
    /**
     * Returns the number of bytes that can be read from this input stream
     */
    public int getLength();
    public int getLastModified();
    public String getURL();
    /**
     * Request reading from input stream
     */
    public void requestRead(ByteRanges ranges );
}


