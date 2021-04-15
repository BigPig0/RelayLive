#include "util.h"
#include "easylog.h"
#include "worker.h"
#include "server.h"
#include "HCNetSDK.h"

#ifdef WINDOWS_IMPL
#pragma comment(lib,"HCCore.lib")
#pragma comment(lib,"HCNetSDK.lib")
#endif


namespace HikSdk {

    static void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser) {
        Log::error("Exception 0X%X", dwType);
    }

    static void CALLBACK RealData(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser) {
        CLiveWorker* lw = (CLiveWorker*)pUser;
        switch (dwDataType) {
        case NET_DVR_SYSHEAD: //系统头
           break;
        case NET_DVR_STREAMDATA:   //码流数据
            if (dwBufSize > 0) {
                lw->push_ps_data((char*)pBuffer, dwBufSize);
            }
            break;
        }

    }

	bool Init() {
        BOOL ret = NET_DVR_Init();
		if(!ret) {
			Log::error("SDK init failed");
			return false;
		}

        //打开日志
        //NET_DVR_SetLogToFile(3, "./sdklog");

        //设置异常消息回调函数
        NET_DVR_SetExceptionCallBack_V30(0, NULL,g_ExceptionCallBack, NULL);

		return true;
	}

	void Cleanup() {
		NET_DVR_Cleanup();
	}

	int Play(CLiveWorker *pWorker) {
        //登录参数，包括设备地址、登录用户、密码等
        NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
        struLoginInfo.bUseAsynLogin = 0; //同步登录方式
        strcpy(struLoginInfo.sDeviceAddress, pWorker->m_pParam->strHost.c_str()); //设备IP地址
        struLoginInfo.wPort = pWorker->m_pParam->nPort; //设备服务端口
        strcpy(struLoginInfo.sUserName, pWorker->m_pParam->strUsr.c_str()); //设备登录用户名
        strcpy(struLoginInfo.sPassword, pWorker->m_pParam->strPwd.c_str()); //设备登录密码
        //struLoginInfo.cbLoginResult = LoginResult;
        //struLoginInfo.pUser = pWorker;

        //设备信息, 输出参数
        NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};
        LONG lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
        if(lUserID < 0) {
            DWORD errorCode = NET_DVR_GetLastError();
            Log::error("Login host(%s) Login(%s,%s) failed:%d"
                , pWorker->m_pParam->strHost.c_str()
                , pWorker->m_pParam->strUsr.c_str()
                , pWorker->m_pParam->strPwd.c_str()
                , errorCode);
            return -1;
        }
        pWorker->m_nLoginID = lUserID;
        Log::debug("Login host(%s) success (%s,%s)"
            , pWorker->m_pParam->strHost.c_str()
            , pWorker->m_pParam->strUsr.c_str()
            , pWorker->m_pParam->strPwd.c_str());

        
        NET_DVR_PREVIEWINFO PreviewInfo = {0};
        uint32_t IpChanNum = (uint32_t)struDeviceInfoV40.struDeviceV30.byHighDChanNum << 8;
        IpChanNum += (uint32_t)struDeviceInfoV40.struDeviceV30.byIPChanNum;
        if(IpChanNum > 0) {
            PreviewInfo.lChannel = pWorker->m_pParam->nChannel + struDeviceInfoV40.struDeviceV30.byStartDChan; //IP通道号
        } else {
            PreviewInfo.lChannel = pWorker->m_pParam->nChannel + struDeviceInfoV40.struDeviceV30.byStartChan; //模拟通道号
        }
        PreviewInfo.dwStreamType = 0;               // 码流类型，0-主码流，1-子码流，2-码流3，3-码流4, 4-码流5,5-码流6,7-码流7,8-码流8,9-码流9,10-码流10
        PreviewInfo.dwLinkMode = 0;                 // 0：TCP方式,1：UDP方式,2：多播方式,3 - RTP方式，4-RTP/RTSP,5-RSTP/HTTP
        PreviewInfo.hPlayWnd = NULL;                // 播放窗口的句柄,为NULL表示不播放图象
        PreviewInfo.bBlocked = 0;                   // 0-非阻塞取流, 1-阻塞取流, 如果阻塞SDK内部connect失败将会有5s的超时才能够返回,不适合于轮询取流操作.
        PreviewInfo.bPassbackRecord = 0;            // 0-不启用录像回传,1启用录像回传
        PreviewInfo.byPreviewMode = 0;              // 预览模式，0-正常预览，1-延迟预览
        pWorker->m_nPlayID = NET_DVR_RealPlay_V40(lUserID, &PreviewInfo, RealData, pWorker);

        if (pWorker->m_nPlayID < 0) {
            DWORD errorCode = NET_DVR_GetLastError();
            Log::error("Play host(%s) Login(%s,%s) failed:%d"
                , pWorker->m_pParam->strHost.c_str()
                , pWorker->m_pParam->strUsr.c_str()
                , pWorker->m_pParam->strPwd.c_str()
                , errorCode);
            NET_DVR_Logout(lUserID);
            return -1;
        }

        Log::debug("Play host(%s) success (%s,%s)"
            , pWorker->m_pParam->strHost.c_str()
            , pWorker->m_pParam->strUsr.c_str()
            , pWorker->m_pParam->strPwd.c_str());

		return 0;
	}

    void Stop(CLiveWorker *pWorker) {
        NET_DVR_StopRealPlay(pWorker->m_nPlayID);
        NET_DVR_Logout(pWorker->m_nLoginID);
        Sleep(2000);
    }
}