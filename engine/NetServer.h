#ifndef NETSERVER_H
#define NETSERVER_H

#include "Net.h"
#include "CVar.h"

/**
 * Abstract class representing a client connected to this server.
 */
class ConnectedClient {

private:
    Msg::Data m_ClientVersion;

public:
    inline const Msg::Data &getClientVersion() const
    { return m_ClientVersion; }
    virtual void disconnect(const std::string &reason) = 0;
    virtual void sendMsg(const Msg::Data &data) = 0;

};

/**
 * A network server.
 */
class NetServer : public virtual NetworkEngine, public virtual CVarHandler {

protected:
    /**
     * Method called each tick for replication.
     */
    virtual void update();

public:
    /**
     * Constructor.
     */
    NetServer(const Msg::Data &serverVersion, int port); // FIXME : port?

    /**
     * Method called when a new client connected, before synchronisation.
     *
     * It allows the application to check the client's software and version, do
     * access control (or limit the number of connections) and such.
     */
    virtual void clientConnecting(ConnectedClient *client) = 0;

    /**
     * Method called when a client has finished connecting.
     *
     * It is now synced with the server. For a game, this might be where we want
     * to notify the other players and to create an avatar for the new player.
     */
    virtual void clientConnected(ConnectedClient *client) = 0;

    /**
     * Method handling a message from a client.
     */
    virtual void handleClientMessage(ConnectedClient *client,
            const Msg::Data &data) = 0;

    /**
     * Method called when a new object is created by the application.
     *
     * Registers it and replicate it to clients.
     */
    virtual unsigned int newObject(NetObject *obj);

    /**
     * Method called when an object is destroyed by the application.
     *
     * Destroy this object on all clients.
     */
    virtual void destroyObject(NetObject *obj);

    /**
     * Method called to register a CVar, to be replicated to clients.
     */
    virtual void addCVar(const char *var_name, CVarBase *cvar);

};

#endif
