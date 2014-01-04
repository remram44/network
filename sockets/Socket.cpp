#include "sockets/Socket.h"

#include <map>

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

const char *SocketBadAddress::what()
{
    return "Bad address";
}


/*============================================================================*/

SockAddress::SockAddress(EType type, unsigned char *data)
  : m_Type(type), m_Data(data)
{
}

SockAddress::SockAddress()
  : m_Type(INVALID), m_Data(NULL)
{
}

SockAddress::SockAddress(const SockAddress &other)
  : m_Type(INVALID), m_Data(NULL)
{
    *this = other;
}

void SockAddress::operator=(const SockAddress &other)
{
    m_Type = other.m_Type;
    if(m_Type == INVALID)
    {
        m_Data = NULL;
        return ;
    }
    size_t len;
    if(m_Type == V4)
        len = 4;
    else
        len = 16;
    m_Data = new unsigned char[len];
    ::memcpy(m_Data, other.m_Data, len);
}

SockAddress::~SockAddress()
{
    if(m_Data != NULL)
        delete[] m_Data;
}

bool SockAddress::isValid() const
{
    return m_Type != INVALID;
}

SockAddress SockAddress::v4(unsigned int address)
{
    unsigned char *data = new unsigned char[4];
    data[0] = address >> 24;
    data[1] = address >> 16;
    data[2] = address >> 8;
    data[3] = address;
    return SockAddress(V4, data);
}

SockAddress SockAddress::v4(unsigned char a, unsigned char b,
                            unsigned char c, unsigned char d)
{
    unsigned char *data = new unsigned char[4];
    data[0] = a;
    data[1] = b;
    data[2] = c;
    data[3] = d;
    return SockAddress(V4, data);
}

SockAddress SockAddress::v4(unsigned char *address)
{
    unsigned char *data = new unsigned char[4];
    ::memcpy(data, address, 4);
    return SockAddress(V4, data);
}

SockAddress SockAddress::v4(const char *address)
{
    return v4(ntohl(inet_addr(address)));
}

unsigned int SockAddress::v4toUint() const throw(SocketBadAddress)
{
    if(m_Type != V4)
        throw SocketBadAddress();
    return (m_Data[0] << 24) | (m_Data[1] << 16) |
            (m_Data[2] << 8) | m_Data[3];
}

void SockAddress::print(std::ostream &out) const
{
    if(m_Type == V4)
        out << (int)m_Data[0] << '.' << (int)m_Data[1] << '.'
            << (int)m_Data[2] << '.' << (int)m_Data[3];
    else /* m_Type == V6 */
    {
        std::ios state(NULL);
        state.copyfmt(out);
        out << std::hex << std::uppercase;
        out << '[' << (int)m_Data[0];
        for(int i = 1; i < 16; ++i)
            out << ':' << (int)m_Data[i];
        out << ']';
        out.copyfmt(state);
    }
}

std::ostream &operator<<(std::ostream &out, const SockAddress &address)
{
    address.print(out);
    return out;
}


/*============================================================================*/

void Waitable::wait(int timeout)
{
    SocketSet set;
    set.add(this);
    set.wait(timeout);
}


/*============================================================================*/

Socket::Socket(int sock)
  : m_Socket(sock)
{
    if(m_Socket == -1)
        throw SocketFatalError();
}

Socket::~Socket()
{
    if(m_Socket != -1)
    {
#ifndef __WIN32__
        ::shutdown(m_Socket, SHUT_RDWR);
        ::close(m_Socket);
#else
        ::closesocket(m_Socket);
#endif
        m_Socket = -1;
    }
}

bool Socket::operator==(const Socket& s)
{
    return m_Socket == s.getSocket();
}

bool Socket::operator<(const Socket& s)
{
    return m_Socket < s.getSocket();
}

void Socket::init() throw(SocketFatalError)
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

bool Socket::wait(int timeout) const
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET((SOCKET)m_Socket, &fds);

    if(timeout < 0)
        ::select(m_Socket + 1, &fds, NULL, NULL, NULL);
    else
    {
        timeval tv;

        tv.tv_sec = timeout/1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        ::select(m_Socket + 1, &fds, NULL, NULL, &tv);
    }

    return FD_ISSET(m_Socket, &fds);
}

void Socket::registerSockets(SocketSetRegistrar *registrar)
{
    registrar->addSocket(this);
}

int Socket::unlock(Socket *s)
{
    int sock = s->getSocket();
    s->m_Socket = -1;
    delete s;
    return sock;
}

// FIXME : to be updated (IPv6, ...)
SockAddress Socket::resolve(const char *name, unsigned int types)
{
    if(!(types & SockAddress::V4))
        throw SocketUnknownHost();
    struct hostent *h = gethostbyname(name);
    if(h == NULL)
        throw SocketUnknownHost();
    else
    {
        unsigned int buf;
        memcpy(&buf, h->h_addr, sizeof(struct in_addr));
        return SockAddress::v4(ntohl(buf));
    }
}


/*============================================================================*/

bool SocketSet::isSet(Waitable *obj)
{
    std::set<Waitable*>::const_iterator it;
    it = m_Set.find(obj);
    return it != m_Set.end();
}

void SocketSet::add(Waitable *obj)
{
    if(!isSet(obj))
        m_Set.insert(obj);
}

bool SocketSet::remove(Waitable *obj)
{
    std::set<Waitable*>::iterator it;
    it = m_Set.find(obj);
    if(it != m_Set.end())
    {
        m_Set.erase(it);
        return true;
    }
    else
        return false;
}

void SocketSet::clear()
{
    m_Set.clear();
}


/*============================================================================*/

class FDSetWrapper {

private:
    fd_set fds;
    int greatest;
    std::map<Socket*, Waitable*> registered_sockets;

public:
    FDSetWrapper();
    void addSocket(Waitable *obj, Socket *sock);
    Waitable *wait(int timeout);

};

FDSetWrapper::FDSetWrapper()
{
    FD_ZERO(&fds);
    greatest = -1;
}

void FDSetWrapper::addSocket(Waitable *obj, Socket *sock)
{
    int s = sock->getSocket();
    if(s > greatest)
        greatest = s;
    registered_sockets[sock] = obj;
    FD_SET((SOCKET)s, &fds);
}

Waitable *FDSetWrapper::wait(int timeout)
{
    if(greatest == -1)
        return NULL; // No valid socket?

    if(timeout == -1)
        ::select(greatest + 1, &fds, NULL, NULL, NULL);
    else
    {
        timeval tv;

        tv.tv_sec = timeout/1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        ::select(greatest + 1, &fds, NULL, NULL, &tv);
    }

    std::map<Socket*, Waitable*>::const_iterator it;
    it = registered_sockets.begin();
    while(it != registered_sockets.end())
    {
        if(FD_ISSET(it->first->getSocket(), &fds))
            return it->second;
        else
            ++it;
    }

    return NULL;
}

SocketSetRegistrar::SocketSetRegistrar(FDSetWrapper *wrapper, Waitable *obj)
  : m_Wrapper(wrapper), m_Waitable(obj)
{
}

void SocketSetRegistrar::addSocket(Socket *sock)
{
    m_Wrapper->addSocket(m_Waitable, sock);
}

Waitable *SocketSet::wait(int timeout)
{
    FDSetWrapper fds;

    std::set<Waitable*>::iterator it = m_Set.begin();
    while(it != m_Set.end())
    {
        SocketSetRegistrar reg(&fds, *it);
        (*it)->registerSockets(&reg);
        ++it;
    }

    return fds.wait(timeout);
}
