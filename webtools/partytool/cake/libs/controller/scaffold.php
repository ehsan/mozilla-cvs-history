<?php
/* SVN FILE: $Id: scaffold.php,v 1.2 2007/11/19 08:49:53 rflint%ryanflint.com Exp $ */
/**
 * Scaffold.
 *
 * Automatic forms and actions generation for rapid web application development.
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
 * @subpackage		cake.cake.libs.controller
 * @since		Cake v 0.10.0.1076
 * @version			$Revision: 1.2 $
 * @modifiedby		$LastChangedBy: phpnut $
 * @lastmodified	$Date: 2007/11/19 08:49:53 $
 * @license			http://www.opensource.org/licenses/mit-license.php The MIT License
 */
/**
 * Scaffolding is a set of automatic views, forms and controllers for starting web development work faster.
 *
 * Scaffold inspects your database tables, and making educated guesses, sets up a
 * number of pages for each of your Models. These pages have data forms that work,
 * and afford the web developer an early look at the data, and the possibility to over-ride
 * scaffolded actions with custom-made ones.
 *
 * @package		cake
 * @subpackage	cake.cake.libs.controller
 */
class Scaffold extends Object {
/**
 * Controller object
 *
 * @var object
 * @access public
 */
	var $controller = null;
/**
 * Name of the controller to scaffold
 *
 * @var string Name of controller
 * @access public
 */
	var $name = null;
/**
 * Action to be performed.
 *
 * @var string Name of action
 * @access public
 */
	var $action = null;
/**
 * Name of current model this view context is attached to
 *
 * @var string
 * @access public
 */
	var $model = null;
/**
 * Path to View.
 *
 * @var string
 * @access public
 */
	var $viewPath;
/**
 * Path parts for creating links in views.
 *
 * @var string Base URL
 * @access public
 */
	var $base = null;
/**
 * Name of layout to use with this View.
 *
 * @var string
 * @access public
 */
	var $layout = 'default';
/**
 * Array of parameter data
 *
 * @var array Parameter data
 */
	var $params;
/**
 * File extension. Defaults to Cake's template ".ctp".
 *
 * @var array
 * @access public
 */
	var $ext = '.ctp';
/**
 * Sub-directory for this view file.
 *
 * @var string
 * @access public
 */
	var $subDir = null;
/**
 * Plugin name. A Plugin is a sub-application. New in Cake RC4.
 *
 * @link http://wiki.cakephp.org/docs:plugins
 * @var string
 * @access public
 */
	var $plugin = null;
/**
 * List of variables to collect from the associated controller
 *
 * @var array
 * @access private
 */
	var $__passedVars = array('action', 'base', 'webroot', 'layout', 'name', 'viewPath', 'ext', 'params', 'data', 'webservices', 'plugin', 'cacheAction');
/**
 * Title HTML element for current scaffolded view
 *
 * @var string
 * @access public
 */
	var $scaffoldTitle = null;
/**
 * Construct and set up given controller with given parameters.
 *
 * @param string $controller_class Name of controller
 * @param array $params
 */
	function __construct(&$controller, $params) {
		$this->controller = &$controller;

		$count = count($this->__passedVars);
		for ($j = 0; $j < $count; $j++) {
			$var = $this->__passedVars[$j];
			$this->{$var} = $controller->{$var};
		}
		$this->redirect = array('action'=> 'index');
		if (!is_null($this->plugin)) {
			$this->redirect = '/' . $this->plugin . '/' . $this->viewPath;
		}

		if (!in_array('Form', $this->controller->helpers)) {
			$this->controller->helpers[] = 'Form';
		}

		if ($this->controller->constructClasses() === false) {
			return $this->cakeError('missingModel', array(array('className' => $this->modelKey, 'webroot' => '', 'base' => $this->controller->base)));
		}

		if (!empty($controller->uses) && class_exists($controller->uses[0])) {
			$controller->modelClass = $controller->uses[0];
			$controller->modelKey = Inflector::underscore($controller->modelClass);
		}
		$this->modelClass = $controller->modelClass;
		$this->modelKey = $controller->modelKey;

		if (!is_object($this->controller->{$this->modelClass})) {
			return $this->cakeError('missingModel', array(array('className' => $this->modelClass, 'webroot' => '', 'base' => $controller->base)));
		}
		$this->ScaffoldModel =& $this->controller->{$this->modelClass};
		$this->scaffoldTitle = Inflector::humanize($this->viewPath);
		$this->scaffoldActions = $controller->scaffold;
		$this->controller->pageTitle = __('Scaffold :: ', true) . Inflector::humanize($this->action) . ' :: ' . $this->scaffoldTitle;

		//template variables
		$modelClass = $this->controller->modelClass;
		$modelKey = $this->controller->modelKey;
		$primaryKey = $this->ScaffoldModel->primaryKey;
		$displayField = $this->ScaffoldModel->displayField;
		$singularVar = Inflector::variable($modelClass);
		$pluralVar = Inflector::variable($this->controller->name);
		$singularHumanName = Inflector::humanize($modelClass);
		$pluralHumanName = Inflector::humanize($this->controller->name);
		$fields = $this->ScaffoldModel->_tableInfo->value;
		$foreignKeys = $this->ScaffoldModel->keyToTable;
		$belongsTo = $this->ScaffoldModel->belongsTo;
		$hasOne = $this->ScaffoldModel->hasOne;
		$hasMany = $this->ScaffoldModel->hasMany;
		$hasAndBelongsToMany = $this->ScaffoldModel->hasAndBelongsToMany;

		$this->controller->set(compact('modelClass', 'primaryKey', 'displayField', 'singularVar', 'pluralVar',
								'singularHumanName', 'pluralHumanName', 'fields', 'foreignKeys',
									'belongsTo', 'hasOne', 'hasMany', 'hasAndBelongsToMany'));
		$this->__scaffold($params);
	 }
/**
 * Renders a view action of scaffolded model.
 *
 * @param array $params Parameters for scaffolding
 * @return A rendered view of a row from Models database table
 * @access private
 */
	function __scaffoldView($params) {
		if ($this->controller->_beforeScaffold('view')) {

			if (isset($params['pass'][0])) {
				$this->ScaffoldModel->id = $params['pass'][0];
			} elseif (isset($this->controller->Session) && $this->controller->Session->valid != false) {
				$this->controller->Session->setFlash(sprintf(__("No id set for %s::view()", true), Inflector::humanize($this->modelKey)));
				$this->controller->redirect($this->redirect);
			} else {
				return $this->controller->flash(sprintf(__("No id set for %s::view()", true), Inflector::humanize($this->modelKey)),
																		'/' . Inflector::underscore($this->controller->viewPath));
			}

			$this->controller->data = $this->ScaffoldModel->read();
			$this->controller->set(Inflector::variable($this->controller->modelClass), $this->controller->data);
			$this->controller->render($this->action, $this->layout, $this->__getViewFileName($params['action']));
		} elseif ($this->controller->_scaffoldError('view') === false) {
			return $this->__scaffoldError();
		}
	}
/**
 * Renders index action of scaffolded model.
 *
 * @param array $params Parameters for scaffolding
 * @return A rendered view listing rows from Models database table
 * @access private
 */
	function __scaffoldIndex($params) {
		if ($this->controller->_beforeScaffold('index')) {
	 		$this->ScaffoldModel->recursive = 0;
	 		$this->controller->set(Inflector::variable($this->controller->name), $this->controller->paginate());
	 		$this->controller->render($this->action, $this->layout, $this->__getViewFileName($params['action']));
		} elseif ($this->controller->_scaffoldError('index') === false) {
			return $this->__scaffoldError();
		}
	}
/**
 * Renders an add or edit action for scaffolded model.
 *
 * @param string $action Action (add or edit)
 * @return A rendered view with a form to edit or add a record in the Models database table
 * @access private
 */
	function __scaffoldForm($action = 'edit') {
		$this->controller->render($action, $this->layout, $this->__getViewFileName($action));
	}
/**
 * Saves or updates the scaffolded model.
 *
 * @param array $params Parameters for scaffolding
 * @param string $action create or update
 * @return success on save/update, add/edit form if data is empty or error if save or update fails
 * @access private
 */
	function __scaffoldSave($params = array(), $action = 'edit') {
		$formAction = 'edit';
		$success = __('updated', true);
		if ($action === 'add') {
			$formAction = 'add';
			$success = __('saved', true);
		}

		if ($this->controller->_beforeScaffold($action)) {
			if ($action == 'edit' && (!isset($params['pass'][0]) || !$this->ScaffoldModel->exists())) {
				if (isset($this->controller->Session) && $this->controller->Session->valid != false) {
					$this->controller->Session->setFlash(sprintf(__("Invalid id for %s::edit()", true), Inflector::humanize($this->modelKey)));
					$this->controller->redirect($this->redirect);
				} else {
					return $this->controller->flash(sprintf(__("Invalid id for %s::edit()", true), Inflector::humanize($this->modelKey)), $this->redirect);
				}
			} elseif ($action == 'edit') {
				$this->ScaffoldModel->id = $params['pass'][0];
			}

			if (!empty($this->controller->data)) {

				$this->controller->cleanUpFields();

				if ($action == 'create') {
					$this->ScaffoldModel->create();
				}

				if ($this->ScaffoldModel->save($this->controller->data)) {
					if ($this->controller->_afterScaffoldSave($action)) {
						if (isset($this->controller->Session) && $this->controller->Session->valid != false) {
							$this->controller->Session->setFlash(sprintf(__('The %1$s has been %2$s', true), Inflector::humanize($this->modelClass), $success));
							$this->controller->redirect($this->redirect);
						} else {
							return $this->controller->flash(sprintf(__('The %1$s has been %2$s', true), Inflector::humanize($this->modelClass), $success), $this->redirect);
						}
					} else {
						return $this->controller->_afterScaffoldSaveError($action);
					}
				} else {
					if (isset($this->controller->Session) && $this->controller->Session->valid != false) {
						$this->controller->Session->setFlash(__('Please correct errors below.', true));
					}
				}

			}

			if (empty($this->controller->data)) {
				if ($this->ScaffoldModel->id) {
					$this->controller->data = $this->ScaffoldModel->read();
				} else {
					$this->controller->data = $this->ScaffoldModel->create();
				}
			}

			foreach ($this->ScaffoldModel->belongsTo as $assocName => $assocData) {
				$varName = Inflector::variable(Inflector::pluralize(preg_replace('/_id$/', '', $assocData['foreignKey'])));
				$this->controller->set($varName, $this->ScaffoldModel->{$assocName}->generateList());
			}
			foreach ($this->ScaffoldModel->hasAndBelongsToMany as $assocName => $assocData) {
				$varName = Inflector::variable(Inflector::pluralize($assocName));
				$this->controller->set($varName, $this->ScaffoldModel->{$assocName}->generateList());
			}

			return $this->__scaffoldForm($formAction);

		} elseif ($this->controller->_scaffoldError($action) === false) {
			return $this->__scaffoldError();
		}
	}
/**
 * Performs a delete on given scaffolded Model.
 *
 * @param array $params Parameters for scaffolding
 * @return success on delete error if delete fails
 * @access private
 */
	function __scaffoldDelete($params = array()) {
		if ($this->controller->_beforeScaffold('delete')) {

			if (isset($params['pass'][0])) {
				$id = $params['pass'][0];
			} elseif (isset($this->controller->Session) && $this->controller->Session->valid != false) {
				$this->controller->Session->setFlash(sprintf(__("No id set for %s::delete()", true), Inflector::humanize($this->modelKey)));
				$this->controller->redirect($this->redirect);
			} else {
				return $this->controller->flash(sprintf(__("No id set for %s::delete()", true), Inflector::humanize($this->modelKey)),
																	'/' . Inflector::underscore($this->controller->viewPath));
			}

			if ($this->ScaffoldModel->del($id)) {
				if (isset($this->controller->Session) && $this->controller->Session->valid != false) {
					$this->controller->Session->setFlash(sprintf(__('The %1$s with id: %2$d has been deleted.', true), Inflector::humanize($this->modelClass), $id));
					$this->controller->redirect($this->redirect);
				} else {
					return $this->controller->flash(sprintf(__('The %1$s with id: %2$d has been deleted.', true), Inflector::humanize($this->modelClass), $id), '/' . $this->viewPath);
				}
			} else {
				if (isset($this->controller->Session) && $this->controller->Session->valid != false) {
					$this->controller->Session->setFlash(sprintf(__('There was an error deleting the %1$s with id: %2$d', true), Inflector::humanize($this->modelClass), $id));
					$this->controller->redirect($this->redirect);
				} else {
					return $this->controller->flash(sprintf(__('There was an error deleting the %1$s with id: %2$d', true), Inflector::humanize($this->modelClass), $id), '/' . $this->viewPath);
				}
			}

		} elseif ($this->controller->_scaffoldError('delete') === false) {
			return $this->__scaffoldError();
		}
	}
/**
 * Show a scaffold error
 *
 * @return A rendered view showing the error
 * @access private
 */
	function __scaffoldError() {
		$pathToViewFile = '';

		if (file_exists(APP . 'views' . DS . $this->viewPath . DS . 'scaffolds' . DS. 'scaffold.error.ctp')) {
			$pathToViewFile = APP . 'views' . DS . $this->viewPath . DS . 'scaffolds' . DS . 'scaffold.error.ctp';
		} elseif (file_exists(APP . 'views' . DS . $this->viewPath . DS . 'scaffolds' . DS. 'scaffold.error.thtml')) {
			$pathToViewFile = APP . 'views' . DS . $this->viewPath . DS . 'scaffolds' . DS . 'scaffold.error.thtml';
		} elseif (file_exists(APP . 'views' . DS . 'scaffolds' . DS . 'scaffold.error.ctp')) {
			$pathToViewFile = APP . 'views' . DS . 'scaffolds' . DS . 'scaffold.error.ctp';
		} elseif (file_exists(APP . 'views' . DS . 'scaffolds' . DS . 'scaffold.error.thtml')) {
			$pathToViewFile = APP . 'views' . DS . 'scaffolds' . DS . 'scaffold.error.thtml';
		} else {
			$pathToViewFile = LIBS . 'view' . DS . 'templates' . DS . 'errors' . DS . 'scaffold_error.ctp';
		}

		return $this->controller->render($this->action, $this->layout, $pathToViewFile);
	}
/**
 * When methods are now present in a controller
 * scaffoldView is used to call default Scaffold methods if:
 * <code>
 * var $scaffold;
 * </code>
 * is placed in the controller's class definition.
 *
 * @param array $params Parameters for scaffolding
 * @since Cake v 0.10.0.172
 * @access private
 */
	function __scaffold($params) {
		$db = &ConnectionManager::getDataSource($this->ScaffoldModel->useDbConfig);

        $admin = Configure::read('Routing.admin');
		if (isset($db)) {
			if (empty($this->scaffoldActions)) {
				$this->scaffoldActions = array('index', 'list', 'view', 'add', 'create', 'edit', 'update', 'delete');
			} elseif (!empty($admin) && $this->scaffoldActions === $admin) {
				$this->scaffoldActions = array($admin .'_index', $admin .'_list', $admin .'_view', $admin .'_add', $admin .'_create', $admin .'_edit', $admin .'_update', $admin .'_delete');
			}

			if (in_array($params['action'], $this->scaffoldActions)) {
				if (!empty($admin)) {
					$params['action'] = str_replace($admin . '_', '', $params['action']);
				}
				switch($params['action']) {
					case 'index':
						$this->__scaffoldIndex($params);
					break;
					case 'view':
						$this->__scaffoldView($params);
					break;
					case 'list':
						$this->__scaffoldIndex($params);
					break;
					case 'add':
						$this->__scaffoldSave($params, 'add');
					break;
					case 'edit':
						$this->__scaffoldSave($params, 'edit');
					break;
					case 'create':
						$this->__scaffoldSave($params, 'add');
					break;
					case 'update':
						$this->__scaffoldSave($params, 'edit');
					break;
					case 'delete':
						$this->__scaffoldDelete($params);
					break;
				}
			} else {
				return $this->cakeError('missingAction', array(array('className' => $this->controller->name . "Controller",
																						'base' => $this->controller->base,
																						'action' => $this->action,
																						'webroot' => $this->controller->webroot)));
			}
		} else {
			return $this->cakeError('missingDatabase', array(array('webroot' => $this->controller->webroot)));
		}
	}
/**
 * Returns scaffold view filename of given action's template file (.ctp) as a string.
 *
 * @param string $action Controller action to find template filename for
 * @return string Template filename
 * @access private
 */
	function __getViewFileName($action) {
		$action = Inflector::underscore($action);
		$scaffoldAction = 'scaffold.'.$action;
		$paths = Configure::getInstance();

		if (!is_null($this->webservices)) {
			$type = strtolower($this->webservices) . DS;
		} else {
			$type = null;
		}

		if (!is_null($this->plugin)) {

			if (file_exists(APP . 'views' . DS . 'plugins' . DS . $this->plugin . DS . $this->subDir . $type . $scaffoldAction . $this->ext)) {
				return APP . 'views' . DS . 'plugins' . DS . $this->plugin . DS . $this->subDir . $type . $scaffoldAction . $this->ext;
			} elseif (file_exists(APP . 'views' . DS . 'plugins' . DS . $this->plugin . DS . $this->subDir . $type . $scaffoldAction . $this->ext)) {
				return APP . 'views' . DS . 'plugins' . DS . $this->plugin . DS . $this->subDir . $type . $scaffoldAction . $this->ext;
			} elseif (file_exists(APP . 'plugins' . DS . $this->plugin . DS . 'views' . DS . $this->viewPath . DS . $scaffoldAction . $this->ext)) {
				return APP . 'plugins' . DS . $this->plugin . DS . 'views' . DS . $this->viewPath . DS . $scaffoldAction . $this->ext;
			} elseif (file_exists(APP . 'views' . DS . 'plugins' . DS . $this->plugin . DS . $this->subDir . $type . $scaffoldAction . '.ctp')) {
				return APP . 'views' . DS . 'plugins' . DS . $this->plugin . DS . $this->subDir . $type . $scaffoldAction . '.ctp';
			} elseif (file_exists(APP . 'views' . DS . 'plugins' . DS . $this->plugin . DS . $this->subDir . $type . $scaffoldAction . '.thtml')) {
				return APP . 'views' . DS . 'plugins' . DS . $this->plugin . DS . $this->subDir . $type . $scaffoldAction . '.thtml';
			} elseif (file_exists(APP . 'views' . DS . 'plugins' . DS . 'scaffolds'. DS . $this->subDir . $type . $scaffoldAction . '.ctp')) {
				return APP . 'views' . DS . 'plugins' . DS . 'scaffolds'. DS . $this->subDir . $type . $scaffoldAction . '.ctp';
			} elseif (file_exists(APP . 'views' . DS . 'plugins' . DS . 'scaffolds'. DS . $this->subDir . $type . $scaffoldAction . '.thtml')) {
				return APP . 'views' . DS . 'plugins' . DS . 'scaffolds'. DS . $this->subDir . $type . $scaffoldAction . '.thtml';
			} elseif (file_exists(APP . 'plugins' . DS . $this->plugin . DS . 'views' . DS . $this->viewPath . DS . $scaffoldAction . '.ctp')) {
				return APP . 'plugins' . DS . $this->plugin . DS . 'views' . DS . $this->viewPath . DS . $scaffoldAction . '.ctp';
			} elseif (file_exists(APP . 'plugins' . DS . $this->plugin . DS . 'views' . DS . $this->viewPath . DS . $scaffoldAction . '.thtml')) {
				return APP . 'plugins' . DS . $this->plugin . DS . 'views' . DS . $this->viewPath . DS . $scaffoldAction . '.thtml';
			}
		}

		foreach ($paths->viewPaths as $path) {
			if (file_exists($path . $this->viewPath . DS . $this->subDir . $type . $scaffoldAction . $this->ext)) {
				return $path . $this->viewPath . DS . $this->subDir . $type . $scaffoldAction . $this->ext;
			} elseif (file_exists($path . 'scaffolds' . DS . $this->subDir . $type . $scaffoldAction . $this->ext)) {
				return $path . 'scaffolds' . DS . $this->subDir . $type . $scaffoldAction . $this->ext;
			} elseif (file_exists($path . $this->viewPath . DS . $this->subDir . $type . $scaffoldAction . '.ctp')) {
				return $path . $this->viewPath . DS . $this->subDir . $type . $scaffoldAction . '.ctp';
			} elseif (file_exists($path . $this->viewPath . DS . $this->subDir . $type . $scaffoldAction . '.thtml')) {
				return $path . $this->viewPath . DS . $this->subDir . $type . $scaffoldAction . '.thtml';
			} elseif (file_exists($path . 'scaffolds' . DS . $this->subDir . $type . $scaffoldAction . '.ctp')) {
				return $path . 'scaffolds' . DS . $this->subDir . $type . $scaffoldAction . '.ctp';
			} elseif (file_exists($path . 'scaffolds' . DS . $this->subDir . $type . $scaffoldAction . '.thtml')) {
				return $path . 'scaffolds' . DS . $this->subDir . $type . $scaffoldAction . '.thtml';
			}
		}
		if ($action === 'add') {
			$action = 'edit';
		}
		return LIBS . 'view' . DS . 'templates' . DS . 'scaffolds' . DS . $action . '.ctp';
	}
}
?>