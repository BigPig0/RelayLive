#pragma once

#include "LiveClient.h"
#include "uv.h"
#include "ring_buff.h"

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
    bool RtpRecv(char* pBuff, long nLen, struct sockaddr_in* addr_in);

    /** 接收超时处理 */
    void RtpOverTime();

    /** rtp数据处理线程 */
    void RtpParse();

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
     */
    void push_es_stream(AV_BUFF buff);

    /**
     * 添加h264数据内容
     * @param[in] buff H264帧数据
     */
    void push_h264_stream(AV_BUFF buff);


    /** 结束时关闭loop */
    void AsyncClose();

public:
    bool        m_bRtpRun;          // rtp 接收是否在执行
    bool        m_bTimeOverRun;     // 超时判断是否在执行
    bool        m_bRun;             // loop是否正在执行
    uv_loop_t   *m_uvLoop;          // udp接收的loop

private:
    int         m_nLocalRTPPort;    // 本地RTP接收端口
    int         m_nLocalRTCPPort;   // 本地RTCP接收端口
    string      m_strRemoteIP;      // 远端发送IP
    int         m_nRemoteRTPPort;   // 远端RTP发送端口
    int         m_nRemoteRTCPPort;  // 远端RTCP发送端口

    uv_udp_t    m_uvRtpSocket;      // rtp接收
    uv_timer_t  m_uvTimeOver;       // 接收超时定时器
    uv_async_t  m_uvAsync;          // 异步操作句柄 外部线程用来结束m_uvLoop

    void*       m_pRtpParser;       // rtp报文解析类
    void*       m_pPsParser;        // PS帧解析类
    void*       m_pPesParser;       // PES包解析类
    void*       m_pEsParser;        // ES包解析类
    CLiveWorker* m_pWorker;         // 回调对象

    uint64_t    m_pts;              // 显示时间戳
    uint64_t    m_dts;              // 解码时间戳
    NalType     m_nalu_type;        // h264片元类型
    ring_buff_t* m_pRingRtp;        // rtp包缓存区，loop线程写入，解析线程读取
};

}

