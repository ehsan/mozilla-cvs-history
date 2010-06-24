#!/usr/bin/python

import unittest

from nss.error import NSPRError
import nss.error as nss_error
import nss.nss as nss

class ExceptionNotRaised(Exception):
    """
    Exception raised when an *expected* exception is *not* raised during a
    unit test.
    """
    msg = 'expected %s'

    def __init__(self, expected):
        self.expected = expected

    def __str__(self):
        return self.msg % self.expected.__name__


class ExceptionWrongErrno(Exception):
    """
    Exception raised when an *expected* exception is raised with the wrong errno.
    """
    msg = 'expected %s with errno = %s but got errno = %s'

    def __init__(self, expected, expected_errno, actual_errno):
        self.expected = expected
        self.expected_errno = expected_errno
        self.actual_errno = actual_errno

    def __str__(self):
        return self.msg % (self.expected.__name__, self.expected_errno, self.actual_errno)


def assertRaisesErrno(exception, errno, callback, *args, **kw):
    """
    Tests that the expected exception is raised; raises ExceptionNotRaised
    if test fails.
    """
    try:
        callback(*args, **kw)
        raise ExceptionNotRaised(exception)
    except exception, e:
        if e.errno != errno:
            raise ExceptionWrongErrno(exception, errno, e.errno)


class TestCertName(unittest.TestCase):
    subject_name = 'CN=www.redhat.com,OU=Web Operations,O=Red Hat Inc,L=Raleigh,ST=North Carolina,C=US'
    cn_name = 'www.redhat.com'
    ou_name = 'Web Operations'
    o_name  = 'Red Hat Inc'
    l_name  = 'Raleigh'
    st_name = 'North Carolina'
    c_name  = 'US'

    def setUp(self):
        nss.nss_init_nodb()

    def tearDown(self):
        nss.nss_shutdown()

    def test_ava_from_name(self):
        ava = nss.AVA('cn', self.cn_name)
        self.assertEqual(str(ava), "CN=%s" % self.cn_name)
        
    def test_ava_from_oid_tag(self):
        ava = nss.AVA(nss.SEC_OID_AVA_COMMON_NAME, self.cn_name)
        self.assertEqual(str(ava), "CN=%s" % self.cn_name)
        self.assertRaises(ValueError, nss.AVA, nss.SEC_OID_UNKNOWN, self.cn_name)
        
    def test_ava_from_oid_string(self):
        ava = nss.AVA('2.5.4.3', self.cn_name)
        self.assertEqual(str(ava), "CN=%s" % self.cn_name)
        self.assertRaises(ValueError, nss.oid_tag, 'OID.99.99.99.99')
        self.assertRaises(KeyError, nss.AVA, 'foo', self.cn_name)
        
    def test_oid_dotted_decimal(self):
        self.assertEqual(nss.oid_dotted_decimal(nss.SEC_OID_AVA_COMMON_NAME),
                         'OID.2.5.4.3')
        self.assertEqual(nss.oid_tag('OID.2.5.4.3'),
                         nss.SEC_OID_AVA_COMMON_NAME)
        self.assertEqual(nss.oid_tag('2.5.4.3'),
                         nss.SEC_OID_AVA_COMMON_NAME)
        self.assertRaises(ValueError, nss.oid_tag, 'OID.99.99.99.99')

    def test_ava_from_bad_type(self):
        self.assertRaises(TypeError, nss.AVA, (), self.cn_name)
        
    def test_ava_compare(self):
        cn_ava1 = nss.AVA('cn', self.cn_name)
        cn_ava2 = nss.AVA('cn', self.cn_name)
        cn_ava3 = nss.AVA('cn', self.cn_name+'A')
        ou_ava = nss.AVA('ou', self.ou_name)

        self.assertEqual(cmp(cn_ava1, cn_ava2), 0)
        self.assertEqual(cmp(cn_ava1, ou_ava), -1)
        self.assertEqual(cmp(cn_ava1, cn_ava3), -1)

    def test_rdn_compare(self):
        cn_rdn1 = nss.RDN(nss.AVA('cn', self.cn_name))
        cn_rdn2 = nss.RDN(nss.AVA('cn', self.cn_name))
        cn_rdn3 = nss.RDN(nss.AVA('cn', self.cn_name+'A'))
        ou_rdn  = nss.RDN(nss.AVA('ou', self.ou_name))

        self.assertEqual(cmp(cn_rdn1, cn_rdn2), 0)
        self.assertEqual(cmp(cn_rdn1, ou_rdn), -1)
        self.assertEqual(cmp(cn_rdn1, cn_rdn3), -1)

    def test_rdn_create(self):
        cn_ava = nss.AVA('cn', self.cn_name)
        ou_ava = nss.AVA('ou', self.ou_name)

        rdn = nss.RDN()
        self.assertEqual(len(rdn), 0)
        self.assertEqual(str(rdn), '')

        rdn = nss.RDN(cn_ava)
        self.assertEqual(len(rdn), 1)
        self.assertEqual(str(rdn), 'CN=%s' % (self.cn_name))
        self.assertEqual(rdn[0], cn_ava)
        self.assertEqual(rdn['cn'], [cn_ava])

        rdn = nss.RDN(cn_ava, ou_ava)
        self.assertEqual(len(rdn), 2)
        self.assertEqual(str(rdn), 'CN=%s+OU=%s' % (self.cn_name, self.ou_name))

        self.assertEqual(rdn[0], cn_ava)
        self.assertEqual(rdn[1], ou_ava)

        i = 0
        for ava in rdn:
            if   i == 0: self.assertEqual(ava, cn_ava)
            elif i == 1: self.assertEqual(ava, ou_ava)
            else: self.fail("excess ava's")
            i += 1

        self.assertEqual(list(rdn), [cn_ava, ou_ava])
        self.assertEqual(rdn[:],    [cn_ava, ou_ava])

        self.assertEqual(rdn['cn'], [cn_ava])
        self.assertEqual(rdn['ou'], [ou_ava])

        self.assertEqual(str(rdn[0]), "CN=%s" % self.cn_name)
        self.assertEqual(str(rdn[1]), "OU=%s" % self.ou_name)

        self.assertEqual(rdn['2.5.4.3'], [cn_ava])
        self.assertEqual(rdn.has_key('cn'), True)
        self.assertEqual(rdn.has_key('2.5.4.3'), True)
        self.assertEqual(rdn.has_key('st'), False)

        self.assertEqual(list(rdn), [cn_ava, ou_ava])
        self.assertEqual(rdn[0:2], [cn_ava, ou_ava])

        i = 0
        for ava in rdn:
            if i == 0: self.assertEqual(rdn[i], cn_ava)
            if i == 1: self.assertEqual(rdn[i], ou_ava)

        try:
            rdn['st']
            self.fail("expected KeyError for 'st'")
        except KeyError:
            pass

        try:
            rdn['junk']
            self.fail("expected KeyError for 'junk'")
        except KeyError:
            pass

    def test_name(self):
        cn_rdn = nss.RDN(nss.AVA('cn', self.cn_name))
        ou_rdn = nss.RDN(nss.AVA('ou', self.ou_name))
        o_rdn  = nss.RDN(nss.AVA('o',  self.o_name))
        l_rdn  = nss.RDN(nss.AVA('l',  self.l_name))
        st_rdn = nss.RDN(nss.AVA('st', self.st_name))
        c_rdn  = nss.RDN(nss.AVA('c',  self.c_name))

        name = nss.DN(self.subject_name)
        self.assertEqual(str(name), self.subject_name)

        self.assertEqual(name[0], c_rdn)
        self.assertEqual(name[1], st_rdn)
        self.assertEqual(name[2], l_rdn)
        self.assertEqual(name[3], o_rdn)
        self.assertEqual(name[4], ou_rdn)
        self.assertEqual(name[5], cn_rdn)

        self.assertEqual(len(name), 6)

        i = 0
        for rdn in name:
            if   i == 0: self.assertEqual(rdn, c_rdn)
            elif i == 1: self.assertEqual(rdn, st_rdn)
            elif i == 2: self.assertEqual(rdn, l_rdn)
            elif i == 3: self.assertEqual(rdn, o_rdn)
            elif i == 4: self.assertEqual(rdn, ou_rdn)
            elif i == 5: self.assertEqual(rdn, cn_rdn)
            else: self.fail("excess rdn's")
            i += 1

        self.assertEqual(list(name), [c_rdn, st_rdn, l_rdn, o_rdn, ou_rdn, cn_rdn])        
        self.assertEqual(name[:],    [c_rdn, st_rdn, l_rdn, o_rdn, ou_rdn, cn_rdn])        

        self.assertEqual(name['c'],  [c_rdn])
        self.assertEqual(name['st'], [st_rdn])
        self.assertEqual(name['l'],  [l_rdn])
        self.assertEqual(name['o'],  [o_rdn])
        self.assertEqual(name['ou'], [ou_rdn])
        self.assertEqual(name['cn'], [cn_rdn])

        self.assertEqual(name.email_address, None)
        self.assertEqual(name.common_name, self.cn_name)
        self.assertEqual(name.country_name, self.c_name)
        self.assertEqual(name.locality_name, self.l_name)
        self.assertEqual(name.state_name, self.st_name)
        self.assertEqual(name.org_name, self.o_name)
        self.assertEqual(name.dc_name, None)
        self.assertEqual(name.org_unit_name, self.ou_name)
        self.assertEqual(name.cert_uid, None)

        name = nss.DN()
        self.assertEqual(str(name), '')

        name = nss.DN([])
        self.assertEqual(str(name), '')

        name = nss.DN(())
        self.assertEqual(str(name), '')

        name = nss.DN('')
        self.assertEqual(str(name), '')

        self.assertRaises(TypeError, nss.DN, 1)

        name.add_rdn(cn_rdn)
        self.assertEqual(name[0], cn_rdn)
        self.assertEqual(name['cn'], [cn_rdn])
        self.assertEqual(str(name), 'CN=%s' % self.cn_name)

        name.add_rdn(ou_rdn)
        self.assertEqual(name[0], cn_rdn)
        self.assertEqual(name[1], ou_rdn)
        self.assertEqual(name['cn'], [cn_rdn])
        self.assertEqual(name['ou'], [ou_rdn])
        self.assertEqual(str(name), 'OU=%s,CN=%s' % (self.ou_name,self.cn_name))

        name = nss.DN(cn_rdn, ou_rdn)
        self.assertEqual(name[0], cn_rdn)
        self.assertEqual(name[1], ou_rdn)
        self.assertEqual(name['cn'], [cn_rdn])
        self.assertEqual(name['ou'], [ou_rdn])
        self.assertEqual(str(name), 'OU=%s,CN=%s' % (self.ou_name,self.cn_name))

        self.assertEqual(name.has_key('cn'), True)
        self.assertEqual(name.has_key('ou'), True)
        self.assertEqual(name.has_key('st'), False)

    def test_oid(self):
        self.assertEqual(nss.oid_str('2.5.4.3'),                   'X520 Common Name')
        self.assertEqual(nss.oid_str(nss.SEC_OID_AVA_COMMON_NAME), 'X520 Common Name')
        self.assertEqual(nss.oid_str('SEC_OID_AVA_COMMON_NAME'),   'X520 Common Name')
        self.assertEqual(nss.oid_str('AVA_COMMON_NAME'),           'X520 Common Name')
        self.assertEqual(nss.oid_str('cn'),                        'X520 Common Name')

        self.assertEqual(nss.oid_tag_name('2.5.4.3'),                   'SEC_OID_AVA_COMMON_NAME')
        self.assertEqual(nss.oid_tag_name(nss.SEC_OID_AVA_COMMON_NAME), 'SEC_OID_AVA_COMMON_NAME')
        self.assertEqual(nss.oid_tag_name('SEC_OID_AVA_COMMON_NAME'),   'SEC_OID_AVA_COMMON_NAME')
        self.assertEqual(nss.oid_tag_name('AVA_COMMON_NAME'),           'SEC_OID_AVA_COMMON_NAME')
        self.assertEqual(nss.oid_tag_name('cn'),                        'SEC_OID_AVA_COMMON_NAME')

        self.assertEqual(nss.oid_dotted_decimal('2.5.4.3'),                   'OID.2.5.4.3')
        self.assertEqual(nss.oid_dotted_decimal(nss.SEC_OID_AVA_COMMON_NAME), 'OID.2.5.4.3')
        self.assertEqual(nss.oid_dotted_decimal('SEC_OID_AVA_COMMON_NAME'),   'OID.2.5.4.3')
        self.assertEqual(nss.oid_dotted_decimal('AVA_COMMON_NAME'),           'OID.2.5.4.3')
        self.assertEqual(nss.oid_dotted_decimal('cn'),                        'OID.2.5.4.3')

        self.assertEqual(nss.oid_tag('2.5.4.3'),                   nss.SEC_OID_AVA_COMMON_NAME)
        self.assertEqual(nss.oid_tag(nss.SEC_OID_AVA_COMMON_NAME), nss.SEC_OID_AVA_COMMON_NAME)
        self.assertEqual(nss.oid_tag('SEC_OID_AVA_COMMON_NAME'),   nss.SEC_OID_AVA_COMMON_NAME)
        self.assertEqual(nss.oid_tag('AVA_COMMON_NAME'),           nss.SEC_OID_AVA_COMMON_NAME)
        self.assertEqual(nss.oid_tag('cn'),                        nss.SEC_OID_AVA_COMMON_NAME)

    def test_multi_value(self):
        subject='CN=www.redhat.com,OU=engineering,OU=boston+OU=westford,C=US'
        cn_ava   = nss.AVA('cn', self.cn_name)
        ou1_ava1 = nss.AVA('ou', 'boston')
        ou1_ava2 = nss.AVA('ou', 'westford')
        ou2_ava1 = nss.AVA('ou', 'engineering')
        c_ava    = nss.AVA('c',  self.c_name)

        cn_rdn  = nss.RDN(cn_ava)
        ou1_rdn = nss.RDN(ou1_ava1, ou1_ava2)
        ou2_rdn = nss.RDN(ou2_ava1)
        c_rdn   = nss.RDN(c_ava)

        name = nss.DN(subject)

        self.assertEqual(len(name), 4)

        self.assertEqual(name['cn'], [cn_rdn])
        self.assertEqual(name['ou'], [ou1_rdn, ou2_rdn])
        self.assertEqual(name['c'],  [c_rdn])

        rdn = name['ou'][0]
        self.assertEqual(len(rdn), 2)
        self.assertEqual(rdn, ou1_rdn)
        self.assertEqual(rdn[0], ou1_ava1)
        self.assertEqual(rdn[1], ou1_ava2)
        self.assertEqual(list(rdn), [ou1_ava1, ou1_ava2])
        self.assertEqual(rdn[:], [ou1_ava1, ou1_ava2])

if __name__ == '__main__':
    unittest.main()
