/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is the Netscape Security Services for Java.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998-2001 Netscape Communications Corporation.  All
 * Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU General Public License Version 2 or later (the
 * "GPL"), in which case the provisions of the GPL are applicable
 * instead of those above.  If you wish to allow use of your
 * version of this file only under the terms of the GPL and not to
 * allow others to use your version of this file under the MPL,
 * indicate your decision by deleting the provisions above and
 * replace them with the notice and other provisions required by
 * the GPL.  If you do not delete the provisions above, a recipient
 * may use your version of this file under either the MPL or the
 * GPL.
 */

package org.mozilla.jss.ssl;

import java.net.*;
import java.io.*;
import java.util.LinkedList;
import java.util.ListIterator;

public class SSLSocket extends java.net.Socket {

    /**
     * For sockets that get created by accept().
     */
    SSLSocket() throws IOException {
    }

    public SSLSocket(String host, int port)
        throws UnknownHostException, IOException
    {
        this(InetAddress.getByName(host), port, null, 0);
    }

    public SSLSocket(InetAddress address, int port)
        throws IOException
    {
        this(address, port, null, 0);
    }

    public SSLSocket(String host, int port, InetAddress localAddr,
        int localPort) throws IOException
    {
        this(InetAddress.getByName(host), port, localAddr, localPort);
    }

    public SSLSocket(InetAddress address, int port, InetAddress localAddr,
        int localPort) throws IOException
    {
        this(address, port, localAddr, localPort, null, null);
    }

    public SSLSocket(String host, int port, InetAddress localAddr,
        int localPort, SSLCertificateApprovalCallback certApprovalCallback,
        SSLClientCertificateSelectionCallback clientCertSelectionCallback)
            throws IOException
    {
        this(InetAddress.getByName(host), port, localAddr, localPort,
                certApprovalCallback, clientCertSelectionCallback);
    }

    /*
     * @param stream Ignored.
     */
    public SSLSocket(InetAddress address, int port, InetAddress localAddr,
        int localPort, boolean stream,
        SSLCertificateApprovalCallback certApprovalCallback,
        SSLClientCertificateSelectionCallback clientCertSelectionCallback)
            throws IOException
    {
        this(address, port, localAddr, localPort,
            certApprovalCallback, clientCertSelectionCallback);
    }

    public SSLSocket(InetAddress address, int port,
        InetAddress localAddr,
        int localPort, SSLCertificateApprovalCallback certApprovalCallback,
        SSLClientCertificateSelectionCallback clientCertSelectionCallback)
            throws IOException
    {
        this(address, address.getHostName(), port, localAddr, localPort,
            certApprovalCallback, clientCertSelectionCallback);
    }

    private SSLSocket(InetAddress address, String hostname, int port,
        InetAddress localAddr,
        int localPort, SSLCertificateApprovalCallback certApprovalCallback,
        SSLClientCertificateSelectionCallback clientCertSelectionCallback)
            throws IOException
    {
        /* create the socket */
        sockProxy = new SocketProxy(
            socketCreate(certApprovalCallback, clientCertSelectionCallback) );

        /* bind it to local address and port */
        if( localAddr != null || localPort > 0 ) {
            // bind because they specified a local address
            byte[] addrBA = null;
            if( localAddr != null ) {
                addrBA = localAddr.getAddress();
            }
            socketBind(addrBA, localPort);
        }

        /* connect to the remote socket */
        socketConnect(address.getAddress(), hostname, port);
    }


    public InetAddress getInetAddress() {
        try {
            int intAddr = getPeerAddressNative();
            InetAddress in;
            byte[] addr = new byte[4];
            addr[0] = (byte)((intAddr >>> 24) & 0xff);
            addr[1] = (byte)((intAddr >>> 16) & 0xff);
            addr[2] = (byte)((intAddr >>>  8) & 0xff);
            addr[3] = (byte)((intAddr       ) & 0xff);
            try {
            in = InetAddress.getByName(
                addr[0] + "." + addr[1] + "." + addr[2] + "." + addr[3] );
            } catch (java.net.UnknownHostException e) {
                in = null;
            }
            return in;
        } catch(SocketException e) {
            e.printStackTrace();
            return null;
        }
    }

    private native int getPeerAddressNative() throws SocketException;

    public InetAddress getLocalAddress() {
        try {
            int intAddr = getLocalAddressNative();
            InetAddress in;
            byte[] addr = new byte[4];
            addr[0] = (byte)((intAddr >>> 24) & 0xff);
            addr[1] = (byte)((intAddr >>> 16) & 0xff);
            addr[2] = (byte)((intAddr >>>  8) & 0xff);
            addr[3] = (byte)((intAddr       ) & 0xff);
            try {
            in = InetAddress.getByName(
                addr[0] + "." + addr[1] + "." + addr[2] + "." + addr[3] );
            } catch (java.net.UnknownHostException e) {
                in = null;
            }
            return in;
        } catch(SocketException e) {
            e.printStackTrace();
            return null;
        }
    }
    private native int getLocalAddressNative() throws SocketException;

    public int getLocalPort() {
        try {
            return getLocalPortNative();
        } catch(SocketException e) {
            e.printStackTrace();
            return 0;
        }
    }
    private native int getLocalPortNative() throws SocketException;

    public native int getPort();

    public InputStream getInputStream() throws IOException {
        return new SSLInputStream(this);
    }

    public OutputStream getOutputStream() throws IOException {
        return new SSLOutputStream(this);
    }


    public native void setTcpNoDelay(boolean on) throws SocketException;

    public native boolean getTcpNoDelay() throws SocketException;

    public native void setKeepAlive(boolean on) throws SocketException;

    public native boolean getKeepAlive(boolean on) throws SocketException;

    public void shutdownInput() throws IOException {
        shutdownNative(PR_SHUTDOWN_RCV);
    }

    public void shutdownOutput() throws IOException {
        shutdownNative(PR_SHUTDOWN_SEND);
    }

    private native void shutdownNative(int how) throws IOException;

    /**
     * param linger The time (in hundredths of a second) to linger for.
     */
    public native void setSoLinger(boolean on, int linger)
        throws SocketException;

    public native int getSoLinger() throws SocketException;

    public void setSoTimeout(int timeout) throws SocketException {
        this.timeout = timeout;
    }

    public int getSoTimeout() throws SocketException {
        return timeout;
    }

    //
    // XXX These aren't implemented by native code. Perhaps they could be.
    //
    public native void setSendBufferSize(int size) throws SocketException;
    public native int getSendBufferSize() throws SocketException;
    public native void setReceiveBufferSize(int size) throws SocketException;
    public native int getReceiveBufferSize(int size) throws SocketException;

    public void close() throws IOException {
        if( sockProxy != null ) {
            socketClose();
            sockProxy = null;
        }
    }

    private native void socketConnect(byte[] addr, String hostname, int port)
        throws SocketException;

    ////////////////////////////////////////////////////////////////////
    // SSL-specific stuff
    ////////////////////////////////////////////////////////////////////
    private LinkedList handshakeCompletedListeners = new LinkedList();

    public void addHandshakeCompletedListener(SSLHandshakeCompletedListener l) {
        handshakeCompletedListeners.add(l);
    }

    public void removeHandshakeCompletedListener(
            SSLHandshakeCompletedListener l) {
        handshakeCompletedListeners.remove(l);
    }

    private void notifyAllHandshakeListeners() {
        SSLHandshakeCompletedEvent event =
            new SSLHandshakeCompletedEvent(this);

        /* XXX NOT THREAD SAFE */
        ListIterator iter = handshakeCompletedListeners.listIterator(0);
        while( iter.hasNext() ) {
            SSLHandshakeCompletedListener l =
                (SSLHandshakeCompletedListener) iter.next();

            l.handshakeCompleted(event);
        }
    }
               

    public void enableSSL2(boolean enable) throws SocketException {
        setSSLOption(SSL_ENABLE_SSL2, enable);
    }

    static public void enableSSL2Default(boolean enable) throws SocketException{
        setSSLDefaultOption(SSL_ENABLE_SSL2, enable);
    }

    public void enableSSL3(boolean enable) throws SocketException {
        setSSLOption(SSL_ENABLE_SSL3, enable);
    }

    static public void enableSSL3Default(boolean enable) throws SocketException{
        setSSLDefaultOption(SSL_ENABLE_SSL3, enable);
    }

    public native void forceHandshake() throws SocketException;

    public native SSLSecurityStatus getStatus() throws SocketException;

    private InetAddress inetAddress;
    private int port;
    private SocketProxy sockProxy;
    private boolean open = false;

    /**
     * Enums. These must match the enums table in SSLSocket.c. This is
     * safer than copying the values of the C constants, which are subject
     * to change, into Java code.
     */
    private static final int SSL_ENABLE_SSL2 = 0;
    private static final int SSL_ENABLE_SSL3 = 1;
    private static final int TCP_NODELAY = 2;
    private static final int SO_KEEPALIVE = 3;
    private static final int PR_SHUTDOWN_RCV = 4;
    private static final int PR_SHUTDOWN_SEND = 5;

    /**
     * SO_TIMEOUT timeout in millis. I don't know why we have to keep it here
     * instead of setting a socket option.
     */
    private int timeout;

    private native void setSSLOption(int option, boolean on)
        throws SocketException;
    private static native void setSSLDefaultOption(int option, boolean on)
        throws SocketException;

    private native byte[] socketCreate(
        SSLCertificateApprovalCallback certApprovalCallback,
        SSLClientCertificateSelectionCallback clientCertSelectionCallback)
            throws IOException;

    private native void socketClose()
        throws IOException;
    /**
     * param addr The 4-byte IP address in network byte order.
     */
    private native void socketBind(byte[] addr, int port)
        throws SocketException;

    public static native void setCipherPreference( int cipher,
        boolean enable);

    native int socketAvailable()
        throws IOException;

    int read(byte[] b, int off, int len) throws IOException {
        return socketRead(b, off, len, timeout);
    }

    void write(byte[] b, int off, int len) throws IOException {
        socketWrite(b, off, len, timeout);
    }

    private native int socketRead(byte[] b, int off, int len, int timeout)
        throws IOException;

    private native void socketWrite(byte[] b, int off, int len, int timeout)
        throws IOException;

    public native void invalidateSession() throws SocketException;

    public void redoHandshake() throws SocketException {
        redoHandshake(false);
    }

    public native void redoHandshake(boolean flushCache) throws SocketException;

    protected void finalize() throws Throwable {
        close();
    }

    public final static int SSL2_RC4_128_WITH_MD5                  = 0xFF01;
    public final static int SSL2_RC4_128_EXPORT40_WITH_MD5         = 0xFF02;
    public final static int SSL2_RC2_128_CBC_WITH_MD5              = 0xFF03;
    public final static int SSL2_RC2_128_CBC_EXPORT40_WITH_MD5     = 0xFF04;
//  public final static int SSL2_IDEA_128_CBC_WITH_MD5             = 0xFF05;
    public final static int SSL2_DES_64_CBC_WITH_MD5               = 0xFF06;
    public final static int SSL2_DES_192_EDE3_CBC_WITH_MD5         = 0xFF07;

    public final static int SSL3_RSA_WITH_NULL_MD5                 = 0x0001;
//  public final static int SSL3_RSA_WITH_NULL_SHA                 = 0x0002;
    public final static int SSL3_RSA_EXPORT_WITH_RC4_40_MD5        = 0x0003;
    public final static int SSL3_RSA_WITH_RC4_128_MD5              = 0x0004;
    public final static int SSL3_RSA_WITH_RC4_128_SHA              = 0x0005;
    public final static int SSL3_RSA_EXPORT_WITH_RC2_CBC_40_MD5    = 0x0006;
//  public final static int SSL3_RSA_WITH_IDEA_CBC_SHA             = 0x0007;
//  public final static int SSL3_RSA_EXPORT_WITH_DES40_CBC_SHA     = 0x0008;
    public final static int SSL3_RSA_WITH_DES_CBC_SHA              = 0x0009;
    public final static int SSL3_RSA_WITH_3DES_EDE_CBC_SHA         = 0x000a;

//  public final static int SSL3_DH_DSS_EXPORT_WITH_DES40_CBC_SHA  = 0x000b;
//  public final static int SSL3_DH_DSS_WITH_DES_CBC_SHA           = 0x000c;
//  public final static int SSL3_DH_DSS_WITH_3DES_EDE_CBC_SHA      = 0x000d;
//  public final static int SSL3_DH_RSA_EXPORT_WITH_DES40_CBC_SHA  = 0x000e;
//  public final static int SSL3_DH_RSA_WITH_DES_CBC_SHA           = 0x000f;
//  public final static int SSL3_DH_RSA_WITH_3DES_EDE_CBC_SHA      = 0x0010;

//  public final static int SSL3_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA = 0x0011;
//  public final static int SSL3_DHE_DSS_WITH_DES_CBC_SHA          = 0x0012;
//  public final static int SSL3_DHE_DSS_WITH_3DES_EDE_CBC_SHA     = 0x0013;
//  public final static int SSL3_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA = 0x0014;
//  public final static int SSL3_DHE_RSA_WITH_DES_CBC_SHA          = 0x0015;
//  public final static int SSL3_DHE_RSA_WITH_3DES_EDE_CBC_SHA     = 0x0016;

//  public final static int SSL3_DH_ANON_EXPORT_WITH_RC4_40_MD5    = 0x0017;
//  public final static int SSL3_DH_ANON_WITH_RC4_128_MD5          = 0x0018;
//  public final static int SSL3_DH_ANON_EXPORT_WITH_DES40_CBC_SHA = 0x0019;
//  public final static int SSL3_DH_ANON_WITH_DES_CBC_SHA          = 0x001a;
//  public final static int SSL3_DH_ANON_WITH_3DES_EDE_CBC_SHA     = 0x001b;

    public final static int SSL3_FORTEZZA_DMS_WITH_NULL_SHA        = 0x001c;
    public final static int SSL3_FORTEZZA_DMS_WITH_FORTEZZA_CBC_SHA= 0x001d;
    public final static int SSL3_FORTEZZA_DMS_WITH_RC4_128_SHA     = 0x001e;

    public final static int SSL_RSA_FIPS_WITH_3DES_EDE_CBC_SHA     = 0xffe0;
    public final static int SSL_RSA_FIPS_WITH_DES_CBC_SHA          = 0xffe1;

    public final static int TLS_RSA_EXPORT1024_WITH_DES_CBC_SHA    = 0x0062;
    public final static int TLS_RSA_EXPORT1024_WITH_RC4_56_SHA     = 0x0064;
 
    public final static int TLS_DHE_DSS_EXPORT1024_WITH_DES_CBC_SHA = 0x0063;
    public final static int TLS_DHE_DSS_EXPORT1024_WITH_RC4_56_SHA  = 0x0065;
    public final static int TLS_DHE_DSS_WITH_RC4_128_SHA            = 0x0066;

}

class SocketProxy extends org.mozilla.jss.util.NativeProxy {

    public SocketProxy(byte[] pointer) {
        super(pointer);
    }

    /** 
     * Theoretically, we don't need to do anything, since SSLSocket should
     * call close() when it finalizes. When a socket is closed all its
     * resources are freed, and there's nothing left to release.
     */
    protected void releaseNativeResources() { }

    protected void finalize() throws Throwable {
        super.finalize();
    }
}
