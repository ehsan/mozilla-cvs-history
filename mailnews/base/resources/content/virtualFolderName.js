/* -*- Mode: Java; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is Mozilla.org code
 *
 * Contributor(s):
 *   David Bienvenu <bienvenu@nventure.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

var dialog;

function onLoad()
{
  var arguments = window.arguments[0];

  dialog = {};

  dialog.OKButton = document.documentElement.getButton("accept");

  dialog.nameField = document.getElementById("name");
  dialog.nameField.value = arguments.name;
  dialog.nameField.select();
  dialog.nameField.focus();

  // call this when OK is pressed
  dialog.okCallback = arguments.okCallback;

  // pre select the folderPicker, based on what they selected in the folder pane
  dialog.siblingFolderURI = arguments.siblingFolderURI;
  dialog.searchFolderURIs = arguments.searchFolderURIs
  moveToAlertPosition();
  doEnabling();
}

function onOK()
{
  dialog.okCallback(dialog.nameField.value, dialog.siblingFolderURI, dialog.searchFolderURIs);

  return true;
}

function doEnabling()
{
  if (dialog.nameField.value) {
    if (dialog.OKButton.disabled)
      dialog.OKButton.disabled = false;
  } else {
    if (!dialog.OKButton.disabled)
      dialog.OKButton.disabled = true;
  }
}

