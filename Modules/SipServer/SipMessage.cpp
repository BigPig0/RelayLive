#include "SipPrivate.h"
#include "SipMessage.h"
#include "SipServer.h"
#include "pugixml.hpp"
#include <string>
#include <map>
using namespace SipServer;
using namespace std;

struct msgPublic
{
    string strCmdType;  //< 命令类型
    string strSN;       //< 命令序列号 
    string strDeviceID; //< 源设备/系统编码
};

/** 通知命令 */
struct msgNotify : public msgPublic
{
};

/** 应答命令 */
struct msgResponse : public msgPublic
{
};

/** 状态信息报送 */
struct msgKeepAlive : public msgNotify
{
    string         strStatus;   //< 是否正常工作
    vector<string> vecDeviceID; //< 故障设备列表
};

/** 设备目录查询信息应答 */
struct msgDevDirQuery : public msgResponse
{
    string            strSumNum;   //< 查询结果总数
    vector<DevInfo*>  vecDevInfo;  //< 设备列表
};

struct SipMessageInfo
{
    int    requestId;              //Sip层返回的请求的标志 响应时返回即可
    string callId;                 //维护一次注册
    string method;                 //消息所属的功能方法名字符串
    string from;                   //地址编码@域名或IP地址:连接端口，例如sip:1111@127.0.0.1:5060
    string to;                     //地址编码@域名或IP地址:连接端口，例如sip:1111@127.0.0.1:5060
    string content;                // 消息体
    string strCmdType;             // 消息类型
    map<string,string> mapBody;    // 消息体解析
    msgPublic* pMsgBody;
};

//////////////////////////////////////////////////////////////////////////

/**
 * 解析状态信息报送
 */
static msgPublic* ParseKeepAlive(pugi::xml_node& root)
{
    msgKeepAlive *pRet = new msgKeepAlive;
    string nodeName;
    string nodeValue;

    LogDebug("<%s>",root.name());
    //遍历历子节点
    for (pugi::xml_node node = root.first_child(); node; node = node.next_sibling())
    {
        nodeName  = node.name();
        if (nodeName == "CmdType") {
            pRet->strCmdType = node.child_value();
            LogDebug("<%s>%s</%s>",nodeName.c_str(),node.child_value(),nodeName.c_str());
        } else if(nodeName == "SN") {
            pRet->strSN = node.child_value();
            LogDebug("<%s>%s</%s>",nodeName.c_str(),node.child_value(),nodeName.c_str());
        } else if (nodeName == "DeviceID") {
            pRet->strDeviceID = node.child_value();
            LogDebug("<%s>%s</%s>",nodeName.c_str(),node.child_value(),nodeName.c_str());
        } else if (nodeName == "Status") {
            pRet->strStatus = node.child_value();
            LogDebug("<%s>%s</%s>",nodeName.c_str(),node.child_value(),nodeName.c_str());
        } else if (!nodeName.empty()) {
            Log::error("%s uncatch", nodeName.c_str());
        }

    }
    LogDebug("</%s>",root.name());

    // 平台保活
    g_nLowExpire = 120;
    return pRet;
}

/**
 * 解析设备目录信息查询报文
 */
static msgPublic* ParseCatalog(pugi::xml_node& root)
{
    msgDevDirQuery *pRet = new msgDevDirQuery;
    string nodeName;
    string nodeValue;

    LogDebug("<%s>",root.name());
    //遍历历子节点
    for (pugi::xml_node node = root.first_child(); node; node = node.next_sibling())
    {
        nodeName  = node.name();
        if(nodeName != "DeviceList") {
            LogDebug("<%s>%s</%s>",nodeName.c_str(),node.child_value(),nodeName.c_str());
        }
        if (nodeName == "CmdType") {
            pRet->strCmdType = node.child_value();
        } else if(nodeName == "SN") {
            pRet->strSN = node.child_value();
        } else if (nodeName == "DeviceID") {
            pRet->strDeviceID = node.child_value();
        } else if (nodeName == "SumNum") {
            pRet->strSumNum = node.child_value();
        } else if (nodeName == "DeviceList") {
            stringstream ss;
            ss << "<" << nodeName;
            pugi::xml_attribute attr = node.attribute("Num");
            if(!attr.empty()) {
                int nNum = attr.as_int();
                ss << " Num=" << nNum;
            }
            ss << ">";
            LogDebug(ss.str().c_str());

            for (pugi::xml_node itemNode = node.first_child(); itemNode; itemNode=itemNode.next_sibling())
            {
                LogDebug("<Item>");
                DevInfo* pDevCtrl = new DevInfo;
                DevInfo& dev = *pDevCtrl;
                for (pugi::xml_node devNode = itemNode.first_child(); devNode; devNode=devNode.next_sibling())
                {
                    nodeName = devNode.name();
                    if(nodeName != "Info") {
                        LogDebug("<%s>%s</%s>",nodeName.c_str(),devNode.child_value(),nodeName.c_str());
                    }
                    if (nodeName == "DeviceID") {
                        dev.strDevID = devNode.child_value();
                    } else if (nodeName == "Name") {
                        dev.strName = devNode.child_value();
                    } else if (nodeName == "Manufacturer") {
                        dev.strManuf = devNode.child_value();
                    } else if (nodeName == "Model") {
                        dev.strModel = devNode.child_value();
                    } else if (nodeName == "Owner") {
                        dev.strOwner = devNode.child_value();
                    } else if (nodeName == "CivilCode") {
                        dev.strCivilCode = devNode.child_value();
                    } else if (nodeName == "Block") {
                        dev.strBlock = devNode.child_value();
                    } else if (nodeName == "Address") {
                        dev.strAddress = devNode.child_value();
                    } else if (nodeName == "Parental") {
                        dev.strParental = devNode.child_value();
                    } else if (nodeName == "ParentID") {
                        dev.strParentID = devNode.child_value();
                    } else if (nodeName == "SafetyWay") {
                        dev.strSafetyWay = devNode.child_value();
                    } else if (nodeName == "RegisterWay") {
                        dev.strRegisterWay = devNode.child_value();
                    } else if (nodeName == "CertNum") {
                        dev.strCertNum = devNode.child_value();
                    } else if (nodeName == "Certifiable") {
                        dev.strCertifiable = devNode.child_value();
                    } else if (nodeName == "ErrCode") {
                        dev.strErrCode = devNode.child_value();
                    } else if (nodeName == "EndTime") {
                        dev.strEndTime = devNode.child_value();
                    } else if (nodeName == "Secrecy") {
                        dev.strSecrecy = devNode.child_value();
                    } else if (nodeName == "Status") {
                        dev.strStatus = devNode.child_value();
                    } else if (nodeName == "IPAddress") {
                        dev.strIPAddress = devNode.child_value();
                    } else if (nodeName == "Port") {
                        dev.strPort = devNode.child_value();
                    } else if (nodeName == "Password") {
                        dev.strPassword = devNode.child_value();
                    } else if (nodeName == "Longitude") {
                        dev.strLongitude = devNode.child_value();
                    } else if (nodeName == "Latitude") {
                        dev.strLatitude = devNode.child_value();
                    } else if (nodeName == "BusinessGroupID") {
                        dev.strBusinessGroupID = devNode.child_value();
                    } else if (nodeName == "Info") {
                        LogDebug("<Info>");
                        for (pugi::xml_node infoNode = devNode.first_child(); infoNode; infoNode=infoNode.next_sibling())
                        {
                            nodeName = infoNode.name();
                            LogDebug("<%s>%s</%s>",nodeName.c_str(),infoNode.child_value(),nodeName.c_str());

                            if (nodeName == "PTZType") {
                                dev.strPTZType = infoNode.child_value();
                            } else if (nodeName == "PositionType") {
                                dev.strPositionType = infoNode.child_value();
                            } else if (nodeName == "RoomType") {
                                dev.strRoomType = infoNode.child_value();
                            } else if (nodeName == "UseType") {
                                dev.strUseType = infoNode.child_value();
                            } else if (nodeName == "SupplyLightType") {
                                dev.strSupplyLightType = infoNode.child_value();
                            } else if (nodeName == "DirectionType") {
                                dev.strDirectionType = infoNode.child_value();
                            } else if (nodeName == "Resolution") {
                                dev.strResolution = infoNode.child_value();
                            } else if (nodeName == "BusinessGroupID") {
                                dev.strBusinessGroupID = infoNode.child_value();
                            } else if (nodeName == "DownloadSpeed") {
                                dev.strDownloadSpeed = infoNode.child_value();
                            } else if (nodeName == "SVCSpaceSupportMode") {
                                dev.strSVCSpaceSupportType = infoNode.child_value();
                            } else if (nodeName == "SVCTimeSupportMode") {
                                dev.strSVCTimeSupportType = infoNode.child_value();
                            } else {
                                Log::error("node[%s] is not catched",nodeName.c_str());
                            }
                        }// for Info Node
                        LogDebug("</Info>");
                    }// Info 
                    else {
                        Log::error("node[%s] is not catched [%s]",nodeName.c_str(), devNode.child_value());
                    }
                }// for Item Node
                LogDebug("</Item>");

                pRet->vecDevInfo.push_back(pDevCtrl);
                if(g_addDevice)
                    g_addDevice(pDevCtrl);
            } // for DeviceList Node

            LogDebug("</DeviceList>");
        }// DeviceList
    }// for root Node
    LogDebug("</%s>",root.name());
    return pRet;
}

/**
 * 目录信息订阅后收到的推送报文
 */
static msgPublic* ParseNotifyCatalog(pugi::xml_node& root)
{
    msgDevDirQuery *pRet = new msgDevDirQuery;
    string nodeName;
    string nodeValue;

    LogDebug("<%s>",root.name());
    //遍历历子节点
    for (pugi::xml_node node = root.first_child(); node; node = node.next_sibling())
    {
        nodeName  = node.name();
        if(nodeName != "DeviceList") {
            LogDebug("<%s>%s</%s>",nodeName.c_str(),node.child_value(),nodeName.c_str());
        }
        if (nodeName == "CmdType") {
            pRet->strCmdType = node.child_value();
        } else if(nodeName == "SN") {
            pRet->strSN = node.child_value();
        } else if (nodeName == "DeviceID") {
            pRet->strDeviceID = node.child_value();
        } else if (nodeName == "SumNum") {
            pRet->strSumNum = node.child_value();
        } else if (nodeName == "DeviceList") {
            stringstream ss;
            ss << "<" << nodeName;
            pugi::xml_attribute attr = node.attribute("Num");
            if(!attr.empty()) {
                int nNum = attr.as_int();
                ss << " Num=" << nNum;
            }
            ss << ">";
            LogDebug(ss.str().c_str());

            for (pugi::xml_node itemNode = node.first_child(); itemNode; itemNode=itemNode.next_sibling())
            {
                LogDebug("<Item>");
                DevInfo* pDevCtrl = new DevInfo;
                DevInfo& dev = *pDevCtrl;
                for (pugi::xml_node devNode = itemNode.first_child(); devNode; devNode=devNode.next_sibling())
                {
                    nodeName = devNode.name();
                    if(nodeName != "Info") {
                        LogDebug("<%s>%s</%s>",nodeName.c_str(),devNode.child_value(),nodeName.c_str());
                    } 
                    if (nodeName == "DeviceID") {
                        dev.strDevID = devNode.child_value();
                    } else if (nodeName == "Status") {
                        dev.strStatus = devNode.child_value();
                    } else if (nodeName == "Longitude") {
                        dev.strLongitude = devNode.child_value();
                    } else if (nodeName == "Latitude") {
                        dev.strLatitude = devNode.child_value();
                    }
                }// for Item Node
                LogDebug("</Item>");

                pRet->vecDevInfo.push_back(pDevCtrl);
            } // for DeviceList Node

            LogDebug("</DeviceList>");
        }// DeviceList
    }// for root Node
    LogDebug("</%s>",root.name());
    return pRet;
}

/**
 * 位置信息订阅后收到的推送报文
 */
static msgPublic* ParseMobilePosition(pugi::xml_node& root)
{
    msgDevDirQuery *pRet = new msgDevDirQuery;
    string nodeName;
    string nodeValue;

    LogDebug("<%s>",root.name());
    DevInfo* pDevCtrl = new DevInfo;
    DevInfo& dev = *pDevCtrl;
    //遍历历子节点
    for (pugi::xml_node node = root.first_child(); node; node = node.next_sibling())
    {
        nodeName  = node.name();
        if (nodeName == "CmdType") {
            pRet->strCmdType = node.child_value();
        } else if(nodeName == "SN") {
            pRet->strSN = node.child_value();
        } else if (nodeName == "DeviceID") {
            dev.strDevID = node.child_value();
        } else if (nodeName == "Longitude") {
            dev.strLongitude = node.child_value();
        } else if (nodeName == "Latitude") {
            dev.strLatitude = node.child_value();
        }
    }
    pRet->vecDevInfo.push_back(pDevCtrl);
    return pRet;
}

/**
 * 通知类信息解析
 * @param ppMsg[out] 输出解析后的结构体
 * @param root[in]   输入xml根节点
 * @return 报文命令类型CmdType
 */
static string ParseNotify(msgPublic** ppMsg, pugi::xml_node& root)
{
    // 查找CmdType节点
    pugi::xml_node nodeCmdType = root.child("CmdType");
    if (!nodeCmdType) {
        Log::error("not find CmdType");
        return "";
    }

    string strCmdType = nodeCmdType.child_value();
    if (strCmdType == "Keepalive"){
        Log::debug("this is a keepalive message");
        *ppMsg = ParseKeepAlive(root);
        return strCmdType;
    } else if(strCmdType == "Catalog") {
        Log::debug("this is a Catalog message");
        *ppMsg = ParseNotifyCatalog(root);
        msgDevDirQuery* pInfo = (msgDevDirQuery*)*ppMsg;
        for(auto& dev:pInfo->vecDevInfo) {
            if(g_updateStatus)
                g_updateStatus(dev->strDevID, dev->strStatus=="ON");
        }
    } else if(strCmdType == "MobilePosition") {
        Log::debug("this is a MobilePosition message");
        *ppMsg = ParseMobilePosition(root);
        msgDevDirQuery* pInfo = (msgDevDirQuery*)*ppMsg;
        for(auto& dev:pInfo->vecDevInfo) {
            if(g_updatePostion) {
                g_updatePostion(dev->strDevID, stod(dev->strLongitude), stod(dev->strLatitude));
            }
        }
    } else {
        Log::error("this notify message is not parse cmdtype");
    }

    return "";
}

/**
 * 应答类信息解析
 * @param ppMsg[out] 输出解析后的结构体
 * @param root[in]   输入xml根节点
 * @return 报文命令类型CmdType
 */
static string ParseResponse(msgPublic** ppMsg, pugi::xml_node& root)
{
    // 查找CmdType节点
    pugi::xml_node nodeCmdType = root.child("CmdType");
    if (!nodeCmdType) {
        Log::error("not find CmdType");
        return "";
    }

    string strCmdType = nodeCmdType.child_value();
    if (strCmdType == "Catalog") {
        // 目录查询的应答消息
        Log::debug("this is a Catalog message");
        *ppMsg = ParseCatalog(root);

        // 解析结果并保存
        msgDevDirQuery* pInfo = (msgDevDirQuery*)*ppMsg;
        return strCmdType;
    }

    Log::error("this reponse message is not parse cmdtype");
    return "";
}

/**
 * 解析报文
 * @param ppMsg[out] 输出解析后的结构体
 * @param szBody[in] 输入报文体内容
 * @return 报文类型 报文类型+CmdType
 */
static string ParseMsgBody(msgPublic** ppMsg, const char* szBody)
{
    pugi::xml_document doc;
    do
    {
        if (!doc.load(szBody)) {
            Log::error("load xml failed");
            break;
        }

        // 根节点
        pugi::xml_node node = doc.first_child();
        if (!node) {
            Log::error("root first_child failed");
            break;
        }

        string strRoot = node.name(); 
        if (strRoot == "Notify") {
            // 通知类
            string strCmdType = ParseNotify(ppMsg, node);
            return strCmdType;
        } else if (strRoot == "Response") {
            // 应答类
            string strCmdType = ParseResponse(ppMsg, node);
            return strCmdType;
        }

        Log::error("this message is not parse type");
    }while (0);

    return "";
}

static void parserMessageInfo(osip_message_t*request, int iReqId, SipMessageInfo &msgInfo)
{
    if (nullptr == request) {
        Log::error("CSipMessage::parserMessageInfo nullptr == request");
        return;
    }
	/*
    Log::debug("CSipMessage::parserMessageInfo from user:%s,host:%s,port:%s; to user:%s,host:%s,port:%s"
        ,request->from->url->username,request->from->url->host,request->from->url->port
        ,request->to->url->username,request->to->url->host,request->to->url->port);
		*/

    msgInfo.method = request->sip_method;
    msgInfo.from   = GetFormatHeader(request->from->url->username, request->from->url->host, atoi(request->from->url->port));
    msgInfo.to     = GetFormatHeader(request->to->url->username, request->to->url->host, atoi(request->to->url->port));
    msgInfo.requestId = iReqId;
    msgInfo.callId = request->call_id->number;

    //解析content消息
    osip_body_t * body = NULL;
    osip_message_get_body(request, 0, &body);
    if (body != nullptr && nullptr != body->body)
    {
        msgInfo.content = body->body;
        msgInfo.strCmdType = ParseMsgBody(&msgInfo.pMsgBody, body->body);
    }
}

static void printMeaassgePkt(SipMessageInfo& info)
{
    LogDebug("接收到报文：");
    LogDebug("================================================================");
    LogDebug("method:      %s",info.method.c_str());
    LogDebug("from:        %s",info.from.c_str());
    LogDebug("to:          %s",info.to.c_str());

    //注册返回 由发送方维护的请求ID 接收方接收后原样返回即可
    LogDebug("RequestId:   %d",info.requestId);
    //CALL_ID
    LogDebug("Call-Id:     %s",info.callId.c_str());
    //解析content消息
    LogDebug("CmdType:     %s",info.strCmdType.c_str());
    LogDebug("\r\n%s"         ,info.content.c_str());
    LogDebug("================================================================");
}

static void sendMessageAnswer(SipMessageInfo& info)
{
    //Log::debug("CSipMessage::sendMessageAnswer answer Keepalive");
    osip_message_t* answer = NULL;
    int iStatus;
    eXosip_lock(g_pExContext);
    if (g_nLowExpire > 0) {
        iStatus = 200;
        int result = ::eXosip_message_build_answer(g_pExContext,info.requestId,iStatus, &answer);
        if (OSIP_SUCCESS != result) {
            ::eXosip_message_send_answer(g_pExContext,info.requestId, 400, NULL);
            //Log::warning("保活应答 发送400报文");
        } else {
            //发送消息体
            ::eXosip_message_send_answer(g_pExContext,info.requestId, iStatus, answer);
            //Log::warning("保活应答 发送200报文");
            osip_message_free(answer);

            if (info.strCmdType == "Keepalive")
                AutoQuery();
        }
    }
    eXosip_unlock(g_pExContext);
}

void CSipMessage::OnMessage(eXosip_event_t *osipEvent)
{
    SipMessageInfo msgInfo;
    parserMessageInfo(osipEvent->request, osipEvent->tid, msgInfo);
    //打印报文
    printMeaassgePkt(msgInfo);
    //发送应答报文
    sendMessageAnswer(msgInfo);
}

void CSipMessage::QueryDirtionary()
{
    string strFrom = GetFormatHeader(g_strCode , g_strSipIP , g_nSipPort);
    string strTo   = GetFormatHeader(g_strLowCode, g_strLowIP, g_nLowPort);

    LogDebug("CSipMessage::QueryDirtionary");

    osip_message_t *qdmMsg = NULL;
    int nID = eXosip_message_build_request(g_pExContext, &qdmMsg, "MESSAGE", 
        strTo.c_str(), strFrom.c_str(), nullptr);
    if (nID != OSIP_SUCCESS){
        Log::error("CSipMessage::QueryDirtionary init msg failed");
        return;
    }

    static int sn = 1;
    stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n\r\n"
        << "<Query>\r\n"
        << "<CmdType>Catalog</CmdType>\r\n"
        << "<SN>" << sn++ << "</SN>\r\n"
        << "<DeviceID>" << g_strLowCode << "</DeviceID>\r\n"
        << "</Query>\r\n";
    string strBody = ss.str();

    osip_message_set_contact(qdmMsg, strFrom.c_str());
    osip_message_set_content_type (qdmMsg, "Application/MANSCDP+xml");
    osip_message_set_body (qdmMsg, strBody.c_str(), strBody.length());

    eXosip_lock(g_pExContext);
    int ret = eXosip_message_send_request(g_pExContext, qdmMsg);
    eXosip_unlock(g_pExContext);
    osip_message_free(qdmMsg);
    if (ret <= 0){
        Log::error("CSipMessage::QueryDirtionary send failed:%d\r\n",ret);
    } else {
        Log::warning("send Query message QueryDirtionary\r\n");
    }
}

void CSipMessage::QueryDeviceStatus(string devID)
{
    string strFrom = GetFormatHeader(g_strCode , g_strSipIP , g_nSipPort);
    string strTo   = GetFormatHeader(g_strLowCode, g_strLowIP, g_nLowPort);

    LogDebug("CSipMessage::QueryDeviceStatus");

    osip_message_t *qdmMsg = NULL;
    int nID = eXosip_message_build_request(g_pExContext, &qdmMsg, "MESSAGE", 
        strTo.c_str(), strFrom.c_str(), nullptr);
    if (nID != OSIP_SUCCESS) {
        Log::error("CSipMessage::QueryDeviceStatus init msg failed");
        return;
    }

    static int sn = 1;
    stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n\r\n"
        << "<Query>\r\n"
        << "<CmdType>DeviceStatus</CmdType>\r\n"
        << "<SN>" << sn++ << "</SN>\r\n"
        << "<DeviceID>" << devID << "</DeviceID>\r\n"
        << "</Query>\r\n";
    string strBody = ss.str();

    osip_message_set_contact(qdmMsg, strFrom.c_str());
    osip_message_set_content_type (qdmMsg, "Application/MANSCDP+xml");
    osip_message_set_body (qdmMsg, strBody.c_str(), strBody.length());

    eXosip_lock(g_pExContext);
    int ret = eXosip_message_send_request(g_pExContext, qdmMsg);
    eXosip_unlock(g_pExContext);
    osip_message_free(qdmMsg);
    if (ret <= 0) {
        Log::error("CSipMessage::QueryDeviceStatus send failed:%d\r\n",ret);
    } else {
        Log::warning("send Query message QueryDeviceStatus\r\n");
    }
}

void CSipMessage::QueryDeviceInfo(string devID)
{
    string strFrom = GetFormatHeader(g_strCode , g_strSipIP , g_nSipPort);
    string strTo   = GetFormatHeader(g_strLowCode, g_strLowIP, g_nLowPort);

    LogDebug("CSipMessage::QueryDeviceInfo");

    osip_message_t *qdmMsg = NULL;
    int nID = eXosip_message_build_request(g_pExContext, &qdmMsg, "MESSAGE", 
        strTo.c_str(), strFrom.c_str(), nullptr);
    if (nID != OSIP_SUCCESS) {
        Log::error("CSipMessage::QueryDeviceInfo init msg failed");
        return;
    }

    static int sn = 1;
    stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n\r\n"
        << "<Query>\r\n"
        << "<CmdType>DeviceInfo</CmdType>\r\n"
        << "<SN>" << sn++ << "</SN>\r\n"
        << "<DeviceID>" << devID << "</DeviceID>\r\n"
        << "</Query>\r\n";
    string strBody = ss.str();

    osip_message_set_contact(qdmMsg, strFrom.c_str());
    osip_message_set_content_type (qdmMsg, "Application/MANSCDP+xml");
    osip_message_set_body (qdmMsg, strBody.c_str(), strBody.length());

    eXosip_lock(g_pExContext);
    int ret = eXosip_message_send_request(g_pExContext, qdmMsg);
    eXosip_unlock(g_pExContext);
    osip_message_free(qdmMsg);
    if (ret <= 0) {
        Log::error("CSipMessage::QueryDeviceInfo send failed:%d\r\n",ret);
    } else {
        Log::warning("send Query message QueryDeviceInfo\r\n");
    }
}

void CSipMessage::QueryRecordInfo(string devID, string strStartTime, string strEndTime)
{
    string strFrom = GetFormatHeader(g_strCode , g_strSipIP , g_nSipPort);
    string strTo   = GetFormatHeader(g_strLowCode, g_strLowIP, g_nLowPort);

    LogDebug("CSipMessage::QueryRecordInfo");

    osip_message_t *qdmMsg = NULL;
    int nID = eXosip_message_build_request(g_pExContext, &qdmMsg, "MESSAGE", 
        strTo.c_str(), strFrom.c_str(), nullptr);
    if (nID != OSIP_SUCCESS) {
        Log::error("CSipMessage::QueryRecordInfo init msg failed");
        return;
    }

    static int sn = 1;
    stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n\r\n"
        << "<Query>\r\n"
        << "<CmdType>RecordInfo</CmdType>\r\n"
        << "<SN>" << sn++ << "</SN>\r\n"
        << "<DeviceID>" << devID << "</DeviceID>\r\n"
        << "<StartTime>" << strStartTime << "</StartTime>\r\n"
        << "<EndTime>" << strEndTime << "</EndTime>\r\n"
        << "</Query>\r\n";
    string strBody = ss.str();

    osip_message_set_contact(qdmMsg, strFrom.c_str());
    osip_message_set_content_type (qdmMsg, "Application/MANSCDP+xml");
    osip_message_set_body (qdmMsg, strBody.c_str(), strBody.length());

    eXosip_lock(g_pExContext);
    int ret = eXosip_message_send_request(g_pExContext, qdmMsg);
    eXosip_unlock(g_pExContext);
    osip_message_free(qdmMsg);
    if (ret <= 0) {
        Log::error("CSipMessage::QueryRecordInfo send failed:%d\r\n",ret);
    } else {
        Log::warning("send Query message QueryRecordInfo\r\n");
    }
}

void CSipMessage::QueryMobilePosition(string devID)
{
    string strFrom = GetFormatHeader(g_strCode , g_strSipIP , g_nSipPort);
    string strTo   = GetFormatHeader(g_strLowCode, g_strLowIP, g_nLowPort);

    LogDebug("CSipMessage::QueryMobilePosition");

    static osip_message_t *qdmMsg = 0;

    int nID = eXosip_message_build_request(g_pExContext, &qdmMsg, "MESSAGE", 
        strTo.c_str(), strFrom.c_str(), nullptr);
    if (nID != OSIP_SUCCESS) {
        Log::error("CSipMessage::QueryMobilePosition init msg failed");
        return;
    }

    static int sn = 1;
    stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n\r\n"
        << "<Query>\r\n"
        << "<CmdType>MobilePosition</CmdType>\r\n"
        << "<SN>" << sn++ << "</SN>\r\n"
        << "<DeviceID>" << devID << "</DeviceID>\r\n"
        << "</Query>\r\n";
    string strBody = ss.str();

    osip_message_set_contact(qdmMsg, strFrom.c_str());
    osip_message_set_content_type (qdmMsg, "Application/MANSCDP+xml");
    osip_message_set_body (qdmMsg, strBody.c_str(), strBody.length());

    eXosip_lock(g_pExContext);
    int ret = eXosip_message_send_request(g_pExContext, qdmMsg);
    eXosip_unlock(g_pExContext);
    osip_message_free(qdmMsg);
    if (ret <= 0) {
        Log::error("CSipMessage::QueryMobilePosition send failed:%d\r\n",ret);
    } else {
        Log::warning("send Query message QueryMobilePosition\r\n");
    }
}

void CSipMessage::DeviceControl(string strDevCode, 
                                int nInOut, int nUpDown, int nLeftRight, uint8_t cInOutSpeed, uint8_t cMoveSpeed)
{
    string strFrom = GetFormatHeader(g_strCode , g_strSipIP , g_nSipPort);
    string strTo   = GetFormatHeader(g_strLowCode, g_strLowIP, g_nLowPort);

    // 创建消息结构
    osip_message_t *dcMsg = 0;
    int nID = eXosip_message_build_request(g_pExContext, &dcMsg, "MESSAGE", 
        strTo.c_str(), strFrom.c_str(), nullptr);
    if (nID != OSIP_SUCCESS) {
        Log::error("eXosip_message_build_request failed");
        return;
    }

    // 控制命令
    uint8_t cControlCode = 0;
    if(nLeftRight == 2) cControlCode|=0x01;       // 右移
    else if(nLeftRight == 1) cControlCode|=0x02;  // 左移
    if (nUpDown == 2) cControlCode|=0x04;         // 下移
    else if(nUpDown == 1) cControlCode|=0x08;     // 上移
    if (nInOut == 2) cControlCode |= 0x10;        // 放大
    else if(nInOut == 1) cControlCode |= 0x20;    // 缩小
    char szCmd[20]={0};
    char szTmp[10]={0};
    szCmd[0] = 'A'; //字节1 A5
    szCmd[1] = '5';
    szCmd[2] = '0'; //字节2 0F
    szCmd[3] = 'F'; 
    szCmd[4] = '0'; //字节3 地址的低8位
    szCmd[5] = '1'; 
    sprintf_s(szTmp, 10,"%02X", cControlCode); 
    //Log::debug("cControlCode is %s", szTmp);
    szCmd[6]  = szTmp[0];  //字节4 控制码
    szCmd[7]  = szTmp[1];
    sprintf_s(szTmp, 10,"%02X", cMoveSpeed); 
    //Log::debug("cMoveSpeed is %s", szTmp);
    szCmd[8]  = szTmp[0];  //字节5 水平控制速度
    szCmd[9]  = szTmp[1];
    szCmd[10] = szTmp[0];  //字节6 垂直控制速度
    szCmd[11] = szTmp[1];
    sprintf_s(szTmp, 10,"%X", cInOutSpeed); 
    //Log::debug("cInOutSpeed is %s", szTmp);
    szCmd[12] = szTmp[0];  //字节7高4位 缩放控制速度
    szCmd[13] = '0';       //字节7低4位 地址的高4位
    //计算校验码
    int nCheck = (0XA5 + 0X0F + 0X01 + cControlCode + cMoveSpeed + cMoveSpeed + cInOutSpeed<<4&0XF0)%0X100;
    sprintf_s(szTmp,10,"%02X", nCheck);
    //Log::debug("nCheck is %s", szTmp);
    szCmd[14] = szTmp[0]; //字节8 校验码
    szCmd[15] = szTmp[1];
    Log::debug("PTZCmd is %s", szCmd);

    // 组成报文体
    static int sn = 1;
    stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n\r\n"
        << "<Control>\r\n"
        << "<CmdType>DeviceControl</CmdType>\r\n"
        << "<SN>" << sn++ << "</SN>\r\n"
        << "<DeviceID>" << strDevCode << "</DeviceID>\r\n"
        << "<PTZCmd>" << szCmd << "</PTZCmd>\r\n"
        << "</Control>\r\n";
    string strBody = ss.str();

    osip_message_set_contact(dcMsg, strFrom.c_str());
    osip_message_set_content_type (dcMsg, "Application/MANSCDP+xml");
    osip_message_set_body (dcMsg, strBody.c_str(), strBody.length());

    eXosip_lock(g_pExContext);
    int ret = eXosip_message_send_request(g_pExContext, dcMsg);
    eXosip_unlock(g_pExContext);
    osip_message_free(dcMsg);
    if (ret <= 0) {
        Log::error("eXosip_message_send_request failed:%d\r\n",ret);
    } else {
        Log::warning("send DeviceControl message\r\n");
    }
}
