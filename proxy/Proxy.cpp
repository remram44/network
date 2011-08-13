#include "Proxy.h"

TCPClient *TCPClient::instance = NULL;

TCPClient *TCPClient::getInstance()
{
    if(instance == NULL)
        instance = new TCPClient();
    return instance;
}

NetStream *TCPClient::Connect(const char *host, int port)
{
    return TCPSocket::Connect(host, port);
}
