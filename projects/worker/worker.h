#pragma once
#include "util.h"
#include "utilc.h"
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

    void push_ps_data(char* pBuff, int nLen);
    int get_ps_data(char* pBuff, int &nLen);
    bool ps_empty();

    void push_flv_frame(char* pBuff, int nLen);
    int get_flv_frame(char **buff);   /** 请求端获取视频数据 */
	void next_flv_frame();

	void close();
private:
	bool is_key(char* pBuff, int nLen);

public:
    void                 *m_pPlay;          // 外部播放实例
    ILiveSession         *m_pSession;       // 连接会话
    RequestParam         *m_pParam;         // 用户请求的参数
    bool                  m_bWebSocket;     // false:http请求，true:websocket
    std::string           m_strClientIP;    // 播放端的ip
	bool                  m_bConnect;       // 客户端连接状态

private:
    ring_buff_t          *m_pPSRing;       // PS数据队列
    ring_buff_t          *m_pFlvRing;      // 目标码流数据队列
	bool                  m_bParseKey;     // 是否取得第一个关键帧
    //下面两个变量用来解决单个输入数据大于ffmpeg输入缓存的情况
	util::CNetStreamMaker m_PsStream;      // 从PS队列中取出一个数据，先存到此，然后一段一段的写到ffmpeg中
	int                   m_nStreamReaded; // PS缓存数据写了多少进入ffmpeg，全部写完再从PS队列获取下一个
};

typedef bool(*funOutPlay)(CLiveWorker* worker);

/** 直播 */
CLiveWorker* CreatLiveWorker(RequestParam param, bool isWs, ILiveSession *pSession, string clientIP);

namespace Worker {
    /**
     * 初始化视频转换
     * @play 外部播放方法
     * @stop 外部停止方法
     * @findFirstKey 外部传入的ps数据，是否需要搜索第一个关键帧
     */
    void Init(funOutPlay play = NULL, funOutPlay stop = NULL, bool findFirstKey = false);
}
