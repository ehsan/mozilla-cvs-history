<?php
/* SVN FILE: $Id: home.ctp,v 1.2 2007/11/19 08:49:55 rflint%ryanflint.com Exp $ */
/**
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
 * @subpackage		cake.cake.libs.view.templates.pages
 * @since			CakePHP(tm) v 0.10.0.1076
 * @version			$Revision: 1.2 $
 * @modifiedby		$LastChangedBy: phpnut $
 * @lastmodified	$Date: 2007/11/19 08:49:55 $
 * @license			http://www.opensource.org/licenses/mit-license.php The MIT License
 */
?>
<?php
if(Configure::read() > 0):
	Debugger::checkSessionKey();
endif;
?>
<p>
	<span class="notice">
		<?php
			if (is_writable(TMP)):
				__('Your tmp directory is writable.');
			else:
				__('Your tmp directory is NOT writable.');
			endif;
		?>
	</span>
</p>
<p>
	<span class="notice">
		<?php
			if (Cache::isInitialized()):
				__('Your cache is set up and initialized properly.');
				$settings = Cache::settings();
				echo '<p>';
				echo sprintf(__('%s is being used to cache, to change this edit config/core.php ', true), $settings['engine'] . 'Engine');
				echo '</p>';

				echo 'Settings: <ul>';
				foreach ($settings as $name => $value):
					echo '<li>' . $name . ': ' . $value . '</li>';
				endforeach;
				echo '</ul>';

			else:
				__('Your cache is NOT working.');
				echo '<br />';
				if (is_writable(TMP . 'cache')):
					__('Edit: config/core.php to insure you have the newset version of this file and the variable $cakeCache set properly');
				else:
					__('Your cache directory is not writable');
				endif;
			endif;
		?>
	</span>
</p>
<p>
	<span class="notice">
		<?php
			$filePresent = null;
			if (file_exists(CONFIGS.'database.php')):
				__('Your database configuration file is present.');
				$filePresent = true;
			else:
				__('Your database configuration file is NOT present.');
				echo '<br/>';
				__('Rename config/database.php.default to config/database.php');
			endif;
		?>
	</span>
</p>
<?php
if (!empty($filePresent)):
 	uses('model' . DS . 'connection_manager');
	$db = ConnectionManager::getInstance();
 	$connected = $db->getDataSource('default');
?>
<p>
	<span class="notice">
		<?php
			if ($connected->isConnected()):
		 		__('Cake is able to connect to the database.');
			else:
				__('Cake is NOT able to connect to the database.');
			endif;
		?>
	</span>
</p>
<?php endif;?>
<h2><?php echo sprintf(__('Release Notes for CakePHP %s.', true), Configure::version()); ?></h2>
<a href="https://trac.cakephp.org/wiki/notes/1.2.x.x"><?php __('Read the release notes and get the latest version'); ?> </a>
<h2><?php __('Editing this Page'); ?></h2>
<p>
<?php
__('To change the content of this page, edit: /app/views/pages/home.ctp.<br />
To change its layout, edit: /Users/phpnut/Sites /app/views/layouts/default.ctp.<br />
You can also add some CSS styles for your pages at: /app/webroot/css.');
?>
</p>
<h2><?php __('Getting Started'); ?></h2>
<p>
	<a href="http://manual.cakephp.org/appendix/blog_tutorial"><?php __('The 15 min Blog Tutorial'); ?></a><br />
	<a href="http://cakephp.org/files/OCPHP.pdf"><?php __('The OCPHP presentation on new features in 1.2'); ?></a><br />
</p>
<h2><?php __('More about Cake'); ?></h2>
<p>
<?php __('CakePHP is a rapid development framework for PHP which uses commonly known design patterns like Active Record, Association Data Mapping, Front Controller and MVC.'); ?>
</p>
<p>
<?php __('Our primary goal is to provide a structured framework that enables PHP users at all levels to rapidly develop robust web applications, without any loss to flexibility.'); ?>
</p>
<br />
<ul>
	<li><a href="http://www.cakefoundation.org/"><?php __('Cake Software Foundation'); ?> </a>
	<ul><li><?php __('Promoting development related to CakePHP'); ?></li></ul></li>
	<li><a href="http://live.cakephp.org"><?php __('The Show'); ?> </a>
	<ul><li><?php __('The Show is a weekly live internet radio broadcast where we discuss CakePHP-related topics and answer questions live via IRC, Skype, and telephone.'); ?></li></ul></li>
	<li><a href="http://bakery.cakephp.org"><?php __('The Bakery'); ?> </a>
	<ul><li><?php __('Everything CakePHP'); ?></li></ul></li>
	<li><a href="http://astore.amazon.com/cakesoftwaref-20/"><?php __('Book Store'); ?> </a>
	<ul><li><?php __('Recommended Software Books'); ?></li></ul></li>
	<li><a href="http://www.cafepress.com/cakefoundation"><?php __('CakeSchwag'); ?> </a>
	<ul><li><?php __('Get your own CakePHP gear - Doughnate to Cake'); ?></li></ul></li>
	<li><a href="http://www.cakephp.org"><?php __('CakePHP'); ?> </a>
	<ul><li><?php __('The Rapid Development Framework'); ?></li></ul></li>
	<li><a href="http://manual.cakephp.org"><?php __('CakePHP Manual'); ?> </a>
	<ul><li><?php __('Your Rapid Development Cookbook'); ?></li></ul></li>
	<li><a href="http://api.cakephp.org"><?php __('CakePHP API'); ?> </a>
	<ul><li><?php __('Docblock Your Best Friend'); ?></li></ul></li>
	<li><a href="http://www.cakeforge.org"><?php __('CakeForge'); ?> </a>
	<ul><li><?php __('Open Development for CakePHP'); ?></li></ul></li>
	<li><a href="https://trac.cakephp.org/"><?php __('CakePHP Trac'); ?> </a>
	<ul><li><?php __('For the Development of CakePHP (Tickets, SVN browser, Roadmap, Changelogs)'); ?></li></ul></li>
	<li><a href="http://groups-beta.google.com/group/cake-php"><?php __('CakePHP Google Group'); ?> </a>
	<ul><li><?php __('Community mailing list'); ?></li></ul></li>
	<li><a href="irc://irc.freenode.net/cakephp">irc.freenode.net #cakephp</a>
	<ul><li><?php __('Live chat about CakePHP'); ?></li></ul></li>
</ul>