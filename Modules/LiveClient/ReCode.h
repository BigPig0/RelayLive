#pragma once
#include "avtypes.h"
#include "ring_buff.h"
#include "uv.h"
#include "Netstreammaker.h"

	enum NalType;
namespace LiveClient
{

    class DECODETOOL;
    class CODECTOOL;

class CReCode
{
public:
    CReCode(void* handle=NULL);
    ~CReCode(void);

    /**
     * 设置通道1的信息
     */
    void SetChannel1(uint32_t nWidth, uint32_t nHeight, AV_CALLBACK cb);

    /**
     * H264数据重新解编码、缩放
     */
    int ReCode(AV_BUFF buff, NalType t);

private:
	/**
     * 生成一个视频断并上抛
     */
    bool MakeVideo(char *data,int size,int bIsKeyFrame);

	/**/
	bool MakeKeyVideo();

private:
    void            *m_hUser;

    DECODETOOL      *m_decode;
    CODECTOOL       *m_codec;

	CNetStreamMaker*        m_pSPS;            // 保存SPS内容的缓存
    CNetStreamMaker*        m_pPPS;            // 保存PPS内容的缓存
    CNetStreamMaker*        m_pKeyFrame;       // 缓存关键帧，sps和pps有可能在后面
	bool               m_bFirstKey;       // 已经处理第一个关键帧
    bool               m_bRun;            // 执行状态
    bool               m_bGotSPS;
    bool               m_bGotPPS;
	uint32_t           m_timestamp;       // 时间戳
    uint32_t           m_tick_gap;        // 两帧间的间隔
};

};