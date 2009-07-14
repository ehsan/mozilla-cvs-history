<?php
// ***** BEGIN LICENSE BLOCK *****
//
// Version: MPL 1.1/GPL 2.0/LGPL 2.1
//
// The contents of this file are subject to the Mozilla Public License Version
// 1.1 (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS" basis,
// WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
// for the specific language governing rights and limitations under the
// License.
//
// The Original Code is AUS.
//
// The Initial Developer of the Original Code is Mike Morgan.
// 
// Portions created by the Initial Developer are Copyright (C) 2006 
// the Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Mike Morgan <morgamic@mozilla.com>
//
// Alternatively, the contents of this file may be used under the terms of
// either the GNU General Public License Version 2 or later (the "GPL"), or
// the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
// in which case the provisions of the GPL or the LGPL are applicable instead
// of those above. If you wish to allow use of your version of this file only
// under the terms of either the GPL or the LGPL, and not to allow others to
// use your version of this file under the terms of the MPL, indicate your
// decision by deleting the provisions above and replace them with the notice
// and other provisions required by the GPL or the LGPL. If you do not delete
// the provisions above, a recipient may use your version of this file under
// the terms of any one of the MPL, the GPL or the LGPL.
//
// ***** END LICENSE BLOCK *****

/**
 * Generic class definition for all AUS objects.
 *
 * @package aus
 * @subpackage inc
 * @author Mike Morgan
 */
class AUS_Object {

    function AUS_Object() {
    }

    /**
     * Set an object parameter.
     * @param string $key
     * @param mixed $val
     * @param bool $overwrite
     * @return boolean
     */
    function setVar($key,$val,$overwrite=false) {
        if (!isset($this->$key) || (isset($this->$key) && $overwrite)) {
            $this->$key = $val;
            return true;
        } else {
            return false;
        }
    }

    /**
     * Determine whether or not a given channel is an exception for throttling.
     * Requires throttleExceptions array in config which is declared as a global because it's faster.
     *
     * @param string $version
     * @param string $channel
     * @return boolean
     */
    function isThrottleException($version, $channel) {

        // I hate PHP and how I use it.
        global $throttleExceptions;

        // If the exception array is not defined or just empty, return false.
        if (empty($throttleExceptions[$version])) {
            return false;
        }

        // Check the passed channel against our patterns to see if it's an exception.
        foreach ($throttleExceptions[$version] as $channelPattern) {

            // No need to create a regular expression if there's no wildcard
            if (strpos($channelPattern, '*') === false && $channelPattern == $channel) {
                return true;
            }

            // If we get here, check the pattern against the channel string and return true if there's a match.
            if (preg_match('/^'. str_replace('\\*', '.*', preg_quote($channelPattern, '/')) .'$/', $channel)) {
                return true;
            }
        }

        return false;
    }
}
?>
