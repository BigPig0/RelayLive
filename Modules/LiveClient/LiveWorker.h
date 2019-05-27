#pragma once
#include "LiveClient.h"
#include "avtypes.h"
#include "uv.h"

namespace LiveClient
{
    class CLiveReceiver;

    class CLiveWorker : public ILiveWorker
    {
    public:
        CLiveWorker(string strCode, int rtpPort, string sdp);
        ~CLiveWorker();

        /** 客户端连接 */
        virtual bool AddHandle(ILiveHandle* h, HandleType t);
        virtual bool RemoveHandle(ILiveHandle* h);
		virtual AV_BUFF GetHeader(HandleType t);
        virtual string GetSDP();

        /** 客户端全部断开，延时后销毁实例 */
        void Clear2Stop();
        bool m_bStop;          //< 进入定时器回调后设为true，close定时器回调中销毁对象
        bool m_bOver;          //< 超时后设为true，客户端全部断开后不延时，立即销毁

        /** 获取客户端信息 */
        string GetClientInfo();

        /**
        * 从源过来的视频数据，单线程输入 
        * 以下方法由rtp接收所在的loop线程调用
        * 类中其他方法包括构造、析构都由http所在的loop线程调用
        */
        void push_flv_stream (AV_BUFF buff);
        void push_h264_stream(AV_BUFF buff);
        void push_ts_stream  (AV_BUFF buff);
        void push_fmp4_stream(AV_BUFF buff);
        void push_rtp_stream (AV_BUFF buff);
        void push_rtcp_stream(AV_BUFF buff);
        void stop();

        bool m_bFlv;
        bool m_bFlvSub;
        bool m_bMp4;
        bool m_bH264;
        bool m_bTs;
        bool m_bRtp;
		AV_BUFF               m_stFlvHead;    //flv头，内容存储在CFlv里面
		AV_BUFF               m_stMp4Head;    //mp4头，内容存储在CMP4里面

    private:
        string                   m_strCode;     // 播放媒体编号
        string                   m_strSDP;      // sip服务器返回的sdp
        CLiveReceiver*           m_pLiveReceiver;  // 直播数据接收和解包装包

        vector<ILiveHandle*>     m_vecLiveFlv;  // 播放实例 
        vector<ILiveHandle*>     m_vecLiveFlvSub;  // 播放实例 
        CriticalSection          m_csFlv;
        vector<ILiveHandle*>     m_vecLiveMp4;  // 播放实例 
        CriticalSection          m_csMp4;
        vector<ILiveHandle*>     m_vecLiveH264; // 播放实例 
        CriticalSection          m_csH264;
        vector<ILiveHandle*>     m_vecLiveTs;   // 播放实例 
        CriticalSection          m_csTs;
        vector<ILiveHandleRtp*>  m_vecLiveRtp;  // 播放实例 
        CriticalSection          m_csRtp;

        int                      m_nType;          //< 0:live直播；1:record历史视频
        int                      m_nPort;          //< rtp接收端口

        uv_timer_t               m_uvTimerStop;    //< http播放端全部连开连接后延迟销毁，以便页面刷新时快速播放
    };

    extern CLiveWorker* CreatLiveWorker(string strCode);
    extern CLiveWorker* GetLiveWorker(string strCode);
    extern bool DelLiveWorker(string strCode);
	extern string GetAllWorkerClientsInfo();
}