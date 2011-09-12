#ifndef CLIENTENGINE_H
#define CLIENTENGINE_H

#include "GameEngine.h"

#include "engine/NetClient.h"

class ClientEngine : public GameEngine, public NetClient {

private:
    bool m_bWasConnected;

public:
    ClientEngine(const char *host, int port, int proto);
    void run();
    void stateChanged(const NetClient::State &state);
    void handleServerMessage(const Msg::Data &data);
    void shutdown();
    void input(EKey key, bool pressed);

};


#endif
