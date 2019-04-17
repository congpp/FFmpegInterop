#pragma once

struct SDL_mutex;

class CSDLAutoMutex
{
public:
    CSDLAutoMutex(SDL_mutex* m_pMutex);
    ~CSDLAutoMutex();

private:
    SDL_mutex* m_pMutex;
};


