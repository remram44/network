#ifndef NET_H
#define NET_H

#include <string>
#include <exception>
#include <map>

class ProtocolMismatch : public std::exception {

private:
    std::string m_sMessage;

public:
    ProtocolMismatch(const char *classname, unsigned int id);
    ProtocolMismatch(const char *classname);
    virtual ~ProtocolMismatch() throw();
    inline const char *what() const throw()
    { return m_sMessage.c_str(); }

};

class ApplicationError : public std::exception {

private:
    std::string m_sMessage;

public:
    ApplicationError(const char *msg);
    virtual ~ApplicationError() throw();
    inline const char *what() const throw()
    { return m_sMessage.c_str(); }

};


/*============================================================================*/

/**
 * A network message.
 *
 * All numbers are sent in big-endian (most significant byte first).
 *
 *  * total size of the message (this header included) (2 bytes)
 *  * message type (2 bytes)
 *  * data (total size - 4 bytes)
 *
 * The data field contains only the data, without header.
 */
class Msg {

public:
    typedef std::basic_string<unsigned char> Data;

public:
    unsigned int type;
    Data data;

public:
    Msg(unsigned int type = 0x0, Data data = Data());

};

/* Utility functions */
/**
 * Reads an integer from 4 bytes.
 */
unsigned int readInt4(const unsigned char *data);
/**
 * Reads a short integer from 2 bytes.
 */
unsigned int readInt2(const unsigned char *data);
/**
 * Writes an integer as 4 bytes.
 */
Msg::Data writeInt4(unsigned int i);
/**
 * Writes an integer as 2 bytes.
 */
Msg::Data writeInt2(unsigned int i);

/**
 * Reads a float from 8 bytes.
 */
float readFloat(const unsigned char *data);
/**
 * Writes a float as 8 bytes.
 */
Msg::Data writeFloat(float f);


/*============================================================================*/

class NetObject;

/**
 * The engine.
 *
 * Base class representing the main object, handling the replication of
 * NetObjects.
 */
class NetworkEngine {

private:
    unsigned short m_iCurrentTick;

protected:
    /**
     * Function overloaded to provide the replication mechanism.
     */
    virtual void update() = 0;

public:
    NetworkEngine();

    /**
     * Function called periodically to advance by 1 tick.
     */
    inline void tick()
    { m_iCurrentTick++; update(); }

    /**
     * Returns the current tick.
     *
     * 0 <= N < 0x8000 and loops.
     */
    inline unsigned short getCurrentTick() const
    { return m_iCurrentTick; }

    /**
     * Function called to register the creation of a new object.
     *
     * @return The ID allocated to this new object.
     */
    virtual unsigned int newObject(NetObject *obj) = 0;

    /**
     * Function called when an object is destroyed.
     */
    virtual void destroyObject(NetObject *obj) = 0;

};

/**
 * Base classe for synchronized objects.
 */
class NetObject {

private:
    NetworkEngine *m_pApp;

    /** Unique object identifier */
    unsigned int m_iID;

    /** Frame in which the object was last modified ;
     * 0 <= N < 0x8000 and loops */
    unsigned short m_iLastModified;

public:
    /**
     * Constructor.
     */
    NetObject(NetworkEngine *app);

    NetObject(NetworkEngine *app, unsigned int id);

    /**
     * Returns the unique identifier of this NetObject.
     */
    inline unsigned int getID() const
    { return m_iID; }

    /**
     * Destructor.
     */
    virtual ~NetObject();

    /**
     * Handles an updated, received from the server.
     */
    virtual void recvUpdate(const Msg::Data &msg) throw(ProtocolMismatch) = 0;

    /**
     * Writes the object's state, to be sent to a client.
     */
    virtual Msg::Data toUpdate() const = 0;

    /**
     * Indicates in which frame the object was last modified.
     * 0 <= N < 0x8000 and loops.
     */
    inline unsigned short lastModified() const
    { return m_iLastModified; }

    /**
     * Flags the object as modified.
     */
    inline void modified()
    { m_iLastModified = m_pApp->getCurrentTick(); }

};

class NetObjectRegistrar {

public:
    typedef NetObject *(*Factory)(
            NetworkEngine *app,
            unsigned int id,
            const Msg::Data &data);

private:
    static std::map<const char*, Factory> factories;

public:
    NetObjectRegistrar(Factory factory, const char *class_id);

};

/**
 * Macro that must be placed after "class {" in subclasses of NetObject that
 * should be replicated to clients.
 */
#define NET_OBJECT_H \
    public: \
        static const char *_class_id; \
        static NetObjectRegistrar _registrar; \
        virtual const char *_getClassId(); \
    private:

/**
 * Macro that must be placed in a source-file in subclasses of NetObject that
 * use NET_OBJECT_H.
 *
 * @param c The full classname, that you would use in this context before :: to
 * define a method/field.
 * @param id An unique string identifying this class.
 */
#define NET_OBJECT_I(c, id) \
    const char *c::_class_id = id; \
    const char *c::_getClassId() \
    { \
        return _class_id; \
    } \
    NetObjectRegistrar c::_registrar(c::newInstance, _class_id)

#endif
