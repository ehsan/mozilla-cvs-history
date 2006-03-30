/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is ChatZilla.
 *
 * The Initial Developer of the Original Code is
 * James Ross.
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   James Ross, <twpol@aol.com>, original author
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the LGPL or the GPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

var strBundle;

function getMsg (msgName)
{
    var restCount = arguments.length - 1;

    if (!strBundle)
    {       
        strBundle = 
            srGetStrBundle("chrome://chatzilla/locale/chatzilla.properties");
    }
    
    try 
    {
        if (restCount == 1 && arguments[1] instanceof Array)
        {
            return strBundle.formatStringFromName (msgName, arguments[1], 
                                                       arguments[1].length);
        }
        else if (restCount > 0)
        {
            var subPhrases = new Array();
            for (var i = 1; i < arguments.length; ++i)
                subPhrases.push(arguments[i]);
            return strBundle.formatStringFromName (msgName, subPhrases,
                                                         subPhrases.length);
        }

        return strBundle.GetStringFromName (msgName);
    }
    catch (ex)
    {
        dump ("caught exception getting value for ``" + msgName + "''\n" + ex);
        return msgName;
    }
}
