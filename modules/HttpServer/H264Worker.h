#pragma once

#include "Singleton.h"
#include "WsLiveServer.h"
using namespace HttpWsServer;

struct CH264Nalu
{
    bool          isKey;          // 标记视频tag是否是sps_pps,关键帧，非关键帧
    char*         pBuff;
    int           nLen;
};

class CH264Worker
{
public:
    CH264Worker(string strPlatformCode, string strDevCode);
    ~CH264Worker(void);

    /**
     * 添加一个flv tag数据
     * @param buff->pBuff tag的内容
     * @param buff->nLen tag内容的大小
     */
    void AddTag(AutoMemoryPtr buff);

    /**
     * 添加一个http连接
     * @param pHttp 已经存在的连接的指针
     * @return 成功true，失败false
     */
    bool AddConnect(void* pHttp);

    /**
     * 移除一个http连接，全部移除时删除本实例
     * @param pHttp 已经存在的连接的指针
     * @return 成功true，失败false
     */
    bool DelConnect(void* pHttp);

    void cull_lagging_clients();

private:
    string                m_strPlatformCode;// 平台编码
    string                m_strDevCode;     // 设备编码
    vector<void*>         m_vecConnect;     // 播放请求连接
    CriticalSection       m_csConnect;      // http连接vector锁

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
//    CH264Worker*          m_pWorker;         //h264工作对象
//    string                strClientIP;       //客户端ip地址
//    list<AutoMemoryPtr>   m_listH264;        // 缓存FLV数据
//    CriticalSection       m_csH264;           // flv数据list锁
//};

class CH264Dock : public Singleton<CH264Dock>
{
    friend class Singleton<CH264Dock>;
    CH264Dock(void);
public:
    ~CH264Dock(void);

    /**
     * 根据平台编码和设备编码，创建视频缓存管理器
     * @param strPlatformCode 输入平台编码
     * @param strDevCode 输入设备编码
     * @return 视频缓存对象指针，nullptr表示创建失败
     */
    CH264Worker* CreateWorker(string strPlatformCode, string strDevCode);

    /**
     * 根据平台编码和设备编码，获取视频缓存管理器
     * @param strPlatformCode 输入平台编码
     * @param strDevCode 输入设备编码
     * @return 视频缓存对象指针，nullptr表示没有该设备的实例
     */
    CH264Worker* GetWorker(string strPlatformCode, string strDevCode);

    /**
     * 移除某个设备的视频缓存管理器
     * @param strPlatformCode 输入平台编码
     * @param strDevCode 输入设备编码
     * @return true成功移除 false没有设备移除，异常情况
     */
    bool DelWorker(string strPlatformCode, string strDevCode);
private:
    map<string,CH264Worker*>  m_workerMap;    // key是平台ID+设备ID，value是视频缓存容器
    CriticalSection          m_cs;        // map的锁
};