#pragma once
#include "LiveClient.h"
#include "avtypes.h"
#include "Recode.h"
#include "uv.h"
#include "rtp.h"

namespace LiveClient
{
    class CLiveReceiver;
    class CLiveChannel;
    struct live_event_loop_t;

    struct PlayAnswerList {
        struct PlayAnswerList *pNext;
        string strDevCode;  //设备编码
        int    nRet;        //播放请求返回值 0成功 非0失败
        string strMark;     //失败时保存错误原因，成功时保存sdp信息
    };

    class CLiveWorker : public ILiveWorker
    {
    public:
        CLiveWorker(string strCode, int rtpPort);
        ~CLiveWorker();

        /** 客户端连接 */
        virtual bool AddHandle(ILiveHandle* h, HandleType t, int c);
        bool AddHandleAsync(ILiveHandle* h, HandleType t, int c);

        /** 客户端断开 */
        virtual bool RemoveHandle(ILiveHandle* h);
        bool RemoveHandleAsync(ILiveHandle* h);
        //virtual string GetSDP();

        /** 获取客户端信息 */
        void GetClientInfo();
        void GetClientInfoAsync();

        /** 接收到的视频流处理 */
        void ReceiveStream(AV_BUFF buff);
        void ReceiveStreamAsync(AV_BUFF buff);


        /** 客户端全部断开，延时后销毁实例 */
        void Clear2Stop();
        bool m_bStop;          //< 进入定时器回调后设为true，close定时器回调中销毁对象
        bool m_bOver;          //< 超时后设为true，客户端全部断开后不延时，立即销毁

#ifdef EXTEND_CHANNELS
        /** yuv视频处理 */
        void ReceiveYUV(AV_BUFF buff);
#endif

        /** 接收数据超时发起的结束操作，通知发送连接断开 */
        void stop();

        /** 播放结果回调 */
        void PlayAnswer(PlayAnswerList *pal);

        /** 解析SDP */
        void ParseSdp();

    public:
        int         m_nRunNum;  //启动的event loop数量
        bool        m_bRtp;

    private:
        string                   m_strServerIP; // rtp发送端IP
        uint32_t                 m_nServerPort; // rtp发送端口
        uint32_t                 m_nPort;       // rtp接收端口
        string                   m_strCode;     // 播放媒体编号
        bool                     m_bPlayed;     // true:播放请求收到应答 false:没有收到应答
        int                      m_nPlayRes;    // 播放返回的结果 0成功，非0失败
        string                   m_strPlayInfo; // sip服务器返回的sdp或失败原因描述
        RTP_STREAM_TYPE          m_stream_type; // 视频流类型
        
        CLiveReceiver           *m_pReceiver;   // 直播数据接收和解包
        CLiveChannel            *m_pOrigin;     // 原始流通道

#ifdef EXTEND_CHANNELS
        map<int, CLiveChannel*>  m_mapChlEx;    // 扩展通道
        IDecoder                *m_pDecoder;    // h264解码
#endif

        vector<ILiveHandleRtp*>  m_vecLiveRtp;  // RTP原始流转发

        int                      m_nType;          //< 0:live直播；1:record历史视频

        live_event_loop_t*       m_pEventLoop;
    };

    /** 建立一个设备的播放实例 */
    extern CLiveWorker* CreatLiveWorker(string strCode);
    /** 获取指定设备播放实例 */
    extern CLiveWorker* GetLiveWorker(string strCode);
    /** 结束指定设备播放实例 */
    extern bool DelLiveWorker(string strCode);
    /** 获取当前所有播放实例的客户端连接情况 */
	extern void GetAllWorkerClientsInfo();
    /** 响应sip服务器返回的播放应答 */
    extern bool AsyncPlayCB(PlayAnswerList *pal);
}