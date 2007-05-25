<?php
/* SVN FILE: $Id: folder.test.php,v 1.1 2007/05/25 05:54:25 rflint%ryanflint.com Exp $ */
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
 * @lastmodified $Date: 2007/05/25 05:54:25 $
 * @license      http://www.opensource.org/licenses/opengroup.php The Open Group Test Suite License
 */
	require_once LIBS.'folder.php';
/**
 * Short description for class.
 *
 * @package    test_suite
 * @subpackage test_suite.cases.libs
 * @since      CakePHP Test Suite v 1.0.0.0
 */
class FolderTest extends UnitTestCase {
	function testBasic() {
		$path = dirname(__FILE__);
		$this->Folder =& new Folder($path);

		$result = $this->Folder->pwd();
		$this->assertEqual($result, $path);

		$result = $this->Folder->isWindowsPath($path);
		$expected = (DS == '\\' ? true : false);
		$this->assertEqual($result, $expected);

		$result = $this->Folder->isAbsolute($path);
		$this->assertTrue($result);

		$result = $this->Folder->isSlashTerm($path);
		$this->assertFalse($result);

		$result = $this->Folder->isSlashTerm($path . DS);
		$this->assertTrue($result);

		$result = $this->Folder->addPathElement($path, 'test');
		$expected = $path . DS . 'test';
		$this->assertEqual($result, $expected);

		$result = $this->Folder->cd(ROOT);
		$expected = ROOT;
		$this->assertEqual($result, $expected);
	}

	function testInPath() {
		$path = dirname(dirname(__FILE__));
		$inside = dirname($path) . DS;

		$this->Folder =& new Folder($path);

		$result = $this->Folder->pwd();
		$this->assertEqual($result, $path);

		$result = $this->Folder->isSlashTerm($inside);
		$this->assertTrue($result);

		//$result = $this->Folder->inPath('tests/');
		//$this->assertTrue($result);

		$result = $this->Folder->inPath(DS . 'non-existing' . DS . $inside);
		$this->assertFalse($result);
	}

	function testOperations() {
		$path = CAKE_CORE_INCLUDE_PATH.DS.'cake'.DS.'console'.DS.'libs'.DS.'templates'.DS.'skel';
		$this->Folder =& new Folder($path);
		
		$result = is_dir($this->Folder->pwd());
		$this->assertTrue($result);
		
		$new = TMP . 'test_folder_new';
		$result = $this->Folder->create($new);
		$this->assertTrue($result);
		
		$copy = TMP . 'test_folder_copy';
		$result = $this->Folder->copy($copy);
		$this->assertTrue($result);
		
		$copy = TMP . 'test_folder_copy';
		$result = $this->Folder->chmod($copy, 0755);
		$this->assertTrue($result);
		
		$result = $this->Folder->cd($copy);
		$this->assertTrue($result);
		
		$mv = TMP . 'test_folder_mv';
		$result = $this->Folder->move($mv);
		$this->assertTrue($result);
		
		$result = $this->Folder->delete($new);
		$this->assertTrue($result);
		
		$result = $this->Folder->delete($mv);
		$this->assertTrue($result);
		
		//pr($this->Folder->messages());
		
		//pr($this->Folder->errors());
	}
}
?>