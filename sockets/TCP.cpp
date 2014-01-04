#include "sockets/TCP.h"

TCPSocket::TCPSocket(int sock)
  : Socket::Socket(sock)
{
}

TCPSocket *TCPSocket::connect(SockAddress dest, int port)
    throw(SocketConnectionRefused)
{
    TCPSocket *sock;

    if(dest.type() == SockAddress::V4)
    {
        sock = new TCPSocket(::socket(AF_INET, SOCK_STREAM, 0));

        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(dest.v4toUint());
        address.sin_port = htons(port);

        memset(&(address.sin_zero), 0, 8);

        if(::connect(sock->getSocket(), (struct sockaddr*)&address,
                sizeof(address)) == -1)
            throw SocketConnectionRefused();
    }
    else
        throw SocketConnectionRefused();

    return sock;
}

TCPSocket *TCPSocket::connect(const char *host, int port)
    throw(SocketUnknownHost, SocketConnectionRefused)
{
    TCPSocket *sock = new TCPSocket(::socket(AF_INET, SOCK_STREAM, 0));

    // Hostname resolution
    struct sockaddr_in address;
    struct hostent* h = ::gethostbyname(host);
    if(h == NULL)
        throw SocketUnknownHost();

    address.sin_family = AF_INET;
    address.sin_addr = *((struct in_addr *)h->h_addr);
    address.sin_port = htons(port);

    memset(&(address.sin_zero), 0, 8);

    if(::connect(sock->getSocket(), (struct sockaddr*)&address,
            sizeof(address)) == -1)
        throw SocketConnectionRefused();

    return sock;
}

void TCPSocket::send(const char *data, size_t size)
    throw(SocketConnectionClosed)
{
    int ret = ::send(getSocket(), data, size, 0);
    if(ret != (int)size)
        throw SocketConnectionClosed();
}

int TCPSocket::recv(char *data, size_t size_max, bool bWait)
    throw(SocketConnectionClosed)
{
    if(bWait || wait(0))
    {
        int ln = ::recv(getSocket(), data, size_max, 0);
        if(ln <= 0)
            throw SocketConnectionClosed();
        else
            return ln;
    }
    else
        return 0;
}

int TCPSocket::getLocalPort() const
{
    struct sockaddr_in address;
    socklen_t size = sizeof(address);
    if(::getsockname(getSocket(), (struct sockaddr*)&address, &size) < 0)
        return -1;
    return ntohs(address.sin_port);
}

/*============================================================================*/

TCPServer::TCPServer(int sock)
  : Socket::Socket(sock)
{
}

TCPServer *TCPServer::listen(int port) throw(SocketCantUsePort)
{
    TCPServer *sock = new TCPServer(::socket(AF_INET, SOCK_STREAM, 0));

    struct sockaddr_in sin;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);

    if( (::bind(sock->getSocket(), (struct sockaddr*)&sin, sizeof(sin) ) == -1)
     || (::listen(sock->getSocket(), 5) == -1) )
        throw SocketCantUsePort();

    return sock;
}

TCPSocket *TCPServer::accept(int timeout)
{
    if(wait(timeout))
    {
        struct sockaddr_in clientsin;
        socklen_t s = sizeof(clientsin);
        int sock = ::accept(getSocket(), (struct sockaddr*)&clientsin, &s);

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

int TCPServer::getLocalPort() const
{
    struct sockaddr_in address;
    socklen_t size = sizeof(address);
    if(::getsockname(getSocket(), (struct sockaddr*)&address, &size) < 0)
        return -1;
    return ntohs(address.sin_port);
}
