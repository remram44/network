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
const unsigned char *Socket::Resolve(const char *name)
{
    struct hostent *h = gethostbyname(name);
    if(h == NULL)
        return NULL;
    else
    {
        static unsigned char buf[4];
        memcpy(buf, h->h_addr, sizeof(struct in_addr));
        return buf;
    }
}

/*============================================================================*/

TCPSocket::TCPSocket(int sock)
  : Socket::Socket(sock)
{
}

TCPSocket *TCPSocket::Connect(const char *host, int port)
    throw(SocketUnknownHost, SocketConnectionRefused)
{
    TCPSocket *sock = new TCPSocket(socket(AF_INET, SOCK_STREAM, 0));

    // Hostname resolution
    struct sockaddr_in adresse;
    struct hostent* h = gethostbyname(host);
    if(h == NULL)
    {
        throw SocketUnknownHost();
    }

    adresse.sin_family = AF_INET;
    adresse.sin_addr = *((struct in_addr *)h->h_addr);
    adresse.sin_port = htons(port);

    memset(&(adresse.sin_zero), 0, 8);

    if(connect(sock->GetSocket(), (struct sockaddr*)&adresse,
        sizeof(adresse)) == -1)
    {
        throw SocketConnectionRefused();
    }

    return sock;
}

void TCPSocket::Send(const char *data, size_t size)
    throw(SocketConnectionClosed)
{
    int ret = send(GetSocket(), data, size, 0);
    if(ret != (int)size)
        throw SocketConnectionClosed();
}

int TCPSocket::Recv(char *data, size_t size_max, bool bWait)
    throw(SocketConnectionClosed)
{
    if(bWait || Wait(0))
    {
        int ln = recv(GetSocket(), data, size_max, 0);
        if(ln <= 0)
            throw SocketConnectionClosed();
        else
            return ln;
    }
    else
        return 0;
}

Socket *TCPSocket::UnderlyingSocket()
{
    return this;
}

/*============================================================================*/

TCPServer::TCPServer(int sock)
  : Socket::Socket(sock)
{
}

TCPServer *TCPServer::Listen(int port) throw(SocketCantUsePort)
{
    TCPServer *sock = new TCPServer(socket(AF_INET, SOCK_STREAM, 0));

    struct sockaddr_in sin;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);

    if( (bind(sock->GetSocket(), (struct sockaddr*)&sin, sizeof(sin) ) == -1)
     || (listen(sock->GetSocket(), 5) == -1) )
        throw SocketCantUsePort();

    return sock;
}

TCPSocket *TCPServer::Accept(int timeout)
{
    if(Wait(timeout))
    {
        struct sockaddr_in clientsin;
        socklen_t taille = sizeof(clientsin);
        int sock = accept(GetSocket(), (struct sockaddr*)&clientsin, &taille);

        if(sock != -1)
            return new TCPSocket(sock);
        else
        {
            // Shoudln't happen! Did someone close our socket?
            throw SocketFatalError();
            return NULL;
        }
    }

    return NULL;
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
        it++;
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
            it++;
    }

    return NULL;
}
