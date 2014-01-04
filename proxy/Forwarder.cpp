#include "proxy/Forwarder.h"

#include <iostream>
#include <sstream>

Forwarder::Forwarder(int port, const std::string &target_host, int target_port,
        Proxy *proxy)
  : BaseTCPProxyServer(port, proxy),
    m_sTargetHost(target_host), m_iTargetPort(target_port)
{
}

Forwarder::Connection *Forwarder::newConnection(TCPSocket *cl)
{
    return new Connection(
            cl,
            m_pProxy->connect(m_sTargetHost.c_str(), m_iTargetPort));
}
