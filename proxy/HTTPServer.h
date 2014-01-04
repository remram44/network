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
        NetStream *stream;
    };

private:
    Proxy *m_pProxy;
    TCPServer *m_pSock;
    // Socket connected to the client => {HTTP request, NetStream}
    std::map<TCPSocket*, Client*> m_aConnections;
    // SocketSet containing the server socket m_pSock, the incoming connections
    // to the clients and the sockets in the NetStream objects
    SocketSet m_Set;
    // Sockets in the NetStreams => socket connected to the client using it
    // Allows to find the correct Client when m_Set indicates that data was
    // received on one of the NetStreams
    std::map<Socket*, TCPSocket*> m_aNetStream2Client;

public:
    HTTPServer(int port, Proxy *proxy = NULL);
    virtual void update(bool bWait = false);

};

#endif
