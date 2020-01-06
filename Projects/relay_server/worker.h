#pragma once
#include "ring_buff.h"
#include "util.h"
#include <string>
#include <list>

namespace Server
{
    struct pss_live;
    enum MediaType;

    class CLiveWorker
    {
    public:
        CLiveWorker();
        ~CLiveWorker();

        bool Play();

        void push_flv_frame(char* pBuff, int nLen);
        int get_flv_frame(char **buff);   /** 请求端获取视频数据 */
		void next_flv_frame();

		void close();
    private:
        void cull_lagging_clients();

    public:
        pss_live             *m_pPss;           //< 连接会话
        std::string           m_strUrl;         //< 播放媒体url
        std::string           m_strType;        // 目标媒体类型 flv mp4 h264
        std::string           m_strHw;          // 目标媒体分辨率 空表示不变
        std::string           m_strMIME;        //< mime type
        bool                  m_bWebSocket;     //< false:http请求，true:websocket

        std::string           m_strPath;        //< 播放端请求地址
        std::string           m_strClientName;  //< 播放端的名称
        std::string           m_strClientIP;    //< 播放端的ip
        std::string           m_strError;       //< sip服务器返回的播放请求失败原因

    private:
        ring_buff_t          *m_pFlvRing;       //< 目标码流数据队列
        std::string           m_SocketBuff;     //< socket发送的数据缓存
		bool                  m_bConnect;       //< 客户端连接状态
    };

    /** 直播 */
    CLiveWorker* CreatLiveWorker(std::string strURL, std::string strType, std::string strHw, bool isWs, pss_live *pss, string clientIP);

    void InitFFmpeg();

};