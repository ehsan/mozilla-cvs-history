#!/usr/bin/python

import subprocess
import sys
import unittest

import nss.nss as nss

#-------------------------------------------------------------------------------

verbose = False
in_filename = sys.argv[0]
chunk_size = 128


#-------------------------------------------------------------------------------

class TestDigest(unittest.TestCase):
    def setUp(self):
        nss.nss_init_nodb()

    def tearDown(self):
        nss.nss_shutdown()

    def do_test(self, name, ref_cmd, nss_digest_func, hash_oid):
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

        self.assertEqual(test_digest, reference_digest,
                         msg='nss %s test failed reference=%s test=%s' % \
                             (nss_digest_func.__name__, reference_digest, test_digest))

        # Run the test using the generic hash_buf function specifying the hash algorithm.
        test_digest =  nss.data_to_hex(nss.hash_buf(hash_oid, ref_data), separator=None)
        if verbose: print 'nss.hash_buf %s\n%s' % (hash_oid_name, test_digest)

        self.assertEqual(test_digest, reference_digest,
                         msg='nss.hash_buf %s test failed reference=%s test=%s' % \
                             (hash_oid_name, reference_digest, test_digest))

        # Run the test using the lowest level hashing functions by specifying the hash algorithm.
        # The entire input data is supplied all at once in a single call.
        context = nss.create_digest_context(hash_oid)
        context.digest_begin()
        context.digest_op(ref_data)
        test_digest = nss.data_to_hex(context.digest_final(), separator=None)
        if verbose: print 'nss.digest_context %s\n%s' % (hash_oid_name, test_digest)

        self.assertEqual(test_digest, reference_digest,
                         msg='nss.digest_context %s test failed reference=%s test=%s' % \
                             (hash_oid_name, reference_digest, test_digest))

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

        self.assertEqual(test_digest, reference_digest,
                         msg='chunked nss.digest_context %s test failed reference=%s test=%s' % \
                             (hash_oid_name, reference_digest, test_digest))

    def test_md5(self):
        self.do_test('md5', 'md5sum', nss.md5_digest, nss.SEC_OID_MD5)

    def test_sha1(self):
        self.do_test('sha1', 'sha1sum', nss.sha1_digest, nss.SEC_OID_SHA1)

    def test_sha256(self):
        self.do_test('sha256', 'sha256sum', nss.sha256_digest, nss.SEC_OID_SHA256)

    def test_sha512(self):
        self.do_test('sha512', 'sha512sum', nss.sha512_digest, nss.SEC_OID_SHA512)


#-------------------------------------------------------------------------------

if __name__ == '__main__':
    unittest.main()
