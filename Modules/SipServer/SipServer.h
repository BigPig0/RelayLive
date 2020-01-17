/**
 * Sip服务器
 */
#pragma once
#include <string>
using namespace std;

#ifdef SIP_EXPORTS
#define SIP_API __declspec(dllexport)
#else
#define SIP_API
#endif

namespace SipServer {

    /** 设备目录查询信息应答中的设备信息 */
    struct DevInfo {
        string strDevID;       //< 国标编码
        string strName;        //< 设备名称
        string strManuf;       //< 厂商
        string strModel;       //< 型号
        string strOwner;       //< 归属
        string strCivilCode;   //< 行政区域编码
        string strBlock;       //< 警区
        string strAddress;     //< 安装地址
        string strParental;    //< 是否有子设备 1:有 0:无
        string strParentID;    //< 父设备ID
        string strSafetyWay;   //< 信令安全模式 0(默认):不采用 2:S/MIME签名方式 3:S/MIME加密签名同时采用方式 4:数字摘要方式
        string strRegisterWay; //< 注册方式 1(默认):符合RETF RFC3261标准的认证注册模式 2:基于口令的双向认证注册模式 3:基于数字证书的双向认证注册模式
        string strCertNum;     //< 证书序列号
        string strCertifiable; //< 证书有效标识 0(默认):无效 1:有效
        string strErrCode;     //< 证书无效原因码
        string strEndTime;     //< 证书终止有效期
        string strSecrecy;     //< 保密属性 0(默认):不涉密 1:涉密
        string strIPAddress;   //< IP地址
        string strPort;        //< 端口
        string strPassword;    //< 密码
        string strStatus;      //< 状态
        string strLongitude;   //< 经度
        string strLatitude;    //< 纬度

        // 以下扩展信息
        string strPTZType;             //< 摄像机类型 1:球机 2:半球 3:固定枪机 4:遥控枪机
        string strPositionType;        //< 位置类型 1:省际检查站 2:党政机关 3:车站码头 4:中心广场 5:体育场馆 6:商业中心 7:宗教场所 8:学校周边 9:治安复杂区域 10:交通干线
        string strRoomType;            //< 1(默认):室外 2:室内
        string strUseType;             //< 用途属性  1:治安 2:交通  3:重点
        string strSupplyLightType;     //< 补光属性 1(默认):无补光 2:红外补光 3:白光补光
        string strDirectionType;       //< 方向属性 1:东 2:西 3:南 4:北 5:东南 6:东北 7:西南 8:西北
        string strResolution;          //< 分辨率
        string strBusinessGroupID;     //< 虚拟组织所属的业务分组ID
        string strDownloadSpeed;       //< 下载倍速范围
        string strSVCSpaceSupportType; //< 空域编码能力 0:不支持 1:一级增强 2:二级增强 3:三级增强
        string strSVCTimeSupportType;  //< 时域编码能力 0:不支持 1:一级增强 2:二级增强 3:三级增强
    };

    string FormatDevInfo(DevInfo *dev, bool ansi2utf8 = false);

    void TransDevInfo(string json, DevInfo *dev, bool utf82ansi = false);

    /** 初始化 */
    bool SIP_API Init();

    /** 清理环境 */
    void SIP_API Cleanup();

    /**
     * 开启一个实时播放
     * @param[in] strProName 进程名称
     * @param[in] nID 请求ID
     * @param[in] strDev 设备编码
     * @return true:成功 false:失败
     */
    bool SIP_API RealPlay(string strProName, uint32_t nID, string strDev);

    /**
     * 关闭一个实时播放
     * @param[in] rtpPort 用端口作为id
     * @return true:成功 false:失败
     */
    bool SIP_API StopPlay(uint32_t rtpPort);

    /**
     * 关闭所有播放
     */
    bool SIP_API StopPlayAll(string strProName);

    /**
     * 开启一个历史视频播放
     * @param[in] strDev 设备编码
     * @param startTime 开始时间
     * @param endTime 结束时间
     * @param session 会话session
     * @return true:成功 false:失败
     */
    bool SIP_API RecordPlay(string strProName, string strDev, string startTime, string endTime);

    /**
     * 云台控制
     * @param strDev[in] 设备编码
     * @param nInOut[in]     镜头放大缩小 0:停止 1:缩小 2:放大
     * @param nUpDown[in]    镜头上移下移 0:停止 1:上移 2:下移
     * @param nLeftRight[in] 镜头左移右移 0:停止 1:左移 2:右移
     */
    bool SIP_API DeviceControl(string strDev,
        int nInOut = 0, int nUpDown = 0, int nLeftRight = 0);

    /**
     * 查询设备目录
     */
	bool SIP_API QueryDirtionary();

    /**
     * 设备状态更新回调
     * @cb 回调函数
     * @cb @strDevID 设备ID
     * @cb @nOnline 在线状态
     */
    typedef void (*UPDATE_STATUS_CB)(string strDevID, string strOnline);
    void SetUpdateStatusCB(UPDATE_STATUS_CB cb);

    /**
     * 设置GPS更新回调
     */
    typedef void (*UPDATE_POSITION_CB)(string strDevID, string strLog, string strLat);
    void SetUpdatePostionCB(UPDATE_POSITION_CB cb);

    /**
     * 设置新增设备回调
     */
    typedef void (*ADD_DEVICE_CB)(DevInfo* dev);
    void SetDeviceCB(ADD_DEVICE_CB cb);

    /**
     * 播放请求回调
     */
    typedef void (*PLAY_CB)(string strProName, bool bRet, uint32_t nID, uint32_t nPort, string strInfo);
    void SetPlayCB(PLAY_CB cb);
};

