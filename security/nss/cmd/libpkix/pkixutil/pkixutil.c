/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Netscape security libraries.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1994-2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Sun Microsystems
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
/*
 * testwrapper.c
 *
 * Wrpper programm for libpkix tests.
 *
 */

#include <stdio.h>

#include "secport.h"

typedef int (*mainTestFn)(int argc, char* argv[]);

extern int libpkix_buildthreads(int argc, char *argv[]);
extern int nss_threads(int argc, char *argv[]);
extern int test_certselector(int argc, char *argv[]);
extern int test_comcertselparams(int argc, char *argv[]);
extern int test_certchainchecker(int argc, char *argv[]);
extern int test_comcrlselparams(int argc, char *argv[]);
extern int test_crlselector(int argc, char *argv[]);

/* This test fails to build. Need to fix                */
/* extern int test_buildparams(int argc, char *argv[]); */
extern int test_procparams(int argc, char *argv[]);
extern int test_resourcelimits(int argc, char *argv[]);
extern int test_trustanchor(int argc, char *argv[]);
extern int test_valparams(int argc, char *argv[]);
extern int test_buildresult(int argc, char *argv[]);
extern int test_policynode(int argc, char *argv[]);
extern int test_valresult(int argc, char *argv[]);
extern int test_verifynode(int argc, char *argv[]);
extern int test_store(int argc, char *argv[]);
extern int test_basicchecker(int argc, char *argv[]);
extern int test_basicconstraintschecker(int argc, char *argv[]);
extern int test_buildchain(int argc, char *argv[]);
extern int test_buildchain_partialchain(int argc, char *argv[]);
extern int test_buildchain_resourcelimits(int argc, char *argv[]);
extern int test_buildchain_uchecker(int argc, char *argv[]);
extern int test_customcrlchecker(int argc, char *argv[]);
extern int test_defaultcrlchecker2stores(int argc, char *argv[]);
extern int test_ocsp(int argc, char *argv[]);
extern int test_policychecker(int argc, char *argv[]);
extern int test_subjaltnamechecker(int argc, char *argv[]);
extern int test_validatechain(int argc, char *argv[]);
extern int test_validatechain_NB(int argc, char *argv[]);
extern int test_validatechain_bc(int argc, char *argv[]);
extern int test_error(int argc, char *argv[]);
extern int test_list(int argc, char *argv[]);
extern int test_list2(int argc, char *argv[]);
extern int test_logger(int argc, char *argv[]);
extern int test_colcertstore(int argc, char *argv[]);
extern int test_ekuchecker(int argc, char *argv[]);
extern int test_httpcertstore(int argc, char *argv[]);
extern int test_pk11certstore(int argc, char *argv[]);
extern int test_socket(int argc, char *argv[]);
extern int test_authorityinfoaccess(int argc, char *argv[]);
extern int test_cert(int argc, char *argv[]);
extern int test_crl(int argc, char *argv[]);
extern int test_crlentry(int argc, char *argv[]);
extern int test_date(int argc, char *argv[]);
extern int test_generalname(int argc, char *argv[]);
extern int test_nameconstraints(int argc, char *argv[]);
extern int test_subjectinfoaccess(int argc, char *argv[]);
extern int test_x500name(int argc, char *argv[]);
extern int stress_test(int argc, char *argv[]);
extern int test_bigint(int argc, char *argv[]);
extern int test_bytearray(int argc, char *argv[]);
extern int test_hashtable(int argc, char *argv[]);
extern int test_mem(int argc, char *argv[]);
extern int test_monitorlock(int argc, char *argv[]);
extern int test_mutex(int argc, char *argv[]);
extern int test_mutex2(int argc, char *argv[]);
extern int test_mutex3(int argc, char *argv[]);
extern int test_object(int argc, char *argv[]);
extern int test_oid(int argc, char *argv[]);

/* Taken out. Problem with build                   */
/* extern int test_rwlock(int argc, char *argv[]); */
extern int test_string(int argc, char *argv[]);
extern int test_string2(int argc, char *argv[]);
extern int build_chain(int argc, char *argv[]);
extern int dumpcert(int argc, char *argv[]);
extern int dumpcrl(int argc, char *argv[]);
extern int validate_chain(int argc, char *argv[]);


typedef struct {
    char *fnName;
    mainTestFn fnPointer;
} testFunctionRef;

testFunctionRef testFnRefTable[] = {
    {"libpkix_buildthreads",           libpkix_buildthreads},
    {"nss_threads",                    nss_threads},
    {"test_certselector",              test_certselector},
    {"test_comcertselparams",          test_comcertselparams},
    {"test_certchainchecker",          test_certchainchecker},
    {"test_comcrlselparams",           test_comcrlselparams},
    {"test_crlselector",               test_crlselector},
/*  {"test_buildparams",               test_buildparams}*/
    {"test_procparams",                test_procparams},
    {"test_resourcelimits",            test_resourcelimits},
    {"test_trustanchor",               test_trustanchor},
    {"test_valparams",                 test_valparams},
    {"test_buildresult",               test_buildresult},
    {"test_policynode",                test_policynode},
    {"test_valresult",                 test_valresult},
    {"test_verifynode",                test_verifynode},
    {"test_store",                     test_store},
    {"test_basicchecker",              test_basicchecker},
    {"test_basicconstraintschecker",   test_basicconstraintschecker},
    {"test_buildchain",                test_buildchain},
    {"test_buildchain_partialchain",   test_buildchain_partialchain},
    {"test_buildchain_resourcelimits", test_buildchain_resourcelimits},
    {"test_buildchain_uchecker",       test_buildchain_uchecker},
    {"test_customcrlchecker",          test_customcrlchecker},
    {"test_defaultcrlchecker2stores",  test_defaultcrlchecker2stores},
    {"test_ocsp",                      test_ocsp},
    {"test_policychecker",             test_policychecker},
    {"test_subjaltnamechecker",        test_subjaltnamechecker},
    {"test_validatechain",             test_validatechain},
    {"test_validatechain_NB",          test_validatechain_NB},
    {"test_validatechain_bc",          test_validatechain_bc},
    {"test_error",                     test_error},
    {"test_list",                      test_list},
    {"test_list2",                     test_list2},
    {"test_logger",                    test_logger},
    {"test_colcertstore",              test_colcertstore},
    {"test_ekuchecker",                test_ekuchecker},
    {"test_httpcertstore",             test_httpcertstore},
    {"test_pk11certstore",             test_pk11certstore},
    {"test_socket",                    test_socket},
    {"test_authorityinfoaccess",       test_authorityinfoaccess},
    {"test_cert",                      test_cert},
    {"test_crl",                       test_crl},
    {"test_crlentry",                  test_crlentry},
    {"test_date",                      test_date},
    {"test_generalname",               test_generalname},
    {"test_nameconstraints",           test_nameconstraints},
    {"test_subjectinfoaccess",         test_subjectinfoaccess},
    {"test_x500name",                  test_x500name},
    {"stress_test",                    stress_test},
    {"test_bigint",                    test_bigint},
    {"test_bytearray",                 test_bytearray},
    {"test_hashtable",                 test_hashtable},
    {"test_mem",                       test_mem},
    {"test_monitorlock",               test_monitorlock},
    {"test_mutex",                     test_mutex},
    {"test_mutex2",                    test_mutex2},
    {"test_mutex3",                    test_mutex3},
    {"test_object",                    test_object},
    {"test_oid",                       test_oid},
/*  {"test_rwlock",                    test_rwlock, }*/
    {"test_string",                    test_string},
    {"test_string2",                   test_string2},
    {"build_chain",                    build_chain},
    {"dumpcert",                       dumpcert},
    {"dumpcrl",                        dumpcrl},
    {"validate_chain",                 validate_chain},
    {NULL,                             NULL },
};

static
void printUsage(char *cmdName) {
    int fnCounter = 0, totalCharLen = 0;

    fprintf(stderr, "Usage: %s [test name] [arg1]...[argN]\n\n", cmdName);
    fprintf(stderr, "List of possible names for the tests:");
    while (testFnRefTable[fnCounter].fnName != NULL) {
        if (fnCounter % 2 == 0) {
            fprintf(stderr, "\n");
        }
        fprintf(stderr, "  %-35s ", testFnRefTable[fnCounter].fnName);
        fnCounter += 1;
    }
    fprintf(stderr, "\n");
}


int main(int argc, char **argv) {
    char *fnName = NULL;
    int fnCounter = 0;

    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    fnName = argv[1];
    while (testFnRefTable[fnCounter].fnName != NULL) {
        int fnNameLen = PORT_Strlen(testFnRefTable[fnCounter].fnName);
        if (!PORT_Strncmp(fnName, testFnRefTable[fnCounter].fnName,
                          fnNameLen)) {
            return testFnRefTable[fnCounter].fnPointer(argc - 1, argv + 1);
        }
        fnCounter += 1;
    }
    printf("ERROR: unknown name of the test: %s.\n", fnName);
    printUsage(argv[0]);
    return -1;
}

