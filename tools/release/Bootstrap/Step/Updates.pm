#
# Updates step. Generates binary update (MAR) files as well as AUS config
# snippets.
# 
package Bootstrap::Step::Updates;

use Bootstrap::Step;
use Bootstrap::Config;
use Bootstrap::Util qw(CvsCatfile GetLocaleManifest);

use File::Find qw(find);
use POSIX qw(strftime);

use MozBuild::Util qw(MkdirWithPath);

@ISA = ("Bootstrap::Step");

sub Execute {
    my $this = shift;

    my $config = new Bootstrap::Config();
    my $product = $config->Get(var => 'product');
    my $logDir = $config->Get(sysvar => 'logDir');
    my $oldVersion = $config->GetOldVersion(longName => 0);
    my $version = $config->GetVersion(longName => 0);
    my $mozillaCvsroot = $config->Get(var => 'mozillaCvsroot');
    my $updateDir = $config->Get(var => 'updateDir');
    my $patcherConfig = $config->Get(var => 'patcherConfig');
    my $patcherToolsRev = $config->Get(var => 'patcherToolsRev');

    my $versionedUpdateDir = catfile($updateDir, $product . '-' . $version);

    # Create updates area.
    if (not -d $versionedUpdateDir) {
        MkdirWithPath(dir => $versionedUpdateDir) 
          or die("Cannot mkdir $versionedUpdateDir: $!");
    }

    # check out patcher
    $this->CvsCo(cvsroot => $mozillaCvsroot,
                 checkoutDir => 'patcher',
                 modules => [CvsCatfile('mozilla', 'tools', 'patcher')],
                 tag => $patcherToolsRev,
                 logFile => catfile($logDir, 'updates_patcher-checkout.log'),
                 workDir => $versionedUpdateDir
    );
          
    # check out utilities
    $this->CvsCo(cvsroot => $mozillaCvsroot,
                 checkoutDir => 'MozBuild',
                 modules => [CvsCatfile('mozilla', 'tools', 'release',
                                        'MozBuild')],
                 tag => $patcherToolsRev,
                 logFile => catfile($logDir,
                                    'updates_patcher-utils-checkout.log'),
                 workDir => catfile($versionedUpdateDir, 'patcher')
    );

    # this config lives in the public repo since bug 408849 was checked in
    $this->CvsCo(cvsroot => $mozillaCvsroot,
                 checkoutDir => 'config',
                 modules => [CvsCatfile('mozilla', 'tools', 'patcher-configs',
                                        $patcherConfig)],
                 logFile => catfile($logDir,
                                    'updates_patcher-config-checkout.log'),
                 workDir => $versionedUpdateDir
    );

    # build tools
    my $originalCvsrootEnv = $ENV{'CVSROOT'};
    $ENV{'CVSROOT'} = $mozillaCvsroot;
    $this->Shell(
      cmd => './patcher2.pl',
      cmdArgs => ['--build-tools', '--tools-revision=' . $patcherToolsRev,
                  '--app=' . $product, 
                  '--config=../config/' . $patcherConfig],
      logFile => catfile($logDir, 'updates_patcher-build-tools.log'),
      dir => catfile($versionedUpdateDir, 'patcher'),
    );
    if ($originalCvsrootEnv) {
        $ENV{'CVSROOT'} = $originalCvsrootEnv;
    }
    
    # download complete MARs
    $this->Shell(
      cmd => './patcher2.pl',
      cmdArgs => ['--download', '--app=' . $product,
                    '--config=../config/' . $patcherConfig],
      logFile => catfile($logDir, 'updates_patcher-download.log'),
      dir => catfile($versionedUpdateDir, 'patcher'),
    );

    # Create partial patches and snippets
    $this->Shell(
      cmd => './patcher2.pl',
      cmdArgs => ['--create-patches', '--app=' . $product, 
                  '--config=../config/' . $patcherConfig,
                  '--partial-patchlist-file=patchlist.cfg'],
      logFile => catfile($logDir, 'updates_patcher-create-patches.log'),
      dir => catfile($versionedUpdateDir, 'patcher'),
      timeout => 18000,
    );
    
    ### quick verification
    my $fullUpdateDir = catfile($versionedUpdateDir, 'patcher', 'temp', 
                          $product, $oldVersion . '-' . $version);
    $snippetErrors = undef;   # evil (??) global to get results from callbacks
    
    # ensure that there are only test channels in aus2.test dir
    File::Find::find(\&TestAusCallback, catfile($fullUpdateDir,"aus2.test"));

    # ensure that there are only beta channels in beta dir (if that exists)
    if (-d catfile($fullUpdateDir, "aus2.beta")) {
      File::Find::find(\&BetaAusCallback, catfile($fullUpdateDir,"aus2.beta"));
      File::Find::find(\&ReleaseAusCallback, catfile($fullUpdateDir,"aus2"));
    } 
    # otherwise allow beta and release in aus2 dir
    else {
      File::Find::find(\&ReleaseBetaAusCallback, catfile($fullUpdateDir,"aus2"));
    }    

    if ($snippetErrors) {
        $snippetErrors =~ s!$fullUpdateDir/!!g;
	die("Execute: Snippets failed location checks: $snippetErrors\n");
    }
}

sub Verify {
    my $this = shift;

    my $config = new Bootstrap::Config();
    my $logDir = $config->Get(sysvar => 'logDir');
    my $hgToolsRepo = $config->Get(var => 'hgToolsRepo');
    my $verifyDir = $config->Get(var => 'verifyDir');
    my $osname = $config->SystemInfo(var => 'osname');
    my $product = $config->Get(var => 'product');
    my $version = $config->GetVersion(longName => 0);
    my $verifyConfig = $config->Get(sysvar => 'verifyConfig');

    # Create verification area.
    my $verifyDirVersion = catfile($verifyDir, $product . '-' . $version);
    MkdirWithPath(dir => $verifyDirVersion) 
      or die("Could not mkdir $verifyDirVersion: $!");

    $this->HgClone(
      repo => $hgToolsRepo,
      workDir => $verifyDirVersion
    );

    my $verifyLog = catfile($logDir, 'updates_verify.log');
    $this->Shell(
      cmd => './verify.sh', 
      cmdArgs => ['-c', $verifyConfig],
      logFile => $verifyLog,
      dir => catfile($verifyDirVersion, 'tools', 'release', 'updates'),
      timeout => 36000,
    );

    $this->CheckLog(
        log => $verifyLog,
        notAllowed => '^FAIL',
    );
}

# locate snippets for which the channel doesn't end in test
sub TestAusCallback { 
    my $dir = $File::Find::name;
    if ( ($dir =~ /\.txt/) and 
         (not $dir =~ /\/\w*test\/(partial|complete)\.txt$/)) {
           $snippetErrors .= "\nNon-test: $dir";
    }
}

# locate snippets for which the channel isn't beta
sub BetaAusCallback { 
    my $dir = $File::Find::name;
    if ( ($dir =~ /\.txt/) and 
         (not $dir =~ /\/beta\/(partial|complete)\.txt$/)) {
           $snippetErrors .= "\nNon-beta: $dir";
    }
}

# locate snippets for which the channel isn't release
sub ReleaseAusCallback { 
    my $dir = $File::Find::name;
    if ( ($dir =~ /\.txt/) and 
         (not $dir =~ /\/release\/(partial|complete)\.txt$/)) {
           $snippetErrors .= "\nNon-release: $dir";
    }
}

# locate snippets for which the channel isn't release or beta
sub ReleaseBetaAusCallback { 
    my $dir = $File::Find::name;
    if ( ($dir =~ /\.txt/) and 
         (not $dir =~ /\/(release|beta)\/(partial|complete)\.txt$/)) {
           $snippetErrors .= "\nNon-release: $dir";
    }
}

sub PermissionsAusCallback {
    my $dir = $File::Find::name;

    if (-f $dir) {
	chmod(0644, $dir) or die("Couldn't chmod $dir to 644: $!");
    } elsif (-d $dir) {
	chmod(0775, $dir) or die("Couldn't chmod $dir to 775: $!");
    }
}

sub Push {
    my $this = shift;

    my $config = new Bootstrap::Config();
    my $logDir = $config->Get(sysvar => 'logDir');
    my $product = $config->Get(var => 'product');
    my $version = $config->GetVersion(longName => 0);
    my $build = $config->Get(var => 'build');
    my $oldVersion = $config->GetOldVersion(longName => 0);
    my $stagingUser = $config->Get(var => 'stagingUser');
    my $stagingServer = $config->Get(var => 'stagingServer');
    my $ausUser = $config->Get(var => 'ausUser');
    my $ausServer = $config->Get(var => 'ausServer');
    my $updateDir = $config->Get(var => 'updateDir');

    my $pushLog = catfile($logDir, 'updates_push.log');
    my $fullUpdateDir = catfile($updateDir, $product . '-' . $version,
                                 'patcher', 'temp', $product, 
                                  $oldVersion . '-' . $version);
    my $candidateDir = $config->GetFtpCandidateDir(bitsUnsigned => 0);

    # push partial mar files up to ftp server
    my $marsDir = catfile('ftp', $product, 'nightly', 
                            $version . '-candidates', 'build' . $build) . '/';

    chmod(0644, glob(catfile($fullUpdateDir,$marsDir,"*partial.mar")))
	or die("Couldn't chmod a partial mar to 644: $!");
    $this->Shell(
     cmd => 'rsync',
     cmdArgs => ['-av', '-e', 'ssh',
                 '--include=*partial.mar', 
                 '--exclude=*',
                 $marsDir,
                 $stagingUser . '@' . $stagingServer . ':' . $candidateDir],
     dir => $fullUpdateDir,
     logFile => $pushLog,
   );

    # push update snippets to AUS server
    my $pushDir = strftime("%Y%m%d", localtime) . '-' . ucfirst($product) . 
                           '-' . $version;
    my $targetPrefix =  CvsCatfile('/opt','aus2','snippets','staging',
                                   $pushDir);
    $config->Set(var => 'ausDeliveryDir', value => $targetPrefix);

    my @snippetDirs = glob(catfile($fullUpdateDir, "aus2*"));

    File::Find::find(\&PermissionsAusCallback, @snippetDirs);

    foreach $dir (@snippetDirs) {
      my $targetDir = $targetPrefix;
      if ($dir =~ /aus2\.(.*)$/) {
        $targetDir .= '-' . $1;
      }

      $this->Shell(
        cmd => 'rsync',
        cmdArgs => ['-av', 
                    '-e', 'ssh -i ' . catfile($ENV{'HOME'},'.ssh','aus'),
                    $dir . '/', 
                    $ausUser . '@' . $ausServer . ':' . $targetDir],
        logFile => $pushLog,
      );
    }

    # Backup test channels
    $this->Shell(
      cmd => 'ssh', 
      cmdArgs => ['-i ' . catfile($ENV{'HOME'},'.ssh','aus'),
                  $ausUser . '@' . $ausServer,
                  '/home/cltbld/bin/backupsnip', $pushDir . '-test'],
      logFile => $pushLog,
      # the 2.x -> 3.x major update generated a *lot* of snippets
      # backupsnip now takes significantly more time to run
      timeout => 7200
    );
    # Push test channels live
    $this->Shell(
      cmd => 'ssh', 
      cmdArgs => ['-i ' . catfile($ENV{'HOME'},'.ssh','aus'),
                  $ausUser . '@' . $ausServer,
                  '/home/cltbld/bin/pushsnip', $pushDir . '-test'],
      logFile => $pushLog,
    );
    # Wait for timeout on AUS's NFS caching to expire before
    # attempting to test newly-pushed snippets
    sleep(360);
}

sub Announce {
    my $this = shift;

    my $config = new Bootstrap::Config();
    my $product = $config->Get(var => 'product');
    my $version = $config->GetVersion(longName => 0);

    $this->SendAnnouncement(
      subject => "$product $version update step finished",
      message => "$product $version updates finished. Partial mars were copied to the candidates dir, and the test snippets were pushed live.",
    );
}

1;
