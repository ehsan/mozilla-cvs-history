<?php
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
 * The Original Code is Mozilla Reporter (r.m.o).
 *
 * The Initial Developer of the Original Code is
 *      Robert Accettura <robert@accettura.com>.
 *
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

require_once('../../config.inc.php');
require_once($config['base_path'].'/includes/iolib.inc.php');
require_once($config['base_path'].'/includes/db.inc.php');
require_once($config['base_path'].'/includes/contrib/smarty/libs/Smarty.class.php');
require_once($config['base_path'].'/includes/security.inc.php');
require_once($config['base_path'].'/includes/query.inc.php');

// start the session
session_name('reportSessID');
session_start();
header("Cache-control: private"); //IE 6 Fix
printheaders();

$method = 'html';

$title = "Searching Results";

$content = initializeTemplate();
$content->assign('method', $method);

// Open DB
$db = NewDBConnection($config['db_dsn']);
$db->SetFetchMode(ADODB_FETCH_ASSOC);

$query = new query;
$query_input = $query->getQueryInputs();

$result = $query->doQuery($query_input['selected'],
                          $query_input['where'],
                          $query_input['orderby'],
                          $query_input['ascdesc'],
                          $query_input['show'],
                          $query_input['page'],
                          $query_input['count']
          );

$continuity_params = $query->continuityParams($query_input);

$output = $query->outputHTML($result, $query_input, $continuity_params, $columnHeaders);

// disconnect database
$db->Close();

if (sizeof($output['data']) == 0){
    $content->assign('error', 'No Results found');
    displayPage($content, 'query', 'query.tpl');
    exit;
}

// Start Next/Prev Navigation
/*******
 * We cap the navigation at 2000 items because php sometimes acts wierd
 * when sessions get to big.  In most cases, this won't effect anyone.
 *******/
if($result['totalResults'] < 2000){
    $_SESSION['reportList'] = $result['reportList'];
} else {
    unset($_SESSION['reportList']);
    $content->assign('notice', 'This query returned too many reports for next/previous navigation to work');
}

$content->assign('continuity_params',  $continuity_params);
$content->assign('column',             $query->columnHeaders($query_input, $continuity_params));
$content->assign('row',                $output['data']);
$content->assign('continuityParams',   $continuity_params[1]);
$content->assign('count',              $result['totalResults']);
$content->assign('show',               $query_input['show']);
$content->assign('page',               $query_input['page']);

displayPage($content, 'query', 'query.tpl');
?>