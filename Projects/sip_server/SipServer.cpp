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

    /** Sip服务器配置 */
    string         g_strCode;      //本平台国标编码
    string         g_strSipIP;     //本平台本地IP
    uint32_t       g_nSipPort;     //本平台Sip监听端口
    bool           g_bRegAuthor;   //本平台是否开启注册鉴权

    static bool           _bSubStat;      //是否订阅设备状态
    static bool           _bSubPos;       //是否订阅设备位置
    static bool           _bSubPosDev;    //按设备订阅移动设备位置
    static vector<string> _vecMobile;     //移动设备部门
    static string         _rtpIP;        //< RTP服务IP
    static list<int>      _rtpPorts;     //< RTP可用端口，使用时从中取出，使用结束重新放入

    // 下级服务器状态
    string         g_strLowCode;          //下级平台国标编码
    string         g_strLowIP;            //下级平台本地IP
    uint32_t       g_nLowPort;            //下级平台Sip监听端口
    bool           g_bLowStatus = false;  //下级平台在线状态
    uint32_t       g_nLowExpire;          //下级平台超时时间
    //map<string,DevInfo*> g_mapDevice;     //下级平台推送的设备

    static bool    _bFirstReg = true;      //第一次收到register或keeplive
    static bool    _bSubscribe = false;    // 是否需要执行订阅
    static time_t  _lastQueryTime = 0;     //上一次查询目录的时间
    static time_t  _lastSubscribe = 0;     //上一次订阅的时间

    //从端口表中取出可用的rtp端口
    static int GetRtpPort() {
        int nRet = -1;
        if(!_rtpPorts.empty()){
            nRet = _rtpPorts.front();
            _rtpPorts.pop_front();
        }
        return nRet;
    }

    //将用完的端口返回到端口表中
    static void GiveBackRtpPort(int nPort) {
        _rtpPorts.push_back(nPort);
    }

    //exosip事件处理线程
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
            osipEventPtr = ::eXosip_event_wait(g_pExContext, 0, 200);// 0的单位是秒，200是毫秒
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

    //查询订阅操作
    static void SubscribeThread() {
        bool isFirst = _bFirstReg;
        _bFirstReg = false;
        time_t now = time(nullptr);

        if(isFirst || difftime(now, _lastQueryTime) > 3600){ //距离上一次查询超过一小时重新查询
            on_clean_everyday(now);
            _pMessage->QueryDirtionary();
            _lastQueryTime = time(NULL);
            Log::debug(" Query dir %s",g_strCode.c_str());
            Sleep(60000); //延时保证查询接受结束再进行订阅
            _bSubscribe = true;
        } 
        
#ifdef FORCE_SUBSCRIBE
        //正常情况下，订阅只需要一次，exosip自己维护刷新
        //对接的海康平台，exosip自动发送的刷新订阅消息总是返回失败，只能一直发送初始订阅消息
        isFirst |= difftime(now, _lastSubscribe) > 600;
        if(isFirst)
            _lastSubscribe = time(NULL);
#endif
        //第一次查询完成后,再开始订阅流程，避免日志混乱
        if(_bSubscribe && isFirst) {
            if(_bSubStat) {
                _pSubscribe->SubscribeDirectory(600);
                Log::debug(" Subscribe dir %s",g_strCode.c_str());
                Sleep(10000);
            }
            if(_bSubPos) {
                _pSubscribe->SubscribeMobilepostion(600);
                Log::debug(" Subscribe mobile pos %s",g_strCode.c_str());
            }
            if(_bSubPosDev){
                //只需要订阅指定目录下的设备的gps
                vector<string> devs;
                MutexLock lock(&g_csDevs);
                for(auto info:g_mapDevs){
                    auto ret = std::find(_vecMobile.begin(), _vecMobile.end(), info.second->strParentID);
                    if(ret != _vecMobile.end())
                        devs.push_back(info.second->strDevID);
                }
                _pSubscribe->SubscribeMobilepostion(600, devs);
                Log::debug(" Subscribe all mobile pos %s", g_strCode.c_str());
            }
        }
    }

    //下级平台超时检测
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

    // 当收到注册或保活时，自动进行目录查询或订阅操作
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
        // SIP配置
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

        // RTP配置
        _rtpIP           = Settings::getValue("RtpClient","IP");                    //< RTP服务IP
        int beginPort    = Settings::getValue("RtpClient","BeginPort",10000);       //< RTP监听的起始端口，必须是偶数
        int portNum      = Settings::getValue("RtpClient","PortNum",1000);          //< RTP使用的个数，从strRTPPort开始每次加2，共strRTPNum个

        Log::debug("RtpConfig IP:%s, BeginPort:%d,PortNum:%d", _rtpIP.c_str(), beginPort, portNum);
        _rtpPorts.clear();
        for (int i=0; i<portNum; ++i) {
            _rtpPorts.push_back(beginPort+i*2);
        }

        //osip库初始化
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

        // 订阅通知
        _pSubscribe = new CSipSubscribe();
        if (nullptr == _pSubscribe) {
            Log::error("Subscribe new failed");
            return false;
        }

        // 消息
        _pMessage = new CSipMessage();
        if (nullptr == _pMessage){
            Log::error("SipMessage new failed");
            return false;
        }

        // 会话邀请
        _pInvite = new CSipInvite();
        if (nullptr == _pInvite){
            Log::error("SipInvite new failed");
            return false;
        }

        // exosip服务
        thread th([](){
            SeverThread();
        });
        th.detach();

        //下级平台超时检测
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

        // rtp客户端端口
        int nPort = GetRtpPort(); 
        if(nPort == -1) {
            Log::error("failed get rtp port");
            IPC::on_play_init_cb(strProName, nID, nPort);
            return false;
        }

        // 创建会话邀请
        _pInvite->InviteInit(strProName, nID, strDev, nPort);

        IPC::on_play_init_cb(strProName, nID, nPort);

        return true;
    }

    bool RealPlay(string strProName, uint32_t nID, uint32_t nPort)
    {
        Log::debug("start real play port:%d", nPort);

        // 创建会话邀请
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

        // 创建会话邀请
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
};