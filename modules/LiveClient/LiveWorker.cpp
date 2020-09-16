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
        uv_timer_t               timer_stop;    //< http���Ŷ�ȫ���������Ӻ��ӳ����٣��Ա�ҳ��ˢ��ʱ���ٲ���
        bool                     running;          // loop�߳��Ƿ�����ִ��
        int                      ref_num;      // ������uv���������Щ���ͨ��uv_close�ر�
        int                      async_event;
        ring_buff_t              *ring;         //< ��Ƶ��������
        list_t                   *adding_list;
        uv_mutex_t               uv_mutex_add;
        list_t                   *removing_list;
        uv_mutex_t               uv_mutex_remove;
        void                     *usr;
    };

    /** �ر�uv����Ļص� */
    static void live_close_cb(uv_handle_t* handle){
        live_event_loop_t *loop = (live_event_loop_t*)handle->data;
        loop->ref_num--;
        //uv���ȫ���رպ�ֹͣloop
        if(!loop->ref_num){
            loop->running = false;
            uv_stop(&loop->loop);
        }
    }

    /** �ⲿ�߳�֪ͨ�ص�����loop�ص��йر��õ���uv��� */
    static void async_cb(uv_async_t* handle){
        live_event_loop_t *loop = (live_event_loop_t*)handle->data;
        CLiveWorker *live = (CLiveWorker*)loop->usr;
        if(loop->async_event & ADDHANDLE) {
            // �����ͻ�������ʵ��
            loop->async_event &= ~ADDHANDLE;
            uv_mutex_lock(&loop->uv_mutex_add);
            LIST_FOR_BEGIN(loop->adding_list, adding_handle*, h){
                live->AddHandleAsync(h->handle, h->type, h->channel);
                free(h);
                //����ոտ����˽�����ʱ������Ҫ����ر�
                if(uv_is_active((const uv_handle_t*)&loop->timer_stop)) {
                    uv_timer_stop(&loop->timer_stop);
                    Log::debug("new client comed, and will not close live stream");
                }
            }LIST_FOR_END
            list_clear(loop->adding_list);
            uv_mutex_unlock(&loop->uv_mutex_add);
        } 
		if(loop->async_event & REMOVEHANDLE) {
            // �Ƴ��ͻ�������ʵ��
            loop->async_event &= ~REMOVEHANDLE;
            uv_mutex_lock(&loop->uv_mutex_remove);
            LIST_FOR_BEGIN(loop->removing_list, ILiveHandle*, h){
                live->RemoveHandleAsync(h);
            }LIST_FOR_END
            list_clear(loop->removing_list);
            uv_mutex_unlock(&loop->uv_mutex_remove);
        } 
		if(loop->async_event & RECEIVING) {
            // ������������
            loop->async_event &= ~RECEIVING;
            AV_BUFF *buff = (AV_BUFF*)simple_ring_get_element(loop->ring);
			while(buff){
				live->ReceiveStreamAsync(*buff);
				simple_ring_cosume(loop->ring);
				buff = (AV_BUFF*)simple_ring_get_element(loop->ring);
			}
        } 
		if(loop->async_event & CLOSE) {
            // ����
            loop->async_event &= ~CLOSE;
            uv_close((uv_handle_t*)&loop->async, live_close_cb);
            uv_close((uv_handle_t*)&loop->timer_stop, live_close_cb);
        } 
		if(loop->async_event & CLIENTSINFO) {
            //�鿴��ǰlive��Ӧ�Ŀͻ���������Ϣ
            loop->async_event &= ~CLIENTSINFO;
            live->GetClientInfoAsync();
        }
    }

    /** �ͻ���ȫ���Ͽ�����ʱ�Ͽ�Դ�Ķ�ʱ�� */
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

    /** �ⲿ�߳��½�һ��ֱ���¼�ѭ�� */
    live_event_loop_t* create_live_event_loop(void *usr) {
        SAFE_MALLOC(live_event_loop_t, loop);
        loop->usr = usr;

        //��ʼ��loop
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

        //live workerʵ����loop�߳�
        loop->running = true;
        uv_thread_t tid;
        uv_thread_create(&tid, run_loop_thread, loop);

		return loop;
    }

    /** �ⲿ�̹߳ر��¼�ѭ�� */
    void close_live_event_loop(live_event_loop_t* loop){
        loop->running = false;
        loop->async_event |= CLOSE;
        uv_async_send(&loop->async);
    }

    /** �ⲿ�߳���ӿͻ������� */
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

    /** �ⲿ�߳��Ƴ��ͻ������� */
    void live_event_remove_handle(live_event_loop_t* loop, ILiveHandle* h){
        uv_mutex_lock(&loop->uv_mutex_remove);
		list_push_back(loop->removing_list, h);
        uv_mutex_unlock(&loop->uv_mutex_remove);

        loop->async_event |= REMOVEHANDLE;
        uv_async_send(&loop->async);
    }

    /** �ⲿ�߳����������� */
    void live_event_push_stream(live_event_loop_t* loop, AV_BUFF *buff){
		AV_BUFF tmp = *buff;
		tmp.pData = (char*)malloc(tmp.nLen);
		memcpy(tmp.pData, buff->pData, tmp.nLen);
        simple_ring_insert(loop->ring, &tmp);
        loop->async_event |= RECEIVING;
        uv_async_send(&loop->async);
    }

    /** �ⲿ�߳������ѯ�ͻ���������Ϣ */
    void live_event_get_clients_info(live_event_loop_t* loop) {
        loop->async_event |= CLIENTSINFO;
        uv_async_send(&loop->async);
    }

    /** �ⲿ�߳�����������ʱ����ָ��ʱ�������¿ͻ��������򲻽��� */
    void live_event_end_timer(live_event_loop_t* loop) {
        uv_timer_start(&loop->timer_stop, stop_timer_cb, 5000, 0);
    }
    
    //////////////////////////////////////////////////////////////////////////

    /** ��ѯ�ͻ������ӵ���Ϣ */
    static int _clients_req_num = 0;
    static vector<ClientInfo> _clients_info;
    static CriticalSection _cs_clients;

    /** ֱ��ʵ����ÿͻ�����Ϣ�󣬷��͵�������� */
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
    /** rtp�������� */
    extern string        g_strRtpIP;       //< RTP����IP
    extern list<int>     g_vecRtpPort;     //< RTP���ö˿ڣ�ʹ��ʱ����ȡ����ʹ�ý������·���

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

    static map<string,CLiveWorker*>  m_workerMap;   //live worker ��httpģ�� event loop���̷߳���

    /** CLiveWorker������ɾ��m_pLive�ȽϺ�ʱ��������event loop�����ʹ���̡߳� */
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
            // �Ѿ�����
            return itFind->second;
        }

        return nullptr;
    }

    bool DelLiveWorker(string strCode)
    {
        auto itFind = m_workerMap.find(strCode);
        if (itFind != m_workerMap.end())
        {
            // CLiveWorker������ɾ��m_pLive�ȽϺ�ʱ��������event loop�����ʹ���߳����ٶ���
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
            // ԭʼͨ��
            if(!m_pOrigin)
                m_pOrigin = new CLiveChannel;
            CHECK_POINT_BOOL(m_pOrigin);
            m_pOrigin->AddHandle(h, t);
        } else {
            // ��չͨ��
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
            // ԭʼͨ��
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
        // ԭʼͨ��
        bool bOriginEmpty = true;
        if(m_pOrigin){
            bOriginEmpty = m_pOrigin->RemoveHandle(h);
            if(bOriginEmpty) {
                SAFE_DELETE(m_pOrigin);
            }
        }

        //��չͨ��
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
                // ��ƵԴû�����ݣ���ʱ������Ҫ��ʱ
                Clear2Stop();
            } else {
                // ��ƵԴ��Ȼ���ӣ���ʱN�������ٶ����Ա��ʱ�������������ܿ��ٲ���
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
            //���ȴ�map�����߶���
            DelLiveWorker(m_strCode);
		}
	}

    void CLiveWorker::stop()
    {
        //��ƵԴû�����ݲ���ʱ
        Log::debug("no data recived any more, stopped");
        //״̬�ı�Ϊ��ʱ����ʱǰ��ȫ���Ͽ�������Ҫ��ʱ��ֱ������
        m_bOver = true;

        //֪ͨ����ͨ���Ͽ�ǰ������
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
            //ԭʼͨ������h264����ת����ȥ
            if(m_pOrigin){
                m_pOrigin->ReceiveStream(buff);
            }
            //��չͨ���������������yuv�ٷ��͹�ȥ
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

            //��sdp��������ƵԴip�Ͷ˿�
            m_strPlayInfo = pal->strMark;
            ParseSdp();

            //��������
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
        //��sdp��������ƵԴip�Ͷ˿�
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
