#include "GameEngine.h"

#include <iostream>

GameEngine::GameEngine(ERole role, bool display)
  : m_eRole(role), m_bDisplay(display), m_pLocalRacket(NULL),
  // CVars
  ball_speed(this, "ball_speed", "Speed of the ball", 0.08f),
  racket_speed(this, "racket_speed", "Speed of the racket", 0.3f)
{
    m_Rackets[0] = NULL;
    m_Rackets[1] = NULL;
    m_pBall = NULL;
}

void GameEngine::initVideo()
{
    SDL_Init(SDL_INIT_VIDEO);
    m_pScreen = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
}

void GameEngine::setup()
{
    m_Rackets[0] = new MovingRect(this);
    m_Rackets[0]->x = 20.f; m_Rackets[0]->y = 20.f;
    m_Rackets[0]->w = 20; m_Rackets[0]->h = 100;
    m_Rackets[0]->vx = 0.f; m_Rackets[0]->vy = 0.f;
    m_Rackets[0]->r = 127; m_Rackets[0]->g = 127; m_Rackets[0]->b = 127;

    if(m_bDisplay)
        m_pLocalRacket = m_Rackets[0];

    m_Rackets[1] = new MovingRect(this);
    m_Rackets[1]->x = 600.f; m_Rackets[1]->y = 20.f;
    m_Rackets[1]->w = 20; m_Rackets[1]->h = 100;
    m_Rackets[1]->vx = 0.f; m_Rackets[1]->vy = 0.f;
    m_Rackets[1]->r = 127; m_Rackets[1]->g = 127; m_Rackets[1]->b = 127;

    m_pBall = new MovingRect(this);
    m_pBall->x = 50.f; m_pBall->y = 100.f;
    m_pBall->w = 20.f; m_pBall->h = 20.f;
    m_pBall->vx = *ball_speed; m_pBall->vy = *ball_speed;
    m_pBall->r = 255; m_pBall->g = 255; m_pBall->b = 255;
}

void GameEngine::mainLoop()
{
    for(;;)
    {
        // Physics stuff
        {
            unsigned int now = SDL_GetTicks();

            // Fixed timesteps
            static const unsigned int step_ms = (unsigned int)(TIMESTEP*1000);
            while(now >= m_iLastFrame + step_ms)
            {
                m_iLastFrame += step_ms;
                tick();
            }
        }

        // Keyboard input
        {
            SDL_Event event;
            while(SDL_PollEvent(&event))
            {
                if(event.type == SDL_QUIT)
                {
                    shutdown();
                    return ;
                }
                else if(m_pLocalRacket == NULL)
                    ; // On ne contrôle rien...
                else if(event.type == SDL_KEYDOWN
                 || event.type == SDL_KEYUP)
                {
                    bool pressed = event.type == SDL_KEYDOWN;
                    switch(event.key.keysym.sym)
                    {
                    case SDLK_UP:
                        input(KEY_UP, pressed);
                        break;
                    case SDLK_DOWN:
                        input(KEY_DOWN, pressed);
                        break;
                    default:
                        break;
                    }
                }
            }
        }

        // Graphical display
        if(m_bDisplay)
        {
            SDL_FillRect(m_pScreen, NULL,
                    SDL_MapRGB(m_pScreen->format, 0, 0, 0));
            std::set<MovingRect*>::iterator rect = m_Rects.begin();
            for(; rect != m_Rects.end(); ++rect)
            {
                SDL_Rect r = {(int)(*rect)->x, (int)(*rect)->y,
                        (*rect)->w, (*rect)->h};
                SDL_FillRect(m_pScreen, &r, SDL_MapRGB(m_pScreen->format,
                        (*rect)->r, (*rect)->g, (*rect)->b));
            }
            SDL_Flip(m_pScreen);
        }
    }
}

void GameEngine::tick()
{
    // New step!
    NetworkEngine::tick();

    // Move everything
    std::set<MovingRect*>::iterator it = m_Rects.begin();
    for(; it != m_Rects.end(); ++it)
        (*it)->tick(1);

    // Serious business (server-side stuff)
    if(m_eRole == ROLE_AUTHORITY)
    {
        // Collision ball/left racket
        if( (m_pBall->x < 40.f) && (m_pBall->y + m_pBall->h > m_Rackets[0]->y)
         && (m_pBall->y < m_Rackets[0]->y + m_Rackets[0]->h)
         && m_pBall->vx < 0.f)
        {
            m_pBall->vx = *ball_speed;
            m_pBall->modified();
        }
        // Collision ball/right racket
        else if( (m_pBall->x + m_pBall->w > 600.f)
         && (m_pBall->y + m_pBall->h > m_Rackets[1]->y)
         && (m_pBall->y < m_Rackets[1]->y + m_Rackets[1]->h)
         && m_pBall->vx > 0.f)
            {
            m_pBall->vx = -*ball_speed;
            m_pBall->modified();
        }

        // Collision ball/screen borders
        if(m_pBall->y <= 0.f && m_pBall->vy < 0.f)
        {
            m_pBall->vy = *ball_speed;
            m_pBall->modified();
        }
        else if(m_pBall->y + m_pBall->h >= 480.f && m_pBall->vy > 0.f)
        {
            m_pBall->vy = -*ball_speed;
            m_pBall->modified();
        }

        // Collision racket/screen borders
        int i;
        for(i = 0; i < 2; ++i)
        {
            if(m_Rackets[i]->y < 0.f)
            {
                m_Rackets[i]->y = 0.f;
                m_Rackets[i]->vy = 0.f;
                m_Rackets[i]->modified();
            }
            else if(m_Rackets[i]->y + m_Rackets[i]->h >= 480.f)
            {
                m_Rackets[i]->y = 480.f - m_Rackets[i]->h - 1.f;
                m_Rackets[i]->vy = 0.f;
                m_Rackets[i]->modified();
            }
        }

        // FIXME : Scoring
        if(m_pBall->x < -100.f && m_pBall->vx < 0.f)
        {
            m_pBall->vx = *ball_speed;
            m_pBall->modified();
        }
        else if(m_pBall->x >= 740.f && m_pBall->vx > 0.f)
        {
            m_pBall->vx = -*ball_speed;
            m_pBall->modified();
        }
    }
}

void GameEngine::console_msg(const std::string &msg)
{
    // TODO : in-game console
    std::cerr << "!!! " << msg << "\n";
}

void GameEngine::reportProgress(float progress, const std::string &details)
{
    // TODO : progress screen
    std::cerr << (int)(100.f*progress) << "% " << details << "\n";
}

Msg::Data GameEngine::version() const
{
    return Msg::Data((unsigned char*)PONG_NET_NAME)
            + writeInt2(PONG_VERSION_MAJOR)
            + writeInt2(PONG_VERSION_MINOR);
}
