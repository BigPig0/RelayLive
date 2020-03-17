#pragma once

namespace SipServer {

/**
 * 会话邀请
 */
class CSipInvite
{
public:
    /**
     * 建立邀请准备
     */
    void InviteInit(string strProName, uint32_t nID, string strCode, int nRTPPort);

    /**
     * 发送会话邀请
     * @param strProName[in] 发起请求的进程名称
     * @param nID[in] 请求ID
     * @param strDevID[in] 视频设备编码
     * @param nRTPPort[in] 接收rtp的端口
     * @return call-id
     */
    int SendInvite(string strProName, uint32_t nID, int nRTPPort);
    int SendRecordInvite(string strProName, uint32_t nID, int nRTPPort, string beginTime, string endTime);

    /**
     * 接收到200OK返回
     */
    void OnInviteOK(eXosip_event_t *osipEvent);
    void OnInviteFailed(eXosip_event_t *osipEvent);

    /**
     * 结束会话,指定端口的会话
     */
    bool StopSipCall(uint32_t nRtpPort);

    /**
     * 结束指定liveserver的所有会话
     */
    vector<uint32_t> StopSipCallAll(string strProName);

    /**
     * 收到新建会话
     */
    void OnCallNew(eXosip_event_t *osipEvent);

    /**
     * 收到对方发起关闭
     */
    void OnCallClose(eXosip_event_t *osipEvent);

    /**
     * 收到会话结束
     */
    void OnCallClear(eXosip_event_t *osipEvent);
};

};