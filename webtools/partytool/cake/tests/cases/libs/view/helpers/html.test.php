<?php
/* SVN FILE: $Id: html.test.php,v 1.1 2007/05/25 05:54:26 rflint%ryanflint.com Exp $ */
/**
 * Short description for file.
 *
 * Long description for file
 *
 * PHP versions 4 and 5
 *
 * CakePHP Test Suite <https://trac.cakephp.org/wiki/Developement/TestSuite>
 * Copyright (c) 2006, Larry E. Masters Shorewood, IL. 60431
 * Author(s): Larry E. Masters aka PhpNut <phpnut@gmail.com>
 *
 *  Licensed under The Open Group Test Suite License
 *  Redistributions of files must retain the above copyright notice.
 *
 * @filesource
 * @author       Larry E. Masters aka PhpNut <phpnut@gmail.com>
 * @copyright    Copyright (c) 2006, Larry E. Masters Shorewood, IL. 60431
 * @link         http://www.phpnut.com/projects/
 * @package      test_suite
 * @subpackage   test_suite.cases.app
 * @since        CakePHP Test Suite v 1.0.0.0
 * @version      $Revision: 1.1 $
 * @modifiedby   $LastChangedBy: phpnut $
 * @lastmodified $Date: 2007/05/25 05:54:26 $
 * @license      http://www.opensource.org/licenses/opengroup.php The Open Group Test Suite License
 */
	require_once LIBS.'../app_helper.php';
	require_once LIBS.'class_registry.php';
	require_once LIBS.DS.'view'.DS.'view.php';
	require_once LIBS.DS.'view'.DS.'helper.php';
	require_once LIBS.DS.'view'.DS.'helpers'.DS.'html.php';
	require_once LIBS.DS.'controller'.DS.'controller.php';

class TheHtmlTestController extends Controller {
	var $name = 'TheTest';
	var $uses = null;
}

class HtmlHelperTest extends UnitTestCase {
	var $html = null;
	
	function setUp() {
		$this->Html =& new HtmlHelper();
		$view =& new View(new TheHtmlTestController());
		ClassRegistry::addObject('view', $view);
	}
	
	function testLinkEscape() {
		$result = $this->Html->link('Next >', '#');
		$expected = '/^<a href="#"\s+>Next &gt;<\/a>$/';
		
		$this->assertPattern($expected, $result);
		
		$result = $this->Html->link('Next >', '#', array('escape' => false));
		$expected = '/^<a href="#"\s+>Next ><\/a>$/';
		
		$this->assertPattern($expected, $result);
	}
	
	function testImageLink() {
		$result = $this->Html->link($this->Html->image('test.gif'), '#', array(), false, false, false);
		$expected = '/^<a href="#"\s+><img\s+src="img\/test.gif"\s+alt=""\s+\/><\/a>$/';
		
		$this->assertPattern($expected, $result);
	}
	
	function testRadio() {
		$result = $this->Html->radio('Tests/process', Array('0'=> 'zero', '1'=>'one'));
		$this->assertNoPattern('/checked="checked"/', $result);
	}

	function tearDown() {
		unset($this->Html);
	}
}
?>