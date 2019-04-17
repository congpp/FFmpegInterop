
#include "StdAfx.h"
#include "Define.h"

//////////////////////////////////////////////////////////////////////////
//
_AutoPacket::_AutoPacket()
{
    pPkt = av_packet_alloc();
}

_AutoPacket::~_AutoPacket()
{
    if (pPkt) av_packet_free(&pPkt);
}

bool _AutoPacket::IsOk()
{
    return NULL != pPkt;
}

std::shared_ptr<_AutoPacket> _AutoPacket::CreateNew()
{
    std::shared_ptr<_AutoPacket> ptr;
    ptr.reset(new _AutoPacket);
    return ptr;
}


//////////////////////////////////////////////////////////////////////////
//
_AutoFrame::_AutoFrame()
    : pBuffer(NULL)
    , bOk(true)
    , pixFmt(AV_PIX_FMT_NONE)
{
    pFrame = av_frame_alloc();
    bOk = pFrame != NULL;
}

_AutoFrame::~_AutoFrame()
{
    if (pFrame) av_frame_free(&pFrame);
    if (pBuffer) av_free(pBuffer);
}

bool _AutoFrame::IsOk()
{
    return bOk;
}

std::shared_ptr<_AutoFrame> _AutoFrame::CreateNew()
{
    std::shared_ptr<_AutoFrame> ptr;
    ptr.reset(new _AutoFrame);
    return ptr;
}

bool _AutoFrame::InitBuffer(AVPixelFormat pixFmt, int width, int height, int align)
{
    pixFmt = pixFmt;
    pFrame = av_frame_alloc();
    int buf_size = av_image_get_buffer_size(pixFmt, width, height, align);
    // buffer将作为p_frm_yuv的视频数据缓冲区
    pBuffer = (BYTE *)av_malloc(buf_size);
    if (pBuffer == NULL)
    {
        bOk = false;
        return false;
    }

    // 使用给定参数设定p_frm_yuv->data和p_frm_yuv->linesize
    bOk = 0 == av_image_fill_arrays(pFrame->data, pFrame->linesize, pBuffer, pixFmt, width, height, 1);

    return bOk;
}

bool _AutoFrame::Scale(int width, int height)
{
    return true;
}
