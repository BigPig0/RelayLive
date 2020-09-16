#ifndef BRCLIENT_DEFINE_H
#define BRCLIENT_DEFINE_H

#include <string>
#include <windows.h>

#define BRCLIENT_OK    0

#define CLIENT_TYPE_NORMAL    0
#define CLIENT_TYPE_AR                1
#define CLIENT_TYPE_SDK               2


typedef enum
{
    OPER_REAL         = 10001,
    OPER_PTZ           = 10004,
    OPER_TALK         = 10100,

    OPER_CUT          = 10032,

    OPER_CAPTURE   = 10027,

    OPER_PLAYBACK    = 10006,
    OPER_DOWNLOAD = 10014,
    OPER_PB   = 10024
}OPERType;

typedef enum
{
    MATRIX                    = 100000,
    CAMERA                   = 10000,
    DEVICE                     = 30000,
    ORGANIZATION        = 1000,
    USERINFO                 = 2000,
    SERVER                     = 100,
    _SERVER_VTM             = 101,
    _SERVER_ITSCMS        = 102,
    _SERVER_MQ        = 103,
    ITS_CROSS                = 700010,
    ITS_LANE                   = 700020,
    ITS_DIRECTION          = 700030,
    ARCAMERA               = 690100
}RESType;

typedef struct tag_GobalConfig 
{
    bool bTokenEnable;
    bool bNtpTiming;

    std::string strSmtp_IP;
    unsigned int dwSmtp_Port;
    std::string strSmtp_UserName;
    std::string strSmtp_PassWord;

    std::string strAES_Key;
    std::string strAES_Vector;

    //后面一些暂时用不到的字段就不放了
    //     string strUpdate_IP;
    //     DWORD dwUpdate_Port;
    //     bool bUpdate_Force;
    tag_GobalConfig()
    {
        bTokenEnable = 1;
        bNtpTiming = 0;
    }

}BrGobalConfig,*LPBrGobalConfig;

typedef enum
{
    STREM_LINK_RTSP = 1,
    STREM_LINK_TCP,
    STREM_LINK_UDP,
}BRSTREAMLINKTYPE;

typedef enum
{
    STREM_TYPE_MAIN = 0,
    STREM_TYPE_SUB,
    STREM_TYPE_THIRD,
}BRSTREAMTYPE;

typedef enum
{
    PTZ_TZ = 1,   //全方位云台(带转动和变焦)
    PTZ_Z,          //只有变焦，不带转动
    PTZ_T,          //只有转动，不带变焦
    PTZ_NULL,    //无云台
}BRPTZTYPE;

typedef struct tag_Camera
{
    int iCameraID;
    int iOrgID;
    std::string strIndex;
    std::string strDevIndex;
    std::string strName;
    unsigned int uChannelNo;
    unsigned int uCameraType;
    unsigned int uRecordLocation;

    std::string strVrmIndex;
    
    BRSTREAMLINKTYPE eStreamLinkType;
    BRSTREAMTYPE eStreamType;
    BRPTZTYPE ePtzType;

    bool bAudio;
    bool bOnline;
    
    std::string strPyCodeAll;
    std::string strPyCode;

    std::string strCascadeCode;

    std::string strDecodeTag;

    int iSequenceIdx;
    
    int iChannelIndexCode;
    int iTalkChennelNo;

    std::string strOrgName;
    std::string strOrgIndex;

    std::string strDeviceIP;
    int iDevicePort;

    int iManufacturer;
    int iDeviceType;
    int iDeviceDetailType;

    bool bPrivilege;
    std::string strPrivilege;

    tag_Camera* pNext;
    tag_Camera()
    {
        iCameraID = -1;
        iOrgID = 0;
        uChannelNo = 0;
        uCameraType = 0;
        uRecordLocation = 0;
        eStreamLinkType = STREM_LINK_RTSP;
        eStreamType = STREM_TYPE_MAIN;
        ePtzType = PTZ_TZ;
        bAudio = 0;
        bOnline = 0;
        iSequenceIdx = 0;
        iChannelIndexCode = 0;
        iTalkChennelNo = 0;
        iDevicePort = 0;
        iManufacturer = 0;
        iDeviceType = 0;
        iDeviceDetailType = 0;
        bPrivilege = false;
        pNext = NULL;
    }
}BrCamera,*lpBrCamera;

typedef struct tag_Device
{
    int iDeviceID;
    int iOrgID;

    std::string strName;
    int iRegisterType;
    int iDevType;

    std::string strIp;
    int iPort;

    std::string strUsername;
    std::string strUserPwd;

    int iManufacturer;
    int iCameraCount;
    int iIpChanCount;

    int iTalkCount;
    int iOnLine;

    std::string strTreatyType;
    std::string strIndex;

    int iNetDomainID;

    int iResType;

    std::string strVagIndex;
    std::string strDagIndex;

    std::string strDeviceKey;
    bool bSmart;
    bool bEnable;

    std::string strOrgName;
    std::string strOrgIndex;

    tag_Device* pNext;
}BrDevice,*lpBrDevice;


typedef struct tag_Cross
{
    int iCrossID;
    std::string strIndex;
    std::string strCrossingIndex;
    std::string strName;

    int iOrgID;
    int iFrontType;
    
    int iSpeedLimitBig;
    int iSpeedLimitSmall;

    std::string strTdaIndex;
    std::string strDevIndex;

    std::string strPmsIndex;
    
    bool bDel;

    int iSequenceIdx;

    std::string strOrgIndex;
    tag_Cross* pNext;
}BrCross, *lpBrCross;

typedef enum
{
    UNACHIEVE = 0,
    ACHIEVING,
    ACHIEVED,
}BRREPTILESTATUS;

typedef enum
{
    SERVERTYPE_VAG = 3,
    SERVERTYPE_VTM = 4,
    SERVERTYPE_VRM = 5,
    SERVERTYPE_NCG = 8,
    SERVERTYPE_TDDC = 8603,
    SERVERTYPE_ITSCMS = 8653,
    SERVERTYPE_MQ = 71,
    
}BrServerType;

typedef struct tag_Organ
{
    int iOrgID;
    std::string strDomain;
    std::string strName;
    std::string strIndex;
    int iParentID;
    std::string strPath;
    bool bDel;
    unsigned int uOrder;
    unsigned int uCameraCount;
    unsigned int uCameraOnlineCount;

    int bPreviewStream;

    std::string strCascadeCode;

    lpBrCamera pCamera;
    lpBrDevice pDevice;
    lpBrCross pCross;
    tag_Organ* pNext;
    tag_Organ* pSub;


    BRREPTILESTATUS eReptileStatus;
    HANDLE hEventStatus;
}BrOrganization,*lpBrOrganization;



typedef struct tag_ServerInfo
{
    int iServerID;
    int iType;
    std::string strName;
    std::string strIndex;
    std::string strIP;
    int iPort;

}BrServerInfo, *lpBrServerInfo;


typedef struct tag_BrOperationLog
{
    std::string strUserName;
    LONG64 lUserOrgID;
    std::string strUserOrgName;
    std::string strIpAddress;
    std::string strMac;
    std::string strObjectType;
    std::string strObjectKeys;
    std::string strObjectValues;
    std::string strObjectOrgIds;
    std::string strObjectOrgNames;
    int iObjectResult;
    int iLogLevel;
    std::string strBusiness;
    std::string strAct;
    std::string strContent;
    std::string strParamInfo;
}BrOperationLog,*lpBrOperationLog;


typedef struct tag_BrARCamera
{
    int iID;
    std::string strIndex;
    int iOrgID;
    std::string strCamIndex;
    std::string strName;
    std::string strOrgIndex;    
}BrARCamera, *lpBrARCamera;
typedef struct tag_BrAppFunInfo
{
    int iAppId;
    int igroupId;
    int iStatus;
    std::string strExportOpType;
    std::string strExportType;
    std::string strFunctionCode;
    std::string strFunctionName;   
    std::string strLicenseKey;
    std::string strType;
    std::string strUrl;    
}BrAppFunInfo, *lpBrAppFunInfo;
#endif