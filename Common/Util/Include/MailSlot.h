#pragma once
#include "ExportDefine.h"
#include <string>
#include <windows.h>

/**
 * 邮件槽服务端
 */
class COMMON_API CMailSlotSever
{
public:
    CMailSlotSever(std::string strName);
    ~CMailSlotSever();

    void SetCallback(void (*p)(std::string));

private:
    static DWORD WINAPI ServeMailslot(LPVOID lpParameter);

private:
    bool        m_bStop;    //< 停止服务
    std::string m_strName;  //< 邮件槽名称

    void (*fMessCallBack)(std::string); 
};

/**
 * 邮件槽客户端
 */
class COMMON_API CMailSlotClient
{
public:
    CMailSlotClient(std::string strName);
    ~CMailSlotClient();

    bool SendMail(std::string strInfo); 

private:
    std::string m_strName;  //< 邮件槽名称
    HANDLE      m_hMailslot;    //< 文件槽句柄
};