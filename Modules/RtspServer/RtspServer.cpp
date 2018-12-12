#include "stdafx.h"
#include "RtspServer.h"
//#include "DeviceMgr.h"
//#include "SipInstance.h"

typedef enum _parse_step_
{
    parse_step_method = 0, //未开始,需解析方法 [OPIONS、DESCRIBE、SETUP、PLAY、TEARDOWN]
    parse_step_uri,        //需解析uri
    parse_step_protocol,   //需解析协议[rtsp]
    parse_step_version,    //需解析版本
    parse_step_header_k,   //需解析请求头字段的key
    parse_step_header_v    //需解析请求头字段的value
}parse_step_t;

char* response_status[] = {
    "100 Continue(all 100 range)",
    "110 Connect Timeout",
    "200 OK",
    "201 Created",
    "250 Low on Storage Space",
    "300 Multiple Choices",
    "301 Moved Permanently",
    "302 Moved Temporarily",
    "303 See Other",
    "304 Not Modified",
    "305 Use Proxy",
    "350 Going Away",
    "351 Load Balancing",
    "400 Bad Request",
    "401 Unauthorized",
    "402 Payment Required",
    "403 Forbidden",
    "404 Not Found",
    "405 Method Not Allowed",
    "406 Not Acceptable",
    "407 Proxy Authentication Required",
    "408 Request Time-out",
    "410 Gone",
    "411 Length Required",
    "412 Precondition Failed",
    "413 Request Entity Too Large",
    "414 Request-URI Too Large",
    "415 Unsupported Media Type",
    "451 Parameter Not Understood",
    "452 reserved",
    "453 Not Enough Bandwidth",
    "454 Session Not Found",
    "455 Method Not Valid in This State",
    "456 Header Field Not Valid for Resource",
    "457 Invalid Range",
    "458 Parameter Is Read-Only",
    "459 Aggregate operation not allowed",
    "460 Only aggregate operation allowed",
    "461 Unsupported transport",
    "462 Destination unreachable",
    "500 Internal Server Error",
    "501 Not Implemented",
    "502 Bad Gateway",
    "503 Service Unavailable",
    "504 Gateway Time-out",
    "505 RTSP Version not supported",
    "551 Option not supported"
};

//////////////////////////////////////////////////////////////////////////////////////

static void on_close(uv_handle_t* peer) {
    free(peer);
}

static void after_shutdown(uv_shutdown_t* req, int status) {
    uv_close((uv_handle_t*)req->handle, on_close);
    free(req);
}

static void echo_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    *buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
}

static void after_read(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
    CClient* client = (CClient*)handle->data;
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

    rtsp_ruquest req = client->parse(buf->base, nread);

    client->answer(req);
}

static void after_write(uv_write_t* req, int status) {
    if (status < 0)
    {
        Log::error("after_write fail:%s", uv_strerror(status));
    }
}

CClient::CClient()
{
}

CClient::~CClient()
{
}

int CClient::Init(uv_loop_t* uv) {
    m_ploop = uv;
    int r = uv_tcp_init(m_ploop, &m_tcp);
    if(r < 0) {
        Log::error("client init error %s",  uv_strerror(r));
        return r;
    }

    m_tcp.data = (void*)this;
    return 0;
}

int CClient::Recv() {
    int r = uv_read_start((uv_stream_t*)&m_tcp, echo_alloc, after_read);
    if (r < 0)
    {
        Log::error("read start error %s",  uv_strerror(r));
        return r;
    }
    return 0;
}

rtsp_ruquest CClient::parse(char* buff, int len) {
    rtsp_ruquest ret;
    ret.method = RTSP_ERROR;
    ret.parse_status = true;
    parse_step_t step =  parse_step_method;
    string tmp,tmp2;
    for (int i = 0; i < len; ++i) {
        if (parse_step_method == step)
        {
            if(buff[i] == ' ') {
                Log::debug("request method is %s", tmp.c_str());
                if(_stricmp(tmp.c_str(), "OPTIONS") == 0){
                    ret.method = RTSP_OPTIONS;
                } else if(_stricmp(tmp.c_str(), "DESCRIBE") == 0){
                    ret.method = RTSP_DESCRIBE;
                }else if(_stricmp(tmp.c_str(), "SETUP") == 0){
                    ret.method = RTSP_SETUP;
                }else if(_stricmp(tmp.c_str(), "PLAY") == 0){
                    ret.method = RTSP_PLAY;
                }else if(_stricmp(tmp.c_str(), "PAUSE") == 0){
                    ret.method = RTSP_PAUSE;
                }else if(_stricmp(tmp.c_str(), "TEARDOWN") == 0){
                    ret.method = RTSP_TEARDOWN;
                } else {
                    Log::error("error method : %s", tmp.c_str());
                    ret.parse_status = false;
                    ret.code = Code_551_OptionNotSupported;
                    break;
                }
                tmp.clear();
                step = parse_step_uri;
            } else {
                tmp.push_back(buff[i]);
            }
        } else if (parse_step_uri == step) {
            if (buff[i] == ' ') {
                ret.uri = tmp;
                tmp.clear();
                step = parse_step_protocol;
            } else {
                tmp.push_back(buff[i]);
            }
        } else if (parse_step_protocol == step) {
            if (buff[i] == '/') {
                if(_stricmp(tmp.c_str(), "RTSP") != 0) {
                    Log::error("error protocol : %s", tmp.c_str());
                    ret.parse_status = false;
                    ret.code = Code_400_BadRequest;
                    break;
                }
                tmp.clear();
                step = parse_step_version;
            } else {
                tmp.push_back(buff[i]);
            }
        } else if (parse_step_version == step) {
            if (buff[i] == '\r' && buff[i+1] == '\n') {
                if(_stricmp(tmp.c_str(), "1.0") != 0) {
                    Log::error("error version : %s", tmp.c_str());
                    ret.parse_status = false;
                    ret.code = Code_505_RtspVersionNotSupported;
                    break;
                }
                tmp.clear();
                i++;
                step = parse_step_header_k;
            } else {
                tmp.push_back(buff[i]);
            }
        } else if (parse_step_header_k == step) {
            if (buff[i] == ':') {
                step = parse_step_header_v;
            } else {
                tmp.push_back(buff[i]);
            }
        } else if (parse_step_header_v == step) {
            if (buff[i] == '\r' && buff[i+1] == '\n') {
                ret.headers.insert(make_pair(tmp,tmp2));
                tmp.clear();
                tmp2.clear();
                i++;
                step = parse_step_header_k;
            } else {
                if(!tmp2.empty() || buff[i] != ' ')
                    tmp2.push_back(buff[i]);
            }
        }
    }

    return ret;
}

int CClient::answer(rtsp_ruquest req)
{
    rtsp_response res;
    do{
        auto seq_it = req.headers.find("CSeq");
        if (seq_it == req.headers.end()) {
            res.code = Code_400_BadRequest;
            break;
        }
        try {
            req.CSeq = stoi(seq_it->second);
        } catch (...) {
            res.code = Code_400_BadRequest;
            break;
        }
        if (!req.parse_status) {
            res.code = req.code;
            break;
        }

        if(req.method == rtsp_method::RTSP_OPTIONS) {
            res = make_option_answer(req);
        } else if(req.method == rtsp_method::RTSP_DESCRIBE) {
            res = make_describe_answer(req);
        } else if(req.method == rtsp_method::RTSP_SETUP) {
            res = make_setup_answer(req);
        } else if(req.method == rtsp_method::RTSP_PLAY) {
            res = make_play_answer(req);
        } else if(req.method == rtsp_method::RTSP_PAUSE) {
            res = make_pause_answer(req);
        } else if(req.method == rtsp_method::RTSP_TEARDOWN) {
            res = make_teardown_answer(req);
        } else {
            res.code = Code_551_OptionNotSupported;
        }
        res.CSeq = req.CSeq;
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
    int ret = uv_write(wr, (uv_stream_t*)&m_tcp,&buff, 1, after_write);
    if (ret < 0)
    {
        Log::error("uv_write fail:%s",  uv_strerror(ret));
    }
    return 0;
}

rtsp_response CClient::make_option_answer(rtsp_ruquest req)
{
    Log::debug("make_option_answer");
    rtsp_response res;
    res.code = Code_200_OK;
    res.headers.insert(make_pair("Public","OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE"));
    return res;
}

rtsp_response CClient::make_describe_answer(rtsp_ruquest req)
{
    Log::debug("make_describe_answer");
    rtsp_response res;
    res.code = Code_200_OK;
    res.body = "v=0\r\n"
        "o=36030100062000000000 0 0 IN IP4 172.31.7.161\r\n"
        "s=Play\r\n"
        "u=36030100061320000026:3\r\n"
        "c=IN IP4 127.0.0.1\r\n"
        "t=0 0\r\n"
        "a=sdplang:en\r\n"
        "a=range:npt=0-\r\n"
        "a=control:*\r\n"
        "m=video 18000 RTP/AVP 96\r\n"
        "a=rtpmap:96 MP2P/90000\r\n"
        "a=recvonly\r\n"
        "y=0301000168\r\n";
    res.headers.insert(make_pair("Content-Type","application/sdp"));
    res.headers.insert(make_pair("Content-Length",StringHandle::toStr<size_t>(res.body.size())));
    return res;
}

rtsp_response CClient::make_setup_answer(rtsp_ruquest req)
{
    Log::debug("make_setup_answer");
    rtsp_response res;
    auto tra_it = req.headers.find("Transport");
    if(tra_it == req.headers.end()) {
        res.code = Code_400_BadRequest;
        return res;
    } else {
        bool port = false;
        string trans = tra_it->second;
        res.headers.insert(make_pair("Transport", trans));
        vector<string> vec_trans = StringHandle::StringSplit(trans, ';');
        for (auto& it:vec_trans){
            if (it.size() > 12) {
                if(0 == it.compare(0,12,"client_port=",0,12)) {
                    size_t pos = it.find('-',12);
                    if (pos == string::npos) {
                        res.code = Code_461_UnsupportedTransport;
                        return res;
                    }
                    string rtpport = it.substr(12,pos-12);
                    string rtcpport = it.substr(pos+1, it.size()-pos-1);
                    m_strRtpPort = stoi(rtpport);
                    try{
                        req.rtp_port = stoi(rtpport);
                        req.rtcp_port= stoi(rtcpport);
                        port = true;
                        break;
                    } catch (...) {
                        res.code = Code_461_UnsupportedTransport;
                        return res;
                    }
                }
            }
        }
    }
    string session = StringHandle::toStr<uint64_t>(CRtspServer::m_nSession++);
    res.headers.insert(make_pair("Session",session));
    res.code = Code_200_OK;
    return res;
}

rtsp_response CClient::make_play_answer(rtsp_ruquest req)
{
    Log::debug("make_play_answer");
    rtsp_response res;
    auto sess_it = req.headers.find("Session");
    if (sess_it == req.headers.end()) {
        res.code = Code_400_BadRequest;
        return res;
    }

    string session = sess_it->second;

    m_strDevCode = "36030100061320000026";
    string rtpIP="172.31.7.88";
    /*
    if(!SipInstance::rtsp_play(m_strDevCode, rtpIP, m_strRtpPort)) {
        res.code = Code_503_ServiceUnavailable;
        return res;
    }*/
    res.headers.insert(make_pair("Session",session));
    res.code = Code_200_OK;
    return res;
}

rtsp_response CClient::make_pause_answer(rtsp_ruquest req)
{
    Log::debug("make_pause_answer");
    rtsp_response res;
    return res;
}

rtsp_response CClient::make_teardown_answer(rtsp_ruquest req)
{
    Log::debug("make_teardown_answer");
    rtsp_response res;
    auto sess_it = req.headers.find("Session");
    if (sess_it == req.headers.end()) {
        res.code = Code_400_BadRequest;
        return res;
    }
    string session = sess_it->second;

    /*
    if(!SipInstance::StopPlay(m_strDevCode)) {
        res.code = Code_503_ServiceUnavailable;
        return res;
    }*/
    res.headers.insert(make_pair("Session",session));
    res.code = Code_200_OK;
    return res;
}

//////////////////////////////////////////////////////////////////////////

volatile uint64_t CRtspServer::m_nSession = 11111111;

static void on_connection(uv_stream_t* server, int status) {
    if (status != 0) {
        Log::error("Connect error %s",  uv_strerror(status));
        return;
    }

    CRtspServer* rtsp = (CRtspServer*)server->data;

    CClient* client = new CClient;
    client->m_server = rtsp;
    int r = client->Init(rtsp->m_ploop);
    if(r < 0) {
        Log::error("client init error %s",  uv_strerror(r));
        return;
    }

    r = uv_accept(server, (uv_stream_t*)&client->m_tcp);
    if(r < 0) {
        Log::error("accept error %s",  uv_strerror(status));
        return;
    }

    r = client->Recv();
    if (r < 0)
    {
        Log::error("read start error %s",  uv_strerror(r));
        return;
    }
}

CRtspServer::CRtspServer(void)
{
}

CRtspServer::~CRtspServer(void)
{
}

int CRtspServer::Init(uv_loop_t* uv)
{
    m_ploop = uv;
    uv_tcp_init(m_ploop, &m_tcp);

    struct sockaddr_in addr;
    int ret = uv_ip4_addr("0.0.0.0", 524, &addr);
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
    return 0;
}