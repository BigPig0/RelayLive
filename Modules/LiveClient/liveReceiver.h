#pragma once

#include "LiveClient.h"
#include "uv.h"

enum NalType;

namespace LiveClient
{
enum STREAM_TYPE;
class CLiveWorker;

/**
 * 视频流rtp/rtcp接收处理模块
 */
class CLiveReceiver
{
    friend class CLiveWorker;
public:
    CLiveReceiver(int nPort, CLiveWorker *worker);
    ~CLiveReceiver(void);

    /** 启动UDP端口监听 */
    void StartListen();

    /** 接收的rtp数据处理 */
    void RtpRecv(char* pBuff, long nLen, struct sockaddr_in* addr_in);

    /** 接收超时处理 */
    void RtpOverTime();

    /**
     * 添加PS数据内容
     * @param[in] buff PS数据
     */
    void push_ps_stream(AV_BUFF buff);

    /**
     * 添加PES数据内容
     * @param[in] buff PES包数据
     */
    void push_pes_stream(AV_BUFF buff);

    /**
     * 添加ES数据内容
     * @param[in] buff ES包数据
     * @param[in] pts 展现时间戳字段
     * @param[in] dts 解码时间戳字段
     */
    void push_es_stream(AV_BUFF buff, uint64_t  pts, uint64_t  dts);

    /**
     * 添加h264数据内容
     * @param[in] buff H264帧数据
     */
    void push_h264_stream(AV_BUFF buff);

    /** H264中sps解析回调 */
    void set_h264_param(uint32_t nWidth, uint32_t nHeight, double fFps);

    /** FLV合成回调 */
    void FlvCb(AV_BUFF buff);

    /** MP4合成回调 */
    void Mp4Cb(AV_BUFF buff);

    /** TS合成回调 */
    void TsCb(AV_BUFF buff);

    /** H264合成回调 */
    void H264Cb(AV_BUFF buff);

    /** 结束时关闭loop */
    void AsyncClose();

private:
    int         m_nLocalRTPPort;    // 本地RTP接收端口
    int         m_nLocalRTCPPort;   // 本地RTCP接收端口
    string      m_strRemoteIP;      // 远端发送IP
    int         m_nRemoteRTPPort;   // 远端RTP发送端口
    int         m_nRemoteRTCPPort;  // 远端RTCP发送端口

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
    CLiveWorker* m_pWorker;        // 回调对象

    uint64_t    m_pts;              // 记录PES中的pts
    uint64_t    m_dts;              // 记录PES中的dts
    NalType     m_nalu_type;        // h264片元类型
};

}

