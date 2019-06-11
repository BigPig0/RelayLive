#include "stdafx.h"
#include "RtspSession.h"

CRtspSubSession::CRtspSubSession()
    : m_nUseTcp(0)
{
}

CRtspSubSession::~CRtspSubSession()
{
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