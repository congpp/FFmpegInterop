
#include "stdafx.h"
#include "AudioDecoder.h"
#include "../SDLAutoMutex.h"
#include "../LogUtil.h"

CAudioDecoder::CAudioDecoder()
    : m_pFmtCtx(NULL)
    , m_pCodecCtx(NULL)
    , m_nErr(DE_OK)
    , m_nStatus(DS_OK)
    , m_nMaxCount(1024)
    , m_ptrFrame(NULL)
{
    m_pMutex = SDL_CreateMutex();
}

CAudioDecoder::~CAudioDecoder()
{
    if (m_pMutex)
        SDL_DestroyMutex(m_pMutex);
    
}

DecoderError CAudioDecoder::Init(AVFormatContext* pFmtCtx, INT nSteamIdx)
{
    AVCodecParameters* pCodecPar = NULL;
    AVCodec* pCodec = NULL;
    int ret = 0;

    m_pFmtCtx = pFmtCtx;
    m_nStreamIdx = nSteamIdx;
    // 1. 为音频流构建解码器AVCodecContext

    // 1.1 获取解码器参数AVCodecParameters
    pCodecPar = pFmtCtx->streams[nSteamIdx]->codecpar;
    // 1.2 获取解码器
    pCodec = avcodec_find_decoder(pCodecPar->codec_id);
    if (pCodec == NULL)
    {
        return ReportError(DE_ERROR_COMMON, L"avcodec_find_decoder() failed %d\n");
    }

    // 1.3 构建解码器AVCodecContext
    // 1.3.1 p_codec_ctx初始化：分配结构体，使用p_codec初始化相应成员为默认值
    m_pCodecCtx = avcodec_alloc_context3(pCodec);
    if (m_pCodecCtx == NULL)
    {
        return ReportError(DE_ERROR_COMMON, L"avcodec_alloc_context3() failed %d\n");
    }
    // 1.3.2 p_codec_ctx初始化：p_codec_par ==> p_codec_ctx，初始化相应成员
    ret = avcodec_parameters_to_context(m_pCodecCtx, pCodecPar);
    if (ret < 0)
    {
        return ReportError(DE_ERROR_COMMON, L"avcodec_parameters_to_context() failed %d\n");
    }
    // 1.3.3 p_codec_ctx初始化：使用p_codec初始化p_codec_ctx，初始化完成
    ret = avcodec_open2(m_pCodecCtx, pCodec, NULL);
    if (ret < 0)
    {
        return ReportError(DE_ERROR_COMMON, L"avcodec_open2() failed %d\n");
    }

    //m_pAudioConvertCtx = swr_alloc();
    m_pAudioConvertCtx = swr_alloc_set_opts(NULL,
        m_pCodecCtx->channel_layout, AV_SAMPLE_FMT_S16, m_pCodecCtx->sample_rate,
        m_pCodecCtx->channel_layout, m_pCodecCtx->sample_fmt, m_pCodecCtx->sample_rate,
        0, NULL);
    swr_init(m_pAudioConvertCtx);

    return DE_OK;
}

INT CAudioDecoder::GetSampleRate()
{
    return m_pCodecCtx ? m_pCodecCtx->sample_rate : 0;
}

INT CAudioDecoder::GetChannels()
{
    return m_pCodecCtx ? m_pCodecCtx->channels : 0;
}

INT CAudioDecoder::GetFrameSize()
{
    return m_pCodecCtx ? m_pCodecCtx->frame_size : 0;
}

AVRational CAudioDecoder::GetTimeBase()
{
    AVRational tb = {};
    if (!m_pFmtCtx)
        return tb;
    return m_pFmtCtx->streams[m_nStreamIdx]->time_base;
}

DecoderError CAudioDecoder::PushPacket(AutoPacketPtr& ptrPkt)
{
    CSDLAutoMutex autoMutex(m_pMutex);
    //
    if ((int)m_listFrames.size() >= m_nMaxCount)
    {
        return DE_ERROR_NO_MORE_BUFFER;
    }

    //m_listPkts.push_back(ptrPkt);
    int ret = avcodec_send_packet(m_pCodecCtx, ptrPkt->pPkt);
    if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
        return ReportError(DE_ERROR_DECODE_FILE, L"Error audio avcodec_send_packet %d\n");

    while (true)
    {
        AutoFramePtr ptrFrame = AutoFrame::CreateNew();
        ret = avcodec_receive_frame(m_pCodecCtx, ptrFrame->pFrame);
        if (ret < 0 && ret != AVERROR_EOF)
            break;

        m_listFrames.push_back(ptrFrame);
    }
    CLogUtil::Log(L"Audio Buffer Size=%d\n", m_listFrames.size());
    return DE_OK;
}

DecoderError CAudioDecoder::PopFrame(AutoFramePtr& ptrFrame)
{
    CSDLAutoMutex autoMutex(m_pMutex);
    if (!m_listFrames.empty())
    {
        ptrFrame = m_listFrames.front();
        m_listFrames.pop_front();
        return DE_OK;
    }
    
    return m_nErr;
    
}

DecoderError CAudioDecoder::PopFrame(BYTE* pStream, INT& len, double *pPts/* = NULL*/)
{
    if (pStream)
        ZeroMemory(pStream, len);

    int nLeft = len;
    while (nLeft > 0)
    {
        AutoFramePtr ptrFrame;
        DecoderError nErr = PopFrame(ptrFrame);
        if (!ptrFrame)
            return nErr;

        AVCodecContext* pCodecCtx = m_pCodecCtx;

        //INT64 audioClock = ptrFrame->pFrame->pts * av_q2d(GetTimeBase());

        // 音频格式转换
        int nRes = swr_get_out_samples(m_pAudioConvertCtx, 1024);
        int ret = swr_convert(m_pAudioConvertCtx,
            &pStream,
            ptrFrame->pFrame->nb_samples,
            (uint8_t const **)(ptrFrame->pFrame->data),
            ptrFrame->pFrame->nb_samples);
        //nLeft -= GetFrameBufferSize(ptrFrame);
        //int nSize = GetFrameSize(ptrFrame);

        if (pPts)
            *pPts = ptrFrame->pFrame->pts * av_q2d(GetTimeBase());

        break;
    }
    
    CLogUtil::Log(L"SDLAudioCallback len=%d, act=%d\n", len, len-nLeft);

    return DE_OK;
}

INT CAudioDecoder::GetFrameBufferSize(AutoFramePtr ptrFrame)
{
    if (ptrFrame && ptrFrame->pFrame)
    {
        int nSize = 0;
        if (m_pCodecCtx->sample_fmt == AV_SAMPLE_FMT_S16P)
            nSize = av_samples_get_buffer_size(ptrFrame->pFrame->linesize, m_pCodecCtx->channels, m_pCodecCtx->frame_size, m_pCodecCtx->sample_fmt, 1);
        else
            av_samples_get_buffer_size(&nSize, m_pCodecCtx->channels, m_pCodecCtx->frame_size, m_pCodecCtx->sample_fmt, 1);
        return nSize;
    }

    return 0;
}

DecoderError CAudioDecoder::ReportError(DecoderError nErr, LPCWSTR strFormat /*= NULL*/)
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
