#ifndef FORWARDER_H
#define FORWARDER_H

#include "proxy/Proxy.h"
#include "proxy/ProxyServer.h"

#include <map>

#ifndef ENABLE_FORWARDER
#error "The forwarder has not been enabled in the configuration"
#endif

struct ForwarderConnection {
    TCPSocket *incoming;
    NetStream *outgoing;

    ForwarderConnection(TCPSocket *i, NetStream *o)
      : incoming(i), outgoing(o)
    {
    }
};

class Forwarder : public ProxyServer {

private:
    Proxy *m_pProxy;
    TCPServer *m_pSock;
    // Maps incoming connections
    std::map<Waitable*, ForwarderConnection*> m_aIncoming;
    // Maps outgoing connections
    std::map<Waitable*, ForwarderConnection*> m_aOutgoing;
    // SocketSet containing the server socket m_pSock, the incoming connections
    // to the clients and the outgoing NetStream objects
    SocketSet m_Set;

    std::string m_sTargetHost;
    int m_iTargetPort;

public:
    Forwarder(int port, const std::string &target_host, int target_port,
        Proxy *proxy = NULL);
    virtual void update(bool bWait = false);

};

#endif
