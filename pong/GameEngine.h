#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "MovingRect.h"

#include "engine/Net.h"
#include "engine/CVar.h"
#include <SDL/SDL.h>
#include <set>

// This is f*cked up
#define QUOTEME(x) #x
#define QUOTE(x) QUOTEME(x)

#define PONG_NAME "Networked Pong Application"
#define PONG_NET_NAME "NETWORKED_PONG"
#define PONG_VERSION_MAJOR 0
#define PONG_VERSION_MINOR 1

#define PONG_VERSION QUOTE(PONG_VERSION_MAJOR) "." QUOTE(PONG_VERSION_MINOR)
#define PONG_FULLNAME PONG_NAME " v" PONG_VERSION

#define TIMESTEP 0.1f

class GameEngine : public virtual NetworkEngine, public virtual CVarHandler {

public:
    enum ERole {
        ROLE_AUTHORITY,
        ROLE_CLIENT
    };

    enum EKey {
        KEY_UP,
        KEY_DOWN
    };

private:
    ERole m_eRole;
    bool m_bDisplay;
    SDL_Surface *m_pScreen;
    std::set<MovingRect*> m_Rects;
    MovingRect *m_Rackets[2];
    MovingRect *m_pBall;
    MovingRect *m_pLocalRacket;

    unsigned int m_iLastFrame;

    CVar<float> ball_speed;
    CVar<float> racket_speed;

public:
    GameEngine(ERole role, bool display);

    void initVideo();
    void setup();
    void mainLoop();
    void tick();
    virtual void shutdown() = 0;
    virtual void input(EKey key, bool pressed) = 0;

    void console_msg(const std::string &msg);
    void reportProgress(float progress, const std::string &details);

    Msg::Data version() const;

};

#endif
