#ifndef DEDICATEDSERVER_H
#define DEDICATEDSERVER_H

#include "engine/NetServer.h"
#include "GameEngine.h"

class ServerEngine : public GameEngine, public NetServer {

public:
    ServerEngine(int port);
    void run();
    void clientConnecting(ConnectedClient *client);
    void clientConnected(ConnectedClient *client);
    void handleClientMessage(ConnectedClient *client, const Msg::Data &data);

};

#endif // DEDICATEDSERVER_H
