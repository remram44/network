#include "GameEngine.h"

GameEngine::GameEngine(ERole role, bool display)
  : m_eRole(role), m_bDisplay(display),
  // CVars
  ball_speed("ball_speed", "Speed of the ball", 0.08f),
  racket_speed("racket_speed", "Speed of the racket", 0.3f)
{
    m_Rackets[0] = NULL;
    m_Rackets[1] = NULL;
    m_pBall = NULL;
}

void GameEngine::setup()
{
    m_Rackets[0] = new Racket(this);
    m_Rackets[0]->x = 20.f; m_Rackets[0]->y = 20.f;
    m_Rackets[0]->w = 20; m_Rackets[0]->h = 100;
    m_Rackets[0]->vx = 0.f; m_Rackets[0]->vy = 0.f;
    m_Rackets[0]->r = 127; m_Rackets[0]->g = 127; m_Rackets[0]->b = 127;

    m_Rackets[1] = new Racket(this);
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

        // TODO : Input

        // TODO : Display
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

Msg::Data GameEngine::version() const
{
    return Msg::Data(PONG_NET_NAME)
            + writeInt2(PONG_VERSION_MAJOR)
            + writeInt2(PONG_VERSION_MINOR);
}