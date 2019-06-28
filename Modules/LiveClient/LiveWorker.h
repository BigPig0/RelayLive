#pragma once
#include "LiveClient.h"
#include "avtypes.h"
#include "Recode.h"
#include "uv.h"

namespace LiveClient
{
    class CLiveReceiver;
    class CLiveChannel;

    class CLiveWorker : public ILiveWorker
    {
    public:
        CLiveWorker(string strCode, int rtpPort, string sdp);
        ~CLiveWorker();

        /** 客户端连接 */
        virtual bool AddHandle(ILiveHandle* h, HandleType t, int c);
        virtual bool RemoveHandle(ILiveHandle* h);
		virtual AV_BUFF GetHeader(HandleType t, int c);
        virtual string GetSDP();

        /** 客户端全部断开，延时后销毁实例 */
        void Clear2Stop();
        bool m_bStop;          //< 进入定时器回调后设为true，close定时器回调中销毁对象
        bool m_bOver;          //< 超时后设为true，客户端全部断开后不延时，立即销毁

        /** 获取客户端信息 */
        vector<ClientInfo> GetClientInfo();

        /** 接收到的视频流处理 */
        void ReceiveStream(AV_BUFF buff);

        /** yuv视频处理 */
        void ReceiveYUV(AV_BUFF buff);

        /** 接收数据超时发起的结束操作，通知发送连接断开 */
        void stop();

        bool m_bRtp;

    private:
        string                   m_strCode;     // 播放媒体编号
        string                   m_strSDP;      // sip服务器返回的sdp
        CLiveReceiver           *m_pReceiver;   // 直播数据接收和解包

        CLiveChannel            *m_pOrigin;     // 原始流通道

#ifdef USE_FFMPEG
        map<int, CLiveChannel*>  m_mapChlEx;    // 扩展通道
        CriticalSection          m_csChls;      // map的锁
        IDecoder                *m_pDecoder;    // h264解码
#endif

        vector<ILiveHandleRtp*>  m_vecLiveRtp;  // RTP原始流转发
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