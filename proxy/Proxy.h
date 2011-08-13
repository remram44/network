#ifndef PROXY_H
#define PROXY_H

#include "sockets/Socket.h"
#include "sockets/TCP.h"

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
 * This class is a very simple implementation of the Proxy interface, used by
 * objects that need to establish a connection when no proxy object is
 * explicitly given.
 * It can be used instead of a real Proxy instance to unify connection methods.
 *
 * The unique instance of this class can be accessed through
 * TCPClient::getInstance().
 */
class TCPClient : public Proxy {

private:
    static TCPClient *instance;

private:
    TCPClient() {}

public:
    static TCPClient *getInstance();
    virtual NetStream *Connect(const char *host, int port);

};

#endif
