#pragma once
#include "ring_buff.h"
#include <string>

namespace Server
{
    struct pss_http_ws_live;
    enum MediaType;
    typedef struct _AV_BUFF_ AV_BUFF;

    class CLiveWorker
    {
    public:
        CLiveWorker();
        ~CLiveWorker();

        bool Play();

        /** 请求端获取视频数据 */
        int GetVideo(char **buff);

        void play_answer(int ret, std::string error_info);

        void push_ps_data(char* pBuff, int nLen);
        int get_ps_data(char* pBuff, int &nLen);

        void push_flv_frame(char* pBuff, int nLen);

        /**
         * 底层通知播放关闭(接收rtp超时、对方关闭等)
         */
        void stop();

		void close();
    private:
        void cull_lagging_clients();

    public:
        pss_http_ws_live     *m_pPss;           //< 连接会话
        std::string           m_strCode;        //< 播放媒体编号
        std::string           m_strType;        // 目标媒体类型 flv mp4 h264
        std::string           m_strHw;          // 目标媒体分辨率 空表示不变
        //HandleType            m_eHandleType;    //< 表明是哪一种类型
        std::string           m_strMIME;        //< mime type
        //MediaType             m_eMediaType;
        //int                   m_nChannel;       //< 通道 0:原始码流  1:小码流
        bool                  m_bWebSocket;     //< false:http请求，true:websocket

        std::string           m_strPath;        //< 播放端请求地址
        std::string           m_strClientName;  //< 播放端的名称
        std::string           m_strClientIP;    //< 播放端的ip
        std::string           m_strError;       //< sip服务器返回的播放请求失败原因

    private:
        //void                 *m_pFormat;     //< 视频格式打包
        ring_buff_t          *m_pPSRing;       //< PS数据队列
        ring_buff_t          *m_pRing;         //< 目标码流数据队列
        std::string           m_SocketBuff;    //< socket发送的数据缓存
		bool                  m_bConnect;      //<
        bool                  m_bStop;

        //int                   m_nType;          //< 0:live直播；1:record历史视频
    };

    /** 直播 */
    CLiveWorker* CreatLiveWorker(std::string strCode, std::string strType, std::string strHw, bool isWs, pss_http_ws_live *pss);

    void InitFFmpeg();

};