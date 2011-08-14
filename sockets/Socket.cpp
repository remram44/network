#include "Socket.h"

const char *SocketFatalError::what()
{
    return "Fatal error";
}

const char *SocketUnknownHost::what()
{
    return "Unknown host";
}

const char *SocketConnectionRefused::what()
{
    return "Connection refused";
}

const char *SocketConnectionClosed::what()
{
    return "Connection closed";
}

const char *SocketCantUsePort::what()
{
    return "Can't use port";
}


/*============================================================================*/

SockAddress4::SockAddress4(unsigned int address)
  : a(address >> 24), b(address >> 16), c(address >> 8), d(address)
{
}

SockAddress4::SockAddress4(unsigned char a_, unsigned char b_, unsigned char c_,
        unsigned char d_)
  : a(a_), b(b_), c(c_), d(d_)
{
}

unsigned int SockAddress4::toUint() const
{
    return (a << 24) | (b << 16) | (c << 8) | d;
}

SockAddress::EType SockAddress4::type() const
{
    return SockAddress::V4;
}


/*============================================================================*/

Socket::Socket(int sock)
  : m_iSocket(sock)
{
    if(m_iSocket == -1)
        throw SocketFatalError();
}

Socket::~Socket()
{
    if(m_iSocket != -1)
    {
#ifndef __WIN32__
        shutdown(m_iSocket, SHUT_RDWR);
        close(m_iSocket);
#else
        closesocket(m_iSocket);
#endif
        m_iSocket = -1;
    }
}

bool Socket::operator==(const Socket& s)
{
    return m_iSocket == s.GetSocket();
}

bool Socket::operator<(const Socket& s)
{
    return m_iSocket < s.GetSocket();
}

void Socket::Init() throw(SocketFatalError)
{
#ifdef __WIN32__
    static bool bInit = false;
    if(!bInit)
    {
        // Initialization of WINSOCK (on Windows machines only)
        WSADATA wsa;
        if(WSAStartup(MAKEWORD(1, 1), &wsa) != 0)
            throw SocketFatalError();
        bInit = true;
    }
#endif
}

bool Socket::Wait(int timeout) const
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET((SOCKET)m_iSocket, &fds);

    if(timeout < 0)
        select(m_iSocket + 1, &fds, NULL, NULL, NULL);
    else
    {
        timeval tv;

        tv.tv_sec = timeout/1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        select(m_iSocket + 1, &fds, NULL, NULL, &tv);
    }

    return FD_ISSET(m_iSocket, &fds);
}

int Socket::Unlock(Socket *s)
{
    int sock = s->GetSocket();
    s->m_iSocket = -1;
    delete s;
    return sock;
}

// FIXME : to be updated (IPv6, ...)
const SockAddress *Socket::Resolve(const char *name, unsigned int types)
{
    if(!(types & SockAddress::V4))
        return NULL;
    struct hostent *h = gethostbyname(name);
    if(h == NULL)
        return NULL;
    else
    {
        unsigned int buf;
        memcpy(&buf, h->h_addr, sizeof(struct in_addr));
        return new SockAddress4(ntohl(buf));
    }
}


/*============================================================================*/

bool SocketSet::IsSet(Socket *sock)
{
    std::set<Socket*>::const_iterator it;
    it = m_Sockets.find(sock);
    return it != m_Sockets.end();
}

void SocketSet::AddSocket(Socket *sock)
{
    if(!IsSet(sock))
        m_Sockets.insert(sock);
}

bool SocketSet::RemoveSocket(Socket *sock)
{
    std::set<Socket*>::iterator it;
    it = m_Sockets.find(sock);
    if(it != m_Sockets.end())
    {
        m_Sockets.erase(it);
        return true;
    }
    else
        return false;
}

void SocketSet::Clear()
{
    m_Sockets.clear();
}

Socket *SocketSet::Wait(int timeout)
{
    fd_set fds;
    FD_ZERO(&fds);

    int greatest = -1;

    std::set<Socket*>::iterator it = m_Sockets.begin();
    while(it != m_Sockets.end())
    {
        if((*it)->GetSocket() > greatest)
            greatest = (*it)->GetSocket();
        FD_SET((SOCKET)(*it)->GetSocket(), &fds);
        ++it;
    }

    if(greatest == -1)
        return NULL; // No valid socket?

    if(timeout == -1)
        select(greatest + 1, &fds, NULL, NULL, NULL);
    else
    {
        timeval tv;

        tv.tv_sec = timeout/1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        select(greatest + 1, &fds, NULL, NULL, &tv);
    }

    it = m_Sockets.begin();
    while(it != m_Sockets.end())
    {
        if(FD_ISSET((*it)->GetSocket(), &fds))
            return *it;
        else
            ++it;
    }

    return NULL;
}
