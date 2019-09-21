#pragma once

#include "ExportDefine.h"
#include "Mutex.h"
#include <vector>
#include <string>
using namespace std;

struct ProcessInfo
{
    DWORD   m_lPID;         //< 进程ID
    int     m_nNum;         //< 进程编号
    string  m_strCMD;       //< 发送给子进程的命令
    time_t  m_tStart;       //< 程序启动时间
};

/**
 * 进程管理器，负责子进程的启动、保护、定时重启
 */
class COMMON_API CProcessMgr
{
public:
    CProcessMgr(void);
    ~CProcessMgr(void);

    /**
     * 启动一个子进程
     * @param nNum 同一个服务器上的进程任务编号
     * @param strDevInfo 该进程处理的设备的信息
     * @return true成功，false失败
     */
    bool RunChild(int nNum, string strDevInfo);

    /**
     * 启动保护线程
     */
    bool Protect();

    /**
     * 保护线程
     */
    bool ProtectRun();

private:
    /**
     * 重启子进程
     */
    bool RunChild(ProcessInfo* pro);

    /**
     * 启动进程，并通过管道写入数据
     * @param nNum 同一个服务器上的进程任务编号
     * @param strDevInfo 该进程处理的设备的信息
     * @return true成功，false失败
     */
    bool CreateChildProcess(int nNum, string strDevInfo, DWORD& lPID);

    /**
     * 移除进程信息
     * @param nNum 进程编号
     */
    bool Remove(int nNum);

    /**
     * 查找一个进程
     * @param lPID 进程ID
     */
    bool Find(DWORD lPID);

    /**
     * 结束一个进程
     * @param lPID 进程ID
     */
    bool Kill(DWORD lPID);

    vector<ProcessInfo*>    m_vecProcess;       //< 子进程信息
    CriticalSection         m_cs;               //< m_vecProcess的锁
    string                  m_strPath;          //< 执行程序路径
};