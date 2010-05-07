import PerfConfigurator as pc
import os, sys, getopt

class remotePerfConfigurator(pc.PerfConfigurator):
    def __init__(self, **kwargs):
        pc.PerfConfigurator.__init__(self, **kwargs)

    def convertUrlToRemote(self, line):
        """
          For a give url line in the .config file, add a webserver.
          In addition if there is a .manifest file specified, covert 
          and copy that file to the remote device.
        """
        parts = line.split(' ')
        newline = ''
        for part in parts:
            if ('.html' in part):
                newline += 'http://' + self.webServer + '/' + part
            elif ('.manifest' in part):
                newline += self.buildRemoteManifest(part)
            elif ('.xul' in part):
                newline += 'http://' + self.webServer + '/' + part
            else:
                newline += part
                if (part <> parts[-1]):
                    newline += ' '
        return newline

    def buildRemoteManifest(self, manifestName):
        """
          Take a given manifest name, convert the localhost->remoteserver, and then copy to the device
          returns the remote filename on the device so we can add it to the .config file
        """
        remoteName = '/tests/talos/'
        fHandle = open(manifestName, 'r')
        manifestData = fHandle.read()
        fHandle.close()

        newHandle = open(manifestName + '.remote', 'w')
        for line in manifestData.split('\n'):
            newHandle.write(line.replace('localhost', self.webServer) + "\n")
        newHandle.close()

        remoteName += os.path.basename(manifestName)
        if self.testAgent.pushFile(manifestName + '.remote', remoteName) == None:
            raise pc.Configuration("Unable to copy remote manifest file " 
                                + manifestName + ".remote to " + remoteName)
        return remoteName


def main(argv=None):
    exePath = ""
    configPath = ""
    sampleConfig = "sample.config"
    output = ""
    title = "mobile-01"
    branch = ""
    branchName = ""
    testDate = ""
    browserWait = "5"
    verbose = False
    buildid = ""
    useId = False
    resultsServer = ''
    resultsLink = ''
    activeTests = ''
    noChrome = False
    fast = False
    symbolsPath = None
    remoteDevice = ''
    remotePort = ''
    webServer = 'localhost'
    deviceRoot = ''

    if argv is None:
        argv = sys.argv
    try:
        try:
            opts, args = getopt.getopt(argv[1:], "hvue:c:t:b:o:i:d:s:l:a:n:r:p:w", 
                ["help", "verbose", "useId", "executablePath=", 
                "configFilePath=", "sampleConfig=", "title=", 
                "branch=", "output=", "id=", "testDate=", "browserWait=",
                "resultsServer=", "resultsLink=", "activeTests=", 
                "noChrome", "branchName=", "fast", "symbolsPath=",
                "remoteDevice=", "remotePort=", "webServer=", "deviceRoot="])
        except getopt.error, msg:
            raise pc.Usage(msg)
        
        # option processing
        for option, value in opts:
            if option in ("-v", "--verbose"):
                verbose = True
            if option in ("-h", "--help"):
                raise Usage(help_message)
            if option in ("-e", "--executablePath"):
                exePath = value
            if option in ("-c", "--configFilePath"):
                configPath = value
            if option in ("-f", "--sampleConfig"):
                sampleConfig = value
            if option in ("-t", "--title"):
                title = value
            if option in ("-b", "--branch"):
                branch = value
            if option in ("--branchName"):
                branchName = value
            if option in ("-o", "--output"):
                output = value
            if option in ("-i", "--id"):
                buildid = value
            if option in ("-d", "--testDate"):
                testDate = value
            if option in ("-w", "--browserWait"):
                browserWait = value
            if option in ("-u", "--useId"):
                useId = True
            if option in ("-s", "--resultsServer"):
                resultsServer = value
            if option in ("-l", "--resultsLink"):
                resultsLink = value
            if option in ("-a", "--activeTests"):
                activeTests = value
            if option in ("-n", "--noChrome"):
                noChrome = True
            if option in ("-r", "--remoteDevice"):
                remoteDevice = value
            if option in ("-p", "--remotePort"):
                remotePort = value
            if option in ("-w", "--webServer"):
                webServer = value
            if option in ("--deviceRoot",):
                deviceRoot = value
            if option in ("--fast",):
                fast = True
            if option in ("--symbolsPath",):
                symbolsPath = value
        
    except pc.Usage, err:
        print >> sys.stderr, sys.argv[0].split("/")[-1] + ": " + str(err.msg)
        print >> sys.stderr, "\t for help use --help"
        return 2

    #remotePort will default to 20701 and is optional.
    #webServer can be used without remoteDevice, but is required when using remoteDevice
    if (remoteDevice != '' or deviceRoot != ''):
        if (webServer == 'localhost' or deviceRoot == '' or remoteDevice == ''):
            print "\nERROR: When running Talos on a remote device, you need to provide a webServer, deviceRoot and optionally a remotePort"
            print pc.help_message
            return 2

    configurator = remotePerfConfigurator(title=title,
                                    executablePath=exePath,
                                    configFilePath=configPath,
                                    sampleConfig=sampleConfig,
                                    buildid=buildid,
                                    branch=branch,
                                    branchName=branchName,
                                    verbose=verbose,
                                    testDate=testDate,
                                    browserWait=browserWait,
                                    outputName=output,
                                    useId=useId,
                                    resultsServer=resultsServer,
                                    resultsLink=resultsLink,
                                    activeTests=activeTests,
                                    noChrome=noChrome,
                                    fast=fast,
                                    symbolsPath=symbolsPath,
                                    remoteDevice=remoteDevice,
                                    remotePort=remotePort,
                                    webServer=webServer,
                                    deviceRoot=deviceRoot)
    try:
        configurator.writeConfigFile()
    except pc.Configuration, err:
        print >> sys.stderr, sys.argv[0].split("/")[-1] + ": " + str(err.msg)
        return 5
    return 0
    
if __name__ == "__main__":
    sys.exit(main())
