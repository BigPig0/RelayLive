#include "common.h"
#include "LiveClient.h"
#include "LiveIpc.h"
#include "LiveWorker.h"

namespace LiveClient
{
    uv_loop_t *g_uv_loop = NULL;

    string g_strRtpIP;            //< RTP����IP
    int    g_nRtpBeginPort;       //< RTP��������ʼ�˿ڣ�������ż��
    int    g_nRtpPortNum;         //< RTPʹ�õĸ�������strRTPPort��ʼÿ�μ�2����strRTPNum��
    int    g_nRtpCatchPacketNum;  //< rtp����İ�������
    int    g_nRtpStreamType;      //< rtp�������ͣ�����libLive��ps h264

    list<int>     g_vecRtpPort;     //< RTP���ö˿ڣ�ʹ��ʱ����ȡ����ʹ�ý������·���
    CriticalSection m_csRTP;          //< RTP�˿���

    LIVECLIENT_CB liveclient_respond = NULL;

    void Init(void* uv){
        g_uv_loop = (uv_loop_t *)uv;

        /** ���̼�ͨ�� */
        LiveIpc::Init();

        /** �������� */
        g_strRtpIP           = Settings::getValue("RtpClient","IP");                    //< RTP����IP
        g_nRtpBeginPort      = Settings::getValue("RtpClient","BeginPort",10000);       //< RTP��������ʼ�˿ڣ�������ż��
        g_nRtpPortNum        = Settings::getValue("RtpClient","PortNum",1000);          //< RTPʹ�õĸ�������strRTPPort��ʼÿ�μ�2����strRTPNum��
        g_nRtpCatchPacketNum = Settings::getValue("RtpClient", "CatchPacketNum", 100);  //< rtp����İ�������

        Log::debug("RtpConfig IP:%s, BeginPort:%d,PortNum:%d,CatchPacketNum:%d"
            , g_strRtpIP.c_str(), g_nRtpBeginPort, g_nRtpPortNum, g_nRtpCatchPacketNum);
        g_vecRtpPort.clear();
        for (int i=0; i<g_nRtpPortNum; ++i) {
            g_vecRtpPort.push_back(g_nRtpBeginPort+i*2);
        }
    }

    void GetClientsInfo() 
    {
        GetAllWorkerClientsInfo();
    }

    void GetDevList(){
        LiveIpc::GetDevList();
    }

    void QueryDirtionary(){
        LiveIpc::QueryDirtionary();
    }

	void DeviceControl(string strDev, int nInOut, int nUpDown, int nLeftRight) {
		LiveIpc::DeviceControl(strDev, nInOut, nUpDown, nLeftRight);
	}

    void SetCallBack(LIVECLIENT_CB cb){
        liveclient_respond = cb;
    }

    ILiveWorker* GetWorker(string strCode){
        CLiveWorker* worker = GetLiveWorker(strCode);
        if(nullptr == worker)
            worker = CreatLiveWorker(strCode);
        return (ILiveWorker*)worker;
    }
}