#pragma once
#include "avtypes.h"
#include "ring_buff.h"
#include "uv.h"
#include "Netstreammaker.h"

enum NalType;
namespace LiveClient
{

    class CDecoder;
    class CEncoder;

    class CRecoder
    {
    public:
        CRecoder(void* handle=NULL);
        ~CRecoder(void);

        /**
        * 设置通道1的信息
        */
        void SetChannel1(uint32_t nWidth, uint32_t nHeight, AV_CALLBACK cb);

        /**
        * H264数据重新解编码、缩放
        */
        int Recode(AV_BUFF buff, NalType t);

        void RecodeThread();

    private:
        int Recode2(AV_BUFF buff, NalType t);

        /**
        * 生成一个视频断并上抛
        */
        bool EncodeVideo(char *data,int size,int bIsKeyFrame);

        /**/
        bool EncodeKeyVideo();

    private:
        void              *m_hUser;
        CDecoder          *m_decode;          // 解码原始码流
        CEncoder          *m_codec;           // 子码流编码
        CNetStreamMaker   *m_pSPS;            // 保存SPS内容的缓存
        CNetStreamMaker   *m_pPPS;            // 保存PPS内容的缓存
        CNetStreamMaker   *m_pKeyFrame;       // 缓存关键帧，sps和pps有可能在后面
        bool               m_bFirstKey;       // 已经处理第一个关键帧
        bool               m_bRun;            // 执行状态
        bool               m_bFinish;         // 执行完成
        bool               m_bGotSPS;         // 标记解码是否输出了sps
        bool               m_bGotPPS;         // 标记解码是否输出了sps
        uint32_t           m_timestamp;       // 时间戳
        uint32_t           m_tick_gap;        // 两帧间的间隔
        ring_buff_t       *m_pRingH264;       // 缓存h264数据
    };

};