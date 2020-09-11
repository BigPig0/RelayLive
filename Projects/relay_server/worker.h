#pragma once
#include "ring_buff.h"
#include "util.h"
#include "util_netstream.h"
#include <string>
#include <list>

class ILiveSession;
struct RequestParam;

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


public:
    ILiveSession         *m_pSession;       // 连接会话
    RequestParam         *m_pParam;         // 用户请求的参数
    bool                  m_bWebSocket;     // false:http请求，true:websocket
    std::string           m_strClientIP;    // 播放端的ip

private:
    ring_buff_t          *m_pFlvRing;       //< 目标码流数据队列
	bool                  m_bConnect;       //< 客户端连接状态
};

/** 直播 */
CLiveWorker* CreatLiveWorker(RequestParam param, bool isWs, ILiveSession *pSession, string clientIP);

void InitFFmpeg();
