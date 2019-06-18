// sever.cpp : 定义控制台应用程序的入口点。
//
#include "common.h"
#include "DeviceMgr.h"
#include "SipInstance.h"
#include "MiniDump.h"
#include "uvIpc.h"
#include "utilc_api.h"
#include "stdio.h"

uv_ipc_handle_t* h = NULL;

static string strfind(char* src, char* begin, char* end){
    char *p1, *p2;
    p1 = strstr(src, begin);
    if(!p1) return "";
    p1 += strlen(begin);
    p2 = strstr(p1, end);
    if(p2) return string(p1, p2-p1);
    else return string(p1);
}

void on_ipc_recv(uv_ipc_handle_t* h, void* user, char* name, char* msg, char* data, int len)
{
    if (!strcmp(msg,"live_play")) {
        // ssid=123&rtpip=1.1.1.1&rtpport=50000
        data[len] = 0;
        string ssid = strfind(data, "ssid=", "&");
        string ip = strfind(data, "rtpip=", "&");
        string port = strfind(data, "rtpport=", "&");

        string sdp;
        bool bplay = SipInstance::RealPlay(ssid, ip, stoi(port), sdp);
        if(bplay) {
            stringstream ss;
            ss << "ssid=" << port << "&ret=0&error=" << sdp;
            string str = ss.str();
            uv_ipc_send(h, "liveDest", "live_play_answer", (char*)str.c_str(), str.size());
        } else {
            stringstream ss;
            ss << "ssid=" << port << "&ret=-1&error=sip play failed";
            string str = ss.str();
            uv_ipc_send(h, "liveDest", "live_play_answer", (char*)str.c_str(), str.size());
        }
    } else if(!strcmp(msg,"stop_play")) {
        string ssid(data, len);

        SipInstance::StopPlay(ssid);
    } else if(!strcmp(msg,"close")) {
        //关闭所有正在进行的播放
        SipInstance::StopPlayAll();
    } else if(!strcmp(msg,"devices_list")) {
		//string user(data, len);
		vector<DevInfo*> vecDev = DeviceMgr::GetDeviceInfo();
        string strResJson = "{\"root\":[";
        for (auto dev:vecDev)
        {
            strResJson += "{";
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
                strResJson += EncodeConvert::AtoUTF8(dev->strName);
                strResJson += "\",";
            }
            if (!dev->strManuf.empty())
            {
                strResJson += "\"Manufacturer\":\"";
                strResJson += EncodeConvert::AtoUTF8(dev->strManuf);
                strResJson += "\",";
            }
            if (!dev->strModel.empty())
            {
                strResJson += "\"Model\":\"";
                strResJson += EncodeConvert::AtoUTF8(dev->strModel);
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
                strResJson += EncodeConvert::AtoUTF8(dev->strAddress);
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
            strResJson += "},";
        }
        strResJson = StringHandle::StringTrimRight(strResJson,',');
        strResJson += "]}";

		//stringstream ss;
		//ss << "ssid=" << user << "devlist=" << strResJson;
		//string str = ss.str();
        uv_ipc_send(h, "liveDest", "dev_list_answer", (char*)strResJson.c_str(), strResJson.size());
	} else if(!strcmp(msg,"QueryDirtionary")) {
        //查询设备
        SipInstance::QueryDirtionary();
    } else if(!strcmp(msg, "DeviceControl")) {
		data[len] = 0;
        string strDev = strfind(data, "dev=", "&");
        string strIO = strfind(data, "io=", "&");
        string strUD = strfind(data, "ud=", "&");
        string strLR = strfind(data, "lr=", "&");
		SipInstance::DeviceControl(strDev, stoi(strIO), stoi(strUD), stoi(strLR));
	}
}

int main()
{
    /** Dump设置 */
    CMiniDump dump("sipServer.dmp");

    /** 进程间通信 */
    int ret = uv_ipc_client(&h, "relay_live", NULL, "liveSrc", on_ipc_recv, NULL);
    if(ret < 0) {
        printf("ipc server err: %s\n", uv_ipc_strerr(ret));
    }

    /** 创建日志文件 */
    char path[MAX_PATH];
    sprintf_s(path, MAX_PATH, ".\\log\\sipServer.txt");
    Log::open(Log::Print::both, Log::Level::debug, path);
    Log::debug("version: %s %s", __DATE__, __TIME__);

    /** 加载配置文件 */
    if (!Settings::loadFromProfile(".\\config.txt"))
    {
        Log::error("配置文件错误");
        return -1;
    }
    Log::debug("Settings::loadFromProfile ok");


    /** 初始化设备模块 */
    if (!DeviceMgr::Init())
    {
        Log::error("DeviceManagerInstance init failed");
        return -1;
    }
    Log::debug("DeviceMgr::Init ok");

    /** 初始化SIP服务器 */
    if (!SipInstance::Init())
    {
        Log::error("SipInstance init failed");
        return -1;
    }
    Log::debug("SipInstance::Init ok");
    
    sleep(INFINITE);
    return 0;
}