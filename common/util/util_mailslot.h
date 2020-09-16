#pragma once
#include "util_public.h"
#ifdef WINDOWS_IMPL
#include <string>
typedef void * HANDLE;

namespace util {
/**
 * 邮件槽服务端
 */
class UTIL_API CMailSlotSever
{
public:
    CMailSlotSever(std::string strName);
    ~CMailSlotSever();

    void SetCallback(void (*p)(std::string));

private:
    static unsigned long _cdecl ServeMailslot(void* lpParameter);

private:
    bool        m_bStop;    //< 停止服务
    std::string m_strName;  //< 邮件槽名称

    void (*fMessCallBack)(std::string); 
};

/**
 * 邮件槽客户端
 */
class UTIL_API CMailSlotClient
{
public:
    CMailSlotClient(std::string strName);
    ~CMailSlotClient();

    bool SendMail(std::string strInfo); 

private:
    std::string m_strName;  //< 邮件槽名称
    HANDLE      m_hMailslot;    //< 文件槽句柄
};
};
#endif