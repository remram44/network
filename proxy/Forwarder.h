#ifndef FORWARDER_H
#define FORWARDER_H

#include "proxy/Proxy.h"
#include "proxy/ProxyServer.h"

#include <map>

#ifndef ENABLE_FORWARDER
#error "The forwarder has not been enabled in the configuration"
#endif

#include "proxy/basetcpproxyserver.hpp"

typedef BaseProxyConnection ForwarderConnection;

class Forwarder : public BaseTCPProxyServer<ForwarderConnection> {

private:
    std::string m_sTargetHost;
    int m_iTargetPort;

public:
    Forwarder(int port, const std::string &target_host, int target_port,
        Proxy *proxy = NULL);

    void setupConnection(ForwarderConnection *conn);

};

#endif
