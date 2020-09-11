#pragma once
#include "uvlogpublic.h"
#include <string>

namespace uvLogPlus {
class UVLOG_API CLog
{
public:
    /** 根据默认配置文件创建日志 */
    static CLog* Create();
    /** 根据日志配置文件路径创建日志 */
    static CLog* Create(std::string path);
    /** 根据日志配置内容创建日志 */
    static CLog* Create(const char *buff);

    virtual ~CLog(){};

    /** 写日志 */
    virtual void Write(std::string name, Level level, const char *file, int line, const char *function, const char *fmt, ...) = 0;
protected:
    CLog(){};
};
};