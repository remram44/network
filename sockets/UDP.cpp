#include "UDP.h"

UDPPacket::UDPPacket()
{
}

UDPPacket::UDPPacket(const unsigned char *data, size_t len)
  : basic_string(data, len)
{
}

UDPPacket::UDPPacket(const SockAddress &address, int port)
  : m_Address(address), m_Port(port)
{
}

void UDPPacket::setAddress(const SockAddress &address)
{
    m_Address = address;
}

void UDPPacket::setAddress(const SockAddress &address, int port)
{
    m_Address = address;
    m_Port = port;
}

void UDPPacket::setPort(int port)
{
    m_Port = port;
}

const SockAddress &UDPPacket::getAddress() const
{
    return m_Address;
}

int UDPPacket::getPort() const
{
    return m_Port;
}

UDPSocket::UDPSocket()
  : Socket(::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))
{
    sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(0); // Any port number

    memset(&(sin.sin_zero), 0, 8);

    if(::bind(getSocket(), (struct sockaddr*)&sin, sizeof(sin)) == -1)
        throw SocketCantUsePort(); // Can't happen?
}

UDPSocket::UDPSocket(int port) throw(SocketCantUsePort)
  : Socket(::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))
{
    sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    memset(&(sin.sin_zero), 0, 8);

    if(::bind(getSocket(), (struct sockaddr*)&sin, sizeof(sin)) == -1)
        throw SocketCantUsePort();
}

bool UDPSocket::sendPacket(const UDPPacket &packet)
{
    const SockAddress &address = packet.getAddress();
    if(!address.isValid())
        throw SocketBadAddress();

    if(address.type() == SockAddress::V4)
    {
        struct sockaddr_in sin;
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = htonl(address.v4toUint());
        sin.sin_port = htons(packet.getPort());

        memset(&(sin.sin_zero), 0, 8);

        return sendto(getSocket(), (char*)packet.c_str(), packet.size(), 0,
                (struct sockaddr*)&sin, sizeof(sin)) == (int)packet.size();
    }
    else
        return false;
}

bool UDPSocket::recvPacket(UDPPacket &packet, bool bWait)
{
    if(bWait || wait(0))
    {
        const unsigned int max_size = 1024;
        unsigned char data[max_size];

        sockaddr_in sin;
        socklen_t length = sizeof(sin);

        int ret = recvfrom(getSocket(), (char*)data, max_size, 0,
                (struct sockaddr*)&sin, &length);

        if(ret <= 0)
            return false;

        SockAddress address = SockAddress::v4(ntohl(sin.sin_addr.s_addr));

        packet.assign(data, ret);
        packet.setAddress(address);
        packet.setPort(ntohs(sin.sin_port));

        return true;
    }
    else
        return false;
}
