#ifndef PROXY_H
#define PROXY_H

#include "sockets/Socket.h"

/**
 * A proxy, through which we can establish a connection.
 *
 * The connection to this proxy can go through another proxy, given to the
 * constructor.
 */
class Proxy {

public:
    virtual NetStream *Connect(const char *host, int port) = 0;

};

/**
 * A stub proxy: direct TCP connection.
 *
 * An object of this class is created by objects which need to establish a
 * connection if no proxy object is explicitly given.
 */
class TCPClient : public Proxy {

public:
    virtual NetStream *Connect(const char *host, int port);

};

#endif
