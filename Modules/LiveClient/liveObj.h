#pragma once

#include "LiveClient.h"
#include "uv.h"

enum NalType;
enum STREAM_TYPE;

/**
 * RTSP功能模块接口
 * 使用前主程序必须先初始化<指UdpSocket必须初始化才能使用>
 */
class CLiveObj : public IlibLive
{
public:
    CLiveObj(liblive_option opt);
    ~CLiveObj(void);

    /** 启动UDP端口监听 */
    void StartListen();

    /** 接收的rtp数据处理 */
    void RtpRecv(char* pBuff, long nLen);

    /** 接收超时处理 */
    void RtpOverTime();

    /**
     * RTP组包回调
     * @param[in] pBuff PS帧数据
     * @param[in] nLen PS帧长度
     */
    void RTPParseCb(char* pBuff, long nLen);

    /**
     * PS帧解析回调
     * @param[in] pBuff PES包数据
     * @param[in] nLen PES包长度
     */
    void PSParseCb(char* pBuff, long nLen);

    /**
     * PES帧解析回调
     * @param[in] pBuff ES包数据
     * @param[in] nLen ES包长度
     * @param[in] pts 展现时间戳字段
     * @param[in] dts 解码时间戳字段
     */
    void PESParseCb(char* pBuff, long nLen, uint64_t pts, uint64_t dts);

    /**
     * ES帧解析回调
     * @param[in] pBuff H264帧数据
     * @param[in] nLen H264帧长度
     * @param[in] nNalType Nalu的类型
     */
    void ESParseCb(char* pBuff, long nLen/*, uint8_t nNalType*/);

    /** H264中sps解析回调 */
    void H264SpsCb(uint32_t nWidth, uint32_t nHeight, double fFps);

    /** FLV合成回调 */
    void FlvCb(FLV_FRAG_TYPE eType, char* pBuff, int nBuffSize);

    /** MP4合成回调 */
    void Mp4Cb(MP4_FRAG_TYPE eType, char* pBuff, int nBuffSize);

    /** TS合成回调 */
    void TsCb(char* pBuff, int nBuffSize);

    /** H264合成回调 */
    void H264Cb(char* pBuff, int nBuffSize);

    /**
     * 设置处理数据回调的对象
     * @param[in] pHandle
     */
    void SetCallback(IlibLiveCb* pHandle)
    {
        m_pCallBack = pHandle;
    }

    /** 结束时关闭loop */
    void AsyncClose();

    bool        m_bRun;
    uv_loop_t   *m_uvLoop;
private:
    int         m_nLocalRTPPort;    // 本地RTP端口
    int         m_nLocalRTCPPort;   // 本地RTCP端口
    string      m_strRemoteIP;      // 远端IP
    int         m_nRemoteRTPPort;   // 远端RTP端口
    int         m_nRemoteRTCPPort;  // 远端RTCP端口

    uv_udp_t    m_uvRtpSocket;      // rtp接收
    uv_timer_t  m_uvTimeOver;       // 接收超时定时器
    uv_async_t  m_uvAsync;          // 异步操作句柄

    void*       m_pRtpParser;       // rtp报文解析类
    void*       m_pPsParser;        // PS帧解析类
    void*       m_pPesParser;       // PES包解析类
    void*       m_pEsParser;        // ES包解析类
    void*       m_pH264;            // H264解析类
    void*       m_pTs;              // TS组包类
    void*       m_pFlv;             // FLV组包类
    void*       m_pMp4;             // MP4组包类
    IlibLiveCb* m_pCallBack;        // 回调对象

    uint64_t    m_pts;              // 记录PES中的pts
    uint64_t    m_dts;              // 记录PES中的dts
    NalType     m_nalu_type;        // h264片元类型
};

