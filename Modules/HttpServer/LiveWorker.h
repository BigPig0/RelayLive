#pragma once
#include "libLive.h"
enum flv_tag_type;
enum NalType;
enum MP4_FRAG_TYPE;

namespace HttpWsServer
{
    struct pss_http_ws_live;
    enum MediaType;

#define BASE_BUFF \
    char *pBuff;\
    int   nLen;

    struct BASIC_BUFF {
        BASE_BUFF
    };

    /**
     * FLV tag内容
     */
    struct FLV_TAG_BUFF {
        BASE_BUFF
        flv_tag_type  eType; // 标记视频tag是否是sps_pps,关键帧，非关键帧
    };

    /**
     * H264 nalu内容
     */
    struct H264_NALU_BUFF {
        BASE_BUFF
        NalType eType; //h264帧类型
    };

    /**
     * MP4 box内容
     */
    struct MP4_FRAG_BUFF {
        BASE_BUFF
        MP4_FRAG_TYPE eType;
    };

    class CLiveWorker : public IlibLiveCb
    {
    public:
        CLiveWorker(string strCode, int rtpPort);
        ~CLiveWorker();

        /** 客户端连接 */
        bool AddConnect(pss_http_ws_live* pss);
        bool DelConnect(pss_http_ws_live* pss);

        /** 从源过来的视频数据，单线程输入 */
        void push_flv_frame(int tag_type, char* pBuff, int nLen);
        void push_h264_stream(NalType eType, char* pBuff, int nLen);
        void push_ts_stream(char* pBuff, int nLen);
        void push_mp4_stream(MP4_FRAG_TYPE eType, char* pBuff, int nBuffSize);

        /** 请求端获取视频数据 */
        FLV_TAG_BUFF GetFlvHeader();
        FLV_TAG_BUFF GetFlvVideo(uint32_t *tail);
        //------------------------------------------
        H264_NALU_BUFF GetH264Video(uint32_t *tail);
        //------------------------------------------
        MP4_FRAG_BUFF GetMp4Header();
        MP4_FRAG_BUFF GetMp4Video(uint32_t *tail);
        //------------------------------------------
        void NextWork(pss_http_ws_live* pss);

        /** 获取客户端信息 */
        string GetClientInfo();
    private:
        void cull_lagging_clients(MediaType type);

        void stop();

    private:
        string                m_strCode;     // 播放媒体编号

        //flv
        FLV_TAG_BUFF          m_stFlvScript;     // ScriptTag，只有一个
        FLV_TAG_BUFF          m_stFlvDecCof;     // 第一个VideoTag，根据sps和pps生成的AVCDecoderConfigurationRecord
        struct lws_ring       *m_pFlvRing;
        pss_http_ws_live      *m_pFlvPssList;

        //h264
        struct lws_ring       *m_pH264Ring;
        pss_http_ws_live      *m_pH264PssList;

        //fMP4
        MP4_FRAG_BUFF         m_stMP4Head;
        struct lws_ring       *m_pMP4Ring;
        pss_http_ws_live      *m_pMP4PssList;

        int                   m_nType;          //< 0:live直播；1:record历史视频
        IlibLive*             m_pLive;          //< 直播数据接收和解包装包
        int                   m_nPort;          //< rtp接收端口
    };

    /** 直播 */
    CLiveWorker* CreatLiveWorker(string strCode);
    CLiveWorker* GetLiveWorker(string strCode);
    bool DelLiveWorker(string strCode);

    /** 点播 */
};