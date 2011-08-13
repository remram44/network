#ifndef SOCKET_H
#define SOCKET_H

#include <cstdio>
#include <cstring>           /* For memset() */
#include <exception>
#include <set>
#include <sstream>

#ifdef __WIN32__
    #include <winsock2.h>

    typedef int socklen_t;
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/param.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <signal.h>
    #include <netdb.h>
    #include <errno.h>
    #include <unistd.h>
    
    typedef int SOCKET;
#endif


/*==============================================================================
 * Exceptions.
 *
 * Class SocketError and subclasses.
 */

 /**
 * An error from this module.
 *
 * Base class for subclassing.
 */
class SocketError : public std::exception {
public:
    virtual const char *what() = 0;
};

/**
 * Fatal error.
 *
 * An error that really shouldn't happen and is not caused by the use the
 * application makes of this module. If such an exception is thrown, we are in
 * big trouble, and this module is probably unusable. This can come from an
 * error or a limit from the OS (firewall?).
 */
class SocketFatalError : public SocketError {
public:
    const char *what();
};

/**
 * Host cannot be found.
 *
 * If the hostname we want to connect to cannot be resolved to an IP address.
 */
class SocketUnknownHost : public SocketError {
public:
    const char *what();
};

/**
 * Connection refused.
 *
 * If the peer we want to reach exists but actively informs us that it won't
 * accept a connection on the given port number.
 */
class SocketConnectionRefused : public SocketError {
public:
    const char *what();
};

/**
 * Connection closed.
 *
 * If the connection has been closed and thus cannot be used for communication.
 */
class SocketConnectionClosed : public SocketError {
public:
    const char *what();
};

/**
 * Impossible to grab the requested port numbre.
 *
 * If we want to bind to a local port number that is already in use or that we
 * are not allowed to use.
 */
class SocketCantUsePort : public SocketError {
public:
    const char *what();
};


/*============================================================================*/

/**
 * A socket.
 *
 * Base class, useful notably to SocketSets.
 */
class Socket {

private:
    int m_iSocket;

public:
    /**
     * Base constructor.
     */
    Socket(int sock);

    /**
     * Destructor: closes the socket.
     */
    virtual ~Socket();

    bool operator==(const Socket& s);
    bool operator<(const Socket& s);

    /**
     * Waits for a change on this socket.
     *
     * @param timeout Maximum time (in milliseconds) to wait for an event. 0
     * returns immediately, and a negative value means to wait forever.
     * @return true If the socket was modified (data are available for reading,
     * someone wants to connect, the connection was closed, ...) during the
     * given delay, or false if nothing happened after this time.
     */
    bool Wait(int timeout = -1) const;

    /**
     * Accessor for the socket.
     *
     * Useful to subclasses.
     */
    inline int GetSocket() const
    {
        return m_iSocket;
    }

    /**
     * Resolves a hostname and returns the 4 bytes of its IPv4 address.
     */
    static const unsigned char *Resolve(const char *name);

public:
    /**
     * Initializes the module.
     */
    static void Init() throw(SocketFatalError);

    /**
     * Frees the socket: it is free()d by this module, but the file descriptor
     * is not closed. Instead, it is returned and can be used through the native
     * system calls.
     */
    static int Unlock(Socket *s);

};


/*============================================================================*/

/**
 * A bidirectional network connection.
 *
 * This interface represents any type of bytestream, for instance a raw TCP
 * socket, a SSL transmission, the traversal of one or more proxies, ...
 */
class NetStream {

public:
    virtual ~NetStream() {}

    /**
     * Sends data.
     *
     * @param data Raw data to send.
     * @param size Size (in bytes) of the data.
     */
    virtual void Send(const char *data, size_t size)
        throw(SocketConnectionClosed) = 0;

    /**
     * Receives data.
     *
     * @param bWait Indicates whether we should wait for data to arrive, if
     * nothing is initially available. If false, this method can return 0.
     * @return Number of bytes that were received.
     */
    virtual int Recv(char *data, size_t size_max, bool bWait = true)
        throw(SocketConnectionClosed) = 0;

    /**
     * The underlying socket, if any.
     *
     * If it has a meaning, the underlying socket used to communicate, for
     * instance with the first proxy server. Obviously, it shouldn't be used for
     * communication, as the transmission can use one or more protocols on top
     * of this socket.
     * If it doesn't have any meaning, this function will return NULL.
     */
    virtual Socket *UnderlyingSocket() = 0;

};


/*============================================================================*/

/**
 * A group of sockets.
 *
 * By putting several sockets in a SocketSet, we can put the process to sleep
 * until something happens to either one of them.
 */
class SocketSet {

private:
    std::set<Socket*> m_Sockets;

public:
    /**
     * Indicates whether a socket is already in this group.
     */
    bool IsSet(Socket *sock);

    /**
     * Adds a socket to this group.
     */
    void AddSocket(Socket *sock);

    /**
     * Removes une socket from this group.
     *
     * @return true If this socket was previously in the group, false elsewise.
     */
    bool RemoveSocket(Socket *sock);

    /**
     * Removes all the sockets from this group.
     */
    void Clear();

    /**
     * Waits for a change on the sockets of this group.
     *
     * @param timeout Maximum time (in milliseconds) to wait for an event. 0
     * returns immediately, and a negative value means to wait forever.
     * @return NULL if no socket was modified, or one of the modified sockets if
     * something happened.
     */
    Socket *Wait(int timeout = -1);

};

#endif
