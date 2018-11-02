#include "stdafx.h"
#include "SipMsgParser.h"
#include "SipMgr.h"

CSipMsgParser::CSipMsgParser(void)
{
}


CSipMsgParser::~CSipMsgParser(void)
{
}

string CSipMsgParser::ParseMsgBody(msgPublic** ppMsg, const char* szBody)
{
    pugi::xml_document doc;
    do
    {
        if (!doc.load(szBody))
        {
            Log::error("load xml failed");
            break;
        }

        // 根节点
        pugi::xml_node node = doc.first_child();
        if (!node)
        {
            Log::error("root first_child failed");
            break;
        }

        string strRoot = node.name();
        if (strRoot == "Notify")
        {
            // 通知类
            string strCmdType = ParseNotify(ppMsg, node);
            return strCmdType;
        }
        else if (strRoot == "Response")
        {
            // 应答类
            string strCmdType = ParseResponse(ppMsg, node);
            return strCmdType;
        }
        
        Log::error("this message is not parse type");
    }while (0);

    return "";
}

string CSipMsgParser::ParseNotify(msgPublic** ppMsg, pugi::xml_node& root)
{
    // 查找CmdType节点
    pugi::xml_node nodeCmdType = root.child("CmdType");
    if (!nodeCmdType)
    {
        Log::error("not find CmdType");
        return "";
    }

    string strCmdType = nodeCmdType.child_value();
    if (strCmdType == "Keepalive")
    {
        Log::debug("this is a keepalive message");
        *ppMsg = ParseKeepAlive(root);
        return strCmdType;
    }
    else if(strCmdType == "Catalog")
    {
        Log::debug("this is a Catalog message");
        *ppMsg = ParseNotifyCatalog(root);
        msgDevDirQuery* pInfo = (msgDevDirQuery*)*ppMsg;
        for(auto& dev:pInfo->vecDevInfo)
        {
            DeviceMgr::UpdateDevice(dev);
        }
    }
    else if(strCmdType == "MobilePosition")
    {
        Log::debug("this is a MobilePosition message");
        *ppMsg = ParseMobilePosition(root);
        msgDevDirQuery* pInfo = (msgDevDirQuery*)*ppMsg;
        for(auto& dev:pInfo->vecDevInfo)
        {
            DeviceMgr::UpdateDevice(dev);
        }
    }
    else
    {
        Log::error("this notify message is not parse cmdtype");
    }
    
    return "";
}

string CSipMsgParser::ParseResponse(msgPublic** ppMsg, pugi::xml_node& root)
{
    // 查找CmdType节点
    pugi::xml_node nodeCmdType = root.child("CmdType");
    if (!nodeCmdType)
    {
        Log::error("not find CmdType");
        return "";
    }

    string strCmdType = nodeCmdType.child_value();
    if (strCmdType == "Catalog")
    {
        // 目录查询的应答消息
        Log::debug("this is a Catalog message");
        *ppMsg = ParseCatalog(root);

        // 解析结果并保存
        msgDevDirQuery* pInfo = (msgDevDirQuery*)*ppMsg;
        DeviceMgr::AddDevice(pInfo->vecDevInfo);
        return strCmdType;
    }

    Log::error("this reponse message is not parse cmdtype");
    return "";
}

msgPublic* CSipMsgParser::ParseKeepAlive(pugi::xml_node& root)
{
    msgKeepAlive *pRet = new msgKeepAlive;
    string nodeName;
    string nodeValue;

    LogDebug("<%s>",root.name());
    //遍历历子节点
    for (pugi::xml_node node = root.first_child(); node; node = node.next_sibling())
    {
        nodeName  = node.name();
        if (nodeName == "CmdType")
        {
            pRet->strCmdType = node.child_value();
            LogDebug("<%s>%s</%s>",nodeName.c_str(),node.child_value(),nodeName.c_str());
        }
        else if(nodeName == "SN")
        {
            pRet->strSN = node.child_value();
            LogDebug("<%s>%s</%s>",nodeName.c_str(),node.child_value(),nodeName.c_str());
        }
        else if (nodeName == "DeviceID")
        {
            pRet->strDeviceID = node.child_value();
            LogDebug("<%s>%s</%s>",nodeName.c_str(),node.child_value(),nodeName.c_str());
        }
        else if (nodeName == "Status")
        {
            pRet->strStatus = node.child_value();
            LogDebug("<%s>%s</%s>",nodeName.c_str(),node.child_value(),nodeName.c_str());
        }
        else if (nodeName == "")
        {

        }

    }
    LogDebug("</%s>",root.name());

    // 平台保活
    if(!DeviceMgr::KeepAlivePlatform())
    {
        Log::error("KeepAlivePlatform failed");
    }
    return pRet;
}

msgPublic* CSipMsgParser::ParseCatalog(pugi::xml_node& root)
{
    msgDevDirQuery *pRet = new msgDevDirQuery;
    string nodeName;
    string nodeValue;

    LogDebug("<%s>",root.name());
    //遍历历子节点
    for (pugi::xml_node node = root.first_child(); node; node = node.next_sibling())
    {
        nodeName  = node.name();
        if(nodeName != "DeviceList")
        {
            LogDebug("<%s>%s</%s>",nodeName.c_str(),node.child_value(),nodeName.c_str());
        }
        if (nodeName == "CmdType")
        {
            pRet->strCmdType = node.child_value();
        }
        else if(nodeName == "SN")
        {
            pRet->strSN = node.child_value();
        }
        else if (nodeName == "DeviceID")
        {
            pRet->strDeviceID = node.child_value();
        }
        else if (nodeName == "SumNum")
        {
            pRet->strSumNum = node.child_value();
        }
        else if (nodeName == "DeviceList")
        {
            stringstream ss;
            ss << "<" << nodeName;
            pugi::xml_attribute attr = node.attribute("Num");
            if(!attr.empty())
            {
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
                    if(nodeName != "Info")
                    {
                        LogDebug("<%s>%s</%s>",nodeName.c_str(),devNode.child_value(),nodeName.c_str());
                    }
                    if (nodeName == "DeviceID")
                    {
                        dev.strDevID = devNode.child_value();
                    }
                    else if (nodeName == "Name")
                    {
                        dev.strName = devNode.child_value();
                    }
                    else if (nodeName == "Manufacturer")
                    {
                        dev.strManuf = devNode.child_value();
                    }
                    else if (nodeName == "Model")
                    {
                        dev.strModel = devNode.child_value();
                    }
                    else if (nodeName == "Owner")
                    {
                        dev.strOwner = devNode.child_value();
                    }
                    else if (nodeName == "CivilCode")
                    {
                        dev.strCivilCode = devNode.child_value();
                    }
                    else if (nodeName == "Block")
                    {
                        dev.strBlock = devNode.child_value();
                    }
                    else if (nodeName == "Address")
                    {
                        dev.strAddress = devNode.child_value();
                    }
                    else if (nodeName == "Parental")
                    {
                        dev.strParental = devNode.child_value();
                    }
                    else if (nodeName == "ParentID")
                    {
                        dev.strParentID = devNode.child_value();
                    }
                    else if (nodeName == "SafetyWay")
                    {
                        dev.strSafetyWay = devNode.child_value();
                    }
                    else if (nodeName == "RegisterWay")
                    {
                        dev.strRegisterWay = devNode.child_value();
                    }
                    else if (nodeName == "CertNum")
                    {
                        dev.strCertNum = devNode.child_value();
                    }
                    else if (nodeName == "Certifiable")
                    {
                        dev.strCertifiable = devNode.child_value();
                    }
                    else if (nodeName == "ErrCode")
                    {
                        dev.strErrCode = devNode.child_value();
                    }
                    else if (nodeName == "EndTime")
                    {
                        dev.strEndTime = devNode.child_value();
                    }
                    else if (nodeName == "Secrecy")
                    {
                        dev.strSecrecy = devNode.child_value();
                    }
                    else if (nodeName == "Status")
                    {
                        dev.strStatus = devNode.child_value();
                    }
                    else if (nodeName == "IPAddress")
                    {
                        dev.strIPAddress = devNode.child_value();
                    }
                    else if (nodeName == "Port")
                    {
                        dev.strPort = devNode.child_value();
                    }
                    else if (nodeName == "Password")
                    {
                        dev.strPassword = devNode.child_value();
                    }
                    else if (nodeName == "Longitude")
                    {
                        dev.strLongitude = devNode.child_value();
                    }
                    else if (nodeName == "Latitude")
                    {
                        dev.strLatitude = devNode.child_value();
                    }
                    else if (nodeName == "Info")
                    {
                        LogDebug("<Info>");
                        for (pugi::xml_node infoNode = devNode.first_child(); infoNode; infoNode=infoNode.next_sibling())
                        {
                            nodeName = infoNode.name();
                            LogDebug("<%s>%s</%s>",nodeName.c_str(),infoNode.child_value(),nodeName.c_str());

                            if (nodeName == "PTZType")
                            {
                                dev.strPTZType = infoNode.child_value();
                            }
                            else if (nodeName == "PositionType")
                            {
                                dev.strPositionType = infoNode.child_value();
                            }
                            else if (nodeName == "RoomType")
                            {
                                dev.strRoomType = infoNode.child_value();
                            }
                            else if (nodeName == "UseType")
                            {
                                dev.strUseType = infoNode.child_value();
                            }
                            else if (nodeName == "SupplyLightType")
                            {
                                dev.strSupplyLightType = infoNode.child_value();
                            }
                            else if (nodeName == "DirectionType")
                            {
                                dev.strDirectionType = infoNode.child_value();
                            }
                            else if (nodeName == "Resolution")
                            {
                                dev.strResolution = infoNode.child_value();
                            }
                            else if (nodeName == "BusinessGroupID")
                            {
                                dev.strBusinessGroupID = infoNode.child_value();
                            }
                            else if (nodeName == "DownloadSpeed")
                            {
                                dev.strDownloadSpeed = infoNode.child_value();
                            }
                            else if (nodeName == "SVCSpaceSupportMode")
                            {
                                dev.strSVCSpaceSupportType = infoNode.child_value();
                            }
                            else if (nodeName == "SVCTimeSupportMode")
                            {
                                dev.strSVCTimeSupportType = infoNode.child_value();
                            }
                            else
                            {
                                Log::error("node[%s] is not catched",nodeName.c_str());
                            }
                        }// for Info Node
                        LogDebug("</Info>");
                    }// Info
                    else
                    {
                        Log::error("node[%s] is not catched [%s]",nodeName.c_str(), devNode.child_value());
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

msgPublic* CSipMsgParser::ParseNotifyCatalog(pugi::xml_node& root)
{
    msgDevDirQuery *pRet = new msgDevDirQuery;
    string nodeName;
    string nodeValue;

    LogDebug("<%s>",root.name());
    //遍历历子节点
    for (pugi::xml_node node = root.first_child(); node; node = node.next_sibling())
    {
        nodeName  = node.name();
        if(nodeName != "DeviceList")
        {
            LogDebug("<%s>%s</%s>",nodeName.c_str(),node.child_value(),nodeName.c_str());
        }
        if (nodeName == "CmdType")
        {
            pRet->strCmdType = node.child_value();
        }
        else if(nodeName == "SN")
        {
            pRet->strSN = node.child_value();
        }
        else if (nodeName == "DeviceID")
        {
            pRet->strDeviceID = node.child_value();
        }
        else if (nodeName == "SumNum")
        {
            pRet->strSumNum = node.child_value();
        }
        else if (nodeName == "DeviceList")
        {
            stringstream ss;
            ss << "<" << nodeName;
            pugi::xml_attribute attr = node.attribute("Num");
            if(!attr.empty())
            {
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
                    if(nodeName != "Info")
                    {
                        LogDebug("<%s>%s</%s>",nodeName.c_str(),devNode.child_value(),nodeName.c_str());
                    }
                    if (nodeName == "DeviceID")
                    {
                        dev.strDevID = devNode.child_value();
                    }
                    else if (nodeName == "Event")
                    {
                        dev.strStatus = devNode.child_value();
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

msgPublic* CSipMsgParser::ParseMobilePosition(pugi::xml_node& root)
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
        if (nodeName == "CmdType")
        {
            pRet->strCmdType = node.child_value();
        }
        else if(nodeName == "SN")
        {
            pRet->strSN = node.child_value();
        }
        else if (nodeName == "DeviceID")
        {
            dev.strDevID = node.child_value();
        }
        else if (nodeName == "Longitude")
        {
            dev.strLongitude = node.child_value();
        }
        else if (nodeName == "Latitude")
        {
            dev.strLatitude = node.child_value();
        }
    }
    pRet->vecDevInfo.push_back(pDevCtrl);
    return pRet;
}