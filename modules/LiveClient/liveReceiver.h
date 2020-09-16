#pragma once

#include "LiveClient.h"
#include "ring_buff.h"
#include "rtp.h"

enum NalType;

namespace LiveClient
{
class CLiveWorker;
struct udp_recv_loop_t;
struct rtp_parse_loop_t;

/**
 * ��Ƶ��rtp/rtcp���մ���ģ��
 */
class CLiveReceiver
{
    friend class CLiveWorker;
public:
    CLiveReceiver(int nPort, CLiveWorker *worker, RTP_STREAM_TYPE rst);
    ~CLiveReceiver(void);

    /** ����UDP�˿ڼ��� */
    void StartListen();

    /** ���յ�rtp���ݴ��� */
    bool RtpRecv(char* pBuff, long nLen);

    /** ���ճ�ʱ���� */
    void RtpOverTime();

    /** rtp���ݴ����߳� */
    void RtpParse();

    /**
     * ���PS��������
     * @param[in] buff PS����
     */
    void push_ps_stream(AV_BUFF buff);

    /**
     * ���PES��������
     * @param[in] buff PES������
     */
    void push_pes_stream(AV_BUFF buff);

    /**
     * ���ES��������
     * @param[in] buff ES������
     */
    void push_es_stream(AV_BUFF buff);

    /**
     * ���h264��������
     * @param[in] buff H264֡����
     */
    void push_h264_stream(AV_BUFF buff);

public:
    int         m_nRunNum;
private:
    int         m_nLocalRTPPort;    // ����RTP���ն˿�
    int         m_nLocalRTCPPort;   // ����RTCP���ն˿�
    string      m_strRemoteIP;      // Զ�˷���IP
    int         m_nRemoteRTPPort;   // Զ��RTP���Ͷ˿�
    int         m_nRemoteRTCPPort;  // Զ��RTCP���Ͷ˿�
    RTP_STREAM_TYPE m_stream_type;      // ��Ƶ������

    udp_recv_loop_t  *m_pUdpRecv;   // udp���չ���
    rtp_parse_loop_t *m_pRtpParse;  // rtp��������

    void*       m_pRtpParser;       // rtp���Ľ�����
    void*       m_pPsParser;        // PS֡������
    void*       m_pPesParser;       // PES��������
    void*       m_pEsParser;        // ES��������
    CLiveWorker* m_pWorker;         // �ص�����

    uint64_t    m_pts;              // ��ʾʱ���
    uint64_t    m_dts;              // ����ʱ���
    NalType     m_nalu_type;        // h264ƬԪ����
    ring_buff_t* m_pRingRtp;        // rtp����������loop�߳�д�룬�����̶߳�ȡ
};

}

