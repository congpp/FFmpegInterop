
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

enum DecoderStatus
{
    DS_OK,
    DS_ERROR,
};

enum DecoderError
{
    DE_OK,
    DE_ERROR_COMMON,
    DE_ERROR_DECODE_FILE,
    DE_ERROR_NO_MORE_FRAME,
    DE_ERROR_NO_MORE_BUFFER,
};

typedef struct _AutoPacket
{
    _AutoPacket();
    ~_AutoPacket();

    bool IsOk();
    
    AVPacket* pPkt;

    static std::shared_ptr<_AutoPacket> CreateNew();
}AutoPacket;

typedef std::shared_ptr<_AutoPacket> AutoPacketPtr;

typedef std::list<AutoPacketPtr> AutoPacketList;

typedef struct _AutoFrame
{
    _AutoFrame();
    ~_AutoFrame();

    bool IsOk();


    static std::shared_ptr<_AutoFrame> CreateNew();

    bool InitBuffer(AVPixelFormat pixFmt, int width, int height, int align);

    bool Scale(int width, int height);

    AVFrame* pFrame;
    BYTE* pBuffer;
    bool bOk;
    AVPixelFormat pixFmt;
}AutoFrame;

typedef std::shared_ptr<_AutoFrame> AutoFramePtr;

typedef std::list<AutoFramePtr> AutoFrameList;
