#include "proxy/HTTPClient.h"

#include <sstream>
#include <iostream>

HTTPClient::HTTPClient(const char *host, int port, Proxy *proxy)
  : m_pProxy(proxy), m_sHost(host), m_iPort(port)
{
    if(m_pProxy == NULL)
        m_pProxy = TCPClient::getInstance();
}

NetStream *HTTPClient::connect(const char *host, int port)
{
#ifdef _DEBUG
    std::cerr << "HTTPClient: connection...";
#endif
    NetStream *s = m_pProxy->connect(m_sHost.c_str(), m_iPort);
#ifdef _DEBUG
    std::cerr << " ok\n";
#endif
    std::ostringstream oss;
#ifdef _DEBUG
    std::cerr << "HTTPClient: forwarding request...";
#endif
    oss << "CONNECT " << host << ":" << port << " HTTP/1.0\n\n";
    s->send(oss.str().c_str(), oss.str().size());
#ifdef _DEBUG
    std::cerr << " ok\n";
#endif
    
    return new HTTPStream(s);
}

HTTPStream::HTTPStream(NetStream *s)
  : m_pStream(s), m_bReqRecv(false)
{
}

HTTPStream::~HTTPStream()
{
    delete m_pStream;
}

void HTTPStream::send(const char *data, size_t size)
    throw(SocketConnectionClosed)
{
    m_pStream->send(data, size);
}

int HTTPStream::recv(char *data, size_t max, bool bWait)
    throw(SocketConnectionClosed)
{
    if(m_bReqRecv)
        return m_pStream->recv(data, max, bWait);
    else
    {
        static char buf[1024];
        int r = m_pStream->recv(buf, (max>1024)?1024:max, bWait);
        if(r >= 1)
        {
            m_sReq.append(buf, r);
            size_t i, j;
            if( ((j = 1, i = m_sReq.find("\n\n")) != std::string::npos)
             || ((j = 2, i = m_sReq.find("\n\r\n")) != std::string::npos) )
            {
                if(i + j + 1 < m_sReq.size())
                    memcpy(data, m_sReq.c_str()+i+j+1,
                        m_sReq.size() - (i+j+1));
                m_bReqRecv = true;
                return m_sReq.size() - (i+j+1);
            }
        }
        return 0;
    }
}

void HTTPStream::registerSockets(SocketSetRegistrar *registrar)
{
    m_pStream->registerSockets(registrar);
}

void HTTPStream::wait(int timeout)
{
    m_pStream->wait(timeout);
}
