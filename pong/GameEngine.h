#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "engine/Net.h"
#include <set>

#include "MovingRect.h"
#include "Racket.h"

#define PONG_NAME "Networked Pong Application"
#define PONG_NET_NAME "NETWORKED_PONG"
#define PONG_VERSION_MAJOR 0
#define PONG_VERSION_MINOR 1

#define PONG_VERSION #PONG_VERSION_MAJOR "." #PONG_VERSION_MINOR
#define PONG_FULLNAME PONG_NAME " v" PONG_VERSION

#define TIMESTEP 0.1f

class GameEngine : public virtual NetworkEngine {

public:
    enum ERole {
        ROLE_AUTHORITY,
        ROLE_CLIENT
    };

private:
    ERole m_eRole;
    bool m_bDisplay;
    std::set<MovingRect*> m_Rects;
    Racket *m_Rackets[2];
    MovingRect *m_pBall;

    unsigned int m_iLastFrame;

    CVar<float> ball_speed;
    CVar<float> racket_speed;

public:
    GameEngine(ERole role, bool display);
    void setup();
    void mainLoop();
    void tick();

};

#endif
