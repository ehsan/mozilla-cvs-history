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
 * AUS main script.
 * @package aus
 * @subpackage docs
 * @author Mike Morgan
 *
 * This script handles incoming requests, reads the related build 
 * snippet and returns a properly formatted XML file for testing.
 */

// Require config and supporting libraries.
require_once('./inc/init.php');

// Instantiate XML object.
$xml = new Xml();

//Are we behind a proxy and given the IP via an alternate enviroment variable? If so, use it.
if (!empty($_SERVER["HTTP_X_FORWARDED_FOR"])) {
    list($ip) = explode(', ',$_SERVER["HTTP_X_FORWARDED_FOR"]);
} else {
    $ip = $_SERVER["REMOTE_ADDR"];
}


//We need to give the user a unique cookie and make it expire in 5 years.
if (!array_key_exists(COOKIE_NAME, $_COOKIE)) {
    setcookie(COOKIE_NAME, $ip . '.' . microtime(true), time() + 157784630, '/', COOKIE_DOMAIN);
}

// Find everything between our CWD and 255 in QUERY_STRING.
$rawPath = substr(urldecode($_SERVER['QUERY_STRING']),5,255);

// Munge he resulting string and store it in $path.
$path = explode('/',$rawPath);

// Determine incoming request and clean inputs.
$clean = Array();
$clean['updateVersion'] = isset($path[0]) ? intval($path[0]) : null;
$clean['product'] = isset($path[1]) ? trim($path[1]) : null;
$clean['version'] = isset($path[2]) ? urlencode($path[2]) : null;
$clean['build'] = isset($path[3]) ? trim($path[3]) : null;
$clean['platform'] = isset($path[4]) ? trim($path[4]) : null;
$clean['locale'] = isset($path[5]) ? trim($path[5]) : null;
$clean['channel'] = isset($path[6]) ? trim($path[6]) : null;
$clean['platformVersion'] = isset($path[7]) ? trim($path[7]) : null;
$clean['dist'] = isset($path[8]) ? trim($path[8]) : null;
$clean['distVersion'] = isset($path[9]) ? trim($path[9]) : null;

// Check to see if we have a beta on PPC, and if so don't update since beta 4
// doesn't work on PPC.  See bug 588412.
if ( $clean['product'] == 'Firefox'
     && (preg_match('/^4\.0.*$/', $clean['version']) || preg_match('/^3\.7.*$/', $clean['version']))
     && strpos($_SERVER['HTTP_USER_AGENT'], 'PPC')) {
        $xml->printXml();
        exit;
}

 
// Check to see if the user is explicitly requesting an update.  If they are,
// skip throttling.  If they aren't, and throttling is enabled, first check
// explicit throttling.  If no specific rules exist, fallback to global rules.
// In either case, updates will be served based on the configured random seed.
if ( (empty($_GET['force']) || $_GET['force']!=1) ) {

    // Default to false.  If conditions are met, flip it.
    $throttleMe = false;

    $aus = new AUS_Object();

    // Check explicit throttling.
    if ( !$aus->isThrottleException($clean['version'], $clean['channel'])
         && isset($productThrottling[$clean['product']][$clean['version']]) 
         && mt_rand(0,99) >= $productThrottling[$clean['product']][$clean['version']]
         ) {
        $throttleMe = true;

    // Check global throttling.
    } elseif ( defined('THROTTLE_GLOBAL') && THROTTLE_GLOBAL && 
      defined('THROTTLE_LEVEL') &&
      mt_rand(0,99) >= THROTTLE_LEVEL ) {
        $throttleMe = true;
    }

    if ($throttleMe) {
        if (defined('THROTTLE_LOGGING') && THROTTLE_LOGGING) {
            error_log('AUS2 THROTTLE: '.$ip.' '.$_SERVER['REQUEST_URI']);
        }

        $xml->printXml();
        exit;
    }
}

// Connect to memcache and try to pull output.
$memcache = new Memcaching();
$_cached_xml = $memcache->get($rawPath);

if ($_cached_xml) {
    $xml = $_cached_xml;
} else {
    /**
     * For each updateVersion, we will run separate code when it differs.
     * Our case statements below are ordered by date added, desc (recent ones first).
     *
     * If the only difference is an added parameter, etc., then we may blend cases
     * together by omitting a break.
     */
    switch ($clean['updateVersion']) {
        
        /*
         * This is for the fifth revision, adding %PLATFORM_VERSION%.
         * /update/4/%PRODUCT%/%VERSION%/%BUILD_ID%/%BUILD_TARGET%/%LOCALE%/%CHANNEL%/%OS_VERSION%/%DISTRIBUTION%/%DISTRIBUTION_VERSION%/%PLATFORM_VERSION%/update.xml
         */
        case 4:

            // Nothing special.
        
        /*
         * This is for the fourth revision, adding %DISTRIBUTION% and
         * %DISTRIBUTION_VERSION%.
         * /update/3/%PRODUCT%/%VERSION%/%BUILD_ID%/%BUILD_TARGET%/%LOCALE%/%CHANNEL%/%OS_VERSION%/%DISTRIBUTION%/%DISTRIBUTION_VERSION%/update.xml
         */
        case 3:

            // Nothing special.
       
        /*
         * This is for the third revision of the URI schema, with %OS_VERSION%.
         * /update2/2/%PRODUCT%/%VERSION%/%BUILD_ID%/%BUILD_TARGET%/%LOCALE%/%CHANNEL%/%OS_VERSION%/update.xml
         */
        case 2:

            // Check for OS_VERSION values and scrub the URI to make sure we aren't getting a malformed request
            // from a client suffering from bug 360127.
            if (empty($clean['platformVersion']) || $clean['platformVersion']=='%OS_VERSION%' || preg_match('/^1\.5.*$/',$clean['version'])) {
                break;
            }

        /*
         * This is for the second revision of the URI schema, with %CHANNEL% added.
         * /update2/1/%PRODUCT%/%VERSION%/%BUILD_ID%/%BUILD_TARGET%/%LOCALE%/%CHANNEL%/update.xml
         */
        case 1:

            // Set a default for platformVersion.
            if (empty($clean['platformVersion'])) {
                $clean['platformVersion'] = null;
            }

            // Check for a set channel.

            // Instantiate Update object and set updateVersion.
            $update = new Update();

            // Instantiate our complete patch.
            $completePatch = new Patch($productBranchVersions,$nightlyChannels,'complete');

            // If our complete patch exists and is valid, set the patch line.
            if ($completePatch->findPatch($clean['product'],$clean['platform'],$clean['locale'],$clean['version'],$clean['build'],$clean['channel']) && $completePatch->isPatch()) {
                
                // Set our patchLine.
                $xml->setPatchLine($completePatch);

                // If available, pull update information from the build snippet.
                if ($completePatch->hasUpdateInfo()) {
                    $update->setVersion($completePatch->updateVersion);
                    $update->setExtensionVersion($completePatch->updateExtensionVersion);
                    $update->setBuild($completePatch->build);
                }

                // If there is details url information, add it to the update object.
                if ($completePatch->hasDetailsUrl()) {
                    $update->setDetails($completePatch->detailsUrl);
                }

                // If we found an update type, pass it along.
                if ($completePatch->hasUpdateType()) {
                    $update->setType($completePatch->updateType);
                }

                // If we have a license URL, pass it along.
                if ($completePatch->hasLicenseUrl()) {
                    $update->setLicense($completePatch->licenseUrl);
                }
            }

            // Instantiate our partial patch.
            $partialPatch = new Patch($productBranchVersions,$nightlyChannels,'partial');

            // If our partial patch exists and is valid, set the patch line.
            if ($partialPatch->findPatch($clean['product'],$clean['platform'],$clean['locale'],$clean['version'],$clean['build'],$clean['channel']) 
                  && $partialPatch->isPatch() 
                  && $partialPatch->isOneStepFromLatest($completePatch->build)) {
                $xml->setPatchLine($partialPatch);
             }

            // If we have valid patchLine(s), set up our output.
            if ($xml->hasPatchLine() && $completePatch->isSupported($completePatch->updateType, $clean['product'], $clean['version'], $clean['platformVersion'], $unsupportedPlatforms)) {
                $xml->startUpdate($update);
                $xml->drawPatchLines();
                $xml->endUpdate();
            }
            break;

        /*
         * This is for the first revision of the URI schema.
         * /update2/0/%PRODUCT%/%VERSION%/%BUILD_ID%/%BUILD_TARGET%/%LOCALE%/update.xml
         */
        case 0:
        default:

            // Instantiate Update object and set updateVersion.
            $update = new Update();

            // Instantiate Patch object and set Path based on passed args.
            $patch = new Patch($productBranchVersions,$nightlyChannels,'complete');

            $patch->findPatch($clean['product'],$clean['platform'],$clean['locale'],$clean['version'],$clean['build'],null);

            if ($patch->isPatch()) {
                $xml->setPatchLine($patch);
            }

            // If we have a new build, draw the update block and patch line.
            // If there is no valid patch file, client will receive no updates by default.
            if ($xml->hasPatchLine()) {
                $xml->startUpdate($update);
                $xml->drawPatchLines();
                $xml->endUpdate();
            }
            break;
    }
}

// If we are debugging output plaintext and exit.
if ( defined('DEBUG') && DEBUG == true ) {

    echo '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">'."\n";
    echo '<html xmlns="http://www.w3.org/1999/xhtml">'."\n";
    echo '<head>'."\n";
    echo '<title>AUS Debug Information</title>'."\n";
    echo '</head>'."\n";
    echo '<body>'."\n";
    echo '<h1>AUS Debug Information</h1>'."\n";

    echo '<h2>XML Output</h2>'."\n";
    echo '<pre>'."\n";
    echo htmlentities($xml->getOutput());
    echo '</pre>'."\n";

    if (!empty($clean)) {
        echo '<h2>Inputs</h2>'."\n";
        echo '<pre>'."\n";
        print_r($clean);
        echo '</pre>'."\n";
    } 
    
    echo '<h2>Patch Objects</h2>'."\n";
    echo '<pre>'."\n";
    if (!empty($patch)) {
        print_r($patch);
    }
    if (!empty($completePatch)) {
        print_r($completePatch);
    }
    if (!empty($partialPatch)) {
        print_r($partialPatch);
    }
    echo '</pre>'."\n";

    if (!empty($update)) {
        echo '<h2>Update Object</h2>'."\n";
        echo '<pre>'."\n";
        print_r($update);
        echo '</pre>'."\n";
    }

    echo '</body>'."\n";
    echo '</html>';

    exit;
}

// Set header and send info.
$memcache->set($rawPath,$xml);
$xml->printXml();
exit;
?>
