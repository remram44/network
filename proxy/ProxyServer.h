#ifndef PROXYSERVER_H
#define PROXYSERVER_H

/**
 * A proxy server, listening on the local host.
 *
 * This object listens on a port and accepts relay requests from clients. It can
 * establish the requested connections through an optionnal proxy.
 */
class ProxyServer {

public:
    virtual ~ProxyServer() {}
    virtual void update(bool bWait = false) = 0;

};

#endif
