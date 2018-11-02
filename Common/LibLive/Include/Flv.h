#pragma once
#include "LiveInstance.h"
#include "NetStreamMaker.h"

enum flv_tag_type
{
    callback_flv_header = 0,
    callback_script_tag,
    callback_video_spspps_tag,
    callback_key_video_tag,
    callback_video_tag
};

typedef function<void(flv_tag_type,char*,uint32_t)> cbFunc;

class flv_buffer : public CNetStreamMaker
{
public:
    void append_amf_string(const char *str );
    void append_amf_double(double d );
} ;

class CFlv : public IAnalyzer
{
public:
    CFlv(void);
    ~CFlv(void);

    int InputBuffer(char* pBuf, long nLen);

    void SetCallBack(cbFunc cb);

    /**
     * 生成flv文件头信息并上抛
     * @param ppBuff 输出flv头缓存
     * @param pLen 输出flv头长度
     */
    static bool MakeHeader(char** ppBuff, int* pLen);

    /**
     * 生成一个码流描述信息，并上抛
     */
    bool MakeScriptTag();

    /**
     * 生成一个视频h264头片段并上抛
     */
    bool MakeVideoH264HeaderTag();

    /**
     * 生成一个视频断并上抛
     */
    bool MakeVideoH264Tag(char *data,int size,int bIsKeyFrame);

private:
    flv_buffer*        m_pSPS;            // 保存SPS内容的缓存
    flv_buffer*        m_pPPS;            // 保存PPS内容的缓存
    flv_buffer*        m_pData;           // 存放生成的flv数据

    uint32_t           m_timestamp;       // 时间戳
    uint32_t           m_tick_gap;        // 两帧间的间隔

    cbFunc             m_funCallBack;     // 回调方法

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

