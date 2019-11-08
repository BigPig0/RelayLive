#pragma once

/** 设备目录查询信息应答中的设备信息 */
struct DevInfo
{
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

/**
 * 向本服务器注册的下级平台信息
 */
struct PlatFormInfo
{
    string strDevCode;    //< 国标编码
    string strAddrIP;     //< IP地址
    string strAddrPort;   //< 端口
    string strStatus;     //< 状态
    int    nExpire;       //< 超时时间
};


/**
 * 记录设备信息的映射关系
 * key:平台编码+设备编码
 * value:设备信息
 */
typedef map<string,DevInfo*> mapDevice;