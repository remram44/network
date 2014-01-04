#include "proxy/Forwarder.h"

#include <iostream>
#include <sstream>

Forwarder::Forwarder(int port, const std::string &target_host, int target_port,
        Proxy *proxy)
  : m_pProxy(proxy), m_sTargetHost(target_host), m_iTargetPort(target_port)
{
#ifdef _DEBUG
    std::cerr << "Forwarder:";
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

void Forwarder::update(bool bWait)
{
#ifdef _DEBUG
    std::cerr << "Forwarder: update(" << (bWait?"true":"false") << ")\n";
#endif
    Waitable *signaled = m_Set.wait(bWait?-1:0);
    if(signaled == m_pSock)
    {
        // New connection
#ifdef _DEBUG
        std::cerr << "Forwarder: connection from a new client...";
#endif
        TCPSocket *cl = m_pSock->accept(0);
        if(cl)
        {
            NetStream *stream;
            try {
                stream = m_pProxy->connect(m_sTargetHost.c_str(),
                        m_iTargetPort);
#ifdef _DEBUG
                std::cerr << " ok\n";
#endif
            }
            // FIXME : other exceptions?
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
                // We add it to the SocketSet, and we store the pair
                // NetStream => TCPSocket
                m_Set.add(cl);
                m_Set.add(stream);
                ForwarderConnection *conn = new ForwarderConnection(
                        cl, stream);
                m_aIncoming[cl] = conn;
                m_aOutgoing[stream] = conn;
            }
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
        std::cerr << "Forwarder: receiving data from a client...";
#endif
        ForwarderConnection *conn = m_aIncoming[signaled];
        TCPSocket *cl = conn->incoming;
        NetStream *stream = conn->outgoing;
        try {
            static char buf[1024];
            int r = cl->recv(buf, 1024, false);
            if(r <= 0)
                throw SocketConnectionClosed();
            // Dumb copy
#ifdef _DEBUG
            std::cerr << " forward (" << r << ")\n";
#endif
            stream->send(buf, r);
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
