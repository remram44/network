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
 * A connected TCP socket.
 *
 * The most simple socket, on which we can send and receive data.
 */
class ClientSocket : public Socket {

public:
    /**
     * Constructor from a previously connected socket.
     */
    ClientSocket(int sock);

    /**
     * Static method establishing a new connection.
     *
     * @param hote Hostname, for instance "www.debian.com".
     * @param port Port number on which to connect.
     */
    static ClientSocket *Connect(const char *host, int port)
        throw(SocketUnknownHost, SocketConnectionRefused);

    /**
     * Sends data.
     *
     * @param data Raw data to send.
     * @param size Size (in bytes) of the data.
     */
    void Send(const char *data, size_t size) throw(SocketConnectionClosed);

    /**
     * Receives data.
     *
     * @param bWait Indicates whether we should wait for data to arrive, if
     * nothing is initially available. If false, this method can return 0.
     * @return Number of bytes that were received.
     */
    int Recv(char *data, int size_max, bool bWait = true)
        throw(SocketConnectionClosed);

};


/*============================================================================*/

/**
 * A TCP server socket.
 *
 * Allows to listen on a port and accept TCP connections.
 */
class ServerSocket : public Socket {

public:
    /**
     * Constructor from a socket already listening on a port.
     */
    ServerSocket(int sock);

    /**
     * Static method creating a server socket listening on a given port number.
     *
     * @param port Port number on which to listen for connections.
     */
    static ServerSocket *Listen(int port) throw(SocketCantUsePort);

    /**
     * Accepts one connection from a client.
     *
     * @param timeout Maximum time (in milliseconds) to wait for a connection. 0
     * returns immediately, and a negative value means to wait forever.
     * @return A socket connected to the client, or NULL if no client attempted
     * to connected after this time.
     */
    ClientSocket *Accept(int timeout = 0);

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
