#include "SipPrivate.h"
#include "SipRegister.h"

using namespace SipServer;

namespace SipServer {
    struct SipContextInfo
    {
        //Sip�㷵�ص�����ı�־ ��Ӧʱ���ؼ���
        int sipRequestId;
        //ά��һ��ע��
        string callId;
        //��Ϣ�����Ĺ��ܷ������ַ���
        string method;
        //��ַ����@������IP��ַ:���Ӷ˿ڣ�����sip:1111@127.0.0.1:5060
        //CSipFromToHeader from;
        string fromCode;
        string fromeIP;
        uint32_t fromPort;
        //��ַ����@������IP��ַ:���Ӷ˿ڣ�����sip:1111@127.0.0.1:5060
        //CSipFromToHeader proxy;
        string proxyCode;
        string proxyIP;
        uint32_t proxyPort;
        //��ַ����@������IP��ַ:���Ӷ˿ڣ�����sip:1111@127.0.0.1:5060
        //CContractHeader contact;
        string contactCode;
        string contactIP;
        uint32_t contactPort;
        //��Ϣ����,һ��ΪDDCP��Ϣ��XML�ĵ�,���߾���Э��֡Ҫ��������ַ����ı�
        string content;
        //��Ӧ״̬��Ϣ
        string status;
        //��ʱ,ʱ�䵥λΪ��
        int expires;
    };

    struct SipAuthInfo
    {
        //ƽ̨������
        string digestRealm;
        //ƽ̨�ṩ�������
        string nonce;
        //�û���
        string userName;
        //����
        string response;
        //��sip:ƽ̨��ַ��,����Ҫuac��ֵ
        string uri;
        //�����㷨MD5
        string algorithm;
    };

    struct SipRegisterInfo
    {
        // SIP���������Ϣ
        int      sipRequestId;        //Sip�㷵�ص�����ı�־ ��Ӧʱ���ؼ���
        string   callId;        //ά��һ��ע��
        string   method;        //��Ϣ�����Ĺ��ܷ������ַ���
        string   from;        //��ַ����@������IP��ַ:���Ӷ˿ڣ�����sip:1111@127.0.0.1:5060
        string   fromCode;
        string   fromeIP;
        uint32_t fromPort;
        string   proxy;        //��ַ����@������IP��ַ:���Ӷ˿ڣ�����sip:1111@127.0.0.1:5060
        string   proxyCode;
        string   proxyIP;
        uint32_t proxyPort;
        string   contact;        //��ַ����@������IP��ַ:���Ӷ˿ڣ�����sip:1111@127.0.0.1:5060
        string   contactCode;
        string   contactIP;
        uint32_t contactPort;
        string   content;          //��Ϣ����,һ��ΪDDCP��Ϣ��XML�ĵ�,���߾���Э��֡Ҫ��������ַ����ı�
        string   status;           //��Ӧ״̬��Ϣ
        int      expires;          //��ʱ,ʱ�䵥λΪ��

        // ��Ȩ��Ϣ
        bool     isAuthNull;       //true:�޼�Ȩ��Ϣ��false:�м�Ȩ��Ϣ
        string   digestRealm;      //ƽ̨������
        string   nonce;            //ƽ̨�ṩ�������
        string   userName;         //�û���
        string   response;         //����
        string   uri;              //sip:ƽ̨��ַ,����Ҫuac��ֵ
        string   algorithm;        //�����㷨MD5
    };

static string    m_strNonce = "9bd055";       //< UAS��ֵ����֤�����
static string    m_strAlgorithm = "MD5";   //< UASĬ�ϼ����㷨

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

    //��ȡexpires
    osip_header_t* header = NULL;
    osip_message_header_get_byname(request, "expires", 0, &header);
    if (NULL != header && NULL != header->hvalue) {
        regInfo.expires = atoi(header->hvalue);
    }

    //contact�ֶ�
    osip_contact_t* contact = NULL;
    osip_message_get_contact(request, 0, &contact);
    if (NULL != contact) {
        regInfo.contactCode = contact->url->username;
        regInfo.contactIP   = contact->url->host;
        regInfo.contactPort = sz2int(contact->url->port);
        regInfo.contact     = GetContractFormatHeader(contact->url->username, contact->url->host, sz2int(contact->url->port), regInfo.expires);
    }

    //via�ֶ�
    osip_via_t* via = NULL;
    osip_message_get_via(request, 0, &via);
    if (NULL != contact){
    }

    //ע�᷵�� �ɷ��ͷ�ά��������ID ���շ����պ�ԭ�����ؼ���
    regInfo.sipRequestId = iReqId;

    //CALL_ID
    stream.str("");
    stream << request->call_id->number;
    regInfo.callId = stream.str();

    //����content��Ϣ
    osip_body_t * body = NULL;
    osip_message_get_body(request, 0, &body);
    if (body != NULL) {
        stream.str("");
        stream << body->body;
        regInfo.content = stream.str();
    }

    //��Ȩ��Ϣ
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
    LogDebug("���յ����ģ�");
    LogDebug("================================================================");
    LogDebug("method:      %s",info.method.c_str());
    LogDebug("from:        %s",info.from.c_str());
    LogDebug("to:          %s",info.proxy.c_str());
    LogDebug("contact:     %s",info.contact.c_str());

    //ע�᷵�� �ɷ��ͷ�ά��������ID ���շ����պ�ԭ�����ؼ���
    LogDebug("RequestId:   %d",info.sipRequestId);
    //CALL_ID
    LogDebug("CallId:      %s",info.callId.c_str());
    //����content��Ϣ
    LogDebug("body:        %s",info.content.c_str());
    //��ȡexpires
    LogDebug("expires:     %d",info.expires);
    //��Ȩ��Ϣ
    if (info.isAuthNull) {
        LogDebug("��ǰ����δ�ṩ��Ȩ��Ϣ!!!");
    } else {
        LogDebug("��ǰ���ļ�Ȩ��Ϣ����:");
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
            Log::warning("ע��Ӧ�� ����401����");
        }
    } else {
        iStatus = 200;
        int result = ::eXosip_message_build_answer(g_pExContext,info.sipRequestId,iStatus, &answer);
        if (OSIP_SUCCESS != result) {
            ::eXosip_message_send_answer(g_pExContext,info.sipRequestId, 400, NULL);
             Log::warning("ע��Ӧ�� ����400����");
        } else {
            osip_message_set_header(answer, "Contact", info.contact.c_str());
            //������Ϣ��
            ::eXosip_message_send_answer(g_pExContext,info.sipRequestId, iStatus, answer);
            Log::warning("ע��Ӧ�� ����200����");

            //�¼�ƽ̨��Ϣ
            //g_strLowCode    = info.contactCode;
            //g_strLowIP      = info.contactIP;
            //g_nLowPort      = info.contactPort;
            g_nLowExpire    = info.expires;
            g_bLowStatus    = true;

            // ��ѯĿ¼��Ϣ
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
    //��ӡ����
    printRegisterPkt(regInfo);
    //����Ӧ����
    sendRegisterAnswer(regInfo);

}

}