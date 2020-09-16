#pragma once
#include "Singleton.h"
#include "uv.h"
#include "rtcp.h"
#include "rtp2.h"

namespace RtspServer
{

class CRtspSession;
extern uv_loop_t *g_uv_loop;

/**
 * �ӻỰ��һ��rtsp�Ự�У����԰�����·rtp/rtcpý��
 */
class CRtspSubSession
{
public:
    CRtspSubSession();
    ~CRtspSubSession();

    uint32_t    m_nUseTcp;         //0ʹ��udp���䣬 1ʹ��tcp����
    uint32_t    m_nRtpPort;        //rtpʹ�õĶ˿ں�[udp]���ŵ���[tcp]
    uint32_t    m_nRtcpPort;       //rtcpʹ�õĶ˿ں�[udp]���ŵ���[tcp]
    uv_async_t  m_uvAsyncSchedule; //�����¼�
    string      m_strControl;      //����·��
    rtcp       *m_pRtcp;           //rtcp��������
    rtp        *m_pRtp;            //rtp��������
};

/**
 * rtsp�Ự
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
 * rtsp�Ự����
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
    map<string, CRtspSession*> m_mapSessions;
    uv_timer_t      m_timerExpire;  //��ʱ�ж϶�ʱ��
    volatile uint64_t m_nSession;
};
}