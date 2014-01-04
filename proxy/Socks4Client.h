#ifndef SOCKS4CLIENT_H
#define SOCKS4CLIENT_H

#include "proxy/Proxy.h"

#ifndef ENABLE_SOCKS4_CLIENT
#error "The SOCKS4 client has not been enabled in the configuration"
#endif

/**
 * Proxy client SOCKS v4.
 */
class Socks4Client : public Proxy {

public:
    /**
     * Indicates when we want to use the v4a extension, which allows not to
     * resolve hostnames locally.
     */
    enum EV4A {
        DO_USE,   //!< Always use it
        FALLBACK, //!< Use it if local resolve fails
        DONT_USE  //!< Never use it
    };

private:
    Proxy *m_pProxy;
    std::string m_sHost;
    int m_iPort;
    std::string m_sAuth;
    EV4A m_eV4A;

public:
    Socks4Client(const char *host, int port, const char *auth = "",
        Proxy *proxy = NULL);
    virtual ~Socks4Client() {}
    virtual NetStream *connect(const char *host, int port);
    inline void useV4A(Socks4Client::EV4A eV4A)
    { m_eV4A = eV4A; }
    
};

/**
 * Stream through a Socks4 proxy.
 *
 * Dump copy of the bytes in both directions, with the exception of the initial
 * response header which is removed (8 bytes).
 */
class Socks4Stream : public NetStream {

private:
    NetStream *m_pStream;
    size_t m_iRecvd;

public:
    Socks4Stream(NetStream *s);
    ~Socks4Stream();
    virtual void send(const char *data, size_t size)
        throw(SocketConnectionClosed);
    virtual int recv(char *data, size_t max, bool bWait = true)
        throw(SocketConnectionClosed);

    void registerSockets(SocketSetRegistrar *registrar);
    void wait(int timeout = 0);

};

#endif
