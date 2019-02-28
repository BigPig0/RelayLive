#pragma once
#include "PublicDefine.h"

/**
 * 一个Invite会话的整个过程
 */
class CSipCall
{
public:
    ~CSipCall(void);

    /**
     * 创建一个SIP通话，发送视频邀请
     * @param strPlatformCode[in] 平台编码
     * @param strDevCode[in] 设备编码
     * @param strIP[in] rtp接收服务IP
     * @param nPort[in] rtp接收服务端口
     * @return 成功true,失败false
     */
    static bool CreatSipCall(string strDevCode, string strIP, int nPort);
    static bool CreatSipCall(string strDevCode, string strIP, int nPort, 
        string startTime, string endTime);

    /**
     * 结束一个SIP通话，并删除对象
     * @param strRtpPort[in] rtp端口，作为会话id
     * @return 成功true,失败false
     */
    static bool StopSipCall(string strRtpPort);

    static bool StopSipCallAll();

    /**
     * 发送视频邀请
     */
    bool SendInvite();
    bool SendRecordInvite();

    /**
     * 发送视频请求成功
     */
    bool OnInviteOk(int nDID, char* szBody, int nLength);
    bool OnInviteFailed();
    bool WaiteInviteFinish();

    /**
     * 结束视频
     */
    bool SendBye();

    /**
     * 根据Call-ID找到对象
     */
    static CSipCall* FindByCallID(int nCID);

private:
    CSipCall();

public:
    static map<int,CSipCall*>     m_mapGlobalCall;  //< 所有的该类实例建立索引，key是callid，以便响应时找到
    static map<string,CSipCall*>  m_mapDeviceCall;  //< key是rtp端口。因为端口不重复，可以作为ID
    static CriticalSection        m_csGlobalCall;

private:
    // 会话信息
    int             m_nCallID;     //< invite建立时得到的ID
    int             m_nDialogID;   //< 建立成功可以从osip_event中得到
    int             m_nRtpPort;    //< rtp接收端口，rtcp端口加1
    string          m_strRtpIP;    //< rtp服务IP
    // 设备信息
    string          m_strPlatformCode;  //< 平台的编码
    string          m_strDevCode;       //< 设备的编码

    int             m_nInvite;     //< 邀请状态标记，发送邀请置为0，收到邀请应答置为1

    //历史视频点播
    bool            m_bRecord;
    string          m_strBeginTime;
    string          m_strEndTime;
};

