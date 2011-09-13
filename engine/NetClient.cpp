#include "NetClient.h"


/*==============================================================================
 * Client states.
 */


/* DisconnectedState */

NetClient::EState NetClient::DisconnectedState::state() const
{
    return NetClient::DISCONNECTED;
}

const char *NetClient::DisconnectedState::getDetails() const
{
    return m_sDetails.c_str();
}


/* ConnectedState */

NetClient::EState NetClient::ConnectedState::state() const
{
    return NetClient::CONNECTED;
}

const Msg::Data &NetClient::ConnectedState::getServerVersion() const
{
    return m_ServerVersion;
}


/* SyncingState */

NetClient::EState NetClient::SyncingState::state() const
{
    return NetClient::RECVING_OBJECTS;
}

unsigned int NetClient::SyncingState::getTotalObjects() const
{
    return m_iTotalObjects;
}

unsigned int NetClient::SyncingState::getRecvdObjects() const
{
    return m_iRecvdObjects;
}

float NetClient::SyncingState::getProgress() const
{
    if(m_iTotalObjects > 0)
        return (1.f * m_iRecvdObjects)/m_iTotalObjects;
    else
        return -1.f;
}


/* SyncedState */

NetClient::EState NetClient::SyncedState::state() const
{
    return NetClient::SYNCED;
}


/*==============================================================================
 * NetClient.
 */

NetClient::NetClient(const Msg::Data &clientVersion, const char *host, int port,
        int proto)
{
    // TODO : NetClient::NetClient()
}

void NetClient::update()
{
    // TODO : NetClient::update()
}

void NetClient::sendClientMsg(const Msg::Data &data)
{
    // TODO : sendClientMsg()
}

unsigned int NetClient::newObject(NetObject *obj)
{
    throw ApplicationError("NetObject created on the client!");
}

void NetClient::destroyObject(NetObject *obj)
{
}

bool NetClient::syncWithServer()
{
    // TODO : NetClient::syncWithServer()
    return false;
}

void NetClient::disconnect(const std::string &reason)
{
    // TODO : NetClient::disconnect()
}

void NetClient::addCVar(const char *var_name, CVarBase *cvar)
{
    // TODO : NetClient::addCVar()
}
