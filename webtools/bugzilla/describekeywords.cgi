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
# The Initial Developer of the Original Code is Terry Weissman.
# Portions created by Terry Weissman are
# Copyright (C) 2000 Terry Weissman. All
# Rights Reserved.
#
# Contributor(s): Terry Weissman <terry@mozilla.org>
# Contributor(s): Gervase Markham <gerv@gerv.net>

use diagnostics;
use strict;
use lib ".";

require "CGI.pl";

# Use the global template variables. 
use vars qw($vars $template);

ConnectToDatabase();

quietly_check_login();

SendSQL("SELECT keyworddefs.name, keyworddefs.description, 
                COUNT(keywords.bug_id)
         FROM keyworddefs LEFT JOIN keywords ON keyworddefs.id=keywords.keywordid
         GROUP BY keyworddefs.id
         ORDER BY keyworddefs.name");

my @keywords;

while (MoreSQLData()) {
    my ($name, $description, $bugs) = FetchSQLData();
   
    push (@keywords, { name => $name, 
                       description => $description,
                       bugcount => $bugs });
}
   
$vars->{'keywords'} = \@keywords;
$vars->{'caneditkeywords'} = UserInGroup("editkeywords");

print "Content-type: text/html\n\n";
$template->process("reports/keywords.html.tmpl", $vars)
  || ThrowTemplateError($template->error());
