#include "Proxy.h"

NetStream *TCPClient::Connect(const char *hote, int port)
{
    return TCPSocket::Connect(hote, port);
}
