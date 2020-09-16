#include "SipPrivate.h"
#include "SipMessage.h"
#include "SipServer.h"
#include "pugixml.hpp"
#include <string>
#include <map>
using namespace SipServer;
using namespace std;

//struct msgPublic
//{
//    string strCmdType;  //< ��������
//    string strSN;       //< �������к� 
//    string strDeviceID; //< Դ�豸/ϵͳ����
//};
//
///** ֪ͨ���� */
//struct msgNotify : public msgPublic
//{
//};
//
///** Ӧ������ */
//struct msgResponse : public msgPublic
//{
//};
//
///** ״̬��Ϣ���� */
//struct msgKeepAlive : public msgNotify
//{
//    string         strStatus;   //< �Ƿ���������
//    //vector<string> vecDeviceID; //< �����豸�б�
//};
//
///** �豸Ŀ¼��ѯ��ϢӦ�� */
//struct msgDevDirQuery : public msgResponse
//{
//    string            strSumNum;   //< ��ѯ�������
//    //vector<DevInfo*>  vecDevInfo;  //< �豸�б�
//};

struct SipMessageInfo
{
    int    requestId;              //Sip�㷵�ص�����ı�־ ��Ӧʱ���ؼ���
    string callId;                 //ά��һ��ע��
    string method;                 //��Ϣ�����Ĺ��ܷ������ַ���
    string from;                   //��ַ����@������IP��ַ:���Ӷ˿ڣ�����sip:1111@127.0.0.1:5060
    string to;                     //��ַ����@������IP��ַ:���Ӷ˿ڣ�����sip:1111@127.0.0.1:5060
    string content;                // ��Ϣ��
    string strCmdType;             // ��Ϣ����
    map<string,string> mapBody;    // ��Ϣ�����
};

//////////////////////////////////////////////////////////////////////////

/**
 * ����״̬��Ϣ����
 */
static void ParseKeepAlive(pugi::xml_node& root)
{
    //msgKeepAlive *pRet = new msgKeepAlive;
    string nodeName;
    string nodeValue;
    string devID, status;

    LogDebug("<%s>",root.name());
    //�������ӽڵ�
    for (pugi::xml_node node = root.first_child(); node; node = node.next_sibling())
    {
        nodeName  = node.name();
        if (nodeName == "CmdType") {
            //pRet->strCmdType = node.child_value();
            LogDebug("<%s>%s</%s>",nodeName.c_str(),node.child_value(),nodeName.c_str());
        } else if(nodeName == "SN") {
            //pRet->strSN = node.child_value();
            LogDebug("<%s>%s</%s>",nodeName.c_str(),node.child_value(),nodeName.c_str());
        } else if (nodeName == "DeviceID") {
            /*pRet->strDeviceID*/
            devID = node.child_value();
            LogDebug("<%s>%s</%s>",nodeName.c_str(),node.child_value(),nodeName.c_str());
        } else if (nodeName == "Status") {
            //pRet->strStatus
            status= node.child_value();
            LogDebug("<%s>%s</%s>",nodeName.c_str(),node.child_value(),nodeName.c_str());
        } else if (!nodeName.empty()) {
            Log::error("%s uncatch", nodeName.c_str());
        }

    }
    LogDebug("</%s>",root.name());

    // ƽ̨����
    g_nLowExpire = 120;
    //return pRet;
}

/**
 * �����豸Ŀ¼��Ϣ��ѯ����
 */
static void ParseCatalog(pugi::xml_node& root)
{
    string nodeName;
    string nodeValue;

    LogDebug("<%s>",root.name());
    //�������ӽڵ�
    for (pugi::xml_node node = root.first_child(); node; node = node.next_sibling())
    {
        nodeName  = node.name();
        if(nodeName != "DeviceList") {
            LogDebug("<%s>%s</%s>",nodeName.c_str(),node.child_value(),nodeName.c_str());
        }
        if (nodeName == "CmdType") {
            //pRet->strCmdType = node.child_value();
        } else if(nodeName == "SN") {
            //pRet->strSN = node.child_value();
        } else if (nodeName == "DeviceID") {
            //pRet->strDeviceID = node.child_value();
        } else if (nodeName == "SumNum") {
            //pRet->strSumNum = node.child_value();
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

                //if(g_mapDevice.count(pDevCtrl->strDevID) == 0)
                //    g_mapDevice.insert(make_pair(pDevCtrl->strDevID, pDevCtrl));
                //else {
                //    delete g_mapDevice[pDevCtrl->strDevID];
                //    g_mapDevice[pDevCtrl->strDevID] = pDevCtrl;
                //}
                if(g_addDevice)
                    g_addDevice(pDevCtrl);
            } // for DeviceList Node

            LogDebug("</DeviceList>");
        }// DeviceList
    }// for root Node
    LogDebug("</%s>",root.name());
    //return pRet;
}

/**
 * Ŀ¼��Ϣ���ĺ��յ������ͱ���
 */
static void ParseNotifyCatalog(pugi::xml_node& root)
{
    //msgDevDirQuery *pRet = new msgDevDirQuery;
    string nodeName;
    string nodeValue;

    LogDebug("<%s>",root.name());
    //�������ӽڵ�
    for (pugi::xml_node node = root.first_child(); node; node = node.next_sibling())
    {
        nodeName  = node.name();
        if(nodeName != "DeviceList") {
            LogDebug("<%s>%s</%s>",nodeName.c_str(),node.child_value(),nodeName.c_str());
        }
        if (nodeName == "CmdType") {
            //pRet->strCmdType = node.child_value();
        } else if(nodeName == "SN") {
            //pRet->strSN = node.child_value();
        } else if (nodeName == "DeviceID") {
            //pRet->strDeviceID = node.child_value();
        } else if (nodeName == "SumNum") {
            //pRet->strSumNum = node.child_value();
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

                //������״̬
                if(g_updateStatus && !pDevCtrl->strStatus.empty()) {
                    g_updateStatus(pDevCtrl->strDevID, pDevCtrl->strStatus);
                }
                // ������GPS
                if(g_updatePostion && !pDevCtrl->strLongitude.empty() && !pDevCtrl->strLatitude.empty()) {
                    g_updatePostion(pDevCtrl->strDevID, pDevCtrl->strLongitude, pDevCtrl->strLatitude);
                }
            } // for DeviceList Node

            LogDebug("</DeviceList>");
        }// DeviceList
    }// for root Node
    LogDebug("</%s>",root.name());
    //return pRet;
}

/**
 * λ����Ϣ���ĺ��յ������ͱ���
 */
static void ParseMobilePosition(pugi::xml_node& root)
{
    //msgDevDirQuery *pRet = new msgDevDirQuery;
    string nodeName;
    string nodeValue;

    LogDebug("<%s>",root.name());
    DevInfo* pDevCtrl = new DevInfo;
    DevInfo& dev = *pDevCtrl;
    //�������ӽڵ�
    for (pugi::xml_node node = root.first_child(); node; node = node.next_sibling())
    {
        nodeName  = node.name();
        if (nodeName == "CmdType") {
            //pRet->strCmdType = node.child_value();
        } else if(nodeName == "SN") {
            //pRet->strSN = node.child_value();
        } else if (nodeName == "DeviceID") {
            dev.strDevID = node.child_value();
        } else if (nodeName == "Longitude") {
            dev.strLongitude = node.child_value();
        } else if (nodeName == "Latitude") {
            dev.strLatitude = node.child_value();
        }
    }
    if(g_updatePostion) {
        g_updatePostion(pDevCtrl->strDevID, pDevCtrl->strLongitude, pDevCtrl->strLatitude);
    }
    //return pRet;
}

/**
 * ֪ͨ����Ϣ����
 * @param ppMsg[out] ���������Ľṹ��
 * @param root[in]   ����xml���ڵ�
 * @return ������������CmdType
 */
static string ParseNotify(pugi::xml_node& root)
{
    // ����CmdType�ڵ�
    pugi::xml_node nodeCmdType = root.child("CmdType");
    if (!nodeCmdType) {
        Log::error("not find CmdType");
        return "";
    }

    string strCmdType = nodeCmdType.child_value();
    if (strCmdType == "Keepalive"){
        Log::debug("this is a keepalive message");
        ParseKeepAlive(root);
    } else if(strCmdType == "Catalog") {
        Log::debug("this is a Catalog message");
        ParseNotifyCatalog(root);
    } else if(strCmdType == "MobilePosition") {
        Log::debug("this is a MobilePosition message");
        ParseMobilePosition(root);
    } else {
        Log::error("this notify message is not parse cmdtype");
    }

    return strCmdType;
}

/**
 * Ӧ������Ϣ����
 * @param ppMsg[out] ���������Ľṹ��
 * @param root[in]   ����xml���ڵ�
 * @return ������������CmdType
 */
static string ParseResponse(pugi::xml_node& root)
{
    // ����CmdType�ڵ�
    pugi::xml_node nodeCmdType = root.child("CmdType");
    if (!nodeCmdType) {
        Log::error("not find CmdType");
        return "";
    }

    string strCmdType = nodeCmdType.child_value();
    if (strCmdType == "Catalog") {
        // Ŀ¼��ѯ��Ӧ����Ϣ
        Log::debug("this is a Catalog message");
        ParseCatalog(root);
        return strCmdType;
    }

    Log::error("this reponse message is not parse cmdtype");
    return "";
}

/**
 * ��������
 * @param ppMsg[out] ���������Ľṹ��
 * @param szBody[in] ���뱨��������
 * @return �������� ��������+CmdType
 */
static string ParseMsgBody(const char* szBody)
{
    pugi::xml_document doc;
    do
    {
        if (!doc.load(szBody)) {
            Log::error("load xml failed");
            break;
        }

        // ���ڵ�
        pugi::xml_node node = doc.first_child();
        if (!node) {
            Log::error("root first_child failed");
            break;
        }

        string strRoot = node.name(); 
        if (strRoot == "Notify") {
            // ֪ͨ��
            string strCmdType = ParseNotify(node);
            return strCmdType;
        } else if (strRoot == "Response") {
            // Ӧ����
            string strCmdType = ParseResponse(node);
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
    msgInfo.from   = GetFormatHeader(request->from->url->username, request->from->url->host, sz2int(request->from->url->port));
    msgInfo.to     = GetFormatHeader(request->to->url->username, request->to->url->host, sz2int(request->to->url->port));
    msgInfo.requestId = iReqId;
    msgInfo.callId = request->call_id->number;

    //����content��Ϣ
    osip_body_t * body = NULL;
    osip_message_get_body(request, 0, &body);
    if (body != nullptr && nullptr != body->body)
    {
        msgInfo.content = body->body;
        msgInfo.strCmdType = ParseMsgBody(body->body);
    }
}

static void printMeaassgePkt(SipMessageInfo& info)
{
    LogDebug("���յ����ģ�");
    LogDebug("================================================================");
    LogDebug("method:      %s",info.method.c_str());
    LogDebug("from:        %s",info.from.c_str());
    LogDebug("to:          %s",info.to.c_str());

    //ע�᷵�� �ɷ��ͷ�ά��������ID ���շ����պ�ԭ�����ؼ���
    LogDebug("RequestId:   %d",info.requestId);
    //CALL_ID
    LogDebug("Call-Id:     %s",info.callId.c_str());
    //����content��Ϣ
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
            //Log::warning("����Ӧ�� ����400����");
        } else {
            //������Ϣ��
            ::eXosip_message_send_answer(g_pExContext,info.requestId, iStatus, answer);
            //Log::warning("����Ӧ�� ����200����");
            //osip_message_free(answer);

            if (info.strCmdType == "Keepalive") {
				g_bLowStatus    = true;
                AutoQuery();
			}
        }
    }
    eXosip_unlock(g_pExContext);
}

void CSipMessage::OnMessage(eXosip_event_t *osipEvent)
{
    SipMessageInfo msgInfo;
    parserMessageInfo(osipEvent->request, osipEvent->tid, msgInfo);
    //��ӡ����
    printMeaassgePkt(msgInfo);
    //����Ӧ����
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
    //osip_message_free(qdmMsg);
    if (ret <= 0){
        Log::error("CSipMessage::QueryDirtionary send failed:%d\r\n",ret);
    } else {
        Log::warning("send Query message QueryDirtionary\r\n");
    }

    //g_mapDevice.clear();
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
    //osip_message_free(qdmMsg);
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
    //osip_message_free(qdmMsg);
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
    //osip_message_free(qdmMsg);
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
    //osip_message_free(qdmMsg);
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

    // ������Ϣ�ṹ
    osip_message_t *dcMsg = 0;
    int nID = eXosip_message_build_request(g_pExContext, &dcMsg, "MESSAGE", 
        strTo.c_str(), strFrom.c_str(), nullptr);
    if (nID != OSIP_SUCCESS) {
        Log::error("eXosip_message_build_request failed");
        return;
    }

    // ��������
    uint8_t cControlCode = 0;
    if(nLeftRight == 2) cControlCode|=0x01;       // ����
    else if(nLeftRight == 1) cControlCode|=0x02;  // ����
    if (nUpDown == 2) cControlCode|=0x04;         // ����
    else if(nUpDown == 1) cControlCode|=0x08;     // ����
    if (nInOut == 2) cControlCode |= 0x10;        // �Ŵ�
    else if(nInOut == 1) cControlCode |= 0x20;    // ��С
    char szCmd[20]={0};
    char szTmp[10]={0};
    szCmd[0] = 'A'; //�ֽ�1 A5
    szCmd[1] = '5';
    szCmd[2] = '0'; //�ֽ�2 0F
    szCmd[3] = 'F'; 
    szCmd[4] = '0'; //�ֽ�3 ��ַ�ĵ�8λ
    szCmd[5] = '1'; 
    sprintf_s(szTmp, 10,"%02X", cControlCode); 
    //Log::debug("cControlCode is %s", szTmp);
    szCmd[6]  = szTmp[0];  //�ֽ�4 ������
    szCmd[7]  = szTmp[1];
    sprintf_s(szTmp, 10,"%02X", cMoveSpeed); 
    //Log::debug("cMoveSpeed is %s", szTmp);
    szCmd[8]  = szTmp[0];  //�ֽ�5 ˮƽ�����ٶ�
    szCmd[9]  = szTmp[1];
    szCmd[10] = szTmp[0];  //�ֽ�6 ��ֱ�����ٶ�
    szCmd[11] = szTmp[1];
    sprintf_s(szTmp, 10,"%X", cInOutSpeed); 
    //Log::debug("cInOutSpeed is %s", szTmp);
    szCmd[12] = szTmp[0];  //�ֽ�7��4λ ���ſ����ٶ�
    szCmd[13] = '0';       //�ֽ�7��4λ ��ַ�ĸ�4λ
    //����У����
    int nCheck = (0XA5 + 0X0F + 0X01 + cControlCode + cMoveSpeed + cMoveSpeed + cInOutSpeed<<4&0XF0)%0X100;
    sprintf_s(szTmp,10,"%02X", nCheck);
    //Log::debug("nCheck is %s", szTmp);
    szCmd[14] = szTmp[0]; //�ֽ�8 У����
    szCmd[15] = szTmp[1];
    Log::debug("PTZCmd is %s", szCmd);

    // ��ɱ�����
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
    //osip_message_free(dcMsg);
    if (ret <= 0) {
        Log::error("eXosip_message_send_request failed:%d\r\n",ret);
    } else {
        Log::warning("send DeviceControl message\r\n");
    }
}
