#pragma once
#include "SipHeaders.h"
#include "SipMsgParser.h"

struct sipMessageInfo
{
    //Sip层返回的请求的标志 响应时返回即可
    int sipRequestId;
    //维护一次注册
    string callId;
    //消息所属的功能方法名字符串
    string method;
    //地址编码@域名或IP地址:连接端口，例如sip:1111@127.0.0.1:5060
    CSipFromToHeader from;
    //地址编码@域名或IP地址:连接端口，例如sip:1111@127.0.0.1:5060
    CSipFromToHeader to;
    // 消息体
    string content;
    // 消息类型
    string strCmdType;
    // 消息体解析
    map<string,string> mapBody;
    msgPublic* pMsgBody;
};

/**
 * 处理接收的消息，解析及应答。发送message请求(查询目录、云台控制)
 */
class CSipMessage
{
public:
    CSipMessage(eXosip_t* pSip);
    ~CSipMessage(void);

    /**
     * 处理Message事件
     */
    void OnMessage(eXosip_event_t *osipEvent);

    /**
     * 目录查询
     * @param strDevCode[in] 平台编码
     * @param strAddrIP[in] 平台IP
     * @param strAddrPort[in] 平台端口
     */
    void QueryDirtionary(string strDevCode, string strAddrIP, string strAddrPort);

    /**
     * 设备状态查询
     * @param strDevCode[in] 平台编码
     * @param strAddrIP[in] 平台IP
     * @param strAddrPort[in] 平台端口
     * @param devID[in] 设备id
     */
    void QueryDeviceStatus(string strDevCode, string strAddrIP, string strAddrPort, string devID);

    /**
     * 设备信息查询请求
     * @param strDevCode[in] 平台编码
     * @param strAddrIP[in] 平台IP
     * @param strAddrPort[in] 平台端口
     * @param devID[in] 设备id
     */
    void QueryDeviceInfo(string strDevCode, string strAddrIP, string strAddrPort, string devID);

    /**
     * 文件目录检索请求
     * @param strDevCode[in] 平台编码
     * @param strAddrIP[in] 平台IP
     * @param strAddrPort[in] 平台端口
     * @param devID[in] 设备id
     */
    void QueryRecordInfo(string strDevCode, string strAddrIP, string strAddrPort, string devID, string strStartTime, string strEndTime);

    /**
     * 移动设备位置查询
     * @param strDevCode[in] 平台编码
     * @param strAddrIP[in] 平台IP
     * @param strAddrPort[in] 平台端口
     * @param devID[in] 设备id
     */
    void QueryMobilePosition(string strDevCode, string strAddrIP, string strAddrPort, string devID);

    /**
     * 云台控制
     * @param strPlatformCode[in] 平台编码
     * @param strAddrIP[in]   平台IP
     * @param strAddrPort[in] 平台端口
     * @param strDevCode[in] 设备编码
     * @param nInOut[in]     镜头放大缩小 0:停止 1:缩小 2:放大
     * @param nUpDown[in]    镜头上移下移 0:停止 1:上移 2:下移
     * @param nLeftRight[in] 镜头左移右移 0:停止 1:左移 2:右移
     * @param cMoveSpeed[in] 镜头缩放速度
     * @param cMoveSpeed[in] 镜头移动速度
     */
    void DeviceControl(string strAddrIP, string strAddrPort, string strDevCode,
        int nInOut = 0, int nUpDown = 0, int nLeftRight = 0, 
        uchar cInOutSpeed = 0X1, uchar cMoveSpeed = 0XFF);
private:
    /**
     * 解析消息信息
     */
    void parserMessageInfo(osip_message_t*request, int iReqId, sipMessageInfo &msgInfo);

    /**
     * 打印接收到的响应报文
     */
    void printMeaassgePkt(sipMessageInfo& info);

    /**
     * 发送应答
     */
    void sendMessageAnswer(sipMessageInfo& info);

private:
    eXosip_t*      m_pExContext;
    CSipMsgParser  m_msgParser;   //< 消息体解析工具
};

