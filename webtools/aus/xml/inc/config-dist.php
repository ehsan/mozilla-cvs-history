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
 * Configuration file.
 * @package aus
 * @subpackage inc
 * @author Mike Morgan
 */
// define('SOURCE_DIR','/home/morgamic/public_html/auslite/source');
define('SOURCE_DIR',getcwd().'/data');

// This is the directory containin channel-specific updates.
// Snippets in this directory override normal updates.
define('OVERRIDE_DIR',getcwd().'/data/3');

// Uncomment this line in order to echo text debug information.
define('DEBUG',false);

// Define default for Update blocks.
define('UPDATE_TYPE','minor');
define('UPDATE_VERSION','1.0+');
define('UPDATE_EXTENSION_VERSION','1.0+');

// Define cookie parameters
define('COOKIE_NAME', 'aus2');
define('COOKIE_DOMAIN', 'aus2.mozilla.org');

// Turns global throttling on and off.
define('THROTTLE_GLOBAL',false);

// Define the throttle -- think gas pedal.  This determines how much the AUS application will
// stagger updates.  Pedal to the floor means we're serving all updates.  Examples:
//  10  = Updates offered only 10% of the time.
//  80  = Updates offered 80% of the time.
//  100 = Updates always offered.
define('THROTTLE_LEVEL',100);

// Turns logging throttled hits on and off.
define('THROTTLE_LOGGING',false);

// This defines explicit throttling levels.  If global throttling is on, these
// override global levels.  If it is off, this still works.  For example, this
// is 10% throttling (only 10% of the time updates are offered):
//
// $productThrottling = array(
//     'Firefox' => array(
//         '3.0' => array(
//             'release' => 10 
//         )
//     )
// );
$productThrottling = array(
     'Firefox' => array(
         '3.0.11' => array(
             'release' => 0,
             'beta' => 0,
             'releasetest' => 0,
             'betatest' => 0,
             'release-cck-acer-google' => 0,
             'release-cck-allpeers' => 0,
             'release-cck-allpeers-cck-allpeers' => 0,
             'release-cck-allpeers-ebay' => 0,
             'release-cck-allpeers-google' => 0,
             'release-cck-allpeers-mozilla05' => 0,
             'release-cck-allpeers-mozilla06' => 0,
             'release-cck-allpeers-tonline' => 0,
             'release-cck-allpeers-yahoo' => 0,
             'release-cck-allpeers-yahooca' => 0,
             'release-cck-allpeers-yahoode' => 0,
             'release-cck-allpeers-yahoomy' => 0,
             'release-cck-allpeers-yandex' => 0,
             'release-cck-baidu' => 0,
             'release-cck-bby' => 0,
             'release-cck-campus-divx-google-mozilla' => 0,
             'release-cck-campus-ebay-google-mozilla' => 0,
             'release-cck-campus-ebay-mozilla' => 0,
             'release-cck-campus-google-mozilla' => 0,
             'release-cck-campus-google-mozilla-packardbell' => 0,
             'release-cck-campus-google-mozilla-realnetworks' => 0,
             'release-cck-campus-googleggic-mozilla' => 0,
             'release-cck-campus-googlegzfb-mozilla' => 0,
             'release-cck-campus-mozilla' => 0,
             'release-cck-campus-mozilla-mozilla01' => 0,
             'release-cck-campus-mozilla-yahoo' => 0,
             'release-cck-cybermentors' => 0,
             'release-cck-daum' => 0,
             'release-cck-divx-ebay-google' => 0,
             'release-cck-divx-google' => 0,
             'release-cck-divx-google-kodakgallery' => 0,
             'release-cck-divx-google-mozilla01' => 0,
             'release-cck-divx-google-mozilla02' => 0,
             'release-cck-divx-google-mozilla03' => 0,
             'release-cck-divx-google-mozilla04' => 0,
             'release-cck-divx-google-mozilla05' => 0,
             'release-cck-divx-google-mozilla06' => 0,
             'release-cck-divx-google-mozilla07' => 0,
             'release-cck-divx-google-packardbell' => 0,
             'release-cck-divx-google-realnetworks' => 0,
             'release-cck-divx-google-realnetworks-yahoo' => 0,
             'release-cck-divx-google-seznam' => 0,
             'release-cck-divx-google-tonline' => 0,
             'release-cck-divx-google-yahoo' => 0,
             'release-cck-divx-google-yahooca' => 0,
             'release-cck-divx-google-yahoode' => 0,
             'release-cck-divx-google-yahoogb' => 0,
             'release-cck-divx-google-yandex' => 0,
             'release-cck-divx-googleggic' => 0,
             'release-cck-divx-googlegzfb' => 0,
             'release-cck-eCompetenceCenter' => 0,
             'release-cck-ebay' => 0,
             'release-cck-ebay-google' => 0,
             'release-cck-ebay-google-packardbell' => 0,
             'release-cck-ebay-google-realnetworks' => 0,
             'release-cck-ebay-googleggic' => 0,
             'release-cck-ebay-googlegzfb' => 0,
             'release-cck-ebay-kodakgallery' => 0,
             'release-cck-ebay-mozilla01' => 0,
             'release-cck-ebay-mozilla02' => 0,
             'release-cck-ebay-tonline' => 0,
             'release-cck-ebay-yahoode' => 0,
             'release-cck-ebay-yahoogb' => 0,
             'release-cck-false' => 0,
             'release-cck-fashionstreet.ru' => 0,
             'release-cck-fujitsusiemens-google' => 0,
             'release-cck-google' => 0,
             'release-cck-google-' => 0,
             'release-cck-google-kodakgallery' => 0,
             'release-cck-google-kodakgallery-packardbell' => 0,
             'release-cck-google-kodakgallery-realnetworks' => 0,
             'release-cck-google-kodakgallery-yahoo' => 0,
             'release-cck-google-mozilla-personas' => 0,
             'release-cck-google-mozilla01' => 0,
             'release-cck-google-mozilla01-realnetworks' => 0,
             'release-cck-google-mozilla02' => 0,
             'release-cck-google-mozilla02-packardbell' => 0,
             'release-cck-google-mozilla02-realnetworks' => 0,
             'release-cck-google-mozilla03' => 0,
             'release-cck-google-mozilla05-realnetworks' => 0,
             'release-cck-google-mozilla06-realnetworks' => 0,
             'release-cck-google-mozilla07' => 0,
             'release-cck-google-mozillaonline' => 0,
             'release-cck-google-nttresonant' => 0,
             'release-cck-google-nttresonant-realnetworks' => 0,
             'release-cck-google-packardbell' => 0,
             'release-cck-google-packardbell-realnetworks' => 0,
             'release-cck-google-packardbell-seznam' => 0,
             'release-cck-google-packardbell-tonline' => 0,
             'release-cck-google-packardbell-yahoo' => 0,
             'release-cck-google-packardbell-yahoofr' => 0,
             'release-cck-google-realnetworks' => 0,
             'release-cck-google-realnetworks-seznam' => 0,
             'release-cck-google-realnetworks-tonline' => 0,
             'release-cck-google-realnetworks-yahoo' => 0,
             'release-cck-google-realnetworks-yahooca' => 0,
             'release-cck-google-realnetworks-yahoogb' => 0,
             'release-cck-google-realnetworks-yahooin' => 0,
             'release-cck-google-realnetworks-yahooserp' => 0,
             'release-cck-google-realnetworks-yandex' => 0,
             'release-cck-google-seznam' => 0,
             'release-cck-google-tonline' => 0,
             'release-cck-google-yahoo' => 0,
             'release-cck-google-yahooca' => 0,
             'release-cck-google-yahoode' => 0,
             'release-cck-google-yahoofr' => 0,
             'release-cck-google-yahoogb' => 0,
             'release-cck-google-yahooserp' => 0,
             'release-cck-google-yahootw' => 0,
             'release-cck-google-yahootwkimoo' => 0,
             'release-cck-google.de' => 0,
             'release-cck-googlede' => 0,
             'release-cck-googleggic' => 0,
             'release-cck-googleggic-kodakgallery' => 0,
             'release-cck-googleggic-kodakgallery-realnetworks' => 0,
             'release-cck-googleggic-mozilla01' => 0,
             'release-cck-googleggic-packardbell' => 0,
             'release-cck-googleggic-realnetworks' => 0,
             'release-cck-googleggic-tonline' => 0,
             'release-cck-googleggic-yahoo' => 0,
             'release-cck-googleggic-yahoode' => 0,
             'release-cck-googlegzfb' => 0,
             'release-cck-googlegzfb-kodakgallery' => 0,
             'release-cck-googlegzfb-mozilla01' => 0,
             'release-cck-googlegzfb-mozilla02' => 0,
             'release-cck-googlegzfb-realnetworks' => 0,
             'release-cck-googlegzfb-tonline' => 0,
             'release-cck-googlegzfb-yahoo' => 0,
             'release-cck-googlegzfb-yahooca' => 0,
             'release-cck-googlegzfb-yahoode' => 0,
             'release-cck-kodakgallery' => 0,
             'release-cck-kodakgallery-mozilla03' => 0,
             'release-cck-mozcptrgame' => 0,
             'release-cck-mozilla-personas' => 0,
             'release-cck-mozilla-rock07' => 0,
             'release-cck-mozilla01' => 0,
             'release-cck-mozilla01-mozilla02' => 0,
             'release-cck-mozilla01-mozilla04' => 0,
             'release-cck-mozilla01-tonline' => 0,
             'release-cck-mozilla02' => 0,
             'release-cck-mozilla03' => 0,
             'release-cck-mozilla04' => 0,
             'release-cck-mozilla05' => 0,
             'release-cck-mozilla06' => 0,
             'release-cck-mozilla07' => 0,
             'release-cck-mozilla07-seznam' => 0,
             'release-cck-mozilla08' => 0,
             'release-cck-mozillaonline' => 0,
             'release-cck-naver' => 0,
             'release-cck-nobody' => 0,
             'release-cck-nttresonant' => 0,
             'release-cck-nttresonant-yahoo' => 0,
             'release-cck-portal20' => 0,
             'release-cck-realnetworks' => 0,
             'release-cck-realnetworks-cck-campus-mozilla' => 0,
             'release-cck-realnetworks-cck-divx-google' => 0,
             'release-cck-realnetworks-cck-divx-google-realnetworks' => 0,
             'release-cck-realnetworks-cck-divx-googlegzfb' => 0,
             'release-cck-realnetworks-cck-ebay' => 0,
             'release-cck-realnetworks-cck-ebay-google' => 0,
             'release-cck-realnetworks-cck-google' => 0,
             'release-cck-realnetworks-cck-google-kodakgallery' => 0,
             'release-cck-realnetworks-cck-google-mozilla01' => 0,
             'release-cck-realnetworks-cck-google-mozilla01-realnetworks' => 0,
             'release-cck-realnetworks-cck-google-packardbell' => 0,
             'release-cck-realnetworks-cck-google-realnetworks' => 0,
             'release-cck-realnetworks-cck-google-yahoo' => 0,
             'release-cck-realnetworks-cck-googleggic' => 0,
             'release-cck-realnetworks-cck-googleggic-realnetworks' => 0,
             'release-cck-realnetworks-cck-googlegzfb' => 0,
             'release-cck-realnetworks-cck-googlegzfb-realnetworks' => 0,
             'release-cck-realnetworks-cck-googlegzfb-yahooserp' => 0,
             'release-cck-realnetworks-cck-kodakgallery' => 0,
             'release-cck-realnetworks-cck-mozilla01' => 0,
             'release-cck-realnetworks-cck-mozilla02' => 0,
             'release-cck-realnetworks-cck-mozilla03' => 0,
             'release-cck-realnetworks-cck-mozillaonline' => 0,
             'release-cck-realnetworks-cck-nttresonant' => 0,
             'release-cck-realnetworks-cck-seznam' => 0,
             'release-cck-realnetworks-cck-tonline' => 0,
             'release-cck-realnetworks-cck-yahoo' => 0,
             'release-cck-realnetworks-cck-yahooca' => 0,
             'release-cck-realnetworks-cck-yahoogb' => 0,
             'release-cck-realnetworks-cck-yahooserp' => 0,
             'release-cck-seznam' => 0,
             'release-cck-seznam-yandex' => 0,
             'release-cck-thome' => 0,
             'release-cck-tonline' => 0,
             'release-cck-yahoo' => 0,
             'release-cck-yahoo-yahooca' => 0,
             'release-cck-yahoo-yahootwkimoo' => 0,
             'release-cck-yahooaa' => 0,
             'release-cck-yahooau' => 0,
             'release-cck-yahooca' => 0,
             'release-cck-yahoode' => 0,
             'release-cck-yahooes' => 0,
             'release-cck-yahoofr' => 0,
             'release-cck-yahoogb' => 0,
             'release-cck-yahooid' => 0,
             'release-cck-yahooin' => 0,
             'release-cck-yahooit' => 0,
             'release-cck-yahoomy' => 0,
             'release-cck-yahoonz' => 0,
             'release-cck-yahoonzserp' => 0,
             'release-cck-yahooph' => 0,
             'release-cck-yahooserp' => 0,
             'release-cck-yahoosg' => 0,
             'release-cck-yahooth' => 0,
             'release-cck-yahootw' => 0,
             'release-cck-yahootwkimoo' => 0,
             'release-cck-yahoovn' => 0,
             'release-cck-yandex' => 0,
             'release-cck-zyahoogb' => 0,
             'release-google-cck-packardbell' => 0,
             'release-google-cck-packardbell-cck-divx-google' => 0,
             'release-google-cck-packardbell-cck-ebay' => 0,
             'release-google-cck-packardbell-cck-google' => 0,
             'release-google-cck-packardbell-cck-google-mozilla01-packardbell' => 0,
             'release-google-cck-packardbell-cck-google-packardbell' => 0,
             'release-google-cck-packardbell-cck-google-realnetworks' => 0,
             'release-google-cck-packardbell-cck-googleggic' => 0,
             'release-google-cck-packardbell-cck-googlegzfb' => 0,
             'release-google-cck-packardbell-cck-googlegzfb-packardbell' => 0,
             'release-google-cck-packardbell-cck-kodakgallery' => 0,
             'release-google-cck-packardbell-cck-mozilla01' => 0,
             'release-google-cck-packardbell-cck-tonline' => 0,
             'release-google-cck-realnetworks' => 0,
             'release-google-cck-realnetworks-cck-allpeers' => 0,
             'release-google-cck-realnetworks-cck-campus-mozilla' => 0,
             'release-google-cck-realnetworks-cck-divx-google' => 0,
             'release-google-cck-realnetworks-cck-divx-google-realnetworks' => 0,
             'release-google-cck-realnetworks-cck-divx-google-yahoo' => 0,
             'release-google-cck-realnetworks-cck-divx-googleggic' => 0,
             'release-google-cck-realnetworks-cck-divx-googlegzfb' => 0,
             'release-google-cck-realnetworks-cck-ebay' => 0,
             'release-google-cck-realnetworks-cck-ebay-google' => 0,
             'release-google-cck-realnetworks-cck-ebay-google-realnetworks' => 0,
             'release-google-cck-realnetworks-cck-ebay-googlegzfb' => 0,
             'release-google-cck-realnetworks-cck-fujitsusiemens-google' => 0,
             'release-google-cck-realnetworks-cck-google' => 0,
             'release-google-cck-realnetworks-cck-google-kodakgallery-realnetworks' => 0,
             'release-google-cck-realnetworks-cck-google-packardbell' => 0,
             'release-google-cck-realnetworks-cck-google-realnetworks' => 0,
             'release-google-cck-realnetworks-cck-google-realnetworks-tonline' => 0,
             'release-google-cck-realnetworks-cck-google-realnetworks-yahoo' => 0,
             'release-google-cck-realnetworks-cck-google-tonline' => 0,
             'release-google-cck-realnetworks-cck-google-yahoo' => 0,
             'release-google-cck-realnetworks-cck-googleggic' => 0,
             'release-google-cck-realnetworks-cck-googleggic-realnetworks' => 0,
             'release-google-cck-realnetworks-cck-googleggic-tonline' => 0,
             'release-google-cck-realnetworks-cck-googleggic-yahoo' => 0,
             'release-google-cck-realnetworks-cck-googlegzfb' => 0,
             'release-google-cck-realnetworks-cck-googlegzfb-realnetworks' => 0,
             'release-google-cck-realnetworks-cck-googlegzfb-tonline' => 0,
             'release-google-cck-realnetworks-cck-kodakgallery' => 0,
             'release-google-cck-realnetworks-cck-mozilla-rock07' => 0,
             'release-google-cck-realnetworks-cck-mozilla01' => 0,
             'release-google-cck-realnetworks-cck-mozilla02' => 0,
             'release-google-cck-realnetworks-cck-mozillaonline' => 0,
             'release-google-cck-realnetworks-cck-nttresonant' => 0,
             'release-google-cck-realnetworks-cck-seznam' => 0,
             'release-google-cck-realnetworks-cck-tonline' => 0,
             'release-google-cck-realnetworks-cck-yahoo' => 0,
             'release-google-cck-realnetworks-cck-yahooca' => 0,
             'release-google-cck-realnetworks-cck-yandex' => 0,
             'release-google-cck-sealoetworks-cck-google' => 0
         )
     )
);

// These are channels that have access to nightly updates.
// All other channels only have access to the OVERRIDE_DIR for update info.
$nightlyChannels = array(
    'nightly'
);

// This hash defines the product->version->patch relationships for nightlies
// It determines which patches are associated to which incoming client versions.
// @todo replace this with a better datasource that can be easily managed via a GUI.
$productBranchVersions = array(
    'Firefox'     =>  array(
        '2.0*'    =>  '2.0',
        '3.0*'   =>  'trunk',
        '3.1*'    => 'mozilla-1.9.1',
        '3.5*'    => 'mozilla-1.9.1',
        '3.2*'    => 'mozilla-central',
        '3.6*'    => 'mozilla-central',
    ),
    'Thunderbird' =>  array(
        '1.5.0.*' =>  '1.5.0.x',
        '2.0*'    =>  '2.0',
        '3.0*'    =>  'trunk'
    ),
    'Sunbird'     =>  array(
        '0.4a1'   =>  'branch',
        '0.5*'    =>  'branch',
        '0.7*'    =>  'branch',
        '0.8*'    =>  'branch',
        '0.9*'    =>  'branch',
        '0.6a1'   =>  'trunk'
    )  // Add this for tests: 'Synthetic'   => array('1.5.0.*' => '1.5.0.x')
);

// Config for memcache.
define('MEMCACHE_NAMESPACE', 'aus'); // set memcache namespace.  Keep this string as short and simple as possible.
define('MEMCACHE_EXPIRE', 1800); // how long items are stored in memcache
define('MEMCACHE_ON', false); // whether or not to cache ever

/**
 * Memcache configuration.
 * See http://php.oregonstate.edu/memcache for info.
 */
$memcache_config = array(
    'localhost' => array(
       'port' => '11211',
       'persistent' => true,
       'weight' => '1',
       'timeout' => '1',
       'retry_interval' => 15
    )
);

/*
 * Array that defines which %OS_VERSION% values are no longer supported.
 * For incoming URIs containing these as their platformVersion, no updates
 * will be offered.  As of bug 418129, this has to be branch-specific and aware
 * of whether or not an update is major.  Use of this array is in
 * inc/patch.class.php.  
 *
 * Array format has changed, and is considered to be:

 * array(
 *      $Product => array(
 *          $Version => array(
 *              $OS_VERSION
 *          )
 *      )
 *  )
 *
 * $Product is the product name (Firefox, Thunderbird, etc. - %PRODUCT%).
 *
 * $Version is the client version in the URL (%VERSION%).
 *
 * $OS_VERSION is used in a string match (existence anywhere in passed
 * %OS_VERSION% triggers blocklisting of that OS).
 */
$unsupportedPlatforms = array(
    'Firefox'     =>  array( // Change to Synthetic for tests
        '2.0*' => array( // Change to 1.0* for tests
            'Darwin 6',
            'Darwin 7',
            'Windows_95',
            'Windows_98',
            'Windows_NT 4',
            'GTK 2.0.',
            'GTK 2.1.',
            'GTK 2.2.',
            'GTK 2.3.',
            'GTK 2.4.',
            'GTK 2.5.',
            'GTK 2.6.',
            'GTK 2.7.',
            'GTK 2.8.',
            'GTK 2.9.'
        )
    )
);
?>
