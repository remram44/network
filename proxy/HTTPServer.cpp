#include "proxy/HTTPServer.h"

#include <iostream>

HTTPServer::HTTPServer(int port, Proxy *proxy)
  : BaseTCPProxyServer(port, proxy)
{
}

void HTTPServer::handleInitialData(HTTPServerConnection *conn,
        const char *buf, size_t r, TCPSocket *cl)
{
    // Client for which we haven't already established a connection
    size_t s = conn->request.size();
    s = (s <= 3)?0:s - 3;
    conn->request.append(buf, r);
    if(conn->request.size() > 4096)
        throw SocketConnectionClosed();
    size_t i, j;
    if( ((j = 2, i = conn->request.find("\n\n", s)) !=
        std::string::npos)
     || ((j = 4, i = conn->request.find("\r\n\r\n", s)) !=
        std::string::npos) )
    {
        // We have received a line, the request is complete
#ifdef _DEBUG
        std::cerr << " request is complete\n";
#endif
        std::string req = conn->request.substr(0, i+j);
        std::string missing = conn->request.substr(i+j);
        // req = "CONNECT <host>:<port> HTTP/......\n\n" or "\n\r\n"
        // missing: beginning of the data, which we'll have to send
        if(req.substr(0, 8) != "CONNECT ")
            throw SocketConnectionClosed();
        size_t colon = req.find(':', 8);
        if(colon == std::string::npos)
            throw SocketConnectionClosed();
        std::string host = req.substr(8, colon-8);
        size_t space = req.find(' ', colon+1);
        if(space == std::string::npos)
            throw SocketConnectionClosed();
        std::istringstream iss(req.substr(colon+1, space-colon-1));
        int port;
        if(iss.eof() || ((iss>>port), false) || iss.fail()
         || !iss.eof())
            throw SocketConnectionClosed();
        // Establishes the connection
        conn->request.reserve(0);
#ifdef _DEBUG
        std::cerr << "HTTPServer: proxy.Connect(" << host << ", "
            << port << ")...";
#endif
        try {
            conn->outgoing = m_pProxy->connect(host.c_str(), port);
        }
        // FIXME : other exceptions?
        catch(SocketError &e)
        {
#ifdef _DEBUG
            std::cerr << " error\n";
#endif
            cl->send("HTTP/1.0 503 Service Unavailable\r\n\r\n", 36);
            throw SocketConnectionClosed();
        }
#ifdef _DEBUG
        std::cerr << " Ok\n";
#endif
        cl->send("HTTP/1.0 200 Connection established\r\n\r\n", 39);
        // We send the initial data, which was received with the
        // request
        conn->outgoing->send(missing.c_str(), missing.size());
    }
    else
    {
#ifdef _DEBUG
        std::cerr << " request in progress\n";
#endif
    }
}
