
#pragma once
#include <list>
#include <memory>
using namespace std;

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

class CVideoDecoder
{
public:
    CVideoDecoder();
    ~CVideoDecoder();

    void SetMaxFrameBufferCount(INT nCount){ m_nMaxCount = nCount; }
    //缓存nMiliSec毫秒的视频，该视频FPS为nFPS
    void SetMaxFrameBufferCount(INT nMiliSec, INT nFPS){ m_nMaxCount = nFPS * nMiliSec / 1000; }

    BOOL Init(AVFormatContext* pFmtCtx, INT nSteamIdx);

    BOOL GetFrameSize(INT* w, INT* h);
    AVRational GetTimeBase();

    DecoderError PushPacket(AutoPacketPtr& ptrPkt);
    DecoderError PopFrame(AutoFramePtr& pFrame);
    DecoderError PopFrame(AutoFramePtr& pFrame, INT w, INT h, double *pPts = NULL);

    DecoderError ReportError(DecoderError nErr, LPCWSTR strFormat = NULL);
private:
    AutoFramePtr m_ptrFrame;

    AVFormatContext* m_pFmtCtx;
    AVCodecContext*  m_pCodecCtx;
    
    SDL_mutex* m_pMutex;
    DecoderError   m_nErr;
    DecoderStatus  m_nStatus;
    AutoFrameList  m_listFrames;
    INT m_nMaxCount;
    INT m_nVideoIdx;
};

typedef std::shared_ptr<CVideoDecoder> CVideoDecoderPtr;
