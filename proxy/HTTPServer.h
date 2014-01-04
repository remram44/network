#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "proxy/Proxy.h"
#include "proxy/ProxyServer.h"

#include <map>

#ifndef ENABLE_HTTP_SERVER
#error "The HTTP server has not been enabled in the configuration"
#endif

#include "proxy/basetcpproxyserver.hpp"

struct HTTPServerConnection : public BaseProxyConnection {
    std::string request;

    HTTPServerConnection(TCPSocket *i)
      : BaseProxyConnection(i)
    {
    }
};

class HTTPServer : public BaseTCPProxyServer<HTTPServerConnection> {

public:
    HTTPServer(int port, Proxy *proxy = NULL);
    void handleInitialData(HTTPServerConnection *conn,
            const char *buf, size_t r, TCPSocket *cl);

};

#endif
