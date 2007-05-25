<?php
/* SVN FILE: $Id: folder.php,v 1.1 2007/05/25 05:54:17 rflint%ryanflint.com Exp $ */
/**
 * Convenience class for handling directories.
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
 * @since			CakePHP(tm) v 0.2.9
 * @version			$Revision: 1.1 $
 * @modifiedby		$LastChangedBy: phpnut $
 * @lastmodified	$Date: 2007/05/25 05:54:17 $
 * @license			http://www.opensource.org/licenses/mit-license.php The MIT License
 */
/**
 * Included libraries.
 *
 */
	if (!class_exists('Object')) {
		uses ('object');
	}
/**
 * Folder structure browser, lists folders and files.
 *
 * Long description for class
 *
 * @package		cake
 * @subpackage	cake.cake.libs
 */
class Folder extends Object{
/**
 * Path to Folder.
 *
 * @var string
 * @access protected
 */
	var $path = null;
/**
 * Sortedness.
 *
 * @var boolean
 * @access protected
 */
	var $sort = false;
/**
 * mode to be used on create.
 *
 * @var boolean
 * @access protected
 */
	var $mode = '755';
/**
 * holds messages from last method.
 *
 * @var array
 * @access private
 */
	var $__messages = array();
/**
 * holds errors from last method.
 *
 * @var array
 * @access private
 */
	var $__errors = false;
/**
 * Constructor.
 *
 * @param string $path Path to folder
 * @param boolean $create Create folder if not found
 * @param mixed $mode Mode (CHMOD) to apply to created folder, false to ignore
 */
	function __construct($path = false, $create = false, $mode = false) {
		parent::__construct();
		if (empty($path)) {
			$path = getcwd();
		}

		if($mode) {
			$this->mode = strval($mode);
		}

		if (!file_exists($path) && $create == true) {
			$this->create($path, $this->mode);
		}
		$this->cd($path);
	}
/**
 * Return current path.
 *
 * @return string Current path
 * @access public
 */
	function pwd() {
		return $this->path;
	}
/**
 * Change directory to $desired_path.
 *
 * @param string $desired_path Path to the directory to change to
 * @return string The new path. Returns false on failure
 * @access public
 */
	function cd($path) {
		$path = realpath($path);
		if(!$this->isAbsolute($path)) {
			$path = $this->addPathElement($this->path, $path);
		}
		if(is_dir($path) && file_exists($path)) {
			return $this->path = $path;
		}
		return false;
	 }
/**
 * Returns an array of the contents of the current directory, or false on failure.
 * The returned array holds two arrays: one of dirs and one of files.
 *
 * @param boolean $sort
 * @param mixed $exceptions either an array or boolean true will no grab dot files
 * @return mixed Contents of current directory as an array, false on failure
 * @access public
 */
	function read($sort = true, $exceptions = false) {
		$dirs = $files = array();
		$dir = opendir($this->path);
		if ($dir) {
			while(false !== ($n = readdir($dir))) {
				$item = false;
				if(is_array($exceptions)) {
					if (!in_array($n, $exceptions)) {
						$item = $n;
					}
				} else if ((!preg_match('#^\.+$#', $n) && $exceptions == false) || ($exceptions == true && !preg_match('#^\.(.*)$#', $n))) {
					$item = $n;
				}

				if ($item) {
					if(is_dir($this->addPathElement($this->path, $item))) {
						$dirs[] = $item;
					} else {
						$files[] = $item;
					}
				}
			}

			if ($sort || $this->sort) {
				sort ($dirs);
				sort ($files);
			}
			closedir ($dir);
		}
		return array($dirs, $files);
	}
/**
 * Returns an array of all matching files in current directory.
 *
 * @param string $pattern Preg_match pattern (Defaults to: .*)
 * @return array Files that match given pattern
 * @access public
 */
	function find($regexp_pattern = '.*') {
		$data = $this->ls();

		if (!is_array($data)) {
			return array();
		}

		list($dirs, $files) = $data;
		$found =  array();

		foreach($files as $file) {
			if (preg_match("/^{$regexp_pattern}$/i", $file)) {
				$found[] = $file;
			}
		}
		return $found;
	}
/**
 * Returns an array of all matching files in and below current directory.
 *
 * @param string $pattern Preg_match pattern (Defaults to: .*)
 * @return array Files matching $pattern
 * @access public
 */
	function findRecursive($pattern = '.*') {
		$startsOn = $this->path;
		$out = $this->_findRecursive($pattern);
		$this->cd($startsOn);
		return $out;
	}
/**
 * Private helper function for findRecursive.
 *
 * @param string $pattern Pattern to match against
 * @return array Files matching pattern
 * @access private
 */
	function _findRecursive($pattern) {
		list($dirs, $files) = $this->ls();

		$found = array();
		foreach($files as $file) {
			if (preg_match("/^{$pattern}$/i", $file)) {
				$found[] = $this->addPathElement($this->path, $file);
			}
		}
		$start = $this->path;
		foreach($dirs as $dir) {
			$this->cd($this->addPathElement($start, $dir));
			$found = array_merge($found, $this->findRecursive($pattern));
		}
		return $found;
	}
/**
 * Returns true if given $path is a Windows path.
 *
 * @param string $path Path to check
 * @return boolean true if windows path, false otherwise
 * @access public
 * @static
 */
	function isWindowsPath($path) {
		if(preg_match('#^[A-Z]:\\\#i', $path)) {
			return true;
		}
		return false;
	}
/**
 * Returns true if given $path is an absolute path.
 *
 * @param string $path Path to check
 * @return boolean
 * @access public
 * @static
 */
	function isAbsolute($path) {
		$match = preg_match('#^\/#', $path) || preg_match('#^[A-Z]:\\\#i', $path);
		return $match;
	}
/**
 * Returns true if given $path ends in a slash (i.e. is slash-terminated).
 *
 * @param string $path Path to check
 * @return boolean true if path ends with slash, false otherwise
 * @access public
 * @static
 */
	function isSlashTerm($path) {
		if(preg_match('#[\\\/]$#', $path)) {
			return true;
		}
		return false;
	}
/**
 * Returns a correct set of slashes for given $path. (\\ for Windows paths and / for other paths.)
 *
 * @param string $path Path to check
 * @return string Set of slashes ("\\" or "/")
 * @access public
 * @static
 */
	function normalizePath($path) {
		if($this->isWindowsPath($path)) {
			return '\\';
		}
		return '/';
	}
/**
 * Returns a correct set of slashes for given $path. (\\ for Windows paths and / for other paths.)
 *
 * @param string $path Path to check
 * @return string Set of slashes ("\\" or "/")
 * @access public
 * @static
 */
	function correctSlashFor($path) {
		if($this->isWindowsPath($path)) {
			return '\\';
		}
		return '/';
	}
/**
 * Returns $path with added terminating slash (corrected for Windows or other OS).
 *
 * @param string $path Path to check
 * @return string Path with ending slash
 * @access public
 * @static
 */
	function slashTerm($path) {
		if($this->isSlashTerm($path)) {
			return $path;
		}
		return $path . $this->correctSlashFor($path);
	 }
/**
 * Returns $path with $element added, with correct slash in-between.
 *
 * @param string $path Path
 * @param string $element Element to and at end of path
 * @return string Combined path
 * @access public
 * @static
 */
	function addPathElement($path, $element) {
		return $this->slashTerm($path) . $element;
	}
/**
 * Returns true if the File is in a given CakePath.
 *
 * @return boolean
 * @access public
 */
	function inCakePath($path = '') {
		$dir = substr($this->slashTerm(ROOT), 0, -1);
		$newdir = $this->slashTerm($dir . $path);
		return $this->inPath($newdir);
	 }
/**
 * Returns true if the File is in given path.
 *
 * @return boolean
 * @access public
 */
	function inPath($path = '') {
		$dir = substr($this->slashTerm($path), 0, -1);
		$return = preg_match('/^' . preg_quote($this->slashTerm($dir), '/') . '(.*)/', $this->slashTerm($this->pwd()));
		if ($return == 1) {
			return true;
		} else {
			return false;
		}
	}
/**
 * Change the mode on a directory structure recursively.
 *
 * @param string $pathname The directory structure to create
 * @return bool Returns TRUE on success, FALSE on failure
 * @access public
 */
	function chmod($path, $mode = false, $exceptions = false) {

		if(!$mode) {
			$mode = $this->mode;
		}
		
		if (is_dir($path)) {
			return chmod($path, intval($mode, 8));
		}

		$dir = opendir($path);
		if($dir) {
			while(false !== ($n = readdir($dir))) {
				$item = false;
				if(is_array($exceptions)) {
					if (!in_array($n, $exceptions)) {
						$item = $n;
					}
				} else if ((!preg_match('#^\.+$#', $n) && $exceptions == false) || ($exceptions == true && !preg_match('#^\.(.*)$#', $n))) {
					$item = $n;
				}

				if ($item) {
					$fullpath = $this->addPathElement($path, $item);
					if (!is_dir($fullpath)) {
						if (chmod($fullpath, intval($mode, 8))) {
							$this->__messages[] = sprintf(__('%s changed to %s', true), $fullpath, $mode);
							return true;
						} else {
							$this->__errors[] = sprintf(__('%s NOT changed to %s', true), $fullpath, $mode);
							return false;
						}
					} else {
						if ($this->chmod($fullpath, $mode)) {
							$this->__messages[] = sprintf(__('%s changed to %s', true), $fullpath, $mode);
							return true;
						} else {
							$this->__errors[] = sprintf(__('%s NOT changed to %s', true), $fullpath, $mode);
							return false;
						}
					}
				}
			}
			closedir($dir);
		}

		if (chmod($path, intval($mode, 8))) {
			$this->__messages[] = sprintf(__('%s changed to %s', true), $path, $mode);
			return true;
		} else {
			return false;
		}
	}
/**
 * Create a directory structure recursively.
 *
 * @param string $pathname The directory structure to create
 * @return bool Returns TRUE on success, FALSE on failure
 * @access public
 */
	function create($pathname, $mode = false) {
		if (is_dir($pathname) || empty($pathname)) {
			return true;
		}

		if(!$mode) {
			$mode = $this->mode;
		}

		if (is_file($pathname)) {
			$this->__errors[] = sprintf(__('%s is a file', true), $pathname);
			return true;
		}
		$nextPathname = substr($pathname, 0, strrpos($pathname, DS));

		if ($this->create($nextPathname, $mode)) {
			if (!file_exists($pathname)) {
				if (mkdir($pathname, intval($mode, 8))) {
					$this->__messages[] = sprintf(__('%s created', true), $pathname);
					return true;
				} else {
					$this->__errors[] = sprintf(__('%s NOT created', true), $pathname);
					return false;
				}
			}
		}
		return true;
	}
/**
 * Returns the size in bytes of this Folder.
 *
 * @param string $directory Path to directory
 * @access public
 */
	function dirsize() {
		$size = 0;
		$directory = $this->slashTerm($this->path);
		$stack = array($directory);
		$count = count($stack);
		for($i = 0, $j = $count; $i < $j; ++$i) {
			if (is_file($stack[$i])) {
				$size += filesize($stack[$i]);
			} elseif (is_dir($stack[$i])) {
				$dir = dir($stack[$i]);
				if($dir) {
					while(false !== ($entry = $dir->read())) {
						if ($entry == '.' || $entry == '..') {
							continue;
						}
						$add = $stack[$i] . $entry;

						if (is_dir($stack[$i] . $entry)) {
							$add = $this->slashTerm($add);
						}
						$stack[ ]= $add;
					}
					$dir->close();
				}
			}
			$j = count($stack);
		}
		return $size;
	}
/**
 * Recursively Remove directories if system allow.
 *
 * @param string $path Path of directory to delete
 * @return boolean Success
 * @access public
 */
	function delete($path) {
		$path = $this->slashTerm($path);
		if (is_dir($path) === true) {
			$files = glob($path . "*", GLOB_NOSORT);
			$normal_files = glob($path . "*");
			$hidden_files = glob($path . "\.?*");
			$files = array_merge($normal_files, $hidden_files);
			if(is_array($files)) {
				foreach ($files as $file) {
					if (preg_match("/(\.|\.\.)$/", $file)) {
						continue;
					}
					if (is_file($file) === true) {
						if(unlink($file)) {
							$this->__messages[] = sprintf(__('%s removed', true), $path);
						} else {
							$this->__errors[] = sprintf(__('%s NOT removed', true), $path);
						}
					} elseif (is_dir($file) === true) {
						if($this->delete($file) === false) {
							return false;
						}
					}
				}
			}
			$path = substr($path, 0, strlen($path) - 1);
			if(rmdir($path) === false) {
				$this->__errors[] = sprintf(__('%s NOT removed', true), $path);
				return false;
			} else {
				$this->__messages[] = sprintf(__('%s removed', true), $path);
			}
		}
		return true;
	}
/**
 * Recursive directory copy.
 *
 * @param array $options (to, from, chmod, skip)
 * @return boolean
 * @access public
 */
	function copy($options = array()) {
		$to = null;
		if(is_string($options)) {
			$to = $options;
			$options = array();
		}
		$options = am(array('to'=> $to, 'from'=> $this->path, 'mode'=> $this->mode, 'skip'=> array()), $options);

		$fromDir = $options['from'];
		$toDir = $options['to'];
		$mode = $options['mode'];

		if (!$this->cd($fromDir)) {
			$this->__errors[] = sprintf(__('%s not found', true), $fromDir);
			return false;
		}

		if(!is_dir($toDir)) {
			$this->mkdir($toDir, $mode);
		}

		if (!is_writable($toDir)) {
			$this->__errors[] = sprintf(__('%s not writable', true), $toDir);
			return false;
		}

		$exceptions = am(array('.','..','.svn'), $options['skip']);
		$handle = opendir($fromDir);
		if($handle) {
			while (false !== ($item = readdir($handle))) {
				if (!in_array($item, $exceptions)) {
					$from = $this->addPathElement($fromDir, $item);
					$to = $this->addPathElement($toDir, $item);
					if (is_file($from)) {
						if (copy($from, $to)) {
							chmod($to, intval($mode, 8));
							touch($to, filemtime($from));
							$this->__messages[] = sprintf(__('%s copied to %s', true), $from, $to);
						} else {
							$this->__errors[] = sprintf(__('%s NOT copied to %s', true), $from, $to);
						}
					}

					if (is_dir($from) && !file_exists($to)) {
						if (mkdir($to, intval($mode, 8))) {
							chmod($to, intval($mode, 8));
							$this->__messages[] = sprintf(__('%s created', true), $to);
							$options = am($options, array('to'=> $to, 'from'=> $from));
							$this->copy($options);
						} else {
							$this->__errors[] = sprintf(__('%s not created', true), $to);
						}
					}
				}
			}
			closedir($handle);
		} else {
			return false;
		}

		if(!empty($this->__errors)) {
			return false;
		}
		return true;
	}
/**
 * Recursive directory move.
 *
 * @param array $options (to, from, chmod, skip)
 * @return boolean Success
 * @access public
 */
	function move($options) {
		$to = null;
		if(is_string($options)) {
			$to = $options;
		}
		$options = am(array('to'=> $to, 'from'=> $this->path, 'mode'=> $this->mode, 'skip'=> array()), $options);

		if($this->copy($options)) {
			if($this->delete($options['from'])) {
				return $this->cd($options['to']);
			}
		}
		return false;
	}
/**
 * get messages from latest method
 *
 * @return array
 * @access public
 */
	function messages() {
		return $this->__messages;
	}
/**
 * get error from latest method
 *
 * @return array
 * @access public
 */
	function errors() {
		return $this->__errors;
	}
/**
 * nix flavored alias
 *
 * @see read
 * @access public
 */
	function ls($sort = true, $exceptions = false) {
		return $this->read($sort, $exceptions);
	}
/**
 * nix flavored alias
 *
 * @see create
 * @access public
 */
	function mkdir($pathname, $mode = 0755) {
		return $this->create($pathname, $mode);
	}
/**
 * nix flavored alias
 *
 * @see copy
 * @access public
 */
	function cp($options) {
		return $this->copy($options);
	}
/**
 * nix flavored alias
 *
 * @see move
 * @access public
 */
	function mv($options) {
		return $this->move($options);
	}
/**
 * nix flavored alias
 *
 * @see delete
 * @access public
 */
	function rm($path) {
		return $this->delete($path);
	}
/**
 *
 * @deprecated
 * @see chmod
 * @access public
 */
	function chmodr($pathname, $mode = 0755) {
		return $this->chmod($pathname, $mode);
	}
/**
 *
 * @deprecated
 * @see mkdir or create
 * @access public
 */
	function mkdirr($pathname, $mode = 0755) {
		return $this->create($pathname, $mode);
	}
}
?>