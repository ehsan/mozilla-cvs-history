#! /bin/bash

proc_args()
{
    while [ -n "$1" ]; do
        OPT=$(echo $1 | cut -d= -f1)
        VAL=$(echo $1 | cut -d= -f2)

        case $OPT in
            "--bits")
                RUN_BITS="${VAL}"
                ;;
            "--opt")
                RUN_OPT="${VAL}"
                ;;
            "--once")
                RUN_ONCE=1
                ;;
            "--cycles")
                RUN_CYCLES=1
                ;;
            "--nomail")
                NO_MAIL=1
                ;;
            "--nocvs")
                NO_CVS=1
                ;;
            "--nobuild")
                NO_BUILD=1
                ;;
            "--notest")
                NO_TEST=1
                ;;
            "--nomove")
                NO_MOVE=1
                ;;
            "--nojss")
                NO_JSS=1
                ;;
            *)
                echo "Usage: $0 [--bits=BITS] [--opt=OPT] [--once] ..."
                echo "    --bits      - bits mode (32 or 64)"
                echo "    --opt       - debug/opt mode (DBG or OPT)"
                echo "    --once      - run only once"
                echo "    --cycles    - run cycles (pkix, shared db,..) in separate runs"
                echo "    --nomail    - don't send e-mail"
                echo "    --nocvs     - skip CVS checkout (work with old data)"
                echo "    --nobuild   - skip build"
                echo "    --notest    - skip testing"
                echo "    --nomove    - don't move old data directory after testing"
                echo "    --nojss     - don't build/test JSS (NSS only)"
                exit 0
                ;;
        esac 

        shift
    done
}

set_env()
{
    TESTDIR=$(pwd)
    DATADIR=$(pwd)$(echo "/data/${HOST}_${RUN_BITS}_${RUN_OPT}" | sed "s/ /_/g")
    LOG_ALL="${DATADIR}/all.log"
    LOG_TMP="${DATADIR}/tmp.log"

    if [ "${BRANCH}" = "stable" ]; then 
        CVS_LIST="${CVS_STABLE}"
        TB_TREE="NSS-Stable-Branch"
    else
        CVS_LIST="${CVS_TRUNK}"
        TB_TREE="NSS"
    fi

    TESTSET=standard
    [ "${NSS_TESTS}" = "memleak" ] && TESTSET=memleak
}

print_log()
{
    DATE=$(date "+TB [%Y-%m-%d %H:%M:%S]")
    echo "${DATE} $*"
    echo "${DATE} $*" >> ${LOG_ALL}
}

print_result()
{
    TESTNAME=$1
    RET=$2
    EXP=$3

    if [ ${RET} -eq ${EXP} ]; then
        print_log "${TESTNAME} PASSED"
    else
        print_log "${TESTNAME} FAILED"
    fi
}

print_env()
{
    print_log "######## Environment variables ########"

    uname -a | tee -a ${LOG_ALL}
    if [ -e "/etc/redhat-release" ]; then
        cat "/etc/redhat-release" | tee -a ${LOG_ALL}
    fi
    env | tee -a ${LOG_ALL}
}

print_mail_header()
{
    TREE=$1
    BUILD_DATE=$2
    STATUS=$3
    BUILD=$4

    echo
    echo "tinderbox: tree: ${TREE}"
    echo "tinderbox: builddate: ${BUILD_DATE}"
    echo "tinderbox: status: ${STATUS}"
    echo "tinderbox: build: ${BUILD}"
    echo "tinderbox: errorparser: unix"
    echo "tinderbox: buildfamily: unix"
    echo "tinderbox: END"
    echo
}

mail_start()
{
    print_mail_header "${TB_TREE}" "${BUILD_DATE}" building "${BRANCH} ${TESTSET} ${HOST} ${ARCH} ${RUN_BITS}bit ${RUN_OPT}" > ${LOG_TMP}
    ${MAIL} ${TB_SERVER} < ${LOG_TMP}
}

mail_finish()
{
    STATUS=$1

    print_mail_header "${TB_TREE}" "${BUILD_DATE}" "${STATUS}" "${BRANCH} ${TESTSET} ${HOST} ${ARCH} ${RUN_BITS}bit ${RUN_OPT}" > ${LOG_TMP}
    cat ${LOG_ALL} >> ${LOG_TMP}
    ${MAIL} ${TB_SERVER} < ${LOG_TMP}
}

cvs_checkout()
{
    print_log "######## CVS checkout ########"

    print_log "$ cd ${DATADIR}"
    cd ${DATADIR}

    for CVS_FILE in ${CVS_LIST}; do
        CVS_FILE=$(echo ${CVS_FILE} | sed "s/:/ /g")

        print_log "$ ${CVS} -d ${CVSROOT} co -A ${CVS_FILE}"
        ${CVS} -d ${CVSROOT} co -A ${CVS_FILE} >> ${LOG_ALL} 2>&1
        RET=$?
        print_result "CVS checkout ${CVS_FILE}" ${RET} 0
        [ ${RET} -eq 0 ] || return ${RET}
    done

    print_log "$ ${CVS} -d ${CVSROOT} stat mozilla" 
    ${CVS} -d ${CVSROOT} stat mozilla > ${LOG_TMP} 2>&1
    RET=$?
    print_result "CVS stat mozilla" ${RET} 0
    [ ${RET} -eq 0 ] || return ${RET}

    if [ -f ${DATADIR}.cvs ]; then
        diff -U4 ${LOG_TMP} ${DATADIR}.cvs > /dev/null
        if [ $? -ne 0 ]; then 
            print_log "CVS change detected" 
            echo "TinderboxPrint:CVS change" >> ${LOG_ALL}
        fi
    fi

    mv ${LOG_TMP} ${DATADIR}.cvs

    return 0
}

apply_patches()
{
    [ -z "${NSS_PATCH}" ] && return 0

    print_log "######## Applying patches ########"

    for PDATA in ${NSS_PATCH}; do
        PDIR=$(echo ${PDATA} | cut -d: -f1)
        PFILE=$(echo ${PDATA} | cut -d: -f2)
       
        cd ${DATADIR}/${PDIR} 
        ${PATCH} -p0 < ${PFILE} >> ${LOG_ALL} 2>&1
        RET=$?
        print_result "Applying patch ${PFILE}" ${RET} 0
        [ ${RET} -eq 0 ] || return ${RET}
    done

    return 0
}

set_cycle()
{
    BITS=$1
    OPT=$2
    CYCLE_CNT=$3

    if [ "${BITS}" = "64" ]; then
        USE_64=1
        JAVA_HOME=${JAVA_HOME_64} 
        PORT_DBG=${PORT_64_DBG}
        PORT_OPT=${PORT_64_OPT}
    else
        USE_64=
        JAVA_HOME=${JAVA_HOME_32} 
        PORT_DBG=${PORT_32_DBG}
        PORT_OPT=${PORT_32_OPT}
    fi
    export USE_64
    export JAVA_HOME

    BUILD_OPT=
    if [ "${OPT}" = "OPT" ]; then
        BUILD_OPT=1
        XPCLASS=xpclass.jar
        PORT=${PORT_OPT}
    else
        BUILD_OPT=
        XPCLASS=xpclass_dbg.jar
        PORT=${PORT_DBG}
    fi
    export BUILD_OPT

    PORT_JSS_SERVER=$(expr ${PORT} + 20)
    PORT_JSSE_SERVER=$(expr ${PORT} + 40)

    export PORT
    export PORT_JSS_SERVER
    export PORT_JSSE_SERVER

    [ -z ${RUN_CYCLES} ] && return 0

    CYCLE_ID=$(expr ${CYCLE_CNT} % 4)
    case ${CYCLE_ID} in
        0)
            export NSS_CYCLES=standard
            CYCLE_TEXT="Standard"
            ;;
        1)
            export NSS_CYCLES=pkix
            CYCLE_TEXT="PKIX"
            ;;
        2)
            export NSS_CYCLES=upgradedb
            CYCLE_TEXT="Upgrade DB"
            ;;
        3)
            export NSS_CYCLES=sharedb
            CYCLE_TEXT="Shared DB"
            ;;
    esac
}

build_nss()
{
    print_log "######## NSS - build - ${BITS} bits - ${OPT} ########"

    print_log "$ cd ${DATADIR}/mozilla/security/nss"
    cd ${DATADIR}/mozilla/security/nss

    print_log "$ ${MAKE} ${NSS_BUILD_TARGET}"
    ${MAKE} ${NSS_BUILD_TARGET} >> ${LOG_ALL} 2>&1
    RET=$?
    print_result "NSS - build - ${BITS} bits - ${OPT}" ${RET} 0
    [ ${RET} -eq 0 ] || return ${RET}

    return 0
}

build_jss()
{
    print_log "######## JSS - build - ${BITS} bits - ${OPT} ########"

    print_log "$ cd ${DATADIR}/mozilla/security/jss"
    cd ${DATADIR}/mozilla/security/jss

    print_log "$ ${MAKE} ${JSS_BUILD_TARGET}"
    ${MAKE} ${JSS_BUILD_TARGET} >> ${LOG_ALL} 2>&1
    RET=$?
    print_result "JSS build - ${BITS} bits - ${OPT}" ${RET} 0
    [ ${RET} -eq 0 ] || return ${RET}

    print_log "$ cd ${DATADIR}/mozilla/dist"
    cd ${DATADIR}/mozilla/dist

    print_log "cat ${TESTDIR}/keystore.pw | ${JAVA_HOME}/bin/jarsigner -keystore ${TESTDIR}/keystore -internalsf ${XPCLASS} jssdsa"
    cat ${TESTDIR}/keystore.pw | ${JAVA_HOME}/bin/jarsigner -keystore ${TESTDIR}/keystore -internalsf ${XPCLASS} jssdsa >> ${LOG_ALL} 2>&1
    RET=$?
    print_result "JSS - sign JAR files - ${BITS} bits - ${OPT}" ${RET} 0
    [ ${RET} -eq 0 ] || return ${RET}

    print_log "${JAVA_HOME}/bin/jarsigner -verify -certs ${XPCLASS}"
    ${JAVA_HOME}/bin/jarsigner -verify -certs ${XPCLASS} >> ${LOG_ALL} 2>&1
    RET=$?
    print_result "JSS - verify JAR files - ${BITS} bits - ${OPT}" ${RET} 0
    [ ${RET} -eq 0 ] || return ${RET}

    return 0
}

test_nss()
{
    print_log "######## NSS - tests - ${BITS} bits - ${OPT} ########"

    [ -n ${RUN_CYCLES} ] && echo "TinderboxPrint:${CYCLE_TEXT}" >> ${LOG_ALL}

    print_log "$ cd ${DATADIR}/mozilla/security/nss/tests"
    cd ${DATADIR}/mozilla/security/nss/tests

    print_log "$ ./all.sh"
    ./all.sh > ${LOG_TMP} 2>&1 
    RET=$?
    cat ${LOG_TMP} >> ${LOG_ALL}
    cat ${LOG_TMP} | grep FAIL
    print_result "NSS - tests - ${BITS} bits - ${OPT}" ${RET} 0
    [ ${RET} -eq 0 ] || return 1

    return 0
}

test_jss()
{
    print_log "######## JSS - tests - ${BITS} bits - ${OPT} ########"

    print_log "$ cd ${DATADIR}/mozilla/security/jss"
    cd ${DATADIR}/mozilla/security/jss

    print_log "$ ${MAKE} platform"
    PLATFORM=$(${MAKE} platform)
    print_log "PLATFORM=${PLATFORM}"

    print_log "$ cd ${DATADIR}/mozilla/security/jss/org/mozilla/jss/tests"
    cd ${DATADIR}/mozilla/security/jss/org/mozilla/jss/tests

    print_log "$ perl all.pl dist ${DATADIR}/mozilla/dist/${PLATFORM}"
    perl all.pl dist ${DATADIR}/mozilla/dist/${PLATFORM} > ${LOG_TMP} 2>&1
    RET=$?
    cat ${LOG_TMP} >> ${LOG_ALL}
    cat ${LOG_TMP} | grep FAIL
    print_result "JSS - tests - ${BITS} bits - ${OPT}" ${RET} 0
    [ ${RET} -eq 0 ] || return 1

    return 0
}

build_and_test()
{
    if [ -z "${NO_BUILD}" ]; then
        build_nss
        [ $? -eq 0 ] || return 1
    fi

    if [ -z "${NO_TEST}" ]; then
        test_nss
        [ $? -eq 0 ] || return 1
    fi

    if [ -z "${NO_JSS}" -a -z "${NO_BUILD}" ]; then
        build_jss
        [ $? -eq 0 ] || return 1
    fi

    if [ -z "${NO_JSS}" -a -z "${NO_TEST}" ]; then
        test_jss
        [ $? -eq 0 ] || return 1
    fi

    return 0
}

run_cycle()
{
    if [ -z "${NO_MAIL}" ]; then
        BUILD_DATE=$(${AWK} 'BEGIN{ srand(); print srand(); }')
        mail_start
    fi

    print_env
    STATUS=success

    if [ -z "${NO_CVS}" ]; then
        cvs_checkout
        [ $? -ne 0 ] && STATUS=busted
    fi
        
    if [ ${STATUS} = "success" -a -z "${NO_CVS}" ]; then
        apply_patches
        [ $? -ne 0 ] && STATUS=busted
    fi

    if [ ${STATUS} = "success" ]; then
        build_and_test
        [ $? -ne 0 ] && STATUS=testfailed
    fi

    grep ^TinderboxPrint ${LOG_ALL}

    if [ -z "${NO_MAIL}" ]; then
        mail_finish ${STATUS}
    fi
}

run_all()
{
    if [ ${SLEEP_TIME} -gt 0 ]; then
        echo "Waiting ${SLEEP_TIME} minutes"
        SLEEP_TIME=$(expr ${SLEEP_TIME} \* 60)
        sleep ${SLEEP_TIME}
        [ $? -eq 0 ] || return 1
    fi

    START_TIME=$(${AWK} 'BEGIN{ srand(); print srand(); }')

    [ -z "${NO_CVS}" ] && rm -rf ${DATADIR}
    [ -f "${LOG_ALL}" ] && rm ${LOG_ALL}
 
    mkdir -p ${DATADIR}

    set_cycle ${BITS} ${OPT} ${CYCLE_CNT}
    run_cycle

    CYCLE_ID=$(expr ${CYCLE_CNT} % ${CYCLE_MAX} + 1)

    cd ${TESTDIR}
    rm -rf ${DATADIR}.last.${CYCLE_ID}

    if [ -z "${NO_MOVE}" ]; then
        mv ${DATADIR} ${DATADIR}.last.${CYCLE_ID}
    else
        cp -r ${DATADIR} ${DATADIR}.last.${CYCLE_ID}
    fi 

    CYCLE_CNT=$(expr ${CYCLE_CNT} + 1)

    FINISH_TIME=$(${AWK} 'BEGIN{ srand(); print srand(); }')
    TESTING_TIME=$(expr ${FINISH_TIME} - ${START_TIME})
    TESTING_TIME=$(expr ${TESTING_TIME} / 60)
    if [ ${TESTING_TIME} -ge ${CYCLE_TIME} ]; then
        SLEEP_TIME=0
    else
        SLEEP_TIME=$(expr ${CYCLE_TIME} - ${TESTING_TIME})
    fi

    [ -n "${RUN_ONCE}" ] && RUN=0
}

main()
{
    CYCLE_CNT=0
    SLEEP_TIME=0
    RUN=1
    VALID=0
 
    while [ ${RUN} -eq 1 ]; do 
        for BITS in 32 64; do
            echo ${RUN_BITS} | grep ${BITS} > /dev/null
            [ $? -eq 0 ] || continue
            for OPT in DBG OPT; do
                echo ${RUN_OPT} | grep ${OPT} > /dev/null
                [ $? -eq 0 ] || continue

                if [ ${RUN} -eq 1 ]; then
                    VALID=1 
                    run_all 
                fi
            done
        done

        if [ ${VALID} -ne 1 ]; then
            echo "Need to set valid bits/opt values."
            RUN=0
        fi
    done

    return 0
}

. env.sh
proc_args "$@"
set_env
main

