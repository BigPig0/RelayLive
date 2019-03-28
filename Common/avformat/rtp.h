/**
 * 输入rtp包
 * 输出PS包
 */
#pragma once
#include "liveObj.h"

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


#define RTP_VERSION          2                   // rtp版本号
#define PACK_MAX_SIZE        1700                // rtp包最大长度
#define PACK_MIN_SIZE        50                  // rtp包最小长度
#define RTP_HEADER_SIZE      12                  // rtp头默认长度
#define SPS_MAX_SIZE         64
#define FRAME_MAX_SIZE       (1024 * 1024 * 3)   // PS帧最大大小

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

// rtp/udp报文缓存列表节点，该列表用来排序
struct rtp_list_node
{
    rtp_list_node*  pre;     //< 前一节点
    rtp_list_node*  next;    //< 后一节点
    char*           data;    //< 数据
    int             len;     //< 数据长度
    int             head_len;     //< rtp报文头长度
    int             playload_len; //< 载荷长度
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

typedef void(*RTP_CALLBACK)(char*, long, void*);

class CRtp
{
public:
    CRtp(void* handle, RTP_CALLBACK cb);
    ~CRtp(void);

    /**
     * 插入一个rtp包
     */
    int InputBuffer(char* pBuf, uint32_t nLen);

    /**
     * 设置缓存帧数量
     * @param[in] nFrameNum 帧缓存数量,数值越大延迟越大，但能应对更差的网络状况
     */
    void SetCatchFrameNum(int nFrameNum)
    {
        m_nCatchPacketNum = nFrameNum;
    }
private:
    /**
     * 向rtp缓存列表中插入新接收的数据
     * @param[in] packetBuf RTP包数据
     * @param[in] packetSize RTP包数据长度
     * @return 0没有完整rtp帧可以合并，1有完整rtp帧可以合并, -1遇到错误，异常退出
     */
    int InserSortList(char* packetBuf, long packetSize);

    /**
     * 解析RTP包头和载荷大小
     * @param[in] pBuf rtp包
     * @param[in] size rtp报文大小
     * @param[out] nHeaderSize 解析出的rtp头大小
     * @param[out] nPlayLoadSize 解析出的载荷的大小
     * @return 0成功  其他失败
     */
    int ParseRtpHeader(char* pBuf, long size, int& nHeaderSize, int& nPlayLoadSize);

    /**
     * 释放rtp节点所占的内存
     * @param[in] pNode rtp包节点
     * @return 0成功  其他失败
     */
    int DelRtpNode(rtp_list_node* pNode);

    /**
     * 合成PS帧数据
     * @return 0成功  其他失败
     */
    int ComposePsFrame();

    /**
     * 大小码转换
     */
    char*  EndianChange(char* src, int bytes);
private:
    char*             m_frame_buf;            // PS帧缓存
    CriticalSection   m_cs;                   // 确保当前对象的处理函数是非并行的

    MapRtpList        m_mapRtpList;            // 接收到的rtp/udp报文排序列表
    ListRtpFrame      m_listRtpFrame;          // 能够组成一帧的连续报文
    int               m_nCatchPacketNum;       // rtp包缓存数量
    uint16_t          m_nDoneSeq;              // 已解析的rtp报文seq number
    bool              m_bBegin;                // 默认false，取出第一个节点改成true


    void*             m_hUser;                  // 回调处理对象
    RTP_CALLBACK      m_fCB;
};

