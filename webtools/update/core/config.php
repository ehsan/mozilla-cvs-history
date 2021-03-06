<?php
// ***** BEGIN LICENSE BLOCK *****
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
// The Original Code is Mozilla Update.
//
// The Initial Developer of the Original Code is
// Chris "Wolf" Crews.
// Portions created by the Initial Developer are Copyright (C) 2004
// the Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Chris "Wolf" Crews <psychoticwolf@carolina.rr.com>
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

// ******  Mozilla Update -- Configuration File  ******
// All common PHP Variables/functions are defined here


// MySQL Server Configuration Variables
include "dbconfig.php"; // Include Database Server Configuration File

// General Website Configuration Variables

// Local Path to Site Files
$websitepath =    "/opt/update-beta";

// Path to XPI/JAR Respository
$repositorypath = "/opt/update-beta/files/";

// DNS Hostname, ex. "update.mozilla.org"
$sitehostname =   $_SERVER["SERVER_NAME"];

// URL to FTP site
$ftpurl =         "http://ftp.mozilla.org/pub/mozilla.org";

// Page Header and Footer Path Variables

// Path to Page Header on Disk
$page_header = "$websitepath/core/inc_header.php";

// Path to Page Footer on Disk
$page_footer = "$websitepath/core/inc_footer.php";

//Function: getmicrotime() - Page Load Timing Debug Function
function getmicrotime() {
  list($usec, $sec) = explode(" ", microtime());

  return ((float)$usec + (float)$sec);
}
$time_start = getmicrotime();

// Update Core Include Files
include "inc_guids.php"; // GUID --> AppName Handler
include "inc_global.php"; // Global Functions - Variable Cleanup
include "inc_browserdetection.php"; //Browser Detection - App Variable Handling
?>
