#pragma once
#include "SipMsgParser.h"

/**
 * 会话邀请
 */
class CSipInvite
{
public:
    CSipInvite(eXosip_t* pSip);
    ~CSipInvite(void);

    /**
     * 发送会话邀请
     * @param pPlatform[in] 下级平台信息
     * @param pDevInfo[in] 视频设备信息
     * @param nRTPPort[in] 接收rtp的端口
     * @return call-id
     */
    int SendInvite(PlatFormInfo* pPlatform, DevInfo* pDevInfo, int nRTPPort);
    int SendRecordInvite(PlatFormInfo* pPlatform, DevInfo* pDevInfo, int nRTPPort, string beginTime, string endTime);

    /**
     * 接收到200OK返回
     */
    void OnInviteOK(eXosip_event_t *osipEvent);

    /**
     * 结束会话
     */
    void SendBye(int cid, int did);

    /**
     * 
     */
    void OnCallNew(eXosip_event_t *osipEvent);

    /**
     * 
     */
    void OnCallClose(eXosip_event_t *osipEvent);

    /**
     * 
     */
    void OnCallClear(eXosip_event_t *osipEvent);
private:
    eXosip_t*      m_pExContext;
};

