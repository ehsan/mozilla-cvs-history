# -*- Mode: perl; indent-tabs-mode: nil -*-

# VCDisplay::Bonsai - This is the implemenation to use if you have
# installed bonsai and are using cvsblame cvsguess and cvsquery to let
# your webserver render html pages of your CVS repository.

# $Revision: 1.6 $ 
# $Date: 2001/03/26 14:04:44 $ 
# $Author: kestes%tradinglinx.com $ 
# $Source: /cvs/cvsroot/mozilla/webtools/tinderbox2/src/lib/VCDisplay/Bonsai.pm,v $ 
# $Name:  $ 



# The contents of this file are subject to the Mozilla Public
# License Version 1.1 (the "License"); you may not use this file
# except in compliance with the License. You may obtain a copy of
# the License at http://www.mozilla.org/NPL/
#
# Software distributed under the License is distributed on an "AS
# IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
# implied. See the License for the specific language governing
# rights and limitations under the License.
#
# The Original Code is the Tinderbox build tool.
#
# The Initial Developer of the Original Code is Netscape Communications
# Corporation. Portions created by Netscape are
# Copyright (C) 1998 Netscape Communications Corporation. All
# Rights Reserved.
#

# complete rewrite by Ken Estes:
#	 kestes@staff.mail.com Old work.
#	 kestes@tradinglinx.com New work.
#	 kestes@walrus.com Home.
# Contributor(s): 





package VCDisplay;


# Load standard perl libraries


# Load Tinderbox libraries

use lib '#tinder_libdir#';

use HTMLPopUp;
use TreeData;

# Remember the HTMLPopUp.pm function Link accepts these arguments:

# Link(
#      "statuslinetxt"=>"", 
#      "windowtxt"=>"", 
#      "linktxt"=>"", 
#      "name"=>"", 
#      "href"=>"",
#     );

# We will pass these values through to Link (except for href which is
# created using additional arguments as needed by the specific
# function).

$VERSION = '#tinder_version#';



# this is how we create a URL to the various CGI programs.

# If you your using VCDisplay:Bonsai we need to know how to make HMTL
# to point o the bonsai CGI programs.

$BONSAI_URL = ( $TinderConfig::BONSAI_URL || 
                "../bonsai");

$CVSQUERY = $BONSAI_URL."/cvsquery.cgi";
$CVSBLAME = $BONSAI_URL."/cvsblame.cgi";
$CVSGUESS = $BONSAI_URL."/cvsguess.cgi";

# Bonsai wants the CVS root to be a local root.  i.e. use '/cvsroot'
# not ':pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot'.  Sometimes
# it is convienent to keep the remote server information in the
# tinderbox config so that we can run BOTH the CVS and Bosai TinderDB
# modules at the same time.

# return the local cvsroot portion of the cvsroot.

sub remote_cvsroot2local_cvsroot {
  my ($root) = @_;
  
  # remove everything up till the last colon.
  $root =~ s!^.*\:!!g;

  return $root;
}


# convert time() format to human readable bonsai format
sub time2bonsai {
  my ($time) = @_;

  my ($sec,$min,$hour,$mday,$mon,
      $year,$wday,$yday,$isdst) =
        localtime($time);
  $mon++;
  $year += 1900;
  my ($bonsai_date_str) = sprintf("%02u/%02u/%04u %02u:%02u:%02u",
                                  $mon, $mday, $year, $hour, $min, $sec);


  $bonsai_date_str = HTMLPopUp::escapeURL($bonsai_date_str);

  return $bonsai_date_str;
}


# create a Link to a VC file and its line number

sub source {
  my (%args) = @_;

  if ($DEBUG) {
    ($args{'tree'} && $args{'file'} && $args{'line'} && $args{'linktxt'}) ||
      die("function VCDisplay::source, not called with enough arguments ");
    
    (TreeData::tree_exists($args{'tree'})) ||
      die("function VCDisplay::source, tree: $args{'tree'} does not exist\n");
  }

  my ($goto_line) = ($line > 10 ? $line - 10 : 1 );
  
  my @url_args = ();
  my %tree = %{$TreeData::VC_TREE{$args{'tree'}}};

  { 

    # bonsai has certain conventions for what we would normally
    # consider empty variables.

    my ($tree) = ($args{'tree'} ||
                  'default');  

    my ($module) = ($tree{'module'} ||
                    'all');

    my ($root) = $tree{'root'}; 
    $root = remote_cvsroot2local_cvsroot($root);

    my ($branch) =$tree{'branch'};
    
    push @url_args, (
                     "treeid=".HTMLPopUp::escapeURL($tree),
                     "cvsroot=".HTMLPopUp::escapeURL($root),
                     "module=".HTMLPopUp::escapeURL($module),
                     "branch=".HTMLPopUp::escapeURL($branch),
                     
                     "rev=".HTMLPopUp::escapeURL($tree{'branch'}),
                     "file=".HTMLPopUp::escapeURL($args{'file'}),
                     "mark=".HTMLPopUp::escapeURL($args{'line'}."\#$goto_line"),
                    );
  }

  $args{'href'} = ("$CVSBLAME?".join('&', @url_args));

  my $output = HTMLPopUp::Link(%args);
  
  return $output;
}


# Create a Link to a VC file and its line number.

# This function is used when only the basename of the file is provided
# and some additional work must be done to figure out the file which
# was meant.  Most compilers only give the basename in their error
# messages and leave the determination of the directory to the user
# who is looking at the error log.

sub guess {
  my (%args) = @_;

  if ($DEBUG) {
    ($args{'tree'} && $args{'file'} && $args{'line'} && $args{'linktxt'}) ||
      die("function VCDisplay::guess, not called with enough arguments ");
    
    (TreeData::tree_exists($args{'tree'})) ||
      die("function VCDisplay::guess, tree: $args{'tree'} does not exist\n");
  }

  my ($goto_line) = ($error_line > 10 ? $error_line : 1 );
  
  my @url_args = ();
  my %tree = %{$TreeData::VC_TREE{$args{'tree'}}};


  { 

    # bonsai has certain conventions for what we would normally
    # consider empty variables.

    my ($tree) = ($args{'tree'} ||
                  'default');  

    my ($module) = ($tree{'module'} ||
                    'all');

    my ($root) = $tree{'root'}; 
    $root = remote_cvsroot2local_cvsroot($root);

    my ($branch) =$tree{'branch'};
    
    push @url_args, (
                     "treeid=".HTMLPopUp::escapeURL($tree),
                     "cvsroot=".HTMLPopUp::escapeURL($root),
                     "module=".HTMLPopUp::escapeURL($module),
                     "branch=".HTMLPopUp::escapeURL($branch),
                     
                     "rev=".HTMLPopUp::escapeURL($tree{'branch'}),
                     "file=".HTMLPopUp::escapeURL($args{'file'}),
                     "mark=".HTMLPopUp::escapeURL($args{'line'}."\#$goto_line"),
                    );
  }

  $args{'href'} = ("$CVSGUESS?".join('&', @url_args));

  my $output = HTMLPopUp::Link(%args);
  
  return $output;
}


# Query VC about checkins by a user or during a particular time or
# with a particular file.


sub query {
  my (%args) = @_;

  if ($DEBUG) {
    ($args{'tree'} && $args{'linktxt'}) ||
      die("function VCDisplay::query, not called with enough arguments ");
    
    ($args{'mindate'} || $args{'who'}) || 
      die("function VCDisplay::query, not called with enough arguments ");
    
    (TreeData::tree_exists($args{'tree'})) ||
      die("function VCDisplay::query, tree: $args{'tree'} does not exist\n");
  }

  my @url_args = ();
  my %tree = %{$TreeData::VC_TREE{$args{'tree'}}};

  { 

    # bonsai has certain conventions for what we would normally
    # consider empty variables.

    my ($tree) = ($args{'tree'} ||
                  'default');  

    my ($module) = ($tree{'module'} ||
                    'all');

    my ($root) = $tree{'root'}; 
    $root = remote_cvsroot2local_cvsroot($root);

    my ($branch) =$tree{'branch'};
    
    push @url_args, (
                     "treeid=".HTMLPopUp::escapeURL($tree),
                     "cvsroot=".HTMLPopUp::escapeURL($root),
                     "module=".HTMLPopUp::escapeURL($module),
                     "branch=".HTMLPopUp::escapeURL($branch),
                    );
  }

  ($args{'who'}) &&
    (push @url_args, "who=".HTMLPopUp::escapeURL($args{'who'}) );

  ($args{'mindate'} || $args{'maxdate'}) && 
    (push @url_args, "date=explicit" );

  # Convert times to a human readable date for the convience of the
  # users.  Bonsai excepts data of form: 'mm/dd/yyyy hh:mm:ss' in
  # addition to time format.

  ($args{'mindate'}) &&
    (push @url_args, "mindate=".time2bonsai($args{'mindate'}) );

  ($args{'maxdate'}) && 
    (push @url_args, "maxdate=".time2bonsai($args{'maxdate'}) );

  $args{'href'} = ("$CVSQUERY?".join('&', @url_args));

  my ($output) = HTMLPopUp::Link(%args);

  return $output;
}


1;
