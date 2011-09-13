#ifndef SERVERENGINE_H
#define SERVERENGINE_H

#include "engine/NetServer.h"

#include "GameEngine.h"

class ServerEngine : public GameEngine, public NetServer {

private:
    bool m_bDedicated;

public:
    ServerEngine(bool dedicated, int port);
    void run();
    void clientConnecting(ConnectedClient *client);
    void clientConnected(ConnectedClient *client);
    void clientDisconnected(ConnectedClient *client);
    void handleClientMessage(ConnectedClient *client, const Msg::Data &data);
    void shutdown();
    void input(EKey key, bool pressed);

};

#endif
