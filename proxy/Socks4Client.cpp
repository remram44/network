#include "Socks4Client.h"

#include <sstream>
#include <iostream>

Socks4Client::Socks4Client(const char *hote, int port, const char *auth,
    Proxy *proxy)
  : m_pProxy(proxy), m_sHost(hote), m_iPort(port), m_sAuth(auth),
    m_eV4A(Socks4Client::FALLBACK)
{
    if(m_pProxy == NULL)
        m_pProxy = new TCPClient;
}

NetStream *Socks4Client::Connect(const char *hote, int port)
{
    Socks4Client::EV4A eV4A = m_eV4A;
#ifdef _DEBUG
    std::cerr << "Socks4Client: connecting...";
#endif
    NetStream *s = m_pProxy->Connect(m_sHost.c_str(), m_iPort);
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
        const unsigned char *ip = Socket::Resolve(hote);
        if(ip == NULL)
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
            req[4] = ip[0]; req[5] = ip[1]; req[6] = ip[2]; req[7] = ip[3];
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
    s->Send((char*)req, 8);
    s->Send(m_sAuth.c_str(), m_sAuth.size() + 1); // auth, null-terminated
    if(eV4A == Socks4Client::DO_USE)
        s->Send(hote, strlen(hote) + 1); // hostname, null-terminated
#ifdef _DEBUG
    std::cerr << " ok\n";
#endif
    
    return new Socks4Stream(s);
}

Socks4Stream::Socks4Stream(NetStream *s)
  : m_pStream(s), m_iRecvd(0)
{
}

void Socks4Stream::Send(const char *data, size_t size)
    throw(SocketConnectionClosed)
{
    m_pStream->Send(data, size);
}

int Socks4Stream::Recv(char *data, size_t max, bool bWait)
    throw(SocketConnectionClosed)
{
    if(m_iRecvd >= 8)
        return m_pStream->Recv(data, max, bWait);
    else
    {
        static char buf[8];
        m_iRecvd += m_pStream->Recv(buf, 8 - m_iRecvd, bWait);
        // FIXME : shouldn't return 0 if bWait == true
        return 0;
    }
}

Socket *Socks4Stream::UnderlyingSocket()
{
    return m_pStream->UnderlyingSocket();
}
