#include "NetServer.h"

NetServer::NetServer(const Msg::Data &serverVersion, int port)
{
    // TODO : NetServer::NetServer()
}

void NetServer::update()
{
    // TODO : NetServer::update()
}

unsigned int NetServer::newObject(NetObject *obj)
{
    // TODO : NetServer::newObject()
    return 0;
}

void NetServer::destroyObject(NetObject *obj)
{
    // TODO : NetServer::destroyObject()
}

void NetServer::addCVar(const char *var_name, CVarBase *cvar)
{
    // TODO : NetServer::addCVar()
}

const std::set<ConnectedClient*> &NetServer::getClients() const
{
    // TODO : NetServer::getClients()
    static std::set<ConnectedClient*> empty; return empty;
}
