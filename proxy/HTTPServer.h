#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "proxy/Proxy.h"
#include "proxy/ProxyServer.h"

#include <map>

#ifndef ENABLE_HTTP_SERVER
#error "The HTTP server has not been enabled in the configuration"
#endif

#include "proxy/basetcpproxyserver.hpp"

class HTTPServer : public BaseTCPProxyServer {

private:
    struct Connection : public BaseTCPProxyServer::Connection {
        std::string request;

        Connection(TCPSocket *i)
          : BaseTCPProxyServer::Connection(i)
        {
        }
    };

public:
    HTTPServer(int port, Proxy *proxy = NULL);

    Connection *newConnection(TCPSocket *cl);
    void handleInitialData(BaseTCPProxyServer::Connection *conn,
            const char *buf, size_t r, TCPSocket *cl);

};

#endif
