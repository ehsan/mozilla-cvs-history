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
class Xml extends AUS_Object {
    var $xmlOutput;
    var $xmlHeader;
    var $xmlFooter;
    var $xmlPatchLines;

    /**
     * Constructor, sets overall header and footer.
     */
    function Xml() {
        $this->xmlHeader = '<?xml version="1.0"?>'."\n".'<updates>';
        $this->xmlFooter = "\n".'</updates>';
        $this->xmlOutput = $this->xmlHeader;
        $this->xmlPatchLines = '';
    }

    /**
     * Start an update block.
     * @param object $update
     */
    function startUpdate($update) {
        $snippetSchemaVersion = htmlentities($update->snippetSchemaVersion);
        $type = htmlentities($update->type);

        if ($snippetSchemaVersion == SNIPPET_SCHEMA_VER_2) {
            $displayVersion = isset($update->displayVersion) ? htmlentities($update->displayVersion) : '';
            $appVersion = isset($update->appVersion) ? htmlentities($update->appVersion) : '';
            $platformVersion = isset($update->platformVersion) ? htmlentities($update->platformVersion) : '';
        } else {
            $version = isset($update->version) ? htmlentities($update->version) : '';
            $extensionVersion = isset($update->extensionVersion) ? htmlentities($update->extensionVersion) : '';
        }

        $build = isset($update->build) ? htmlentities($update->build) : '';
        $details = htmlentities($update->details);
        $license = htmlentities($update->license);
        $billboard = isset($update->billboard) ? htmlentities($update->billboard) : '';
        $showPrompt = isset($update->showPrompt) ? htmlentities($update->showPrompt) : '';
        $showNeverForVersion = isset($update->showNeverForVersion) ? htmlentities($update->showNeverForVersion) : '';
        $showSurvey = isset($update->showSurvey) ? htmlentities($update->showSurvey) : '';
        $actions = isset($update->actions) ? htmlentities($update->actions) : '';
        $open = isset($update->open) ? htmlentities($update->open) : '';
        $notification = isset($update->notification) ? htmlentities($update->notification) : '';
        $alert = isset($update->alert) ? htmlentities($update->alert) : '';

        $version_xml = '';
        if (!empty($version)) {
            $version_xml = " version=\"{$version}\"";
        }

        $extensionVersion_xml= '';
        if (!empty($extensionVersion)) {
            $extensionVersion_xml = " extensionVersion=\"{$extensionVersion}\"";
        }

        $displayVersion_xml = '';
        if (!empty($displayVersion)) {
            $displayVersion_xml = " displayVersion=\"{$displayVersion}\"";
        }

        $appVersion_xml = '';
        if (!empty($appVersion)) {
            $appVersion_xml = " appVersion=\"{$appVersion}\"";
        }

        $platformVersion_xml = '';
        if (!empty($platformVersion)) {
            $platformVersion_xml = " platformVersion=\"{$platformVersion}\"";
        }

        $build_xml = '';
        if (!empty($build)) {
            $build_xml = " buildID=\"{$build}\"";
        }

        $details_xml = '';
        if (!empty($details)) {
            $details_xml = " detailsURL=\"{$details}\"";
        }

        $license_xml = '';
        if (!empty($license)) {
            $license_xml = " licenseURL=\"{$license}\"";
        }

        $billboard_xml = '';
        if (!empty($billboard)) {
            $billboard_xml = " billboardURL=\"{$billboard}\"";
        }

        $showPrompt_xml = '';
        if (!empty($showPrompt)) {
            $showPrompt_xml = " showPrompt=\"{$showPrompt}\"";
        }

        $showNeverForVersion_xml = '';
        if (!empty($showNeverForVersion)) {
            $showNeverForVersion_xml = " showNeverForVersion=\"{$showNeverForVersion}\"";
        }

        $showSurvey_xml = '';
        if (!empty($showSurvey)) {
            $showSurvey_xml = " showSurvey=\"{$showSurvey}\"";
        }

        $actions_xml = '';
        if (!empty($actions)) {
            $actions_xml = " actions=\"{$actions}\"";
        }

        $open_xml = '';
        if (!empty($open)) {
            $open_xml = " openURL=\"{$open}\"";
        }

        $notification_xml = '';
        if (!empty($notification)) {
            $notification_xml = " notificationURL=\"{$notification}\"";
        }

        $alert_xml = '';
        if (!empty($alert)) {
            $alert_xml = " alertURL=\"{$alert}\"";
        }

        $this->xmlOutput .= <<<startUpdate

    <update type="{$type}"{$version_xml}{$extensionVersion_xml}{$displayVersion_xml}{$appVersion_xml}{$platformVersion_xml}{$build_xml}{$details_xml}{$license_xml}{$billboard_xml}{$showPrompt_xml}{$showNeverForVersion_xml}{$showSurvey_xml}{$actions_xml}{$open_xml}{$notification_xml}{$alert_xml}>
startUpdate;

        /**
         * @TODO Add buildID attribute to <update> element.
         *
         * Right now it is pending QA on the client side, so we will leave it
         * out for now.
         *
         * buildID="{$build}"
         */
    }

    /**
     * Set a patch line.  This pulls info from a patch object.
     * @param object $patch
     */
    function setPatchLine($patch) {
        $type = htmlentities($patch->type);
        $url = htmlentities($patch->url);
        $hashFunction = htmlentities($patch->hashFunction);
        $hashValue = htmlentities($patch->hashValue);
        $size = htmlentities($patch->size);

        $force = null;
        if (!empty($_GET['force']) && $_GET['force']==1) {
            // Determine if the outgoing URL is bouncer
            // we're looking for download.m.o/?product=foo....
            if ( strpos($url, '/?') !== false ) {
                $force = htmlentities('&force=1');
            }
        }

        $this->xmlPatchLines .= <<<patchLine

        <patch type="{$type}" URL="{$url}{$force}" hashFunction="{$hashFunction}" hashValue="{$hashValue}" size="{$size}"/>
patchLine;
    }

    /**
     * Determines whether or not patchLines have been set.
     * @return bool
     */
    function hasPatchLine() {
        return (empty($this->xmlPatchLines)) ? false : true;
    }

    /**
     * End an update block.
     */
    function endUpdate() {
        $this->xmlOutput .= <<<endUpdate

    </update>
endUpdate;
    }

    /**
     * Add patchLines to output.
     */
    function drawPatchLines() {
        $this->xmlOutput .= $this->xmlPatchLines;
    }

    /**
     * Get XML output.
     * @return $string $this->xmlOutput
     */
    function getOutput() {
        $this->xmlOutput .= $this->xmlFooter;
        return $this->xmlOutput; 
    }

    /**
     * Print XML output with header.
     */
    function printXml() {
        header('Content-type: text/xml;');
        echo $this->getOutput();
    }
}
?>
