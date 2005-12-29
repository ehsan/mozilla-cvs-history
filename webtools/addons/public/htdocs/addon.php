<?php
/**
 * Addon summary page.  Displays a top-down view of all Addon properties.
 * 
 * @package amo
 * @subpackage docs
 */

// Get the int value of our addon ID.
$clean['ID'] = intval($_GET['id']);
$sql['ID'] =& $clean['ID'];

// Set the cachId so we have a unique cache for each AddOn ID.
$cacheId = $clean['ID'];

startProcessing('addon.tpl',$cacheId,$compileId);
require_once('includes.php');


// Create our AddOn object using the ID.
$addon = new AddOn($sql['ID']);

// Assign template variables.
$tpl->assign(
    array(  'addon'     => $addon,
            'title'     => $addon->Name,
            'content'   => 'addon.tpl',
            'sidebar'   => 'inc/addon-sidebar.tpl')
);
?>
