#!/usr/bin/perl -wT
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
# The Original Code is the Bugzilla Test Runner System.
#
# The Initial Developer of the Original Code is Maciej Maczynski.
# Portions created by Maciej Maczynski are Copyright (C) 2001
# Maciej Maczynski. All Rights Reserved.
#
# Contributor(s): Greg Hendricks <ghendricks@novell.com>

use strict;
use lib ".";

use Bugzilla;
use Bugzilla::Constants;
use Bugzilla::Error;
use Bugzilla::Util;
use Bugzilla::User;
use Bugzilla::Version;
use Bugzilla::Testopia::Util;
use Bugzilla::Testopia::TestRun;
use Bugzilla::Testopia::TestCaseRun;
use Bugzilla::Testopia::TestTag;
use Bugzilla::Testopia::Environment;
use Bugzilla::Testopia::Search;
use Bugzilla::Testopia::Table;
use Bugzilla::Testopia::Product;

local our $vars = {};
local our $template = Bugzilla->template;
local our $query_limit = 15000;

Bugzilla->login(LOGIN_REQUIRED);
   
my $dbh = Bugzilla->dbh;
local our $cgi = Bugzilla->cgi;

local our $run_id = trim($cgi->param('run_id') || '');

unless ($run_id){
  print $cgi->header;
  $template->process("testopia/run/choose.html.tmpl", $vars) 
      || ThrowTemplateError($template->error());
  exit;
}
validate_test_id($run_id, 'run');
my $serverpush = support_server_push($cgi);

local our $action = $cgi->param('action') || '';

####################
### Edit Actions ###
####################
if ($action eq 'Commit'){
    print $cgi->header;
    my $run = Bugzilla::Testopia::TestRun->new($run_id);
    ThrowUserError("testopia-read-only", {'object' => $run}) unless $run->canedit;
    do_update($run);
    $vars->{'tr_message'} = "Test run updated";
    $vars->{'backlink'} = $run;
    display($run);    
}

elsif ($action eq 'History'){
    print $cgi->header;
    my $run = Bugzilla::Testopia::TestRun->new($run_id);
    ThrowUserError("testopia-permission-denied", {'object' => $run}) unless $run->canview;
    $vars->{'run'} = $run; 
    $template->process("testopia/run/history.html.tmpl", $vars)
      || ThrowTemplateError($template->error());
       
}

#############
### Clone ###
#############
elsif ($action =~ /^Clone/){
    print $cgi->header;
    my $run = Bugzilla::Testopia::TestRun->new($run_id);
    ThrowUserError("testopia-read-only", {'object' => $run->plan}) unless $run->plan->canedit;
    my $case_list = $cgi->param('case_list');
    do_update($run);
    my @ids;
    foreach my $id (split(",", $case_list)){
        detaint_natural($id);
        push @ids, $id;
    }
    
    my $ref;
    if ($case_list){ 
        $ref = $dbh->selectcol_arrayref(
            "SELECT DISTINCT case_id 
               FROM test_case_runs
              WHERE case_run_id IN (" . join(",",@ids) . ")");
    }
    
    $vars->{'product'} = Bugzilla::Testopia::Product->new($run->plan->product_id);
    $vars->{'run'} = $run;
    $vars->{'case_list'} = join(",", @$ref) if ($action =~/These Cases/ && $ref);
    $vars->{'caserun'} = Bugzilla::Testopia::TestCaseRun->new({'case_run_id' => 0});
    $template->process("testopia/run/clone.html.tmpl", $vars) 
      || ThrowTemplateError($template->error());
    
}
elsif ($action eq 'do_clone'){
    my $run = Bugzilla::Testopia::TestRun->new($run_id);
    if ($serverpush) {
        print $cgi->multipart_init();
        print $cgi->multipart_start();

        # Under mod_perl, flush stdout so that the page actually shows up.
        if ($ENV{MOD_PERL}) {
            require Apache2::RequestUtil;
            Apache2::RequestUtil->request->rflush();
        }
    
        $template->process("list/server-push.html.tmpl", $vars)
          || ThrowTemplateError($template->error());

    }
    
    ThrowUserError("testopia-read-only", {'object' => $run->plan}) unless $run->plan->canedit;
    my $summary = $cgi->param('summary');
    my $build = $cgi->param('build');
    my $plan_id = $cgi->param('plan_id');
    trick_taint($summary);
    detaint_natural($build);
    validate_test_id($plan_id, 'plan');
    my $manager = $cgi->param('keepauthor') ? $run->manager->id : Bugzilla->user->id;
    my $newrun = Bugzilla::Testopia::TestRun->new($run->clone($summary, $manager, $plan_id, $build));

    if($cgi->param('copy_tags')){
        foreach my $tag (@{$run->tags}){
            $newrun->add_tag($tag->name);
        }
    }
    my $progress_interval = 250;
        
    if ($cgi->param('case_list')){
        my @case_ids;
        foreach my $id (split(",", $cgi->param('case_list'))){
            
            detaint_natural($id);
            my $case = Bugzilla::Testopia::TestCase->new($id);
            unless ($case->canview){
                print $cgi->multipart_end if $serverpush;
                ThrowUserError('testopia-permission-denied', {'object' => $case});
            }
            push @case_ids, $id
        }

        my $i = 0;
        my $total = scalar @case_ids;

        foreach my $id (@case_ids){
            $i++;
            if ($i % $progress_interval == 0 && $serverpush){
                print $cgi->multipart_end;
                print $cgi->multipart_start;
                $vars->{'complete'} = $i;
                $vars->{'total'} = $total;
                $template->process("testopia/progress.html.tmpl", $vars)
                  || ThrowTemplateError($template->error());
            } 
            
            $newrun->add_case_run($id);
        }
    }
    if ($cgi->param('copy_test_cases')){
        if ($cgi->param('status')){
            my @status = $cgi->param('status');
            foreach my $s (@status){
                detaint_natural($s);
            }
            my $ref = $dbh->selectcol_arrayref(
                "SELECT case_id
                   FROM test_case_runs
                  WHERE run_id = ?
                    AND case_run_status_id IN (". join(",", @status) .")
                    AND iscurrent = 1", undef, $run->id);

            my $i = 0;
            my $total = scalar @$ref;

            foreach my $case_id (@{$ref}){
                $i++;
                if ($i % $progress_interval == 0 && $serverpush){
                    print $cgi->multipart_end;
                    print $cgi->multipart_start;
                    $vars->{'complete'} = $i;
                    $vars->{'total'} = $total;
                    $template->process("testopia/progress.html.tmpl", $vars)
                      || ThrowTemplateError($template->error());
                } 

                $newrun->add_case_run($case_id);
            }
        }
        else {
            my $i = 0;
            my $total = scalar @{$run->cases};
            
            foreach my $case (@{$run->cases}){
                $i++;
                if ($i % $progress_interval == 0 && $serverpush){
                    print $cgi->multipart_end;
                    print $cgi->multipart_start;
                    $vars->{'complete'} = $i;
                    $vars->{'total'} = $total;
                    $template->process("testopia/progress.html.tmpl", $vars)
                      || ThrowTemplateError($template->error());
                }
                 
                $newrun->add_case_run($case->id);
            }
        }
    }
    if ($serverpush) {
        print $cgi->multipart_end;
        print $cgi->multipart_start;
    } else {
        print $cgi->header;
    }
    
    $cgi->delete_all;
    $cgi->param('run_id', $newrun->id);
    $vars->{'tr_message'} = "Test run cloned";
    $vars->{'backlink'} = $run;
    display($newrun);
    print $cgi->multipart_final if $serverpush; 
}
elsif ($action eq 'Filter'){
    $cgi->send_cookie(-name => 'TESTOPIA-FILTER-RUN-' . $run_id,
                      -value => $cgi->canonicalise_query('run_id'),
                      -expires => 'Fri, 01-Jan-2038 00:00:00 GMT');
    
    $vars->{'filtered'} = 1;
    print $cgi->header;
    display(Bugzilla::Testopia::TestRun->new($run_id));
    
}
elsif ($action eq 'clear_filter'){
    $cgi->remove_cookie('TESTOPIA-FILTER-RUN-' . $run_id);
    $vars->{'filtered'} = 0;    
    print $cgi->header;
    display(Bugzilla::Testopia::TestRun->new($run_id));

}

####################
### Ajax Actions ###
####################
elsif ($action eq 'addcc'){
    my $run = Bugzilla::Testopia::TestRun->new($run_id);
    ThrowUserError("testopia-read-only", {'object' => $run}) unless $run->canedit;
    my @cclist = split(/[\s,]+/, $cgi->param('cc'));
    my %ccids;
    foreach my $email (@cclist){
        my $ccid = login_to_id($email) || ThrowUserError("invalid_username", { name => $email });
        trick_taint($ccid);
        if ($ccid && !$ccids{$ccid}) {
           $ccids{$ccid} = 1;
        }
    }
    foreach my $ccid (keys(%ccids)) {
        $run->add_cc($ccid);
    }
    my $cc = get_cc_xml($run);
    print $cgi->header;
    print $cc;
}
elsif ($action eq 'removecc'){
    my $run = Bugzilla::Testopia::TestRun->new($run_id);
    ThrowUserError('insufficient-case-perms') unless $run->canedit;
    foreach my $ccid (split(",", $cgi->param('cc'))){
        detaint_natural($ccid);
        $run->remove_cc($ccid);
    }
    my $cc = get_cc_xml($run);
    print $cgi->header;
    print $cc;
}
elsif ($action eq 'Delete'){
    print $cgi->header;
    my $run = Bugzilla::Testopia::TestRun->new($run_id);
    ThrowUserError("testopia-no-delete", {'object' => $run}) unless $run->candelete;
    $vars->{'run'} = $run;
    
    $template->process("testopia/run/delete.html.tmpl", $vars) ||
        ThrowTemplateError($template->error());
    
}
elsif ($action eq 'do_delete'){
    my $run = Bugzilla::Testopia::TestRun->new($run_id);
    unless ($run->candelete){
        print $cgi->header;
        ThrowUserError("testopia-no-delete", {'object' => $run});
    }
    if ($serverpush) {
        print $cgi->multipart_init();
        print $cgi->multipart_start();
        $vars->{'complete'} = 1;
        $vars->{'total'} = 250;
        $template->process("testopia/progress.html.tmpl", $vars)
          || ThrowTemplateError($template->error());

        $run->obliterate($cgi,$template);
    }
    else{
        $run->obliterate;
    }
    if ($serverpush) {
        print $cgi->multipart_end;
        print $cgi->multipart_start;
    } else {
        print $cgi->header;
    }
    
    $vars->{'deleted'} = 1;
    $template->process("testopia/run/delete.html.tmpl", $vars) ||
        ThrowTemplateError($template->error());
    print $cgi->multipart_final if $serverpush;
}

####################
### Just show it ###
####################
else {
    print $cgi->header;
    display(Bugzilla::Testopia::TestRun->new($run_id));
}
###################
### Helper Subs ###
###################
#TODO: Replace this with json
sub get_cc_xml {
    my ($run) = @_;
    my $ret = "<cclist>";
    foreach my $c (@{$run->cc}){
        $ret .= "<user>";
        $ret .= "<id>". $c->id ."</id>";
        $ret .= "<name>". $c->login ."</name>";
        $ret .= "</user>";
    }
    $ret .= "</cclist>";
    return $ret;
} 

sub do_update {
    my ($run) = @_;

    my $timestamp;
    $timestamp = $run->stop_date;
    $timestamp = undef if $cgi->param('status');
    $timestamp = get_time_stamp() if $cgi->param('status') == 0 && !$run->stop_date;
 
    $run->set_summary($cgi->param('summary'));
    $run->set_product_version($cgi->param('product_version'));
    $run->set_plan_text_version($cgi->param('plan_version'));
    $run->set_build($cgi->param('build'));
    $run->set_environment($cgi->param('environment'));
    $run->set_manager($cgi->param('manager'));
    $run->set_notes($cgi->param('notes'));
    $run->set_stop_date($timestamp);
    
    $run->update();
    
    # Add new tags
    $run->add_tag($cgi->param('newtag')); 

    $cgi->delete_all;
    $cgi->param('run_id', $run->id);
}

sub display {
    my $run = shift;
    ThrowUserError("testopia-permission-denied", {'object' => $run}) unless $run->canview;
    # See if there is a saved filter
    if ($cgi->cookie('TESTOPIA-FILTER-RUN-' . $run_id) && $action ne 'Filter' && $action ne 'clear_filter'){
        $cgi = Bugzilla::CGI->new($cgi->cookie('TESTOPIA-FILTER-RUN-' . $run_id));
        $cgi->param('run_id', $run_id);
        $vars->{'filtered'} = 1;
        
    }

    $cgi->param('current_tab', 'case_run');
    my $search = Bugzilla::Testopia::Search->new($cgi);
    my $table = Bugzilla::Testopia::Table->new('case_run', 'tr_show_run.cgi', $cgi, undef, $search->query);
    ThrowUserError('testopia-query-too-large', {'limit' => $query_limit}) if $table->view_count > $query_limit;
    
    my $case = Bugzilla::Testopia::TestCase->new({'case_id' => 0});
    $vars->{'fullwidth'} = 1;
    $vars->{'expand_report'} = $cgi->param('expand_report') || 0;
    $vars->{'expand_filter'} = $cgi->param('expand_filter') || 0;
    $vars->{'caserun'} = Bugzilla::Testopia::TestCaseRun->new({});
    $vars->{'case'} = Bugzilla::Testopia::TestCase->new({});
    $vars->{'run'} = $run;
    $vars->{'table'} = $table;
    $vars->{'action'} = 'Commit';
    if ($cgi->param('case_id')){
        $vars->{'expand_filter'} = 1;
        $vars->{'filtered'} = 1;
    }
    $template->process("testopia/run/show.html.tmpl", $vars) ||
        ThrowTemplateError($template->error());
}
