#!/usr/bonsaitools/bin/perl --
# -*- Mode: perl; indent-tabs-mode: nil -*-
#
# The contents of this file are subject to the Netscape Public License
# Version 1.0 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://www.mozilla.org/NPL/
#
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
# License for the specific language governing rights and limitations
# under the License.
#
# The Original Code is the Tinderbox build tool.
#
# The Initial Developer of the Original Code is Netscape Communications
# Corporation. Portions created by Netscape are Copyright (C) 1998
# Netscape Communications Corporation. All Rights Reserved.

use lib "../bonsai";

require 'globals.pl';
require 'lloydcgi.pl';
require 'imagelog.pl';
require 'header.pl';
$|=1;

print "Content-type: text/html\n\n<HTML>\n";


#
# show 36 hours by default
#
if($form{'showall'} != 0 ){
    $mindate = 0;
}
else {
    $hours = 36;
    if( $form{hours} ne "" ){
        $hours = $form{hours};
    }
    $mindate = time - ($hours*60*60);
}

$colormap = {
                success => '00ff00',
                busted => 'red',
                building => 'yellow',
		testfailed => 'orange'
            };

#
# Debug hack 
#
#$form{'tree'} = DogbertTip;
$tree = $form{'tree'};

if( $form{'tree'} eq '' ){
    &show_tree_selector;
}
else {
    if( $form{'express'} ) {
        &do_express;
    }
    else {
        &load_data;
        &load_javascript;
        &display_page_head;
        &display_build_table;
    }
}


sub show_tree_selector {

    EmitHtmlHeader("tinderbox");

    print "<P><TABLE WIDTH=\"100%\">";
    print "<TR><TD ALIGN=CENTER>Select one of the following trees:</TD></TR>";
    print "<TR><TD ALIGN=CENTER>\n";
    print " <TABLE><TR><TD><UL>\n";

    while(<*>) {
        if( -d $_ && $_ ne 'data' && $_ ne 'CVS' ){
            print "<LI><a href=showbuilds.cgi?tree=$_>$_</a>\n";
        }
    }
    print "<//UL></TD></TR></TABLE></TD></TR></TABLE>";
}



sub display_page_head {

    my ($imageurl,$imagewidth,$imageheight,$quote) = &get_image;


# srand;
# $when = 60*10 + int rand(60*40);
# <META HTTP-EQUIV=\"REFRESH\" CONTENT=\"$when\">

    
if( -r "$tree/mod.pl" ){
    require "$tree/mod.pl";
}
else {
    $message_of_day = "";
}

$treename = $tree . ($tree2 ne "" ? " and $tree2" : "" );

    EmitHtmlTitleAndHeader("tinderbox: $treename", "tinderbox",
                           "tree: $treename");


    print "$script_str\n";
    print "$message_of_day\n";

    print "<table width='100%'>";
    print "<tr>";
    print "<td valign=bottom>";
    print "<p><center><a href=addimage.cgi><img src='$imageurl' ";
    print "width=$imagewidth height=$imageheight><br>";
    print "$quote</a><br>";
    print "</center>";
    print "<p>";
    print "<td align=right valign=bottom>";
    print "<table><tr><td>";
    print "<TT>L</TT> = Show Build Log<br>";
    print "<TT><img src=star.gif>L</TT> = Show Log comments<br>";
    print "<TT>C</TT> = Show changes that occured since the last build<br>";
    print "<TT>B</TT> = Download binary generated by the build<br>";
    print "<table cellspacing=2 border>";
    print "<tr bgcolor=00ff00><td>Built successfully";
    print "<tr bgcolor=yellow><td>Currently Building";
    print "<tr bgcolor=orange><td>Build Successful, Build Tests Failed";
    print "<tr bgcolor=red><td>Build failed";
    print "</table>";
    print "</td></tr></table>";
    print "</table>";

    if($bonsai_tree){
        print "<p>The tree is currently <font size=+2>";
        if( &tree_open ){
            print "OPEN";
        }
        else {
            print "<FONT COLOR=\"#FF0000\">CLOSED</FONT>";
        }
        print "</font>\n";
    }
}

sub display_build_table {
    &display_build_table_header;
    &display_build_table_body;
    &display_build_table_footer;

}

sub display_build_table_body {
    local($t);

    $t = 1;
    while( $t <= $time_count ){
        display_build_table_row( $t );    
        $t++; 
    }
}

sub display_build_table_row {
    local($t) = @_;
    local($tt);
    $tt = &print_time($build_time_times->[$t]);

    if( $tree2 ne "" ){
        $qr = "";
        $er = "";
    }
    else {
        $qr = &query_ref( $td1, $build_time_times->[$t]);
        $er = "</a>";
    }

    if ($build_time_times->[$t] % 7200 > 3600) {
        $color = "white";
    } else {
        $color = "beige";
    }

    print "<tr align=center>\n";
    print "<td bgcolor=$color>${qr}\n${tt}${er}\n";


    if( $tree2 ne "" ){
        print "<td align=center bgcolor=beige>\n";
        $qr = &query_ref( $td1, $build_time_times->[$t]);
        print "${qr}<tt><b>X</b></tt></a>\n";
    }

    print "<td align=center>\n";

    for $who (sort keys %{$who_list->[$t]} ){
        #$qr = &query_ref( $td1, $build_time_times->[$t],$build_time_times->[$t-1],$who);
        $qr = &who_menu( $td1, $build_time_times->[$t],$build_time_times->[$t-1],$who);
        print "  ${qr}$who</a>\n";
    }

    if( $tree2 ne "" ){
        print "<td align=cenger bgcolor=beige >\n";
        $qr = &query_ref( $td2, $build_time_times->[$t]);
        print "${qr}<tt><b>X</b></tt></a>\n";

        print "<td align=center>\n";

        for $who (sort keys %{$who_list2->[$t]} ){
            #$qr = &query_ref( $td2, $build_time_times->[$t],$build_time_times->[$t-1],$who);
            $qr = &who_menu( $td2, $build_time_times->[$t],$build_time_times->[$t-1],$who);
            print "  ${qr}$who</a>\n";
        }
    }
    
    $bn = 1;
    while( $bn <= $name_count ){
        if( defined($br = $build_table->[$t][$bn])){
            if( $br != -1 ){

                $hasnote = $br->{hasnote};
                $noteid = $hasnote ? $br->{noteid} : 0;
                $rowspan = $br->{rowspan};
                $color = $colormap->{$br->{buildstatus}};
                $status = $br->{buildstatus};
                print "<td rowspan=$rowspan bgcolor=${color}>\n";

                $logfile = $br->{logfile};
                $errorparser = $br->{errorparser};
                $buildname = $br->{buildname};
                if( $tree2 ne "" ){
                    $buildname =~ s/^[^ ]* //;
                }
                $popupbuildname = $buildname; #Added so text in popup isn't whacky
                $buildname = &url_encode($buildname);
                $buildtime = $br->{buildtime};
                $buildtree = $br->{td}->{name};

                print "<tt>\n";

                if( $hasnote ){
                    print "<a href='' onClick=\"return js_what_menu(event,$noteid,'$logfile','$errorparser','$buildname','$buildtime');\">";
                    print "<img src=star.gif border=0></a>\n";
                }
                print "<A HREF='showlog.cgi?logfile=$logfile\&tree=$buildtree\&errorparser=$errorparser&buildname=$buildname&buildtime=$buildtime&mainframe=1' " .
                      "onClick=\"return log_popup(event,'$buildtree','$popupbuildname'," .
                      "'showlog.cgi?logfile=$logfile\&tree=$buildtree\&errorparser=$errorparser&buildname=$buildname&buildtime=$buildtime&mainframe=1'," .
                      "'showlog.cgi?logfile=$logfile\&tree=$buildtree\&errorparser=$errorparser&buildname=$buildname&buildtime=$buildtime&fulltext=1&mainframe=1'," .
                      "'addnote.cgi?tree=$treename&buildname=$buildname&buildtime=$buildtime&logfile=$logfile&errorparser=$errorparser');\">";


                print "L</a>\n";

                #print "Build Summary</a><br>\n";

                if( $br->{previousbuildtime} ){
                    $qr = &query_ref($br->{td}, $br->{previousbuildtime},$br->{buildtime});
                    print "$qr\n";
                    print " C</a>\n";
                    #print "What Changed</a><br>\n";
                }

                if( $br->{binaryname} ne '' ){
                    $binfile = "$buildtree/bin/$buildtime/$br->{buildname}/$br->{binaryname}";
                    $binfile =~ s/ //g;
                    print " <a href=$binfile>B</a>";
                }
                print "</tt>\n";


            }
        }
        else  {
            print "<td>&nbsp;\n";
        }
        $bn++;
    }
    

    print "</tr>\n";
}


sub display_build_table_header {
    local($i,$nspan);

    print "<TABLE border cellspacing=2>\n";

    print "<tr align=center>\n";
    print "<td rowspan=1><font size=-1>Click time to <br>see changes <br>since time</font>";
    $nspan = ( $tree2 ne "" ? 4 : 1);
    print "<td colspan=$nspan><font size=-1>Click name to see what they did</font>";
    #print "<td colspan=$name_count><font size=-1>Burning builds are busted</font>";
    #print "</tr>\n";

    
    #print "<tr>\n";
    $i = 1;
    while ($i <= $name_count){

        $bn = $build_name_names->[$i];
        $bn =~ s/Clobber/Clbr/g;
        $bn =~ s/Depend/Dep/g;
        $t = &last_good_time($i);

        if( $form{'narrow'} ){
            $bn =~ s/([^:])/$1<br>/g;
            $bn = "<tt>$bn</tt>";
        }
        else {
            $bn = "<font face='Arial,Helvetica' size=-2>$bn</font>";

        }

        if( $t->{isbusted} ){
            print "<td rowspan=2 bgcolor=000000 background=1afi003r.gif>";
            print "<font color=white>$bn</font>\n";
            #print "<img src=reledanim.gif>\n";
        }
        else {
            print "<th rowspan=2 bgcolor=00ff00>";
            print "$bn\n";
        }
        $i++;
    }
    print "</tr>\n";

    print "<tr>\n";
    print "<b><TH>Build Time\n";
    if( $tree2 ne "" ){
        print "<TH colspan=2>$td1->{name}\n";
        print "<TH colspan=2>$td2->{name}\n";
    }
    else {
        print "<TH>Guilty\n";
    }
    print "</b></tr>\n";

}

sub display_build_table_footer {
    print "</table>\n";
    print "<a href=showbuilds.cgi?tree=$tree&showall=1.cgi>Show more checkin history</a><br><br>\n";

    if (open(FOOTER, "<$data_dir/footer.html")) {
        while (<FOOTER>) {
            print $_;
        }
        close FOOTER;
    }
    print "<a href=admintree.cgi?tree=$tree>Administrate Tinderbox Trees</a><br>";

    print "<br><br>";
}


sub query_ref {
    local( $td, $mindate, $maxdate, $who ) = @_;

    return "<a href=../bonsai/cvsquery.cgi?module=$td->{cvs_module}&branch=$td->{cvs_branch}&cvsroot=$td->{cvs_root}&date=explicit&mindate=$mindate&maxdate=$maxdate&who=$who>";
}

sub query_ref2 {
    local( $td, $mindate, $maxdate, $who ) = @_;
    return "../bonsai/cvsquery.cgi?module=$td->{cvs_module}&branch=$td->{cvs_branch}&cvsroot=$td->{cvs_root}&date=explicit&mindate=$mindate&maxdate=$maxdate&who=$who";
}


sub who_menu {
    local( $td, $mindate, $maxdate, $who ) = @_;
    my $treeflag;
    #$qr="../bonsai/cvsquery.cgi?module=$td->{cvs_module}&branch=$td->{cvs_branch}&cvsroot=$td->{cvs_root}&date=explicit&mindate=$mindate&maxdate=$maxdate&who=$who";
    $qr = "../registry/who.cgi?email=$who"
        . "&t0=" . &url_encode("What did $who check into the source tree" )
        . "&u0=" . &url_encode( &query_ref2($td,$mindate,$maxdate,$who) )
        . "&t1=" . &url_encode("What has $who been checking in in the last day" )
        . "&u1=" . &url_encode( &query_ref2($td,$mindate,$maxdate,$who) );

    return "<a href='$qr' onClick=\"return js_who_menu($td->{num},'$who',event,$mindate,$maxdate);\" >";
}


sub tree_open {
    local($done, $line, $a, $b);
    open( BID, "<../bonsai/data/$bonsai_tree/batchid") || print "can't open batchid<br>";
    ($a,$b,$bid) = split(/ /,<BID>);
    close( BID );
    open( BATCH, "<../bonsai/data/$bonsai_tree/batch-${bid}") || print "can't open batch-${bid}<br>";;
    $done = 0;
    while( ($line = <BATCH>) && !$done ){ 
        if($line =~ /^set treeopen/) {
            chop( $line );
            ($a,$b,$treestate) = split(/ /, $line );
            $done = 1;
        }
    }
    close( BATCH );
    return $treestate;
}

sub load_javascript {

$script_str =<<'ENDJS';
<script>


if( parseInt(navigator.appVersion) < 4 ){
    window.event = 0;
}


function js_who_menu(tree,n,d,mindate,maxdate) {
    if( parseInt(navigator.appVersion) < 4 ){
        return true;
    }

    l = document.layers['popup'];
    l.src = "../registry/who.cgi?email=" + n  
        + "&t0=" + escape("Last check-in" )
        + "&u0=" + escape( js_qr(tree,mindate,maxdate,n) )
        + "&t1=" + escape("Check-ins within 24 hours" )
        + "&u1=" + escape( js_qr24(tree,n) );

    //l.document.write(
    //    "<table border=1 cellspacing=1><tr><td>" + 
    //    js_qr(mindate,maxdate,n) + "What did " + n + " <b>check in to the source tree</b>?</a><br>" +
    //    js_qr24(n) +"What has " + n + " <b>been checking in over the last day</b>?</a> <br>" +
    //    "<a href=https://endor.mcom.com/ds/dosearch/endor.mcom.com/uid%3D" +n + "%2Cou%3DPeople%2Co%3DNetscape%20Communications%20Corp.%2Cc%3DUS>" +
    //        "Who is <b>" + n + "</b> and how do <b>I wake him/her up</b></a>?<br>" +
    //    "<a href='mailto:" + n + "?subject=Whats up with...'>Send mail to <b>" + n + "</b></a><br>" +
    //    "<a href=http://dome/locator/findUser.cgi?email="+n+">Where is <b>"+n+"'s office?</b></a>" +
    //    "</tr></table>");
    //l.document.close();

    //alert( d.y );
    l.top = d.target.y - 6;
    l.left = d.target.x - 6;
    if( l.left + l.clipWidth > window.width ){
        l.left = window.width - l.clipWidth;
    }
    l.visibility="show";
    return false;
}

function js_what_menu(d,noteid,logfile,errorparser,buildname,buildtime) {
    if( parseInt(navigator.appVersion) < 4 ){
        return true;
    }

    l = document.layers['popup'];
    l.document.write(
        "<table border=1 cellspacing=1><tr><td>" + 
        note_array[noteid] + 
        "</tr></table>");
    l.document.close();

    l.top = d.y-10;
    zz = d.x;
    //alert( l.clip.right+ " " + (window.innerWidth -30) );
    if( zz + l.clip.right > window.innerWidth ){
        zz = (window.innerWidth-30) - l.clip.right;
        if( zz < 0 ){
            zz = 0;
        }
        
    }
    l.left = zz;
    l.visibility="show";
    return false;
}

note_array = new Array();

</script>

<layer name="popup"  onMouseOut="this.visibility='hide';" left=0 top=0 bgcolor="#ffffff" visibility="hide">
</layer>

<layer name="logpopup"  onMouseOut="this.visibility='hide';" left=0 top=0 bgcolor="#ffffff" visibility="hide">
</layer>

<SCRIPT>
function log_popup(e,tree,platform,brieflogurl,fulllogurl,commenturl) {

    if( parseInt(navigator.appVersion) < 4 ){
        return true;
    }

    q = document.layers["logpopup"];

    q.top = e.target.y - 6;

    yy = e.target.x;
    if( yy + q.clip.right > window.innerWidth ){
        yy = (window.innerWidth-30) - q.clip.right;
        if( yy < 0 ){
            yy = 0;
        }
    
    }

    q.left = yy;


//    q.left = e.target.x - 6;
//    if( q.left + q.clipWidth > window.width ){
//        q.left = window.width - q.clipWidth;
//    }

    q.visibility="show"; 
    q.document.write("<TABLE BORDER=1><TR><TD><B>" + platform + "--" + tree + "</B><BR>" +
        "<A HREF=\"" + brieflogurl + "\">View Brief Log</A><BR>" +
        "<A HREF=\"" + fulllogurl + "\">View Full Log</A><BR>" +
        "<A HREF=\"" + commenturl + "\">Add a Comment this Log</A><BR>" +
	"</TD></TR></TABLE>");
    q.document.close();
    return false;
}

</SCRIPT>

ENDJS

$script_str .= "
<script>

function js_qr(tree,mindate, maxdate, who ){
    if (tree == 0 ){
        return '../bonsai/cvsquery.cgi?module=${cvs_module}&branch=${cvs_branch}&cvsroot=${cvs_root}&date=explicit&mindate=' 
            + mindate + '&maxdate=' +maxdate + '&who=' + who ;
    }
    else {
        return '../bonsai/cvsquery.cgi?module=$td2->{cvs_module}&branch=$td2->{cvs_branch}&cvsroot=$td2->{cvs_root}&date=explicit&mindate=' 
            + mindate + '&maxdate=' +maxdate + '&who=' + who ;
    }
}

function js_qr24(tree,who){
    if (tree == 0 ){
        return '../bonsai/cvsquery.cgi?module=${cvs_module}&branch=${cvs_branch}&cvsroot=${cvs_root}&date=day' 
            + '&who=' +who;
    }
    else{
        return '../bonsai/cvsquery.cgi?module=$td2->{cvs_module}&branch=$td2->{cvs_branch}&cvsroot=$td2->{cvs_root}&date=day' 
            + '&who=' +who;
    }
}
";

$i = 0;
while( $i < @note_array ){
    $s = $note_array[$i];
    $s =~ s/\\/\\\\/g;
    $s =~ s/\"/\\\"/g;
    $s =~ s/\n/\\n/g;
    $script_str .= "note_array[$i] = \"$s\";\n";
    $i++;
}

$script_str .= "</script>\n";

}


sub do_express {
    local($mailtime, $buildtime, $buildname, $errorparser, $buildstatus, $logfile);
    local($buildrec);
    local(%build);

    open(BUILDLOG, "<$form{'tree'}/build.dat" ) || die ;
    while( <BUILDLOG> ){
        chop;
        ($mailtime, $buildtime, $buildname, $errorparser, $buildstatus, $logfile) = 
            split( /\|/ );
        if( $buildstatus eq 'success' || $buildstatus eq 'busted'){
            $build{$buildname} = $buildstatus;
        }
    }
    close( BUILDLOG );

    @keys = sort keys %build;
    $keycount = @keys;
    $treename = $form{tree};
    $tm = &print_time(time);
    print "<table border=1 align=center><tr><th colspan=$keycount><a href=showbuilds.cgi?tree=$treename>$tree as of $tm</a></tr>"
          ."<tr>\n";
    for $buildname (@keys ){
        if( $build{$buildname} eq 'success' ){
            print "<td bgcolor=00ff00>";
        }
        else {
            print "<td bgcolor=000000 background=1afi003r.gif>";
            print "<font color=white>\n";
        }
        print "$buildname";
    }
    print "</tr></table>\n";
}
