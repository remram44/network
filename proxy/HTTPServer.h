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
    struct HTTPServerConnection : public BaseProxyConnection {
        std::string request;

        HTTPServerConnection(TCPSocket *i)
          : BaseProxyConnection(i)
        {
        }
    };

public:
    HTTPServer(int port, Proxy *proxy = NULL);

    HTTPServerConnection *newConnection(TCPSocket *cl);
    void handleInitialData(BaseProxyConnection *conn,
            const char *buf, size_t r, TCPSocket *cl);

};

#endif
