// FFmpegVideoPlayer.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "FFmpegVideoPlayer.h"
#include "Decoder/FFmpegDecoder.h"
#include "Window/SDLWindow.h"

// 全局变量: 
HINSTANCE hInst;								// 当前实例

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPTSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    CSDLWindow* pWindow = new CSDLWindow;
    pWindow->Create("SimplePlayer", 0, 40, 680, 480, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);

    CFFmpegDecoderPtr ptrDecoder;
    ptrDecoder.reset(new CFFmpegDecoder);
    //if (!ptrDecoder->Open("D:\\UpopooRes\\2000139884\\合ddd成sdf1.mp4"))
    if (!ptrDecoder->Open("bird.mov"))
    //if (!ptrDecoder->Open("D:\\UpopooRes\\1800023890\\Destiny22017.11.01-02.09.17.03.mp4"))
    {
        MessageBox(NULL, L"Error Open", L"", MB_OK);
        return 0;
    }

    pWindow->SetFPS(ptrDecoder->GetFPS());
    pWindow->SetVideoDecoder(ptrDecoder->GetVideoDecoder());
    pWindow->SetAudioDecoder(ptrDecoder->GetAudioDecoder());
    pWindow->MessageLoop();
    return 0;
}