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

    //如果FPS太快，则延时等待再返回，太慢的话就没办法了，换电脑咯
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
// 计算FPS
class CFPS
{
public:
    CFPS();
   
    //开始计时
    VOID Start();

    //添加一个帧
    VOID IncFrame();
    
    //获取当前FPS，1S一次，返回负值说明还没有到计算的时刻
    float GetCurrentFPS();
private:
    INT m_nFrames;
    CFrameTimer m_timer;
};