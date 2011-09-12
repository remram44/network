#include "CVar.h"

#include <string>

CVarBase::CVarBase(CVarHandler *app, const char *var_name,
        const char *var_desc)
  : m_pApp(app), m_pDescription(var_desc)
{
    m_pApp->addCVar(var_name, this);
}


/*==============================================================================
 * CVar<float>
 */

template<>
void CVar<float>::recvUpdate(const Msg::Data &data)
{
    if(data.size() != 8)
        throw ProtocolMismatch("CVar<float>");
    m_Value = readFloat(&data[0]);
}

template<>
Msg::Data CVar<float>::toUpdate() const
{
    return writeFloat(m_Value);
}


/*==============================================================================
 * CVar<int>
 */

template<>
void CVar<int>::recvUpdate(const Msg::Data &data)
{
    if(data.size() != 4)
        throw ProtocolMismatch("CVar<int>");
    m_Value = readInt4(&data[0]);
}

template<>
Msg::Data CVar<int>::toUpdate() const
{
    return writeInt4(m_Value);
}


/*==============================================================================
 * CVar<std::string>
 */

template<>
void CVar<std::string>::recvUpdate(const Msg::Data &data)
{
    m_Value = std::string((const char*)&data[0], data.size());
}

template<>
Msg::Data CVar<std::string>::toUpdate() const
{
    return Msg::Data((unsigned char*)m_Value.c_str(), m_Value.size());
}
