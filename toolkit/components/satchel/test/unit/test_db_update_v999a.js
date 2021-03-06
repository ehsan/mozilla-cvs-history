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
 * The Original Code is Satchel Test Code.
 *
 * The Initial Developer of the Original Code is
 * Mozilla Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2008
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Justin Dolske <dolske@mozilla.com> (Original Author)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

/*
 * This test uses a formhistory.sqlite with schema version set to 999 (a
 * future version). This exercies the code that allows using a future schema
 * version as long as the expected columns are present.
 *
 * Part A tests this when the columns do match, so the DB is used.
 * Part B tests this when the columns do *not* match, so the DB is reset.
 */

function run_test()
{
  try {
  var testnum = 0;

  // ===== test init =====
  var testfile = do_get_file("toolkit/components/satchel/test/unit/formhistory_v999a.sqlite");
  var profileDir = dirSvc.get("ProfD", Ci.nsIFile);

  // Cleanup from any previous tests or failures.
  var destFile = profileDir.clone();
  destFile.append("formhistory.sqlite");
  if (destFile.exists())
    destFile.remove(false);

  testfile.copyTo(profileDir, "formhistory.sqlite");
  do_check_eq(999, getDBVersion(testfile));

  var fh = Cc["@mozilla.org/satchel/form-history;1"].
           getService(Ci.nsIFormHistory2);


  // ===== 1 =====
  testnum++;
  // Check for expected contents.
  do_check_true(fh.hasEntries);
  do_check_true(fh.entryExists("name-A", "value-A"));
  do_check_true(fh.entryExists("name-B", "value-B"));
  do_check_true(fh.entryExists("name-C", "value-C1"));
  do_check_true(fh.entryExists("name-C", "value-C2"));
  do_check_true(fh.entryExists("name-E", "value-E"));
  // check for downgraded schema.
  // 1.9.0 can't directly check fh.DBConnection.schemaVersion, so look at the file
  do_check_eq(CURRENT_SCHEMA, getDBVersion(destFile));

  // ===== 2 =====
  testnum++;
  // Exercise adding and removing a name/value pair
  do_check_false(fh.entryExists("name-D", "value-D"));
  fh.addEntry("name-D", "value-D");
  do_check_true(fh.entryExists("name-D", "value-D"));
  fh.removeEntry("name-D", "value-D");
  do_check_false(fh.entryExists("name-D", "value-D"));

  } catch (e) {
    throw "FAILED in test #" + testnum + " -- " + e;
  }
}
