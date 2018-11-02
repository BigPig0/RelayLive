#include "stdafx.h"
#include "Log.h"
#include "ProcessMgr.h"
#include <windows.h>
#include <time.h>
#include <thread>

CProcessMgr::CProcessMgr(void)
{
    char selfPath[MAX_PATH];
    GetModuleFileName(NULL, selfPath, MAX_PATH);
    m_strPath = selfPath;
}


CProcessMgr::~CProcessMgr(void)
{
}

bool CProcessMgr::RunChild(int nNum, string strDevInfo)
{
    ProcessInfo* newProcess = new ProcessInfo;
    if(nullptr == newProcess)
        return false;
    newProcess->m_nNum = nNum;
    newProcess->m_strCMD = strDevInfo;
    newProcess->m_lPID = 0;

//     if(!CreateChildProcess(nNum, strDevInfo, newProcess->m_lPID))
//     {
//         delete newProcess;
//         return false;
//     }
//
//    newProcess->m_tStart = time(NULL);

    m_cs.lock();
    m_vecProcess.push_back(newProcess);
    m_cs.unlock();

    return true;
}

bool CProcessMgr::Protect()
{
    std::thread t([&](){
        while(true)
        {
            try
            {
                ProtectRun();
            }
            catch(...)
            {
                Log::error("ProtectRun error");
            }
            Sleep(60000);
        }
    });
    t.detach();
    return true;
}

bool CProcessMgr::ProtectRun()
{
    time_t now = time(NULL);
    struct tm * timeinfo = localtime(&now);
    MutexLock lock(&m_cs);
    for (auto& pProcess:m_vecProcess)
    {
        if ( pProcess->m_lPID == 0                        //< 尚未启动
            || !Find(pProcess->m_lPID)                    //< 进程异常退出
            || (difftime(now,pProcess->m_tStart) > 3600   //< 60*60
               && timeinfo->tm_hour == 5)                 //< 每天5:00以后
           )
        {
            // 重启程序
            std::thread t([&](){
                try
                {
                    RunChild(pProcess);
                }
                catch(...)
                {
                    Log::error("RunChild error");
                }
            });
            t.detach();
        }
    }
    return true;
}

bool CProcessMgr::RunChild(ProcessInfo* pro)
{
    if(pro->m_lPID != 0 && Find(pro->m_lPID) && !Kill(pro->m_lPID))
    {
        Log::debug("kill process:%d failed", pro->m_nNum);
        return false;
    }

    if(!CreateChildProcess(pro->m_nNum, pro->m_strCMD, pro->m_lPID))
    {
        Log::error("restart process failed");
        return false;
    }

    pro->m_tStart = time(NULL);

    return true;
}

bool CProcessMgr::CreateChildProcess(int nNum, string strDevInfo, DWORD& lPID)
{
    bool bRes = false;

    /** 创建管道，重定向子进程标准输入 */
    SECURITY_ATTRIBUTES sa;
    memset(&sa, 0, sizeof(sa));
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;

    HANDLE childStdInRead = INVALID_HANDLE_VALUE, 
        childStdInWrite = INVALID_HANDLE_VALUE;

    CreatePipe(&childStdInRead, &childStdInWrite, &sa, 0);
    SetHandleInformation(childStdInWrite, HANDLE_FLAG_INHERIT, 0);

    /** 创建子进程 */
    STARTUPINFO si;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    si.hStdInput = childStdInRead;
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si.wShowWindow = SW_MINIMIZE;
    si.dwFlags |= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

    PROCESS_INFORMATION pi;
    memset(&pi, 0, sizeof(pi));
    pi.hProcess = INVALID_HANDLE_VALUE;
    pi.hThread = INVALID_HANDLE_VALUE;

    char cmdline[MAX_PATH];
    sprintf_s(cmdline, MAX_PATH, "%s %d", m_strPath.c_str(), nNum);

    if (!CreateProcess(NULL, cmdline, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
    {
        Log::error("CreateProcess failed:%d", GetLastError());
        goto end;
    }
    WaitForInputIdle(pi.hProcess, INFINITE); //<  等待新进程完成它的初始化并等待用户输入

    /** 向子进程写入设备信息 */
    DWORD writeBytes;
    if (!WriteFile(childStdInWrite, strDevInfo.c_str(), strDevInfo.size(), &writeBytes, NULL))
    {
        Log::error("WriteFile failed:%d", GetLastError());
        while(!Kill(pi.dwProcessId))
        {
            Log::error("kill failed");
            Sleep(1000);
        }
        goto end;
    }

    lPID = pi.dwProcessId;
    bRes = true;

end:
    if(INVALID_HANDLE_VALUE != childStdInWrite)
        CloseHandle(childStdInWrite);
    if(INVALID_HANDLE_VALUE != pi.hProcess)
        CloseHandle(pi.hProcess);
    if(INVALID_HANDLE_VALUE != pi.hThread)
        CloseHandle(pi.hThread);

    if(bRes)
        Log::debug("RunChild num:%d, PID:%ld sucess",nNum,pi.dwProcessId);
    else
        Log::error("RunChild num:%d, PID:%ld failed",nNum,pi.dwProcessId);

    return bRes;
}

bool CProcessMgr::Remove(int nNum)
{
    bool bKill = false;
    m_cs.lock();
    for (auto it=m_vecProcess.begin(); it!=m_vecProcess.end();++it)
    {
        if ((*it)->m_nNum == nNum)
        {
            if(Kill((*it)->m_lPID))
            {
                delete (*it);
                m_vecProcess.erase(it);
                bKill = true;
            }
            break;
        }
    }
    m_cs.unlock();
    return bKill;
}

bool CProcessMgr::Find(DWORD lPID)
{
    HANDLE h = OpenProcess(PROCESS_ALL_ACCESS,FALSE,lPID);
    if (NULL == h)
    {
        Log::debug("unfind process PID:%ld",lPID);
        return false;
    }
    CloseHandle(h);
    return true;
}

bool CProcessMgr::Kill(DWORD lPID)
{
    int i = 5;
    //Log::debug("begin kill PID:%ld",lPID);
    while(i--)
    {
        HANDLE h=OpenProcess(PROCESS_TERMINATE,FALSE,lPID);
        if(NULL == h)
        {
            Log::warning("kill process %ld sucess", lPID);
            return true;
        }
        if(FALSE == TerminateProcess(h,0))
        {
            DWORD dwError = GetLastError();
            Log::error("TerminateProcess failed:%d",dwError);
        }
        CloseHandle(h);
        Sleep(100);
    }
    //Log::debug("end kill PID:%ld",lPID);
    HANDLE h=OpenProcess(PROCESS_TERMINATE,FALSE,lPID);
    if(NULL == h)
    {
        Log::warning("kill process %ld sucess", lPID);
        return true;
    }

    CloseHandle(h);
    Log::error("process is still exist:%ld(handle:%d)", lPID,h);
    return false;
}