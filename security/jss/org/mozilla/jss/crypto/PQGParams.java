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
 * The Original Code is the Netscape Security Services for Java.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998-2000
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

package org.mozilla.jss.crypto;

import java.math.BigInteger;
import java.security.spec.DSAParameterSpec;
import org.mozilla.jss.util.Assert;

/**
 * PQG parameters for DSA key generation, along with the seed, counter,
 * and H values for verification.
 * <p>This class has two main purposes:
 * generating PQG parameters and verifying PQG parameters.  To generate
 * PQG parameters, call one of the static <code>generate</code> methods.
 * They will return a new set of PQG paramters.  To verify existing PQG
 * parameters, create a new <code>PQGParams</code> object with the
 * constructor and call <code>paramsAreValid</code> on the object.
 *
 * <p>It is necessary to call <code>CryptoManager.initialize</code> before
 * using this class.
 *
 */
public class PQGParams extends DSAParameterSpec {

    /**
     * Creates a PQGParams object from a set of pre-computed DSA
     * parameters.
     *
     * @param P The DSA prime parameter.
     * @param Q The DSA sub-prime parameter.
     * @param G The DSA base parameter.
     * @param seed The Seed used to calculate P, Q, and G.
     * @param counter The Counter (C) used to calculate P, Q, and G.
     * @param H The H value used to generate P, Q, and G.
     */
    public PQGParams(BigInteger P, BigInteger Q, BigInteger G,
                    BigInteger seed, int counter, BigInteger H)
    {
        super(P, Q, G);
        this.seed = seed;
        this.counter = counter;
        this.H = H;
    }

    /**
     * Generates P, Q, and G parameters for DSA key generation.  Also
     *  provides the seed, counter, and H values for verification of the
     *  P, Q, and G.  The parameters are generated and then verified
     *  before being returned. The length of the Seed will equal the
     *  length of P.
     *
     * It is necessary to call one of the
     *  <code>CryptoManager.initialize</code> functions before calling
     *  this method.
     *
     * @param keySize The size of P in bits.  Keys generated by these P,
     *      Q, and G values will have this length.  Valid key sizes
     *      are multiples of 64 in the closed interval [512,1024].
     *      This also dictates the length of H and Seed.
     * @return A new set of P, Q, and G parameters, along with the Seed,
     *      Counter, and H values used to generate them.
     * @exception java.security.InvalidParameterException If the keySize
     *      is outside the bounds described by the DSA key pair
     *      generation algorithm.
     * @exception org.mozilla.jss.crypto.PQGParamGenException If an error
     *      occurs during the generation process.
     * @see org.mozilla.jss.CryptoManager#initialize
     */
    public static PQGParams
    generate(int keySize)
        throws java.security.InvalidParameterException,
                PQGParamGenException
    {
        PQGParams pqg = generateNative(keySize);
        if( ! pqg.paramsAreValid() ) {
            throw new PQGParamGenException(
                "Generated parameters did not verify correctly");
        }
        return pqg;
    }

    /**
     * Does the actual work of generation, but does not verify.
     */
    private static native PQGParams
    generateNative(int keySize)
        throws java.security.InvalidParameterException,
                PQGParamGenException;

    /**
     * Generates P, Q, and G parameters for DSA key generation.  Also
     *  provides the seed, counter, and H values for verification of the
     *  P, Q, and G.  The parameters are generated and then verified
     *  before being returned.
     *
     * It is necessary to call one of the
     *  <code>CryptoManager.initialize</code> functions before calling
     *  this method.
     *
     * @param keySize The size of P in bits.  Keys generated by these P,
     *      Q, and G values will have this length.  Valid key sizes
     *      are multiples of 64 in the closed interval [512,1024].
     *      This also dictates the length of H.
     * @param seedBytes The number of bytes in the Seed value used to
     *      generate P, Q, and G.  <code>seedBytes</code> must be
     *      from the closed interval [20,255].
     * @return A new set of P, Q, and G parameters, along with the Seed,
     *      Counter, and H values used to generate them.
     * @exception java.security.InvalidParameterException If the keySize
     *      or number of seed bytes is outside the bounds described by the
     *      DSA key pair generation algorithm.
     * @exception org.mozilla.jss.crypto.PQGParamGenException If an error
     *      occurs during the generation process.
     * @see org.mozilla.jss.CryptoManager#initialize
     */
    public static PQGParams
    generate(int keySize, int seedBytes)
        throws java.security.InvalidParameterException,
                PQGParamGenException
    {
        PQGParams pqg = generateNative(keySize, seedBytes);
        if( ! pqg.paramsAreValid() ) {
            throw new PQGParamGenException(
                "Generated parameters did not verify correctly");
        }
        return pqg;
    }

    /**
     * Does the actual work of generation, but does not verify.
     */
    private static native PQGParams
    generateNative(int keySize, int seedBytes)
        throws java.security.InvalidParameterException,
                PQGParamGenException;

    /**
     * Produces an unsigned byte-array representation of a BigInteger.
     *
     * <p>BigInteger adds an extra sign bit to the beginning of its byte
     * array representation.  In some cases this will cause the size
     * of the byte array to increase, which may be unacceptable for some
     * applications. This function returns a minimal byte array representing
     * the BigInteger without extra sign bits.
     *
     * @return An unsigned, big-endian byte array representation
     *      of a BigInteger.
     */
    public static byte[] BigIntegerToUnsignedByteArray(BigInteger big) {
        byte[] ret;

        // big must not be negative
        Assert._assert(big.signum() != -1);

        // bitLength is the size of the data without the sign bit.  If
        // it exactly fills an integral number of bytes, that means a whole
        // new byte will have to be added to accomodate the sign bit. In
        // this case we need to remove the first byte.
        if(big.bitLength() % 8 == 0) {
            byte[] array = big.toByteArray();
            // The first byte should just be sign bits
            Assert._assert( array[0] == 0 );
            ret = new byte[array.length-1];
            System.arraycopy(array, 1, ret, 0, ret.length);
        } else {
            ret = big.toByteArray();
        }
        return ret;
    }

    /**
     * Verifies the PQG parameters using the seed, counter, and H values.
     * @return true if the parameters verified correctly, false if they
     *      did not verify.
     */
    public boolean paramsAreValid() {
        return paramsAreValidNative(BigIntegerToUnsignedByteArray( getP() ),
                                    BigIntegerToUnsignedByteArray( getQ() ),
                                    BigIntegerToUnsignedByteArray( getG() ),
                                    BigIntegerToUnsignedByteArray( seed ),
                                    counter,
                                    BigIntegerToUnsignedByteArray( H ));
    }

    private native boolean paramsAreValidNative(byte[] P, byte[] Q, byte[]G,
        byte[] seed, int counter, byte[] H);

    /**
     * @return The Seed used to generate P, Q, and G.
     */
    public BigInteger getSeed() {
        return seed;
    }

    /**
     * @return The Counter (C) used to generate P, Q, and G.
     */
    public int getCounter() {
        return counter;
    }

    /**
     * @return The H value used to generate P, Q, and G.
     */
    public BigInteger getH() {
        return H;
    }

    private BigInteger  seed;
    private int         counter;
    private BigInteger  H;
}
