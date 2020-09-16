#pragma once

#include "Singleton.h"
#include "WsLiveServer.h"
using namespace HttpWsServer;

struct CH264Nalu
{
    bool          isKey;          // �����Ƶtag�Ƿ���sps_pps,�ؼ�֡���ǹؼ�֡
    char*         pBuff;
    int           nLen;
};

class CH264Worker
{
public:
    CH264Worker(string strPlatformCode, string strDevCode);
    ~CH264Worker(void);

    /**
     * ���һ��flv tag����
     * @param buff->pBuff tag������
     * @param buff->nLen tag���ݵĴ�С
     */
    void AddTag(AutoMemoryPtr buff);

    /**
     * ���һ��http����
     * @param pHttp �Ѿ����ڵ����ӵ�ָ��
     * @return �ɹ�true��ʧ��false
     */
    bool AddConnect(void* pHttp);

    /**
     * �Ƴ�һ��http���ӣ�ȫ���Ƴ�ʱɾ����ʵ��
     * @param pHttp �Ѿ����ڵ����ӵ�ָ��
     * @return �ɹ�true��ʧ��false
     */
    bool DelConnect(void* pHttp);

    void cull_lagging_clients();

private:
    string                m_strPlatformCode;// ƽ̨����
    string                m_strDevCode;     // �豸����
    vector<void*>         m_vecConnect;     // ������������
    CriticalSection       m_csConnect;      // http����vector��

    struct lws_ring       *m_sRing;
    pss_http_ws_live           *m_sPssList;
};

//class CH264Connect {
//public:
//    ~CH264Connect(){
//        m_pWorker->DelConnect((void*)this);
//    }
//    void AddTag(AutoMemoryPtr newTag);
//
//    CH264Worker*          m_pWorker;         //h264��������
//    string                strClientIP;       //�ͻ���ip��ַ
//    list<AutoMemoryPtr>   m_listH264;        // ����FLV����
//    CriticalSection       m_csH264;           // flv����list��
//};

class CH264Dock : public Singleton<CH264Dock>
{
    friend class Singleton<CH264Dock>;
    CH264Dock(void);
public:
    ~CH264Dock(void);

    /**
     * ����ƽ̨������豸���룬������Ƶ���������
     * @param strPlatformCode ����ƽ̨����
     * @param strDevCode �����豸����
     * @return ��Ƶ�������ָ�룬nullptr��ʾ����ʧ��
     */
    CH264Worker* CreateWorker(string strPlatformCode, string strDevCode);

    /**
     * ����ƽ̨������豸���룬��ȡ��Ƶ���������
     * @param strPlatformCode ����ƽ̨����
     * @param strDevCode �����豸����
     * @return ��Ƶ�������ָ�룬nullptr��ʾû�и��豸��ʵ��
     */
    CH264Worker* GetWorker(string strPlatformCode, string strDevCode);

    /**
     * �Ƴ�ĳ���豸����Ƶ���������
     * @param strPlatformCode ����ƽ̨����
     * @param strDevCode �����豸����
     * @return true�ɹ��Ƴ� falseû���豸�Ƴ����쳣���
     */
    bool DelWorker(string strPlatformCode, string strDevCode);
private:
    map<string,CH264Worker*>  m_workerMap;    // key��ƽ̨ID+�豸ID��value����Ƶ��������
    CriticalSection          m_cs;        // map����
};