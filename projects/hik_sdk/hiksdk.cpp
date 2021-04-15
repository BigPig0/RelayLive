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
        case NET_DVR_SYSHEAD: //ϵͳͷ
           break;
        case NET_DVR_STREAMDATA:   //��������
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

        //����־
        //NET_DVR_SetLogToFile(3, "./sdklog");

        //�����쳣��Ϣ�ص�����
        NET_DVR_SetExceptionCallBack_V30(0, NULL,g_ExceptionCallBack, NULL);

		return true;
	}

	void Cleanup() {
		NET_DVR_Cleanup();
	}

	int Play(CLiveWorker *pWorker) {
        //��¼�����������豸��ַ����¼�û��������
        NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
        struLoginInfo.bUseAsynLogin = 0; //ͬ����¼��ʽ
        strcpy(struLoginInfo.sDeviceAddress, pWorker->m_pParam->strHost.c_str()); //�豸IP��ַ
        struLoginInfo.wPort = pWorker->m_pParam->nPort; //�豸����˿�
        strcpy(struLoginInfo.sUserName, pWorker->m_pParam->strUsr.c_str()); //�豸��¼�û���
        strcpy(struLoginInfo.sPassword, pWorker->m_pParam->strPwd.c_str()); //�豸��¼����
        //struLoginInfo.cbLoginResult = LoginResult;
        //struLoginInfo.pUser = pWorker;

        //�豸��Ϣ, �������
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
            PreviewInfo.lChannel = pWorker->m_pParam->nChannel + struDeviceInfoV40.struDeviceV30.byStartDChan; //IPͨ����
        } else {
            PreviewInfo.lChannel = pWorker->m_pParam->nChannel + struDeviceInfoV40.struDeviceV30.byStartChan; //ģ��ͨ����
        }
        PreviewInfo.dwStreamType = 0;               // �������ͣ�0-��������1-��������2-����3��3-����4, 4-����5,5-����6,7-����7,8-����8,9-����9,10-����10
        PreviewInfo.dwLinkMode = 0;                 // 0��TCP��ʽ,1��UDP��ʽ,2���ಥ��ʽ,3 - RTP��ʽ��4-RTP/RTSP,5-RSTP/HTTP
        PreviewInfo.hPlayWnd = NULL;                // ���Ŵ��ڵľ��,ΪNULL��ʾ������ͼ��
        PreviewInfo.bBlocked = 0;                   // 0-������ȡ��, 1-����ȡ��, �������SDK�ڲ�connectʧ�ܽ�����5s�ĳ�ʱ���ܹ�����,���ʺ�����ѯȡ������.
        PreviewInfo.bPassbackRecord = 0;            // 0-������¼��ش�,1����¼��ش�
        PreviewInfo.byPreviewMode = 0;              // Ԥ��ģʽ��0-����Ԥ����1-�ӳ�Ԥ��
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