#!/usr/bonsaitools/bin/perl -w
# -*- Mode: perl; indent-tabs-mode: nil -*-
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
# The Original Code is the Despot Account Administration System.
#
# The Initial Developer of the Original Code is Netscape Communications
# Corporation. Portions created by Netscape are
# Copyright (C) 1998 Netscape Communications Corporation. All
# Rights Reserved.
#
# Contributor(s): Terry Weissman <terry@mozilla.org>
#                 Dan Mosedale <dmose@mozilla.org>

# $F::debug = 1;

#$cvs = "/opt/cvs-tools/bin/cvs";
#$whoami = "/usr/ucb/whoami";
#$perlbin = "/tools/ns/bin/perl5.004";

$cvs = "/usr/bin/cvs";
$whoami = "/usr/bin/whoami";
$perlbin = "/usr/bin/perl";

#$curdir = `pwd`;
#chop($curdir);
#$ENV{"CVS_PASSFILE"} = "$curdir/.cvspass";
$ENV{"CVS_RSH"} = "ssh";    
$ENV{"CVS_PASSFILE"} = "/usr/local/etc/despot-cvspass";

$dontcommit = 0;
$usertoblame = "";

for ($i=0 ; $i<@ARGV ; $i++) {
    if ($ARGV[$i] eq "-n") {
        $dontcommit = 1;
    }
    if ($ARGV[$i] eq "-user") {
        $usertoblame = $ARGV[++$i];
    }
}

$srcdir = $0;
$srcdir =~ s:/[^/]*$::;      # Remove last word, and slash before it.
if ($srcdir eq "") {
    $srcdir = ".";
}
chdir $srcdir || die "Couldn't chdir to $srcdir";

use Mysql;
require 'utils.pl';

$db = Mysql->Connect("localhost", "mozusers", "despot")
    || die "Can't connect to database server";

$db = $db;                      # Make -w shut up.

($mylogin = `$whoami`); 
chop($mylogin);

$hostname = 'unknown';
if (open(HOST, "/bin/hostname|")) {
    $hostname = <HOST>;
    chop($hostname);
    close(HOST);
}


$ENV{"HOME"} = glob("~$mylogin"); # CVS wants this.

if ($usertoblame eq "") {
    $usertoblame = $mylogin;
}

$boilerplate = "";

open(BOILERPLATE, "<commitcheck.templ") || die "Can't open template file";
while (<BOILERPLATE>) {
    if ( /^#/ ) {
        # Strip out comments from the boilerplate.  Might as well; the
        # faster our generated perl script runs, the better.
        next;
    }
    $boilerplate .= $_;
}
close BOILERPLATE;


open(BOILERPLATE, "<owners.templ") || die "Can't open template file";
while (<BOILERPLATE>) {
    push @ownersplate, $_;
}
close BOILERPLATE;




my @grouplist = ('cvs', 'gila'); # Mailing group list.  Should really query
                                 # for this list rom the db ... ###

$repquery = Query("select id,name,cvsroot,ownersrepository,ownerspath,domailing from repositories order by name");
while (@reprow = $repquery->fetchrow()) {
    ($repid,$repname,$reproot,$ownersrepository,$ownerspath,$domailing) =
        (@reprow);

    $query = Query("select email,passwd,${repname}_group,neednewpassword,disabled from users where ${repname}_group != 'None' and passwd != '' order by email");

    $tmpdir = "/tmp/syncit-$repname.$$";
    mkdir $tmpdir, 0777;
    chdir $tmpdir;
    
    $ENV{CVSROOT} = $reproot;

    my @filelist = ('passwd', 'commitcheck.pl');
    if ($domailing) {
        foreach my $g (@grouplist) {
            push @filelist, "${g}_list";
        }
    }
    system "$cvs co " . join(' ', grep(s@^@CVSROOT/@, @filelist))
        || die "Couldn't checkout files.";


    if ($domailing) {
        foreach my $group (@grouplist) {
            $outfile = "CVSROOT/${group}_list";
            open(MAILLIST, ">$outfile") || die "Can't open $outfile";
            
            print MAILLIST "# DO NOT EDIT THIS FILE!  This file is created\n";
            print MAILLIST "# by despot, and is used by /etc/aliases.\n";

            my $q = Query("select email from users where ${group}_group != 'None' and passwd != '' and disabled != 'Yes' order by email");
            while (@r = $q->fetchrow()) {
                print MAILLIST $r[0]. "\n";
            }
            close MAILLIST;
        }
    }



    $outfile = "CVSROOT/passwd";
    open(PASSWD, ">$outfile") || die "Can't open $outfile: $!";
    
    print PASSWD "# DO NOT EDIT THIS FILE!  You must instead go to https://despot.mozilla.org/, and\n";
    print PASSWD "# tweak things from there.\n";
    
    while (@row = $query->fetchrow()) {
        ($email,$password,$group,$neednew,$disabled) = @row;
        if ($neednew eq "Yes" || $disabled eq "Yes") {
            next;
        }
        $login = $email;
        $login =~ s/@/%/g;
        print PASSWD "$login:$password:$group\n";
    }
    close PASSWD;
    
#    system "$cvs co CVSROOT/commitcheck.pl" || die "Couldn't checkout passwd file.";
    
    $outfile = "CVSROOT/commitcheck.pl";
    open(COMMITCHECK, ">$outfile") || die "Can't open $outfile";

    print COMMITCHECK "#!$perlbin --\n";
    print COMMITCHECK "# DO NOT EDIT THIS FILE!  You must instead go to https://despot.mozilla.org/, and\n";
    print COMMITCHECK "# tweak things from there.\n\n";

    $query = Query("select partitions.id,partitions.name,state,branches.name from partitions,branches where repositoryid = '$repid' and branches.id=branchid order by partitions.name");
    $founddefault = 0;
    while (@row = $query->fetchrow()) {
        ($id,$name,$state,$branch) = (@row);
        $d = "\$";
        print COMMITCHECK $d . "mode{'$id'} = '$state';\n";
        print COMMITCHECK $d . "branch{'$id'} = '$branch';\n";
        print COMMITCHECK $d . "fullname{'$id'} = '$name';\n";
        if ($name eq 'default') {
            print COMMITCHECK $d . "defaultid = '$id';\n";
            $founddefault = 1;
        }
        if ($state ne "Open") {
            foreach $n ("blessed", "super") {
                print COMMITCHECK $d . "$n" . "{'$id'} = [";
                $eq = "=";
                if ($n eq "super") {
                    $eq = "!=";
                }
                $q2 = Query("select email from members,users where partitionid = $id and class $eq 'Member' and users.id = userid");
                while (@r2 = $q2->fetchrow()) {
                    my $n = $r2[0];
                    $n =~ s/@/%/;
                    print COMMITCHECK "'$n',";
                }
                print COMMITCHECK "];\n";
            }
        }
    }
    if (!$founddefault) {
        print COMMITCHECK $d . "defaultid = 'none';\n";
    }
    print COMMITCHECK "sub GetT {\n";
    print COMMITCHECK '($b,$_) = (@_);' . "\n";
    $query = Query("select branches.name,partitions.id from partitions,branches where repositoryid = '$repid' and branches.id = branchid order by branches.name");
    $lastbranch = "";
    while (@row = $query->fetchrow()) {
        ($branchname,$partid) = (@row);
        if ($branchname ne $lastbranch) {
            if ($lastbranch ne "") {
                print COMMITCHECK "}\n";
            }
            print COMMITCHECK "if (" . $d . "b eq '$branchname') {\n";
            $lastbranch = $branchname;
        }
        $q2 = Query("select pattern from files where partitionid=$partid order by pattern");
        while (@r2 = $q2->fetchrow()) {
            my $regexp = $r2[0];
            $regexp =~ s/\./\\./g;
            $regexp =~ s:\*$:.*:;
            $regexp =~ s:\%$:[^/]*:;
            $regexp = '^' . $regexp . "\$";
            print COMMITCHECK "if (m:$regexp:) {return '$partid';}\n";
        }
    }
    if ($lastbranch ne "") {
        print COMMITCHECK "}\n";
    }
    print COMMITCHECK "return '';\n";
    print COMMITCHECK "}\n";

    print COMMITCHECK $boilerplate;

    close COMMITCHECK;
    
    chdir "CVSROOT";
    if ($dontcommit) {
        system "$cvs diff -c passwd";
        system "$cvs diff -c commitcheck.pl";
        # system "$cvs -q commit -m 'Pseudo-automatic update of changes made by $usertoblame.' commitcheck.pl";
    } else {
        system ("$cvs -q commit -m 'Pseudo-automatic update of changes" .
                " made by $usertoblame.'") == 0
          or die "cvs commit failed";
    }
    

    if (defined $ownersrepository && $ownersrepository > 0 &&
        defined $ownerspath && $ownerspath ne "") {
        $query = Query("select cvsroot from repositories where id = $ownersrepository");
        $ENV{CVSROOT} = ($query->fetchrow())[0];
        $tdir = "$tmpdir/ownerstuff";
        mkdir $tdir, 0777;
        chdir $tdir;
        system "$cvs co $ownerspath" || die "Couldn't checkout $ownerspath";

        open(OWNERS, ">$ownerspath") || die "Can't open $ownerspath";
        print OWNERS "<!-- THIS FILE IS AUTOMATICALLY GENERATED; DO NOT EDIT. -->\n";

        foreach (@ownersplate) {
            if ($_ !~ m/^%%DATA%%/) {
                print OWNERS $_;
                next;
            }
            $query = Query("select id,name,description,newsgroups,doclinks from partitions where repositoryid = '$repid' order by name");
            while (@row = $query->fetchrow()) {
                ($id,$name,$desc,$newsgroups,$doclinks) = (@row);
                if ($name eq "default" || $name eq "despotaccess") {
                    next;
                }
                my $fullname = $name;
                if (defined $desc && $desc ne "") {
                    $fullname .= " ($desc)";
                }
                my $anchor_name = name_to_id_token($name);
                $q2 = Query("select class,email,realname from members,users where partitionid = $id and class != 'Member' and users.id = userid");
                my @owners;
                my @ownernames;
                my @peers;
                my @peernames;
                while (@r2 = $q2->fetchrow()) {
                    if ($r2[0] eq "Owner") {
                        push @owners, $r2[1];
                        push @ownernames, $r2[2];
                    } else {
                        push @peers, $r2[1];
                        push @peernames, $r2[2];
                    }
                }

                my $maillist = "mailto:" . join(',', @owners);
                if (@peers > 0) {
                    $maillist .= "?cc=" . join(',', @peers);
                }
                $maillist =~ s/\@/&#x40;/g;

                print OWNERS qq|    <a name="$anchor_name"></a>
    <table class="data" style="width:100%">
    <thead>
      <tr>
        <th style="width:15%; text-align:right; vertical-align:top">
          Module:
        </th>
        <th style="text-align:left">
          $fullname
        </th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <th>Owner:</th>
        <td><a href="$maillist">| . join(', ', @ownernames) . qq|</a></td>
      </tr>
      <tr>
        <th>Source:</th>
        <td>|;
                $q2 = Query("select pattern from files where partitionid=$id order by pattern");
                my @filelist;
                while (@r2 = $q2->fetchrow()) {
                    my $name = $r2[0];
                    $name =~ s/\*$//;
                    $name =~ s/%$//;
                    $name =~ s:/$::;
                    $name =~ s:^mozilla/::;
                    push @filelist, qq|<a href="http://lxr.mozilla.org/mozilla/source/$name">$name</a>|;
                }
                print OWNERS join(', ', @filelist);
                print OWNERS qq|</td>
      </tr>
      <tr>
        <th>Newsgroup:</th>
        <td>|;
                my @grouplist;
                if (!defined $newsgroups) {
                    $newsgroups = "";
                }
                foreach $i (split(/[,\s]+/, $newsgroups)) {
                    my $base = "news:";
                    if ($i =~ /^netscape\.public/) {
                        $base = "nntp://news.mozilla.org/";
                    }
                    push @grouplist, qq|<a href="$base$i">$i</a>|;
                }
                print OWNERS join(', ', @grouplist);
                print OWNERS qq|</td>
      </tr>
      <tr>
        <th>Peers:</th>
        <td>|;
                my @peerlist;
                foreach $i (@peers) {
                    $i =~ s/\@/&#x40;/g;
                    push @peerlist, qq|<a href="mailto:$i">| . shift(@peernames) . "</a>";
                }
                print OWNERS join(', ', @peerlist);
                print OWNERS qq|</td>
      </tr>
      <tr>
        <th>Documents:</th>
        <td>|;
                my @doclist;
                if (!defined $doclinks) {
                    $doclinks = "";
                }
                foreach $i (split(/[,\s]+/, $doclinks)) {
                    push @doclist, qq|<a href="$i">$i</a>|;
                }
                print OWNERS join(', ', @doclist);
                print OWNERS qq|</td>
      </tr>
    </tbody>
    </table>

|;
            }
        }

        close OWNERS;

        if ($dontcommit) {
            system "$cvs diff -c";
        } else {
            system ( "$cvs -q commit -m 'Pseudo-automatic update of changes" . 
                     " made by $usertoblame.'" ) == 0
              or die "cvs commit failed";
        }
    }

    chdir "/";
    
    system "rm -rf $tmpdir";
    
    
}


Query("delete from syncneeded");
    
