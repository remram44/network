#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include "Proxy.h"

/**
 * HTTP client proxy: sends "CONNECT host:port HTTP/1.0\n\n", receives the
 * response header, then the connection is established in both directions.
 */
class HTTPClient : public Proxy {

private:
    Proxy *m_pProxy;
    std::string m_sHost;
    int m_iPort;

public:
    HTTPClient(const char *host, int port, Proxy *proxy = NULL);
    virtual ~HTTPClient() {}
    virtual NetStream *Connect(const char *host, int port);
    
};

/**
 * Stream through a HTTP proxy.
 *
 * Dump copy of the bytes in both direction, with the exception of the initial
 * HTTP header from the proxy which is removed.
 */
class HTTPStream : public NetStream {

private:
    NetStream *m_pStream;
    bool m_bReqRecv;
    std::string m_sReq;

public:
    HTTPStream(NetStream *s);
    virtual ~HTTPStream() {}
    virtual void Send(const char *data, size_t size)
        throw(SocketConnectionClosed);
    virtual int Recv(char *data, size_t max, bool bWait = true)
        throw(SocketConnectionClosed);
    Socket *UnderlyingSocket();

};

#endif
