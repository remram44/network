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
    Waitable *sock = m_Set.wait(bWait?-1:0);
    if(sock == m_pSock)
    {
        // New connection
#ifdef _DEBUG
        std::cerr << "HTTPServer: connection from a new client...";
#endif
        TCPSocket *cl = m_pSock->accept(0);
        if(cl)
        {
            Client *c = new Client;
            c->stream = NULL;
            m_aConnections[cl] = c;
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
    else if(m_aConnections.find((TCPSocket*)sock) != m_aConnections.end())
    {
        // Receives data from a client
#ifdef _DEBUG
        std::cerr << "HTTPServer: receiving data from a client...";
#endif
        TCPSocket *cl = (TCPSocket*)sock;
        try {
            Client *c = m_aConnections[cl];
            static char buf[1024];
            int r = cl->recv(buf, 1024, false);
            if(r <= 0)
                throw SocketConnectionClosed();
            if(c->stream != NULL)
            {
                // Client already connected: dumb copy
#ifdef _DEBUG
                std::cerr << " forward (" << r << ")\n";
#endif
                c->stream->send(buf, r);
            }
            else
            {
                // Client for which we haven't already established a connection
                c->request.append(buf, r);
                if(c->request.size() > 4096)
                    throw SocketConnectionClosed();
                size_t i, j;
                if( ((j = 1, i = c->request.find("\n\n")) !=
                    std::string::npos)
                 || ((j = 2, i = c->request.find("\n\r\n")) !=
                    std::string::npos) )
                {
                    // We have received a line, the request is complete
#ifdef _DEBUG
                    std::cerr << " request is complete\n";
#endif
                    std::string req = c->request.substr(0, i+j);
                    std::string missing = c->request.substr(i+j+1);
                    // req = "CONNECT <host>:<port> HTTP/......\n\n" ou "\n\r\n"
                    // missing : beginning of the data, which we'll have to send
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
                    c->request = "";
#ifdef _DEBUG
                    std::cerr << "HTTPServer: proxy.Connect(" << host << ", "
                        << port << ")...";
#endif
                    try {
                        c->stream = m_pProxy->connect(host.c_str(), port);
                    }
                    // FIXME : other exceptions?
                    catch(SocketError &e)
                    {
#ifdef _DEBUG
                        std::cerr << " error\n";
#endif
                        cl->send("HTTP/1.0 503 Service Unavailable\n\n", 34);
                        throw SocketConnectionClosed();
                    }
#ifdef _DEBUG
                    std::cerr << " Ok\n";
#endif
                    cl->send("HTTP/1.0 200 Connection established\n\n", 37);
                    // We add it to the SocketSet, and we store the pair
                    // NetStream => TCPSocket
                    m_Set.add(c->stream);
                    m_aNetStream2Client[c->stream] = cl;
                    // We send the initial data, which was received with the
                    // request
                    c->stream->send(missing.c_str(), missing.size());
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
            NetStream *stream = m_aConnections[cl]->stream;
            if(stream)
            {
                m_Set.remove(stream);
                m_aNetStream2Client.erase(stream);
                delete stream;
            }
            m_aConnections.erase(cl);
            m_Set.remove(cl);
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
        std::map<Socket*, TCPSocket*>::iterator it =
            m_aNetStream2Client.find(sock);
        if(it != m_aNetStream2Client.end())
        {
            TCPSocket *cl = it->second;
            try {
                static char buf[1024];
                int r = m_aConnections[cl]->stream->recv(buf, 1024, false);
                // Dump copy
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
                m_aNetStream2Client.erase(sock);
                if(m_aConnections[cl]->stream)
                    delete m_aConnections[cl]->stream;
                m_aConnections.erase(cl);
                m_Set.remove(cl);
                m_Set.remove(sock);
                delete cl;
#ifdef _DEBUG
                std::cerr << " connection closed by the remote host\n";
#endif
            }
        }
    }
}
