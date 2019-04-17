#include "stdafx.h"
#include "Timer.h"
#include "../LogUtil.h"

extern "C"
{
#include <libavcodec/avcodec.h>
}

//////////////////////////////////////////////////////////////////////////
// Frame Timer
CFrameTimer::CFrameTimer()
{
    //Initialize the variables
    m_nStartTicks = 0;
    m_nPausedTicks = 0;
    m_bPaused = false;
    m_bStarted = false;
}

void CFrameTimer::Start()
{
    //Start the timer
    m_bStarted = TRUE;

    //Unpause the timer
    m_bPaused = FALSE;

    //Get the current clock time
    m_nStartTicks = SDL_GetTicks();
}

void CFrameTimer::Stop()
{
    //Stop the timer
    m_bStarted = FALSE;

    //Unpause the timer
    m_bPaused = FALSE;
}

void CFrameTimer::Pause()
{
    //If the timer is running and isn't already paused
    if ((m_bStarted == TRUE) && (m_bPaused == FALSE))
    {
        //Pause the timer
        m_bPaused = TRUE;

        //Calculate the paused ticks
        m_nPausedTicks = SDL_GetTicks() - m_nStartTicks;
    }
}

void CFrameTimer::Resume()
{
    //If the timer is paused
    if (m_bPaused == TRUE)
    {
        //Unpause the timer
        m_bPaused = FALSE;

        //Reset the starting ticks
        m_nStartTicks = SDL_GetTicks() - m_nPausedTicks;

        //Reset the paused ticks
        m_nPausedTicks = 0;
    }
}

UINT CFrameTimer::GetTicks()
{
    //If the timer is running
    if (m_bStarted == TRUE)
    {
        //If the timer is paused
        if (m_bPaused == TRUE)
        {
            //Return the number of ticks when the timer was paused
            return m_nPausedTicks;
        }
        else
        {
            //Return the current time minus the start time
            return SDL_GetTicks() - m_nStartTicks;
        }
    }

    //If the timer isn't running
    return 0;
}

BOOL CFrameTimer::IsStarted()
{
    return m_bStarted;
}

BOOL CFrameTimer::IsPaused()
{
    return m_bPaused;
}

void CFrameTimer::SyncFPS(INT nFPS)
{
    int nTick = GetTicks();
    if (nTick < (1000 / nFPS))
    {
        SDL_Delay(1000 / nFPS - nTick);
    }
}

//////////////////////////////////////////////////////////////////////////
// Calculate FPS
CFPS::CFPS() :m_nFrames(0)
{

}

VOID CFPS::Start()
{
    m_timer.Start();
}

VOID CFPS::IncFrame()
{
    m_nFrames++;
}

float CFPS::GetCurrentFPS()
{
    if (m_timer.GetTicks() < 1000)
        return -1;

    float fFPS = m_nFrames / (m_timer.GetTicks() / 1000.0f);

    m_timer.Start();
    m_nFrames = 0;

    return fFPS;
}
