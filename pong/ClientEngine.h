#ifndef CLIENTENGINE_H
#define CLIENTENGINE_H

#include "GameEngine.h"

#include "engine/NetClient.h"

class ClientEngine : public GameEngine, public NetClient {

private:
    bool m_bWasConnected;

public:
    ClientEngine(const char *host, int port, NetClient::EProto proto);
    void run();
    void stateChanged(NetClient::EState state);
    void handleServerMessage(const Msg::Data &data);
    void input(EKey key, bool pressed);

};


#endif
