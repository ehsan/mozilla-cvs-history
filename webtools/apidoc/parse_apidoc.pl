#!/usr/bin/perl
#
# The contents of this file are subject to the Netscape Public
# License Version 1.1 (the "License"); you may not use this file
# except in compliance with the License. You may obtain a copy of
# the License at http://www.mozilla.org/NPL/
#
# Software distributed under the License is distributed on an "AS
# IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
# implied. See the License for the specific language governing
# rights and limitations under the License.
#
# The Original Code is Mozilla WebTools.
#
# The Initial Developer of the Original Code is Netscape
# Communications Corporation.  Portions created by Netscape are
# Copyright (C) 1997-1999 Netscape Communications Corporation. All
# Rights Reserved.
#
# Alternatively, the contents of this file may be used under the
# terms of the GNU Public License (the "GPL"), in which case the
# provisions of the GPL are applicable instead of those above.
# If you wish to allow use of your version of this file only
# under the terms of the GPL and not to allow others to use your
# version of this file under the NPL, indicate your decision by
# deleting the provisions above and replace them with the notice
# and other provisions required by the GPL.  If you do not delete
# the provisions above, a recipient may use your version of this
# file under either the NPL or the GPL.
#
# Contributor(s):
#  Robert Ginda <rginda@netscape.com>, Initial development.
#  Pavel Hlavnicka <pavel@gingerall.cz>, separate tocs, param linking.
#  Petr Cimprich <petr@gingerall.cz>, nested frameset fix, encoded URLs.
#  Petr Cimprich <petr@gingerall.cz>, Mozilla sidebar generated from TOC
#

use strict;
use XML::Parser;

my $file = shift;
my $outdir = shift || "apidocs";
my $c;
my $tagname;
my $pending_param;
my $pending_text;
my %pending_attrs;
my %groups;
my %externals;
my %toc_externals;
my $user_foot;
my $user_head;
my @tag_stack;
my @text_stack;
my @attr_stack;
my $inited = 0;
my %entries;
my $apiid;

my $API         = 0;
my $ENTRY       = 1;
my $TYPE        = 2;
my $SUMMARY     = 3;
my $SYNTAX      = 4;
my $PARAM       = 5;
my $RETVAL      = 6;
my $DESCRIPTION = 7;
my $EXAMPLE     = 8;
my $NOTE        = 9;
my $SEE_ALSO    = 10;
my $DEPRECATED  = 11;
my $EXTERNALREF = 12;
my $GROUP       = 13;
my $C           = 14;
my $P           = 15;
my $BR          = 16;
my $B           = 17;
my $I           = 18;
my $S           = 19;
my $FOOT        = 20;
my $HEAD        = 21;
my $COMPLETED   = 22;
my @TAGS = ("API", "ENTRY", "TYPE", "SUMMARY", "SYNTAX", "PARAM", "RETVAL",
            "DESCRIPTION", "EXAMPLE", "NOTE", "SEEALSO", "DEPRECATED", 
            "EXTERNALREF", "GROUP", "C", "P", "BR", "B", "I", "S", "FOOT",
            "HEAD");
my @CDATA_TAGS = ($TAGS[$SUMMARY], $TAGS[$SYNTAX], $TAGS[$DESCRIPTION], 
                  $TAGS[$EXAMPLE], $TAGS[$NOTE], $TAGS[$PARAM], $TAGS[$C],
                  $TAGS[$B], $TAGS[$I], $TAGS[$S], $TAGS[$FOOT], $TAGS[$HEAD]);
my @FORMATTING_TAGS = ($TAGS[$P], $TAGS[$C], $TAGS[$B], $TAGS[$I], $TAGS[$BR],
                       $TAGS[$BR], $TAGS[$S]);
my @FORMAT_CONTAINERS = ($TAGS[$SUMMARY], $TAGS[$PARAM], $TAGS[$RETVAL],
                         $TAGS[$DESCRIPTION], $TAGS[$NOTE], $TAGS[$C],
                         $TAGS[$B], $TAGS[$I], $TAGS[$FOOT], $TAGS[$HEAD]);
my @CODE_TAGS = ($TAGS[$SYNTAX], $TAGS[$EXAMPLE]);

my $URLVAR_ENTRY = "{e}";

my $footstr = "<center>This page was generated by " .
  "<a href='http://www.mozilla.org/projects/apidoc' target='other_window'>" .
  "<b>APIDOC</b></a>" .
  "</center>\n</body></html>";

my $WARNING = "<!--\n" .
  "  -- HEADS UP!  This page was *GENERATED* by APIDOC,\n".
  "  -- DO NOT EDIT THIS FILE BY HAND!\n" .
  "  -- See http://www.mozilla.org/projects/apidoc for information on APIDOC\n" .
  "  -- The original source file was " . $file . "\n" .
  "  -->\n";

my $JS_COMPLETE = ("\n<script>\n" .
                   "function navToEntry(entry) {\n" .
                   "  window.location.hash=entry;\n" .
                   "}\n" .
                   "function navToGroup(group) {\n" .
                   "  var f = parent.frames['toc-container'];\n" .
                   "  if (!f)\n".
                   "    window.open ('complete-toc.html#' + group, " .
                   "'toc_container');\n" .
                   "  else {\n" .
                   "    if (f.location.href.search('abc') != -1)\n" .
                   "        f.location.href = 'complete-toc.html#' + group;\n" .
                   "    else\n" .
                   "        f.location.hash = group;\n" .
                   "  }\n" .
                   "}\n" .
                   "</script>\n");

my $JS_SPARSE = ("\n<script>\n" .
                 "function navToEntry(entry) {\n" .
                 "  window.location.href='api-' + entry + '.html';\n" .
                 "}\n" .
                 "function navToGroup(group) {\n" .
                 "  var f = parent.frames['toc-container'];\n" .
                 "  if (!f)\n".
                 "    window.open ('sparse-toc.html#' + group, " .
                 "'toc_container');\n" .
                 "  else {\n" .
                 "    if (f.location.href.search('abc') != -1)\n" .
                 "        f.location.href = 'sparse-toc.html#' + group;\n" .
                 "    else\n" .
                 "        f.location.hash = group;\n" .
                 "  }\n" .
                 "}\n" .
                 "</script>\n");

open (COMPLETE, ">" . $outdir . "/complete.html") ||
  die ("Couldn't open $outdir/complete.html.\n");
open (COMPLETE_TOC, ">" . $outdir . "/complete-toc.html") ||
  die ("Couldn't open $outdir/complete-toc.html.\n");

open (COMPLETE_TOC_ABC, ">" . $outdir . "/complete-toc-abc.html") ||
  die ("Couldn't open $outdir/complete-toc-abc.html.\n");
open (COMPLETE_TOC_GRP, ">" . $outdir . "/complete-toc-grp.html") ||
  die ("Couldn't open $outdir/complete-toc-grp.html.\n");

open (SPARSE_TOC, ">" . $outdir . "/sparse-toc.html") ||
  die ("Couldn't open $outdir/sparse-toc.html.\n");

open (SPARSE_TOC_ABC, ">" . $outdir . "/sparse-toc-abc.html") ||
  die ("Couldn't open $outdir/sparse-toc-abc.html.\n");
open (SPARSE_TOC_GRP, ">" . $outdir . "/sparse-toc-grp.html") ||
  die ("Couldn't open $outdir/sparse-toc-grp.html.\n");

open (SIDEBAR_TOC, ">" . $outdir . "/sidebar-toc.html") ||
  die ("Couldn't open $outdir/sidebar-toc.html.\n");


&main();

sub main {
    my $parser = new XML::Parser(ErrorContext => 2);

    # pass 1, scan all <ENTRY> tags.
    $parser->setHandlers(Start => \&p1_Start, End => \&p1_End);
    $parser->parsefile($file);

    # sanity check the tag stack from p1
    if ($#tag_stack != -1) {
        die ("OOPS: p1 left the tag stack in a bad state.\n");
    }

    # pass 2, populate the $entries hash.
    $parser = new XML::Parser(Style => "Stream", ErrorContext => 2);
    $parser->parsefile ($file);

    # finally, write it all out.
    &init_files();

    my $k;
    my $html;

    for $k (sort (keys(%entries))) {
        $c = $entries{$k};
        $html = &get_entry_html();
        &add_entry_complete($html);
        &add_entry_sparse($html);
        &add_toc_complete(*COMPLETE_TOC);
        &add_toc_complete(*COMPLETE_TOC_ABC);
        &add_toc_sparse(*SPARSE_TOC);
        &add_toc_sparse(*SPARSE_TOC_ABC);
        #&debug_write_entry();
    }

    &end_abc(*SPARSE_TOC);
    &end_abc(*SPARSE_TOC_ABC);
    &end_abc(*COMPLETE_TOC);
    &end_abc(*COMPLETE_TOC_ABC);

    &write_toc_groups();

    &close_files();

}

sub p1_Start {
    # pass 1, find all <ENTRY/>, <EXTERNALREF/>, and <GROUP/> tags
    # (as well as groups implied by <TYPE/> and <DEPRECATED/> tags), so we
    # can do things like auto link <C/> tags that refer to entrys,
    # and validate <S/> tags in pass 2.
    my $expat = shift;
    my $lasttagname = $tagname;
    my $n;
    my %pending_attrs;

    push (@tag_stack, $lasttagname);
    $tagname = shift;

    while ($n = shift) {
        $pending_attrs{$n} = shift;
    }

    my $value = $pending_attrs{"value"};
    my $id = $pending_attrs{"id"};

    if ($tagname eq $TAGS[$ENTRY]) {
        if ($id) {
            $c = $entries{$id} = {$TAGS[$ENTRY] => $id};
        } else {
            &croak_attr ($expat, $tagname, "id");
        }
    } elsif ($tagname eq $TAGS[$TYPE]) {
        if (!$value) {
            &croak_attr ($expat, $tagname, "value");
        }
        $c->{$tagname} = $value;
        push (@{$groups{$value}}, $c->{$TAGS[$ENTRY]});
        push (@{$c->{$TAGS[$GROUP]}}, $value);
    } elsif ($tagname eq $TAGS[$DEPRECATED]) {
        $c->{$tagname} = 1;
        push (@{$groups{"Deprecated"}}, $c->{$TAGS[$ENTRY]});
        push (@{$c->{$TAGS[$GROUP]}}, "Deprecated");
    } elsif ($tagname eq $TAGS[$GROUP]) {
        if (($lasttagname ne $TAGS[$API]) && 
            ($lasttagname ne $TAGS[$ENTRY])) {
            $expat->xpcroak ("Tag $tagname can only be contained by ".
                             "an '" . $TAGS[$API] . "' or '" .
                             $TAGS[$ENTRY] . "' tag");
        }
        my $name = $pending_attrs{"name"};
        if (!$name) {
            &croak_attr ($expat, $tagname, "name");
        }
        if ($lasttagname ne $TAGS[$ENTRY]) {
            if (!$value) {
                &croak_attr ($expat, $tagname, "value");
            }
        } else {
            $value = $c->{$TAGS[$ENTRY]};
        }
        if (!grep (/^$value$/, @{$groups{$name}})) {
            # if it isn't already there, add it
            push (@{$groups{$name}}, $value);
            push (@{$entries{$value}->{$TAGS[$GROUP]}}, $name);
        }
    } elsif ($tagname eq $TAGS[$EXTERNALREF]) {
        my $name = $pending_attrs{"name"};
        my $value = $pending_attrs{"value"};
        if (!$name) {
            &croak_attr ($expat, $tagname, "name");
        }
        if (!$value) {
            &croak_attr ($expat, $tagname, "value");
        }
        if ($lasttagname eq $TAGS[$API]) {
            # if the externalref is a child of the API tag
            if ($value =~ /$URLVAR_ENTRY/) {
                # and it has a placeholder for the entry id,
                # then attach it to every entry
                $externals{$name} = $value;
            } else {
                # otherwise, just put it in the toc.
                $toc_externals{$name} = $value;
            }
        } elsif ($lasttagname eq $TAGS[$ENTRY]) {
            # if the externalref is a child of the ENTRY tag
            # only attach it to this entry
            $c->{$TAGS[$EXTERNALREF]}{$name} = $value;
        } else {
            $expat->xpcroak ("Tag $tagname can only be contained by ".
                             "an '" . $TAGS[$API] . "' or '" .
                             $TAGS[$ENTRY] . "' tag");
        }
    }
}

sub p1_End {
    $tagname = pop (@tag_stack);
}

sub StartTag {
    # phase 2 open tag handler
    my ($expat) = @_;
    $_ =~ /<([^\s>]*)/;
    my $lasttagname = $tagname;
    $tagname = $1;

    push (@tag_stack, $lasttagname);
    push (@text_stack, $pending_text);
    my $s = $#attr_stack + 1;
    $attr_stack[$s]{"foo"} = "bar";
    for (keys (%pending_attrs)) {
        $attr_stack[$s]{$_} = $pending_attrs{$_};
    }

    $pending_text = "";
    %pending_attrs = %_;

    if (!grep(/^$tagname$/, @TAGS)) {
        $expat->xpcroak ("Unknown tag '$tagname'");
    }

    #    print ("opening: ");
    #    &debug_dump_c();

    my $value = $pending_attrs{"value"};
    my $id = $pending_attrs{"id"};

    if ($tagname eq $TAGS[$API]) {
        if ($inited) {
            $expat->xpcroak ("Only one '$tagname' tag allowed");
        }
        if (!$id) {
            &croak_attr ($expat, $tagname, "id");
        }
        $apiid = $id;
        $inited = 1;
    } elsif ($inited) {
        if ($tagname eq $TAGS[$ENTRY]) {
            if (!$id) {
                &croak_attr ($expat, $tagname, "id");
            }
            $c = $entries{$id};
        } elsif ($tagname eq $TAGS[$SEE_ALSO]) {
            if (!$value) {
                &croak_attr ($expat, $tagname, "value");
            } elsif (!$entries{$value}) {
                $expat->xpcroak ("Undefined SEEALSO reference, '$value'");
            }
            if (!grep (/^$value$/, @{$c->{$TAGS[$SEE_ALSO]}})) {
                push (@{$c->{$TAGS[$SEE_ALSO]}}, $value);
            }
        } elsif (grep(/^$tagname$/, @FORMATTING_TAGS)) {
            if (!grep(/^$lasttagname$/, @FORMAT_CONTAINERS)) {
                $expat->xpcroak ("Tag $lasttagname cannot contain formatting " .
                                 "tags");
            }
        } elsif (($tagname eq $TAGS[$PARAM]) || ($tagname eq $TAGS[$RETVAL])) {
            if ($lasttagname ne $TAGS[$SYNTAX]) {
                $expat->xpcroak ("Tag $tagname can only be contained by a '" .
                                 $TAGS[$SYNTAX] . "' tag");
            }
            if (!$pending_attrs{"name"}) {
                if ($tagname eq $TAGS[$RETVAL]) {
                    $pending_attrs{"name"} = "Return Value";
                } else {
                    &croak_attr ($expat, $tagname, "name");
                }
            }
            if (!$pending_attrs{"type"}) {
                $pending_attrs{"type"} = "&nbsp;";
                # &croak_attr ($expat, $tagname, "type");
            }
        } elsif ((($tagname eq $TAGS[$HEAD]) || ($tagname eq $TAGS[$HEAD])) &&
                 ($lasttagname ne $TAGS[$API])) {
            $expat->xpcroak ("Tag $tagname can only be contained by ".
                             "an '" . $TAGS[$API] . "' tag");
        }
    } else {
        $expat->xpcroak ("Tag '$tagname' must be contained in an '" .
                         $TAGS[$API] . " tag");
    }
}

sub EndTag {
    # phase 2 close tag handler
    my ($expat) = @_;
    my $iscontainer = 0;
    $_ =~ /<\/([^\s>]*)/;
    $tagname = $1;

#    print ("closing: ");
#    &debug_dump_c();

    if (grep(/^$tagname$/, @CDATA_TAGS)) {
        $iscontainer = 1;
        if ($pending_text eq "") {
            print STDERR "WARNING: Empty container '$tagname' at line " .
              $expat->current_line . " ignored.\n";
            #$expat->xpcroak ("Empty container '$tagname'.");
        }
    }

    if (grep(/^$tagname$/, @FORMATTING_TAGS)) {
        if ($iscontainer) {
            if ($tagname eq $TAGS[$C]) {
                # code tag
                if (($pending_text ne $c->{$TAGS[$ENTRY]}) &&
                    ($entries{$pending_text})) {
                    # if the contents are a valid entry
                    # add it to the SEEALSO, in case it's not already there
                    if (!grep (/^$pending_text$/, @{$c->{$TAGS[$SEE_ALSO]}})) {
                        push (@{$c->{$TAGS[$SEE_ALSO]}}, $pending_text);
                    }
                    # and make it a link
                    $pending_text = &get_link($pending_text);
                }
                $pending_text = "<code>$pending_text</code>";
            } elsif ($tagname eq $TAGS[$S]) {
                # seealso reference
                my $value;
                if (($value = $entries{$pending_text}) ||
                    ($value = $toc_externals{$pending_text}) ||
                    ($value = $c->{$TAGS[$EXTERNALREF]}{$pending_text})) {
                    # it's a valid external reference
                    # put it in this entry's external references incase it isn't
                    # already there.
                    $c->{$TAGS[$EXTERNALREF]}{$pending_text} = $value;
                    $_ = $value;
                    s/$URLVAR_ENTRY/$c->{$TAGS[$ENTRY]}/g;
                    $pending_text =
                      "<a href='$_' target='other_window'>" .
                        "$pending_text</a>";
                } elsif ($value = $groups{$pending_text}) {
                    # it's a valid group
                    # put it in this entry's group references incase it isn't
                    # already there.
                    if ((!$c->{$TAGS[$GROUP]}) ||
                        (!grep (/^$pending_text$/, @{$c->{$TAGS[$GROUP]}}))) {
                        push (@{$c->{$TAGS[$GROUP]}}, $pending_text);
                    }
                    $pending_text ="<a href='javascript:" .
                      "navToGroup(\"GROUP_$pending_text\")'>$pending_text</a>";
                } else {
                    # it's just not valid
                    $expat->xpcroak ("Unknown reference in '" . $TAGS[$S] .
                                     "' tag");
                }
            } elsif ($tagname eq $TAGS[$B]) {
                # bold
                $pending_text = "<b>$pending_text</b>";
            } elsif ($tagname eq $TAGS[$I]) {
                # italic
                $pending_text = "<i>$pending_text</i>";
            } else {
                expat->xpcroak 
                  ("OOPS: Unhandled container formatting tag '$tagname'");
            }
        } else {
            if ($tagname eq $TAGS[$P]) {
                # paragraph
                $pending_text = "<P>";
            } elsif ($tagname eq $TAGS[$BR]) {
                # br
                $pending_text = "<BR>";
            } else {
                expat->xpcroak
                  ("OOPS: Unhandled non-container formatting tag '$tagname'");
            }
        }
        # combine with previous pendingtext
        $pending_text = pop(@text_stack) . $pending_text;
    } elsif ($iscontainer) {
        # not a formatting tag, store the accumulated pendingtext in the
        # right place, after some whitespace trimming
        my @lines = split ("\n", $pending_text);
        my $iscode = grep (/^$tagname$/, @CODE_TAGS);
        my $i;
        my $line;
        my $result_text = "";

        for $i (0 ... $#lines) {
            $line = $lines[$i];
            if ((($i != 0) && ($i != $#lines)) || ($line =~ /[\S\n]/)) {
                if ($iscode) {
                    $line = &add_leading_nbsp($line);
                } else {
                    $_ = $line;
                    s/\.\s\s(.)/\.\&nbsp;\&nbsp;$1/g;
                    $line = $_;
                }
                $line =~ /^[\s\n]*(.*)[\s\n]*/;
                $result_text .= $1 . "\n";
            }
        }

        $_ = $result_text;
#        s/\n/<br>/g;
        $result_text = $_;

        if (($tagname eq $TAGS[$PARAM]) || ($tagname eq $TAGS[$RETVAL])) {
            # parameter block
            my $name = $pending_attrs{"name"};
            my $type = $pending_attrs{"type"};
            my $html = ("<td class='param-name'><code>$name</code></td>" .
                        "<td class='param-type'><code>$type</code></td>" .
                        "<td class='param-desc'>$result_text</td>\n");
            push (@{$c->{$TAGS[$PARAM]}}, $html);
        } elsif ($tagname eq $TAGS[$EXAMPLE]) {
            $c->{$tagname . "_DESC"} = $pending_attrs{"desc"};
            $c->{$tagname} = $result_text;
        } elsif ($tagname eq $TAGS[$HEAD]) {
            $user_head .= $result_text;
        } elsif ($tagname eq $TAGS[$FOOT]) {
            $user_foot .= $result_text;
        } else {
            $c->{$tagname} .= $result_text;
        }

        $pending_text = pop (@text_stack);
    } else {
        $pending_text = pop (@text_stack);
    }

    %pending_attrs = %{pop (@attr_stack)};
    $tagname = pop (@tag_stack);

#    print ("popped: ");
#    &debug_dump_c();

}

sub Text {
    my ($expat) = @_;

    if (/^[\s\n]+$/) {
        return;
    }
    if (!grep(/^$tagname$/, @CDATA_TAGS)) {
        $expat->xpcroak ("Tag '$tagname' cannot contain text");
    }

    $pending_text .= $_;

}

sub EndDocument {
}

sub get_type_links {
    my @types = split /\s*,\s*/, shift;
    foreach my $type (@types) {
	#_PH_ - fix to better parse C pointer types - a bit unsafe
	$type =~ m|(&\s*)*([^ *]+)(\s*\*)*|;
	my ($pre, $realtype, $post) = ($1, $2, $3);
	if (exists $entries{$realtype}) {
	    if (!grep (/^$realtype$/, @{$c->{$TAGS[$SEE_ALSO]}})) {
		push (@{$c->{$TAGS[$SEE_ALSO]}}, $realtype);
	    }
	    $realtype = &get_link($realtype) ;
	    $type = "$pre$realtype$post";
	}
    }
    return join ", ", @types;
}

sub get_param_links {
    my $html = shift;
    if ($html =~ m|class='(param-type'><code>)(.*?)(</code>)|g) {
	my $new_param = &get_type_links($2);
	$html =~ s|class='(param-type'><code>)(.*?)(</code>)|$1$new_param$3|;
    }
    return $html;
}

sub get_entry_html {
    # get html for the current entry ($c)
    my $html = "";
    $c->{$TAGS[$ENTRY]} =~ /\.?(.*)/;
    my $id = $1;
    $html = "<center><table class='api-entry' width='100%' cellspacing='0'" .
      "border='1' cellpadding='10'>\n";
    $html .= "<tr><td class='entry-heading'>\n";

    $html .= "<table class='entry-heading-table' width='100%' cellpadding='5'" .
      "cellspacing='0'><tr>\n";
    $html .= "<td class='entry-title' valign='center'><font size='+5'>" .
      $id . "</font></td>\n";
    $html .= "<td class='entry-type' align='center' width='25%'>" .
      $c->{$TAGS[$TYPE]} . "</td>\n";
    if ($c->{$TAGS[$DEPRECATED]}) {
        $html .= "<td class='entry-deprecated' align='center' width='25%'>" .
          "Deprecated</td>\n";
    }
    $html .= "</tr></table>\n";

    $html .= "</td></tr>\n";

    if ($c->{$TAGS[$SUMMARY]}) {
        $html .= "<tr><td class='entry-summary'>\n";
        $html .= "<h4 class='entry-subhead'>Summary</h4>\n";
        $html .= $c->{$TAGS[$SUMMARY]};
        $html .= "</td></tr>\n";
    }
    if ($c->{$TAGS[$SYNTAX]}) {
        $html .= "<tr><td class='entry-syntax'>\n";
        $html .= "<h4 class='entry-subhead'>Syntax</h4><pre>\n";
        $html .= $c->{$TAGS[$SYNTAX]};
        $html .= "</pre>\n";
        if ($c->{$TAGS[$PARAM]}) {
            $html .= "<center><table class='param-list' border='1' " .
              "cellpadding='3' cellspacing='1'>";
            $html .= "<tr class='param-list-head'>";
            $html .= "<th>Name</th><th>Type</th><th>Description</th></tr>\n";
            my $param = shift (@{$c->{$TAGS[$PARAM]}});
            my $even = 1;
            while ($param) {
                $_ = $param;
                if ($even == 1) {
                    $html .= "<tr class='param-row-even'>";
                } else {
                    $html .= "<tr class='param-row-odd'>";
                }
                $param = $_;

		$param = &get_param_links($param);
                $even *= -1;
                $html .= $param . "</tr>\n";
                $param = shift (@{$c->{$TAGS[$PARAM]}});
            }
            $html .= "</table></center>\n"
        }
        $html .= "</td></tr>\n";
    }
    if ($c->{$TAGS[$DESCRIPTION]}) {
        $html .= "<tr><td class='entry-description'>\n";
        $html .= "<h4 class='entry-subhead'>Description</h4>\n";
        $html .= $c->{$TAGS[$DESCRIPTION]};
        $html .= "</td></tr>\n";
    }
    if ($c->{$TAGS[$EXAMPLE]}) {
        $html .= "<tr><td class='entry-example'>\n";
        $html .= "<h4 class='entry-subhead'>Example</h4>\n";
        if ($c->{$TAGS[$EXAMPLE] . "_DESC"}) {
            $html .= $c->{$TAGS[$EXAMPLE] . "_DESC"} . "<br>";
        }
        $html .= "<pre>" . $c->{$TAGS[$EXAMPLE]};
        $html .= "</pre></td></tr>\n";
    }
    if ($c->{$TAGS[$NOTE]}) {
        $html .= "<tr><td class='entry-notes'>\n";
        $html .= "<h4 class='entry-subhead'>Notes</h4>\n";
        $html .= $c->{$TAGS[$NOTE]};
        $html .= "</td></tr>\n";
    }

    my $sa = get_seealso();
    if ($sa) {
        $html .= "<tr><td class='entry-seealso'>\n";
        $html .= "<h4 class='entry-subhead'>See Also</h4>\n";
        $html .= $sa;
        $html .= "</td></tr>\n";
    }

    $html .= "</table></center><br>\n";
    return $html;

}

sub get_seealso {
    # get the see also section for the current entry ($c);
    my @links;
    my $k;
    my $i;
    my $html = "";

    for (@{$c->{$TAGS[$GROUP]}}) {
        push (@links, "<a href='javascript:navToGroup(\"GROUP_$_\")'>$_</a>");
    }

    if ($#links != -1) {
        $html .= "<tr class='seealso-groups'><td>Groups</td>\n";
        $html .= "<td>[ " . join (" | ", sort(@links)) . " ]</td></tr>\n";
    }

    @links = ();

    # global externals (had a parent tag of <API/>)
    for $k (keys(%externals)) {
        $_ = $externals{$k};
        s/$URLVAR_ENTRY/$c->{$TAGS[$ENTRY]}/g;
        push (@links, "<a href='$_' target='other_window'>$k</a>");
    }

    # local externals (parented by <ENTRY/>
    for $k (keys(%{$c->{$TAGS[$EXTERNALREF]}})) {
        $_ = $c->{$TAGS[$EXTERNALREF]}{$k};
        s/$URLVAR_ENTRY/$c->{$TAGS[$ENTRY]}/g;
        push (@links, "<a href='$_' target='other_window'>$k</a>");
    }

    if ($#links != -1) {
        $html .= "<tr class='seealso-externals'><td>Documents</td>\n";
        $html .= "<td>[ " . join (" | ", sort(@links)) . " ]</td></tr>\n";
    }

    @links = ();

    for $k (@{$c->{$TAGS[$SEE_ALSO]}}) {
        push (@links, &get_link($k));
    }

    if ($#links != -1) {
        $html .= "<tr class='seealso-internals'><td>Entries</td>\n";
        $html .= "<td>[ " . join (" | ", sort(@links)) . " ]</td></tr>\n";
    }

    if ($html) {
        $html = "<table class='seealso-table'>\n" . $html . "\n</table>\n";
    }

    return $html;

}

sub add_entry_complete {
    # add html for the current entry to the "complete" page
    my ($html) = @_;

    print COMPLETE "<a name='" . $c->{$TAGS[$ENTRY]} . "'></a>\n";
    print COMPLETE $html;

}

sub add_entry_sparse {
    # add html for the current entry to a new "sparse" page
    my ($html) = @_;
    my $outfile = $outdir . "/api-" . $c->{$TAGS[$ENTRY]} . ".html";

    open (SPARSE, ">$outfile") ||
      die ("Couldn't open $outfile.\n");

    my $headstr = "<html><head><link rel=StyleSheet href='api-content.css' " .
      "TYPE='text/css' MEDIA='screen'>" .
        "<title>" . $c->{$TAGS[$ENTRY]} . "</title>" . $JS_SPARSE .
          "</head><body bgcolor='white'>\n$WARNING" .
            "<h1 class='title'>$apiid Reference</h1>\n" . $user_head;

    print SPARSE $headstr;
    print SPARSE $html;

    print SPARSE $user_foot;
    print SPARSE $footstr;

    close SPARSE;
}

sub end_abc {
    local (*G) = shift;
    print G "</table></center></td></tr>\n";
}

sub write_toc_groups {
    # Write the groups section of the toc to both the sparse and complete
    # toc files
    my @groups = sort(keys (%groups));
    my $g;
    my $even = 1;
    my $head = "<tr class='toc-title'><th><br><h3>Grouped Listing</h3></th>" .
        "</tr>\n";

    print COMPLETE_TOC $head;
    print COMPLETE_TOC_GRP $head;
    print SPARSE_TOC $head;
    print SPARSE_TOC_GRP $head;
    #print SIDEBAR_TOC $head;

    for $g (@groups) {
        $head = "<tr><td class='";
        if ($even == 1) {
            $head .= "toc-group-even";
        } else {
            $head .= "toc-group-odd";
        }
        $head .= "'><center><table border='0' cellspacing='0' cellpading='0' " .
          "width='100%'>\n";
        $head .= "<tr><th><a name='GROUP_$g'>$g</a></th><td>&nbsp;</td></tr>\n";
        print COMPLETE_TOC $head;
        print COMPLETE_TOC_GRP $head;
        print SPARSE_TOC $head;
        print SPARSE_TOC_GRP $head;
	print SIDEBAR_TOC $head;
        my $e;
        for $e (sort(@{$groups{$g}})) {
            $c = $entries{$e};
            &add_toc_complete(*COMPLETE_TOC);
            &add_toc_complete(*COMPLETE_TOC_GRP);
            &add_toc_sparse(*SPARSE_TOC);
            &add_toc_sparse(*SPARSE_TOC_GRP);
            &add_toc_sparse(*SIDEBAR_TOC, 1);
        }
        $head = "</table></center><br></td></tr>\n";
        print COMPLETE_TOC $head;
        print COMPLETE_TOC_GRP $head;
        print SPARSE_TOC $head;
        print SPARSE_TOC_GRP $head;
	print SIDEBAR_TOC $head;
        $even *= -1;
    }

}

sub add_toc_complete {
    local (*G) = shift;
    # add the current entry ($c) to the complete toc
    #print COMPLETE_TOC &add_toc(0);
    print G &add_toc(0);
}

sub add_toc_sparse {
    local (*G) = shift;
    my $sidebar = shift;
    # add the current entry ($c) to the sparse toc
    #print SPARSE_TOC &add_toc(1);
    print G &add_toc(1,$sidebar);
}

sub add_toc {
    # add the current entry ($c) to the either the complete or sparse toc,
    # based on the is_sparse parameter.  Should only be called from
    # add_toc_sparse or add_toc_complete.
    my ($is_sparse, $sidebar) = @_;
    my $html;
    my $classsuffix = $c->{$TAGS[$DEPRECATED]} ? "-deprecated" : "";

    $html = "<tr><td class='toc-row$classsuffix'>";

    $html .= &get_toc_link($c->{$TAGS[$ENTRY]}, $is_sparse,
                           "toc-entry$classsuffix", $sidebar)
      . "</td>";

    if ($classsuffix) {
        $html .= "<td class='toc-ind-deprecated' width='10%' " .
          "align='center' valign='center'>D</td>";
    } else {
        $html .= "<td>&nbsp;</td>";
    }

    $html .= "</tr>\n";

    return $html;
}

sub get_link {
    # get a link for use in a content page.  Works in both sparse and complete
    # pages (because of the navToEntry call.)
    my ($entry) = @_;
    my $entryE = _encode($entry); #_PC_
    return ("<a href='javascript:navToEntry(\"$entryE\");'>$entry</a>");
}

sub get_toc_link {
    # get a link for use in a toc page.
    my ($entry, $is_sparse, $class, $sidebar) = @_;

    my $entryE = _encode($entry); #_PC_
    my $contentTarget = $sidebar ? '_content' : 'content-container';

    if ($is_sparse) {
        return "<a class='$class' href='api-$entryE.html' " .
          "target='$contentTarget'>$entry</a>\n";
    } else {
        return "<a href='complete.html#$entryE' class='$class' " .
          "target='$contentTarget'>$entry</a>";
    }
}

sub get_menu {
    my $type = shift;
    my %menu = 
      ('full-sparse' => [['alphabetical listing' => 'sparse-toc-abc.html'],
			 ['grouped listing' => 'sparse-toc-grp.html']],
       'abc-sparse' => [['full listing' => 'sparse-toc.html'],
			['grouped listing' => 'sparse-toc-grp.html']],
       'grp-sparse' => [['full listing' => 'sparse-toc.html'],
			['alphabetical listing' => 'sparse-toc-abc.html']],
       'full-compl' => [['alphabetical listing' => 'complete-toc-abc.html'],
			 ['grouped listing' => 'complete-toc-grp.html']],
       'abc-compl' => [['full listing' => 'complete-toc.html'],
			['grouped listing' => 'complete-toc-grp.html']],
       'grp-compl' => [['full listing' => 'complete-toc.html'],
			['alphabetical listing' => 'complete-toc-abc.html']],
      );
    my $menu = $menu{$type};
    my $ret;

    foreach my $item (@$menu) {
	$ret .= "<a href='" . $$item[1] . "'>" . $$item[0] . "</a><br>\n";
    }

    return $ret;
}

sub init_files {
    # initialize the complete content and toc files.
    my $headstr = "<html><head><link rel=StyleSheet href='api-content.css' " .
      "TYPE='text/css' MEDIA='screen'><title>$apiid</title></head>" .
        $JS_COMPLETE . "<body bgcolor='white'>\n$WARNING" .
          "<h1 class='title'>$apiid Reference</h1>\n" . $user_head;

    print COMPLETE $headstr;

    my $tocstr1 = 
      ("<html><head><link rel=StyleSheet href='api-toc.css' " .
       "TYPE='text/css' MEDIA='screen'>" .
       "<title>$apiid table of contents</title>" .
       "</head>\n<body bgcolor='white'>\n$WARNING" .
       "<h1 class='title'>$apiid Reference</h1><h4>Table of Contents</h4>\n");
    my $tocstr2 = 
      ("<center><table class='toc-table' border='1' cellpadding='0' " .
       "cellspacing='0' width='100%'>\n");
    my $abcstr = 
      ("<tr class='toc-title'><th><br><h3>Alphabetical Listing</h3></th>" .
       "</tr>\n<tr><td>\n" .
       "<center><table class='toc-abc' border='0' cellspacing='0' " .
       "cellpadding='0' width='100%'>\n");

    my $sidebar1 = $tocstr1;
    $sidebar1 =~ s/api\-toc\.css/sidebar.css/;

    print COMPLETE_TOC $tocstr1;
    print COMPLETE_TOC &get_menu("full-compl");
    print COMPLETE_TOC $tocstr2;
    print COMPLETE_TOC $abcstr;

    print COMPLETE_TOC_ABC $tocstr1;
    print COMPLETE_TOC_ABC &get_menu("abc-compl");
    print COMPLETE_TOC_ABC $tocstr2;
    print COMPLETE_TOC_ABC $abcstr;

    print COMPLETE_TOC_GRP $tocstr1;
    print COMPLETE_TOC_GRP &get_menu("grp-compl");
    print COMPLETE_TOC_GRP $tocstr2;

    print SPARSE_TOC $tocstr1;
    print SPARSE_TOC &get_menu("full-sparse");
    print SPARSE_TOC $tocstr2;
    print SPARSE_TOC $abcstr;

    print SPARSE_TOC_ABC $tocstr1;
    print SPARSE_TOC_ABC &get_menu("abc-sparse");
    print SPARSE_TOC_ABC $tocstr2;
    print SPARSE_TOC_ABC $abcstr;

    print SPARSE_TOC_GRP $tocstr1;
    print SPARSE_TOC_GRP &get_menu("grp-sparse");
    print SPARSE_TOC_GRP $tocstr2;

    print SIDEBAR_TOC $sidebar1;
    #print SIDEBAR_TOC &get_menu("grp-sparse");
    print SIDEBAR_TOC $tocstr2;
}

sub close_toc {
    local (*G) = shift;
    my $menu = shift;
    my $sidebar = shift;

    print G "</table></center>\n";
    print G &get_menu($menu) . "<p>\n" unless $sidebar;
    print G $user_foot . "\n";
    print G $footstr;
    close G;

}

sub close_files {
    # finish up the complete content and toc files.

    print COMPLETE $user_foot;
    print COMPLETE $footstr;
    close COMPLETE;

    &close_toc(*COMPLETE_TOC, "full-compl");
    &close_toc(*COMPLETE_TOC_ABC, "abc-compl");
    &close_toc(*COMPLETE_TOC_GRP, "grp-compl");
    &close_toc(*SPARSE_TOC, "full-sparse");
    &close_toc(*SPARSE_TOC_ABC, "abc-sparse");
    &close_toc(*SPARSE_TOC_GRP, "grp-sparse");
    &close_toc(*SIDEBAR_TOC, "grp-sparse", 1);
}

sub add_leading_nbsp {
    # replaces leading spaces with &nbsp; entities.  Used for tags which
    # contain code.
    my ($str) = @_;
    my $i;
    my $pfx = "";

    if (!($str =~ /^(\s+)/)) {
        return $str;
    }

    my $len = length($1);
    for $i (0 .. $len) {
        $pfx .= "&nbsp;";
    }

    substr ($str, 0, $len) = $pfx;
    return $str;

}

sub debug_dump_c {

    print ("tag: $tagname, attrs: " .
           join (", ", keys (%pending_attrs)) .
           ", stacks: " . $#text_stack . ", " . $#tag_stack . ", " .
           $#attr_stack . "\n");

    for (0 ... $#attr_stack) {
        print ("attribs at $_: " . join (", ", keys (%{$attr_stack[$_]})) .
               "\n");
    }

}

sub debug_write_entry {
    my $i;

    for $i (keys(%{$c})) {
        my $str = "";
        if (($i eq $TAGS[$SEE_ALSO]) || ($i eq ($TAGS[$PARAM]))) {
            $str = join (", ", @{$c->{$i}});
        } else {
            $str = $c->{$i};
        }

        print ("$i : $str\n");
    }
    print ("===\n");
}

sub croak_attr {
    my ($expat, $tagname, $attr) = @_;

    $expat->xpcroak ("Tag $tagname needs an $attr attribute");
}

#_PC_ - subroutine to encode URLs
sub _encode {
    my ($entry) = @_;

    $entry =~ s/(\W)/sprintf("%%%x", ord($1))/eg;
    return $entry;
}
