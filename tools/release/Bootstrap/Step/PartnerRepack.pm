#
# PartnerRepack step. Unpacks, modifies, repacks a Firefox en-US build.
#
package Bootstrap::Step::PartnerRepack;
use Bootstrap::Step;
use Bootstrap::Config;
use Bootstrap::Util qw(CvsCatfile);
use MozBuild::Util qw(MkdirWithPath);
@ISA = ("Bootstrap::Step");

sub Execute {
    my $this = shift;

    my $config = new Bootstrap::Config();
    my $product = $config->Get(var => 'product');
    my $version = $config->GetVersion(longName => 0);
    my $build = $config->Get(var => 'build');
    my $logDir = $config->Get(sysvar => 'logDir');
    my $hgPartnersRepo = $config->Get(var => 'hgPartnersRepo');
    my $hgPartnersTag = $config->Get(var => 'hgPartnersTag');
    my $partnerRepackDir = $config->Get(var => 'partnerRepackDir');
    my $mozillaCvsroot = $config->Get(var => 'mozillaCvsroot');

    my $buildLog = catfile($logDir,
                           'repack_' . $version . '-build-partner-repack.log');

    MkdirWithPath(dir => $partnerRepackDir)
      or die("Cannot mkdir $partnerRepackDir: $!");

    $this->CvsCo(cvsroot => $mozillaCvsroot,
                 checkoutDir => 'package_tools',
                 modules => [CvsCatfile('mozilla', 'build',
                                        'package','mac_osx')],
                 logFile => $buildLog,
                 workDir => $partnerRepackDir
    );

    $this->HgClone(
      repo => $hgPartnersRepo,
      tag => $hgPartnersTag,
      workDir => $partnerRepackDir
    );
    
    $this->Shell(
      cmd => './partner-repacks.py',
      cmdArgs => ['--version', $version,
                  '--build-number', $build],
      dir => catfile($partnerRepackDir, 'partner-repacks', 'scripts'),
      appendToPath => "${partnerRepackDir}/package_tools",
      logFile => $buildLog,
      timeout => 3600
    );
    
}

# Make sure we have appropriate repacks for every partner in the repo.
sub Verify {
    my $this = shift;

    my $config = new Bootstrap::Config();
    my $product = $config->Get(var => 'product');
    my $build = $config->Get(var => 'build');
    my $version = $config->GetVersion(longName => 0);
    my $logDir = $config->Get(sysvar => 'logDir');
    my $hgPartnersRepo = $config->Get(var => 'hgPartnersRepo');
    my $hgPartnersTag = $config->Get(var => 'hgPartnersTag');
    my $partnerRepackDir = $config->Get(var => 'partnerRepackDir');

    my $buildLog = catfile($logDir,
                           'repack_' . $version . '-build-partner-repack.log');

    $this->Shell(
      cmd => './partner-repacks.py',
      cmdArgs => ['--version', $version,
                  '--build-number', $build,
                  '--verify-only'],
      dir => catfile($partnerRepackDir, 'partner-repacks', 'scripts'),
      logFile => $buildLog,
      timeout => 3600
    );
}

sub Push {
    my $this = shift;

    my $config = new Bootstrap::Config();
    my $product = $config->Get(var => 'product');
    my $version = $config->GetVersion(longName => 0);
    my $build = $config->Get(var => 'build');
    my $logDir = $config->Get(sysvar => 'logDir');
    my $stagingUser = $config->Get(var => 'stagingUser');
    my $stagingServer = $config->Get(var => 'stagingServer');
    my $partnerRepackDir = $config->Get(var => 'partnerRepackDir');
    my $remoteRepackDir = $config->Get(var => 'remoteRepackDir');
    my $linuxExtension = $config->GetLinuxExtension();

    my $pushLog = catfile($logDir,
                             'repack_' . $version . '-push-partner-repack.log');
    
    my $pushDir = catfile($partnerRepackDir, "partner-repacks",
                             "scripts", "repacked_builds");
    
    $this->Shell(
        cmd => 'ssh',
        cmdArgs => ['-2', '-l', $stagingUser, $stagingServer,
                    'mkdir', '-p', $remoteRepackDir
                    ],
        logFile => $pushLog,
    );

    $this->Shell(
      cmd => 'rsync',
      cmdArgs => ['-Lav',
                  '-e', 'ssh',
                  '--include=*.dmg',
                  '--include=*.exe',
                  '--include=*.tar.'.$linuxExtension,
                  '.',
                  $stagingServer . ":" . $remoteRepackDir,
                 ],
      dir => $pushDir,
      logFile => $pushLog,
      timeout => 3600
    );
}

sub Announce {
    my $this = shift;

    my $config = new Bootstrap::Config();
    my $product = $config->Get(var => 'product');
    my $productTag = $config->Get(var => 'productTag');
    my $version = $config->GetVersion(longName => 0);
    my $build = $config->Get(var => 'build');
    my $logDir = $config->Get(sysvar => 'logDir');
    my $stagingServer = $config->Get(var => 'stagingServer');
    my $remoteRepackDir = $config->Get(var => 'remoteRepackDir');
    my $partnerRepackCC = $config->Get(var => 'partnerRepackCC');

    my $buildLog = catfile($logDir,
                           'repack_' . $version . '-build-partner-repack.log');

    $remoteRepackDir =~ s|^/home/ftp||;

    $this->SendAnnouncement(
      subject => "$product $version partner repack step finished",
      message => "$product $version partner repacks were copied to the staging directory:\n\n" .
                 "http://${stagingServer}${remoteRepackDir}/${version}/build${build}/",
      cc => $partnerRepackCC,
    );
}

1;
