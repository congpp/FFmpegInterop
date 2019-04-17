#pragma once
extern "C"
{
#include "SDL.h"

}

#include "../Decoder/VideoDecoder.h"
#include "../Decoder/AudioDecoder.h"

#include <memory>
#include <string>

//The timer
class CFrameTimer
{
public:
    //Initializes variables
    CFrameTimer();

    void Start();

    void Stop();

    void Pause();

    void Resume();

    UINT GetTicks();

    BOOL IsStarted();

    BOOL IsPaused();

    //���FPS̫�죬����ʱ�ȴ��ٷ��أ�̫���Ļ���û�취�ˣ������Կ�
    void SyncFPS(INT nFPS);
private:
    //The clock time when the timer started
    UINT m_nStartTicks;

    //The ticks stored when the timer was paused
    UINT m_nPausedTicks;

    //The timer status
    BOOL m_bPaused;
    BOOL m_bStarted;
};

//////////////////////////////////////////////////////////////////////////
// ����FPS
class CFPS
{
public:
    CFPS();
   
    //��ʼ��ʱ
    VOID Start();

    //���һ��֡
    VOID IncFrame();
    
    //��ȡ��ǰFPS��1Sһ�Σ����ظ�ֵ˵����û�е������ʱ��
    float GetCurrentFPS();
private:
    INT m_nFrames;
    CFrameTimer m_timer;
};