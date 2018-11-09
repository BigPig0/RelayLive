/**
 * 该文件是rtsp模块唯一对外导出的头文件
 */
#pragma once

#ifdef RTSP_EXPORTS
#define RTSP_API __declspec(dllexport)
#else
#define RTSP_API
#endif

enum NalType;
enum flv_tag_type;
enum MP4_FRAG_TYPE;

/**
 * 数据回调处理接口
 * 由上层实现一个继承该接口的类来处理RTP解析后的数据
 */
struct IlibLiveCb
{
    /**
     * FLV数据处理接口
     */
    virtual void push_flv_frame(int tag_type, char* frames, int frames_size) = 0;

    /**
     * TS数据处理接口(HLS)
     */
    virtual void push_ts_stream(char* pBuff, int nBuffSize) = 0;

    /**
     * h264裸流处理接口
     */
    virtual void push_h264_stream(NalType eType, char* pBuff, int nBuffSize) = 0;

    /**
     * mp4数据处理接口
     */
    virtual void push_mp4_stream(MP4_FRAG_TYPE eType, char* pBuff, int nBuffSize) = 0;

    /**
     * rtsp接收端结束
     */
    virtual void stop() = 0;

    bool        m_bFlv;
    bool        m_bTs;
    bool        m_bH264;
    bool        m_bMp4;
    IlibLiveCb():m_bFlv(false),m_bTs(false),m_bH264(false),m_bMp4(false){}
};

struct RTSP_API IlibLive
{
    virtual ~IlibLive(){}

    /**
     * 创建一个拥有本接口功能的实例
     * return 实例的指针，注意要由申请者释放
     */
    static IlibLive* CreateObj();

    /**
     * 创建flv头
     */
    static bool MakeFlvHeader(char** ppBuff, int* pLen);

    /**
     * 设置本地监听的IP和端口
     * @param[in] strIP 本地IP
     * @param[in] nPort 监听的UDP端口
     */
    virtual void SetLocalAddr(string strIP, int nPort) = 0;

    /**
     * 设置缓存帧数量
     * @param[in] nPacketNum 帧缓存数量,数值越大延迟越大，但能应对更差的网络状况
     */
    virtual void SetCatchPacketNum(int nPacketNum) = 0;

    /** 
     * 启动UDP端口监听 
     */
    virtual void StartListen() = 0;

    /**
     * 设置回调处理对象
     * @param[in] pHandle 回调处理对象指针
     */
    virtual void SetCallback(IlibLiveCb* pHandle) = 0;
};
