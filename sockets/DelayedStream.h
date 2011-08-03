#ifndef DELAYEDSTREAM_H
#define DELAYEDSTREAM_H

#include "Socket.h"
#include <pthread.h>

class DelayedStream : public NetStream {

private:
    pthread_t m_Thread;
    int m_iDelay;
    TCPServer *m_TempServer; // used to create the two sockets
    TCPSocket *m_AppSocket, *m_InterSocket; // two inter-connected sockets
    NetStream *m_Uplink;
    SocketSet m_Set;

private:
    static void *run_wrapper(void *ptr);

public:
    DelayedStream(NetStream *upstream, int delay);
    ~DelayedStream();
    void Send(const char *data, size_t size) throw (SocketConnectionClosed);
    int Recv(char *data, size_t size_max, bool bWait)
            throw (SocketConnectionClosed);
    Socket *UnderlyingSocket();

private:
    void run();

};

#endif
