#include "proxy/Socks4Client.h"

#include <sstream>
#include <iostream>

Socks4Client::Socks4Client(const char *host, int port, const char *auth,
    Proxy *proxy)
  : m_pProxy(proxy), m_sHost(host), m_iPort(port), m_sAuth(auth),
    m_eV4A(Socks4Client::FALLBACK)
{
    if(m_pProxy == NULL)
        m_pProxy = TCPClient::getInstance();
}

NetStream *Socks4Client::connect(const char *host, int port)
{
    Socks4Client::EV4A eV4A = m_eV4A;
#ifdef _DEBUG
    std::cerr << "Socks4Client: connecting...";
#endif
    NetStream *s = m_pProxy->connect(m_sHost.c_str(), m_iPort);
#ifdef _DEBUG
    std::cerr << " ok\n";
    std::cerr << "Socks4Client: forwarding request...";
#endif
    unsigned char req[8];
    req[0] = 0x04; // protocol version
    req[1] = 0x01; // command: TCP connect
    req[2] = m_iPort >> 8; // port number, big endian (NBE)
    req[3] = m_iPort & 0xFF;
    if(eV4A != Socks4Client::DO_USE)
    {
        // Attempts to resolve the hostname locally
        const SockAddress ip = Socket::resolve(host, SockAddress::V4);
        if(!ip.isValid())
        {
            if(eV4A == Socks4Client::FALLBACK)
            {
#ifdef _DEBUG
                std::cerr << " V4A activated (fallback)...";
#endif
                eV4A = Socks4Client::DO_USE;
            }
            else
            {
#ifdef _DEBUG
                std::cerr << " cannot resolve hostname\n";
#endif
                throw SocketUnknownHost();
            }
        }
        else
        {
#ifdef _DEBUG
            std::cerr << " DNS resolution ok...";
#endif
            unsigned int address = ip.v4toUint();
            req[4] = address >> 24; req[5] = address >> 16;
            req[6] = address >> 8; req[7] = address;
        }
    }
    if(eV4A == Socks4Client::DO_USE)
    {
        // Don't resolve hostnames, send them to the server
        // This is an extension of SOCKS v4, often referred to as v4a
        req[4] = 0x00;
        req[5] = 0x00;
        req[6] = 0x00;
        req[7] = 0x02;
    }
    s->send((char*)req, 8);
    s->send(m_sAuth.c_str(), m_sAuth.size() + 1); // auth, null-terminated
    if(eV4A == Socks4Client::DO_USE)
        s->send(host, strlen(host) + 1); // hostname, null-terminated
#ifdef _DEBUG
    std::cerr << " ok\n";
#endif
    
    return new Socks4Stream(s);
}

Socks4Stream::Socks4Stream(NetStream *s)
  : m_pStream(s), m_iRecvd(0)
{
}

Socks4Stream::~Socks4Stream()
{
    delete m_pStream;
}

void Socks4Stream::send(const char *data, size_t size)
    throw(SocketConnectionClosed)
{
    m_pStream->send(data, size);
}

int Socks4Stream::recv(char *data, size_t max, bool bWait)
    throw(SocketConnectionClosed)
{
    if(m_iRecvd >= 8)
        return m_pStream->recv(data, max, bWait);
    else
    {
        static char buf[8];
        m_iRecvd += m_pStream->recv(buf, 8 - m_iRecvd, bWait);
        // FIXME : shouldn't return 0 if bWait == true
        return 0;
    }
}

void Socks4Stream::registerSockets(SocketSetRegistrar *registrar)
{
    m_pStream->registerSockets(registrar);
}

void Socks4Stream::wait(int timeout)
{
    m_pStream->wait(timeout);
}
