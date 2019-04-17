#include "stdafx.h"
#include "LogUtil.h"
extern "C"
{
#define __STDC_CONSTANT_MACROS
#include <libavutil/avutil.h>
}


CLogUtil::CLogUtil()
{
}


CLogUtil::~CLogUtil()
{
}

void CLogUtil::Log(LPCWSTR strFormat, ...)
{
    int nLength;

    va_list argptr;
    va_start(argptr, strFormat);
    nLength = _vscwprintf_p(strFormat, argptr);
    WCHAR* pText = new WCHAR[nLength + 1];
    if (pText)
    {
        _vsnwprintf_s(pText, nLength + 1, nLength + 1, strFormat, argptr);

        OutputDebugString(pText);
        delete[] pText;
    }

    va_end(argptr);
}

void CLogUtil::Log(LPCSTR strFormat, ...)
{
    int nLength;

    va_list argptr;
    va_start(argptr, strFormat);
    nLength = _vscprintf_p(strFormat, argptr);
    CHAR* pText = new CHAR[nLength + 1];
    if (pText)
    {
        _vsnprintf_s(pText, nLength + 1, nLength + 1, strFormat, argptr);

        OutputDebugStringA(pText);
        delete[] pText;
    }

    va_end(argptr);
}

void CLogUtil::LogAVError(int nAVErrCode)
{
    char szLog[256] = {};
    char szMsg[128] = {};
    av_strerror(nAVErrCode, szMsg, 128);
    sprintf_s(szLog, 256, "FFMPEG Error(%d): %s\n", nAVErrCode, szMsg);
    OutputDebugStringA(szLog);
}
