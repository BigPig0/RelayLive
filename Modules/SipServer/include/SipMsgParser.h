/**
 * 监听服务接收到的报文中，带有MANSDP+XML格式的报文body解析
 */
#pragma once
#include "pugixml.hpp"
#include "PublicDefine.h"

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
    vector<DevInfo*> vecDevInfo;  //< 设备列表
};

/**
 * 解析报文
 */
class CSipMsgParser
{
public:
    CSipMsgParser(void);
    ~CSipMsgParser(void);

    /**
     * 解析报文
     * @param ppMsg[out] 输出解析后的结构体
     * @param szBody[in] 输入报文体内容
     * @return 报文类型 报文类型+CmdType
     */
    string ParseMsgBody(msgPublic** ppMsg, const char* szBody);

private:
    /**
     * 通知类信息解析
     * @param ppMsg[out] 输出解析后的结构体
     * @param root[in]   输入xml根节点
     * @return 报文命令类型CmdType
     */
    string ParseNotify(msgPublic** ppMsg, pugi::xml_node& root);

    /**
     * 应答类信息解析
     * @param ppMsg[out] 输出解析后的结构体
     * @param root[in]   输入xml根节点
     * @return 报文命令类型CmdType
     */
    string ParseResponse(msgPublic** ppMsg, pugi::xml_node& root);

    /**
     * 解析状态信息报送
     */
    msgPublic* ParseKeepAlive(pugi::xml_node& root);

    /**
     * 解析设备目录信息查询报文
     */
    msgPublic* ParseCatalog(pugi::xml_node& root);

    /**
     * 目录信息订阅后收到的推送报文
     */
    msgPublic* ParseNotifyCatalog(pugi::xml_node& root);

    /**
     * 位置信息订阅后收到的推送报文
     */
    msgPublic* ParseMobilePosition(pugi::xml_node& root);
};

