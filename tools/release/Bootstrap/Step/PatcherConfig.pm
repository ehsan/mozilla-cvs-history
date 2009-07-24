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
    my $longVersion = $config->GetVersion(longName => 1);
    my $oldVersion = $config->GetOldVersion(longName => 0);
    my $oldAppVersion = $config->GetOldAppVersion();
    my $oldLongVersion = $config->GetOldVersion(longName => 1);
    my $oldBuild = $config->Get(var => 'oldBuild');
    my $mozillaCvsroot = $config->Get(var => 'mozillaCvsroot');
    my $patcherConfig = $config->Get(var => 'patcherConfig');
    my $ftpServer = $config->Get(var => 'ftpServer');
    my $bouncerServer = $config->Get(var => 'bouncerServer');
    my $hgToolsRepo = $config->Get(var => 'hgToolsRepo');
    my $appName = $config->Get(var => 'appName');
    my $releaseTag = $config->Get(var => 'productTag') . '_RELEASE';
    my $stagingServer = $config->Get(var => 'stagingServer');
    my $ausServerUrl = $config->Get(var => 'ausServerUrl');
    my $useBetaChannel = $config->Get(var => 'useBetaChannel');
    my $linuxExtension = $config->GetLinuxExtension();

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

    $this->HgClone(
      repo => $hgToolsRepo,
      workDir => catfile($versionedConfigBumpDir)
    );
    $this->CvsCo(
      cvsroot => $mozillaCvsroot,
      modules => ['mozilla/' . $appName . '/locales/shipped-locales'],
      tag => $releaseTag,
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
                   .  "for $product $version build$build\"", $patcherConfig],
      logFile => catfile($logDir, 'patcherconfig-checkin.log'),
      dir => catfile($versionedConfigBumpDir, 'patcher'),
    ); 

    # bump the update verify configs too
    my $oldCandidatesDir = CvsCatfile('pub', 'mozilla.org', $product, 'nightly',
                                     $oldVersion . '-candidates',
                                     'build' . $oldBuild . '/');

    my $oldTagVersion = $oldVersion;
    $oldTagVersion =~ s/\./_/g;
    my $oldReleaseTag = uc($product).'_'.$oldTagVersion.'_RELEASE';

    $this->CvsCo(
      cvsroot => $mozillaCvsroot,
      modules => [CvsCatfile('mozilla', $appName, 'locales',
                             'shipped-locales')],
      tag => $oldReleaseTag,
      workDir => $versionedConfigBumpDir,
      checkoutDir => 'old-locales'
    );
    my $oldShippedLocales = catfile($versionedConfigBumpDir, 'old-locales',
                                 'shipped-locales');

    foreach my $osname (qw/ linux macosx win32/ ) {
        my $verifyConfig = $config->Get(var => $osname.'_verifyConfig');
        my @args = (catfile($versionedConfigBumpDir, 'tools', 'release',
                            'update-verify-bump.pl'),
                '-o', $osname,
                '-p', $product,
                '--old-version=' . $oldVersion,
                '--old-app-version=' . $oldAppVersion,
                '--old-long-version=' . $oldLongVersion,
                '-v', $version,
                '--app-version=' . $appVersion,
                '--long-version=' . $longVersion,
                '-n', $build,
                '-a', $ausServerUrl,
                '-s', $stagingServer,
                '-c', catfile($versionedConfigBumpDir, 'tools', 'release', 'updates',
                              $verifyConfig),
                '-d', $oldCandidatesDir,
                '-e', $linuxExtension,
                '-l', $oldShippedLocales
        );

        $this->Shell(
          cmd => 'perl',
          cmdArgs => \@args
        );
    }

    $this->Shell(
      cmd => 'hg',
      cmdArgs => ['commit', '-m',
                  '"Automated configuration bump: update verify configs for '
                  . $product  . ' ' . $version . "build$build" . '"'],
      logFile => catfile($logDir, 'update_verify-checkin.log'),
      dir => catfile($versionedConfigBumpDir, 'tools')
    );
    $this->HgPush(
      repo => $hgToolsRepo,
      workDir => catfile($versionedConfigBumpDir, 'tools')
    );

}

sub Verify {
    my $this = shift;
    return;
}

1;
