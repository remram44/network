#include "MovingRect.h"

NET_OBJECT_I(MovingRect, "MovingRect");

MovingRect::MovingRect(NetworkEngine *app)
  : NetObject(app)
{
}

MovingRect::MovingRect(NetworkEngine *app, unsigned int id)
  : NetObject(app, id)
{
}

void MovingRect::tick(unsigned int ticks)
{
    x += vx * TIMESTEP * ticks;
    y += vy * TIMESTEP * ticks;
}

void MovingRect::recvUpdate(const Msg::Data &data) throw ProtocolMismatch
{
    if(data.size() != 40)
        throw ProtocolMismatch("MovingRect", getID());

    x = readFloat(&data[ 0]);
    y = readFloat(&data[ 8]);
    w =  readInt2(&data[16]);
    h =  readInt2(&data[18]);
    r = data[20]; g = data[21]; b = data[22];
    vx = readFloat(&data[24]);
    vy = readFloat(&data[32]);
}

Msg::Data MovingRect::toUpdate() const
{
    unsigned char color[4] = {r, g, b, 0};
    return writeFloat(x)
        + writeFloat(y)
        + writeInt2(w)
        + writeInt2(h)
        + Msg::Data(color, 4)
        + writeFloat(vx)
        + writeFloat(vy);
}

NetObject *MovingRect::newInstance(NetworkEngine *app, unsigned int id,
        const Msg::Data &data)
{
    MovingRect *r = new MovingRect(app, id);
    r->recvUpdate(data);
    return r;
}
