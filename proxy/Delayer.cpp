#include "Delayer.h"
#include "sockets/DelayedStream.h"

Delayer::Delayer(unsigned int delay, Proxy *proxy)
  : m_iDelay(delay), m_pProxy(proxy)
{
    if(m_pProxy == NULL)
        m_pProxy = new TCPClient;
}

NetStream *Delayer::Connect(const char *host, int port)
{
    return new DelayedStream(m_pProxy->Connect(host, port), m_iDelay);
}
