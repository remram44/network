#include "Proxy.h"

NetStream *TCPClient::Connect(const char *host, int port)
{
    return TCPSocket::Connect(host, port);
}
