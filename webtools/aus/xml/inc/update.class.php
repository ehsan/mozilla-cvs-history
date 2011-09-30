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
 * @package aus
 * @subpackage inc
 * @author Mike Morgan
 */
class Update extends AUS_Object {
    var $type;
    var $version;
    var $extensionVersion;
    var $build;
    var $details;
    var $license;
    var $billboard;
    var $showPrompt;
    var $showNeverForVersion;
    var $showSurvey;
    var $actions;
    var $open;
    var $notification;
    var $alert;
    var $displayVersion;
    var $appVersion;
    var $platformVersion;

    /**
     * Default constructor.
     */
    function Update($type=UPDATE_TYPE,$version=UPDATE_VERSION,$extensionVersion=UPDATE_EXTENSION_VERSION) {
        $this->setType($type);
        $this->setVersion($version);
        $this->setExtensionVersion($extensionVersion);
    }

    /**
     * Set type.
     * @param string $type
     */
     function setType($type) {
        $this->type = $type;
     }

    /**
     * Set verison.
     * @param string $type
     */
     function setVersion($version) {
        $this->version = $version;
     }

    /**
     * Set extensionVersion.
     * @param string $extensionVersion
     */
     function setExtensionVersion($extensionVersion) {
        $this->extensionVersion = $extensionVersion;
     }    

     /**
      * Set the build.
      * @param string $build
      */
     function setBuild($build) {
        return $this->build = $build;
     }

     /**
      * Set the details URL.
      * @param string $details
      */
     function setDetails($details) {
        return $this->details = $details;
     }

     /**
      * Set the license URL.
      * @param string $license
      */
     function setLicense($license) {
        return $this->license = $license;
     }

     /**
      * Set the billboard URL.
      * @param string $billboard
      */
     function setBillboard($billboard) {
        return $this->billboard = $billboard;
     }

     /**
      * Set showPrompt.
      * @param string $showPrompt
      */
     function setShowPrompt($showPrompt) {
        return $this->showPrompt = $showPrompt;
     }

     /**
      * Set showNeverForVersion.
      * @param string $showNeverForVersion
      */
     function setShowNeverForVersion($showNeverForVersion) {
        return $this->showNeverForVersion = $showNeverForVersion;
     }

     /**
      * Set showSurvey.
      * @param string $showSurvey
      */
     function setShowSurvey($showSurvey) {
        return $this->showSurvey = $showSurvey;
     }

     /**
      * Set actions.
      * @param string $actions
      */
     function setActions($actions) {
        return $this->actions = $actions;
     }

     /**
      * Set the open URL.
      * @param string $open
      */
     function setOpen($open) {
        return $this->open = $open;
     }

     /**
      * Set the notification URL.
      * @param string $notification
      */
     function setNotification($notification) {
        return $this->notification = $notification;
     }

     /**
      * Set the alert URL.
      * @param string $alert
      */
     function setAlert($alert) {
        return $this->alert = $alert;
     }

     /**
      * Set the displayVersion.
      * @param string $displayVersion
      */
     function setDisplayVersion($displayVersion) {
        return $this->displayVersion = $displayVersion;
     }

     /**
      * Set the appVersion.
      * @param string $appVersion
      */
     function setAppVersion($appVersion) {
        return $this->appVersion = $appVersion;
     }

     /**
      * Set the platformVersion.
      * @param string $platformVersion
      */
     function setPlatformVersion($platformVersion) {
        return $this->platformVersion = $platformVersion;
     }

     /**
      * Set the snippet schema version.
      * @param string $snippetSchemaVersion
      */
     function setSnippetSchemaVersion($snippetSchemaVersion) {
        return $this->snippetSchemaVersion = $snippetSchemaVersion;
     }

     /**
      * Get type.
      * @return string
      */
     function getType() {
        return $this->type;
     }

     /**
      * Get version.
      * @return string
      */
     function getVersion() {
        return $this->version;
     }

     /**
      * Get extension version.
      * @return string
      */
     function getExtensionVersion() {
        return $this->extensionVersion;
     }
}
?>
