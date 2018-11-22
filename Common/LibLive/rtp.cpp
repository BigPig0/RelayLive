#include "stdafx.h"
#include "rtp.h"
#include "ps.h"


CRtp::CRtp(CLiveObj* pObj)
    : m_frame_buf(nullptr)
    , m_nCatchPacketNum(5000)
    , m_nDoneSeq(0)
    , m_bBegin(false)
    , m_pObj(pObj)
{
    m_frame_buf = new char[FRAME_MAX_SIZE];
    memset(m_frame_buf, '\0', FRAME_MAX_SIZE);
}

CRtp::~CRtp(void)
{
    SAFE_DELETE(m_frame_buf);
}

int CRtp::InputBuffer(char* pBuf, uint32_t nLen)
{
    if (NULL == pBuf || RTP_HEADER_SIZE > nLen)
    {
        return -1;
    }
    RtpHeader* head = (RtpHeader *)pBuf;
    EndianChange((char*)(&head->seq),2);
    EndianChange((char*)(&head->ssrc),4);
    EndianChange((char*)(&head->ts),4);

    //Log::debug("CRtp::InputBuffer recieve size:%d, RTP header: version=%d,padding=%d,extension=%d,"
    //    "CSRC count=%d,marker=%d,payloadtype=%d,sequence number=%d,timestamp=%ld,SSRC=%ld", 
    //    nLen,head->v,head->p,head->x,head->cc,head->m,head->pt,head->seq,head->ts,head->ssrc);

    if (head->pt != 96)
    {
        return -1;
    }

    MutexLock lock(&m_cs);
    return InserSortList(pBuf, nLen);
}

int CRtp::InserSortList(char* packetBuf, long packetSize)
{
    RtpHeader* newRtp = (RtpHeader *)packetBuf;
    //Log::debug("rtp recive seq:%lu",newRtp->seq);
    Sequence newSeq(newRtp->seq);
    // 过来的数据位于已播放的之前，需要抛弃
    if (m_bBegin)
    {
        Sequence doneSeq(m_nDoneSeq);
        if (newSeq < doneSeq)
        {
            Log::error("rtp sort error: seq is old seq:%d,m_nDoneSeq:%d",newRtp->seq,m_nDoneSeq);
            return -1;
        }
    }

    //序列号存在，重复数据抛弃
    auto itfind = m_mapRtpList.find(newSeq);
    if (itfind != m_mapRtpList.end())
    {
        Log::error("rtp sort error:same seq %d", newRtp->seq);
        return -1;
    }

    // 解析该rtp报文，rtp头和playload的长度
    int nHeaderSize = 0; 
    int nPlayLoadSize = 0;
    int ret = ParseRtpHeader(packetBuf, packetSize,nHeaderSize, nPlayLoadSize);
    if (ERR_RTP_SUCCESS != ret)
    {
        Log::error("parse header failed:%d",ret);
        return -1;
    }

    // 组成新节点
    rtp_list_node* pNode = new rtp_list_node;
    CHECK_POINT_INT(pNode, -1);
    pNode->data         = packetBuf;
    pNode->len          = packetSize;
    pNode->head_len     = nHeaderSize;
    pNode->playload_len = nPlayLoadSize;
    // 插入列表
    m_mapRtpList.insert(make_pair(newSeq, pNode));

    //Log::debug("parse header success, header size:%d, playload size:%d, seq:%lu"
    //    ,pNode->head_len, pNode->playload_len,newRtp->seq);


    //查询是否收到完整的帧
    pNode = nullptr;
    RtpHeader* pRTP      = nullptr;
    ps_header_t* pPS     = nullptr;

    auto it_pos          = m_mapRtpList.begin();
    auto it_end          = m_mapRtpList.end();
    auto it_first        = it_pos;   //找到的rtp帧第一个包，ps头
    auto it_last         = it_pos;    //rtp帧的最后一个包
    uint16_t seqLast     = m_nDoneSeq;

    for (; it_pos != it_end; ++it_pos)
    {
        pNode = it_pos->second;
        pRTP  = (RtpHeader *)(pNode->data);
        pPS   = (ps_header_t*)(pNode->data + pNode->head_len);

        if(it_pos == it_first) {
            /** 队列中第一个不是ps包头 */
            if(!is_ps_header(pPS)) {
                break;
            }
            /** 队列中第一个rtp包是已完成的rtp后那一个，没有中断 */
            if(m_bBegin && m_mapRtpList.size() < m_nCatchPacketNum && seqLast+1 != it_pos->first.seq) {
                break;
            }
        } else {
            //这个包不是紧接着前一个包
            if(seqLast+1 != it_pos->first.seq) {
                break;
            }
        }
        seqLast = it_pos->first.seq;

        //这个包是一个rtp帧的末尾包，且从头到尾连续
        if(pRTP->m != 0)
        {
            auto it_next = it_last = it_pos;
            it_next++;
            //组包成帧，解出PS包
            m_listRtpFrame.clear();
            for(auto pos = it_first; pos != it_next; ++pos)
                m_listRtpFrame.push_back(pos->second);
            ComposePsFrame();
            m_nDoneSeq = it_last->first.seq;
            m_bBegin = true;
            //删除已经处理好的rtp包
            for(auto pos = m_mapRtpList.begin(); pos != it_next;)
            {
                DelRtpNode(pos->second);
                pos = m_mapRtpList.erase(pos);
            }
            break;
        }
    }//for
    //查询是否收到完整的帧 end

    // 达到最大缓存数，抛弃最早的数据
    while (m_mapRtpList.size() > m_nCatchPacketNum)
    {
        auto it_begin = m_mapRtpList.begin();
        m_nDoneSeq = it_begin->first.seq;
        DelRtpNode(it_begin->second);
        m_mapRtpList.erase(it_begin);
    }
    return 0;
}

int CRtp::ParseRtpHeader(char* pBuf, long size, int& nHeaderSize, int& nPlayLoadSize)
{
    if ( size <= RTP_HEADER_SIZE )
    {
        return ERR_RTP_LENGTH ;
    }
    RtpHeader* rtp_header_ = (RtpHeader *)pBuf;

    // Check the RTP version number (it should be 2):
    if ( rtp_header_->v != RTP_VERSION )
    {
        return ERR_RTP_VERSION ;
    }

    nHeaderSize = RTP_HEADER_SIZE;
    nPlayLoadSize = size - RTP_HEADER_SIZE;

    if (rtp_header_->cc)
    {
        long cc_len = rtp_header_->cc * 4 ;
        if ( size < cc_len )
        {
            return ERR_RTP_LENGTH ;
        }
        nHeaderSize = RTP_HEADER_SIZE + cc_len;
        nPlayLoadSize -= cc_len;
    }

    // Check for (& ignore) any RTP header extension
    if ( rtp_header_->x )
    {
        if ( size < 4 )
        {
            return ERR_RTP_LENGTH ;
        }

        int32_t len = pBuf[nHeaderSize] ;
        len <<= 8 ;
        len |= pBuf[nHeaderSize + 1] ;
        len *= 4 ;
        if ( size < len ) 
        {
            return ERR_RTP_LENGTH ;
        }
        nPlayLoadSize = nPlayLoadSize - 4 - len;
        nHeaderSize = nHeaderSize + 4 + len;
    }

    // Discard any padding bytes:
    if ( rtp_header_->p )
    {
        if ( size == 0 )
        {
            return ERR_RTP_LENGTH;
        }
        long Padding = pBuf[size - 1] ;
        if ( size < Padding )
        {
            return ERR_RTP_LENGTH ;
        }
        nPlayLoadSize -= Padding ;
    }

    return ERR_RTP_SUCCESS;
}

int CRtp::DelRtpNode(rtp_list_node* pNode)
{
    CHECK_POINT_INT(pNode,0);
    SAFE_DELETE(pNode->data);
    pNode->data = nullptr;
    pNode->len = 0;
    SAFE_DELETE(pNode);
    return 0;
}

int CRtp::ComposePsFrame()
{
    // 拷贝rtp的载荷数据到帧缓存
    long nPsLen = 0;
    memset(m_frame_buf, '\0', FRAME_MAX_SIZE);
    for (auto pNode : m_listRtpFrame)
    {
        memcpy(m_frame_buf+nPsLen, pNode->data+pNode->head_len, pNode->playload_len);
        nPsLen += pNode->playload_len;  // 累加载荷大小
        if (nPsLen > FRAME_MAX_SIZE)
        {
            Log::error("CRtpAnalyzer::ComposePsFrame failed nPSLen:%ld",nPsLen);
            return -1;
        }
    }

    // PS帧组合完毕，回调处理PS帧
    if (m_pObj != nullptr)
    {
        m_pObj->RTPParseCb(m_frame_buf, nPsLen);
    }
    return 0;
}

char* CRtp::EndianChange(char* src, int bytes)
{
    if (bytes == 2)
    {
        char c=src[0];
        src[0]=src[1];
        src[1]=c;
    }
    else if (bytes == 4)
    {
        char c=src[0];
        src[0]=src[3];
        src[3]=c;
        c=src[1];
        src[1]=src[2];
        src[2]=c;
    }
    return src;
}