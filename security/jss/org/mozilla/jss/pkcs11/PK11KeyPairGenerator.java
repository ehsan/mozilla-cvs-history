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

package org.mozilla.jss.pkcs11;

import org.mozilla.jss.crypto.*;
import org.mozilla.jss.util.*;
import org.mozilla.jss.asn1.*;
import java.math.BigInteger;
import java.security.*;
import java.security.SecureRandom;
import java.security.spec.AlgorithmParameterSpec;
import java.security.spec.DSAParameterSpec;


/**
 * A Key Pair Generator implemented using PKCS #11.
 *
 * @see org.mozilla.jss.crypto.PQGParams
 */
public final class PK11KeyPairGenerator
    extends org.mozilla.jss.crypto.KeyPairGeneratorSpi
{

    // opFlag constants: each of these flags specifies a crypto operation
    // the key will support.  Their values must match the same-named C
    // preprocessor macros defined in the PKCS #11 header pkcs11t.h.
    private static final int CKF_ENCRYPT = 0x00000100;
    private static final int CKF_DECRYPT = 0x00000200;
    private static final int CKF_SIGN = 0x00000800;
    private static final int CKF_SIGN_RECOVER = 0x00001000;
    private static final int CKF_VERIFY = 0x00002000;
    private static final int CKF_VERIFY_RECOVER = 0x00004000;
    private static final int CKF_WRAP = 0x00020000;
    private static final int CKF_UNWRAP = 0x00040000;
    private static final int CKF_DERIVE = 0x00080000;

    // A table for mapping SymmetricKey.Usage to opFlag.  This must be
    // synchronized with SymmetricKey.Usage.
    private static final int opFlagForUsage[] = {
        CKF_ENCRYPT,        /* 0 */
        CKF_DECRYPT,        /* 1 */
        CKF_SIGN,           /* 2 */
        CKF_SIGN_RECOVER,   /* 3 */
        CKF_VERIFY,         /* 4 */
        CKF_VERIFY_RECOVER, /* 5 */
        CKF_WRAP,           /* 6 */
        CKF_UNWRAP,         /* 7 */
        CKF_DERIVE          /* 8 */
    };

    // The crypto operations the key will support.  It is the logical OR
    // of the opFlag constants, each specifying a supported operation.
    private int opFlags = 0;
    private int opFlagsMask = 0;


    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////
    // Constructors
    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////

    /**
     * Constructor for PK11KeyPairGenerator.
     * @param token The PKCS #11 token that the keypair will be generated on.
     * @param algorithm The type of key that will be generated.  Currently,
     *      <code>KeyPairAlgorithm.RSA</code> , 
     *      <code>KeyPairAlgorithm.DSA</code> and
     *      <code>KeyPairAlgorithm.EC</code> are supported.
     * @throws NoSuchAlgorithmException
     * @throws TokenException
     */
    public PK11KeyPairGenerator(PK11Token token, KeyPairAlgorithm algorithm)
        throws NoSuchAlgorithmException, TokenException
    {
        Assert._assert(token!=null && algorithm!=null);

        mKeygenOnInternalToken = false;

        // Make sure we can do this kind of keygen
        if( ! token.doesAlgorithm(algorithm) ) {
            if( token.doesAlgorithm( algorithm.getAlgFamily() ) &&
                token.isWritable() )
            {
                // NSS will do the keygen on the internal module
                // and move the key to the token. We'll say this is
                // OK for now.
                mKeygenOnInternalToken = true;
            } else {
                throw new NoSuchAlgorithmException();
            }
        }

        this.token = token;
        this.algorithm = algorithm;
    }

    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////
    // Public Methods
    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////

    /**
     * Initializes this KeyPairGenerator with the given key strength.
     *
     * <p>For DSA key generation, pre-cooked PQG values will be used
     * be used if the key size is 512, 768, or 1024. Otherwise, an
     * InvalidParameterException will be thrown.
     *
     * @param strength The strength (size) of the keys that will be generated.
     * @param random <b>Ignored</b>
     * @exception InvalidParameterException If the key strength is not
     *      supported by the algorithm or this implementation.
     */
    public void initialize(int strength, SecureRandom random)
        throws InvalidParameterException
    {
        if(algorithm == KeyPairAlgorithm.RSA) {
            params =
                new RSAParameterSpec(strength, DEFAULT_RSA_PUBLIC_EXPONENT);
        } else if(algorithm == KeyPairAlgorithm.DSA) {
            if(strength==512) {
                params = PQG512;
            } else if(strength==768) {
                params = PQG768;
            } else if(strength==1024) {
                params = PQG1024;
            } else {
                throw new InvalidParameterException(
                    "In order to use pre-cooked PQG values, key strength must"+
                    "be 512, 768, or 1024.");
            }
        } else {
            Assert._assert( algorithm == KeyPairAlgorithm.EC );
            params = getCurve(strength);
        }
    }

    /**
     * Initializes this KeyPairGenerator with the given algorithm-specific
     * parameters.
     *
     * @param params The algorithm-specific parameters that will govern
     *      key pair generation.
     * @param random <b>Ignored</b>
     * @throws InvalidAlgorithmParameterException If the parameters
     *      are inappropriate for the key type or are not supported by
     *      this implementation.
     */
    public void initialize(AlgorithmParameterSpec params, SecureRandom random)
        throws InvalidAlgorithmParameterException
    {
        if(params == null) {
            Assert.notReached("Don't pass in null parameters");
            throw new InvalidAlgorithmParameterException();
        }

        if(algorithm == KeyPairAlgorithm.RSA) {
            if(! (params instanceof RSAParameterSpec) ) {
                throw new InvalidAlgorithmParameterException();
            }

            // Security library stores public exponent in an unsigned long
            if( ((RSAParameterSpec)params).getPublicExponent().bitLength()
                                                                        > 31) {
                throw new InvalidAlgorithmParameterException(
                        "RSA Public Exponent must fit in 31 or fewer bits.");
            }
        } else if ( algorithm == KeyPairAlgorithm.DSA ){
            if(! (params instanceof DSAParameterSpec) ) {
                throw new InvalidAlgorithmParameterException();
            }
        } else {
            Assert._assert( algorithm == KeyPairAlgorithm.EC);
            // requires JAVA 1.5 
            // if(! (params instanceof ECParameterSpec) ) {
            //   throw new InvalidAlgorithmParameterException();
            //}
            // requires JAVA 1.5 
            if(! (params instanceof PK11ParameterSpec) ) {
               throw new InvalidAlgorithmParameterException();
            }
	} // future add support for X509EncodedSpec 

        this.params = params;
    }

    /**
     * Generates a key pair on a token. Uses parameters if they were passed
     * in through a call to <code>initialize</code>, otherwise uses defaults.
     * @return
     * @throws TokenException
     */

    public KeyPair generateKeyPair()
        throws TokenException
    {
        if(algorithm == KeyPairAlgorithm.RSA) {
            if(params != null) {
                RSAParameterSpec rsaparams = (RSAParameterSpec)params;
                return generateRSAKeyPairWithOpFlags(
                                    token,
                                    rsaparams.getKeySize(),
                                    rsaparams.getPublicExponent().longValue(),
                                    temporaryPairMode,
                                    sensitivePairMode,
                                    extractablePairMode,
                                    opFlags, opFlagsMask);
            } else {
                return generateRSAKeyPairWithOpFlags(
                                    token,
                                    DEFAULT_RSA_KEY_SIZE,
                                    DEFAULT_RSA_PUBLIC_EXPONENT.longValue(),
                                    temporaryPairMode,
                                    sensitivePairMode,
                                    extractablePairMode,
                                    opFlags, opFlagsMask);
            }
        } else if(algorithm == KeyPairAlgorithm.DSA ) {
            if(params==null) {
                params = PQG1024;
            }
            DSAParameterSpec dsaParams = (DSAParameterSpec)params;
            return generateDSAKeyPairWithOpFlags(
                token,
                PQGParams.BigIntegerToUnsignedByteArray(dsaParams.getP()),
                PQGParams.BigIntegerToUnsignedByteArray(dsaParams.getQ()),
                PQGParams.BigIntegerToUnsignedByteArray(dsaParams.getG()),
                temporaryPairMode,
                sensitivePairMode,
                extractablePairMode,
                opFlags, opFlagsMask);
        } else {
            Assert._assert( algorithm == KeyPairAlgorithm.EC );
            // requires JAVA 1.5 for ECParameters.
            //
            //AlgorithmParameters ecParams = 
	        //			AlgorithmParameters.getInstance("ECParameters");
	        // ecParams.init(params);
            PK11ParameterSpec ecParams = (PK11ParameterSpec) params;

            return generateECKeyPairWithOpFlags(
                token,
		        ecParams.getEncoded(), /* curve */
                temporaryPairMode,
                sensitivePairMode,
                extractablePairMode,
                opFlags,
                opFlagsMask);
        } 
    }

    /**
     * @return true if the keypair generation will be done on the
     *      internal token and then moved to this token.
     */
    public boolean
    keygenOnInternalToken() {
        return mKeygenOnInternalToken;
    }

    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////
    // Native Implementation Methods
    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////

    /**
     * Generates an RSA key pair with the given size and public exponent.
     */
    private native KeyPair
    generateRSAKeyPair(PK11Token token, int keySize, long publicExponent,
            boolean temporary, int sensitive, int extractable)
        throws TokenException;

    /**
     * Generates an RSA key pair with the given size and public exponent.
     * Adds the ability to specify a set of flags and masks
     * to control how NSS generates the key pair.
     */
    private native KeyPair
    generateRSAKeyPairWithOpFlags(PK11Token token, int keySize, long publicExponent,
            boolean temporary, int sensitive, int extractable,
            int op_flags, int op_flags_mask)
        throws TokenException;

    /**
     * Generates a DSA key pair with the given P, Q, and G values.
     * P, Q, and G are stored as big-endian twos-complement octet strings.
     */
    private native KeyPair
    generateDSAKeyPair(PK11Token token, byte[] P, byte[] Q, byte[] G,
            boolean temporary, int sensitive, int extractable)
        throws TokenException;

    /**
     * Generates a DSA key pair with the given P, Q, and G values.
     * P, Q, and G are stored as big-endian twos-complement octet strings.
     * Adds the ability to specify a set of flags and masks
     * to control how NSS generates the key pair.
     */
    private native KeyPair
    generateDSAKeyPairWithOpFlags(PK11Token token, byte[] P, byte[] Q, byte[] G,
            boolean temporary, int sensitive, int extractable,
            int op_flags, int op_flags_mask)
        throws TokenException;


    /**
     * Generates a EC key pair with the given a curve.
     * Curves are stored as DER Encoded Parameters.
     */
    private native KeyPair
    generateECKeyPair(PK11Token token, byte[] Curve, 
            boolean temporary, int sensitive, int extractable)
        throws TokenException;
    /**
     * Generates a EC key pair with the given a curve.
     * Curves are stored as DER Encoded Parameters.
     * Adds the ability to specify a set of flags and masks
     * to control how NSS generates the key pair.
     */

    private native KeyPair
    generateECKeyPairWithOpFlags(PK11Token token, byte[] Curve, 
            boolean temporary, int sensitive, int extractable,
            int op_flags, int op_flags_mask)
        throws TokenException;

    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////
    // Defaults
    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////
    private static final int
    DEFAULT_RSA_KEY_SIZE = 2048;

    private static final BigInteger
    DEFAULT_RSA_PUBLIC_EXPONENT = BigInteger.valueOf(65537);

    //////////////////////////////////////////////////////////////////////
    // 1024-bit PQG parameters
    // These were generated and verified using the
    // org.mozilla.jss.crypto.PQGParams class.
    //////////////////////////////////////////////////////////////////////
    private static final String p1024= "135839652435190934085800139191680301864221874900900696919010316554114342871389066526982023513828891845682496590926522957486592076092819515303251959435284578074745022943475516750500021440278782993316814165831392449756706958779266394680687474113495272524355075654433600922751920548192539119568200162784715902571";
    private static final String q1024= "1289225024022202541601051225376429063716419728261";
    private static final String g1024= "9365079229044517973775853147905914670746128595481021901304748201231245287028018351117994470223961367498481071639148009750748608477086914440940765659773400233006204556380834403997210445996745757996115285802608489502160956380238739417382954486304730446079375880915926936667959108637040861595671319957190194969";
    private static final String h1024= "52442921523337940900621893014039829709890959980326720828933601226740749524581606283131306291278616835323956710592993182613544059214633911716533418368425327413628234095671352084418677611348898811691107611640282098605730539089258655659910952545940615065321018163879856499128636989240131903260975031351698627473";
    private static final String seed1024= "99294487279227522120410987308721952265545668206337642687581596155167227884587528576179879564731162765515189527566190299324927751299864501359105446993895763527646668003992063667962831489586752303126546478655915858883436326503676798267446073013163508014466163441488290854738816489359449082537210762470653355837";
    private static final int counter1024 = 159;

    /**
     * Pre-cooked PQG values for 1024-bit keypairs, along with the seed,
     * counter, and H values needed to verify them.
     */
    public static final
    PQGParams PQG1024 = new PQGParams(
                                    new BigInteger(p1024),
                                    new BigInteger(q1024),
                                    new BigInteger(g1024),
                                    new BigInteger(seed1024),
                                    counter1024,
                                    new BigInteger(h1024));


    //////////////////////////////////////////////////////////////////////
    // 768-bit PQG parameters
    // These were generated and verified using the
    // org.mozilla.jss.crypto.PQGParams class.
    //////////////////////////////////////////////////////////////////////
    private static final String p768 = "1334591549939035619289567230283054603122655003980178118026955029363553392594293499178687789871628588392413078786977899109276604404053531960657701920766542891720144660923735290663050045086516783083489369477138289683344192203747015183";
    private static final String q768 = "1356132865877303155992130272917916166541739006871";
    private static final String g768 = "1024617924160404238802957719732914916383807485923819254303813897112921288261546213295904612554364830820266594592843780972915270096284099079324418834215265083315386166747220804977600828688227714319518802565604893756612386174125343163";
    private static final String seed768 = "818335465751997015393064637168438154352349887221925302425470688493624428407506863871577128315308555744979456856342994235252156194662586703244255741598129996211081771019031721876068721218509213355334043303099174315838637885951947797";
    private static final int counter768 = 80;
    private static final String h768 = "640382699969409389484886950168366372251172224987648937408021020040753785108834000620831523080773231719549705102680417704245010958792653770817759388668805215557594892534053348624875390588773257372677159854630106242075665177245698591";

    /**
     * Pre-cooked PQG values for 768-bit keypairs, along with the seed,
     * counter, and H values needed to verify them.
     */
    public static final
    PQGParams PQG768 = new PQGParams(
                                    new BigInteger(p768),
                                    new BigInteger(q768),
                                    new BigInteger(g768),
                                    new BigInteger(seed768),
                                    counter768,
                                    new BigInteger(h768));


    //////////////////////////////////////////////////////////////////////
    // 512-bit PQG parameters
    // These were generated and verified using the
    // org.mozilla.jss.crypto.PQGParams class.
    //////////////////////////////////////////////////////////////////////
    private static final String p512 = "6966483207285155416780416172202915863379050665227482416115451434656043093992853756903066653962454938528584622842487778598918381346739078775480034378802841";
    private static final String q512 = "1310301134281640075932276656367326462518739803527";
    private static final String g512 = "1765808308320938820731237312304158486199455718816858489736043318496656574508696475222741642343469219895005992985361010111736160340009944528784078083324884";
    private static final String h512 = "1166033533097555931825481846268490827226947692615252570752313574243187654088977281409544725210974913958636100321681636002587474728477655589742540645702652";
    private static final String seed512 = "1823071686803672528716836609217295942310764795778335243232708299998660956064222751939859670873282519585591423918449571513004815205037154878988595168291600";
    private static final int counter512 = 186;

    /**
     * Pre-cooked PQG values for 512-bit keypairs, along with the seed,
     * counter, and H values needed to verify them.
     */
    public static final
    PQGParams PQG512 = new PQGParams(
                                    new BigInteger(p512),
                                    new BigInteger(q512),
                                    new BigInteger(g512),
                                    new BigInteger(seed512),
                                    counter512,
                                    new BigInteger(h512));

    ///////////////////////////////////////////////////////////////////////
    // Test the PQG parameters
    ///////////////////////////////////////////////////////////////////////
    private static boolean defaultsTested = false;
    private static synchronized void
    testDefaults() {
        if(Debug.DEBUG && !defaultsTested) {
            Assert._assert(PQG1024.paramsAreValid());
            Assert._assert(PQG768.paramsAreValid());
            Assert._assert(PQG512.paramsAreValid());
            defaultsTested = true;
        }
    }

    public void temporaryPairs(boolean temp) {
        temporaryPairMode = temp;
    }

    public void sensitivePairs(boolean sensitive) {
        sensitivePairMode = sensitive ? 1 : 0;
    }

    public void extractablePairs(boolean extractable) {
        extractablePairMode = extractable ? 1 : 0;
    }

    /**
     * Sets the requested key usages desired for the 
     * generated key pair. 
     * This allows the caller to suggest how NSS generates the key pair.
     * @param usages List of desired key usages. 
     * @param usages_mask Corresponding mask for the key usages.
     * if a usages is desired, make sure it is in the mask as well.
     */

    public void setKeyPairUsages(org.mozilla.jss.crypto.KeyPairGeneratorSpi.Usage[] usages, 
                                 org.mozilla.jss.crypto.KeyPairGeneratorSpi.Usage[] usages_mask) {

        this.opFlags = 0;
        this.opFlagsMask = 0;

        if(usages != null) {
            for( int i = 0; i < usages.length; i++ ) {
                if( usages[i] != null ) {
                    this.opFlags |= opFlagForUsage[usages[i].getVal()];
                }
            }
        }

        if(usages_mask != null) {
            for( int i = 0; i < usages_mask.length; i++ ) {
                if( usages_mask[i] != null ) {
                    this.opFlagsMask |= opFlagForUsage[usages_mask[i].getVal()];
                }
            }
        }
    }

    //
    // requires JAVA 1.5
    //
    //private AlgorithmParameterSpec getCurve(int strength) {
    //}


    private static final OBJECT_IDENTIFIER ANSI_X962_PRIME_CURVE =
	new OBJECT_IDENTIFIER( new long[] { 1, 2, 840, 10045, 3, 1 } );
    private static final OBJECT_IDENTIFIER ANSI_X962_BINARY_CURVE =
	new OBJECT_IDENTIFIER( new long[] { 1, 2, 840, 10045, 3, 0 } );
    private static final OBJECT_IDENTIFIER SECG_EC_CURVE =
	new OBJECT_IDENTIFIER( new long[] { 1, 3, 132, 0 } );

    // ANSI Prime curves
    static final OBJECT_IDENTIFIER CURVE_ANSI_P192V1 
	= ANSI_X962_PRIME_CURVE.subBranch(1);
    static final OBJECT_IDENTIFIER CURVE_ANSI_P192V2 
	= ANSI_X962_PRIME_CURVE.subBranch(2);
    static final OBJECT_IDENTIFIER CURVE_ANSI_P192V3 
	= ANSI_X962_PRIME_CURVE.subBranch(3);
    static final OBJECT_IDENTIFIER CURVE_ANSI_P239V1 
	= ANSI_X962_PRIME_CURVE.subBranch(4);
    static final OBJECT_IDENTIFIER CURVE_ANSI_P239V2 
	= ANSI_X962_PRIME_CURVE.subBranch(5);
    static final OBJECT_IDENTIFIER CURVE_ANSI_P239V3 
	= ANSI_X962_PRIME_CURVE.subBranch(6);
    static final OBJECT_IDENTIFIER CURVE_ANSI_P256V1 
	= ANSI_X962_PRIME_CURVE.subBranch(7);

    // ANSI Binary curves
    static final OBJECT_IDENTIFIER CURVE_ANSI_PNB163V1 
	=ANSI_X962_BINARY_CURVE.subBranch(1);
    static final OBJECT_IDENTIFIER CURVE_ANSI_PNB163V2 
	=ANSI_X962_BINARY_CURVE.subBranch(2);
    static final OBJECT_IDENTIFIER CURVE_ANSI_PNB163V3 
	=ANSI_X962_BINARY_CURVE.subBranch(3);
    static final OBJECT_IDENTIFIER CURVE_ANSI_PNB176V1 
	=ANSI_X962_BINARY_CURVE.subBranch(4);
    static final OBJECT_IDENTIFIER CURVE_ANSI_TNB191V1 
	=ANSI_X962_BINARY_CURVE.subBranch(5);
    static final OBJECT_IDENTIFIER CURVE_ANSI_TNB191V2 
	=ANSI_X962_BINARY_CURVE.subBranch(6);
    static final OBJECT_IDENTIFIER CURVE_ANSI_TNB191V3 
	=ANSI_X962_BINARY_CURVE.subBranch(7);
    static final OBJECT_IDENTIFIER CURVE_ANSI_ONB191V4 
	=ANSI_X962_BINARY_CURVE.subBranch(8);
    static final OBJECT_IDENTIFIER CURVE_ANSI_ONB191V5 
	=ANSI_X962_BINARY_CURVE.subBranch(9);
    static final OBJECT_IDENTIFIER CURVE_ANSI_PNB208W1 
	=ANSI_X962_BINARY_CURVE.subBranch(10);
    static final OBJECT_IDENTIFIER CURVE_ANSI_TNB239V1 
	=ANSI_X962_BINARY_CURVE.subBranch(11);
    static final OBJECT_IDENTIFIER CURVE_ANSI_TNB239V2 
	=ANSI_X962_BINARY_CURVE.subBranch(12);
    static final OBJECT_IDENTIFIER CURVE_ANSI_TNB239V3 
	=ANSI_X962_BINARY_CURVE.subBranch(13);
    static final OBJECT_IDENTIFIER CURVE_ANSI_ONB239V4 
	=ANSI_X962_BINARY_CURVE.subBranch(14);
    static final OBJECT_IDENTIFIER CURVE_ANSI_ONB239V5 
	=ANSI_X962_BINARY_CURVE.subBranch(15);
    static final OBJECT_IDENTIFIER CURVE_ANSI_PNB272W1 
	=ANSI_X962_BINARY_CURVE.subBranch(16);
    static final OBJECT_IDENTIFIER CURVE_ANSI_PNB304W1 
	=ANSI_X962_BINARY_CURVE.subBranch(17);
    static final OBJECT_IDENTIFIER CURVE_ANSI_TNB359V1 
	=ANSI_X962_BINARY_CURVE.subBranch(18);
    static final OBJECT_IDENTIFIER CURVE_ANSI_PNB368W1 
	=ANSI_X962_BINARY_CURVE.subBranch(19);
    static final OBJECT_IDENTIFIER CURVE_ANSI_TNB431R1 
	=ANSI_X962_BINARY_CURVE.subBranch(20);

    // SEG Prime curves
    static final OBJECT_IDENTIFIER CURVE_SECG_P112R1
	= SECG_EC_CURVE.subBranch(6);
    static final OBJECT_IDENTIFIER CURVE_SECG_P112R2
	= SECG_EC_CURVE.subBranch(7);
    static final OBJECT_IDENTIFIER CURVE_SECG_P128R1
	= SECG_EC_CURVE.subBranch(28);
    static final OBJECT_IDENTIFIER CURVE_SECG_P128R2
	= SECG_EC_CURVE.subBranch(29);
    static final OBJECT_IDENTIFIER CURVE_SECG_P160K1
	= SECG_EC_CURVE.subBranch(9);
    static final OBJECT_IDENTIFIER CURVE_SECG_P160R1
	= SECG_EC_CURVE.subBranch(8);
    static final OBJECT_IDENTIFIER CURVE_SECG_P160R2
	= SECG_EC_CURVE.subBranch(30);
    static final OBJECT_IDENTIFIER CURVE_SECG_P192K1
	= SECG_EC_CURVE.subBranch(31);
    static final OBJECT_IDENTIFIER CURVE_SECG_P224K1
	= SECG_EC_CURVE.subBranch(32);
    static final OBJECT_IDENTIFIER CURVE_SECG_P224R1
	= SECG_EC_CURVE.subBranch(33);
    static final OBJECT_IDENTIFIER CURVE_SECG_P256K1
	= SECG_EC_CURVE.subBranch(10);
    static final OBJECT_IDENTIFIER CURVE_SECG_P384R1
	= SECG_EC_CURVE.subBranch(34);
    static final OBJECT_IDENTIFIER CURVE_SECG_P521R1
	= SECG_EC_CURVE.subBranch(35);

    // SEG Binary curves
    static final OBJECT_IDENTIFIER CURVE_SECG_T113R1
	= SECG_EC_CURVE.subBranch(4);
    static final OBJECT_IDENTIFIER CURVE_SECG_T113R2
	= SECG_EC_CURVE.subBranch(5);
    static final OBJECT_IDENTIFIER CURVE_SECG_T131R1
	= SECG_EC_CURVE.subBranch(22);
    static final OBJECT_IDENTIFIER CURVE_SECG_T131R2
	= SECG_EC_CURVE.subBranch(23);
    static final OBJECT_IDENTIFIER CURVE_SECG_T163K1
	= SECG_EC_CURVE.subBranch(1);
    static final OBJECT_IDENTIFIER CURVE_SECG_T163R1
	= SECG_EC_CURVE.subBranch(2);
    static final OBJECT_IDENTIFIER CURVE_SECG_T163R2
	= SECG_EC_CURVE.subBranch(15);
    static final OBJECT_IDENTIFIER CURVE_SECG_T193R1
	= SECG_EC_CURVE.subBranch(24);
    static final OBJECT_IDENTIFIER CURVE_SECG_T193R2
	= SECG_EC_CURVE.subBranch(25);
    static final OBJECT_IDENTIFIER CURVE_SECG_T233K1
	= SECG_EC_CURVE.subBranch(26);
    static final OBJECT_IDENTIFIER CURVE_SECG_T233R1
	= SECG_EC_CURVE.subBranch(27);
    static final OBJECT_IDENTIFIER CURVE_SECG_T239K1
	= SECG_EC_CURVE.subBranch(3);
    static final OBJECT_IDENTIFIER CURVE_SECG_T283K1
	= SECG_EC_CURVE.subBranch(16);
    static final OBJECT_IDENTIFIER CURVE_SECG_T283R1
	= SECG_EC_CURVE.subBranch(17);
    static final OBJECT_IDENTIFIER CURVE_SECG_T409K1
	= SECG_EC_CURVE.subBranch(36);
    static final OBJECT_IDENTIFIER CURVE_SECG_T409R1
	= SECG_EC_CURVE.subBranch(37);
    static final OBJECT_IDENTIFIER CURVE_SECG_T571K1
	= SECG_EC_CURVE.subBranch(38);
    static final OBJECT_IDENTIFIER CURVE_SECG_T571R1
	= SECG_EC_CURVE.subBranch(39);

    private AlgorithmParameterSpec getCurve(int strength) 
        throws InvalidParameterException
    {
	OBJECT_IDENTIFIER oid;
	switch (strength) {
	case 112:
	    oid = CURVE_SECG_P112R1; // == WTLS-6
        // Can't get to curve SECG P-112R2
        // Can't get to curve WTLS-8 (No oid for WTLS-8)
	    break;
        case 113:
	    oid = CURVE_SECG_T113R1; // == WTLS-4
        // Can't get to curve SECG T-113R2
        // Can't get to curve WTLS-1 (No oid for WTLS-1)
	    break;
	case 128:
	    oid = CURVE_SECG_P128R1;
        // Can't get to curve SECG P-128R2
	    break;
	case 131:
	    oid = CURVE_SECG_T131R1;
        // Can't get to curve SECG T-131R2
	    break;
	case 160:
	    oid = CURVE_SECG_P160R1; // == WTLS-7 (TLS-16)
        // Can't get to curve SECG P-160K1 (TLS-15)
        // Can't get to curve SECG P-160R2 (TLS-17)
        // Can't get to curve WTLS-9 (No oid for WTLS-9)
	    break;
	case 163:
	    oid = CURVE_SECG_T163K1; // == NIST K-163 == WTLS-3 (TLS-1)
        // Can't get to curve ANSI C2-PNB163V1 == WTLS-5
        // Can't get to curve ANSI C2-PNB163V2
        // Can't get to curve ANSI C2-PNB163V3
        // Can't get to curve SECG T-163R1 (TLS-2)
	// Can't get to curve SECG T-163R2 == NIST B-163 (TLS-3)
	    break;
	case 176:
	    oid = CURVE_ANSI_PNB176V1;
	    break;
	case 191:
	    oid = CURVE_ANSI_TNB191V1;
	// Can't get to curve ANSI C2-TNB191V2
	// Can't get to curve ANSI C2-TNB191V3
	// Can't get to curve ANSI C2-ONB191V4
	// Can't get to curve ANSI C2-ONB191V5
	    break;
	case 192:
	    oid = CURVE_ANSI_P192V1; // == NIST P-192 == SECG P-192R1 (TLS-19)
	// Can't get to curve ANSI P-192V2
	// Can't get to curve ANSI P-192V3
        // Can't get to curve SECG P-192K1 (TLS-18)
	    break;
	case 193:
	    oid = CURVE_SECG_T193R1;  // (TLS-4)
        // Can't get to curve SECG T-193R2 // (TLS-5)
	    break;
	case 208:
	    oid = CURVE_ANSI_PNB208W1;
	    break;
	case 224:
	    oid = CURVE_SECG_P224R1; // == NIST P-224 == WTLS-12 (TLS-21)
        // Can't get to curve SECG P-224K1 (TLS-20)
	    break;
	case 233:
	    oid = CURVE_SECG_T233R1; // == NIST B-233 == WTLS-11 (TLS-7)
        // Can't get to curve SECG T-233K1 == NIST K-233 == WTLS-10 (TLS-6)
	    break;
	case 239:
	    oid = CURVE_SECG_T239K1; // (TLS8)
        // Can't get to curve ANSI P-239V1
        // Can't get to curve ANSI P-239V2
	// Can't get to curve ANSI P-239V3
	// Can't get to curve ANSI C2-TNB239V1
	// Can't get to curve ANSI C2-TNB239V2
	// Can't get to curve ANSI C2-TNB239V3
	// Can't get to curve ANSI C2-ONB239V4
	// Can't get to curve ANSI C2-ONB239V5
	    break;
	case 256:
	    oid = CURVE_ANSI_P256V1; // == NIST P-256 == SECG P-256R1 (TLS-23)
	// Can't get to curve SECG P-256K1 (TLS-22)
	    break;
	case 272:
	    oid = CURVE_ANSI_PNB272W1;
	    break;
	case 283:
	    oid = CURVE_SECG_T283R1; // == NIST B-283 (TLS-10)
        // Can't get to curve SECG T-283K1 == NIST K-283 (TLS-9)
	    break;
	case 304:
	    oid = CURVE_ANSI_PNB304W1;
	    break;
	case 359:
	    oid = CURVE_ANSI_TNB359V1;
	    break;
	case 368:
	    oid = CURVE_ANSI_PNB368W1;
	    break;
	case 384:
	    oid = CURVE_SECG_P384R1; // == NIST P-384 (TLS-24)
	    break;
	case 409:
	    oid = CURVE_SECG_T409R1; // == NIST B-409 (TLS-12)
        // Can't get to curve SECG T-409K1 == NIST K-409 (TLS-11)
	    break;
	case 431:
	    oid = CURVE_ANSI_TNB431R1;
	    break;
	case 521:
	    oid = CURVE_SECG_P521R1; // == NIST P-521 (TLS-25)
	    break;
	case 571:
	    oid = CURVE_SECG_T571R1; // == NIST B-571 (TLS-14)
        // Can't get to curve SECG T-571K1 == NIST K-571 (TLS-13)
	    break;
	default:
             throw new InvalidParameterException();
        }
        return new PK11ParameterSpec(ASN1Util.encode(oid));
    }


    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////
    // Private members
    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////
    private PK11Token token;
    private AlgorithmParameterSpec params;
    private KeyPairAlgorithm algorithm;
    private boolean mKeygenOnInternalToken;
    private boolean temporaryPairMode = false;
    //  1: sensitive
    //  0: insensitive
    // -1: sensitive if temporaryPairMode is false,
    //     insensitive if temporaryPairMode is true
    //     (the default depends on temporaryPairMode for backward
    //     compatibility)
    private int sensitivePairMode = -1;
    //  1: extractable
    //  0: unextractable
    // -1: unspecified (token dependent)
    private int extractablePairMode = -1;
}
