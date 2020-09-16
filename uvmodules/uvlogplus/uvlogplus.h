#pragma once
#include "uvlogpublic.h"
#include <string>

namespace uvLogPlus {
class UVLOG_API CLog
{
public:
    /** ����Ĭ�������ļ�������־ */
    static CLog* Create();
    /** ������־�����ļ�·��������־ */
    static CLog* Create(std::string path);
    /** ������־�������ݴ�����־ */
    static CLog* Create(const char *buff);

    virtual ~CLog(){};

    /** д��־ */
    virtual void Write(std::string name, Level level, const char *file, int line, const char *function, const char *fmt, ...) = 0;
protected:
    CLog(){};
};
};