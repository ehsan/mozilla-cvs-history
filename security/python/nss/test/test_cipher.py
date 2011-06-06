#!/usr/bin/python

import sys
import os
import unittest

import nss.nss as nss

#-------------------------------------------------------------------------------

verbose = False
mechanism = nss.CKM_DES_CBC_PAD
plain_text = "Encrypt me!"
key = "e8:a7:7c:e2:05:63:6a:31"
iv = "e4:bb:3b:d3:c3:71:2e:58"
in_filename = sys.argv[0]
chunk_size = 128

#-------------------------------------------------------------------------------

def setup_contexts(mechanism, key, iv):
    # Get a PK11 slot based on the cipher
    slot = nss.get_best_slot(mechanism)

    # If key was supplied use it, otherwise generate one
    if key:
        if verbose:
            print "using supplied key data"
            print "key:\n%s" % (key)
        key_si = nss.SecItem(nss.read_hex(key))
        sym_key = nss.import_sym_key(slot, mechanism, nss.PK11_OriginUnwrap,
                                     nss.CKA_ENCRYPT, key_si)
    else:
        if verbose:
            print "generating key data"
        sym_key = slot.key_gen(mechanism, None, slot.get_best_key_length(mechanism))

    # If initialization vector was supplied use it, otherwise set it to None
    if iv:
        if verbose:
            print "supplied iv:\n%s" % (iv)
        iv_data = nss.read_hex(iv)
        iv_si = nss.SecItem(iv_data)
        iv_param = nss.param_from_iv(mechanism, iv_si)
    else:
        iv_length = nss.get_iv_length(mechanism)
        if iv_length > 0:
            iv_data = nss.generate_random(iv_length)
            iv_si = nss.SecItem(iv_data)
            iv_param = nss.param_from_iv(mechanism, iv_si)
            if verbose:
                print "generated %d byte initialization vector: %s" % \
                    (iv_length, nss.data_to_hex(iv_data, separator=":"))
        else:
            iv_param = None

    # Create an encoding context
    encoding_ctx = nss.create_context_by_sym_key(mechanism, nss.CKA_ENCRYPT,
                                                 sym_key, iv_param)

    # Create a decoding context
    decoding_ctx = nss.create_context_by_sym_key(mechanism, nss.CKA_DECRYPT,
                                                 sym_key, iv_param)

    return encoding_ctx, decoding_ctx

#-------------------------------------------------------------------------------

class TestCipher(unittest.TestCase):
    def setUp(self):
        nss.nss_init_nodb()
        self.encoding_ctx, self.decoding_ctx = setup_contexts(mechanism, key, iv)

    def tearDown(self):
        del self.encoding_ctx
        del self.decoding_ctx
        nss.nss_shutdown()

    def test_string(self):
        if verbose:
            print "Plain Text:\n%s" % (plain_text)

        # Encode the plain text by feeding it to cipher_op getting cipher text back.
        # Append the final bit of cipher text by calling digest_final
        cipher_text = self.encoding_ctx.cipher_op(plain_text)
        cipher_text += self.encoding_ctx.digest_final()

        if verbose:
            print "Cipher Text:\n%s" % (nss.data_to_hex(cipher_text, separator=":"))

        # Decode the cipher text by feeding it to cipher_op getting plain text back.
        # Append the final bit of plain text by calling digest_final
        decoded_text = self.decoding_ctx.cipher_op(cipher_text)
        decoded_text += self.decoding_ctx.digest_final()

        if verbose:
            print "Decoded Text:\n%s" % (decoded_text)

        # Validate the encryption/decryption by comparing the decoded text with
        # the original plain text, they should match.
        self.assertEqual(decoded_text, plain_text)

        self.assertNotEqual(cipher_text, plain_text)

    def test_file(self):
        encrypted_filename = os.path.basename(in_filename) + ".encrypted"
        decrypted_filename = os.path.basename(in_filename) + ".decrypted"

        in_file = open(in_filename, "r")
        encrypted_file = open(encrypted_filename, "w")

        if verbose:
            print "Encrypting file \"%s\" to \"%s\"" % (in_filename, encrypted_filename)

        # Encode the data read from a file in chunks
        while True:
            # Read a chunk of data until EOF, encrypt it and write the encrypted data
            in_data = in_file.read(chunk_size)
            if len(in_data) == 0:   # EOF
                break
            encrypted_data = self.encoding_ctx.cipher_op(in_data)
            encrypted_file.write(encrypted_data)
        # Done encoding the input, get the final encoded data, write it, close files
        encrypted_data = self.encoding_ctx.digest_final()
        encrypted_file.write(encrypted_data)
        in_file.close()
        encrypted_file.close()

        # Decode the encoded file in a similar fashion
        if verbose:
            print "Decrypting file \"%s\" to \"%s\"" % (encrypted_filename, decrypted_filename)

        encrypted_file = open(encrypted_filename, "r")
        decrypted_file = open(decrypted_filename, "w")
        while True:
            # Read a chunk of data until EOF, encrypt it and write the encrypted data
            in_data = encrypted_file.read(chunk_size)
            if len(in_data) == 0:   # EOF
                break
            decrypted_data = self.decoding_ctx.cipher_op(in_data)
            decrypted_file.write(decrypted_data)
        # Done encoding the input, get the final encoded data, write it, close files
        decrypted_data = self.decoding_ctx.digest_final()
        decrypted_file.write(decrypted_data)
        encrypted_file.close()
        decrypted_file.close()

        # Validate the encryption/decryption by comparing the decoded text with
        # the original plain text, they should match.
        in_data        = open(in_filename).read()
        encrypted_data = open(encrypted_filename).read()
        decrypted_data = open(decrypted_filename).read()
        if decrypted_data != in_data:
            result = 1
            print "FAILED! decrypted_data != in_data"

        if encrypted_data == in_data:
            result = 1
            print "FAILED! encrypted_data == in_data"

        # clean up
        os.unlink(encrypted_filename)
        os.unlink(decrypted_filename)



#-------------------------------------------------------------------------------

if __name__ == '__main__':
    unittest.main()
