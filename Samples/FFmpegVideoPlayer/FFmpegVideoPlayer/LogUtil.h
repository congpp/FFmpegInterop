#pragma once
class CLogUtil
{
public:
    CLogUtil();
    ~CLogUtil();

    static void Log(LPCWSTR strFormat, ...);
    static void Log(LPCSTR strFormat, ...);

    static void LogAVError(int nAVErrCode);
};

