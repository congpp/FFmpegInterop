
#include "stdafx.h"
#include "FFmpegDecoder.h"
#include "../LogUtil.h"

CFFmpegDecoder::CFFmpegDecoder()
    :m_pFmtCtx(NULL)
    , m_nAudioIndex(-1)
    , m_nVideoIdx(-1)
    , m_pThread(NULL)
    , m_bExit(FALSE)
{

}

CFFmpegDecoder::~CFFmpegDecoder()
{
    ;
}

BOOL CFFmpegDecoder::Open(LPCSTR strFile)
{
    // 打开视频文件
    int ret = avformat_open_input(&m_pFmtCtx, strFile, NULL, NULL);
    if (ret != 0)
    {
        OutputDebugStringA("avformat_open_input failed\n");
        return FALSE;
    }

    // 搜索流信息
    ret = avformat_find_stream_info(m_pFmtCtx, NULL);
    if (ret < 0)
    {
        OutputDebugStringA("avformat_find_stream_info() failed %d\n");
        return FALSE;
    }
    
    // 查找第一个音频流/视频流
    for (int i = 0; i < m_pFmtCtx->nb_streams; i++)
    {
        if ((m_pFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) &&
            (m_nAudioIndex == -1))
        {
            m_nAudioIndex = i;
            // 打开音频流
            m_ptrAudioDecoder.reset(new CAudioDecoder);
            m_ptrAudioDecoder->Init(m_pFmtCtx, m_nAudioIndex);
            m_ptrAudioDecoder->SetMaxFrameBufferCount(5000, m_ptrAudioDecoder->GetSampleRate()/m_ptrAudioDecoder->GetFrameSize());
        }
        else if ((m_pFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) &&
            (m_nVideoIdx == -1))
        {
            m_nVideoIdx = i;

            // 打开视频流
            m_ptrVideoDecoder.reset(new CVideoDecoder);
            m_ptrVideoDecoder->Init(m_pFmtCtx, m_nVideoIdx);
            m_ptrVideoDecoder->SetMaxFrameBufferCount(200, GetFPS());
        }

        if (m_nAudioIndex != -1 && m_nVideoIdx != -1)
        {
            break;
        }
    }

    if (m_nAudioIndex == -1 && m_nVideoIdx == -1)
    {
        CLogUtil::Log("Cann't find any audio/video stream\n");
        return FALSE;
    }

    m_pThread = SDL_CreateThread(&CFFmpegDecoder::DecodeThread, "DecodeThread", this);

    return TRUE;
}

INT CFFmpegDecoder::GetFPS()
{
    if (!m_pFmtCtx || -1 == m_nVideoIdx)
        return 0;

    int temp_num = m_pFmtCtx->streams[m_nVideoIdx]->avg_frame_rate.num;
    int temp_den = m_pFmtCtx->streams[m_nVideoIdx]->avg_frame_rate.den;
    int frame_rate = (temp_den > 0) ? temp_num / temp_den : 25;
    //int interval = (temp_num > 0) ? (temp_den * 1000) / temp_num : 40;
    return frame_rate;
}

INT CFFmpegDecoder::DecodeThread(LPVOID pParam)
{
    CFFmpegDecoder* pThis = (CFFmpegDecoder*)pParam;
    if (!pThis || !pThis->m_pFmtCtx)
        return -1;


    while (pThis->m_bExit == FALSE)
    {
        //视频解码 发送一个收到一个
        AutoPacketPtr ptrPkt = AutoPacket::CreateNew();

        if (!ptrPkt->IsOk())
            return pThis->ReportError(DE_ERROR_DECODE_FILE, L"Error new Video Frame: %d\n");

        int ret = av_read_frame(pThis->m_pFmtCtx, ptrPkt->pPkt);
        if (ret != 0)
        {
            if (ret == AVERROR_EOF || avio_feof(pThis->m_pFmtCtx->pb))
            {
                pThis->m_ptrVideoDecoder->ReportError(DE_ERROR_NO_MORE_FRAME, L"OK %d\n");
                return pThis->ReportError(DE_ERROR_NO_MORE_FRAME, L"Finished av_read_frame: %d\n");
            }
            return pThis->ReportError(DE_ERROR_DECODE_FILE, L"Error av_read_frame: %d\n");
        }

        if (ptrPkt->pPkt->stream_index == pThis->m_nVideoIdx)
        {
            while (DE_ERROR_NO_MORE_BUFFER == pThis->m_ptrVideoDecoder->PushPacket(ptrPkt))
                av_usleep(100);
        }
        else if (ptrPkt->pPkt->stream_index == pThis->m_nAudioIndex)
        {
            while (DE_ERROR_NO_MORE_BUFFER == pThis->m_ptrAudioDecoder->PushPacket(ptrPkt))
                av_usleep(100);
        }
    }

    return 0;
}

DecoderError CFFmpegDecoder::ReportError(DecoderError nErr, LPCWSTR strFormat /*= NULL*/)
{
    m_nErr = nErr;
    m_nStatus = DS_ERROR;
#ifdef _DEBUG
    if (strFormat)
    {
        CLogUtil::Log(strFormat, nErr);
    }
#endif // _DEBUG
    return nErr;
}

CAudioDecoderPtr CFFmpegDecoder::GetAudioDecoder() const
{
    return m_ptrAudioDecoder;
}

