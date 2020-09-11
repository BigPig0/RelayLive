#include "util_mailslot.h"
#ifdef WINDOWS_IMPL
#include <windows.h>

namespace util {

CMailSlotSever::CMailSlotSever(std::string strName)
    :m_bStop(false)
    ,m_strName(strName)
    ,fMessCallBack(NULL)
{
    DWORD ThreadId;
    HANDLE MailslotThread = CreateThread(NULL,0,ServeMailslot,LPVOID(this),0,&ThreadId);
    CloseHandle(MailslotThread);
}

CMailSlotSever::~CMailSlotSever()
{
    m_bStop=true;
    CMailSlotClient* client = new CMailSlotClient(m_strName);
    client->SendMail("STOP");
    delete client;
}

//This function is the mailslot server worker function to
//process all incoming mailslot I/O
DWORD _cdecl CMailSlotSever::ServeMailslot(void* lpParameter)
{
    CMailSlotSever* pSever = (CMailSlotSever*)lpParameter;

    /** 创建邮件槽 */
    std::string strName = "\\\\.\\Mailslot\\" + pSever->m_strName;
    HANDLE Mailslot = CreateMailslotA(strName.c_str(),0,MAILSLOT_WAIT_FOREVER,NULL);
    if (INVALID_HANDLE_VALUE == Mailslot)
    {
        int i = GetLastError();
        printf("Failed to create a MailSlot %d/n",i);
        return 1;
    }

    /** 读取邮件槽数据 */
    std::string strRead;
    BOOL Ret = FALSE;
    char buffer[256];
    DWORD NumberOfBytesRead;
    while (true)
    {
        if (pSever->m_bStop)
            break;

        memset(buffer,0,256);
        Ret = ReadFile(Mailslot,buffer,256,&NumberOfBytesRead,NULL);
        if(!Ret || NumberOfBytesRead == 0) continue;

        strRead = buffer;
        if(pSever->fMessCallBack)
            pSever->fMessCallBack(strRead);
    }

    /** 关闭邮件槽 */
    CloseHandle(Mailslot);
    return 0;
}

void CMailSlotSever::SetCallback(void (*p)(std::string))
{
    fMessCallBack = p;
}

CMailSlotClient::CMailSlotClient(std::string strName)
    :m_hMailslot(INVALID_HANDLE_VALUE)
{
    m_strName = "\\\\.\\Mailslot\\" + strName;
    m_hMailslot = CreateFileA(m_strName.c_str(), GENERIC_WRITE,
        FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL);
}

CMailSlotClient::~CMailSlotClient()
{
    CloseHandle(m_hMailslot);
}

bool CMailSlotClient::SendMail(std::string strInfo)
{
    DWORD BytesWritten = 0;
    if(0 == WriteFile(m_hMailslot, strInfo.c_str(), (DWORD)strInfo.size(), &BytesWritten,NULL))
    {
        return false;
    }
    return true;
}
}
#endif