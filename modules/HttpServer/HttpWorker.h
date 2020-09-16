#pragma once
#include "LiveClient.h"

namespace HttpWsServer
{
    struct pss_http_ws_live;
    enum MediaType;

    class CHttpWorker : public LiveClient::ILiveHandle
    {
    public:
        CHttpWorker(string strCode, HandleType t, int nChannel, bool isWs);
        ~CHttpWorker();

        /** 请求端获取视频数据 */
        int GetVideo(char **buff);

        virtual void play_answer(int ret, string error_info);

        /**
         * 底层推送H264进来
         */
        virtual void push_video_stream(AV_BUFF buff);

        /**
         * 底层通知播放关闭(接收rtp超时、对方关闭等)
         */
        virtual void stop();

        /**
         * 封装媒体类型
         */
        void MediaCb(AV_BUFF buff);

        virtual LiveClient::ClientInfo get_clients_info();

		void close();
    private:
        void cull_lagging_clients();

    public:
        pss_http_ws_live     *m_pPss;           //< 连接会话
        string                m_strCode;        //< 播放媒体编号
        HandleType            m_eHandleType;    //< 表明是哪一种类型
        string                m_strMIME;        //< mime type
        //MediaType             m_eMediaType;
        int                   m_nChannel;       //< 通道 0:原始码流  1:小码流
        bool                  m_bWebSocket;     //< false:http请求，true:websocket

        string                m_strPath;        //< 播放端请求地址
        string                m_strClientName;  //< 播放端的名称
        string                m_strClientIP;    //< 播放端的ip
        string                m_strError;       //< sip服务器返回的播放请求失败原因

    private:
        LiveClient::ILiveWorker *m_pLive;       //< 接收RTP数据并输出264报文
        void                    *m_pFormat;     //< 视频格式打包
        struct lws_ring         *m_pRing;       //< 缓存媒体内容的缓冲区
        AV_BUFF                  m_SocketBuff;  //< socket发送的数据缓存
		bool                     m_bConnect;       //<

        int                   m_nType;          //< 0:live直播；1:record历史视频
    };

};