#ifndef MOVINGRECT_H
#define MOVINGRECT_H

#include "engine/Net.h"

/**
 * Base class of the objects in a pong game.
 *
 * A MovingRect is a rectangle moving on the screen (position, size, speed and
 * color).
 */
class MovingRect : public NetObject {

    NET_OBJECT_H

public:
    float x, y;
    unsigned int w, h;
    float vx, vy;
    unsigned char r, g, b;

public:
    MovingRect(NetworkEngine *app);
    MovingRect(NetworkEngine *app, unsigned int id);

    void tick(unsigned int ticks);

    void recvUpdate(const Msg::Data &data) throw ProtocolMismatch;
    Msg::Data toUpdate() const;

    /**
     * Factory function, used for client-side instanciation of NetObjects.
     */
    static NetObject *newInstance(NetworkEngine *app, unsigned int id,
            const Msg::Data &data);

};

#endif
