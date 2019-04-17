#pragma once
extern "C"
{
#include "SDL.h"

}

#include "Timer.h"
#include "../Decoder/VideoDecoder.h"
#include "../Decoder/AudioDecoder.h"

#include <memory>
#include <string>


class CSDLWindow
{
public:
    CSDLWindow();
    ~CSDLWindow();

    BOOL Create(LPCSTR strName, INT x, INT y, INT w, INT h, DWORD dwFlag = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    BOOL IsOk();

    BOOL SetFPS(UINT nFPS);
    BOOL SetFullScreen(BOOL bFullScreen);

    BOOL MessageLoop();


    VOID SetVideoDecoder(CVideoDecoderPtr ptrVideoDecoder);
    VOID SetAudioDecoder(CAudioDecoderPtr ptrAudioDecoder);
protected:
    BOOL RenderVideo();

    //窗口大小发生改变要改变这个大小额
    BOOL RecreateTexture();
    //Set m_bOk to FALSE, and return FALSE;
    BOOL ReportError(LPCWSTR strMsgFormat = NULL);

    //音频回调
    static void SDLAudioCallback(LPVOID userdata, BYTE* stream, int len);
private:
    BOOL m_bOk;
    
    CFrameTimer     m_timerFPS;
    UINT            m_nFPS;
    UINT            m_nFPSDynamic;

    double m_lfPtsVideo;
    double m_lfPtsAudio;
    
    SDL_Window*     m_pWin;     //用这个窗口
    SDL_Renderer*   m_pRender;  //的这个渲染器
    SDL_Texture*    m_pTexture; //去显示这一帧图像

    CVideoDecoderPtr  m_ptrVideoDecoder;
    CAudioDecoderPtr  m_ptrAudioDecoder;
    SDL_AudioDeviceID m_nAudioDevId;
};
