#include "stdafx.h"
#include "SDLAutoMutex.h"
extern "C"
{
#include "SDL.h"
}

CSDLAutoMutex::CSDLAutoMutex(SDL_mutex* pMutex)
    : m_pMutex(pMutex)
{
    SDL_LockMutex(pMutex);
}


CSDLAutoMutex::~CSDLAutoMutex()
{
    if (m_pMutex)
        SDL_UnlockMutex(m_pMutex);
}
