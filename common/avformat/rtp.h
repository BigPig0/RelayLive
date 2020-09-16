/**
 * ����rtp��
 * ���PS��
 */
#pragma once
#include "avtypes.h"

//error code
#define ERR_RTP_SUCCESS            0
#define ERR_RTP_LENGTH             -1
#define ERR_RTP_VERSION            -2
#define ERR_RTP_PT                 -3
#define ERR_RTP_SEQ                -4
#define ERR_RTP_MEMORY             -5
#define ERR_RTP_WAIT               -6
#define ERR_RTP_FORMAT             -7
#define ERR_RTP_PARAM              -8


#define RTP_VERSION          2                   // rtp�汾��
#define PACK_MAX_SIZE        1700                // rtp����󳤶�
#define PACK_MIN_SIZE        50                  // rtp����С����
#define RTP_HEADER_SIZE      12                  // rtpͷĬ�ϳ���
#define SPS_MAX_SIZE         64
#define FRAME_MAX_SIZE       (1024 * 1024 * 3)   // PS֡����С

enum RTP_STREAM_TYPE {
    RTP_STREAM_UNKNOW = 0,
    RTP_STREAM_PS,
    RTP_STREAM_H264
};

//frame type
enum{
    FRAME_TYPE_OTHER = 0,
    FRAME_TYPE_KEY   = 5,
    FRAME_TYPE_SPS   = 7,
    FRAME_TYPE_PPS   = 8,

    FRAME_TYPE_AUDIO = 100,
};

#pragma pack(1)
typedef struct tagRtpHeader
{
#ifdef _BIG_ENDIAN
    uint8_t v:2;	 /* packet type */
    uint8_t p:1;	 /* padding flag */
    uint8_t x:1;	 /* header extension flag */
    uint8_t cc:4;	 /* CSRC count */
    uint8_t m:1;	 /* marker bit */
    uint8_t pt:7;	 /* payload type */
#else
    uint8_t cc:4;	 /* CSRC count */
    uint8_t x:1;	 /* header extension flag */
    uint8_t p:1;	 /* padding flag */
    uint8_t v:2;	 /* packet type */
    uint8_t pt:7;	 /* payload type */
    uint8_t m:1;	 /* marker bit */
#endif
    uint16_t seq;	 /* sequence number */
    uint32_t ts;	 /* timestamp */
    uint32_t ssrc;	 /* synchronization source */
} RtpHeader;
#pragma pack()

// rtp/udp���Ļ����б�ڵ㣬���б���������
struct rtp_list_node
{
    rtp_list_node*  pre;     //< ǰһ�ڵ�
    rtp_list_node*  next;    //< ��һ�ڵ�
    char*           data;    //< ����
    int             len;     //< ���ݳ���
    int             head_len;     //< rtp����ͷ����
    int             playload_len; //< �غɳ���
    rtp_list_node()
        : pre(nullptr)
        , next(nullptr)
        , data(nullptr)
        , len(0)
        , head_len(RTP_HEADER_SIZE)
        , playload_len(0)
    {}
};

struct Sequence
{
    uint16_t seq;
    Sequence():seq(0){}
    Sequence(uint16_t& s):seq(s){}

    bool operator==(const Sequence& rs) const
    {
        return this->seq == rs.seq;
    }
    bool operator<(const Sequence& rs) const
    {
        if (seq == rs.seq)
        {
            return false;
        }
        else if (seq < rs.seq)
        {
            if(rs.seq - seq > 32767)
                return false;
            else
                return true;
        }
        else if (seq > rs.seq)
        {
            if(seq - rs.seq > 32767)
                return true;
            else
                return false;
        }
        return false;
    }
    bool operator>(const Sequence& rs) const
    {
        return rs < *this;
    }
    bool operator<=(const Sequence& rs) const
    {
        return *this<rs || *this==rs;
    }
    bool operator>=(const Sequence& rs) const
    {
        return *this>rs || *this==rs;
    }
};

typedef map<Sequence,rtp_list_node*> MapRtpList;
typedef list<rtp_list_node*> ListRtpFrame;

class CRtp
{
public:
    CRtp(AV_CALLBACK cb, void* handle=NULL);
    ~CRtp(void);

    /**
     * ����һ��rtp��
     */
    int DeCode(char* pBuf, uint32_t nLen);

    /**
     * ���û���֡����
     * @param[in] nFrameNum ֡��������,��ֵԽ���ӳ�Խ�󣬵���Ӧ�Ը��������״��
     */
    void SetCatchFrameNum(int nFrameNum)
    {
        m_nCatchPacketNum = nFrameNum;
    }

    /**
     * ������Ƶ������
     */
    void SetRtpStreamType(RTP_STREAM_TYPE rst){
        m_stream_type = rst;
    }
private:
    /**
     * ��rtp�����б��в����½��յ�����
     * @param[in] packetBuf RTP������
     * @param[in] packetSize RTP�����ݳ���
     * @return 0û������rtp֡���Ժϲ���1������rtp֡���Ժϲ�, -1���������쳣�˳�
     */
    int InserSortList(char* packetBuf, long packetSize);

    /**
     * ����RTP��ͷ���غɴ�С
     * @param[in] pBuf rtp��
     * @param[in] size rtp���Ĵ�С
     * @param[out] nHeaderSize ��������rtpͷ��С
     * @param[out] nPlayLoadSize ���������غɵĴ�С
     * @return 0�ɹ�  ����ʧ��
     */
    int ParseRtpHeader(char* pBuf, long size, int& nHeaderSize, int& nPlayLoadSize);

    /**
     * �ͷ�rtp�ڵ���ռ���ڴ�
     * @param[in] pNode rtp���ڵ�
     * @return 0�ɹ�  ����ʧ��
     */
    int DelRtpNode(rtp_list_node* pNode);

    /**
     * �ϳ�PS֡����
     * @return 0�ɹ�  ����ʧ��
     */
    int ComposePsFrame();

    /**
     * ��С��ת��
     */
    char*  EndianChange(char* src, int bytes);
private:
    char*             m_frame_buf;            // PS֡����
    CriticalSection   m_cs;                   // ȷ����ǰ����Ĵ������Ƿǲ��е�
    RTP_STREAM_TYPE   m_stream_type;          // ��Ƶ������

    MapRtpList        m_mapRtpList;            // ���յ���rtp/udp���������б�
    ListRtpFrame      m_listRtpFrame;          // �ܹ����һ֡����������
    int               m_nCatchPacketNum;       // rtp����������
    uint16_t          m_nDoneSeq;              // �ѽ�����rtp����seq number
    bool              m_bBegin;                // Ĭ��false��ȡ����һ���ڵ�ĳ�true


    void*             m_hUser;                  // �ص��������
    AV_CALLBACK      m_fCB;
};

