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
 * The Original Code is Network Security Services for Java.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

package org.mozilla.jss.SecretDecoderRing;

import java.security.*;
import javax.crypto.*;
import org.mozilla.jss.crypto.*;
import org.mozilla.jss.util.Assert;

/**
 * Creates, finds, and deletes keys for SecretDecoderRing.
 */
public class KeyManager {
    private static final int KEYID_LEN = 16;

    private static final String RNG_ALG = "pkcs11prng";
    private static final String RNG_PROVIDER = "Mozilla-JSS";

    /**
     * The default key generation algorithm, currently DES3.
     */
    public static final KeyGenAlgorithm DEFAULT_KEYGEN_ALG =
        KeyGenAlgorithm.DES3;

    /**
     * The default key size. This is only relevant for algorithms
     * with variable-length keys, such as AES.
     */
    public static final int DEFAULT_KEYSIZE = 0;

    private CryptoToken token;

    /**
     * Creates a new KeyManager using the given CryptoToken.
     * @param token The token on which this KeyManager operates.
     */
    public KeyManager(CryptoToken token) {
        this.token = token;
    }

    /**
     * Generates an SDR key with the default algorithm and key size.
     * The default algorithm is stored in the constant DEFAULT_KEYGEN_ALG.
     * The default key size is stored in the constant DEFAULT_KEYSIZE.
     * @return The keyID of the generated key. A random keyID will be chosen
     *  that is not currently used on the token. The keyID must be stored
     *  by the application in order to use this key for encryption in the
     *  future.
     */
    public byte[] generateKey() throws TokenException {
        return generateKey(DEFAULT_KEYGEN_ALG, DEFAULT_KEYSIZE);
    }

    /**
     * Generates an SDR key with the given algorithm and key size.
     * @return The keyID of the generated key. A random keyID will be chosen
     *  that is not currently used on the token. The keyID must be stored
     *  by the application in order to use this key for encryption in the
     *  future.
     */
    public byte[] generateKey(KeyGenAlgorithm alg, int keySize)
            throws TokenException
    {
        byte[] keyID = generateUnusedKeyID();
        generateKeyNative(token, alg, keyID, keySize);
        return keyID;
    }

    private native void generateKeyNative(CryptoToken token,
        KeyGenAlgorithm alg, byte[] keyID, int keySize);

    /**
     * Generates a key ID that is currently unused on this token.
     * The caller is responsible for synchronization issues that may arise
     * if keys are generated by different threads.
     */
    private byte[] generateUnusedKeyID() throws TokenException {
      try {
        SecureRandom rng = SecureRandom.getInstance(RNG_ALG, RNG_PROVIDER);
        byte[] keyID = new byte[KEYID_LEN];
        do {
            rng.nextBytes(keyID);
        } while( keyExists(keyID) );
        return keyID;
      } catch(NoSuchAlgorithmException nsae) {
            throw new RuntimeException("No such algorithm: " + RNG_ALG);
      } catch(NoSuchProviderException nspe) {
            throw new RuntimeException("No such provider: " + RNG_PROVIDER);
      }
    }

    private boolean keyExists(byte[] keyid) throws TokenException {
        return (lookupKey(Encryptor.DEFAULT_ENCRYPTION_ALG, keyid) != null);
    }
    
    /**
     * Looks up the key on this token with the given algorithm and key ID.
     * @param alg The algorithm that this key will be used for.
     * This is necessary because it will be stored along with the 
     * key for later use by the security library. It should match
     * the actual algorithm of the key you are looking for. If you 
     * pass in a different algorithm and try to use the key that is returned,
     * the results are undefined.
     * @return The key, or <tt>null</tt> if the key is not found.
     */
    public SecretKey lookupKey(EncryptionAlgorithm alg, byte[] keyid)
        throws TokenException
    {
        SymmetricKey k = lookupKeyNative(token, alg, keyid);
        if( k == null ) {
            return null;
        } else {
            return new SecretKeyFacade(k);
        }
    }

    private native SymmetricKey lookupKeyNative(CryptoToken token,
        EncryptionAlgorithm alg, byte[] keyid) throws TokenException;

    /**
     * Deletes the key with the given keyID from this token.
     * @throws InvalidKeyException If the key does not exist on this token.
     */
    public void deleteKey(byte[] keyID) throws TokenException,
        InvalidKeyException
    {
        deleteKey(lookupKey(Encryptor.DEFAULT_ENCRYPTION_ALG, keyID));
    }

    /**
     * Deletes this key from this token.
     * @throws InvalidKeyException If the key does not reside on this token,
     * or is not a JSS key.
     */
    public void deleteKey(SecretKey key) throws TokenException,
            InvalidKeyException
    {
        if( ! (key instanceof SecretKeyFacade) ) {
            throw new InvalidKeyException("Key must be a JSS key");
        }
        deleteKeyNative(token, ((SecretKeyFacade)key).key);
    }

    private native void deleteKeyNative(CryptoToken token, SymmetricKey key)
        throws TokenException;
}
