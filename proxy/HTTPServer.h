#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "proxy/Proxy.h"
#include "proxy/ProxyServer.h"

#include <map>

#ifndef ENABLE_HTTP_SERVER
#error "The HTTP server has not been enabled in the configuration"
#endif

class HTTPServer : public ProxyServer {

public:
    struct Client {
        std::string request;
        TCPSocket *incoming;
        NetStream *outgoing;

        Client(TCPSocket *i)
          : incoming(i), outgoing(NULL)
        {
        }
    };

private:
    Proxy *m_pProxy;
    TCPServer *m_pSock;
    // Maps incoming connections
    std::map<Waitable*, Client*> m_aIncoming;
    // Maps outgoing connections
    std::map<Waitable*, Client*> m_aOutgoing;
    // SocketSet containing the server socket m_pSock, the incoming connections
    // to the clients and the sockets in the NetStream objects
    SocketSet m_Set;

public:
    HTTPServer(int port, Proxy *proxy = NULL);
    virtual void update(bool bWait = false);

};

#endif
