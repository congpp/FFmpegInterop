
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
#include "SDL_audio.h"
}

#include "Define.h"

class CAudioDecoder
{
public:
    CAudioDecoder();
    ~CAudioDecoder();

    void SetMaxFrameBufferCount(INT nCount){ m_nMaxCount = nCount; }
    //缓存nMiliSec毫秒的音频，该音频采样率为nSampleRate
    void SetMaxFrameBufferCount(INT nMiliSec, INT nSampleRate){ m_nMaxCount = nSampleRate * nMiliSec / 1000; }

    DecoderError Init(AVFormatContext* pFmtCtx, INT steam_idx);

    INT GetSampleRate();
    INT GetChannels();
    INT GetFrameSize();
    AVRational GetTimeBase();

    DecoderError PushPacket(AutoPacketPtr& ptrPkt);
    DecoderError PopFrame(AutoFramePtr& ptrFrame);
    DecoderError PopFrame(BYTE* pStream, INT& len, double *pPts = NULL);
    //获得帧数据总大小
    INT GetFrameBufferSize(AutoFramePtr ptrFrame);

    DecoderError ReportError(DecoderError nErr, LPCWSTR strFormat = NULL);
    void NeedDelay(UINT nDelay);
private:
    AutoFramePtr m_ptrFrame;

    AVFormatContext* m_pFmtCtx;
    AVCodecContext*  m_pCodecCtx;
    SwrContext *m_pAudioConvertCtx;

    SDL_mutex* m_pMutex;
    DecoderError   m_nErr;
    DecoderStatus  m_nStatus;
    AutoFrameList m_listFrames;
    INT m_nMaxCount;
    INT m_nStreamIdx;
};

typedef std::shared_ptr<CAudioDecoder> CAudioDecoderPtr;
