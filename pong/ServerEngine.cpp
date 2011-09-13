#include "ServerEngine.h"

ServerEngine::ServerEngine(bool dedicated, int port)
  : GameEngine(GameEngine::ROLE_AUTHORITY, !dedicated),
    NetServer(version(), port), m_bDedicated(dedicated)
{
}

void ServerEngine::run()
{
    if(!m_bDedicated)
        initVideo();
    setup();
    mainLoop();
}

void ServerEngine::clientConnecting(ConnectedClient *client)
{
    // Compare versions
    Msg::Data client_str = client->getClientVersion();
    const unsigned int vserver =
            (PONG_VERSION_MAJOR << 16) | PONG_VERSION_MINOR;
    size_t name_len = strlen(PONG_NET_NAME);
    if(client_str.size() != name_len + 4
     || client_str.substr(0, name_len) != (unsigned char*)PONG_NET_NAME)
    {
        client->disconnect("Incompatible software");
        return ;
    }
    unsigned int vclient = readInt4(&client_str[name_len]);
    if(vserver != vclient)
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

void ServerEngine::clientDisconnected(ConnectedClient *client)
{
    // TODO : Assign a racket to another client?
}

void ServerEngine::handleClientMessage(ConnectedClient *client, const Msg::Data &data)
{
    // TODO : Move a racket
}

void ServerEngine::shutdown()
{
    const std::set<ConnectedClient*> &clients = getClients();
    std::set<ConnectedClient*>::const_iterator c = clients.begin();
    for(; c != clients.end(); ++c)
        (*c)->disconnect("Server shutting down");
}

void ServerEngine::input(EKey key, bool pressed)
{
    // TODO : ServerEngine::input()
    if(pressed)
    {
        switch(key)
        {
        case KEY_UP:
            break;
        case KEY_DOWN:
            break;
        }
    }
    else
        ;
}
