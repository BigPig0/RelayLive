#include "stdafx.h"
#include "RtspSession.h"

namespace RtspServer
{
/**
 * 会话生成一个rtcp报文
 */
static void sess_rtcp_cb(void *user, AV_BUFF buff) {

}

/**
 * 会话生成一个rtp报文
 */
static void sess_rtp_cb(void *user, AV_BUFF buff) {

}

/**
 * 
 */
static void sess_async_cb(uv_async_t* handle) {

}

CRtspSubSession::CRtspSubSession()
    : m_nUseTcp(0)
{
    m_pRtcp = create_rtcp(this, sess_rtcp_cb);
    m_pRtp  = create_rtp(this, sess_rtp_cb);

    m_uvAsyncSchedule.data = this;
    uv_async_init(g_uv_loop, &m_uvAsyncSchedule, sess_async_cb);
}

CRtspSubSession::~CRtspSubSession()
{
    destory_rtcp(m_pRtcp);
    destory_rtp(m_pRtp);
    uv_close((uv_handle_t*)&m_uvAsyncSchedule, NULL);
}

//////////////////////////////////////////////////////////////////////////

CRtspSession::CRtspSession(void)
{
}


CRtspSession::~CRtspSession(void)
{
}

CRtspSubSession* CRtspSession::NewSubSession(string control)
{
    CRtspSubSession *newSubSess = new CRtspSubSession();
    newSubSess->m_strControl = control;
    m_mapSubSessions.insert(make_pair(control, newSubSess));
    return newSubSess;
}

CRtspSubSession* CRtspSession::GetSubSession(string control)
{
    CRtspSubSession* ret = NULL;
    auto fit = m_mapSubSessions.find(control);
    if (fit != m_mapSubSessions.end())
    {
        ret = fit->second;
    }
    return ret;
}

//////////////////////////////////////////////////////////////////////////

CRtspSessionMgr::CRtspSessionMgr()
    : m_nSession(11111111)
{
}

CRtspSessionMgr::~CRtspSessionMgr()
{
}

CRtspSession* CRtspSessionMgr::GetSession(string session)
{
    auto fit = m_mapSessions.find(session);
    if(fit != m_mapSessions.end()) {
        return fit->second;
    }

    return nullptr;
}

CRtspSession* CRtspSessionMgr::NewSession()
{
    string session = StringHandle::toStr<uint64_t>(m_nSession++);
    CRtspSession *newSess = new CRtspSession();
    newSess->m_strSessID = session;
    m_mapSessions.insert(make_pair(session, newSess));
    return newSess;
}
}