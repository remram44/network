#include "Forwarder.h"

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
    m_pSock = TCPServer::Listen(port);
    m_Set.AddSocket(m_pSock);
#ifdef _DEBUG
    std::cerr << " listening\n";
#endif
}

void Forwarder::update(bool bWait)
{
#ifdef _DEBUG
    std::cerr << "Forwarder: update(" << (bWait?"true":"false") << ")\n";
#endif
    Socket *sock = m_Set.Wait(bWait?-1:0);
    if(sock == m_pSock)
    {
        // New connection
#ifdef _DEBUG
        std::cerr << "Forwarder: connection from a new client...";
#endif
        TCPSocket *cl = m_pSock->Accept(0);
        if(cl)
        {
            try {
                NetStream *stream = m_pProxy->Connect(m_sTargetHost.c_str(),
                    m_iTargetPort);
                m_aConnections[cl] = stream;
                m_Set.AddSocket(cl);
                // We add it to the SocketSet, and we store the pair
                // NetStream => TCPSocket
                Socket *ul = stream->UnderlyingSocket();
                m_Set.AddSocket(ul);
                m_aNetStream2Client[ul] = cl;
#ifdef _DEBUG
                std::cerr << " ok\n";
#endif
            }
            // FIXME : other exceptions?
            catch(SocketError &e)
            {
#ifdef _DEBUG
                std::cerr << " error";
#endif
                delete cl;
            }
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
        std::cerr << "Forwarder: receiving data from a client...";
#endif
        TCPSocket *cl = (TCPSocket*)sock;
        try {
            NetStream *stream = m_aConnections[cl];
            static char buf[1024];
            int r = cl->Recv(buf, 1024, false);
            if(r <= 0)
                throw SocketConnectionClosed();
            // Dump copy
#ifdef _DEBUG
            std::cerr << " forward (" << r << ")\n";
#endif
            stream->Send(buf, r);
        }
        catch(SocketConnectionClosed &e)
        {
            if(m_aConnections[cl])
            {
                Socket *ul = m_aConnections[cl]->UnderlyingSocket();
                m_Set.RemoveSocket(ul);
                m_aNetStream2Client.erase(ul);
                delete m_aConnections[cl];
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
        std::cerr << "Forwarder: receiving data on a NetStream...";
#endif
        std::map<Socket*, TCPSocket*>::iterator it =
            m_aNetStream2Client.find(sock);
        if(it != m_aNetStream2Client.end())
        {
            TCPSocket *cl = it->second;
            try {
                static char buf[1024];
                int r = m_aConnections[cl]->Recv(buf, 1024, false);
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
                if(m_aConnections[cl])
                    delete m_aConnections[cl];
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
