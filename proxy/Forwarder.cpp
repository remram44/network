#include "proxy/Forwarder.h"

#include <iostream>
#include <sstream>

Forwarder::Forwarder(int port, const std::string &target_host, int target_port,
        Proxy *proxy)
  : BaseTCPProxyServer(port, proxy),
    m_sTargetHost(target_host), m_iTargetPort(target_port)
{
}

void Forwarder::setupConnection(ForwarderConnection *conn)
{
    conn->outgoing = m_pProxy->connect(m_sTargetHost.c_str(), m_iTargetPort);
}
