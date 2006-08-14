-- version 2.8.1
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Aug 10, 2006 at 02:36 PM
-- Server version: 4.1.20
-- PHP Version: 4.3.9
--
-- Database: `remora`
--

-- --------------------------------------------------------

--
-- Table structure for table `addons`
--

DROP TABLE IF EXISTS `addons`;
CREATE TABLE `addons` (
 `id` int(11) unsigned NOT NULL auto_increment,
 `guid` varchar(255) NOT NULL default '',
 `name` varchar(255) NOT NULL default '',
 `addontype_id` int(11) unsigned NOT NULL default '0',
 `icon` varchar(255) NOT NULL default '',
 `homepage` varchar(255) NOT NULL default '',
 `description` text NOT NULL,
 `summary` text NOT NULL,
 `averagerating` varchar(255) NOT NULL default '',
 `weeklydownloads` int(11) unsigned NOT NULL default '0',
 `totaldownloads` int(11) unsigned NOT NULL default '0',
 `developercomments` text NOT NULL,
 `inactive` tinyint(1) unsigned NOT NULL default '0',
 `prerelease` tinyint(1) unsigned NOT NULL default '0',
 `adminreview` tinyint(1) unsigned NOT NULL default '0',
 `sitespecific` tinyint(1) unsigned NOT NULL default '0',
 `externalsoftware` tinyint(1) unsigned NOT NULL default '0',
 `created` datetime NOT NULL default '0000-00-00 00:00:00',
 `modified` datetime NOT NULL default '0000-00-00 00:00:00',
 `approvalnotes` text NOT NULL,
 `eula` text NOT NULL,
 `privacypolicy` text NOT NULL,
 PRIMARY KEY  (`id`),
 KEY `addontype_id` (`addontype_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- Dumping data for table `addons`
--


-- --------------------------------------------------------

--
-- Table structure for table `addons_tags`
--

DROP TABLE IF EXISTS `addons_tags`;
CREATE TABLE `addons_tags` (
 `addon_id` int(11) unsigned NOT NULL default '0',
 `tag_id` int(11) unsigned NOT NULL default '0',
 PRIMARY KEY  (`addon_id`,`tag_id`),
 KEY `tag_id` (`tag_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `addons_tags`
--


-- --------------------------------------------------------

--
-- Table structure for table `addons_users`
--

DROP TABLE IF EXISTS `addons_users`;
CREATE TABLE `addons_users` (
 `addon_id` int(11) unsigned NOT NULL default '0',
 `user_id` int(11) unsigned NOT NULL default '0',
 `created` datetime NOT NULL default '0000-00-00 00:00:00',
 `modified` datetime NOT NULL default '0000-00-00 00:00:00',
 PRIMARY KEY  (`addon_id`,`user_id`),
 KEY `user_id` (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `addons_users`
--


-- --------------------------------------------------------

--
-- Table structure for table `addontypes`
--

DROP TABLE IF EXISTS `addontypes`;
CREATE TABLE `addontypes` (
 `id` int(11) unsigned NOT NULL auto_increment,
 `name` varchar(255) character set latin1 NOT NULL default '',
 `created` datetime NOT NULL default '0000-00-00 00:00:00',
 `modified` datetime NOT NULL default '0000-00-00 00:00:00',
 PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- Dumping data for table `addontypes`
--


-- --------------------------------------------------------

--
-- Table structure for table `applications`
--

DROP TABLE IF EXISTS `applications`;
CREATE TABLE `applications` (
 `id` int(11) unsigned NOT NULL auto_increment,
 `guid` varchar(255) character set latin1 NOT NULL default '',
 `name` varchar(255) character set latin1 NOT NULL default '',
 `shortname` varchar(255) character set latin1 NOT NULL default '',
 `supported` tinyint(1) NOT NULL default '0',
 `created` datetime NOT NULL default '0000-00-00 00:00:00',
 `modified` datetime NOT NULL default '0000-00-00 00:00:00',
 PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- Dumping data for table `applications`
--


-- --------------------------------------------------------

--
-- Table structure for table `applications_versions`
--

DROP TABLE IF EXISTS `applications_versions`;
CREATE TABLE `applications_versions` (
 `application_id` int(11) unsigned NOT NULL default '0',
 `version_id` int(11) unsigned NOT NULL default '0',
 `min` varchar(255) character set latin1 NOT NULL default '',
 `max` varchar(255) character set latin1 NOT NULL default '',
 `public` tinyint(1) unsigned NOT NULL default '0',
 `created` datetime NOT NULL default '0000-00-00 00:00:00',
 `modified` datetime NOT NULL default '0000-00-00 00:00:00',
 PRIMARY KEY  (`application_id`,`version_id`),
 KEY `version_id` (`version_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `applications_versions`
--


-- --------------------------------------------------------

--
-- Table structure for table `approvals`
--

DROP TABLE IF EXISTS `approvals`;
CREATE TABLE `approvals` (
 `id` int(11) unsigned NOT NULL auto_increment,
 `user_id` int(11) unsigned NOT NULL default '0',
 `action` varchar(255) character set latin1 NOT NULL default '',
 `file_id` int(11) unsigned NOT NULL default '0',
 `comments` text character set latin1 NOT NULL,
 PRIMARY KEY  (`id`),
 KEY `user_id` (`user_id`),
 KEY `file_id` (`file_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- Dumping data for table `approvals`
--


-- --------------------------------------------------------

--
-- Table structure for table `appversions`
--

DROP TABLE IF EXISTS `appversions`;
CREATE TABLE `appversions` (
 `id` int(11) unsigned NOT NULL auto_increment,
 `application_id` int(11) unsigned NOT NULL default '0',
 `version` varchar(255) NOT NULL default '',
 `created` datetime NOT NULL default '0000-00-00 00:00:00',
 `modified` datetime NOT NULL default '0000-00-00 00:00:00',
 PRIMARY KEY  (`id`),
 KEY `application_id` (`application_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- Dumping data for table `appversions`
--


-- --------------------------------------------------------

--
-- Table structure for table `blapps`
--

DROP TABLE IF EXISTS `blapps`;
CREATE TABLE `blapps` (
 `id` int(11) unsigned NOT NULL auto_increment,
 `blitem_id` int(11) unsigned NOT NULL default '0',
 `guid` int(11) default NULL,
 `min` varchar(255) character set latin1 default NULL,
 `max` varchar(255) character set latin1 default NULL,
 `created` datetime NOT NULL default '0000-00-00 00:00:00',
 `modified` datetime NOT NULL default '0000-00-00 00:00:00',
 PRIMARY KEY  (`id`),
 KEY `blitem_id` (`blitem_id`),
 KEY `guid` (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- Dumping data for table `blapps`
--


-- --------------------------------------------------------

--
-- Table structure for table `blitems`
--

DROP TABLE IF EXISTS `blitems`;
CREATE TABLE `blitems` (
 `id` int(11) unsigned NOT NULL auto_increment,
 `guid` varchar(255) character set latin1 NOT NULL default '',
 `min` varchar(255) character set latin1 default NULL,
 `max` varchar(255) character set latin1 default NULL,
 `created` datetime NOT NULL default '0000-00-00 00:00:00',
 `modified` datetime NOT NULL default '0000-00-00 00:00:00',
 PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- Dumping data for table `blitems`
--


-- --------------------------------------------------------

--
-- Table structure for table `downloads`
--

DROP TABLE IF EXISTS `downloads`;
CREATE TABLE `downloads` (
 `id` int(11) unsigned NOT NULL auto_increment,
 `file_id` int(11) unsigned default NULL,
 `userip` varchar(255) NOT NULL default '',
 `useragent` varchar(255) NOT NULL default '',
 `counted` tinyint(1) unsigned NOT NULL default '0',
 `created` datetime NOT NULL default '0000-00-00 00:00:00',
 PRIMARY KEY  (`id`),
 KEY `file_id` (`file_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- Dumping data for table `downloads`
--


-- --------------------------------------------------------

--
-- Table structure for table `features`
--

DROP TABLE IF EXISTS `features`;
CREATE TABLE `features` (
 `id` int(11) unsigned NOT NULL auto_increment,
 `addon_id` int(11) unsigned NOT NULL default '0',
 `start` datetime NOT NULL default '0000-00-00 00:00:00',
 `end` datetime NOT NULL default '0000-00-00 00:00:00',
 `created` datetime NOT NULL default '0000-00-00 00:00:00',
 `modified` datetime NOT NULL default '0000-00-00 00:00:00',
 PRIMARY KEY  (`id`),
 KEY `addon_id` (`addon_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- Dumping data for table `features`
--


-- --------------------------------------------------------

--
-- Table structure for table `files`
--

DROP TABLE IF EXISTS `files`;
CREATE TABLE `files` (
 `id` int(11) unsigned NOT NULL auto_increment,
 `version_id` int(11) unsigned NOT NULL default '0',
 `platform_id` int(11) unsigned NOT NULL default '0',
 `filename` varchar(255) character set latin1 NOT NULL default '',
 `size` varchar(255) character set latin1 NOT NULL default '',
 `hash` varchar(255) character set latin1 NOT NULL default '',
 `created` datetime NOT NULL default '0000-00-00 00:00:00',
 `modified` datetime NOT NULL default '0000-00-00 00:00:00',
 PRIMARY KEY  (`id`),
 KEY `version_id` (`version_id`),
 KEY `platform_id` (`platform_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- Dumping data for table `files`
--


-- --------------------------------------------------------

--
-- Table structure for table `langs`
--

DROP TABLE IF EXISTS `langs`;
CREATE TABLE `langs` (
 `id` varchar(255) NOT NULL default '',
 `name` varchar(255) NOT NULL default '',
 `meta` varchar(255) NOT NULL default '',
 `error_text` varchar(255) NOT NULL default '',
 PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Dumping data for table `langs`
--

INSERT INTO `langs` (`id`, `name`, `meta`, `error_text`) VALUES ('en-US', 'en-US', 'en-US', ''),
('en-GB', 'en-GB', 'en-GB', ''),
('fr', 'fr', 'fr', ''),
('de', 'de', 'de', ''),
('ja', 'ja', 'ja', ''),
('pl', 'pl', 'pl', ''),
('es-ES', 'es-ES', 'es-ES', ''),
('zh-CN', 'zh-CN', 'zh-CN', ''),
('zh-TW', 'zh-TW', 'zh-TW', ''),
('cs', 'cs', 'cs', ''),
('da', 'da', 'da', ''),
('nl', 'nl', 'nl', ''),
('fi', 'fi', 'fi', ''),
('hu', 'hu', 'hu', ''),
('it', 'it', 'it', ''),
('ko', 'ko', 'ko', ''),
('pt-BR', 'pt-BR', 'pt-BR', ''),
('ru', 'ru', 'ru', ''),
('es-AR', 'es-AR', 'es-AR', ''),
('sv-SE', 'sv-SE', 'sv-SE', ''),
('tr', 'tr', 'tr', '');

-- --------------------------------------------------------

--
-- Table structure for table `platforms`
--

DROP TABLE IF EXISTS `platforms`;
CREATE TABLE `platforms` (
 `id` int(11) unsigned NOT NULL auto_increment,
 `name` varchar(255) NOT NULL default '',
 `shortname` varchar(255) NOT NULL default '',
 `created` datetime NOT NULL default '0000-00-00 00:00:00',
 `modified` datetime NOT NULL default '0000-00-00 00:00:00',
 PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- Dumping data for table `platforms`
--


-- --------------------------------------------------------

--
-- Table structure for table `previews`
--

DROP TABLE IF EXISTS `previews`;
CREATE TABLE `previews` (
 `id` int(11) unsigned NOT NULL auto_increment,
 `addon_id` int(11) unsigned NOT NULL default '0',
 `filename` varchar(255) NOT NULL default '',
 `caption` varchar(255) NOT NULL default '',
 `highlight` tinyint(1) unsigned NOT NULL default '0',
 `created` datetime NOT NULL default '0000-00-00 00:00:00',
 `modified` datetime NOT NULL default '0000-00-00 00:00:00',
 PRIMARY KEY  (`id`),
 KEY `addon_id` (`addon_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- Dumping data for table `previews`
--


-- --------------------------------------------------------

--
-- Table structure for table `reviewratings`
--

DROP TABLE IF EXISTS `reviewratings`;
CREATE TABLE `reviewratings` (
 `review_id` int(11) unsigned NOT NULL default '0',
 `user_id` int(11) unsigned NOT NULL default '0',
 `helpful` tinyint(1) unsigned NOT NULL default '0',
 `created` datetime NOT NULL default '0000-00-00 00:00:00',
 PRIMARY KEY  (`review_id`,`user_id`),
 KEY `user_id` (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `reviewratings`
--


-- --------------------------------------------------------

--
-- Table structure for table `reviews`
--

DROP TABLE IF EXISTS `reviews`;
CREATE TABLE `reviews` (
 `id` int(11) unsigned NOT NULL auto_increment,
 `version_id` int(11) unsigned NOT NULL default '0',
 `rating` int(11) unsigned NOT NULL default '0',
 `title` varchar(255) NOT NULL default '',
 `body` text NOT NULL,
 `editorreview` tinyint(1) unsigned NOT NULL default '0',
 `flag` tinyint(1) unsigned NOT NULL default '0',
 `created` datetime NOT NULL default '0000-00-00 00:00:00',
 `modified` datetime NOT NULL default '0000-00-00 00:00:00',
 PRIMARY KEY  (`id`),
 KEY `version_id` (`version_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- Dumping data for table `reviews`
--


-- --------------------------------------------------------

--
-- Table structure for table `session_data`
--

DROP TABLE IF EXISTS `session_data`;
CREATE TABLE `session_data` (
 `id` int(11) unsigned NOT NULL auto_increment,
 `user_id` int(11) unsigned default NULL,
 `expires` datetime NOT NULL default '0000-00-00 00:00:00',
 `data` text NOT NULL,
 PRIMARY KEY  (`id`),
 KEY `user_id` (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- Dumping data for table `session_data`
--


-- --------------------------------------------------------

--
-- Table structure for table `tags`
--

DROP TABLE IF EXISTS `tags`;
CREATE TABLE `tags` (
 `id` int(11) unsigned NOT NULL auto_increment,
 `name` varchar(255) NOT NULL default '',
 `description` text NOT NULL,
 `addontype_id` int(11) unsigned NOT NULL default '0',
 `application_id` int(11) unsigned NOT NULL default '0',
 `created` datetime NOT NULL default '0000-00-00 00:00:00',
 `modified` datetime NOT NULL default '0000-00-00 00:00:00',
 PRIMARY KEY  (`id`),
 KEY `addontype_id` (`addontype_id`),
 KEY `application_id` (`application_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- Dumping data for table `tags`
--


-- --------------------------------------------------------

--
-- Table structure for table `translations`
--

DROP TABLE IF EXISTS `translations`;
CREATE TABLE `translations` (
 `id` varchar(255) NOT NULL default '',
 `page_id` varchar(255) NOT NULL default '',
 `en-US` text,
 `en-GB` text,
 `fr` text,
 `de` text,
 `ja` text,
 `pl` text,
 `es-ES` text,
 `zh-CN` text,
 `zh-TW` text,
 `cs` text,
 `da` text,
 `nl` text,
 `fi` text,
 `hu` text,
 `it` text,
 `ko` text,
 `pt-BR` text,
 `ru` text,
 `es-AR` text,
 `sv-SE` text,
 `tr` text,
 PRIMARY KEY  (`id`,`page_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Dumping data for table `translations`
--


-- --------------------------------------------------------

--
-- Table structure for table `users`
--

DROP TABLE IF EXISTS `users`;
CREATE TABLE `users` (
 `id` int(11) unsigned NOT NULL auto_increment,
 `username` varchar(255) NOT NULL default '',
 `firstname` varchar(255) NOT NULL default '',
 `lastname` varchar(255) NOT NULL default '',
 `email` varchar(255) NOT NULL default '',
 `emailhidden` tinyint(1) unsigned NOT NULL default '0',
 `homepage` varchar(255) NOT NULL default '',
 `confirmationcode` varchar(255) NOT NULL default '',
 `created` datetime NOT NULL default '0000-00-00 00:00:00',
 `modified` datetime NOT NULL default '0000-00-00 00:00:00',
 `notes` text NOT NULL,
 PRIMARY KEY  (`id`),
 UNIQUE KEY `email` (`email`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- Dumping data for table `users`
--


-- --------------------------------------------------------

--
-- Table structure for table `versions`
--

DROP TABLE IF EXISTS `versions`;
CREATE TABLE `versions` (
 `id` int(11) unsigned NOT NULL auto_increment,
 `addon_id` int(11) unsigned NOT NULL default '0',
 `version` varchar(255) NOT NULL default '',
 `dateaded` datetime NOT NULL default '0000-00-00 00:00:00',
 `dateapproved` datetime NOT NULL default '0000-00-00 00:00:00',
 `dateupdated` datetime NOT NULL default '0000-00-00 00:00:00',
 `approved` tinyint(1) unsigned NOT NULL default '0',
 `releasenotes` text NOT NULL,
 `created` datetime NOT NULL default '0000-00-00 00:00:00',
 `modifid` datetime NOT NULL default '0000-00-00 00:00:00',
 PRIMARY KEY  (`id`),
 KEY `addon_id` (`addon_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- Dumping data for table `versions`
--


--
-- Constraints for dumped tables
--

--
-- Constraints for table `addons`
--
ALTER TABLE `addons`
 ADD CONSTRAINT `addons_ibfk_1` FOREIGN KEY (`addontype_id`) REFERENCES `addontypes` (`id`);

--
-- Constraints for table `addons_tags`
--
ALTER TABLE `addons_tags`
 ADD CONSTRAINT `addons_tags_ibfk_4` FOREIGN KEY (`tag_id`) REFERENCES `tags` (`id`),
 ADD CONSTRAINT `addons_tags_ibfk_3` FOREIGN KEY (`addon_id`) REFERENCES `addons` (`id`);

--
-- Constraints for table `addons_users`
--
ALTER TABLE `addons_users`
 ADD CONSTRAINT `addons_users_ibfk_2` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`),
 ADD CONSTRAINT `addons_users_ibfk_1` FOREIGN KEY (`addon_id`) REFERENCES `addons` (`id`);

--
-- Constraints for table `applications_versions`
--
ALTER TABLE `applications_versions`
 ADD CONSTRAINT `applications_versions_ibfk_2` FOREIGN KEY (`version_id`) REFERENCES `versions` (`id`),
 ADD CONSTRAINT `applications_versions_ibfk_1` FOREIGN KEY (`application_id`) REFERENCES `applications` (`id`);

--
-- Constraints for table `approvals`
--
ALTER TABLE `approvals`
 ADD CONSTRAINT `approvals_ibfk_2` FOREIGN KEY (`file_id`) REFERENCES `files` (`id`),
 ADD CONSTRAINT `approvals_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`);

--
-- Constraints for table `appversions`
--
ALTER TABLE `appversions`
 ADD CONSTRAINT `appversions_ibfk_1` FOREIGN KEY (`application_id`) REFERENCES `applications` (`id`);

--
-- Constraints for table `blapps`
--
ALTER TABLE `blapps`
 ADD CONSTRAINT `blapps_ibfk_1` FOREIGN KEY (`blitem_id`) REFERENCES `blitems` (`id`);

--
-- Constraints for table `downloads`
--
ALTER TABLE `downloads`
 ADD CONSTRAINT `downloads_ibfk_1` FOREIGN KEY (`file_id`) REFERENCES `files` (`id`);

--
-- Constraints for table `features`
--
ALTER TABLE `features`
 ADD CONSTRAINT `features_ibfk_1` FOREIGN KEY (`addon_id`) REFERENCES `addons` (`id`);

--
-- Constraints for table `files`
--
ALTER TABLE `files`
 ADD CONSTRAINT `files_ibfk_2` FOREIGN KEY (`platform_id`) REFERENCES `platforms` (`id`),
 ADD CONSTRAINT `files_ibfk_1` FOREIGN KEY (`version_id`) REFERENCES `versions` (`id`);

--
-- Constraints for table `previews`
--
ALTER TABLE `previews`
 ADD CONSTRAINT `previews_ibfk_1` FOREIGN KEY (`addon_id`) REFERENCES `addons` (`id`);

--
-- Constraints for table `reviewratings`
--
ALTER TABLE `reviewratings`
 ADD CONSTRAINT `reviewratings_ibfk_2` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`),
 ADD CONSTRAINT `reviewratings_ibfk_1` FOREIGN KEY (`review_id`) REFERENCES `reviews` (`id`);

--
-- Constraints for table `reviews`
--
ALTER TABLE `reviews`
 ADD CONSTRAINT `reviews_ibfk_1` FOREIGN KEY (`version_id`) REFERENCES `versions` (`id`);

--
-- Constraints for table `session_data`
--
ALTER TABLE `session_data`
 ADD CONSTRAINT `session_data_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`);
