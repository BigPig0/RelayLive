#pragma once
#include "commonDefine.h"
#include "avtypes.h"
#include "NetStreamMaker.h"
#include "h264.h"

/** 直播时用的fmp4格式
---------------fmp4-head-----------------------------
ftyp
moov    mvhd
        trak    tkhd
                media   mdhd
                        hdlr
                        minf    head    vmhd
                                        smhd
                                        hmhd
                                        nmhd
                                dinf    dref    url
                                                urn
                                stbl    stsd
                                        stts
                                        stsz
                                        stsc
                                        stss
                                        stco
        trak...
        mvex    trex
--------------fmp4-fragment--------------------------
moof    mfhd
        traf    tfhd
                tfdt
                trun
mdat
moof
mdat...
 */

class CMP4
{
public:
    CMP4(AV_CALLBACK cb, void* handle=NULL);
    ~CMP4();

    int Code(AV_BUFF buff);

    void SetSps(uint32_t nWidth, uint32_t nHeight, double fFps);

    void SetNodelay(uint32_t nodelay){m_nNodelay = nodelay;};

private:
    /**
     * 生成mp4文件头信息并上抛
     */
    bool MakeHeader();

    /**
     * 生成mp4片段
     */
    bool MakeMP4Frag(bool bIsKeyFrame);

    /**
     * 生成一个视频断并上抛
     */
    bool MakeVideo(char *data, uint32_t size, bool bIsKeyFrame);

    /* 生成关键帧 */
    bool MakeKeyVideo();

private:
    CNetStreamMaker    *m_pSPS;            // 缓存SPS
    CNetStreamMaker    *m_pPPS;            // 缓存PPS
    CNetStreamMaker    *m_pKeyFrame;       // 缓存关键帧，sps和pps有可能在后面
    CNetStreamMaker    *m_pMdat;           // 缓存h264数据
    CNetStreamMaker    *m_pSamplePos;      // 缓存每个sample的位移，trun的sample_composition_time_offset
    uint32_t           m_nSampleNum;       // 缓存的sample个数，即不包含sps和pps的nalu个数(每个nalu为一帧)
    CNetStreamMaker    *m_pHeader;         // 保存生成的MP4头
    CNetStreamMaker    *m_pFragment;       // 保存生成的MP4内容

    uint32_t           m_timestamp;       // 时间戳
    uint32_t           m_tick_gap;        // 两帧间的间隔
    uint32_t           m_nNodelay;        // 是否立即发送

    void*              m_hUser;           // 回调处理对象
    AV_CALLBACK        m_fCB;

    // SPS解析出的信息
    uint32_t           m_nWidth;          // 视频宽
    uint32_t           m_nHeight;         // 视频高
    uint32_t           m_nHorizresolution;// 水平dpi
    uint32_t           m_mVertresolution; // 垂直dpi
    double             m_nfps;            // 视频帧率

    // 状态
    bool               m_bMakeHeader;     // 创建了MP4头
    bool               m_bFirstKey;       // 已经处理第一个关键帧
    bool               m_bRun;            // 执行状态
    bool               m_bGotSPS;
    bool               m_bGotPPS;
    uint32_t           m_nSeq;            // MP4片段序号
};