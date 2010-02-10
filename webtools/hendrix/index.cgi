#!/usr/bin/perl -wT
# -*- Mode: perl; indent-tabs-mode: nil -*-

# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Original Code is the Hendrix Feedback System.
#
# The Initial Developer of the Original Code is
# Gervase Markham.
# Portions created by the Initial Developer are Copyright (C) 2004
# the Initial Developer. All Rights Reserved.
#
# Contributor(s): Reed Loden <reed@reedloden.com>
#
# The Initial Developer wrote this software to the Glory of God.
# ***** END LICENSE BLOCK *****

use strict;

# This application requires installation of the "Email::Send" (note: not 
# Mail::Send) module.
use Template;
use CGI;
use Email::Send;
use Net::RBLClient;
use Captcha::reCAPTCHA;

# use CGI::Carp qw(fatalsToBrowser);

# If you are using Hendrix on your site, you'll need new API keys.
# Get them at http://recaptcha.net/api/getkey . If these variables contain
# keys, they won't work for you anyway.
my $RECAPTCHA_PUBLIC  = "6LckBgMAAAAAAHlkAGWVrdc3gorusTVlLjixmeFh";
my $RECAPTCHA_PRIVATE = "6LckBgMAAAAAACSa3zXC9K3_TU2KaDsLyt9UXSW2";

# Map products to destination
# When updating these two lists, don't forget the browser detection code in
# index.html.tmpl.
my %product_destination_map = (
	"Firefox"                       => "mozilla.feedback.firefox",
	"Firefox Release Candidate"     => "mozilla.feedback.firefox.prerelease",
	"Firefox (Mobile)"              => "mozilla.feedback.firefox.mobile",	
  "Firefox (Mobile) Release Candidate" => 
                                  "mozilla.feedback.firefox.mobile.prerelease",
	"Leibnitz"                      => "mozilla.feedback.firefox.prerelease",
	"Minefield"                     => "mozilla.feedback",
	"Fennec"                        => "mozilla.feedback",
	"Thunderbird"                   => "mozilla.feedback.thunderbird",
	"Thunderbird Release Candidate" => "mozilla.feedback.thunderbird.prerelease",
	"Shredder"                      => "mozilla.feedback",
	"SeaMonkey"                     => "mozilla.feedback",
	"Sunbird"                       => "mozilla.feedback",
	"Camino"                        => "caminofeedback\@mozilla.org",
	"eBay Companion"                => "mozilla.feedback.companion.ebay",
	"Developer Center"              => "mozilla.dev.mdc.feedback",
	"Other"                         => "mozilla.feedback"
);

# List of products to show on the main Hendrix page (in order)
my @products_list = (
	"Firefox", "Firefox Release Candidate", 
  "Firefox (Mobile)", "Firefox (Mobile) Release Candidate", 
  "Thunderbird", "Thunderbird Release Candidate", 
  "Lorentz", "Minefield", "Fennec",
  "Camino", "SeaMonkey", "Sunbird", 
	"Developer Center", "Other"
);

# The default newsgroup if the product isn't in the above map (NNTP only)
my $default_newsgroup = $::ENV{'HENDRIX_NEWSGROUP'} || "mozilla.feedback";

# The news (NNTP) server to use for posting
my $nntp_server = $::ENV{'HENDRIX_NNTP_SERVER'} || "news.mozilla.org";

# The default sender to use if no e-mail address is provided
my $default_sender = $::ENV{'HENDRIX_SENDER'} || "hendrix-no-reply\@mozilla.org";

# The mail (SMTP) server to use for posting
my $smtp_server = $::ENV{'HENDRIX_SMTP_SERVER'} || "smtp.mozilla.org";

# The CSS file to include in the template
my $skin = $::ENV{'HENDRIX_SKIN'} || "skin/planet.css";

# The DNS blacklists by which to check sender IP addresses
my $rbl = Net::RBLClient->new(
    lists => [
        'dnsbl.ahbl.org',
        'http.dnsbl.sorbs.net',
        'socks.dnsbl.sorbs.net',
        'misc.dnsbl.sorbs.net',
    ],
    query_txt => 1
);

my $cgi = new CGI;
my $form = $cgi->Vars;
my $vars;
$vars->{'form'} = $form;
$vars->{'products'} = \@products_list;
$vars->{'default_product'} = $form->{'product'};
$vars->{'stylesheet'} = $skin;
$vars->{'referer'} = $::ENV{'HTTP_REFERER'};
$vars->{'recaptcha_public_key'} = $RECAPTCHA_PUBLIC;

my $template = Template->new({
    INCLUDE_PATH => ["template"],
    PRE_CHOMP => 1,
    TRIM => 1,
    FILTERS => {
        email => \&emailFilter,
        remove_newlines => \&removeNewlinesFilter,
    },
}) || die("Template creation failed.\n");

my $action = $cgi->param("action");

if (!$action) {
    # If no action, show the submission form
    print "Content-Type: text/html;charset=UTF-8\n\n";
    $template->process("index.html.tmpl", $vars)
      || die("Template process failed: " . $template->error() . "\n");
}
elsif ($action eq "submit") {  
    # Check the poster's IP against some blacklists
    $rbl->lookup($::ENV{REMOTE_ADDR});
    my %rbl_results = $rbl->txt_hash();
    if (scalar(keys %rbl_results) > 0) {
      $vars->{'rbl_results'} = \%rbl_results;
      throwError("rbl_hit");
    }

    # Check the CAPTCHA
    # throwError("captcha_error");
    # XXX Currently we allow posting if CAPTCHA is not present. This is
    # a temporary measure so we don't have to have a flag day to update
    # all the different places which feed into Hendrix. We can then come
    # back and change this to throw an error for a bogus CAPTCHA.
    if ($form->{'recaptcha_response_field'} ||
        $form->{'recaptcha_challenge_field'}) 
    {
      my $c = Captcha::reCAPTCHA->new;

      my $result = $c->check_answer($RECAPTCHA_PRIVATE, 
                                    $ENV{'REMOTE_ADDR'},
                                    $form->{'recaptcha_challenge_field'}, 
                                    $form->{'recaptcha_response_field'});

      if (!$result->{is_valid}) {
        throwError("captcha_error", $result->{error});
      }
    }
    
    # Format the parameters and send to the newsgroup.
    
    # Check for compulsory parameters
    if (!$form->{'subject'} || !$form->{'product'}) {
      throwError("bad_parameters");
    }

    $vars->{'destination'} = $product_destination_map{$form->{'product'}} || $default_newsgroup;
    $vars->{'method'} = $vars->{'destination'} =~ /@/ ? "SMTP" : "NNTP";
    $vars->{'email'} = $form->{'email'} =~ /d:^[\w\.\+\-=]+@[\w\.\-]+\.[\w\-]+$:/ ? $form->{'email'} : $default_sender;

    my $message;
    my $headers;
    
    $template->process("message-headers.txt.tmpl", $vars, \$headers)
      || die("Template process failed: " . $template->error() . "\n");
    $template->process("message.txt.tmpl", $vars, \$message)
      || die("Template process failed: " . $template->error() . "\n");

    # Post formatted message to newsgroup/email
    my $theMsg = Email::Simple->new($headers . "\n\n" . $message);
    my $sender = Email::Send->new({
        mailer      => $vars->{'method'},
        mailer_args => [ Host => $vars->{'method'} eq "SMTP" ? $smtp_server : $nntp_server ],
    });
    my $success = $sender->send($theMsg);

    # Give user feedback on success/failure
    throwError("cant_post") if (!$success);
    
    $vars->{'headers'} = $headers;
    $vars->{'message'} = $message;

    print "Content-Type: text/html;charset=UTF-8\n\n";
    $template->process("submit-successful.html.tmpl", $vars)
      || die("Template process failed: " . $template->error() . "\n");
}
else {
    die("Unknown action $action\n");
}

exit;

# Simple email obfuscation
sub emailFilter {
    my ($var) = @_;
    $var =~ s/\@/at/;
    $var =~ s/\./dot/g;
    return $var;
}

# Remove newlines and replace them with spaces
sub removeNewlinesFilter {
    my ($var) = @_;
    $var =~ s/\r\n/ /g;
    $var =~ s/\n\r/ /g;
    $var =~ s/\r/ /g;
    $var =~ s/\n/ /g;
    return $var;
}

sub throwError {
    my ($error, $info) = @_;
    $vars->{'error'} = $error;
    $vars->{'info'} = $info || "";
    
    print "Content-Type: text/html;charset=UTF-8\n\n";
    $template->process("error.html.tmpl", $vars)
      || die("Template process failed: " . $template->error() . "\n");
    
    exit;
}
