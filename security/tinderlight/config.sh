#! /bin/bash

#### LOCAL MACHINE SETTINGS ####

# example configuration
case ${HOST} in
host1) 
    JAVA_HOME_64=/opt/jdk/1.6.0_01/SunOS64
    JAVA_HOME_32=/opt/jdk/1.6.0_01/SunOS
    ;;
host2)
    run_bits="32"
    export NSS_TESTS=memleak
    NO_JSS=1
    ;;
esac

