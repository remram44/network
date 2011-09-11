#include "ServerEngine.h"

ServerEngine::ServerEngine(bool dedicated, int port)
  : GameEngine(GameEngine::ROLE_AUTHORITY, !dedicated),
    NetServer(version(), port)
{
}

void ServerEngine::run()
{
    if(!isDedicated())
        initVideo();
    setup();
    mainLoop();
}

void ServerEngine::clientConnecting(ConnectedClient *client)
{
    // Compare versions
    Msg::Data client_str = client->getClientVersion();
    const unsigned int server = (PONG_VERSION_MAJOR << 16) | PONG_VERSION_MINOR;
    size_t name_len = strlen(PONG_NET_NAME);
    if(client_str.size() != name_len + 4
     || client_str.substr(0, name_len) != PONG_NET_NAME)
    {
        client->disconnect("Incompatible software");
        return ;
    }
    unsigned int client = readInt4(&client_str[name_len]);
    if(server != client)
    {
        client->disconnect("Incompatible version");
        return ;
    }
    console_msg("Client connecting...");
}

void ServerEngine::clientConnected(ConnectedClient *client)
{
    // TODO : Assign a racket to the new client
}

void ServerEngine::handleClientMessage(ConnectedClient *client, const Msg::Data &data)
{
    // TODO : Move a racket
}
