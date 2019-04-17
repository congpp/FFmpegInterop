#include "stdafx.h"
#include "SDLWindow.h"
#include "../LogUtil.h"

extern "C"
{
#include <libavcodec/avcodec.h>
}

//////////////////////////////////////////////////////////////////////////
// SDL Window
CSDLWindow::CSDLWindow()
    : m_bOk(TRUE)
    , m_pWin(NULL)
    , m_nFPS(30)
    , m_nFPSDynamic(30)
    , m_lfPtsVideo(0)
    , m_lfPtsAudio(0)
    , m_nAudioDevId(0)
{
}

CSDLWindow::~CSDLWindow()
{
    if (m_pTexture)
        SDL_DestroyTexture(m_pTexture);
}

BOOL CSDLWindow::Create(LPCSTR strName, INT x, INT y, INT w, INT h, DWORD dwFlag)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
    {
        m_bOk = FALSE;
        return FALSE;
    }

    //Set up the screen
    m_pWin = SDL_CreateWindow(strName, x, y, w, h, dwFlag);

    if (!m_pWin)
    {
        return ReportError(L"Error Create SDL Window: %d \n");
    }


    //������Ⱦ��
    m_pRender = SDL_CreateRenderer(m_pWin, -1, 0);
    if (m_pRender == NULL)
    {
        return ReportError(L"Error Create SDL Renderer: %d \n");
    }

    //����SDL_Texture
    m_pTexture = SDL_CreateTexture(m_pRender,
        SDL_PIXELFORMAT_IYUV,
        SDL_TEXTUREACCESS_STREAMING,
        w, h);

    if (m_pTexture == NULL)
    {
        return ReportError(L"Error Create SDL Texture: %d \n");
    }

    return TRUE;
}

BOOL CSDLWindow::SetFPS(UINT nFPS)
{
    m_nFPS = nFPS;
    return TRUE;
}


BOOL CSDLWindow::MessageLoop()
{
    CFPS fps;
    fps.Start();

    //ͨ���ı�֡�ʵķ���������Ƶͬ������Ƶ���о�����ȥ������Ƶ����Ϊ�������Ŷ��������Ļ����о����ˮ��
    m_nFPSDynamic = m_nFPS;

    //While the user hasn't quit
    BOOL bLoop = TRUE;
    while (bLoop)
    {
        //Start the frame timer
        m_timerFPS.Start();

        //While there's events to handle
        SDL_Event event = {};
        while (SDL_PollEvent(&event))
        {
            //If the user has Xed out the window
            if (event.type == SDL_QUIT)
            {
                //Quit the program
                bLoop = FALSE;
            }
            else if (event.type == SDL_WINDOWEVENT)
            {
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    RecreateTexture();
                }
            }
        }

        fps.IncFrame();

        if (!RenderVideo())
            m_nFPSDynamic = m_nFPS;

        //��ӡ��Ϣ
        float fFps = fps.GetCurrentFPS();
        if (fFps > 0)
        {
            char szTitle[128] = {};
            sprintf_s(szTitle, 128, "FPS: %0.2f | VideoPTS: %0.2f AudioPTS: %0.2f DifPTS: %0.2f", fFps, m_lfPtsVideo, m_lfPtsAudio, m_lfPtsVideo-m_lfPtsAudio);
            SDL_SetWindowTitle(m_pWin, szTitle);

        }

        //����Ƶͬ��
        if (m_lfPtsVideo > 0.01 && m_lfPtsAudio > 0.01)
        {
            //����ƵPTS������ƵPTS���򽵵�֡�ʣ�����Ƶ������
            if (m_lfPtsVideo - m_lfPtsAudio > 0.1)
            {
                m_nFPSDynamic -= 2;
                m_nFPSDynamic = max(m_nFPS / 2, m_nFPSDynamic);
                //nDelay = 0
                //SDL_Delay((lfPtsV - lfPtsA) * 1000);
            }
            //����ƵPTSС����ƵPTS�������֡�ʣ�������Ƶ
            else if (m_lfPtsVideo - m_lfPtsAudio < -0.1)
            {
                m_nFPSDynamic += 2;
                m_nFPSDynamic = min(m_nFPS + m_nFPS / 2, m_nFPSDynamic);
                //nDelay = ((lfPtsA - lfPtsV) * 1000);
            }
            else
            {
                m_nFPSDynamic = m_nFPS;
            }
        }

        //֡�ʿ���
        m_timerFPS.SyncFPS(m_nFPSDynamic);
    }
    return 0;
}

BOOL CSDLWindow::RenderVideo()
{
    if (!m_ptrVideoDecoder)
        return FALSE;

    //texture �Ĵ�С���� window �Ĵ�С
    SDL_Rect rcTexture = {};
    SDL_Rect rcRender = {};
    if (m_pTexture)
        SDL_QueryTexture(m_pTexture, NULL, NULL, &rcTexture.w, &rcTexture.h);
    //SDL_GetWindowSize(m_pWin, &rcWindow.w, &rcWindow.h);
    rcRender = rcTexture;

    //�ȱ�������Ƶ����
    SDL_Rect rcFrame = {};
    if (m_ptrVideoDecoder->GetFrameSize(&rcFrame.w, &rcFrame.h) && !SDL_RectEquals(&rcTexture, &rcFrame))
    {
        //����, ���м���
        if (rcTexture.w > rcFrame.w && rcTexture.h > rcFrame.h)
        {
            rcTexture.x = (rcTexture.w - rcFrame.w) / 2;
            rcTexture.y = (rcTexture.h - rcFrame.h) / 2;
        }
        else
        {
            //���߶���С
            float fH = (float)rcFrame.h / rcTexture.h;
            rcFrame.h = rcTexture.h;
            rcFrame.w = rcFrame.w / fH;

            //̫���ˣ�����С���
            if (rcFrame.w > rcTexture.w)
            {
                float fW = (float)rcFrame.w / rcTexture.w;
                rcFrame.w = rcTexture.w;
                rcFrame.h = rcFrame.h / fW;
            }

            rcTexture.x = (rcTexture.w - rcFrame.w) / 2;
            rcTexture.y = (rcTexture.h - rcFrame.h) / 2;
            rcTexture.h = rcFrame.h;
            rcTexture.w = rcFrame.w;
        }
    }

    AutoFramePtr pFrameYUV;
    DecoderError nErr = m_ptrVideoDecoder->PopFrame(pFrameYUV, rcFrame.w, rcFrame.h, &m_lfPtsVideo);
    if (nErr == DE_OK && pFrameYUV && m_pTexture)
    {

        //����
        SDL_UpdateYUVTexture(m_pTexture,   // sdl texture
            &rcTexture,                    // sdl rect
            pFrameYUV->pFrame->data[0],            // y plane
            pFrameYUV->pFrame->linesize[0],        // y pitch
            pFrameYUV->pFrame->data[1],            // u plane
            pFrameYUV->pFrame->linesize[1],        // u pitch
            pFrameYUV->pFrame->data[2],            // v plane
            pFrameYUV->pFrame->linesize[2]         // v pitch
            );

        //���Render
        SDL_RenderClear(m_pRender);
        //����Texture
        SDL_RenderCopy(m_pRender,                // sdl renderer
            m_pTexture,                          // sdl texture
            &rcTexture,                                // src rect, if NULL copy texture
            &rcTexture                            // dst rect
            );

        //��Ч
        SDL_RenderPresent(m_pRender);
        return TRUE;
    }
    else if (nErr == DE_ERROR_NO_MORE_FRAME)
    {
        //�������

        //���Render
        SDL_RenderClear(m_pRender);
        //��Ч
        SDL_RenderPresent(m_pRender);

        return FALSE;
    }

    return FALSE;
}

BOOL CSDLWindow::RecreateTexture()
{
    SDL_Rect rcWindow = {};
    //SDL_GetWindowPosition(m_pWin, &rcWindow.x, &rcWindow.y);
    SDL_GetWindowSize(m_pWin, &rcWindow.w, &rcWindow.h);
    
    if (m_pTexture)
        SDL_DestroyTexture(m_pTexture);

    m_pTexture = NULL;

    //����SDL_Texture
    m_pTexture = SDL_CreateTexture(m_pRender,
        SDL_PIXELFORMAT_IYUV,
        SDL_TEXTUREACCESS_STREAMING,
        rcWindow.w, rcWindow.h);

    if (m_pTexture == NULL)
    {
        return ReportError(L"Error Create SDL Texture: %d \n");
    }

    return TRUE;
}

VOID CSDLWindow::SetVideoDecoder(CVideoDecoderPtr pVideoDecoder)
{
    m_ptrVideoDecoder = pVideoDecoder;
}

VOID CSDLWindow::SetAudioDecoder(CAudioDecoderPtr ptrAudioDecoder)
{
    m_ptrAudioDecoder = ptrAudioDecoder;
    if (!ptrAudioDecoder)
        return;

    SDL_AudioSpec wantedSpec = {};
    SDL_AudioSpec actualSpec = {};
    wantedSpec.freq = ptrAudioDecoder->GetSampleRate();
    wantedSpec.format = AUDIO_S16SYS;
    wantedSpec.channels = ptrAudioDecoder->GetChannels();
    wantedSpec.silence = 0;
    wantedSpec.samples = ptrAudioDecoder->GetFrameSize();
    wantedSpec.callback = SDLAudioCallback;
    wantedSpec.userdata = this;
    
    int nCount = SDL_GetNumAudioDevices(0);
    for (int i = 0; i < nCount; i++)
    {
        CLogUtil::Log("Audio device %d => %s\n", i, SDL_GetAudioDeviceName(i, 0));
    }

    if (nCount <= 0) return;

    m_nAudioDevId = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(0, 0), false, &wantedSpec, &actualSpec, 0);

    SDL_PauseAudioDevice(m_nAudioDevId, 0);
}

BOOL CSDLWindow::ReportError(LPCWSTR strMsgFormat)
{
#ifdef _DEBUG
    if (strMsgFormat)
    {
        WCHAR szMsg[1024] = {};
        swprintf_s(szMsg, _countof(szMsg), strMsgFormat, SDL_GetError());
        OutputDebugString(szMsg);
    }
#endif // _DEBUG

    m_bOk = FALSE;
    return FALSE;
}

void CSDLWindow::SDLAudioCallback(LPVOID userdata, BYTE* pStream, int len)
{
    CSDLWindow* pThis = (CSDLWindow*)userdata;
    CAudioDecoderPtr ptrAudioDecoder = pThis->m_ptrAudioDecoder;

    //if (pStream)
    //    ZeroMemory(pStream, len);

    //if (nDelay)
    //    SDL_Delay(nDelay);

    if (ptrAudioDecoder)
        ptrAudioDecoder->PopFrame(pStream, len, &pThis->m_lfPtsAudio);
}
