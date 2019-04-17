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

    //���ڴ�С�����ı�Ҫ�ı������С��
    BOOL RecreateTexture();
    //Set m_bOk to FALSE, and return FALSE;
    BOOL ReportError(LPCWSTR strMsgFormat = NULL);

    //��Ƶ�ص�
    static void SDLAudioCallback(LPVOID userdata, BYTE* stream, int len);
private:
    BOOL m_bOk;
    
    CFrameTimer     m_timerFPS;
    UINT            m_nFPS;
    UINT            m_nFPSDynamic;

    double m_lfPtsVideo;
    double m_lfPtsAudio;
    
    SDL_Window*     m_pWin;     //���������
    SDL_Renderer*   m_pRender;  //�������Ⱦ��
    SDL_Texture*    m_pTexture; //ȥ��ʾ��һ֡ͼ��

    CVideoDecoderPtr  m_ptrVideoDecoder;
    CAudioDecoderPtr  m_ptrAudioDecoder;
    SDL_AudioDeviceID m_nAudioDevId;
};
