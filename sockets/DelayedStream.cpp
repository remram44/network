#include "DelayedStream.h"

#include <string>
#include <list>
#include <ctime>

void *DelayedStream::run_wrapper(void *ptr)
{
    DelayedStream *inst = (DelayedStream*)ptr;
    inst->run();
    return NULL;
}

DelayedStream::DelayedStream(NetStream *upstream, int delay)
  : m_iDelay(delay), m_Uplink(upstream)
{
    // We need 2 interconnected sockets
    // The main thread creates a TCP server, the thread will connect
    m_TempServer = TCPServer::Listen(0); // random port

    pthread_create(&m_Thread, NULL, run_wrapper, (void*)this);

    // The thread will connect
    m_AppSocket = m_TempServer->Accept(-1);
    delete m_TempServer; m_TempServer = NULL;
}

DelayedStream::~DelayedStream()
{
    delete m_AppSocket;
    // Don't delete the other sockets! The thread will do it
    pthread_join(m_Thread, NULL);
}

void DelayedStream::Send(const char *data, size_t size) throw (SocketConnectionClosed)
{
    m_AppSocket->Send(data, size);
}

int DelayedStream::Recv(char *data, size_t size_max, bool bWait)
        throw (SocketConnectionClosed)
{
    return m_AppSocket->Recv(data, size_max, bWait);
}

Socket *DelayedStream::UnderlyingSocket()
{
    return m_AppSocket;
}

struct Event {
    time_t when;
    std::string what;
    NetStream *where;
};

void DelayedStream::run()
{
    m_InterSocket =
            TCPSocket::Connect("localhost", m_TempServer->GetLocalPort());
    m_Set.AddSocket(m_InterSocket);
    m_Set.AddSocket(m_Uplink->UnderlyingSocket());

    char buffer[512];
    std::list<Event> queue;
    try {
        for(;;)
        {
            Socket *sock;
            if(queue.size() > 0)
            {
                std::list<Event>::iterator it = queue.begin();
                time_t nextEvent = it->when;
                if(nextEvent > time(NULL))
                {
                    nextEvent -= time(NULL);
                    sock = m_Set.Wait(nextEvent * 1000);
                }
                else
                    sock = m_Set.Wait(0);
            }
            else
                sock = m_Set.Wait(-1);

            if(sock != NULL)
            {
                NetStream *stream;
                if(sock == m_InterSocket)
                    stream = m_InterSocket;
                else
                    stream = m_Uplink;
                int ret = stream->Recv(buffer, 512, false);
                if(ret > 0)
                {
                    time_t t = time(NULL);
                    Event event;
                    event.when = t + m_iDelay;
                    event.what = std::string(buffer, ret);
                    event.where = (sock == m_InterSocket)?
                            m_Uplink:m_InterSocket;
                    queue.push_back(event);
                }
            }

            std::list<Event>::iterator it = queue.begin();
            while(it != queue.end() && time(NULL) >= it->when)
            {
                NetStream *target = it->where;
                target->Send(it->what.c_str(), it->what.size());
                std::list<Event>::iterator tmp = it;
                ++it;
                queue.erase(tmp);
            }
        }
    } catch(SocketConnectionClosed) {
    }
    delete m_InterSocket;
    delete m_Uplink;
}
