#!/usr/bin/perl

use strict;
use warnings;
use CGI qw/:standard/;

# File: uploadpatch.cgi
# Author: Ben Hearsum
# Description:
#   This cgi script displays a simple form that allows a user to submit a diff
#   that will eventually be uploaded to a Buildbot installation.
#   Along with the patch this script generates a .info file that contains the
#   name of the submitter, as read from $ENV['REMOTE_USER'], the date, in unix
#   time, the name of the patchfile, and the description read from the form.
#   This information is used by the download script to generate the
#   'buildbot sendchange' command.


# where the patches will go after being submitted
my $PATCH_DIR = '/buildbot/patches';
# the size limit for the file, in bytes
# 10*1024*1024 is 10MB
my $SIZE_LIMIT = 10*1024*1024;
# the URL to the buildbot installation the patches will eventually go to
my $BUILDBOT_URL = 'http://localhost:8010';
# the URL to the uploadpatch.cgi script
my $UPLOADPATCH_URL = 'http://localhost/cgi-bin/uploadpatch.cgi';

$CGI::POST_MAX = $SIZE_LIMIT;

sub WriteSuccessPage
{
    print "Content-type: text/html\n\n";
    print <<__END_OF_HTML__;
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
          "http://www.w3.org/TR/html4/strict.dtd">
<html lang="en">

<head><title>Patch uploaded successfully</title></head>

<body>
<h3 style="text-align: center">Patch Uploaded Successfully</h3>
<div style="text-align: center">
  Look for your patch <a href="$BUILDBOT_URL">here</a>
</div>
</body>
</html>
__END_OF_HTML__

}

sub WritePage
{
    my %args = @_;
    my $description = $args{'description'};
    my $err = $args{'err'};

    my $limit = $SIZE_LIMIT / 1024;

    print "Content-type: text/html\n\n";
    print <<__END_OF_HTML__;
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
          "http://www.w3.org/TR/html4/strict.dtd">
<html lang="en">

<head>
<title>Upload a Patch to Buildbot</title>
<style type="text/css">
  body {
    margin-left: auto;
    margin-right: auto;
  }

  div {
    text-align: center;
  }

  p {
    text-align: center;
    color: red;
    font-weight: bold;
  }

  table {
    width: 35%;
    margin-left: auto;
    margin-right: auto;
  }

  td {
    margin-top: 2px;
    margin-bottom: 2px;
    border: none;
    width: 50%;
  }

  tr#title {
    padding-bottom: 6px;
  }

  th {
    text-align: center;
  }

  td.lbl {
    text-align: right;
    vertical-align: top;
    padding-top: 5px;
  }

  td.field {
    text-align: left;
  }
</style>
</head>

<body>

<form action="$UPLOADPATCH_URL" method="post"
      enctype="multipart/form-data">
<div>
<table>
  <tr id="title">
    <th colspan="2">Upload a patch to Buildbot</th>
  </tr>
  <tr>
    <td class="lbl">Description:</td>
    <td class="field">
      <textarea name="description" cols="35" rows="6">$description</textarea>
    </td>
  </tr>
  <tr>
    <td class="lbl">File:</td>
    <td class="field">
      <input type="file" name="patchfile">
    </td>
  </tr>
  <tr>
    <td colspan="2" style="text-align: center">
      <input type="submit" />
    </td>
  </tr>
</table>
</div>

</form>
<p>Note: Uploaded files must be less than ${limit}kB</p>
</body>
</html>
__END_OF_HTML__

    if ($err) {
        print "<p>$err</p>";
    }
    print "</body>\n</html>";
}

sub Process
{
    my $time = time();
    # get the parameters
    my $name = '';
    $name = $ENV{'REMOTE_USER'};
    if ($name eq "") {
        $name = 'Noname';
    }

    my $description = param('description');
    my $patchFile = param('patchfile');

    if ($description eq '') {
        $description = 'No description given.';
    } else {
        if ($description =~ /([`]|\$\()/) {
            WritePage(description => $description,
                      err => 'Description must not contain backticks or "\$("');
            return;
        }
        $description =~ s/\n//g;
    }

    # only allow alphanumeric, hyphens, and single dots
    if ($patchFile !~ /^([\w-]|\.[\w-])+$/) {
        # ends the script
        
        WritePage(description => $description,
                  err => 'Invalid filename. Please use only alphanumeric, -, _,'
                   . ' and single dots');
        return;
    }

    # if we get here the file is small enough and passes the filename test

    # pull all of the contents of the file
    my $patchHandle = upload('patchfile');

    # strip off everything except the filename itself
    $patchFile =~ s/.*[\/\\](.*)/$1/;

    # generate the filenames
    $patchFile = "$time-$patchFile";
    my $infoFile = "$patchFile.info";

    # make sure the file has a non-zero length
    # this also handles a case where the file specified doesn't exist
    if (-z $patchHandle) {
        WritePage(description => $description,
                  err => 'Specified file has a length of zero...');
        return;
    }

    # write the patch
    my $filename = "$PATCH_DIR/$patchFile";
    open(PATCH, ">$filename") or WritePage($description,
      'Server error - Could not open file for writing.')
      and return;
    binmode PATCH;
    
    while (<$patchHandle>) {
      print PATCH;
    }
    close PATCH;

    # now write the infofile
    $filename = "$PATCH_DIR/$infoFile";

    open(INFO, ">$filename");

    print INFO "submitter: $name\n";
    print INFO "date: $time\n";
    print INFO "patchfile: $patchFile\n";
    print INFO "description: $description\n";

    close(INFO);

    WriteSuccessPage();
}

if (param()) {
    Process();
} else {
    WritePage();
}
