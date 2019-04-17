
#pragma once
extern "C"
{
#include <libavutil/time.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

#include "SDL_thread.h"
}

#include "Define.h"
#include "VideoDecoder.h"
#include "AudioDecoder.h"

#include <list>
#include <memory>
using namespace std;

//打开文件、读取packet并分发到video、audio线程解码
class CFFmpegDecoder
{
public:
    CFFmpegDecoder();
    ~CFFmpegDecoder();
    BOOL Open(LPCSTR strFile);

    CVideoDecoderPtr GetVideoDecoder(){ return m_ptrVideoDecoder; }

    INT GetFPS();

    static INT DecodeThread(LPVOID pParam);

    DecoderError ReportError(DecoderError nErr, LPCWSTR strFormat = NULL);
    CAudioDecoderPtr GetAudioDecoder() const;
private:
    int m_nAudioIndex;
    int m_nVideoIdx;
    CVideoDecoderPtr m_ptrVideoDecoder;
    CAudioDecoderPtr m_ptrAudioDecoder;
    AVFormatContext *m_pFmtCtx;
    DecoderError   m_nErr;
    DecoderStatus  m_nStatus;
    SDL_Thread* m_pThread;
    BOOL m_bExit;
};


typedef std::shared_ptr<CFFmpegDecoder> CFFmpegDecoderPtr;
