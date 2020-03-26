#include "SipPrivate.h"
#include "SipRegister.h"

using namespace SipServer;

namespace SipServer {
    struct SipContextInfo
    {
        //Sip层返回的请求的标志 响应时返回即可
        int sipRequestId;
        //维护一次注册
        string callId;
        //消息所属的功能方法名字符串
        string method;
        //地址编码@域名或IP地址:连接端口，例如sip:1111@127.0.0.1:5060
        //CSipFromToHeader from;
        string fromCode;
        string fromeIP;
        uint32_t fromPort;
        //地址编码@域名或IP地址:连接端口，例如sip:1111@127.0.0.1:5060
        //CSipFromToHeader proxy;
        string proxyCode;
        string proxyIP;
        uint32_t proxyPort;
        //地址编码@域名或IP地址:连接端口，例如sip:1111@127.0.0.1:5060
        //CContractHeader contact;
        string contactCode;
        string contactIP;
        uint32_t contactPort;
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

    struct SipRegisterInfo
    {
        // SIP请求基本信息
        int      sipRequestId;        //Sip层返回的请求的标志 响应时返回即可
        string   callId;        //维护一次注册
        string   method;        //消息所属的功能方法名字符串
        string   from;        //地址编码@域名或IP地址:连接端口，例如sip:1111@127.0.0.1:5060
        string   fromCode;
        string   fromeIP;
        uint32_t fromPort;
        string   proxy;        //地址编码@域名或IP地址:连接端口，例如sip:1111@127.0.0.1:5060
        string   proxyCode;
        string   proxyIP;
        uint32_t proxyPort;
        string   contact;        //地址编码@域名或IP地址:连接端口，例如sip:1111@127.0.0.1:5060
        string   contactCode;
        string   contactIP;
        uint32_t contactPort;
        string   content;          //消息内容,一般为DDCP消息体XML文档,或者具体协议帧要求的其他字符串文本
        string   status;           //响应状态信息
        int      expires;          //超时,时间单位为秒

        // 鉴权信息
        bool     isAuthNull;       //true:无鉴权信息，false:有鉴权信息
        string   digestRealm;      //平台主机名
        string   nonce;            //平台提供的随机数
        string   userName;         //用户名
        string   response;         //密码
        string   uri;              //sip:平台地址,不需要uac赋值
        string   algorithm;        //加密算法MD5
    };

static string    m_strNonce = "9bd055";       //< UAS赋值的认证随机数
static string    m_strAlgorithm = "MD5";   //< UAS默认加密算法

static void parserRegisterInfo(osip_message_t* request, int iReqId, SipRegisterInfo &regInfo)
{
    std::stringstream stream;

    if (nullptr == request) {
        Log::error("CSipRegister::parserRegisterInfo nullptr == request");
        return;
    }
    Log::debug("CSipRegister::parserRegisterInfo from user:%s,host:%s,port:%s; to user:%s,host:%s,port:%s"
        ,request->from->url->username,request->from->url->host,request->from->url->port
        ,request->to->url->username,request->to->url->host,request->to->url->port);

    regInfo.method    = request->sip_method;
    regInfo.fromCode  = request->from->url->username;
    regInfo.fromeIP   = request->from->url->host;
    regInfo.fromPort  = sz2int(request->from->url->port);
    regInfo.from      = GetFormatHeader(request->from->url->username, request->from->url->host, sz2int(request->from->url->port));
    regInfo.proxyCode = request->to->url->username;
    regInfo.proxyIP   = request->to->url->host;
    regInfo.proxyPort = sz2int(request->to->url->port);
    regInfo.proxy     = GetFormatHeader(request->to->url->username, request->to->url->host, sz2int(request->to->url->port));

    //获取expires
    osip_header_t* header = NULL;
    osip_message_header_get_byname(request, "expires", 0, &header);
    if (NULL != header && NULL != header->hvalue) {
        regInfo.expires = atoi(header->hvalue);
    }

    //contact字段
    osip_contact_t* contact = NULL;
    osip_message_get_contact(request, 0, &contact);
    if (NULL != contact) {
        regInfo.contactCode = contact->url->username;
        regInfo.contactIP   = contact->url->host;
        regInfo.contactPort = sz2int(contact->url->port);
        regInfo.contact     = GetContractFormatHeader(contact->url->username, contact->url->host, sz2int(contact->url->port), regInfo.expires);
    }

    //via字段
    osip_via_t* via = NULL;
    osip_message_get_via(request, 0, &via);
    if (NULL != contact){
    }

    //注册返回 由发送方维护的请求ID 接收方接收后原样返回即可
    regInfo.sipRequestId = iReqId;

    //CALL_ID
    stream.str("");
    stream << request->call_id->number;
    regInfo.callId = stream.str();

    //解析content消息
    osip_body_t * body = NULL;
    osip_message_get_body(request, 0, &body);
    if (body != NULL) {
        stream.str("");
        stream << body->body;
        regInfo.content = stream.str();
    }

    //鉴权信息
    osip_authorization_t* authentication = NULL;
    osip_message_get_authorization(request, 0, &authentication);
    if (NULL == authentication) {
        regInfo.isAuthNull = true;
    } else {
        regInfo.isAuthNull = false;
        stream.str("");
        stream << authentication->username;
        regInfo.userName = stream.str();
        stream.str("");
        stream << authentication->algorithm;
        regInfo.algorithm = stream.str();
        stream.str("");
        stream << authentication->realm;
        regInfo.digestRealm = stream.str();
        stream.str("");
        stream << authentication->nonce;
        regInfo.nonce = stream.str();
        stream.str("");
        stream << authentication->response;
        regInfo.response = stream.str();
        stream.str("");
        stream << authentication->uri;
        regInfo.uri = stream.str();
    }
    authentication = NULL;
}

static void printRegisterPkt(SipRegisterInfo& info)
{
    LogDebug("接收到报文：");
    LogDebug("================================================================");
    LogDebug("method:      %s",info.method.c_str());
    LogDebug("from:        %s",info.from.c_str());
    LogDebug("to:          %s",info.proxy.c_str());
    LogDebug("contact:     %s",info.contact.c_str());

    //注册返回 由发送方维护的请求ID 接收方接收后原样返回即可
    LogDebug("RequestId:   %d",info.sipRequestId);
    //CALL_ID
    LogDebug("CallId:      %s",info.callId.c_str());
    //解析content消息
    LogDebug("body:        %s",info.content.c_str());
    //获取expires
    LogDebug("expires:     %d",info.expires);
    //鉴权信息
    if (info.isAuthNull) {
        LogDebug("当前报文未提供鉴权信息!!!");
    } else {
        LogDebug("当前报文鉴权信息如下:");
        LogDebug("username: %s",info.userName.c_str());
        LogDebug("algorithm:%s",info.algorithm.c_str());
        LogDebug("Realm:    %s",info.digestRealm.c_str());
        LogDebug("nonce:    %s",info.nonce.c_str());
        LogDebug("response: %s",info.response.c_str());
        LogDebug("uri:      %s",info.uri.c_str());
    }
    LogDebug("================================================================");
    return;
}

static void sendRegisterAnswer(SipRegisterInfo& info)
{
    osip_message_t* answer = NULL;
    int iStatus;
    eXosip_lock(g_pExContext);
    if (g_bRegAuthor && info.isAuthNull) {
        iStatus = 401;
        int result = ::eXosip_message_build_answer(g_pExContext,info.sipRequestId,iStatus, &answer);
        if (OSIP_SUCCESS != result) {
            ::eXosip_message_send_answer(g_pExContext,info.sipRequestId, 400, NULL);
        } else {
            std::stringstream stream;
            string nonce = m_strNonce;
            string algorithm = m_strAlgorithm;
            stream << "Digest realm=\"" << info.fromeIP 
                << "\",nonce=\"" << nonce
                << "\",algorithm=" << algorithm;

            osip_message_set_header(answer, "WWW-Authenticate", stream.str().c_str());
            ::eXosip_message_send_answer(g_pExContext,info.sipRequestId, iStatus, answer);
            Log::warning("注册应答 发送401报文");
        }
    } else {
        iStatus = 200;
        int result = ::eXosip_message_build_answer(g_pExContext,info.sipRequestId,iStatus, &answer);
        if (OSIP_SUCCESS != result) {
            ::eXosip_message_send_answer(g_pExContext,info.sipRequestId, 400, NULL);
             Log::warning("注册应答 发送400报文");
        } else {
            osip_message_set_header(answer, "Contact", info.contact.c_str());
            //发送消息体
            ::eXosip_message_send_answer(g_pExContext,info.sipRequestId, iStatus, answer);
            Log::warning("注册应答 发送200报文");

            //下级平台信息
            //g_strLowCode    = info.contactCode;
            //g_strLowIP      = info.contactIP;
            //g_nLowPort      = info.contactPort;
            g_nLowExpire    = info.expires;
            g_bLowStatus    = true;

            // 查询目录信息
            AutoQuery();
        }
    }


    if (0 == info.expires){
        eXosip_register_remove(g_pExContext,info.sipRequestId);
    }

    eXosip_unlock(g_pExContext);
}

void CSipRegister::OnRegister(eXosip_event_t *osipEvent)
{
    SipRegisterInfo regInfo;
    parserRegisterInfo(osipEvent->request, osipEvent->tid, regInfo);
    //打印报文
    printRegisterPkt(regInfo);
    //发送应答报文
    sendRegisterAnswer(regInfo);

}

}