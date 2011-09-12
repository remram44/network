#include "Net.h"

#include <iostream>
#include <cstring>
#include <stdexcept>
#include <cmath>
#include <climits>

#include <string>
#include <sstream>
#include <iomanip>


ProtocolMismatch::ProtocolMismatch(const char *classname, unsigned int id)
{
    std::ostringstream oss;
    oss << "Protocol mismatch receiving an update for " << classname
            << " #" << id;
    m_sMessage = oss.str();
}

ProtocolMismatch::ProtocolMismatch(const char *classname)
{
    std::ostringstream oss;
    oss << "Protocol mismatch receiving an update for " << classname;
    m_sMessage = oss.str();
}

ProtocolMismatch::~ProtocolMismatch() throw()
{
}

ApplicationError::ApplicationError(const char *msg)
  : m_sMessage(msg)
{
}

ApplicationError::~ApplicationError() throw()
{
}


/*==============================================================================
 * Utility functions.
 */

std::string dump_msg(const Msg &msg)
{
    std::ostringstream s;
    s << "Msg(";
    switch(msg.type)
    {
    //case ...: s << "..."; break;
    default: s << "0x" << std::hex << (unsigned int)msg.type << std::dec; break;
    }
    s << ", \"";
    size_t i;
    for(i = 0; i < msg.data.size(); i++)
    {
        if(msg.data[i] >= 32 && msg.data[i] <= 126)
            s << msg.data[i];
        else
            s << "\\x" << std::setfill('0') << std::setw(2) << std::hex
                << (int)msg.data[i];
    }
    s << "\")";
    return s.str();
}

Msg::Msg(unsigned int _type, Msg::Data _data)
  : type(_type), data(_data)
{
}

unsigned int readInt4(const unsigned char *data)
{
    return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
}

unsigned int readInt2(const unsigned char *data)
{
    return data[0] | (data[1] << 8);
}

Msg::Data writeInt4(unsigned int i)
{
    Msg::Data d;
    d.resize(4);
    d[0] =  i        & 0xFF;
    d[1] = (i >> 8 ) & 0xFF;
    d[2] = (i >> 16) & 0xFF;
    d[3] = (i >> 24) & 0xFF;
    return d;
}

Msg::Data writeInt2(unsigned int i)
{
    Msg::Data d;
    d.resize(2);
    d[0] =  i        & 0xFF;
    d[1] = (i >> 8 ) & 0xFF;
    return d;
}

float readFloat(const unsigned char *data)
{
    int f_exp = readInt4(data);
    long f_mant = readInt4(data + 4);
    float f = scalbnf((fabsf(f_mant) / LONG_MAX) + 1, f_exp);
    if(f_mant < 0)
        f = -f;
    return f;
}

Msg::Data writeFloat(float f)
{
    int f_exp;
    long f_mant;

    f_exp = ilogbf(f);
    f_mant = (scalbnf(fabsf(f), -f_exp) - 1) * LONG_MAX;
    if(f < 0.0)
        f_mant = -f_mant;
    return writeInt4(f_exp) + writeInt4(f_mant);
}


/*============================================================================*/

NetObject::NetObject(NetworkEngine *app)
  : m_pApp(app)
{
    m_iLastModified = m_pApp->getCurrentTick();
    m_iID = m_pApp->newObject(this);
}

NetObject::NetObject(NetworkEngine *app, unsigned int id)
  : m_pApp(app), m_iID(id)
{
    m_iLastModified = m_pApp->getCurrentTick();
}

NetObject::~NetObject()
{
    m_pApp->destroyObject(this);
}

std::map<const char*, NetObjectRegistrar::Factory>
        NetObjectRegistrar::factories;

NetObjectRegistrar::NetObjectRegistrar(NetObjectRegistrar::Factory factory,
        const char *class_id)
{
    factories[class_id] = factory;
}
