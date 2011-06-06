#!/usr/bin/python

import sys
import os
import subprocess
import shlex
import unittest

from nss.error import NSPRError
import nss.error as nss_error
import nss.nss as nss

#-------------------------------------------------------------------------------

verbose = False
certdir = 'pki'
db_passwd = 'db_passwd'
pkcs12_file_password = 'pk12_passwd'

read_nickname = 'test_user'
read_pkcs12_file = '%s.p12' % read_nickname

write_export_file = False
export_nickname = 'test_server'

#-------------------------------------------------------------------------------

def run_cmd(cmd):
    if verbose: print "running command: %s" % cmd

    args = shlex.split(cmd)
    subprocess.check_call(args, stdout=subprocess.PIPE)

#-------------------------------------------------------------------------------

def password_callback(slot, retry):
    return db_passwd

#-------------------------------------------------------------------------------

def nickname_collision_callback(old_nickname, cert):
    cancel = False
    new_nickname = cert.make_ca_nickname()
    return new_nickname, cancel


#-------------------------------------------------------------------------------

def load_tests(loader, tests, pattern):
    suite = unittest.TestSuite()
    tests = loader.loadTestsFromNames(['test_pkcs12.TestPKCS12Decoder.test_read',
                                       'test_pkcs12.TestPKCS12Decoder.test_import',
                                       'test_pkcs12.TestPKCS12Export.test_export',
                                       ])
    suite.addTests(tests)
    return suite

#-------------------------------------------------------------------------------

class TestPKCS12Decoder(unittest.TestCase):
    def setUp(self):
        nss.nss_init_read_write(certdir)
        nss.set_password_callback(password_callback)
        nss.pkcs12_set_nickname_collision_callback(nickname_collision_callback)
        nss.pkcs12_enable_all_ciphers()

    def tearDown(self):
        nss.nss_shutdown()

    def test_read(self):
        if verbose: print "test_read"
        cmd='pk12util -o %s -n %s -d pki -K %s -W %s' % \
            (read_pkcs12_file, read_nickname, db_passwd, pkcs12_file_password)
        run_cmd(cmd)

        slot = nss.get_internal_key_slot()
        pkcs12 = nss.PKCS12Decoder(read_pkcs12_file, pkcs12_file_password, slot)

        self.assertEqual(len(pkcs12), 3)
        cert_bag_count = 0
        key_seen = None
        for bag in pkcs12:
            if bag.type == nss.SEC_OID_PKCS12_V1_CERT_BAG_ID:
                self.assertIsNone(bag.shroud_algorithm_id)
                cert_bag_count += 1
                if key_seen is None:
                    key_seen = bag.has_key
                elif key_seen is True:
                    self.assertIs(bag.has_key, False)
                elif key_seen is False:
                    self.assertIs(bag.has_key, True)
                else:
                    self.fail("unexpected has_key for bag type = %s(%d)" % (bag.has_key, nss.oid_tag_name(bag.type), bag.type))

            elif bag.type == nss.SEC_OID_PKCS12_V1_PKCS8_SHROUDED_KEY_BAG_ID:
                self.assertIsInstance(bag.shroud_algorithm_id, nss.AlgorithmID)
                self.assertIs(bag.has_key, False)
            else:
                self.fail("unexpected bag type = %s(%d)" % (nss.oid_tag_name(bag.type), bag.type))

        self.assertEqual(cert_bag_count, 2)

    def test_import(self):
        if verbose: print "test_import"
        cmd='certutil -d pki -D -n %s' % (read_nickname)
        run_cmd(cmd)

        slot = nss.get_internal_key_slot()
        pkcs12 = nss.PKCS12Decoder(read_pkcs12_file, pkcs12_file_password, slot)
        slot.authenticate()
        pkcs12.database_import()

#-------------------------------------------------------------------------------

class TestPKCS12Export(unittest.TestCase):
    def setUp(self):
        nss.nss_init(certdir)
        nss.set_password_callback(password_callback)
        nss.pkcs12_enable_all_ciphers()

    def tearDown(self):
        nss.nss_shutdown()

    def test_export(self):
        if verbose: print "test_export"
        pkcs12_data = nss.pkcs12_export(export_nickname, pkcs12_file_password)
        if write_export_file:
            p12_file_path = os.path.join(os.path.dirname(sys.argv[0]), "%s.p12" % export_nickname)
            f = open(p12_file_path, 'w')
            f.write(pkcs12_data)
            f.close()

if __name__ == '__main__':
    unittest.main()
