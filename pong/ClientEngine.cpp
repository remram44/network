#include "ClientEngine.h"

ClientEngine::ClientEngine(const char *host, int port, NetClient::EProto proto)
  : GameEngine(GameEngine::ROLE_CLIENT),
    NetClient(version(), host, port, proto), m_bWasConnected(false)
{
}

void ClientEngine::run()
{
    initVideo();
    reportProgress(0.f, "Initializing...\n");
    if(syncWithServer()) // While syncing, stateChanged() will be called
            // regularly
    {
        mainLoop();
    }
    else
    {
        SDL_Delay(5000);
    }
}

void ClientEngine::stateChanged(NetClient::State *state)
{
    switch(state.type())
    {
    case NetClient::DISCONNECTED:
        {
            const char *reason = static_cast<NetClient::DisconnectedState*>(
                    state)->getDetails();
            if(reason == NULL)
                reason = "unknown";
            if(m_bWasConnected)
            {
                reportProgress(-1.0f,
                        std::string("Error: disconnected from the server\n"
                        "Reason: ") + reason);
            }
            else
            {
                reportProgress(-1.0f,
                        std::string("Unable to connect to the server\n"
                        "Reason: ") + reason);
            }
        }
        break;
    case NetClient::CONNECTED:
        {
            // Compare versions
            Msg::Data server_str = static_cast<NetClient::ConnectedState*>(
                    state)->getServerVersion();
            const unsigned int client = (PONG_VERSION_MAJOR << 16)
                    | PONG_VERSION_MINOR;
            size_t name_len = strlen(PONG_NET_NAME);
            if(server_str.size() != name_len + 4
             || server_str.substr(0, name_len) != PONG_NET_NAME)
            {
                disconnect("Unknown server software or invalid response");
                return ;
            }
            unsigned int server = readInt4(&server_str[name_len]);
            if(client != server)
            {
                disconnected("Server uses incompatible version");
                return ;
            }
            reportProgress(0.1f, "Connected; versions are compatible");
        }
        break;
    case NetClient::RECVING_OBJECTS:
        {
            float progress;
            NetClient::SyncingState *sstate =
                    static_cast<NetClient::SyncingState*>(state);
            int max = sstate->getTotalObjects();
            if(max <= 0)
                progress = 0.5f; // Unknown
            else
                progress = (1.f*sstate->getRecvdObjects())/max;
        }
        reportProgress(0.1f + 0.9f * progress,
                "Receiving objects from server...");
    case NetClient::SYNCED:
        reportProgress(1.0f, "Synced with server");
        break;
    }
}

void ClientEngine::handleServerMessage(const Msg::Data &data)
{
    // Nothing to see here
}

void ClientEngine::input(EKey key, bool pressed)
{
    if(pressed)
    {
        switch(key)
        {
        case KEY_UP:
            sendClientMsg(Msg::Data("\x01"));
            break;
        case KEY_DOWN:
            sendClientMsg(Msg::Data("\x02"));
            break;
        }
    }
    else
        sendClientMsg(Msg::Data("\x00"));
}
