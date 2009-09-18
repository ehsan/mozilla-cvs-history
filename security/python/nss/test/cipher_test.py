#!/usr/bin/python

# used for local testing
#from test_util import insert_build_dir_into_path
#insert_build_dir_into_path()

import sys
import os
import getopt
import nss.nss as nss

verbose = 0

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

    # If initial value was supplied use it, otherwise set it to None
    if iv:
        if verbose:
            print "iv:\n%s" % (iv)
        iv_si = nss.SecItem(nss.read_hex(iv))
        iv_param = nss.param_from_iv(mechanism, iv_si)
    else:
        iv_param = None

    # Create an encoding context
    encoding_ctx = nss.create_context_by_sym_key(mechanism, nss.CKA_ENCRYPT,
                                                 sym_key, iv_param)

    # Create a decoding context
    decoding_ctx = nss.create_context_by_sym_key(mechanism, nss.CKA_DECRYPT,
                                                 sym_key, iv_param)

    return encoding_ctx, decoding_ctx

def simple_test(encoding_ctx, decoding_ctx, plain_text):
    result = 0

    if verbose:
        print "Plain Text:\n%s" % (plain_text)

    # Encode the plain text by feeding it to cipher_op getting cipher text back.
    # Append the final bit of cipher text by calling digest_final
    cipher_text = encoding_ctx.cipher_op(plain_text)
    cipher_text += encoding_ctx.digest_final()

    if verbose:
        print "Cipher Text:\n%s" % (nss.data_to_hex(cipher_text, separator=":"))

    # Decode the cipher text by feeding it to cipher_op getting plain text back.
    # Append the final bit of plain text by calling digest_final
    decoded_text = decoding_ctx.cipher_op(cipher_text)
    decoded_text += decoding_ctx.digest_final()

    if verbose:
        print "Decoded Text:\n%s" % (decoded_text)

    # Validate the encryption/decryption by comparing the decoded text with
    # the original plain text, they should match.
    if decoded_text != plain_text:
        result = 1
        print "FAILED! decoded_text != plain_text"

    if cipher_text == plain_text:
        result = 1
        print "FAILED! cipher_text == plain_text"

    return result

def file_test(encoding_ctx, decoding_ctx, in_filename, chunk_size):
    result = 0

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
        encrypted_data = encoding_ctx.cipher_op(in_data)
        encrypted_file.write(encrypted_data)
    # Done encoding the input, get the final encoded data, write it, close files
    encrypted_data = encoding_ctx.digest_final()
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
        decrypted_data = decoding_ctx.cipher_op(in_data)
        decrypted_file.write(decrypted_data)
    # Done encoding the input, get the final encoded data, write it, close files
    decrypted_data = decoding_ctx.digest_final()
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

    return result

def usage():
    print '''\
digest_test [-v -h] filename
    filename     file to be used as test data
    -v --verbose turn on verbose output
    -h --help    print usage
    -s --size    number of octets processed in one iteration
    -m --mech    encryption mechanism name (e.g. CKM_*)
                 name is case insensitive, CKM_ prefix is optional
    -t --text    plain text
    -k --key     key (in hexadecimal format)
    -i --iv      parameter initial value (in hexadecimal format)
'''

def main():
    global verbose
    mechanism = nss.CKM_DES_CBC_PAD
    plain_text = "Encrypt me!"
    key = "e8:a7:7c:e2:05:63:6a:31"
    iv = "e4:bb:3b:d3:c3:71:2e:58"
    in_filename = None
    chunk_size = 128

    try:
        opts, args = getopt.getopt(sys.argv[1:], "hvs:m:t:k:i:",
                                   ["help", "verbose", "size=", "mechanism=", "text=",
                                    "key=", "iv="])
    except getopt.GetoptError, err:
        print str(err) # will print something like "option -a not recognized"
        usage()
        sys.exit(2)
    verbose = False
    for o, a in opts:
        if o in ("-s", "--size"):
            chunk_size = int(a)
        elif o in ("-m", "--mech"):
            try:
                mechanism = nss.key_mechanism_type_from_name(a)
            except Exception, e:
                print "error with mech argument (%s)" % (e)
                sys.exit(2)
        elif o in ("-t", "--text"):
            plain_text = a
        elif o in ("-k", "--key"):
            key = a
        elif o in ("-i", "--iv"):
            iv = a
        elif o in ("-v", "--verbose"):
            verbose += 1
        elif o in ("-h", "--help"):
            usage()
            sys.exit(0)
        else:
            assert False, "unhandled option"

    if (len(args) > 1):
        print "expected single file name"
        usage()
        sys.exit(2)
    elif (len(args) == 1):
        in_filename = args[0]

    nss.nss_init_nodb()

    result = 0
    encoding_ctx, decoding_ctx = setup_contexts(mechanism, key, iv)
    result += simple_test(encoding_ctx, decoding_ctx, plain_text)
    if in_filename:
        # In theory we should be able to reuse a context by calling finalize()
        # on it, however at the time of this writing it only works for
        # digest contexts, not encryption/decryption contexts
        # so as a workaround we just create the contexts again
        #
        #encoding_ctx.finalize()
        #decoding_ctx.finalize()
        encoding_ctx, decoding_ctx = setup_contexts(mechanism, key, iv)
        result += file_test(encoding_ctx, decoding_ctx, in_filename, chunk_size)

    if result == 0:
        print "SUCCESS"
    else:
        print "FAILED %d tests" % (result)

    sys.exit(result)


if __name__ == "__main__":
    main()
