#pragma once
#include "Singleton.h"
#include "uv.h"

class CRtspSession;

/**
 * 子会话，一个rtsp会话中，可以包含多路rtp/rtcp媒体
 */
class CRtspSubSession
{
public:
    CRtspSubSession();
    ~CRtspSubSession();

    uint32_t    m_nUseTcp;   //0使用udp传输， 1使用tcp传输
    uint32_t    m_nRtpPort;  //rtp使用的端口号[udp]，
    uint32_t    m_nRtcpPort;
    string m_strControl;//控制路径
};

/**
 * rtsp会话
 */
class CRtspSession
{
public:
    CRtspSession(void);
    ~CRtspSession(void);

    CRtspSubSession* NewSubSession(string control);
    CRtspSubSession* GetSubSession(string control);

    string m_strSessID;
    map<string, CRtspSubSession*> m_mapSubSessions;
};

/**
 * rtsp会话管理
 */
class CRtspSessionMgr : public Singleton<CRtspSessionMgr>
{
    friend class Singleton<CRtspSessionMgr>;
public:
    CRtspSessionMgr();
    ~CRtspSessionMgr();

    CRtspSession* GetSession(string session);
    CRtspSession* NewSession();

private:
    static map<string, CRtspSession*> m_mapSessions;
    uv_timer_t      m_timerExpire;  //超时判断定时器
    volatile uint64_t m_nSession;
};
