#pragma once
#include "liveObj.h"
#include "NetStreamMaker.h"

enum flv_tag_type
{
    callback_flv_header = 0,
    callback_script_tag,
    callback_video_spspps_tag,
    callback_key_video_tag,
    callback_video_tag
};


class CFlvStreamMaker : public CNetStreamMaker
{
public:
    void append_amf_string(const char *str );
    void append_amf_double(double d );
} ;

class CFlv
{
public:
    CFlv(CLiveObj* pObj);
    ~CFlv(void);

    int InputBuffer(NalType eType, char* pBuf, uint32_t nLen);

    void SetSps(uint32_t nWidth, uint32_t nHeight, double fFps);

private:
    /**
     * 生成flv文件头信息并上抛
     * @param ppBuff 输出flv头缓存
     * @param pLen 输出flv头长度
     */
    bool MakeHeader();

    /**
     * 生成一个视频断并上抛
     */
    bool MakeVideo(char *data,int size,int bIsKeyFrame);

private:
    CFlvStreamMaker*        m_pSPS;            // 保存SPS内容的缓存
    CFlvStreamMaker*        m_pPPS;            // 保存PPS内容的缓存
    CFlvStreamMaker*        m_pKeyFrame;       // 缓存关键帧，sps和pps有可能在后面
    CFlvStreamMaker*        m_pHeader;         // flv头数据
    CFlvStreamMaker*        m_pData;           // 存放生成的flv数据

    uint32_t           m_timestamp;       // 时间戳
    uint32_t           m_tick_gap;        // 两帧间的间隔

    CLiveObj*          m_pObj;

    // SPS解析出的信息
    uint32_t           m_nWidth;          // 视频宽
    uint32_t           m_nHeight;         // 视频高
    double             m_nfps;            // 视频帧率

    // 状态
    bool               m_bMakeScript;     // 创建了scriptTag
    bool               m_bFirstKey;       // 已经处理第一个关键帧
    bool               m_bRun;            // 执行状态

    CriticalSection    m_cs;
};

