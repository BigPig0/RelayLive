#include "common.h"
#include "LiveWorker.h"
#include "LiveClient.h"
#include "liveReceiver.h"
#include "LiveChannel.h"
#include "LiveIpc.h"

#include "uv.h"
#include "cstl.h"
#include "cstl_easy.h"
#include "utilc.h"

namespace LiveClient
{
	extern uv_loop_t      *g_uv_loop;
    extern LIVECLIENT_CB  liveclient_respond;

#define ADDHANDLE    0X0001
#define REMOVEHANDLE 0X0002
#define RECEIVING    0X0004
#define CLOSE        0X0008
#define CLIENTSINFO  0X0010

    struct adding_handle {
        ILiveHandle* handle;
        HandleType   type;
        int          channel;
    };

    struct live_event_loop_t {
        uv_loop_t                loop;
        uv_async_t               async;
        uv_timer_t               timer_stop;    //< http播放端全部连开连接后延迟销毁，以便页面刷新时快速播放
        bool                     running;          // loop线程是否正在执行
        int                      ref_num;      // 启动的uv句柄数，这些句柄通过uv_close关闭
        int                      async_event;
        ring_buff_t              *ring;         //< 视频码流缓存
        list_t                   *adding_list;
        uv_mutex_t               uv_mutex_add;
        list_t                   *removing_list;
        uv_mutex_t               uv_mutex_remove;
        void                     *usr;
    };

    /** 关闭uv句柄的回调 */
    static void live_close_cb(uv_handle_t* handle){
        live_event_loop_t *loop = (live_event_loop_t*)handle->data;
        loop->ref_num--;
        //uv句柄全部关闭后，停止loop
        if(!loop->ref_num){
            loop->running = false;
            uv_stop(&loop->loop);
        }
    }

    /** 外部线程通知回调，在loop回调中关闭用到的uv句柄 */
    static void async_cb(uv_async_t* handle){
        live_event_loop_t *loop = (live_event_loop_t*)handle->data;
        CLiveWorker *live = (CLiveWorker*)loop->usr;
        if(loop->async_event & ADDHANDLE) {
            // 新增客户端连接实例
            loop->async_event &= ~ADDHANDLE;
            uv_mutex_lock(&loop->uv_mutex_add);
            LIST_FOR_BEGIN(loop->adding_list, adding_handle*, h){
                live->AddHandleAsync(h->handle, h->type, h->channel);
                free(h);
                //如果刚刚开启了结束定时器，需要将其关闭
                if(uv_is_active((const uv_handle_t*)&loop->timer_stop)) {
                    uv_timer_stop(&loop->timer_stop);
                    Log::debug("new client comed, and will not close live stream");
                }
            }LIST_FOR_END
            list_clear(loop->adding_list);
            uv_mutex_unlock(&loop->uv_mutex_add);
        } 
		if(loop->async_event & REMOVEHANDLE) {
            // 移除客户端连接实例
            loop->async_event &= ~REMOVEHANDLE;
            uv_mutex_lock(&loop->uv_mutex_remove);
            LIST_FOR_BEGIN(loop->removing_list, ILiveHandle*, h){
                live->RemoveHandleAsync(h);
            }LIST_FOR_END
            list_clear(loop->removing_list);
            uv_mutex_unlock(&loop->uv_mutex_remove);
        } 
		if(loop->async_event & RECEIVING) {
            // 上抛码流处理
            loop->async_event &= ~RECEIVING;
            AV_BUFF *buff = (AV_BUFF*)simple_ring_get_element(loop->ring);
			while(buff){
				live->ReceiveStreamAsync(*buff);
				simple_ring_cosume(loop->ring);
				buff = (AV_BUFF*)simple_ring_get_element(loop->ring);
			}
        } 
		if(loop->async_event & CLOSE) {
            // 结束
            loop->async_event &= ~CLOSE;
            uv_close((uv_handle_t*)&loop->async, live_close_cb);
            uv_close((uv_handle_t*)&loop->timer_stop, live_close_cb);
        } 
		if(loop->async_event & CLIENTSINFO) {
            //查看当前live对应的客户端连接信息
            loop->async_event &= ~CLIENTSINFO;
            live->GetClientInfoAsync();
        }
    }

    /** 客户端全部断开后，延时断开源的定时器 */
    static void stop_timer_cb(uv_timer_t* handle) {
        live_event_loop_t *loop = (live_event_loop_t*)handle->data;
        CLiveWorker *live = (CLiveWorker*)loop->usr;
        int ret = uv_timer_stop(handle);
        if(ret < 0) {
            Log::error("timer stop error:%s",uv_strerror(ret));
        }
        live->Clear2Stop();
    }

    /** event loop thread */
    static void run_loop_thread(void* arg) {
        live_event_loop_t *loop = (live_event_loop_t*)arg;
        CLiveWorker *live = (CLiveWorker*)loop->usr;
        live->m_nRunNum++;
        while (loop->running) {
            uv_run(&loop->loop, UV_RUN_DEFAULT);
            Sleep(40);
        }
        uv_loop_close(&loop->loop);
        destroy_ring_buff(loop->ring);
        list_destroy(loop->adding_list);
        list_destroy(loop->removing_list);
        delete loop;
        live->m_nRunNum--;
    }

	static void destory_avbuf(void* arg) {
		AV_BUFF *buff = (AV_BUFF*)arg;
		SAFE_FREE(buff->pData);
	}

    /** 外部线程新建一个直播事件循环 */
    live_event_loop_t* create_live_event_loop(void *usr) {
        SAFE_MALLOC(live_event_loop_t, loop);
        loop->usr = usr;

        //初始化loop
        loop->loop.data = loop;
        int ret =uv_loop_init(&loop->loop);
        if(ret < 0) {
            Log::error("uv_loop init error: %s", uv_strerror(ret));
            return NULL;
        }

        //async
        loop->async.data = loop;
        ret = uv_async_init(&loop->loop, &loop->async, async_cb);
        loop->ref_num++;

        //timer
        loop->timer_stop.data = loop;
        ret = uv_timer_init(&loop->loop, &loop->timer_stop);
        loop->ref_num++;

        //ring buff
        loop->ring = create_ring_buff(sizeof(AV_BUFF), 100, destory_avbuf);

        //adding list
        loop->adding_list = create_list(void*); //list<adding_handle>
        list_init(loop->adding_list);
        uv_mutex_init(&loop->uv_mutex_add);

        //removing list
        loop->removing_list = create_list(void*); //list<ILiveHandle>
        list_init(loop->removing_list);
        uv_mutex_init(&loop->uv_mutex_remove);

        //live worker实例中loop线程
        loop->running = true;
        uv_thread_t tid;
        uv_thread_create(&tid, run_loop_thread, loop);

		return loop;
    }

    /** 外部线程关闭事件循环 */
    void close_live_event_loop(live_event_loop_t* loop){
        loop->running = false;
        loop->async_event |= CLOSE;
        uv_async_send(&loop->async);
    }

    /** 外部线程添加客户端连接 */
    void live_event_add_handle(live_event_loop_t* loop, ILiveHandle* h, HandleType t, int c){
        SAFE_MALLOC(adding_handle, new_handle);
        new_handle->handle = h;
        new_handle->type = t;
        new_handle->channel = c;
        uv_mutex_lock(&loop->uv_mutex_add);
        list_push_back(loop->adding_list, new_handle);
        uv_mutex_unlock(&loop->uv_mutex_add);

        loop->async_event |= ADDHANDLE;
        uv_async_send(&loop->async);
    }

    /** 外部线程移除客户端连接 */
    void live_event_remove_handle(live_event_loop_t* loop, ILiveHandle* h){
        uv_mutex_lock(&loop->uv_mutex_remove);
		list_push_back(loop->removing_list, h);
        uv_mutex_unlock(&loop->uv_mutex_remove);

        loop->async_event |= REMOVEHANDLE;
        uv_async_send(&loop->async);
    }

    /** 外部线程输入裸码流 */
    void live_event_push_stream(live_event_loop_t* loop, AV_BUFF *buff){
		AV_BUFF tmp = *buff;
		tmp.pData = (char*)malloc(tmp.nLen);
		memcpy(tmp.pData, buff->pData, tmp.nLen);
        simple_ring_insert(loop->ring, &tmp);
        loop->async_event |= RECEIVING;
        uv_async_send(&loop->async);
    }

    /** 外部线程请求查询客户端连接信息 */
    void live_event_get_clients_info(live_event_loop_t* loop) {
        loop->async_event |= CLIENTSINFO;
        uv_async_send(&loop->async);
    }

    /** 外部线程启动结束定时器，指定时间内有新客户端请求，则不结束 */
    void live_event_end_timer(live_event_loop_t* loop) {
        uv_timer_start(&loop->timer_stop, stop_timer_cb, 5000, 0);
    }
    
    //////////////////////////////////////////////////////////////////////////

    /** 查询客户端连接的信息 */
    static int _clients_req_num = 0;
    static vector<ClientInfo> _clients_info;
    static CriticalSection _cs_clients;

    /** 直播实例获得客户端信息后，发送到这里汇总 */
    static void PushWorkerClientsInfo(vector<ClientInfo> infos) {
        _clients_req_num--;
        MutexLock lock(&_cs_clients);
        _clients_info.insert(_clients_info.end(), infos.begin(), infos.end());

        if(!_clients_req_num && liveclient_respond) {
            stringstream ss;
            ss << "{\"root\":[";
            for(auto c:_clients_info){
                ss << "{\"DeviceID\":\"" << c.devCode 
                    << "\",\"Connect\":\"" << c.connect
                    << "\",\"Media\":\"" << c.media
                    << "\",\"ClientIP\":\"" << c.clientIP
                    << "\",\"Channel\":\"" << c.channel
                    << "\"},";
            }
            string strResJson = StringHandle::StringTrimRight(ss.str(),',');
            strResJson += "]}";
            liveclient_respond("clientsinfo", strResJson);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    /** rtp接收配置 */
    extern string        g_strRtpIP;       //< RTP服务IP
    extern list<int>     g_vecRtpPort;     //< RTP可用端口，使用时从中取出，使用结束重新放入

    static int GetRtpPort()
    {
        int nRet = -1;
        if(!g_vecRtpPort.empty()){
            nRet = g_vecRtpPort.front();
            g_vecRtpPort.pop_front();
        }
        return nRet;
    }

    static void GiveBackRtpPort(int nPort)
    {
        g_vecRtpPort.push_back(nPort);
    }

    //////////////////////////////////////////////////////////////////////////

    static map<string,CLiveWorker*>  m_workerMap;   //live worker 由http模块 event loop单线程访问

    /** CLiveWorker析构中删除m_pLive比较耗时，会阻塞event loop，因此使用线程。 */
    static void live_worker_destory_thread(void* arg) {
        CLiveWorker* live = (CLiveWorker*)arg;
        SAFE_DELETE(live);
    }

    CLiveWorker* CreatLiveWorker(string strCode)
    {
        Log::debug("CreatFlvBuffer begin");
        int rtpPort = GetRtpPort();
        if(rtpPort < 0) {
            Log::error("play failed %s, no rtp port",strCode.c_str());
            return nullptr;
        }

        CLiveWorker* pNew = new CLiveWorker(strCode, rtpPort);
        m_workerMap.insert(make_pair(strCode, pNew));

        LiveIpc::RealPlay(strCode, g_strRtpIP,  rtpPort);
        Log::debug("RealPlay start: %s",strCode.c_str());

        return pNew;
    }

    CLiveWorker* GetLiveWorker(string strCode)
    {
        auto itFind = m_workerMap.find(strCode);
        if (itFind != m_workerMap.end())
        {
            // 已经存在
            return itFind->second;
        }

        return nullptr;
    }

    bool DelLiveWorker(string strCode)
    {
        auto itFind = m_workerMap.find(strCode);
        if (itFind != m_workerMap.end())
        {
            // CLiveWorker析构中删除m_pLive比较耗时，会阻塞event loop，因此使用线程销毁对象。
            uv_thread_t tid;
            uv_thread_create(&tid, live_worker_destory_thread, itFind->second);

            m_workerMap.erase(itFind);
            return true;
        }

        Log::error("dosn't find worker object");
        return false;
    }

	void GetAllWorkerClientsInfo(){
        _clients_req_num = 0;
        _cs_clients.lock();
        _clients_info.clear();
        _cs_clients.unlock();
        
		if(m_workerMap.empty()){
			_clients_req_num = 1;
			vector<ClientInfo> ret;
			PushWorkerClientsInfo(ret);
			return;
		}

        for (auto w : m_workerMap) {
            CLiveWorker *worker = w.second;
            _clients_req_num++;
			worker->GetClientInfo();
		}
    }

    bool AsyncPlayCB(PlayAnswerList *pal){
        CLiveWorker* pWorker = GetLiveWorker(pal->strDevCode);
        if(!pWorker){
            Log::error("get live worker failed %s", pal->strDevCode.c_str());
            return false;
        }
        pWorker->PlayAnswer(pal);
        return true;
    }

    //////////////////////////////////////////////////////////////////////////

    static void H264DecodeCb(AV_BUFF buff, void* user) {
        CLiveWorker* live = (CLiveWorker*)user;
#ifdef EXTEND_CHANNLES
        live->ReceiveYUV(buff);
#endif
    }

    CLiveWorker::CLiveWorker(string strCode, int rtpPort)
        : m_strCode(strCode)
        , m_bPlayed(false)
        , m_nPlayRes(0)
        , m_nPort(rtpPort)
        , m_nType(0)
        , m_pReceiver(nullptr)
        , m_pOrigin(nullptr)
#ifdef EXTEND_CHANNELS
        , m_pDecoder(nullptr)
#endif
        , m_bStop(false)
        , m_bOver(false)
        , m_stream_type(RTP_STREAM_UNKNOW)
        , m_nRunNum(0)
    {
        m_pEventLoop = create_live_event_loop(this);
    }

    CLiveWorker::~CLiveWorker()
    {
        if(LiveIpc::StopPlay(m_nPort)) {
            Log::error("stop play failed");
        }
        SAFE_DELETE(m_pReceiver);
        close_live_event_loop(m_pEventLoop);
        while (m_nRunNum)
            sleep(50);

        SAFE_DELETE(m_pOrigin);
#ifdef EXTEND_CHANNLES
        for(auto it:m_mapChlEx){
            SAFE_DELETE(it.second);
        }
        m_mapChlEx.clear();
#endif
        GiveBackRtpPort(m_nPort);
        Log::debug("CLiveWorker release");
    }

    bool CLiveWorker::AddHandle(ILiveHandle* h, HandleType t, int c)
    {
        live_event_add_handle(m_pEventLoop, h, t, c);
        return true;
    }

    bool CLiveWorker::AddHandleAsync(ILiveHandle* h, HandleType t, int c)
    {
        if(c == 0) {
            // 原始通道
            if(!m_pOrigin)
                m_pOrigin = new CLiveChannel;
            CHECK_POINT_BOOL(m_pOrigin);
            m_pOrigin->AddHandle(h, t);
        } else {
            // 扩展通道
#ifdef EXTEND_CHANNELS
            //MutexLock lock(&m_csChls);
            auto fit = m_mapChlEx.find(c);
            if(fit != m_mapChlEx.end()) {
                fit->second->AddHandle(h, t);
            } else {
                CLiveChannel *nc = new CLiveChannel(c, 640, 480);
                nc->AddHandle(h, t);
                nc->SetDecoder(m_pDecoder);
                m_mapChlEx.insert(make_pair(c, nc));
            }
#else
            // 原始通道
            if(!m_pOrigin)
                m_pOrigin = new CLiveChannel;
            CHECK_POINT_BOOL(m_pOrigin);
            m_pOrigin->AddHandle(h, t);
#endif
        }
        return true;
    }

    bool CLiveWorker::RemoveHandle(ILiveHandle* h)
    {
        live_event_remove_handle(m_pEventLoop, h);
        return true;
    }

    bool CLiveWorker::RemoveHandleAsync(ILiveHandle* h)
    {
        // 原始通道
        bool bOriginEmpty = true;
        if(m_pOrigin){
            bOriginEmpty = m_pOrigin->RemoveHandle(h);
            if(bOriginEmpty) {
                SAFE_DELETE(m_pOrigin);
            }
        }

        //扩展通道
        bool bExEmpty = true;
#ifdef EXTEND_CHANNLES
        //MutexLock lock(&m_csChls);
        for(auto it = m_mapChlEx.begin(); it != m_mapChlEx.end();) {
            bool bEmpty = it->second->RemoveHandle(h);
            if(bEmpty) {
                delete it->second;
                it = m_mapChlEx.erase(it);
            } else {
                it++;
                bExEmpty = false;
            }
        }
#endif

        if(bExEmpty && bOriginEmpty) {
            if(m_bOver) {
                // 视频源没有数据，超时，不需要延时
                Clear2Stop();
            } else {
                // 视频源依然连接，延时N秒再销毁对象，以便短时间内有新请求能快速播放
                live_event_end_timer(m_pEventLoop);
            }
        }
        return true;
    }

    //string CLiveWorker::GetSDP(){
    //    return m_strPlayInfo;
    //}

	void CLiveWorker::Clear2Stop() {
        bool bOriginEmpty = !m_pOrigin || m_pOrigin->Empty();

        //MutexLock lock(&m_csChls);
        bool bExEmpty = true;
#ifdef EXTEND_CHANNLES
        bExEmpty = m_mapChlEx.empty();
#endif

        if(bOriginEmpty && bExEmpty) {
			Log::debug("need close live stream");
            //首先从map中移走对象
            DelLiveWorker(m_strCode);
		}
	}

    void CLiveWorker::stop()
    {
        //视频源没有数据并超时
        Log::debug("no data recived any more, stopped");
        //状态改变为超时，此时前端全部断开，不需要延时，直接销毁
        m_bOver = true;

        //通知所有通道断开前端连接
		if(m_pOrigin)
			m_pOrigin->stop();
        //MutexLock lock(&m_csChls);
#ifdef EXTEND_CHANNLES
        for(auto it:m_mapChlEx){
            it.second->stop();
        }
#endif
    }

    void CLiveWorker::GetClientInfo()
    {
        live_event_get_clients_info(m_pEventLoop);
    }

    void CLiveWorker::GetClientInfoAsync()
    {
		vector<ClientInfo> ret;
		if(m_pOrigin)
			ret = m_pOrigin->GetClientInfo();
        //MutexLock lock(&m_csChls);
#ifdef EXTEND_CHANNLES
        for(auto chl : m_mapChlEx){
            vector<ClientInfo> tmp = chl.second->GetClientInfo();
            ret.insert(ret.end(), tmp.begin(), tmp.end());
        }
#endif
        PushWorkerClientsInfo(ret);
    }

    void CLiveWorker::ReceiveStream(AV_BUFF buff){
        live_event_push_stream(m_pEventLoop, &buff);
    }

    void CLiveWorker::ReceiveStreamAsync(AV_BUFF buff)
    {
        if(buff.eType == AV_TYPE::RTP) {

        } else if(buff.eType == AV_TYPE::H264_NALU) {
            //原始通道，将h264码流转发过去
            if(m_pOrigin){
                m_pOrigin->ReceiveStream(buff);
            }
            //扩展通道，将码流解码成yuv再发送过去
#ifdef EXTEND_CHANNELS
            //MutexLock lock(&m_csChls);
            if(!m_mapChlEx.empty()){
                if(nullptr == m_pDecoder) {
                    m_pDecoder = IDecoder::Create(H264DecodeCb,this);
                    for(auto it:m_mapChlEx){
                        it.second->SetDecoder(m_pDecoder);
                    }
                }
                m_pDecoder->Decode(buff);
            } else {
                SAFE_DELETE(m_pDecoder);
            }
#endif
        }
    }

#ifdef EXTEND_CHANNELS
    void CLiveWorker::ReceiveYUV(AV_BUFF buff)
    {
        for (auto it:m_mapChlEx)
        {
            it.second->ReceiveStream(buff);
        }
    }
#endif

    void CLiveWorker::PlayAnswer(PlayAnswerList *pal){
        if(!pal->nRet){
            Log::debug("RealPlay ok: %s",pal->strDevCode.c_str());

            //从sdp解析出视频源ip和端口
            m_strPlayInfo = pal->strMark;
            ParseSdp();

            //启动监听
            m_pReceiver = new CLiveReceiver(m_nPort, this, m_stream_type);
            m_pReceiver->m_strRemoteIP     = m_strServerIP;
            m_pReceiver->m_nRemoteRTPPort  = m_nServerPort;
            m_pReceiver->m_nRemoteRTCPPort = m_nServerPort+1;
            m_pReceiver->StartListen();
        } else {
            Log::error("RealPlay failed: %s %s",pal->strDevCode.c_str(), pal->strMark.c_str());
        }

        m_nPlayRes = pal->nRet;
        m_bPlayed = true;
    }

    void CLiveWorker::ParseSdp()
    {
        //从sdp解析出视频源ip和端口
        bnf_t* sdp_bnf = create_bnf(m_strPlayInfo.c_str(), m_strPlayInfo.size());
        char *sdp_line = NULL;
        char remoteIP[25]={0};
        int remotePort = 0;
        while (bnf_line(sdp_bnf, &sdp_line)) {
            if(sdp_line[0]=='c'){
                sscanf(sdp_line, "c=IN IP4 %[^/\r\n]", remoteIP);
                m_strServerIP = remoteIP;
            } else if(sdp_line[0]=='m') {
                sscanf(sdp_line, "m=video %d ", &remotePort);
                m_nServerPort = remotePort;
            }

            if(sdp_line[0]=='a' && !strncmp(sdp_line, "a=rtpmap:", 9)){
                //a=rtpmap:96 PS/90000
                //a=rtpmap:96 H264/90000
                char tmp[256]={0};
                int num = 0;
                char type[20]={0};
                int bitrate = 0;
                sscanf(sdp_line, "a=rtpmap:%d %[^/]/%d", &num, type, &bitrate);
                if(!strcmp(type, "PS") || !strcmp(type, "MP2P")){
                    m_stream_type = RTP_STREAM_PS;
                }else{
                    m_stream_type = RTP_STREAM_H264;
                }
            }
        }
        destory_bnf(sdp_bnf);
    }
}
