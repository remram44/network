#ifndef FORWARDER_H
#define FORWARDER_H

#include "Proxy.h"
#include "ProxyServer.h"

#include <map>

class Forwarder : public ProxyServer {

public:
    struct Client {
        NetStream *stream;
    };

private:
    Proxy *m_pProxy;
    TCPServer *m_pSock;
    // Socket connected to the client => NetStream
    std::map<TCPSocket*, Client*> m_aConnections;
    // SocketSet containing the server socket m_pSock, the incoming connections
    // to the clients and the sockets in the NetStream objects
    SocketSet m_Set;
    // Sockets in the NetStreams => socket connected to the client using it
    // Allows to find the correct Client when m_Set indicates that data was
    // received on one of the NetStreams
    std::map<Socket*, TCPSocket*> m_aNetStream2Client;

    std::string m_sTargetHost;
    int m_iTargetPort;

public:
    Forwarder(int port, const std::string &target_host, int target_port,
        Proxy *proxy = NULL);
    virtual void update(bool bWait = false);

};

#endif
