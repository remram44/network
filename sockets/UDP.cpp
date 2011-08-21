#include "UDP.h"

UDPPacket::UDPPacket()
  : m_pAddress(NULL)
{
}

UDPPacket::UDPPacket(const unsigned char *data, size_t len)
  : basic_string(data, len), m_pAddress(NULL)
{
}

void UDPPacket::setAddress(const SockAddress *address)
{
    if(m_pAddress != NULL)
        delete m_pAddress;
    m_pAddress = address->clone();
}

void UDPPacket::setAddress(const SockAddress *address, int port)
{
    if(m_pAddress != NULL)
        delete m_pAddress;
    m_pAddress = address->clone();
    m_iPort = port;
}

void UDPPacket::setPort(int port)
{
    m_iPort = port;
}

const SockAddress *UDPPacket::getAddress() const
{
    return m_pAddress;
}

int UDPPacket::getPort() const
{
    return m_iPort;
}

UDPSocket::UDPSocket()
  : Socket(socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP))
{
    sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(0); // Any port number

    memset(&(sin.sin_zero), 0, 8);

    if(bind(GetSocket(), (struct sockaddr*)&sin, sizeof(sin)) == -1)
        throw SocketCantUsePort(); // Can't happen?
}

UDPSocket::UDPSocket(int port) throw(SocketCantUsePort)
  : Socket(socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP))
{
    sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    memset(&(sin.sin_zero), 0, 8);

    if(bind(GetSocket(), (struct sockaddr*)&sin, sizeof(sin)) == -1)
        throw SocketCantUsePort();
}

bool UDPSocket::sendPacket(const UDPPacket &packet)
{
    const SockAddress *address = packet.getAddress();

    if(address->type() != SockAddress::V4)
        return false;

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(((SockAddress4*)address)->toUint());
    sin.sin_port = htons(packet.getPort());

    memset(&(sin.sin_zero), 0, 8);

    return sendto(GetSocket(), (char*)packet.c_str(), packet.size(), 0,
            (struct sockaddr*)&sin, sizeof(sin)) == (int)packet.size();
}

bool UDPSocket::recvPacket(UDPPacket &packet, bool bWait)
{
    if(bWait || Wait(0))
    {
        const unsigned int max_size = 1024;
        unsigned char data[max_size];

        sockaddr_in sin;
        socklen_t length = sizeof(sin);

        int ret = recvfrom(GetSocket(), (char*)data, max_size, 0,
                (struct sockaddr*)&sin, &length);

        if(ret <= 0)
            return false;

        SockAddress4 *address = new SockAddress4(ntohl(sin.sin_addr.s_addr));

        packet.assign(data, ret);
        packet.setAddress(address);
        packet.setPort(ntohs(sin.sin_port));

        delete address;

        return true;
    }
    else
        return false;
}
