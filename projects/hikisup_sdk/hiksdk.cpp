#include "util.h"
#include "hiksdk.h"
#include "uvnetplus.h"
#include "easylog.h"
#include "database.h"
#include "Task.h"
#include "worker.h"
#include "server.h"
#ifdef WIN32
#include <windows.h>
#include "HCISUPCMS.h"
#include "HCISUPAlarm.h"
#include "HCISUPStream.h"
#else
#include <hcisup/HCISUPCMS.h>
#include <hcisup/HCISUPAlarm.h>
#include <hcisup/HCISUPStream.h>
#include <memory.h>
#endif
#include <stdint.h>

#ifdef WINDOWS_IMPL
#pragma comment(lib,"HCISUPCMS.lib")
#pragma comment(lib,"HCISUPAlarm.lib")
#pragma comment(lib,"HCISUPStream.lib")
#endif

using namespace util;
using namespace uvNetPlus;
using namespace uvNetPlus::Http;

namespace HikSdk {

    struct PlayInfo {
        long iUserId;
        long iHandle;
        long iSession;
    };

    //static CNet *cnet;
    //static CHttpClient *httpClient;
    static LONG iListenHandle;
//     static string svrip;
//     static int svrport;
    static map<LONG, CLiveWorker*> workerMap;
    static map<CLiveWorker*, PlayInfo> handleMap;
    static CriticalSection cs;

    LONG lLoginID = -1;
    LONG lListenHandle = -1;
    LONG lLinkHandle = -1;
    LONG lRealHandle = -1;
    FILE *Videofile = NULL;
    static uint32_t nCmsPort = 7660;
    static std::string strAmsIp;
    static uint32_t nAmsPort = 7200;
    static std::string strSmsIp;
    static uint32_t nSmsport;

    //�豸ID��ע��IDӳ��
    static map<string, LONG> g_mapUsers;
    static CriticalSection g_csUsers;

//////////////////////////////////////////////////////////////////////
//ע��ص�����
BOOL CALLBACK RegisterCallBack(LONG lUserID, DWORD dwDataType, void *pOutBuffer, DWORD dwOutLen, void *pInBuffer, DWORD dwInLen, void *pUser)
{
    if (ENUM_DEV_ON == dwDataType) {
        NET_EHOME_DEV_REG_INFO *pDevInfo = (NET_EHOME_DEV_REG_INFO *)pOutBuffer;
        if (pDevInfo != NULL)
        {
            lLoginID = lUserID;
            Log::debug("On-line, lUserID: %d, Device ID: %s", lLoginID, pDevInfo->byDeviceID);
            string devid = (char*)(pDevInfo->byDeviceID);
            g_csUsers.lock();
            if(g_mapUsers.count(devid) > 0)
                g_mapUsers[devid] = lUserID;
            else
                g_mapUsers.insert(make_pair(devid, lUserID));
            g_csUsers.unlock();
        }
        //�������
        NET_EHOME_SERVER_INFO *pServerInfo = (NET_EHOME_SERVER_INFO *)pInBuffer;
        pServerInfo->dwTimeOutCount = 6; //������ʱ����
        pServerInfo->dwKeepAliveSec = 15; //�������
        memcpy(pServerInfo->struUDPAlarmSever.szIP, strAmsIp.c_str(), strAmsIp.size()); //���������� IP ��ַ��TCP Э�飩
        pServerInfo->struUDPAlarmSever.wPort = nAmsPort; //�����������˿ڣ�UDP Э�飩����Ҫ�ͱ������������������Ķ˿�һ��
        pServerInfo->dwAlarmServerType = 0; //�������������ͣ�0- ֻ֧�� UDP Э���ϱ���1- ֧�� UDP�� TCP ����Э���ϱ�
    } else if (ENUM_DEV_OFF == dwDataType) {
        Log::debug("Off-line, lUserID: %d", lUserID);
        NET_ECMS_ForceLogout(lUserID);
        g_csUsers.lock();
        auto it = g_mapUsers.begin();
        for(; it != g_mapUsers.end(); ++it) {
            if(it->second == lUserID) {
                g_mapUsers.erase(it);
                break;
            }
        }
        g_csUsers.unlock();
    } else {
        Log::debug("%d, lUserID: %d", dwDataType, lUserID);
    }
    return TRUE;
}

//////////////////////////////////////////////////////////////////////
static double posTrans(DWORD pos) {
    DWORD d = pos/360000;
    DWORD a = pos%360000;
    DWORD m = a/6000;
    double s = (double)(a%6000)/100.0;
    return (double)d + (double)m/60.0 + (double)s/3600.0;
}

//AMS �����ص�����
BOOL CALLBACK AlarmMSGCallBack(LONG lHandle, NET_EHOME_ALARM_MSG *pAlarmMsg, void *pUserData) {
    lListenHandle = lHandle;
    DWORD dwType = pAlarmMsg->dwAlarmType; //��ͬ�ı�������(dwAlarmType)��pAlarmInfo ��Ӧ��ͬ�ı�����Ϣ����
    
    if(dwType == EHOME_ALARM_GPS) {//GPS ��Ϣ�ϴ�
        NET_EHOME_GPS_INFO *struGpsInfo = (NET_EHOME_GPS_INFO *)pAlarmMsg->pAlarmInfo;
        string devid = struGpsInfo->byDeviceID;
        VipTask tskInfo;
        if(DbTsk::CheckDeviceId(devid, tskInfo)) {
            Gps *gps = new Gps;
            gps->taskId = tskInfo.taskId;
            gps->realTime = struGpsInfo->bySampleTime;
            gps->lon = posTrans(struGpsInfo->dwLongitude);
            gps->lat = posTrans(struGpsInfo->dwLatitude);
            gps->angle = struGpsInfo->dwDirection / 100.0;
            gps->speed = struGpsInfo->dwSpeed / 100000.0;
            Log::debug("GPS info: DeviceID:%s, SampleTime:%s, gps:%lf, %lf direction:%lf speed:%lf",
                struGpsInfo->byDeviceID, struGpsInfo->bySampleTime, gps->lon, gps->lat, gps->angle, gps->speed);
            Task::addTask(gps);
        }
    } else {
        Log::debug("Callback of alarm listening, dwAlarmType[%d]", dwType);
    }
    return TRUE;
}
	
//////////////////////////////////////////////////////////////////////
//ע��ʵʱ�����ص�����
void CALLBACK fnPREVIEW_DATA_CB(LONG lPreviewHandle, NET_EHOME_PREVIEW_CB_MSG *pPreviewCBMsg, void *pUserData)
{
    if (NULL == pPreviewCBMsg) {
        return;
    }
    //Log::debug("data type:%d length:%d", pPreviewCBMsg->byDataType, pPreviewCBMsg->dwDataLen);
    printf(".");
    CLiveWorker* lw = (CLiveWorker*)pUserData;
    if(pPreviewCBMsg->byDataType == 2 && pPreviewCBMsg->dwDataLen > 0) {
        lw->push_ps_data((char*)pPreviewCBMsg->pRecvdata, pPreviewCBMsg->dwDataLen);
    }
}
//////////////////////////////////////////////////////////////////////
//ע��Ԥ���������Ӧ�ص�����
BOOL CALLBACK fnPREVIEW_NEWLINK_CB(LONG lPreviewHandle, NET_EHOME_NEWLINK_CB_MSG *pNewLinkCBMsg, void *pUserData)
{
    Log::debug("Callback of preview listening handle:%d, session:%d, DeviceID:%s, Channel:%d", lPreviewHandle, pNewLinkCBMsg->iSessionID,pNewLinkCBMsg->szDeviceID, pNewLinkCBMsg->dwChannelNo);
     CLiveWorker* lw = NULL;
     cs.lock();
     if(workerMap.count(pNewLinkCBMsg->iSessionID) > 0) {
         lw = workerMap[pNewLinkCBMsg->iSessionID];
         if(handleMap.count(lw) > 0) {
             handleMap[lw].iHandle = lPreviewHandle;
         }
     }
     cs.unlock();
     if(lw == NULL) {
         Log::error("preview session not found: %d", pNewLinkCBMsg->iSessionID);
         return FALSE;
     }
    //Ԥ�����ݵĻص�����
    NET_EHOME_PREVIEW_DATA_CB_PARAM struDataCB = {0};
    struDataCB.pUserData = lw;
    struDataCB.fnPreviewDataCB = fnPREVIEW_DATA_CB;
    struDataCB.byStreamFormat = 0;//��װ��ʽ��0-PS ��ʽ
    if (!NET_ESTREAM_SetPreviewDataCB(lPreviewHandle, &struDataCB))
    {
        Log::debug("NET_ESTREAM_SetPreviewDataCB failed, error code: %d", NET_ESTREAM_GetLastError());
        return FALSE;
    }
    Log::debug("NET_ESTREAM_SetPreviewDataCB!");
    return TRUE;
}

	bool Init(uv_loop_t *loop) {
        //hik����
        nCmsPort = util::Settings::getValue("Hik", "emsport", 7660);
        strAmsIp = util::Settings::getValue("Hik", "amsip");
        nAmsPort = util::Settings::getValue("Hik", "amsport", 7200);
        strSmsIp = util::Settings::getValue("Hik", "smsip");
        nSmsport = util::Settings::getValue("Hik", "smsport", 8003);
//         svrip = util::Settings::getValue("HttpServer", "ip");
//         svrport = util::Settings::getValue("HttpServer", "port", 80);
		LONG lHandle = -1;

        //cnet = CNet::Create(loop);
        //httpClient = new CHttpClient(cnet);
        ////////////////////////////////////////////////////////////////////////
        //SMS �ڼ������������ȡ����
        //��ʼ�� SMS ��
        NET_ESTREAM_Init();
        //Ԥ���ļ�������
        NET_EHOME_LISTEN_PREVIEW_CFG esListen = {0};
        memcpy(esListen.struIPAdress.szIP, "0.0.0.0", sizeof("0.0.0.0"));
        esListen.struIPAdress.wPort = nSmsport; //SMS �ļ����˿ں�
        esListen.fnNewLinkCB = fnPREVIEW_NEWLINK_CB; //Ԥ������ص�����
        esListen.pUser = NULL;
        esListen.byLinkMode = 0; //0-TCP, 1-UDP
        //������������
        iListenHandle = NET_ESTREAM_StartListenPreview(&esListen);
        if(iListenHandle < -1) {
            Log::error("NET_ESTREAM_StartListenPreview failed, error code: %d\n", NET_ESTREAM_GetLastError());
            NET_ESTREAM_Fini();
            return false;
        }
        Log::debug("NET_ESTREAM_StartListenPreview!");
	
	    ////////////////////////////////////////////////////////////////////////
        //���� AMS ��������
        //AMS ��ʼ��
        NET_EALARM_Init();
        //������������
        NET_EHOME_ALARM_LISTEN_PARAM struListen = {0};
        memcpy(struListen.struAddress.szIP, "0.0.0.0", sizeof("0.0.0.0"));
        struListen.struAddress.wPort = nAmsPort; //��������ļ����˿�
        struListen.fnMsgCb = AlarmMSGCallBack; //�����ص�����
        struListen.pUserData = NULL;
        struListen.byProtocolType = 1; //0- TCP ��ʽ(�������ݲ�֧��)��1- UDP ��ʽ
        //������������
        lHandle = NET_EALARM_StartListen(&struListen);
        if(lHandle < -1) {
            Log::error("NET_EALARM_StartListen failed, error code: %d", NET_EALARM_GetLastError());
            NET_EALARM_Fini();
            return false;
        }
        Log::debug("NET_EALARM_StartListen!");
        //////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        //CMS ע��ģ���ʼ��
        //NET_ECMS_SetLogToFile(3, "C:\\SdkLog", false);
        NET_ECMS_Init();
        //ע���������
        NET_EHOME_CMS_LISTEN_PARAM struCMSListenPara = {0};
        memcpy(struCMSListenPara.struAddress.szIP, "0.0.0.0", sizeof("0.0.0.0"));
        struCMSListenPara.struAddress.wPort = nCmsPort;
        struCMSListenPara.fnCB = RegisterCallBack;
        //���������������豸ע����Ϣ��ע��ص�����������Ҫ���ͱ������� IP �Ͷ˿ڸ��豸
        LONG lListen = NET_ECMS_StartListen(&struCMSListenPara);
        if(lListen < -1) {
            Log::error("NET_ECMS_StartListen failed, error code: %d", NET_ECMS_GetLastError());
            NET_ECMS_Fini();
            return false;
        }
        Log::debug("NET_ECMS_StartListen!");

        return true;
    }


	void Cleanup() {
        NET_ESTREAM_StopListenPreview(iListenHandle);
		NET_ESTREAM_Fini();
	}

//     static void httpResCB(CHttpRequest *req, CHttpMsg* response) {
//         Log::debug(response->content.c_str());
//         CLiveWorker *pWorker = (CLiveWorker*)req->usrData;
//         if(response->statusCode != 200) {
//             //����ʧ��
//             return;
//         }
//         PlayInfo pi;
//         sscanf(response->content.c_str(),"user=%d&session=%d&&handle=%d", &pi.iUserId, &pi.iSession, &pi.iHandle);
//         cs.lock();
//         if(workerMap.count(pi.iHandle)) {
//             Log::error("this hanle %d is already exist", pi.iHandle);
//         } else if(handleMap.count(pWorker)) {
//             Log::error("this worker %s is already exist", pWorker->m_pParam->strCode.c_str());
//         } else {
//             workerMap.insert(make_pair(pi.iHandle, pWorker));
//             handleMap.insert(make_pair(pWorker, pi));
//         }
//         cs.unlock();
//     }
// 
//     static void httpReqCB(CHttpRequest *req, void* usr, std::string error) {
//         if(!error.empty()) {
//             Log::error("http connect failed: %s", error.c_str());
//             return;
//         }
//         req->usrData = usr;
// 
//         CLiveWorker *pWorker = (CLiveWorker*)usr;
//         req->path = "/Play?code=" + pWorker->m_pParam->strCode 
//             + "&channel=" + to_string(pWorker->m_pParam->nChannel)
//             + "&port=" + to_string(nSmsport);
//         req->method = GET;
//         req->OnResponse = httpResCB;
//         req->End();
//     }

	int Play(CLiveWorker *pWorker) {
        //httpClient->Request(svrip, svrport, pWorker, httpReqCB);
        Log::debug("%s %d devid(%s) channel(%d)", strSmsIp.c_str(), nSmsport, pWorker->m_pParam->strCode.c_str(), pWorker->m_pParam->nChannel);
       
        LONG userId = -1;
        g_csUsers.lock();
        if(g_mapUsers.count(pWorker->m_pParam->strCode) > 0)
            userId = g_mapUsers[pWorker->m_pParam->strCode];
        g_csUsers.unlock();
        if(userId == -1) {
            return -1;
        }

        NET_EHOME_PREVIEWINFO_IN_V11 struParamIn = { 0 };
        struParamIn.dwLinkMode = 0;  // 0��TCP��ʽ,1��UDP��ʽ,2: HRUDP��ʽ  
        struParamIn.struStreamSever.wPort = nSmsport;
        memcpy(struParamIn.struStreamSever.szIP, strSmsIp.c_str(), strSmsIp.size());
        struParamIn.dwStreamType = 1; // �������ͣ�0-��������1-������, 2-��������,3- ��������
        struParamIn.iChannel = pWorker->m_pParam->nChannel;
        struParamIn.byEncrypt = 0;
        NET_EHOME_PREVIEWINFO_OUT struParamOut = { 0 };
        if(!NET_ECMS_StartGetRealStreamV11(userId, &struParamIn, &struParamOut)) {
            Log::error("Get real stream failed %d", NET_ECMS_GetLastError());
            return -1;
        }
        Log::debug("Get real stream ok: user:%d session:%d handle:%d", userId, struParamOut.lSessionID, struParamOut.lHandle);

        PlayInfo pi;
        pi.iUserId = userId;
        pi.iSession = struParamOut.lSessionID;
        pi.iHandle = struParamOut.lHandle;
        cs.lock();
        if(workerMap.count(pi.iSession)) {
            Log::error("this hanle %d is already exist", pi.iSession);
        } else if(handleMap.count(pWorker)) {
            Log::error("this worker %s is already exist", pWorker->m_pParam->strCode.c_str());
        } else {
            workerMap.insert(make_pair(pi.iSession, pWorker));
            handleMap.insert(make_pair(pWorker, pi));
        }
        cs.unlock();
        
		return 0;
	}

    void Stop(CLiveWorker *pWorker) {
        PlayInfo pi = {-1, -1, -1};
        cs.lock();
        if(handleMap.count(pWorker) > 0) {
            pi = handleMap[pWorker];
            handleMap.erase(handleMap.find(pWorker));
            if(workerMap.count(pi.iSession) > 0) {
                workerMap.erase(workerMap.find(pi.iSession));
            }
        }
        cs.unlock();
        if(pi.iUserId == -1) {
            Log::error("play info not exist");
            return;
        } else {
            Log::debug("stop play user:%d handle:%d session:%d", pi.iUserId, pi.iHandle, pi.iSession);
            NET_ESTREAM_StopPreview(pi.iHandle);
            NET_ECMS_StopGetRealStream(pi.iUserId, pi.iSession);
        }

//         httpClient->Request(svrip, svrport, pWorker, [](CHttpRequest *req, void* usr, std::string error){
//             if(!error.empty()) {
//                 Log::error("http connect failed: %s", error.c_str());
//                 return;
//             }
//             CLiveWorker *pWorker = (CLiveWorker*)usr;
//             PlayInfo pi = {-1, -1, -1};
//             cs.lock();
//             if(handleMap.count(pWorker) > 0) {
//                 pi = handleMap[pWorker];
//                 handleMap.erase(handleMap.find(pWorker));
//                 if(workerMap.count(pi.iHandle) > 0) {
//                     workerMap.erase(workerMap.find(pi.iHandle));
//                 }
//             }
//             cs.unlock();
//             req->usrData = usr;
//             req->path = "/Stop?user="+to_string(pi.iUserId)+"&session="+to_string(pi.iSession)+"&handle="+to_string(pi.iHandle);
//             req->method = GET;
//             req->OnResponse = [](CHttpRequest *req, CHttpMsg* response){
//                 CLiveWorker *pWorker = (CLiveWorker*)req->usrData;
//                 Log::debug("stop worker %s ok", pWorker->m_pParam->strCode.c_str());
//             };
//             req->End();
//         });
         Sleep(2000);
    }
}