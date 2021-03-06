#!/bin/sh
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
# The Original Code is The Waterfall Java Plugin Module
# 
# The Initial Developer of the Original Code is Sun Microsystems Inc
# Portions created by Sun Microsystems Inc are Copyright (C) 2001
# All Rights Reserved.
#
# $Id: set_env.sh,v 1.4 2001/07/18 20:49:58 edburns%acm.org Exp $
#
# 
# Contributor(s): 
#
#   Nikolay N. Igotti <inn@sparc.spb.su>

# Set those variables
# for me this var set in .bashrc so I don't need it here
#WFJDKHOME=/usr/java/jdk1.3.0_02
WFDIR=`cd ../wf; pwd;`
# end of customizable part

CURDIR=`pwd`
cd ${WFDIR}/build/unix
source ./set_paths
cd ${CURDIR}
export WFDIR WFJDKHOME
