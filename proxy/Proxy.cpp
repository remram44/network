#include "proxy/Proxy.h"

TCPClient *TCPClient::instance = NULL;

TCPClient *TCPClient::getInstance()
{
    if(instance == NULL)
        instance = new TCPClient();
    return instance;
}

NetStream *TCPClient::connect(const char *host, int port)
{
    return TCPSocket::connect(host, port);
}
