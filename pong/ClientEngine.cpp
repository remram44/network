#include "ClientEngine.h"

ClientEngine::ClientEngine(const char *host, int port, int proto)
  : GameEngine(GameEngine::ROLE_CLIENT, true),
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

void ClientEngine::stateChanged(const NetClient::State &state)
{
    switch(state.state())
    {
    case NetClient::DISCONNECTED:
        {
            const char *reason =
                    static_cast<const NetClient::DisconnectedState&>(
                    state).getDetails();
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
            Msg::Data server_str =
                    static_cast<const NetClient::ConnectedState&>(
                    state).getServerVersion();
            const unsigned int client = (PONG_VERSION_MAJOR << 16)
                    | PONG_VERSION_MINOR;
            size_t name_len = strlen(PONG_NET_NAME);
            if(server_str.size() != name_len + 4
             || server_str.substr(0, name_len) != (unsigned char*)PONG_NET_NAME)
            {
                disconnect("Unknown server software or invalid response");
                return ;
            }
            unsigned int server = readInt4(&server_str[name_len]);
            if(client != server)
            {
                disconnect("Server uses incompatible version");
                return ;
            }
            reportProgress(0.1f, "Connected; versions are compatible");
        }
        break;
    case NetClient::RECVING_OBJECTS:
        {
            float progress;
            const NetClient::SyncingState &sstate =
                    static_cast<const NetClient::SyncingState&>(state);
            unsigned int max = sstate.getTotalObjects();
            if(max <= 0)
                progress = 0.5f; // Unknown
            else
                progress = (1.f*sstate.getRecvdObjects())/max;
            reportProgress(0.1f + 0.9f * progress,
                    "Receiving objects from server...");
        }
        break;
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
            sendClientMsg(Msg::Data((unsigned char*)"\x01", 1));
            break;
        case KEY_DOWN:
            sendClientMsg(Msg::Data((unsigned char*)"\x02", 1));
            break;
        }
    }
    else
        sendClientMsg(Msg::Data((unsigned char*)"\x00", 1));
}
