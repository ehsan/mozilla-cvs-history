#!/usr/bin/python

import subprocess
import sys
import os
import getopt
import nss.nss as nss

verbose = False

tests = [
    {'name'            : 'md5',
     'ref_cmd'         : 'md5sum',
     'nss_digest_func' : nss.md5_digest,
     'hash_oid'        : nss.SEC_OID_MD5},
    {'name'            : 'sha1',
     'ref_cmd'         : 'sha1sum',
     'nss_digest_func' : nss.sha1_digest,
     'hash_oid'        : nss.SEC_OID_SHA1},
    {'name'            : 'sha256',
     'ref_cmd'         : 'sha256sum',
     'nss_digest_func' : nss.sha256_digest,
     'hash_oid'        : nss.SEC_OID_SHA256},
    {'name'            : 'sha512',
     'ref_cmd'         : 'sha512sum',
     'nss_digest_func' : nss.sha512_digest,
     'hash_oid'        : nss.SEC_OID_SHA512},
]


def do_test(name, ref_cmd, nss_digest_func, hash_oid, in_filename, chunk_size):
    result = 0

    hash_oid_name = nss.oid_str(hash_oid)

    if verbose:
        print 'running test %s: nss_digest_func=%s hash_oid=%s in_filename=%s' % \
            (name, nss_digest_func.__name__, hash_oid_name, in_filename)

    # read the data in from the file
    ref_data = open(in_filename).read()

    # Run the system hash function to get a reference result.
    # Since we're testing the python-nss binding we assume
    # the system command is entirely independent and correct.
    #
    # Because our digest routines return raw data (e.g. a buffer of octets)
    # and the system hash command returns a hex string which we need to compare agains,
    # and because we sometimes want to print the result of our digest functions
    # always convert our results to a hex string via nss.data_to_hex()
    proc = subprocess.Popen([ref_cmd, in_filename], stdout=subprocess.PIPE)
    status = proc.wait();
    reference_digest = proc.stdout.read().split()[0]
    if verbose:
        print 'reference_digest\n%s' % (reference_digest)

    # Run the test with convenience digest function (e.g. nss.sha256_digest, etc.).
    test_digest =  nss.data_to_hex(nss_digest_func(ref_data), separator=None)
    if verbose: print 'nss %s\n%s' % (nss_digest_func.__name__, test_digest)

    if test_digest != reference_digest:
        result += 1
        print 'nss %s test failed' % (nss_digest_func.__name__)
        print 'reference = %s' % (reference_digest)
        print 'test      = %s' % (test_digest)

    # Run the test using the generic hash_buf function specifying the hash algorithm.
    test_digest =  nss.data_to_hex(nss.hash_buf(hash_oid, ref_data), separator=None)
    if verbose: print 'nss.hash_buf %s\n%s' % (hash_oid_name, test_digest)

    if test_digest != reference_digest:
        result += 1
        print 'nss.hash_buf %s test failed' % (hash_oid_name)
        print 'reference = %s' % (reference_digest)
        print 'test      = %s' % (test_digest)

    # Run the test using the lowest level hashing functions by specifying the hash algorithm.
    # The entire input data is supplied all at once in a single call.
    context = nss.create_digest_context(hash_oid)
    context.digest_begin()
    context.digest_op(ref_data)
    test_digest = nss.data_to_hex(context.digest_final(), separator=None)
    if verbose: print 'nss.digest_context %s\n%s' % (hash_oid_name, test_digest)

    if test_digest != reference_digest:
        result += 1
        print 'nss.digest_context %s test failed' % (hash_oid_name)
        print 'reference = %s' % (reference_digest)
        print 'test      = %s' % (test_digest)

    # Run the test using the lowest level hashing functions by specifying the hash algorithm
    # and feeding 'chunks' of data one at a time to be consumed.
    in_file = open(in_filename, 'r')
    context = nss.create_digest_context(hash_oid)
    context.digest_begin()
    while True:
        in_data = in_file.read(chunk_size)
        if len(in_data) == 0:
            break
        context.digest_op(in_data)

    test_digest = nss.data_to_hex(context.digest_final(), separator=None)
    if verbose: print 'chunked nss.digest_context %s\n%s' % (hash_oid_name, test_digest)

    if test_digest != reference_digest:
        result += 1
        print 'chunked nss.digest_context %s test failed' % (hash_oid_name)
        print 'reference = %s' % (reference_digest)
        print 'test      = %s' % (test_digest)

    return result

def usage():
    print '''\
digest_test [-v -h] filename
    -v --verbose         turn on verbose output
    -h --help            print usage
    -s --size            number of octets processed in one iteration
    -i --infile filename file to be used as test data
'''

def main():
    global verbose
    in_filename = sys.argv[0]
    chunk_size = 128

    try:
        opts, args = getopt.getopt(sys.argv[1:], 'hvs:i:',
                                   ['help', 'verbose', 'size=', 'infile='])
    except getopt.GetoptError, err:
        print str(err) # will print something like 'option -a not recognized'
        usage()
        sys.exit(2)
    verbose = False
    for o, a in opts:
        if o in ('-s', '--size'):
            chunk_size = int(a)
        elif o in ('-i', '--infile'):
            in_filename = a
        elif o in ('-v', '--verbose'):
            verbose = True
        elif o in ('-h', '--help'):
            usage()
            sys.exit(0)
        else:
            assert False, 'unhandled option'

    nss.nss_init_nodb()

    result = 0
    for test in tests:
        result += do_test(test['name'], test['ref_cmd'], test['nss_digest_func'],
                          test['hash_oid'], in_filename, chunk_size)

    if result == 0:
        print 'SUCCESS'
    else:
        print 'FAILED %d tests' % (result)

    sys.exit(result)


if __name__ == '__main__':
    main()
