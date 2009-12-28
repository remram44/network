#include "Socks4Server.h"

#include <iostream>
#include <sstream>

Socks4Server::Socks4Server(int port, Proxy *proxy)
  : m_pProxy(proxy)
{
#ifdef _DEBUG
    std::cerr << "Socks4Server:";
#endif
    if(m_pProxy == NULL)
    {
        m_pProxy = new TCPClient;
#ifdef _DEBUG
        std::cerr << " using direct TCP connection mode...";
#endif
    }
    m_pSock = TCPServer::Listen(port);
    m_Set.AddSocket(m_pSock);
#ifdef _DEBUG
    std::cerr << " listening\n\n";
#endif
}

void Socks4Server::update(bool bWait)
{
#ifdef _DEBUG
    std::cerr << "Socks4Server: update(" << (bWait?"true":"false") << ")\n";
#endif
    Socket *sock = m_Set.Wait(bWait?-1:0);
    if(sock == m_pSock)
    {
        // New connection
#ifdef _DEBUG
        std::cerr << "Socks4Server: connection from a new client...";
#endif
        TCPSocket *cl = m_pSock->Accept(0);
        if(cl)
        {
            Client *c = new Client;
            c->stream = NULL;
            m_aConnections[cl] = c;
            m_Set.AddSocket(cl);
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
        std::cerr << "Socks4Server: receiving data from a client...";
#endif
        TCPSocket *cl = (TCPSocket*)sock;
        try {
            Client *c = m_aConnections[cl];
            static char buf[1024];
            int r = cl->Recv(buf, 1024, false);
            if(r <= 0)
                throw SocketConnectionClosed();
            if(c->stream != NULL)
            {
                // Client already connected: dumb copy
#ifdef _DEBUG
                std::cerr << " forward (" << r << ")\n";
#endif
                c->stream->Send(buf, r);
            }
            else
            {
                // Client for which we haven't already established a connection
                c->request.append(buf, r);
                if(c->request.size() > 4096)
                    throw SocketConnectionClosed();
                if( (c->request.size() >= 8)
                 && ( (c->request[0] != 0x04)
                   || (c->request[1] != 0x01) ) )
                {
                    cl->Send("\x00\x5b" "\0\0" "\0\0\0\0", 8);
                    throw SocketConnectionClosed();
                }
                bool v4a = (c->request[4] == 0x00) && (c->request[5] == 0x00)
                    && (c->request[6] == 0x00) && (c->request[7] != 0x00);
                size_t i, j;
                i = c->request.find('\0', 8);
                // If we received the authentification string and we are not
                // using v4a or we have received the hostname as well
                if( (i != std::string::npos)
                 && (!v4a || (j = c->request.find('\0', i+1)) !=
                    std::string::npos) )
                {
                    // The request is complete
#ifdef _DEBUG
                    std::cerr << " request is complete\n";
#endif
                    std::string hote;
                    std::string missing;
                    if(!v4a)
                    {
                        std::ostringstream oss;
                        const unsigned char *ureq =
                            (unsigned char*)c->request.c_str();
                        oss << (int)ureq[4] << '.'
                            << (int)ureq[5] << '.'
                            << (int)ureq[6] << '.'
                            << (int)ureq[7];
                        hote = oss.str();
                        missing = c->request.substr(i+1);
                    }
                    else
                    {
                        hote = c->request.substr(i+1, j-i-1);
                        missing = c->request.substr(j+1);
                    }
                    int port;
                    {
                        const unsigned char *ureq =
                            (unsigned char*)c->request.c_str();
                        port = (ureq[2] << 8) | ureq[3];
                    }
                    // Establishes the connection
                    c->request = "";
#ifdef _DEBUG
                    std::cerr << "Socks4Server: proxy.Connect(" << hote << ", "
                        << port << ")...";
#endif
                    try {
                        c->stream = m_pProxy->Connect(hote.c_str(), port);
                    }
                    // FIXME : other exceptions?
                    catch(SocketError &e)
                    {
#ifdef _DEBUG
                        std::cerr << " error\n";
#endif
                        cl->Send("\x00\x5b" "\0\0" "\0\0\0\0", 8);
                        throw SocketConnectionClosed();
                    }
#ifdef _DEBUG
                    std::cerr << " Ok\n";
#endif
                    cl->Send("\x00\x5a" "\0\0", 4);
                    // In every case, send the IPv4 address we reached
                    const unsigned char *ip = Socket::Resolve(hote.c_str());
                    cl->Send(ip?(const char*)ip:"\0\0\0\0", 4);
                    // We add it to the SocketSet, and we store the pair
                    // NetStream => TCPSocket
                    Socket *ul = c->stream->UnderlyingSocket();
                    m_Set.AddSocket(ul);
                    m_aNetStream2Client[ul] = cl;
                    // We send the initial data, which was received with the
                    // request
                    c->stream->Send(missing.c_str(), missing.size());
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
            if(m_aConnections[cl]->stream)
            {
                Socket *ul = m_aConnections[cl]->stream->UnderlyingSocket();
                m_Set.RemoveSocket(ul);
                m_aNetStream2Client.erase(ul);
                delete m_aConnections[cl]->stream;
            }
            m_aConnections.erase(cl);
            m_Set.RemoveSocket(cl);
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
        std::cerr << "Socks4Server: receiving data on a NetStream....";
#endif
        std::map<Socket*, TCPSocket*>::iterator it =
            m_aNetStream2Client.find(sock);
        if(it != m_aNetStream2Client.end())
        {
            TCPSocket *cl = it->second;
            try {
                static char buf[1024];
                int r = m_aConnections[cl]->stream->Recv(buf, 1024, false);
                // Dump copy
                if(r > 0)
                {
#ifdef _DEBUG
                    std::cerr << " forward (" << r << ")\n";
#endif
                    cl->Send(buf, r);
                }
            }
            catch(SocketConnectionClosed &e)
            {
                m_aNetStream2Client.erase(sock);
                if(m_aConnections[cl]->stream)
                    delete m_aConnections[cl]->stream;
                m_aConnections.erase(cl);
                m_Set.RemoveSocket(cl);
                m_Set.RemoveSocket(sock);
                delete cl;
#ifdef _DEBUG
                std::cerr << " connection closed by the remote host\n";
#endif
            }
        }
    }
}
