#ifndef NETCLIENT_H
#define NETCLIENT_H

#include "Net.h"
#include "CVar.h"

class NetClient : public virtual NetworkEngine, public virtual CVarHandler {

public:
    enum EState {
        /** Couldn't connect to the server, or disconnected. */
        DISCONNECTED,
        /** Connection established, waiting for server to begin
         * synchronisation. */
        CONNECTED,
        /** Synchronising: receiving objects from server. */
        RECVING_OBJECTS,
        /** Synced: hooray \o/ */
        SYNCED
    };

    // FIXME : union?
    class State {

    public:
        virtual EState state() const = 0;

    };

    class DisconnectedState : public State {

    private:
        std::string m_sDetails;

    public:
        EState state() const; // DISCONNECTED
        const char *getDetails() const;

    };

    class ConnectedState : public State {

    private:
        Msg::Data m_ServerVersion;

    public:
        EState state() const; // CONNECTED
        const Msg::Data &getServerVersion() const;

    };

    class SyncingState : public State {

    private:
        unsigned int m_iTotalObjects;
        unsigned int m_iRecvdObjects;

    public:
        EState state() const; // RECVING_OBJECTS
        unsigned int getTotalObjects() const;
        unsigned int getRecvdObjects() const;
        float getProgress() const;

    };

    class SyncedState : public State {

    public:
        EState state() const; // SYNCED

    };

protected:
    /**
     * Method called each tick for replication.
     */
    virtual void update();

public:
    /**
     * Constructor.
     */
    NetClient(const Msg::Data &clientVersion, const char *host, int port,
            int proto); // FIXME : host, port, proto?

    /**
     * Method called when the state of the client changes.
     *
     * Used during connection/synchronisation, and to handle disconnection.
     */
    virtual void stateChanged(const NetClient::State &state) = 0;

    /**
     * Method handling a special message from the server.
     */
    virtual void handleServerMessage(const Msg::Data &data) = 0;

    /**
     * Send a special message to the server.
     */
    void sendClientMsg(const Msg::Data &data);

    /**
     * Method called when an object is created by the application.
     *
     * @warning This method should never be called! It will throw an exception!
     * It is an error to instanciate a NetObject on the client; they are only
     * created by the server and then replicated automatically to clients.
     */
    virtual unsigned int newObject(NetObject *obj);

    /**
     * Method called when an object is destroyed by the application.
     *
     * This method does nothing.
     */
    virtual void destroyObject(NetObject *obj);

    /**
     * Block until synchronisation with server is complete.
     *
     * stateChanged() will be calledto report progress.
     * You can also call update() in a loop if you prefer.
     * @return True if we are now synced with the server, false elsewise
     * (stateChanged() would then have already been called with a
     * DisconnectedState describing the problem).
     */
    bool syncWithServer();

    /**
     * Terminate the connection to the server.
     *
     * The given message is sent, if specified.
     */
    void disconnect(const std::string &reason);

    /**
     * Method called to register a CVar, to keep in sync with the server.
     */
    virtual void addCVar(const char *var_name, CVarBase *cvar);

};

#endif
