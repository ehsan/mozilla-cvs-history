<?php
/* SVN FILE: $Id: session.php,v 1.2 2007/11/19 08:49:53 rflint%ryanflint.com Exp $ */
/**
 * Session class for Cake.
 *
 * Cake abstracts the handling of sessions.
 * There are several convenient methods to access session information.
 * This class is the implementation of those methods.
 * They are mostly used by the Session Component.
 *
 * PHP versions 4 and 5
 *
 * CakePHP(tm) :  Rapid Development Framework <http://www.cakephp.org/>
 * Copyright 2005-2007, Cake Software Foundation, Inc.
 *								1785 E. Sahara Avenue, Suite 490-204
 *								Las Vegas, Nevada 89104
 *
 * Licensed under The MIT License
 * Redistributions of files must retain the above copyright notice.
 *
 * @filesource
 * @copyright		Copyright 2005-2007, Cake Software Foundation, Inc.
 * @link				http://www.cakefoundation.org/projects/info/cakephp CakePHP(tm) Project
 * @package			cake
 * @subpackage		cake.cake.libs
 * @since			CakePHP(tm) v .0.10.0.1222
 * @version			$Revision: 1.2 $
 * @modifiedby		$LastChangedBy: phpnut $
 * @lastmodified	$Date: 2007/11/19 08:49:53 $
 * @license			http://www.opensource.org/licenses/mit-license.php The MIT License
 */
/**
 * Database name for cake sessions.
 *
 */
uses('set');
/**
 * Session class for Cake.
 *
 * Cake abstracts the handling of sessions. There are several convenient methods to access session information.
 * This class is the implementation of those methods. They are mostly used by the Session Component.
 *
 * @package		cake
 * @subpackage	cake.cake.libs
 */
class CakeSession extends Object {
/**
 * True if the Session is still valid
 *
 * @var boolean
 * @access public
 */
	var $valid = false;
/**
 * Error messages for this session
 *
 * @var array
 * @access public
 */
	var $error = false;
/**
 * User agent string
 *
 * @var string
 * @access protected
 */
	var $_userAgent = '';
/**
 * Path to where the session is active.
 *
 * @var string
 * @access public
 */
	var $path = false;
/**
 * Error number of last occurred error
 *
 * @var integer
 * @access public
 */
	var $lastError = null;
/**
 * 'Security.level' setting, "high", "medium", or "low".
 *
 * @var string
 * @access public
 */
	var $security = null;
/**
 * Start time for this session.
 *
 * @var integer
 * @access public
 */
	var $time = false;
/**
 * Time when this session becomes invalid.
 *
 * @var integer
 * @access public
 */
	var $sessionTime = false;
/**
 * Keeps track of keys to watch for writes on
 *
 * @var array
 * @access public
 */
	var $watchKeys = array();
/**
 * Constructor.
 *
 * @param string $base The base path for the Session
 * @param boolean $start Should session be started right now
 * @access public
 */
	function __construct($base = null, $start = true) {
		if (Configure::read('Session.save') === 'database' && !class_exists('ConnectionManager')) {
			uses('model' . DS . 'connection_manager');
		}

		if (Configure::read('Session.checkAgent') === true) {
			if (env('HTTP_USER_AGENT') != null) {
				$this->_userAgent = md5(env('HTTP_USER_AGENT') . Configure::read('Security.salt'));
			}
		}
		$this->time = time();

		if ($start === true) {
			$this->host = env('HTTP_HOST');

			if (empty($base) || strpos($base, '?' || strpos($base, 'index.php'))) {
				$this->path = '/';
			} else {
				$this->path = $base;
			}

			if (strpos($this->host, ':') !== false) {
				$this->host = substr($this->host, 0, strpos($this->host, ':'));
			}

			if (!class_exists('Security')) {
				uses('security');
			}

			$this->sessionTime = $this->time + (Security::inactiveMins() * Configure::read('Session.timeout'));
			$this->security = Configure::read('Security.level');

			if (function_exists('session_write_close')) {
				session_write_close();
			}

			$this->__initSession();
			$this->__startSession();
			$this->__checkValid();
		}
		parent::__construct();
	}
/**
 * Returns true if given variable is set in session.
 *
 * @param string $name Variable name to check for
 * @return boolean True if variable is there
 * @access public
 */
	function check($name) {
		$var = $this->__validateKeys($name);
		if (empty($var)) {
		  return false;
		}
		$result = Set::extract($_SESSION, $var);
		return isset($result);
	}

/**
 * Temp method until we are able to remove the last eval().
 * Builds an expression to fetch a session variable with specified name.
 *
 * @param string $name Name of variable (in dot notation)
 * @access private
 */
	function __sessionVarNames($name) {
		if (is_string($name) && preg_match("/^[ 0-9a-zA-Z._-]*$/", $name)) {
			if (strpos($name, ".")) {
				$names = explode(".", $name);
			} else {
				$names = array($name);
			}
			$expression = "\$_SESSION";
			foreach ($names as $item) {
				$expression .= is_numeric($item) ? "[$item]" : "['$item']";
			}
			return $expression;
		}
		$this->__setError(3, "$name is not a string");
		return false;
	}
/**
 * Removes a variable from session.
 *
 * @param string $name Session variable to remove
 * @return boolean Success
 * @access public
 */
	function del($name) {
		if ($this->check($name)) {
			if ($var = $this->__validateKeys($name)) {
				if (in_array($var, $this->watchKeys)) {
					trigger_error('Deleting session key {' . $var . '}', E_USER_NOTICE);
				}
				$this->__overwrite($_SESSION, Set::remove($_SESSION, $var));
				return ($this->check($var) == false);
			}
		}
		$this->__setError(2, "$name doesn't exist");
		return false;
	}
/**
 * Used to write new data to _SESSION, since PHP doesn't like us setting the _SESSION var itself
 *
 * @param array $old Set of old variables => values
 * @param array $new New set of variable => value
 * @access private
 */
	function __overwrite(&$old, $new) {
		foreach ($old as $key => $var) {
			if (!isset($new[$key])) {
				unset($old[$key]);
			}
		}
		foreach ($new as $key => $var) {
			$old[$key] = $var;
		}
	}
/**
 * Return error description for given error number.
 *
 * @param integer $errorNumber Error to set
 * @return string Error as string
 * @access private
 */
	function __error($errorNumber) {
		if (!is_array($this->error) || !array_key_exists($errorNumber, $this->error)) {
			return false;
		} else {
			return $this->error[$errorNumber];
		}
	}
/**
 * Returns last occurred error as a string, if any.
 *
 * @return mixed Error description as a string, or false.
 * @access public
 */
	function error() {
		if ($this->lastError) {
			return $this->__error($this->lastError);
		} else {
			return false;
		}
	}
/**
 * Returns true if session is valid.
 *
 * @return boolean Success
 * @access public
 */
	function valid() {
		if ($this->read('Config')) {
			if (Configure::read('Session.checkAgent') === false || $this->_userAgent == $this->read("Config.userAgent") && $this->time <= $this->read("Config.time")) {
				if ($this->error === false) {
					$this->valid = true;
				}
			} else {
				$this->valid = false;
				$this->__setError(1, "Session Highjacking Attempted !!!");
			}
		}
		return $this->valid;
	}
/**
 * Returns given session variable, or all of them, if no parameters given.
 *
 * @param mixed $name The name of the session variable (or a path as sent to Set.extract)
 * @return mixed The value of the session variable
 * @access public
 */
	function read($name = null) {
		if (is_null($name)) {
			return $this->__returnSessionVars();
		}
		if (empty($name)) {
			return false;
		}
		$result = Set::extract($_SESSION, $name);

		if (!is_null($result)) {
			return $result;
		}
		$this->__setError(2, "$name doesn't exist");
		return null;
	}
/**
 * Returns all session variables.
 *
 * @return mixed Full $_SESSION array, or false on error.
 * @access private
 */
	function __returnSessionVars() {
		if (!empty($_SESSION)) {
			return $_SESSION;
		}
		$this->__setError(2, "No Session vars set");
		return false;
	}
/**
 * Tells Session to write a notification when a certain session path or subpath is written to
 *
 * @param mixed $var The variable path to watch
 * @access public
 */
	function watch($var) {
		$var = $this->__validateKeys($var);
		if (empty($var)) {
			return false;
		}
		$this->watchKeys[] = $var;
	}
/**
 * Tells Session to stop watching a given key path
 *
 * @param mixed $var The variable path to watch
 * @access public
 */
	function ignore($var) {
		$var = $this->__validateKeys($var);
		if (!in_array($var, $this->watchKeys)) {
			return;
		}
		foreach ($this->watchKeys as $i => $key) {
			if ($key == $var) {
				unset($this->watchKeys[$i]);
				$this->watchKeys = array_values($this->watchKeys);
				return;
			}
		}
	}
/**
 * Writes value to given session variable name.
 *
 * @param mixed $name Name of variable
 * @param string $value Value to write
 * @return boolean True if the write was successful, false if the write failed
 * @access public
 */
	function write($name, $value) {
		$var = $this->__validateKeys($name);

		if (empty($var)) {
			return false;
		}
		if (in_array($var, $this->watchKeys)) {
			trigger_error('Writing session key {' . $var . '}: ' . Debugger::exportVar($value), E_USER_NOTICE);
		}
		$this->__overwrite($_SESSION, Set::insert($_SESSION, $var, $value));
		return (Set::extract($_SESSION, $var) === $value);
	}
/**
 * Helper method to destroy invalid sessions.
 *
 * @access public
 */
	function destroy() {
		$sessionpath = session_save_path();
		if (empty($sessionpath)) {
			$sessionpath = "/tmp";
		}

		if (isset($_COOKIE[session_name()])) {
			setcookie(Configure::read('Session.cookie'), '', time() - 42000, $this->path);
		}

		$_SESSION = array();
		$file = $sessionpath . DS . "sess_" . session_id();
		@session_destroy();
		@unlink ($file);
		$this->__construct($this->path);
		$this->renew();
	}
/**
 * Helper method to initialize a session, based on Cake core settings.
 *
 * @access private
 */
	function __initSession() {
		switch($this->security) {
			case 'high':
				$this->cookieLifeTime = 0;
				if (function_exists('ini_set')) {
					ini_set('session.referer_check', $this->host);
				}
			break;
			case 'medium':
				$this->cookieLifeTime = 7 * 86400;
				if (function_exists('ini_set')) {
					ini_set('session.referer_check', $this->host);
				}
			break;
			case 'low':
			default:
				$this->cookieLifeTime = 788940000;
			break;
		}

		switch(Configure::read('Session.save')) {
			case 'cake':
				if (!isset($_SESSION)) {
					if (function_exists('ini_set')) {
						ini_set('session.use_trans_sid', 0);
						ini_set('url_rewriter.tags', '');
						ini_set('session.serialize_handler', 'php');
						ini_set('session.use_cookies', 1);
						ini_set('session.name', Configure::read('Session.cookie'));
						ini_set('session.cookie_lifetime', $this->cookieLifeTime);
						ini_set('session.cookie_path', $this->path);
						ini_set('session.auto_start', 0);
						ini_set('session.save_path', TMP . 'sessions');
					}
				}
			break;
			case 'database':
				if (!isset($_SESSION)) {
					if (Configure::read('Session.table') === null) {
						trigger_error(__("You must set the all Configure::write('Session.*') in core.php to use database storage"), E_USER_WARNING);
						exit();
					} elseif (Configure::read('Session.database') === null) {
						Configure::write('Session.database', 'default');
					}
					if (function_exists('ini_set')) {
						ini_set('session.use_trans_sid', 0);
						ini_set('url_rewriter.tags', '');
						ini_set('session.save_handler', 'user');
						ini_set('session.serialize_handler', 'php');
						ini_set('session.use_cookies', 1);
						ini_set('session.name', Configure::read('Session.cookie'));
						ini_set('session.cookie_lifetime', $this->cookieLifeTime);
						ini_set('session.cookie_path', $this->path);
						ini_set('session.auto_start', 0);
					}
				}
				session_set_save_handler(array('CakeSession','__open'),
													array('CakeSession', '__close'),
													array('CakeSession', '__read'),
													array('CakeSession', '__write'),
													array('CakeSession', '__destroy'),
													array('CakeSession', '__gc'));
			break;
			case 'php':
				if (!isset($_SESSION)) {
					if (function_exists('ini_set')) {
						ini_set('session.use_trans_sid', 0);
						ini_set('session.name', Configure::read('Session.cookie'));
						ini_set('session.cookie_lifetime', $this->cookieLifeTime);
						ini_set('session.cookie_path', $this->path);
					}
				}
			break;
			default:
				if (!isset($_SESSION)) {
					$config = CONFIGS . Configure::read('Session.cookie') . '.php';

					if (is_file($config)) {
						require_once ($config);
					}
				}
			break;
		}
	}
/**
 * Helper method to start a session
 *
 * @access private
 */
	function __startSession() {
		if (headers_sent()) {
			if (!isset($_SESSION)) {
				$_SESSION = array();
			}
		} else {
			session_cache_limiter ("must-revalidate");
			session_start();
			if (Configure::read('Security.level') === 'high') {
				$this->renew();
			}
			header ('P3P: CP="NOI ADM DEV PSAi COM NAV OUR OTRo STP IND DEM"');
		}
	}
/**
 * Helper method to create a new session.
 *
 * @access private
 */
	function __checkValid() {
		if ($this->read('Config')) {
			if (Configure::read('Session.checkAgent') === false || $this->_userAgent == $this->read("Config.userAgent") && $this->time <= $this->read("Config.time")) {
				$this->write("Config.time", $this->sessionTime);
				$this->valid = true;
			} else {
				$this->destroy();
				$this->valid = false;
				$this->__setError(1, "Session Highjacking Attempted !!!");
			}
		} else {
			srand ((double)microtime() * 1000000);
			$this->write("Config.userAgent", $this->_userAgent);
			$this->write("Config.time", $this->sessionTime);
			$this->write('Config.rand', rand());
			$this->valid = true;
			$this->__setError(1, "Session is valid");
		}
	}
/**
 * Helper method to restart a session.
 *
 * @access private
 */
	function __regenerateId() {
		$oldSessionId = session_id();
		$sessionpath = session_save_path();
		if (empty($sessionpath)) {
			$sessionpath = "/tmp";
		}

		if (isset($_COOKIE[session_name()])) {
			setcookie(Configure::read('Session.cookie'), '', time() - 42000, $this->path);
		}
		session_regenerate_id();
		$newSessid = session_id();

		if (function_exists('session_write_close')) {
			session_write_close();
		}
		$this->__initSession();
		session_id($oldSessionId);
		session_start();
		session_destroy();
		$file = $sessionpath . DS . "sess_$oldSessionId";
		@unlink($file);
		$this->__initSession();
		session_id ($newSessid);
		session_start();
	}
/**
 * Restarts this session.
 *
 * @access public
 */
	function renew() {
		$this->__regenerateId();
	}
/**
 * Validate that the $name is in correct dot notation
 * example: $name = 'ControllerName.key';
 *
 * @param string $name Session key names as string.
 * @return mixed false is $name is not correct format, or $name if it is correct
 * @access private
 */
	function __validateKeys($name) {
		if (is_string($name) && preg_match("/^[ 0-9a-zA-Z._-]*$/", $name)) {
			return $name;
		}
		$this->__setError(3, "$name is not a string");
		return false;
	}
/**
 * Helper method to set an internal error message.
 *
 * @param integer $errorNumber Number of the error
 * @param string $errorMessage Description of the error
 * @access private
 */
	function __setError($errorNumber, $errorMessage) {
		if ($this->error === false) {
			$this->error = array();
		}
		$this->error[$errorNumber] = $errorMessage;
		$this->lastError = $errorNumber;
	}
/**
 * Method called on open of a database session.
 *
 * @return boolean Success
 * @access private
 */
	function __open() {
		return true;
	}
/**
 * Method called on close of a database session.
 *
 * @return boolean Success
 * @access private
 */
	function __close() {
		$probability = mt_rand(1, 150);
		if ($probability <= 3) {
			CakeSession::__gc();
		}
		return true;
	}
/**
 * Method used to read from a database session.
 *
 * @param mixed $key The key of the value to read
 * @return mixed The value of the key or false if it does not exist
 * @access private
 */
	function __read($key) {
		$db =& ConnectionManager::getDataSource(Configure::read('Session.database'));
		$table = $db->fullTableName(Configure::read('Session.table'), false);
		$row = $db->query("SELECT " . $db->name($table.'.data') . " FROM " . $db->name($table) . " WHERE " . $db->name($table.'.id') . " = " . $db->value($key), false);

		if ($row && !isset($row[0][$table]) && isset($row[0][0])) {
			$table = 0;
		}

		if ($row && $row[0][$table]['data']) {
			return $row[0][$table]['data'];
		} else {
			return false;
		}
	}
/**
 * Helper function called on write for database sessions.
 *
 * @param mixed $key The name of the var
 * @param mixed $value The value of the var
 * @return boolean Success
 * @access private
 */
	function __write($key, $value) {
		$db =& ConnectionManager::getDataSource(Configure::read('Session.database'));
		$table = $db->fullTableName(Configure::read('Session.table'));

		switch(Configure::read('Security.level')) {
			case 'high':
				$factor = 10;
			break;
			case 'medium':
				$factor = 100;
			break;
			case 'low':
				$factor = 300;
			break;
			default:
				$factor = 10;
			break;
		}
		$expires = time() +  Configure::read('Session.timeout') * $factor;
		$row = $db->query("SELECT COUNT(id) AS count FROM " . $db->name($table) . " WHERE "
								 . $db->name('id') . " = "
								 . $db->value($key), false);

		if ($row[0][0]['count'] > 0) {
			$db->execute("UPDATE " . $db->name($table) . " SET " . $db->name('data') . " = "
								. $db->value($value) . ", " . $db->name('expires') . " = "
								. $db->value($expires) . " WHERE " . $db->name('id') . " = "
								. $db->value($key));
		} else {
			$db->execute("INSERT INTO " . $db->name($table) . " (" . $db->name('data') . ","
							  	. $db->name('expires') . "," . $db->name('id')
							  	. ") VALUES (" . $db->value($value) . ", " . $db->value($expires) . ", "
							  	. $db->value($key) . ")");
		}
		return true;
	}
/**
 * Method called on the destruction of a database session.
 *
 * @param integer $key Key that uniquely identifies session in database
 * @return boolean Success
 * @access private
 */
	function __destroy($key) {
		$db =& ConnectionManager::getDataSource(Configure::read('Session.database'));
		$table = $db->fullTableName(Configure::read('Session.table'));
		$db->execute("DELETE FROM " . $db->name($table) . " WHERE " . $db->name($table.'.id') . " = " . $db->value($key, 'integer'));
		return true;
	}
/**
 * Helper function called on gc for database sessions.
 *
 * @param integer $expires Timestamp (defaults to current time)
 * @return boolean Success
 * @access private
 */
	function __gc($expires = null) {
		$db =& ConnectionManager::getDataSource(Configure::read('Session.database'));
		$table = $db->fullTableName(Configure::read('Session.table'));
		$db->execute("DELETE FROM " . $db->name($table) . " WHERE " . $db->name($table.'.expires') . " < ". $db->value(time()));
		return true;
	 }
}
?>