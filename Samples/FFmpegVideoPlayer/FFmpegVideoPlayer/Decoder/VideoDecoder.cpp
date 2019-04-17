
#include "stdafx.h"
#include "VideoDecoder.h"
#include "../SDLAutoMutex.h"
#include "../LogUtil.h"

CVideoDecoder::CVideoDecoder()
    : m_pFmtCtx(NULL)
    , m_pCodecCtx(NULL)
    , m_nErr(DE_OK)
    , m_nStatus(DS_OK)
    , m_nMaxCount(6)
    , m_ptrFrame(NULL)
{
    m_pMutex = SDL_CreateMutex();
}

CVideoDecoder::~CVideoDecoder()
{
    if (m_pMutex)
        SDL_DestroyMutex(m_pMutex);
    
}

BOOL CVideoDecoder::Init(AVFormatContext* pFmtCtx, INT nSteamIdx)
{
    if (!pFmtCtx) return FALSE;

    m_pFmtCtx = pFmtCtx;
    m_nVideoIdx = nSteamIdx;

    AVCodecParameters* pCodecPar = NULL;
    AVCodec* pCodec = NULL;
    int ret = 0;
    
    // 1. 为视频流构建解码器AVCodecContext
    // 1.1 获取解码器参数AVCodecParameters
    pCodecPar = m_pFmtCtx->streams[nSteamIdx]->codecpar;

    // 1.2 获取解码器
    pCodec = avcodec_find_decoder(pCodecPar->codec_id);
    if (pCodec == NULL)
    {
        ReportError(DE_ERROR_COMMON, L"Cann't find codec! %d\n");
        return FALSE;
    }

    // 1.3 构建解码器AVCodecContext
    // 1.3.1 p_codec_ctx初始化：分配结构体，使用p_codec初始化相应成员为默认值
    m_pCodecCtx = avcodec_alloc_context3(pCodec);
    if (m_pCodecCtx == NULL)
    {
        ReportError(DE_ERROR_COMMON, L"avcodec_alloc_context3() failed %d\n");
        return FALSE;
    }
    // 1.3.2 p_codec_ctx初始化：p_codec_par ==> p_codec_ctx，初始化相应成员
    ret = avcodec_parameters_to_context(m_pCodecCtx, pCodecPar);
    if (ret < 0)
    {
        ReportError(DE_ERROR_COMMON, L"avcodec_parameters_to_context() failed %d\n");
        return FALSE;
    }
    // 1.3.3 p_codec_ctx初始化：使用p_codec初始化p_codec_ctx，初始化完成
    ret = avcodec_open2(m_pCodecCtx, pCodec, NULL);
    if (ret < 0)
    {
        ReportError(DE_ERROR_COMMON, L"avcodec_open2() failed %d\n");
        return FALSE;
    }

    m_ptrFrame = AutoFrame::CreateNew();
    m_ptrFrame->InitBuffer(AV_PIX_FMT_YUV420P, m_pCodecCtx->width, m_pCodecCtx->height, 1);
    return TRUE;
}


BOOL CVideoDecoder::GetFrameSize(INT* w, INT* h)
{
    if (!m_pCodecCtx)
        return FALSE;

    if (w) *w = m_pCodecCtx->width;
    if (h) *h = m_pCodecCtx->height;
    return TRUE;
}

AVRational CVideoDecoder::GetTimeBase()
{
    AVRational tb = {};
    if (!m_pFmtCtx)
        return tb;
    return m_pFmtCtx->streams[m_nVideoIdx]->time_base;
}

DecoderError CVideoDecoder::PushPacket(AutoPacketPtr& ptrPkt)
{
    CSDLAutoMutex autoMutex(m_pMutex);
    //
    if ((int)m_listFrames.size() >= m_nMaxCount)
    {
        return DE_ERROR_NO_MORE_BUFFER;
    }

    CLogUtil::Log(L"Video Buffer Size=%d\n", m_listFrames.size());

    //av_gettime_relative();
    int ret = avcodec_send_packet(m_pCodecCtx, ptrPkt->pPkt);
    if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
    {
        CLogUtil::LogAVError(ret);
        return ReportError(DE_ERROR_COMMON, L"Error avcodec_send_packet %d\n");
    }

    AutoFramePtr ptrTempFrame = AutoFrame::CreateNew();
    if (!ptrTempFrame->IsOk())
        return ReportError(DE_ERROR_DECODE_FILE, L"av_frame_alloc() for p_frm_yuv failed %d\n");

    ret = avcodec_receive_frame(m_pCodecCtx, ptrTempFrame->pFrame);
    if (ret < 0 && ret != AVERROR_EOF)
        return ReportError(DE_ERROR_COMMON, L"Error avcodec_receive_frame %d\n");

    m_listFrames.push_back(ptrTempFrame);
    return DE_OK;
}

DecoderError CVideoDecoder::PopFrame(AutoFramePtr& ptrPkt)
{
    CSDLAutoMutex autoMutex(m_pMutex);
    if (!m_listFrames.empty())
    {
        ptrPkt = m_listFrames.front();
        m_listFrames.pop_front();
        return DE_OK;
    }
    
    return m_nErr;
    
}

DecoderError CVideoDecoder::PopFrame(AutoFramePtr& ptrFrame, INT w, INT h, double *pPts)
{
    AutoFramePtr ptrTempFrame;
    DecoderError nErr = PopFrame(ptrTempFrame);
    if (!ptrTempFrame)
        return nErr;

    //CLogUtil::Log(L"PopFrame w=%d, h=%d\n", w, h);

    CSDLAutoMutex autoMutex(m_pMutex);

    //create frame
    ptrFrame = AutoFrame::CreateNew();
    ptrFrame->InitBuffer(AV_PIX_FMT_YUV420P, w, h, 1);

    if (pPts)
    {
        *pPts = ptrTempFrame->pFrame->pts * av_q2d(GetTimeBase());
        //double pts = ptrTempFrame->pFrame->pts;
        //double timeBase = av_q2d(GetTimeBase());
        //if ((pts = av_frame_get_best_effort_timestamp(ptrTempFrame->pFrame)) == AV_NOPTS_VALUE)
        //    pts = 0;
        //else
        //    pts = *pPts;

        //pts = pts * timeBase;

        //double frame_delay = timeBase;
        //frame_delay += ptrTempFrame->pFrame->repeat_pict * (frame_delay * 0.5);

        //*pPts = frame_delay + pts;
        //ptrTempFrame->pFrame->opaque = &pts;
    }


    //scale
    SwsContext * sws_ctx = sws_getContext(m_pCodecCtx->width,    // src width
        m_pCodecCtx->height,   // src height
        m_pCodecCtx->pix_fmt,  // src format
        w,    // dst width
        h,   // dst height
        AV_PIX_FMT_YUV420P,    // dst format
        SWS_BICUBIC,           // flags
        NULL,                  // src filter
        NULL,                  // dst filter
        NULL                   // param
        );
    
    if (sws_ctx == NULL)
    {
        return ReportError(DE_ERROR_COMMON, L"Error sws_getContext %d\n");
    }

    sws_scale(sws_ctx,                                  // sws context
        (const uint8_t *const *)ptrTempFrame->pFrame->data,  // src slice
        ptrTempFrame->pFrame->linesize,                      // src stride
        0,                                        // src slice y
        m_pCodecCtx->height,                      // src slice height
        ptrFrame->pFrame->data,               // dst planes
        ptrFrame->pFrame->linesize            // dst strides
        );

    sws_freeContext(sws_ctx);

    return nErr;
}

DecoderError CVideoDecoder::ReportError(DecoderError nErr, LPCWSTR strFormat /*= NULL*/)
{
    m_nErr = nErr;
    m_nStatus = DS_ERROR;
#ifdef _DEBUG
    if (strFormat)
    {
        WCHAR szMsg[1024] = {};
        swprintf_s(szMsg, _countof(szMsg), strFormat, nErr);
        OutputDebugString(szMsg);
    }
#endif // _DEBUG
    return nErr;
}

