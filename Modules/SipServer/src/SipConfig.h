#pragma once
/**
 * 本地配置信息
 */
class CSipConfig
{
public:
    CSipConfig():bRegAuthor(false){};
    ~CSipConfig(){};
    /**
     * 加载配置文件
     */
    void Load();

public:
    /** Sip服务器配置 */
    string   strDevCode;       //< 国标编码
    string   strAddrIP;        //< 本地IP
    string   strAddrPort;      //< Sip监听端口
    bool     bRegAuthor;       //< 是否开启鉴权
};
