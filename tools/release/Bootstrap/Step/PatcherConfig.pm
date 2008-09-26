#
# Updates step. Configures patcher files.
# 
# 
package Bootstrap::Step::PatcherConfig;

use File::Temp qw(tempfile);

use MozBuild::Util qw(MkdirWithPath);

use Bootstrap::Step;
use Bootstrap::Config;
use Bootstrap::Util qw(CvsCatfile GetBouncerPlatforms
                       GetBouncerToPatcherPlatformMap);

@ISA = ("Bootstrap::Step");

use strict;

# Channels that we want to add an extra annotation to
my $RELEASE_CANDIDATE_CHANNELS = ['betatest','DisableCompleteJump'];

sub Execute {
    my $this = shift;

    my $config = new Bootstrap::Config();
    my $logDir = $config->Get(sysvar => 'logDir');
    my $configBumpDir = $config->Get(var => 'configBumpDir');
    my $product = $config->Get(var => 'product');
    my $build = $config->Get(var => 'build');
    my $version = $config->GetVersion(longName => 0);
    my $appVersion = $config->GetAppVersion();
    my $oldVersion = $config->GetOldVersion(longName => 0);
    my $mozillaCvsroot = $config->Get(var => 'mozillaCvsroot');
    my $patcherConfig = $config->Get(var => 'patcherConfig');
    my $ftpServer = $config->Get(var => 'ftpServer');
    my $bouncerServer = $config->Get(var => 'bouncerServer');
    my $hgToolsRepo = $config->Get(var => 'hgToolsRepo');
    my $appName = $config->Get(var => 'appName');
    my $branchTag = $config->Get(var => 'branchTag');
    my $stagingServer = $config->Get(var => 'stagingServer');
    my $useBetaChannel = $config->Get(var => 'useBetaChannel');

    my $versionedConfigBumpDir = catfile($configBumpDir, 
                                          "$product-$version-build$build");

    my $checkedOutPatcherConfig = catfile($versionedConfigBumpDir, 'patcher', 
                                          $patcherConfig);

    # Create patcher config area in the config bump area.
    if (not -d $versionedConfigBumpDir) {
        MkdirWithPath(dir => $versionedConfigBumpDir) 
          or die("Cannot mkdir $versionedConfigBumpDir: $!");
    }

    # checkout config to bump
    $this->CvsCo(cvsroot => $mozillaCvsroot,
                 checkoutDir => 'patcher',
                 modules => [CvsCatfile('mozilla', 'tools', 'patcher-configs',
                                        $patcherConfig)],
                 logFile => catfile($logDir, 'patcherconfig-checkout.log'),
                 workDir => $versionedConfigBumpDir
    );

    # Do all the work...
    # bug 456400 moved the BumpPatcherConfig logic to an external script to
    # more easily support both CVS and Mercurial based releases

    $this->Shell(
      cmd => 'hg',
      cmdArgs => ['clone', $hgToolsRepo],
      dir => catfile($versionedConfigBumpDir)
    );
    $this->CvsCo(
      cvsroot => $mozillaCvsroot,
      modules => ['mozilla/' . $appName . '/locales/shipped-locales'],
      tag => $branchTag,
      workDir => $versionedConfigBumpDir,
      checkoutDir => 'locales'
    );
    my $shippedLocales = $versionedConfigBumpDir . '/locales/shipped-locales';
    my @args = (catfile($versionedConfigBumpDir, 'tools', 'release',
                        'patcher-config-bump.pl'),
            '-p', $product,
            '-v', $version,
            '-o', $oldVersion,
            '-a', $appVersion,
            '-b', $build,
            '-c', $checkedOutPatcherConfig,
            '-t', $stagingServer,
            '-f', $ftpServer,
            '-d', $bouncerServer,
            '-l', $shippedLocales);

    if ($useBetaChannel) {
        push(@args, '-u');
    }

    $this->Shell(
      cmd => 'perl',
      cmdArgs => \@args
    );

    # verify that BumpPatcherConfig() actually did something.
    $this->Log(msg=> 'Ignoring shell value here because cvs diff returns a ' .
     'non-zero value if a diff exists; this is an assertion that a diff does ' .
     'exist');

    $this->Shell(
      cmd => 'cvs',
      cmdArgs => ['diff', $patcherConfig ],
      logFile => catfile($logDir, 'patcherconfig-diff.log'),
      ignoreExitValue => 1,
      dir => catfile($versionedConfigBumpDir, 'patcher'),
    ); 

    $this->Shell(
      cmd => 'cvs',
      cmdArgs => ['-d', $mozillaCvsroot,
                  'ci', '-m', "\"Automated configuration bump: $patcherConfig, "
                   .  "from $oldVersion to $version\"", $patcherConfig],
      logFile => catfile($logDir, 'patcherconfig-checkin.log'),
      dir => catfile($versionedConfigBumpDir, 'patcher'),
    ); 
}

sub Verify {
    my $this = shift;
    return;
}

1;
