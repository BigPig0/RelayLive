#include "stdafx.h"
#include "RtspSocket.h"
#include "RtspWorker.h"

namespace RtspServer
{
typedef enum _parse_step_
{
    parse_step_method = 0, //未开始,需解析方法 [OPIONS、DESCRIBE、SETUP、PLAY、TEARDOWN]
    parse_step_uri,        //需解析uri
    parse_step_protocol,   //需解析协议[rtsp]
    parse_step_version,    //需解析版本
    parse_step_header_k,   //需解析请求头字段的key
    parse_step_header_v    //需解析请求头字段的value
}parse_step_t;
    
static void on_close(uv_handle_t* peer) {
    CRtspSocket* client = (CRtspSocket*)peer->data;
	client->m_server->m_options.cb(client, RTSP_REASON_CLOSE, client->m_user);
}

static void after_shutdown(uv_shutdown_t* req, int status) {
    uv_close((uv_handle_t*)req->handle, on_close);
    free(req);
}

static void echo_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    *buf = uv_buf_init((char*)calloc(1,1024), 1024);
}

static void after_read(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
    CRtspSocket* client = (CRtspSocket*)handle->data;
    if (nread < 0) {
        if(nread == UV_EOF){
            Log::debug("remote close this socket");
        } else {
            Log::debug("other close %s",  uv_strerror(nread));
        }

        if (buf->base) {
            free(buf->base);
        }

        uv_shutdown_t* req = (uv_shutdown_t*) malloc(sizeof(uv_shutdown_t));
        uv_shutdown(req, handle, after_shutdown);

        return;
    }

    if (nread == 0) {
        /* Everything OK, but nothing read. */
        free(buf->base);
        return;
    }

    client->parse(buf->base, nread);
}

static void request_cb(void *user, rtsp_ruquest_t *req) {
    CRtspSocket* client = (CRtspSocket*)user;
    client->answer(req);
}

static void after_write(uv_write_t* req, int status) {
    if (status < 0)
    {
        Log::error("after_write fail:%s", uv_strerror(status));
    }
}

static void on_async(uv_async_t* handle){
    CRtspSocket *c = (CRtspSocket*)handle->data;
    uv_mutex_lock(&c->m_asyncMutex);
    while(!c->m_asyncList.empty()){
        rtsp_event e = c->m_asyncList.front();
        c->m_server->m_options.cb(c, (RTSP_REASON)e.resaon, c->m_user);
        c->m_asyncList.pop_front();
    }
    uv_mutex_unlock(&c->m_asyncMutex);
}

CRtspSocket::CRtspSocket()
    : m_ploop(nullptr)
    , m_server(nullptr)
    , m_Request(nullptr)
    , m_Response(nullptr)
    , m_pSession(nullptr)
{
    m_pRtspParse = create_rtsp(this, request_cb);
    m_pSessMgr = CRtspSessionMgr::GetInstance();
}

CRtspSocket::~CRtspSocket()
{
    SAFE_FREE(m_user);
	uv_mutex_destroy(&m_asyncMutex);
	m_server->GiveBackRtpPort(m_nLocalPort);
    destory_rtsp(m_pRtspParse);
}

int CRtspSocket::Init(uv_loop_t* uv) {
    m_ploop = uv;
    int r = uv_tcp_init(m_ploop, &m_rtsp);
    if(r < 0) {
        Log::error("client init rtsp error %s",  uv_strerror(r));
        return r;
    }
    /*
    r = uv_udp_init(m_ploop, &m_rtp);
    if(r < 0) {
        Log::error("client init rtp error %s", uv_strerror(r));
        return r;
    }
	m_nLocalPort = m_server->GetRtpPort();
	struct sockaddr_in addr;
    r = uv_ip4_addr(m_server->m_options.ip.c_str(), m_nLocalPort, &addr);
    if(r < 0) {
        Log::error("make address err: %s",  uv_strerror(r));
        return -1;
    }
	r = uv_udp_bind(&m_rtp, (struct sockaddr*)&addr, 0);
    if(r < 0) {
        Log::error("client init bind rtp error %s", uv_strerror(r));
        return r;
    }
    r = uv_udp_init(m_ploop, &m_rtcp);
    if(r < 0) {
        Log::error("client init rtcp error %s", uv_strerror(r));
        return r;
    }
    */
    r = uv_async_init(m_ploop, &m_async, on_async);
    if(r < 0) {
        Log::error("client init rtcp error %s", uv_strerror(r));
        return r;
    }
    r = uv_mutex_init(&m_asyncMutex);
    if(r < 0) {
        Log::error("client init mutex error %s", uv_strerror(r));
        return r;
    }
    m_user = malloc(m_server->m_options.user_len);
    if(NULL == m_user){
        Log::error("client init malloc error");
        return 1;
    }
	memset(m_user, 0, m_server->m_options.user_len);

    m_rtsp.data  = (void*)this;
    //m_rtp.data   = (void*)this;
    //m_rtcp.data  = (void*)this;
    m_async.data = (void*)this;
    return 0;
}

int CRtspSocket::Recv() {
    int r = uv_read_start((uv_stream_t*)&m_rtsp, echo_alloc, after_read);
    if (r < 0)
    {
        Log::error("read start error %s",  uv_strerror(r));
        return r;
    }
    return 0;
}

void CRtspSocket::parse(char* buff, int len) {
    rtsp_handle_request(m_pRtspParse, buff, len);
}

int CRtspSocket::answer(rtsp_ruquest_t *req)
{
    rtsp_response res;
    m_Request = req;
    m_Response = &res;

    do{
        if(req->method == rtsp_method::RTSP_ERROR) {
            res.code = Code_400_BadRequest;
            break;
        }
       
        res.CSeq = req->CSeq;   //应答序号和请求序号一致

        if(req->method == rtsp_method::RTSP_OPTIONS) {
            int ret = m_server->m_options.cb(this, RTSP_REASON_OPTIONS, m_user);
            if(ret)
                break;
        } else if(req->method == rtsp_method::RTSP_DESCRIBE) {
            //DESCRIBE的时候就建立会话
            m_pSession = m_pSessMgr->NewSession();
            int ret = m_server->m_options.cb(this, RTSP_REASON_DESCRIBE, m_user);
            if(ret)
                break;
        } else if(req->method == rtsp_method::RTSP_SETUP) {
			res.headers.insert(make_pair("Session",m_pSession->m_strSessID));
            int ret = m_server->m_options.cb(this, RTSP_REASON_SETUP, m_user);
            if(ret)
                break;

        } else if(req->method == rtsp_method::RTSP_PLAY) {
            int ret = m_server->m_options.cb(this, RTSP_REASON_PLAY, m_user);
            if(ret)
                break;
        } else if(req->method == rtsp_method::RTSP_PAUSE) {
            int ret = m_server->m_options.cb(this, RTSP_REASON_PAUSE, m_user);
            if(ret)
                break;
        } else if(req->method == rtsp_method::RTSP_TEARDOWN) {
            int ret = m_server->m_options.cb(this, RTSP_REASON_TEARDOWN, m_user);
            if(ret)
                break;
        } else {
            res.code = Code_551_OptionNotSupported;
        }
    }while(0);

    //生成tcp应答报文
    char time_buff[50]={0};
    time_t time_now = time(NULL);
    ctime_s(time_buff, 49, &time_now);
    string strTime = time_buff;
    strTime = strTime.substr(0,strTime.size()-1);
    strTime += " GMT";
    stringstream ss;
    ss << "RTSP/1.0 " << response_status[res.code] << "\r\n"
        << "CSeq: " << res.CSeq << "\r\n"
        << "Date: " << strTime << "\r\n";
    for (auto& h:res.headers)
    {
        ss << h.first << ": " << h.second << "\r\n";
    }
    if (!res.body.empty())
    {
        ss << "Content-Length: " << res.body.size()+2 << "\r\n\r\n";
        ss << res.body;
    }
    ss << "\r\n";
    string strResponse = ss.str();

    //发送应答
    uv_write_t *wr = (uv_write_t*)malloc(sizeof(uv_write_t));
    wr->data = this;
    uv_buf_t buff = uv_buf_init((char*)strResponse.c_str(), strResponse.size());
    int ret = uv_write(wr, (uv_stream_t*)&m_rtsp,&buff, 1, after_write);
    if (ret < 0)
    {
        Log::error("uv_write fail:%s",  uv_strerror(ret));
    }
    return 0;
}

void CRtspSocket::SetRemotePort(int rtp, int rtcp)
{
    m_nRtpPort = rtp;
    m_nRtcpPort = rtcp;
    int ret = uv_ip4_addr(m_strRtpIP.c_str(), rtp, &m_addrRtp);
    if(ret < 0) {
        Log::error("make address err: %s",  uv_strerror(ret));
        return ;
    }
}

//////////////////////////////////////////////////////////////////////////


static void on_connection(uv_stream_t* server, int status) {
    if (status != 0) {
        Log::error("Connect error %s",  uv_strerror(status));
        return;
    }

    CRtspServer* rtsp = (CRtspServer*)server->data;

    CRtspSocket* client = new CRtspSocket;
    client->m_server = rtsp;
    int r = client->Init(rtsp->m_ploop);
    if(r < 0) {
        Log::error("client init error %s",  uv_strerror(r));
        return;
    }

    r = uv_accept(server, (uv_stream_t*)&client->m_rtsp);
    if(r < 0) {
        Log::error("accept error %s",  uv_strerror(status));
        return;
    }

    // 客户端IP
    struct sockaddr_in addr;
    char ipv4addr[64];
    int namelen = sizeof(addr);
    uv_tcp_getpeername(&client->m_rtsp, (struct sockaddr*)&addr, &namelen);
    uv_ip4_name(&addr, ipv4addr, 64);
    client->m_strRtpIP = ipv4addr;

    // 本地IP
    uv_tcp_getsockname(&client->m_rtsp, (struct sockaddr*)&addr, &namelen);
    uv_ip4_name(&addr, ipv4addr, 64);
    client->m_strLocalIP = ipv4addr;

    r = client->Recv();
    if (r < 0)
    {
        Log::error("read start error %s",  uv_strerror(r));
        return;
    }
}

CRtspServer::CRtspServer(rtsp_options options)
    : m_options(options)
{
    for (int i=0; i<options.rtp_port_num; ++i) {
        m_vecRtpPort.push_back(options.rtp_port+i*2);
    }
}

CRtspServer::~CRtspServer(void)
{
}

int CRtspServer::Init(uv_loop_t* uv)
{
    m_ploop = uv;
    uv_tcp_init(m_ploop, &m_tcp);

    struct sockaddr_in addr;
    int ret = uv_ip4_addr(m_options.ip.c_str(), m_options.port, &addr);
    if(ret < 0) {
        Log::error("make address err: %s",  uv_strerror(ret));
        return -1;
    }

    ret = uv_tcp_bind(&m_tcp, (struct sockaddr*)&addr, 0);
    if(ret < 0) {
        Log::error("tcp bind err: %s",  uv_strerror(ret));
        return -1;
    }

    m_tcp.data = (void*)this;
    ret = uv_listen((uv_stream_t*)&m_tcp, SOMAXCONN, on_connection);
    if (ret < 0)
    {
        Log::error("uv listen err:%s", uv_strerror(ret));
        return -1;
    }

    Log::debug("rtsp server[%s:%d] init success", m_options.ip.c_str(), m_options.port);
    return 0;
}

int  CRtspServer::GetRtpPort()
{
    MutexLock lock(&m_csRTP);

    int nRet = -1;
    auto it = m_vecRtpPort.begin();
    if (it != m_vecRtpPort.end()) {
        nRet = *it;
        m_vecRtpPort.erase(it);
    }

    return nRet;
}

void  CRtspServer::GiveBackRtpPort(int nPort)
{
    MutexLock lock(&m_csRTP);
    m_vecRtpPort.push_back(nPort);
}


//////////////////////////////////////////////////////////////////////////

int rtp_callback_on_writable(CRtspSocket *client){
    uv_mutex_lock(&client->m_asyncMutex);
    rtsp_event e = {RTSP_REASON_RTP_WRITE};
    client->m_asyncList.push_back(e);
    uv_mutex_unlock(&client->m_asyncMutex);
    uv_async_send(&client->m_async);
    return 0;
}

static void on_udp_send(uv_udp_send_t* req, int status){
    SAFE_FREE(req->data);
    SAFE_FREE(req);
}

int rtp_write(CRtspSocket *client, char* buff, int len){
    SAFE_MALLOC(uv_udp_send_t, req);
	char* cache = (char *)malloc(len);
    memcpy(cache, buff, len);
    uv_buf_t buf = uv_buf_init(cache, len);
    req->data = cache;
    //int ret = uv_udp_send(req, &client->m_rtp, &buf, 1, (const sockaddr*)&client->m_addrRtp, on_udp_send);
    //if(ret != 0){
    //    Log::debug("udp send failed %d: %s", ret, uv_err_name((ret)));
    //    return ret;
    //}
    return 0;
}

}