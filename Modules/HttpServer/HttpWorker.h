#pragma once
#include "LiveClient.h"

namespace HttpWsServer
{
    struct pss_http_ws_live;
    enum MediaType;

    struct LIVE_BUFF {
        char *pBuff;
        int   nLen;
    };

    class CHttpWorker : public IlibLiveCb
    {
    public:
        CHttpWorker(string strCode);
        ~CHttpWorker();

        /** 客户端连接 */
        bool AddConnect(pss_http_ws_live* pss);
        bool DelConnect(pss_http_ws_live* pss);

		/** 客户端全部断开，延时后销毁实例 */
		void Clear2Stop();
        bool m_bStop;          //< 进入定时器回调后设为true，close定时器回调中销毁对象
        bool m_bOver;          //< 超时后设为true，客户端全部断开后不延时，立即销毁

        /** 请求端获取视频数据 */
        LIVE_BUFF GetFlvHeader();
        LIVE_BUFF GetFlvVideo(uint32_t *tail);
        //------------------------------------------
        LIVE_BUFF GetH264Video(uint32_t *tail);
        //------------------------------------------
        LIVE_BUFF GetMp4Header();
        LIVE_BUFF GetMp4Video(uint32_t *tail);
        //------------------------------------------
        void NextWork(pss_http_ws_live* pss);

        /** 获取客户端信息 */
        string GetClientInfo();

        /**
         * 从源过来的视频数据，单线程输入 
         * 以下继承自IlibLiveCb的方法由rtp接收所在的loop线程调用
         * 类中其他方法包括构造、析构都由http所在的loop线程调用
         */
        void push_flv_frame(FLV_FRAG_TYPE eType, char* pBuff, int nLen);
        void push_h264_stream(char* pBuff, int nLen);
        void push_ts_stream(char* pBuff, int nLen);
        void push_mp4_stream(MP4_FRAG_TYPE eType, char* pBuff, int nBuffSize);
        void stop();
    private:
        void cull_lagging_clients(MediaType type);


    private:
        string                m_strCode;     // 播放媒体编号

        /**
         * lws_ring无锁环形缓冲区，只能一个线程写入，一个线程读取
         * m_pFlvRing、m_pH264Ring、m_pMP4Ring由rtp读取的loop线程写入，http服务所在的loop线程读取
         */
        //flv
        LIVE_BUFF             m_stFlvHead;  //flv头数据保存在libLive模块，外部不需要释放
        struct lws_ring       *m_pFlvRing;
        pss_http_ws_live      *m_pFlvPssList;

        //h264
        struct lws_ring       *m_pH264Ring;
        pss_http_ws_live      *m_pH264PssList;

        //fMP4
        LIVE_BUFF             m_stMP4Head;  //mp4头数据保存在libLive模块，外部不需要释放
        struct lws_ring       *m_pMP4Ring;
        pss_http_ws_live      *m_pMP4PssList;

        int                   m_nType;          //< 0:live直播；1:record历史视频
        IlibLive*             m_pLive;          //< 直播数据接收和解包装包

        uv_timer_t            m_uvTimerStop;    //< http播放端全部连开连接后延迟销毁，以便页面刷新时快速播放
    };

    /** ipc 初始化 */
    void ipc_init();

    /** 直播 */
    CHttpWorker* CreatHttpWorker(string strCode);
    CHttpWorker* GetHttpWorker(string strCode);
    bool DelHttpWorker(string strCode);

    /** 点播 */

    /** 获取播放信息，返回json */
    string GetClientsInfo();
	/** 获取设备列表 */
	int GetDevList(int pss);
	/** 发送设备查询命令 */
	int QueryDirtionary();
};