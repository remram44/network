#include "proxy/HTTPServer.h"

#include <iostream>

HTTPServer::HTTPServer(int port, Proxy *proxy)
  : m_pProxy(proxy)
{
#ifdef _DEBUG
    std::cerr << "HTTPServer:";
#endif
    if(m_pProxy == NULL)
    {
        m_pProxy = TCPClient::getInstance();
#ifdef _DEBUG
        std::cerr << " using direct TCP connection mode...";
#endif
    }
    m_pSock = TCPServer::listen(port);
    m_Set.add(m_pSock);
#ifdef _DEBUG
    std::cerr << " listening\n";
#endif
}

void HTTPServer::update(bool bWait)
{
#ifdef _DEBUG
    std::cerr << "HTTPServer: update(" << (bWait?"true":"false") << ")\n";
#endif
    Waitable *signaled = m_Set.wait(bWait?-1:0);
    if(signaled == m_pSock)
    {
        // New connection
#ifdef _DEBUG
        std::cerr << "HTTPServer: connection from a new client...";
#endif
        TCPSocket *cl = m_pSock->accept(0);
        if(cl)
        {
            Client *c = new Client(cl);
            m_aIncoming[cl] = c;
            m_Set.add(cl);
#ifdef _DEBUG
            std::cerr << " ok\n";
#endif
        }
        else
        {
#ifdef _DEBUG
            std::cerr << " fail\n";
#endif
        }
    }
    else if(m_aIncoming.find(signaled) != m_aIncoming.end())
    {
        // Receives data from a client
#ifdef _DEBUG
        std::cerr << "HTTPServer: receiving data from a client...";
#endif
        Client *c = m_aIncoming[signaled];
        TCPSocket *cl = c->incoming;
        try {
            static char buf[1024];
            int r = cl->recv(buf, 1024, false);
            if(r <= 0)
                throw SocketConnectionClosed();
            if(c->outgoing != NULL)
            {
                // Client already connected: dumb copy
#ifdef _DEBUG
                std::cerr << " forward (" << r << ")\n";
#endif
                c->outgoing->send(buf, r);
            }
            else
            {
                // Client for which we haven't already established a connection
                size_t s = c->request.size();
                s = (s <= 3)?0:s - 3;
                c->request.append(buf, r);
                if(c->request.size() > 4096)
                    throw SocketConnectionClosed();
                size_t i, j;
                if( ((j = 2, i = c->request.find("\n\n", s)) !=
                    std::string::npos)
                 || ((j = 4, i = c->request.find("\r\n\r\n", s)) !=
                    std::string::npos) )
                {
                    // We have received a line, the request is complete
#ifdef _DEBUG
                    std::cerr << " request is complete\n";
#endif
                    std::string req = c->request.substr(0, i+j);
                    std::string missing = c->request.substr(i+j);
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
                    c->request.reserve(0);
#ifdef _DEBUG
                    std::cerr << "HTTPServer: proxy.Connect(" << host << ", "
                        << port << ")...";
#endif
                    try {
                        c->outgoing = m_pProxy->connect(host.c_str(), port);
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
                    // We add it to the SocketSet, and we store the pair
                    // NetStream => TCPSocket
                    m_Set.add(c->outgoing);
                    m_aOutgoing[c->outgoing] = c;
                    // We send the initial data, which was received with the
                    // request
                    c->outgoing->send(missing.c_str(), missing.size());
                }
                else
                {
#ifdef _DEBUG
                    std::cerr << " request in progress\n";
#endif
                }
            }
        }
        catch(SocketConnectionClosed &e)
        {
            if(c->outgoing)
            {
                m_Set.remove(c->outgoing);
                m_aOutgoing.erase(c->outgoing);
                delete c->outgoing;
            }
            m_Set.remove(cl);
            m_aIncoming.erase(cl);
            delete c;
            delete cl;
#ifdef _DEBUG
            std::cerr << " connection closed by the client\n";
#endif
        }
    }
    else
    {
        // Receives data on a NetStream, for the client
#ifdef _DEBUG
        std::cerr << "HTTPServer: receiving data on a NetStream...";
#endif
        Client *c = m_aOutgoing[signaled];
        TCPSocket *cl = c->incoming;
        try {
            static char buf[1024];
            int r = c->outgoing->recv(buf, 1024, false);
            // Dumb copy
            if(r > 0)
            {
#ifdef _DEBUG
                std::cerr << " forward (" << r << ")\n";
#endif
                cl->send(buf, r);
            }
        }
        catch(SocketConnectionClosed &e)
        {
            m_Set.remove(c->outgoing);
            m_aOutgoing.erase(c->outgoing);
            delete c->outgoing;
            m_Set.remove(cl);
            m_aIncoming.erase(cl);
            delete c;
            delete cl;
#ifdef _DEBUG
            std::cerr << " connection closed by the remote host\n";
#endif
        }
    }
}
