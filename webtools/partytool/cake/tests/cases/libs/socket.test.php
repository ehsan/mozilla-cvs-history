<?php
/* SVN FILE: $Id: socket.test.php,v 1.1 2007/05/25 05:54:25 rflint%ryanflint.com Exp $ */
/**
 * Short description for file.
 *
 * Long description for file
 *
 * PHP versions 4 and 5
 *
 * CakePHP(tm) Tests <https://trac.cakephp.org/wiki/Developement/TestSuite>
 * Copyright 2005-2007, Cake Software Foundation, Inc.
 *								1785 E. Sahara Avenue, Suite 490-204
 *								Las Vegas, Nevada 89104
 *
 *  Licensed under The Open Group Test Suite License
 *  Redistributions of files must retain the above copyright notice.
 *
 * @filesource
 * @copyright		Copyright 2005-2007, Cake Software Foundation, Inc.
 * @link				https://trac.cakephp.org/wiki/Developement/TestSuite CakePHP(tm) Tests
 * @package			cake.tests
 * @subpackage		cake.tests.cases.libs
 * @since			CakePHP(tm) v 1.2.0.4206
 * @version			$Revision: 1.1 $
 * @modifiedby		$LastChangedBy: phpnut $
 * @lastmodified	$Date: 2007/05/25 05:54:25 $
 * @license			http://www.opensource.org/licenses/opengroup.php The Open Group Test Suite License
 */
	require_once LIBS.'socket.php';
/**
 * Short description for class.
 *
 * @package		cake.tests
 * @subpackage	cake.tests.cases.libs
 */
class SocketTest extends UnitTestCase {

	function setUp() {
		$this->socket = new CakeSocket();
	}

	function testSocketConnection() {
		$this->assertFalse($this->socket->connected);
		$this->socket->disconnect();
		$this->assertFalse($this->socket->connected);
		$this->socket->connect();
		$this->assertTrue($this->socket->connected);
	}

	function testSocketHost() {
		$this->assertEqual($this->socket->address(), '127.0.0.1');
		$this->assertPattern('/local/', $this->socket->host());
		$this->assertEqual($this->socket->lastError(), null);
		$this->assertTrue(in_array('127.0.0.1', $this->socket->addresses()));
	}

	function testSocketWriting() {
		$request = "GET / HTTP/1.1\r\nConnection: close\r\n\r\n";
		$this->assertTrue($this->socket->write($request));
	}
}
?>