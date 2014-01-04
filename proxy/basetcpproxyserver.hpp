#ifndef BASETCPPROXYSERVER_HPP
#define BASETCPPROXYSERVER_HPP

struct BaseProxyConnection {
    TCPSocket *incoming;
    NetStream *outgoing;
    bool forwarding;

    BaseProxyConnection(TCPSocket *i, NetStream *o = NULL)
      : incoming(i), outgoing(o), forwarding(false)
    {
    }
};

template<class Connection>
class BaseTCPProxyServer : ProxyServer {

private:
    Proxy *m_pProxy;
    TCPServer *m_pSock;
    // Maps incoming connections
    std::map<Waitable*, Connection*> m_aIncoming;
    // Maps outgoing connections
    std::map<Waitable*, Connection*> m_aOutgoing;
    // SocketSet containing the server socket m_pSock, the incoming connections
    // to the clients and the outgoing NetStream objects
    SocketSet m_Set;

public:
    BaseTCPProxyServer(int port, Proxy *proxy = NULL)
      : m_pProy(proxy)
    {
        if(m_pProxy == NULL)
        {
            m_pProxy = TCPClient::getInstance();
#ifdef _DEBUG
            std::cerr << " using direct TCP connection mode...";
#endif
        }
        m_pSock = TCPServer::listen(port);
        m_pSet.add(m_pSock);
#ifdef _DEBUG
        std::cerr << " listening\n";
#endif
    }

    virtual void update(bool bWait = false)
    {
#ifdef _DEBUG
        std::cerr << "update(" << (bWait?"true":"false") << ")\n";
#endif
        Waitable *signaled = m_Set.wait(bWait?-1:0);
        if(signaled == m_pSock)
        {
            // New connection
#ifdef _DEBUG
            std::cerr << "connection from a new client...";
#endif
            TCPSocket *cl = m_pSock->accept(0);
            if(cl)
            {
                Connection *conn = new Connection(cl);
                try {
                    setupConnection(conn);
                }
                catch(SocketError &e)
                {
#ifdef _DEBUG
                    std::cerr << " error\n";
#endif
                    delete cl;
                    cl = NULL;
                }

                if(cl != NULL)
                {
                    m_Set.add(cl);
                    m_aIncoming[cl] = conn;
                    if(conn->outgoing != NULL)
                    {
                        conn->forwarding = true;
                        m_Set.add(conn->outgoing);
                        m_aOutgoing[conn->outgoing] = conn;
#ifdef _DEBUG
                        std::cerr << " ok, stream opened\n";
#endif
                    }
#ifdef _DEBUG
                    else
                        std::cerr << " ok, no stream yet\n";
#endif
                }
            }
#ifdef _DEBUG
            else
                std::cerr << " fail\n";
#endif
        }
        else if(m_aIncoming.find(signaled) != m_aIncoming.end())
        {
            // Receives data from a client
#ifdef _DEBUG
            std::cerr << "receiving data from a client...";
#endif
            Connection *conn = m_aIncoming[signaled];
            TCPSocket *cl = conn->incoming;
            try {
                static char buf[1024];
                int r = cl->recv(buf, 1024, false);
                if(r <= 0)
                    throw SocketConnectionClosed();
                if(conn->forwarding)
                {
                    // Dumb copy
#ifdef _DEBUG
                    std::cerr << " forward (" << r << ")\n";
#endif
                    conn->outgoing->send(buf, r);
                }
                else
                {
                    handleInitialData(conn, buf, r, cl);
                    if(c->outgoing != NULL)
                    {
                        m_Set.add(conn->outgoing);
                        m_aOutgoing[conn->outgoing] = conn;
#ifdef _DEBUG
                        std::cerr << " stream now open\n";
#endif
                    }
                }
            }
            catch(SocketConnectionClosed &e)
            {
                if(conn->outgoing)
                {
                    m_Set.remove(conn->outgoing);
                    m_aOutgoing.erase(conn->outgoing);
                    delete conn->outgoing;
                }
                m_Set.remove(cl);
                m_aIncoming.erase(cl);
                delete conn;
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
            std::cerr << "Forwarder: receiving data on a NetStream...";
#endif
            ForwarderConnection *conn = m_aOutgoing[signaled];
            TCPSocket *cl = conn->incoming;
            NetStream *stream = conn->outgoing;
            try {
                static char buf[1024];
                int r = stream->recv(buf, 1024, false);
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
                m_Set.remove(cl);
                m_Set.remove(stream);
                m_aIncoming.erase(cl);
                m_aOutgoing.erase(stream);
                delete conn;
                delete cl;
                delete stream;
#ifdef _DEBUG
                std::cerr << " connection closed by the remote host\n";
#endif
            }
        }
    }

};

#endif
