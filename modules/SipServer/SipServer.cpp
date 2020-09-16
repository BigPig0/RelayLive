#include "SipPrivate.h"
#include "SipServer.h"
#include "SipRegister.h"
#include "SipMessage.h"
#include "SipSubscribe.h"
#include "SipInvite.h"
#include "cJSON.h"
#include <winsock.h>
#include <list>
#include <map>

#pragma comment(lib, "Dnsapi.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "delayimp.lib")
#pragma comment(lib, "Qwave.lib")
#pragma comment(lib, "exosip.lib")

namespace SipServer {
    static CSipSubscribe* _pSubscribe;
    static CSipMessage*   _pMessage;
    static CSipInvite*    _pInvite;
    eXosip_t*             g_pExContext = NULL;

    /** Sip���������� */
    string         g_strCode;      //��ƽ̨�������
    string         g_strSipIP;     //��ƽ̨����IP
    uint32_t       g_nSipPort;     //��ƽ̨Sip�����˿�
    bool           g_bRegAuthor;   //��ƽ̨�Ƿ���ע���Ȩ

    static bool           _bSubStat;      //�Ƿ����豸״̬
    static bool           _bSubPos;       //�Ƿ����豸λ��
    static bool           _bSubPosDev;    //���豸�����ƶ��豸λ��
    static vector<string> _vecMobile;     //�ƶ��豸����
    static string         _rtpIP;        //< RTP����IP
    static list<int>      _rtpPorts;     //< RTP���ö˿ڣ�ʹ��ʱ����ȡ����ʹ�ý������·���

    // �¼�������״̬
    string         g_strLowCode;          //�¼�ƽ̨�������
    string         g_strLowIP;            //�¼�ƽ̨����IP
    uint32_t       g_nLowPort;            //�¼�ƽ̨Sip�����˿�
    bool           g_bLowStatus = false;  //�¼�ƽ̨����״̬
    uint32_t       g_nLowExpire;          //�¼�ƽ̨��ʱʱ��
    //map<string,DevInfo*> g_mapDevice;     //�¼�ƽ̨���͵��豸

    // �ص�����
    UPDATE_STATUS_CB   g_updateStatus = NULL;
    UPDATE_POSITION_CB g_updatePostion = NULL;
    ADD_DEVICE_CB      g_addDevice = NULL;
    PLAY_INIT_CB       g_playInit = NULL;
    PLAY_CB            g_playResult = NULL;

    static bool    _bFirstReg = true;      //��һ���յ�register��keeplive
    static time_t  _lastQueryTime = 0;     //��һ�β�ѯĿ¼��ʱ��
    static time_t  _lastSubscribe = 0;     //��һ�ζ��ĵ�ʱ��

    //�Ӷ˿ڱ���ȡ�����õ�rtp�˿�
    static int GetRtpPort() {
        int nRet = -1;
        if(!_rtpPorts.empty()){
            nRet = _rtpPorts.front();
            _rtpPorts.pop_front();
        }
        return nRet;
    }

    //������Ķ˿ڷ��ص��˿ڱ���
    static void GiveBackRtpPort(int nPort) {
        _rtpPorts.push_back(nPort);
    }

    //exosip�¼������߳�
    static void SeverThread()
    {
        DWORD nThreadID = GetCurrentThreadId();
        Log::debug("Sip Sever Thread ID : %d", nThreadID);

        eXosip_set_user_agent(g_pExContext, NULL);

        if (OSIP_SUCCESS != eXosip_listen_addr(g_pExContext,IPPROTO_UDP, NULL, g_nSipPort, AF_INET, 0))
        {
            Log::error("CSipSever::SeverThread eXosip_listen_addr failure.");
            return;
        }

        if (OSIP_SUCCESS != eXosip_set_option(g_pExContext, EXOSIP_OPT_SET_IPV4_FOR_GATEWAY, g_strSipIP.c_str()))
        {
            Log::error("CSipSever::SeverThread eXosip_set_option failure.");
            return;
        }

        eXosip_event_t* osipEventPtr = NULL;
        while (true)
        {
            // Wait the osip event.
            osipEventPtr = ::eXosip_event_wait(g_pExContext, 0, 200);// 0�ĵ�λ���룬200�Ǻ���
            eXosip_lock(g_pExContext);
            eXosip_automatic_action(g_pExContext);
            eXosip_unlock(g_pExContext);
            // If get nothing osip event,then continue the loop.
            if (NULL == osipEventPtr)
                continue;
            Log::warning("new sip event: %d",osipEventPtr->type);
            switch (osipEventPtr->type)
            {
            case EXOSIP_MESSAGE_NEW:
                {
                    if (!strncmp(osipEventPtr->request->sip_method, "REGISTER", 
                        strlen("REGISTER")))
                    {
                        Log::warning("recive REGISTER");
                        //OnRegister(osipEventPtr);
                        CSipRegister Register;
                        Register.OnRegister(osipEventPtr);
                    }
                    else if (!strncmp(osipEventPtr->request->sip_method, "MESSAGE",
                        strlen("MESSAGE")))
                    {
                        Log::warning("recive MESSAGE");
                        //OnMessage(osipEventPtr);
                        _pMessage->OnMessage(osipEventPtr);
                    }
                }
                break;
                //case EXOSIP_MESSAGE_ANSWERED:
                //    {
                //        Log::warning("recive message ok 200");
                //    }
                //    break;
                //case EXOSIP_SUBSCRIPTION_ANSWERED:
                //    {
                //        Log::warning("recive subscription ok 200");
                //    }
                //    break;
                //case EXOSIP_CALL_MESSAGE_ANSWERED:
                //    {
                //        Log::warning("recive call message ok 200");
                //    }
                //    break;
                //case EXOSIP_CALL_PROCEEDING:
                //    {
                //        Log::warning("recive call-trying message 100");
                //    }
                //    break;
            case EXOSIP_CALL_ANSWERED:
                {
                    Log::warning("recive call-answer message 200");
                    //OnInviteOK(osipEventPtr);
                    _pInvite->OnInviteOK(osipEventPtr);
                }
                break;
            case EXOSIP_CALL_NOANSWER:
            case EXOSIP_CALL_REQUESTFAILURE:
            case EXOSIP_CALL_SERVERFAILURE:
            case EXOSIP_CALL_GLOBALFAILURE:
                {
                    Log::warning("recive call-answer failed %d", osipEventPtr->type);
                    _pInvite->OnInviteFailed(osipEventPtr);
                }
                break;
                //case EXOSIP_CALL_MESSAGE_NEW:
                //    {
                //        Log::warning("announce new incoming request");
                //        OnCallNew(osipEventPtr);
                //    }
                //    break;
            case EXOSIP_SUBSCRIPTION_NOTIFY:
                {
                    Log::warning("recive notify");
                    //OnMessage(osipEventPtr);
                    _pMessage->OnMessage(osipEventPtr);
                }
                break;
                //case EXOSIP_CALL_CLOSED:
                //    {
                //        Log::warning("recive EXOSIP_CALL_CLOSED");
                //        OnCallClose(osipEventPtr);
                //    }
                //    break;
                //case EXOSIP_CALL_RELEASED:
                //    {
                //        Log::warning("recive EXOSIP_CALL_RELEASED");
                //        //OnCallClear(osipEventPtr);
                //    }
                //    break;
            default:
                Log::warning("The sip event type that not be precessed.the event type is : %d, %s",osipEventPtr->type, osipEventPtr->textinfo);
                break;
            }
            eXosip_event_free(osipEventPtr);
            osipEventPtr = NULL;
        } //while(true)
    }

    //��ѯ���Ĳ���
    static void SubscribeThread() {
		sleep(5000);
        time_t now = time(nullptr);
        if(_bFirstReg || difftime(now, _lastQueryTime) > 3600){ //������һ�β�ѯ����һСʱ���²�ѯ
            _bFirstReg = false;
            struct tm * timeinfo = localtime(&now);
            if(timeinfo->tm_hour == 1){ //ҹ��1��
                //DeviceMgr::CleanPlatform(); //��ջ����е����ݺ����ݿ��豸���еļ�¼
            }
            _pMessage->QueryDirtionary();
            _lastQueryTime = time(NULL);
            Log::debug(" Query dir %s",g_strCode.c_str());
            Sleep(60000); //��ʱ��֤��ѯ���ܽ����ٽ��ж���
        } 
        //�Խӵĺ���ƽ̨��exosip�Զ����͵�ˢ�¶�����Ϣ���Ƿ���ʧ�ܣ�ֻ��һֱ���ͳ�ʼ������Ϣ
        bool runSubsctibe = _bFirstReg;
#ifdef FORCE_SUBSCRIBE
        runSubsctibe |= difftime(now, _lastSubscribe) > 600;
        if(runSubsctibe)
            _lastSubscribe = time(NULL);
#endif
        if(runSubsctibe && _bSubStat) {
            _pSubscribe->SubscribeDirectory(600);
            Log::debug(" Subscribe dir %s",g_strCode.c_str());
            Sleep(10000);
        }
        if(runSubsctibe && _bSubPos) {
            _pSubscribe->SubscribeMobilepostion(600);
            Log::debug(" Subscribe mobile pos %s",g_strCode.c_str());
        }
        if(runSubsctibe && _bSubPosDev){
            //vector<DevInfo*> devInfo = DeviceMgr::GetDeviceInfo();
            //vector<string> devs;
            //for(auto info:devInfo){
            //    auto ret = std::find(_vecMobile.begin(), _vecMobile.end(), info->strParentID);
            //    if(ret != _vecMobile.end())
            //        devs.push_back(info->strDevID);
            //}
            _pSubscribe->SubscribeMobilepostion(600, _vecMobile);
            Log::debug(" Subscribe all mobile pos %s", g_strCode.c_str());
        }
    }

    //�¼�ƽ̨��ʱ���
    static void ExpireThread(){
        while (true)
        {
            if(g_bLowStatus) {
                g_nLowExpire -= 10;
                if(g_nLowExpire <= 0)
                    g_bLowStatus = false;
            }
            sleep(10000);
        }
    }

    // ���յ�ע��򱣻�ʱ���Զ�����Ŀ¼��ѯ���Ĳ���
    void AutoQuery() {
        thread th([](){
            SubscribeThread();
        });
        th.detach();
    }

    string FormatDevInfo(DevInfo *dev, bool ansi2utf8) {
        string strResJson = "{";
#if 1
        if (!dev->strDevID.empty())
        {
            strResJson += "\"DeviceID\":\"";
            strResJson += dev->strDevID;
            strResJson += "\",";
        }
        if (!dev->strName.empty())
        {
            strResJson += "\"Name\":\"";
            if(ansi2utf8)
                strResJson += EncodeConvert::AtoUTF8(dev->strName);
            else
                strResJson += dev->strName;
            strResJson += "\",";
        }
        if (!dev->strManuf.empty())
        {
            strResJson += "\"Manufacturer\":\"";
            if(ansi2utf8)
                strResJson += EncodeConvert::AtoUTF8(dev->strManuf);
            else
                strResJson += dev->strManuf;
            strResJson += "\",";
        }
        if (!dev->strModel.empty())
        {
            strResJson += "\"Model\":\"";
            if(ansi2utf8)
                strResJson += EncodeConvert::AtoUTF8(dev->strModel);
            else
                strResJson += dev->strModel;
            strResJson += "\",";
        }
        if (!dev->strOwner.empty())
        {
            strResJson += "\"Owner\":\"";
            strResJson += dev->strOwner;
            strResJson += "\",";
        }
        if (!dev->strCivilCode.empty())
        {
            strResJson += "\"CivilCode\":\"";
            strResJson += dev->strCivilCode;
            strResJson += "\",";
        }
        if (!dev->strBlock.empty())
        {
            strResJson += "\"Block\":\"";
            strResJson += dev->strBlock;
            strResJson += "\",";
        }
        if (!dev->strAddress.empty())
        {
            strResJson += "\"Address\":\"";
            if(ansi2utf8)
                strResJson += EncodeConvert::AtoUTF8(dev->strAddress);
            else
                strResJson += dev->strAddress;
            strResJson += "\",";
        }
        if (!dev->strParental.empty())
        {
            strResJson += "\"Parental\":\"";
            strResJson += dev->strParental;
            strResJson += "\",";
        }
        if (!dev->strParentID.empty())
        {
            strResJson += "\"ParentID\":\"";
            strResJson += dev->strParentID;
            strResJson += "\",";
        }
        if (!dev->strSafetyWay.empty())
        {
            strResJson += "\"SafetyWay\":\"";
            strResJson += dev->strSafetyWay;
            strResJson += "\",";
        }
        if (!dev->strRegisterWay.empty())
        {
            strResJson += "\"RegisterWay\":\"";
            strResJson += dev->strRegisterWay;
            strResJson += "\",";
        }
        if (!dev->strCertNum.empty())
        {
            strResJson += "\"CertNum\":\"";
            strResJson += dev->strCertNum;
            strResJson += "\",";
        }
        if (!dev->strCertifiable.empty())
        {
            strResJson += "\"Certifiable\":\"";
            strResJson += dev->strCertifiable;
            strResJson += "\",";
        }
        if (!dev->strErrCode.empty())
        {
            strResJson += "\"ErrCode\":\"";
            strResJson += dev->strErrCode;
            strResJson += "\",";
        }
        if (!dev->strEndTime.empty())
        {
            strResJson += "\"EndTime\":\"";
            strResJson += dev->strEndTime;
            strResJson += "\",";
        }
        if (!dev->strSecrecy.empty())
        {
            strResJson += "\"Secrecy\":\"";
            strResJson += dev->strSecrecy;
            strResJson += "\",";
        }
        if (!dev->strStatus.empty())
        {
            strResJson += "\"Status\":\"";
            strResJson += dev->strStatus;
            strResJson += "\",";
        }
        if (!dev->strIPAddress.empty())
        {
            strResJson += "\"IPAddress\":\"";
            strResJson += dev->strIPAddress;
            strResJson += "\",";
        }
        if (!dev->strPort.empty())
        {
            strResJson += "\"Port\":\"";
            strResJson += dev->strPort;
            strResJson += "\",";
        }
        if (!dev->strPassword.empty())
        {
            strResJson += "\"Password\":\"";
            strResJson += dev->strPassword;
            strResJson += "\",";
        }
        if (!dev->strLongitude.empty())
        {
            strResJson += "\"Longitude\":\"";
            strResJson += dev->strLongitude;
            strResJson += "\",";
        }
        if (!dev->strLatitude.empty())
        {
            strResJson += "\"Latitude\":\"";
            strResJson += dev->strLatitude;
            strResJson += "\",";
        }
        if (!dev->strPTZType.empty())
        {
            strResJson += "\"PTZType\":\"";
            strResJson += dev->strPTZType;
            strResJson += "\",";
        }
        if (!dev->strPositionType.empty())
        {
            strResJson += "\"PositionType\":\"";
            strResJson += dev->strPositionType;
            strResJson += "\",";
        }
        if (!dev->strRoomType.empty())
        {
            strResJson += "\"RoomType\":\"";
            strResJson += dev->strRoomType;
            strResJson += "\",";
        }
        if (!dev->strUseType.empty())
        {
            strResJson += "\"UseType\":\"";
            strResJson += dev->strUseType;
            strResJson += "\",";
        }
        if (!dev->strSupplyLightType.empty())
        {
            strResJson += "\"SupplyLightType\":\"";
            strResJson += dev->strSupplyLightType;
            strResJson += "\",";
        }
        if (!dev->strDirectionType.empty())
        {
            strResJson += "\"DirectionType\":\"";
            strResJson += dev->strDirectionType;
            strResJson += "\",";
        }
        if (!dev->strResolution.empty())
        {
            strResJson += "\"Resolution\":\"";
            strResJson += dev->strResolution;
            strResJson += "\",";
        }
        if (!dev->strBusinessGroupID.empty())
        {
            strResJson += "\"BusinessGroupID\":\"";
            strResJson += dev->strBusinessGroupID;
            strResJson += "\",";
        }
        if (!dev->strDownloadSpeed.empty())
        {
            strResJson += "\"DownloadSpeed\":\"";
            strResJson += dev->strDownloadSpeed;
            strResJson += "\",";
        }
        if (!dev->strSVCSpaceSupportType.empty())
        {
            strResJson += "\"SVCSpaceSupportMode\":\"";
            strResJson += dev->strSVCSpaceSupportType;
            strResJson += "\",";
        }
        if (!dev->strSVCTimeSupportType.empty())
        {
            strResJson += "\"SVCTimeSupportMode\":\"";
            strResJson += dev->strSVCTimeSupportType;
            strResJson += "\",";
        }
#endif
        strResJson = StringHandle::StringTrimRight(strResJson,',');
        strResJson += "}";

        return strResJson;
    }

    void TransDevInfo(string json, DevInfo *dev, bool utf82ansi) {
        cJSON* jroot = cJSON_Parse(json.c_str());
        if(jroot == NULL)
            return;
        for (cJSON *attr = jroot->child; attr; attr = attr->next){
            if(!strcmp(attr->string, "DeviceID")){
                dev->strDevID = attr->valuestring;
            } else if(!strcmp(attr->string, "Name")){
                if(utf82ansi)
                    dev->strName = EncodeConvert::UTF8toA(attr->valuestring);
                else
                    dev->strName = attr->valuestring;
            } else if(!strcmp(attr->string, "Manufacturer")){
                if(utf82ansi)
                    dev->strManuf = EncodeConvert::UTF8toA(attr->valuestring);
                else
                    dev->strManuf = attr->valuestring;
            } else if(!strcmp(attr->string, "Model")){
                if(utf82ansi)
                    dev->strModel = EncodeConvert::UTF8toA(attr->valuestring);
                else
                    dev->strModel = attr->valuestring;
            } else if(!strcmp(attr->string, "Owner")){
                dev->strOwner = attr->valuestring;
            } else if(!strcmp(attr->string, "CivilCode")){
                dev->strCivilCode = attr->valuestring;
            } else if(!strcmp(attr->string, "Block")){
                dev->strBlock = attr->valuestring;
            } else if(!strcmp(attr->string, "Address")){
                if(utf82ansi)
                    dev->strAddress = EncodeConvert::UTF8toA(attr->valuestring);
                else
                    dev->strAddress = attr->valuestring;
            } else if(!strcmp(attr->string, "Parental")){
                dev->strParental = attr->valuestring;
            } else if(!strcmp(attr->string, "ParentID")){
                dev->strParentID = attr->valuestring;
            } else if(!strcmp(attr->string, "SafetyWay")){
                dev->strSafetyWay = attr->valuestring;
            } else if(!strcmp(attr->string, "RegisterWay")){
                dev->strRegisterWay = attr->valuestring;
            } else if(!strcmp(attr->string, "CertNum")){
                dev->strCertNum = attr->valuestring;
            } else if(!strcmp(attr->string, "Certifiable")){
                dev->strCertifiable = attr->valuestring;
            } else if(!strcmp(attr->string, "ErrCode")){
                dev->strErrCode = attr->valuestring;
            } else if(!strcmp(attr->string, "EndTime")){
                dev->strEndTime = attr->valuestring;
            } else if(!strcmp(attr->string, "Secrecy")){
                dev->strSecrecy = attr->valuestring;
            } else if(!strcmp(attr->string, "Status")){
                dev->strStatus = attr->valuestring;
            } else if(!strcmp(attr->string, "IPAddress")){
                dev->strIPAddress = attr->valuestring;
            } else if(!strcmp(attr->string, "Port")){
                dev->strPort = attr->valuestring;
            } else if(!strcmp(attr->string, "Password")){
                dev->strPassword = attr->valuestring;
            } else if(!strcmp(attr->string, "Longitude")){
                dev->strLongitude = attr->valuestring;
            } else if(!strcmp(attr->string, "Latitude")){
                dev->strLatitude = attr->valuestring;
            } else if(!strcmp(attr->string, "PTZType")){
                dev->strPTZType = attr->valuestring;
            } else if(!strcmp(attr->string, "PositionType")){
                dev->strPositionType = attr->valuestring;
            } else if(!strcmp(attr->string, "RoomType")){
                dev->strRoomType = attr->valuestring;
            } else if(!strcmp(attr->string, "UseType")){
                dev->strUseType = attr->valuestring;
            } else if(!strcmp(attr->string, "SupplyLightType")){
                dev->strSupplyLightType = attr->valuestring;
            } else if(!strcmp(attr->string, "DirectionType")){
                dev->strDirectionType = attr->valuestring;
            } else if(!strcmp(attr->string, "Resolution")){
                dev->strResolution = attr->valuestring;
            } else if(!strcmp(attr->string, "BusinessGroupID")){
                dev->strBusinessGroupID = attr->valuestring;
            } else if(!strcmp(attr->string, "DownloadSpeed")){
                dev->strDownloadSpeed = attr->valuestring;
            } else if(!strcmp(attr->string, "SVCSpaceSupportMode")){
                dev->strSVCSpaceSupportType = attr->valuestring;
            } else if(!strcmp(attr->string, "SVCTimeSupportMode")){
                dev->strSVCTimeSupportType = attr->valuestring;
            } 
        }
    }

    bool Init()
    {
        // SIP����
        g_strCode       = Settings::getValue("SipSever","Code");
        g_strSipIP      = Settings::getValue("SipSever","IP");
        g_nSipPort      = Settings::getValue("SipSever","Port", 5060);
        g_bRegAuthor    = Settings::getValue("SipSever","RegAuthor",0)>0;
		g_strLowCode    = Settings::getValue("PlatFormInfo","Code");
		g_strLowIP      = Settings::getValue("PlatFormInfo","IP");
		g_nLowPort      = Settings::getValue("PlatFormInfo","Port", 5060);
		g_nLowExpire    = 9999999;
        _bSubStat       = Settings::getValue("PlatFormInfo","SubscribeStatus",0)>0;
        _bSubPos        = Settings::getValue("PlatFormInfo","SubscribePos",0)>0;
        _bSubPosDev     = Settings::getValue("PlatFormInfo","SubscribePosDev",0)>0;
        if(_bSubPosDev) {
            string strMobile = Settings::getValue("PlatFormInfo","SubscribePosDepart");
            _vecMobile      = StringHandle::StringSplit(strMobile, ',');
        }
        Log::debug("SipSever Code:%s IP:%s Port:%d RegAuthor:%d Subscribe[status:%d position:%d Dev position:%d]",
            g_strCode.c_str(), g_strSipIP.c_str(), g_nSipPort, g_bRegAuthor, _bSubStat, _bSubPos, _bSubPosDev);

        // RTP����
        _rtpIP           = Settings::getValue("RtpClient","IP");                    //< RTP����IP
        int beginPort    = Settings::getValue("RtpClient","BeginPort",10000);       //< RTP��������ʼ�˿ڣ�������ż��
        int portNum      = Settings::getValue("RtpClient","PortNum",1000);          //< RTPʹ�õĸ�������strRTPPort��ʼÿ�μ�2����strRTPNum��

        Log::debug("RtpConfig IP:%s, BeginPort:%d,PortNum:%d", _rtpIP.c_str(), beginPort, portNum);
        _rtpPorts.clear();
        for (int i=0; i<portNum; ++i) {
            _rtpPorts.push_back(beginPort+i*2);
        }

        //osip���ʼ��
        g_pExContext = eXosip_malloc();
        if (nullptr == g_pExContext){
            Log::error("eXosip_malloc failed");
            return false;
        }

        int result = OSIP_SUCCESS;
        result = eXosip_init(g_pExContext);
        if (OSIP_SUCCESS != result){
            Log::error("eXosip_init failed");
            return false;
        }

        // ����֪ͨ
        _pSubscribe = new CSipSubscribe();
        if (nullptr == _pSubscribe) {
            Log::error("Subscribe new failed");
            return false;
        }

        // ��Ϣ
        _pMessage = new CSipMessage();
        if (nullptr == _pMessage){
            Log::error("SipMessage new failed");
            return false;
        }

        // �Ự����
        _pInvite = new CSipInvite();
        if (nullptr == _pInvite){
            Log::error("SipInvite new failed");
            return false;
        }

        // exosip����
        thread th([](){
            SeverThread();
        });
        th.detach();

        //�¼�ƽ̨��ʱ���
        thread tt([](){
            ExpireThread();
        });
        tt.detach();

        return true;
    }

    void Cleanup()
    {
        SAFE_DELETE(_pSubscribe);
    }

    bool PlayInit(string strProName, uint32_t nID, string strDev)
    {
        Log::debug("init real play strDev:%s", strDev.c_str());

        // rtp�ͻ��˶˿�
        int nPort = GetRtpPort(); 
        if(nPort == -1) {
            Log::error("failed get rtp port");
            if(g_playInit)
                g_playInit(strProName, nID, nPort);
            return false;
        }

        // �����Ự����
        _pInvite->InviteInit(strProName, nID, strDev, nPort);

        if(g_playInit)
            g_playInit(strProName, nID, nPort);

        return true;
    }

    bool RealPlay(string strProName, uint32_t nID, uint32_t nPort)
    {
        Log::debug("start real play port:%d", nPort);

        // �����Ự����
        if (!_pInvite->SendInvite(strProName, nID, nPort))
        {
            Log::error("creat sip call failed");
            return false;
        }
        return true;
    }

    bool RecordPlay(string strProName, uint32_t nID, uint32_t nPort, string startTime, string endTime)
    {
        Log::debug("start SipInstance::RealPlay port:%d", nPort);

        // �����Ự����
        if (!_pInvite->SendRecordInvite(strProName, nID, nPort, startTime, endTime)) {
            return false;
        }
        return true;
    }

    bool StopPlay(uint32_t rtpPort)
    {
        Log::debug("stop %d", rtpPort);
        _pInvite->StopSipCall(rtpPort);
        GiveBackRtpPort(rtpPort);

        return true;
    }

    bool StopPlayAll(string strProName)
    {
        Log::debug("stop all %s", strProName.c_str());
        vector<uint32_t> ports = _pInvite->StopSipCallAll(strProName);
        for(auto &port:ports) {
            GiveBackRtpPort(port);
        }

        return true;
    }

    bool DeviceControl(string strDev, int nInOut, int nUpDown, int nLeftRight)
    {
        _pMessage->DeviceControl(strDev, nInOut, nUpDown, nLeftRight);

        return true;
    }

    bool QueryDirtionary()
    {
        _pMessage->QueryDirtionary();
        return true;
    }

    void SetUpdateStatusCB(UPDATE_STATUS_CB cb) {
        g_updateStatus = cb;
    }

    void SetUpdatePostionCB(UPDATE_POSITION_CB cb) {
        g_updatePostion = cb;
    }

    void SetDeviceCB(ADD_DEVICE_CB cb) {
        g_addDevice = cb;
    }

    void SetInitCB(PLAY_INIT_CB cb) {
        g_playInit = cb;
    }

    void SetPlayCB(PLAY_CB cb) {
        g_playResult = cb;
    }
};