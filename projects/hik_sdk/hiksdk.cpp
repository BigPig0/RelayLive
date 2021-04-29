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

    struct HikData {
        int32_t nLoginID;       //< �����豸��½���
        int32_t nPlayID;        //< ���Ž��
    };

    static void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser) {
        Log::error("Exception 0X%X", dwType);
    }

    static void CALLBACK RealData(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void* pUser) {
        CLiveWorker* lw = (CLiveWorker*)pUser;
        switch (dwDataType) {
        case NET_DVR_SYSHEAD: //ϵͳͷ
           break;
        case NET_DVR_STREAMDATA:   //��������
            if (dwBufSize > 0) {
                lw->push_ps_data((char*)pBuffer, dwBufSize);
                //static FILE *f = fopen("1.ps", "wb+");
                //fwrite(pBuffer, 1, dwBufSize, f);
                //fflush(f);
            }
            break;
        }

    }

    static NET_DVR_TIME makeTime(std::string strTime) {
        NET_DVR_TIME ret;
        ret.dwYear  = stoi(strTime.substr(0,4));
        ret.dwMonth = stoi(strTime.substr(4,2));
        ret.dwDay   = stoi(strTime.substr(6,2));
        ret.dwHour  = stoi(strTime.substr(8,2));
        ret.dwMinute= stoi(strTime.substr(10,2));
        ret.dwSecond= stoi(strTime.substr(12,2));
        return ret;
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
        HikData *hik = new HikData();
        pWorker->m_pPlay = hik;

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
        hik->nLoginID = lUserID;
        Log::debug("Login host(%s) success (%s,%s)"
            , pWorker->m_pParam->strHost.c_str()
            , pWorker->m_pParam->strUsr.c_str()
            , pWorker->m_pParam->strPwd.c_str());

        // ���ŵ�ͨ����
        LONG lChannel = pWorker->m_pParam->nChannel;
        uint32_t IpChanNum = (uint32_t)struDeviceInfoV40.struDeviceV30.byHighDChanNum << 8;
        IpChanNum += (uint32_t)struDeviceInfoV40.struDeviceV30.byIPChanNum;
        if(IpChanNum > 0) {
            lChannel = pWorker->m_pParam->nChannel + struDeviceInfoV40.struDeviceV30.byStartDChan - 1; //IPͨ����
        } else {
            lChannel = pWorker->m_pParam->nChannel + struDeviceInfoV40.struDeviceV30.byStartChan - 1; //ģ��ͨ����
        }

        if(pWorker->m_pParam->strBeginTime.size() >= 14 && pWorker->m_pParam->strEndTime.size() >= 14)  {
            NET_DVR_RECORD_TIME_SPAN_INQUIRY timeSpanInquiry = {sizeof(NET_DVR_RECORD_TIME_SPAN_INQUIRY), 0};
            NET_DVR_RECORD_TIME_SPAN timeSpan = {sizeof(NET_DVR_RECORD_TIME_SPAN), 0};
            if(NET_DVR_InquiryRecordTimeSpan(lUserID, lChannel, &timeSpanInquiry, &timeSpan)) {
                Log::debug("beginTime:%04d%02d%02d %02d%02d%02d endTime:%04d%02d%02d %02d%02d%02d", 
                    timeSpan.strBeginTime.dwYear, timeSpan.strBeginTime.dwMonth, timeSpan.strBeginTime.dwDay,
                    timeSpan.strBeginTime.dwHour, timeSpan.strBeginTime.dwMinute, timeSpan.strBeginTime.dwSecond,
                    timeSpan.strEndTime.dwYear, timeSpan.strEndTime.dwMonth, timeSpan.strEndTime.dwDay,
                    timeSpan.strEndTime.dwHour, timeSpan.strEndTime.dwMinute, timeSpan.strEndTime.dwSecond);
            } else {
                DWORD errorCode = NET_DVR_GetLastError();
                Log::error("InquiryRecordTimeSpan host(%s) Login(%s,%s) failed:%d"
                    , pWorker->m_pParam->strHost.c_str()
                    , pWorker->m_pParam->strUsr.c_str()
                    , pWorker->m_pParam->strPwd.c_str()
                    , errorCode);
            }

            //����ʱ�䲥����ʷ��Ƶ
            NET_DVR_TIME beginTime = makeTime(pWorker->m_pParam->strBeginTime);
            NET_DVR_TIME endTime   = makeTime(pWorker->m_pParam->strEndTime);
            NET_DVR_VOD_PARA vodPara = {0};
            vodPara.dwSize = sizeof(NET_DVR_VOD_PARA);
            vodPara.struIDInfo.dwSize = sizeof(NET_DVR_STREAM_INFO);
            vodPara.struIDInfo.dwChannel = lChannel;
            vodPara.struBeginTime = beginTime;
            vodPara.struEndTime = endTime;
            hik->nPlayID = NET_DVR_PlayBackByTime_V40(lUserID, &vodPara);
            if (hik->nPlayID >= 0) {
                if(!NET_DVR_SetPlayDataCallBack_V40(hik->nPlayID, RealData, pWorker)) {
                    DWORD errorCode = NET_DVR_GetLastError();
                    Log::error("SetPlayDataCallBack host(%s) Login(%s,%s) failed:%d"
                        , pWorker->m_pParam->strHost.c_str()
                        , pWorker->m_pParam->strUsr.c_str()
                        , pWorker->m_pParam->strPwd.c_str()
                        , errorCode);
                }
                if(!NET_DVR_PlayBackControl(hik->nPlayID,NET_DVR_PLAYSTART,0,NULL)) {
                    DWORD errorCode = NET_DVR_GetLastError();
                    Log::error("PlayBack host(%s) Login(%s,%s) failed:%d"
                        , pWorker->m_pParam->strHost.c_str()
                        , pWorker->m_pParam->strUsr.c_str()
                        , pWorker->m_pParam->strPwd.c_str()
                        , errorCode);
                }
            }
        } else {
            // ʵʱ������Ƶ
            NET_DVR_PREVIEWINFO PreviewInfo = {0};
            PreviewInfo.lChannel = lChannel;
            PreviewInfo.dwStreamType = 0;               // �������ͣ�0-��������1-��������2-����3��3-����4, 4-����5,5-����6,7-����7,8-����8,9-����9,10-����10
            PreviewInfo.dwLinkMode = 0;                 // 0��TCP��ʽ,1��UDP��ʽ,2���ಥ��ʽ,3 - RTP��ʽ��4-RTP/RTSP,5-RSTP/HTTP
            PreviewInfo.hPlayWnd = NULL;                // ���Ŵ��ڵľ��,ΪNULL��ʾ������ͼ��
            PreviewInfo.bBlocked = 0;                   // 0-������ȡ��, 1-����ȡ��, �������SDK�ڲ�connectʧ�ܽ�����5s�ĳ�ʱ���ܹ�����,���ʺ�����ѯȡ������.
            PreviewInfo.bPassbackRecord = 0;            // 0-������¼��ش�,1����¼��ش�
            PreviewInfo.byPreviewMode = 0;              // Ԥ��ģʽ��0-����Ԥ����1-�ӳ�Ԥ��
            hik->nPlayID = NET_DVR_RealPlay_V40(lUserID, &PreviewInfo, RealData, pWorker);
            //if(pWorker->m_nPlayID >= 0) {
            //    NET_DVR_SetRealDataCallBack(pWorker->m_nPlayID, RealData, (DWORD)pWorker);
            //}
       }

        if (hik->nPlayID < 0) {
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
        HikData *hik = (HikData*)pWorker->m_pPlay;
        if(pWorker->m_pParam->strBeginTime.size() >= 14 && pWorker->m_pParam->strEndTime.size() >= 14)  {
            NET_DVR_StopPlayBack(hik->nPlayID);
        } else {
            NET_DVR_StopRealPlay(hik->nPlayID);
        }
        NET_DVR_Logout(hik->nLoginID);
        delete hik;
        Sleep(2000);
    }
}