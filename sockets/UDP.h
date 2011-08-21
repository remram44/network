#ifndef UDP_H
#define UDP_H

#include "Socket.h"

#include <string>


/*============================================================================*/

class UDPPacket : public std::basic_string<unsigned char> {

private:
    const SockAddress *m_pAddress;
    unsigned short m_iPort;

public:
    UDPPacket();

    UDPPacket(const unsigned char *data, size_t len);

    /**
     * Changes the destination of this packet.
     */
    void setAddress(const SockAddress *address);

    /**
     * Changes the destination of this packet.
     */
    void setAddress(const SockAddress *address, int port);

    /**
     * Changes the destination port number of this packet.
     */
    void setPort(int port);

    /**
     * Returns the address associated with this packet.
     *
     * Please note that the SockAddress object will be destroyed with this
     * packet, so make sure to copy it with SockAddress::clone() if needed.
     */
    const SockAddress *getAddress() const;

    /**
     * Returns the port number associated with this packet.
     */
    int getPort() const;

};

class UDPSocket : public Socket {

public:
    /**
     * Default constructor, binds to a random port.
     */
    UDPSocket();

    /**
     * Constructor binding on the specified port number.
     */
    UDPSocket(int port) throw(SocketCantUsePort);

    /**
     * Sends a packet on this socket.
     *
     * The packet must have an attached address and port number.
     */
    bool sendPacket(const UDPPacket &packet);

    /**
     * Receives a packet on this socket.
     *
     * The packet will have its address and port number set to the sender's.
     * All previous content in packet will be discarded.
     *
     * @param bWait Indicates whether we should wait for data to arrive, if
     * nothing is initially available. If false, this method can return false.
     * @return True if a packet was received and put in packet.
     */
    bool recvPacket(UDPPacket &packet, bool bWait);

};

#endif
