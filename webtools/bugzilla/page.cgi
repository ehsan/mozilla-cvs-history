#!/usr/bonsaitools/bin/perl -wT
# -*- Mode: perl; indent-tabs-mode: nil -*-
#
# The contents of this file are subject to the Mozilla Public
# License Version 1.1 (the "License"); you may not use this file
# except in compliance with the License. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS
# IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
# implied. See the License for the specific language governing
# rights and limitations under the License.
#
# The Original Code is the Bugzilla Bug Tracking System.
#
# The Initial Developer of the Original Code is Netscape Communications
# Corporation. Portions created by Netscape are
# Copyright (C) 1998 Netscape Communications Corporation. All
# Rights Reserved.
#
# Contributor(s): Gervase Markham <gerv@gerv.net>
#

###############################################################################
# This CGI is a general template display engine. To display templates using it,
# add them to the %pages hash in localconfig with a tag to refer to them by,
# then call page.cgi?page=<tag> . Tags may only contain the letters A-Z (in
# either case), numbers 0-9, the underscore "_" and the hyphen "-".
###############################################################################

use strict;

use lib ".";
require "CGI.pl";

use vars qw($template $vars %pages);

ConnectToDatabase();

quietly_check_login();

print "Content-Type: text/html\n\n";

if (defined $::FORM{'id'}) {
    $::FORM{'id'} =~ s/[^\w-]//g;

    if ($pages{$::FORM{'id'}}) {
        $template->process($pages{$::FORM{'id'}}, $vars)
          || ThrowTemplateError($template->error());
        exit;
    }
}

$vars->{'message'} = "page_not_found";

$template->process("global/message.html.tmpl", $vars)
  || ThrowTemplateError($template->error());
