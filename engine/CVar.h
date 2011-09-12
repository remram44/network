#ifndef CVAR_H
#define CVAR_H

#include "Net.h"

class CVarBase;

/**
 * Interface for the class handling the CVars.
 */
class CVarHandler {

public:
    virtual void addCVar(const char *var_name, CVarBase *cvar) = 0;

};

/**
 * Base class of CVars.
 */
class CVarBase {

private:
    CVarHandler *m_pApp;
    const char *m_pDescription;

public:
    CVarBase(CVarHandler *app, const char *var_name, const char *var_desc);
    virtual void recvUpdate(const Msg::Data &data) = 0;
    virtual Msg::Data toUpdate() const = 0;

};

/**
 * A replicated variable set by the server.
 */
template<typename T>
class CVar : public CVarBase {

private:
    T m_Value;

public:
    CVar(CVarHandler *app, const char *var_name, const char *var_desc,
            T defvalue);
    const T &operator*() const;
    void recvUpdate(const Msg::Data &data);
    Msg::Data toUpdate() const;
    void set(const T &value);

};


/*============================================================================*/

template<typename T>
CVar<T>::CVar(CVarHandler *app, const char *var_name, const char *var_desc,
        T defvalue)
  : CVarBase(app, var_name, var_desc), m_Value(defvalue)
{
}

template<typename T>
const T &CVar<T>::operator*() const
{
    return m_Value;
}

template<typename T>
void CVar<T>::set(const T &value)
{
    m_Value = value;
}

#endif
