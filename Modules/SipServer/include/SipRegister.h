#pragma once
#include "SipHeaders.h"

struct SipContextInfo
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
    CSipFromToHeader proxy;
    //地址编码@域名或IP地址:连接端口，例如sip:1111@127.0.0.1:5060
    CContractHeader contact;
    //消息内容,一般为DDCP消息体XML文档,或者具体协议帧要求的其他字符串文本
    string content;
    //响应状态信息
    string status;
    //超时,时间单位为秒
    int expires;
};

struct SipAuthInfo
{
    //平台主机名
    string digestRealm;
    //平台提供的随机数
    string nonce;
    //用户名
    string userName;
    //密码
    string response;
    //“sip:平台地址”,不需要uac赋值
    string uri;
    //加密算法MD5
    string algorithm;
};

struct sipRegisterInfo
{
    SipContextInfo baseInfo;
    SipAuthInfo authInfo;
    bool isAuthNull;             //< true:无鉴权信息，false:有鉴权信息
};

/**
 * 处理接收的注册请求
 */
class CSipRegister
{
public:
    CSipRegister(eXosip_t* pSip);
    ~CSipRegister(void);

    /**
     * 设置是否开启鉴权
     * @param bAuth true:需要鉴权，false:不需要鉴权
     */
    void SetAuthorization(bool bAuth);

    /**
     * 处理注册事件
     */
    void OnRegister(eXosip_event_t *osipEvent);

private:
    /**
     * 解析注册信息
     * @param 
     */
    void parserRegisterInfo(osip_message_t*request, int iReqId, sipRegisterInfo &regInfo);

    //打印接收到的响应报文
    void printRegisterPkt(sipRegisterInfo& info);

    /**
     * 发送应答
     */
    void sendRegisterAnswer(sipRegisterInfo& info);

private:
    eXosip_t* m_pExContext;
    string    m_strNonce;       //< UAS赋值的认证随机数
    string    m_strAlgorithm;   //< UAS默认加密算法
    bool      m_bAuthorization; //< 是否需要鉴权
};

