#!/bin/sh
#
# The contents of this file are subject to the Mozilla Public License
# Version 1.0 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
# 
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
# License for the specific language governing rights and limitations
# under the License.
# 
# The Original Code is the Bugzilla Bug Tracking System.
# 
# The Initial Developer of the Original Code is Netscape Communications
# Corporation. Portions created by Netscape are Copyright (C) 1998
# Netscape Communications Corporation. All Rights Reserved.
# 
# Contributor(s): Terry Weissman <terry@mozilla.org>

mysql > /dev/null 2>/dev/null << OK_ALL_DONE

use bugs;

drop table products;
OK_ALL_DONE

mysql << OK_ALL_DONE
use bugs;
create table products (
product tinytext,
description tinytext
);


insert into products (product, description) values ("Bugzilla", "Use this to describe a problem you're having with the bug system itself");
insert into products (product, description) values ("Mozilla", "For bugs about the Mozilla web browser");
insert into products (product, description) values ("NGLayout", 'For bugs about the <a href="http://www.mozilla.org/newlayout/">New Layout</a> project');
