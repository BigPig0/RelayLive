#include "stdafx.h"
#include "rtsp_client.h"
#include "md5.h"
#include "base64.h"

extern uv_loop_t* _uv_loop_;
extern void sure_move(string ssid);

static uint32_t _seq_ = 1;

/** socket关闭回调 */
static void close_cb(uv_handle_t* handle) {
    CRtspClient* rtsp = (CRtspClient*)handle->data;
	Log::debug("close %s", rtsp->_uri.c_str());
    rtsp->_conn_state = RTSP_CONNECT_CLOSE;
    sure_move(rtsp->_option.ssid);
    delete rtsp;
}

static void shutdown_cb(uv_shutdown_t* req, int status) {
    CRtspClient* rtsp = (CRtspClient*)req->data;
	Log::debug("shutdown %s,%d", rtsp->_uri.c_str(), status);
    rtsp->_conn_state = RTSP_CONNECT_SHUTDOWN;
    uv_close((uv_handle_t*)&rtsp->_tcp, close_cb);
}

/** 接收数据申请空间回调 */
static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
    CRtspClient* rtsp = (CRtspClient*)handle->data;
    memset(rtsp->_recv_buff, 0, SOCKET_RECV_BUFF_LEN);
    *buf = uv_buf_init(rtsp->_recv_buff, SOCKET_RECV_BUFF_LEN);
}

/** 接收数据回调 */
static void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    CRtspClient* rtsp = (CRtspClient*)stream->data;
    if (nread < 0) {
        fprintf(stderr, "read_cb error %s-%s\r\n", uv_err_name(nread), uv_strerror(nread));
        if(nread == UV_EOF) {
            uv_close((uv_handle_t*)&rtsp->_tcp, close_cb);
        }
        uv_shutdown(&rtsp->_shutdown, (uv_stream_t*)&rtsp->_tcp, shutdown_cb);
        rtsp->_step_state = RTSP_STEP_FAILED;
		rtsp->_play_cb(rtsp->_option.ssid, RTSP_ERR_TCP_RECV_FAILED);
        return;
    }
    rtsp->_recv_len = nread;
    Log::debug(rtsp->_recv_buff);

    //读取到数据
    if(rtsp->_step == RTSP_STEP_OPTION)
        rtsp->parse_options();
    else if(rtsp->_step == RTSP_STEP_DESCRIBE)
        rtsp->parse_describe();
    else if(rtsp->_step == RTSP_STEP_SETUP)
        rtsp->parse_setup();
    else if(rtsp->_step == RTSP_STEP_PLAY)
        rtsp->parse_play();
    else if(rtsp->_step == RTSP_STEP_TEARDOWN)
        rtsp->parse_teardown();
}

/** 发送数据回调 */
static void write_cb(uv_write_t* req, int status) {
    CRtspClient* rtsp = (CRtspClient*)req->data;
    if (status < 0)
    {
        Log::error("tcp send failed:%s-%s", uv_err_name(status), uv_strerror(status)); 
        rtsp->_step_state = RTSP_STEP_FAILED;
        uv_shutdown(&rtsp->_shutdown, (uv_stream_t*)&rtsp->_tcp, shutdown_cb);
		rtsp->_play_cb(rtsp->_option.ssid, RTSP_ERR_TCP_SEND_FAILED);
    }
	Log::debug("send ok: %d", rtsp->_step);
}

void on_connect(uv_connect_t* req, int status) {
    CRtspClient* rtsp = (CRtspClient*)req->data;
	if(status < 0) {
		Log::error("tcp connect failed:%s-%s\r\n", uv_err_name(status), uv_strerror(status)); 
		rtsp->_play_cb(rtsp->_option.ssid, RTSP_ERR_CONNECT_FAILED);
        return;
	}

    int ret = uv_read_start(req->handle, alloc_cb, read_cb);//客户端开始接收服务器的数据
    if (ret) {
        Log::error("tcp receive failed:%s-%s\r\n", uv_err_name(ret), uv_strerror(ret)); 
		rtsp->_play_cb(rtsp->_option.ssid, RTSP_ERR_CONNECT_FAILED);
        return;
    }

    rtsp->_conn_state = RTSP_CONNECT_CONNECTED;
    rtsp->send_options();
}

CRtspClient::CRtspClient(RTSP_REQUEST option)
    : _option(option)
    , _step(RTSP_STEP_NONE)
    , _conn_state(RTSP_CONNECT_INIT)
    , _need_auth(RTSP_AUTHORIZATION_NONE)
    , _send_buff(NULL)
    , _recv_buff(NULL)
{
    uv_tcp_init(_uv_loop_, &_tcp);
	int nOverTime = 3000; 
	setsockopt(_tcp.socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&nOverTime, sizeof(nOverTime));
	setsockopt(_tcp.socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&nOverTime, sizeof(nOverTime));
}

CRtspClient::~CRtspClient()
{
  //  stop();
  //  while (_conn_state != RTSP_CONNECT_CLOSE)
  //  {
		//sleep(100);
  //  }
    if(_send_buff) free(_send_buff);
    if(_recv_buff) free(_recv_buff);
}

int CRtspClient::play(play_cb cb)
{
    _play_cb       = cb;
    _conn.data     = this;
    _tcp.data      = this;
    _write.data    = this;
    _shutdown.data = this;
    _send_buff     = (char*)malloc(SOCKET_RECV_BUFF_LEN);
    _recv_buff     = (char*)malloc(SOCKET_RECV_BUFF_LEN);

    struct sockaddr_in addr;
    int ret = uv_ip4_addr(_option.ip.c_str(), _option.port, &addr);
    if(ret < 0) {
        Log::error("make address err: %s",  uv_strerror(ret));
        return RTSP_ERR_IP4_ADDR_FAILED;
    }

    make_uri();
    Log::debug(_uri.c_str());

    ret = uv_tcp_connect(&_conn, &_tcp, (struct sockaddr*)&addr, on_connect);
    if(ret < 0) {
        Log::error("tcp connect failed: %s", uv_strerror(ret));
        return RTSP_ERR_CONNECT_FAILED;
    }
    return 0;
}

int CRtspClient::stop()
{
    if(_step == RTSP_STEP_PLAY && _step_state == RTSP_STEP_SUCESS)
        send_teardown();
    else if(_conn_state == RTSP_CONNECT_CONNECTED)
        uv_shutdown(&_shutdown, (uv_stream_t*)&_tcp, shutdown_cb);

    return 0;
}

int CRtspClient::send_options()
{
    _step = RTSP_STEP_OPTION;
    _step_state = RTSP_STEP_BEGIN;

    stringstream ss;
    ss << "OPTIONS " << _uri << " RTSP/1.0\r\n"
        << "CSeq: " << _seq_++ << "\r\n"
        << "User-Agent: relay live rtsp\r\n"
        << "\r\n";
    memset(_send_buff, 0 , SOCKET_RECV_BUFF_LEN);
    strncpy(_send_buff, ss.str().c_str(), SOCKET_RECV_BUFF_LEN);
	Log::warning(_send_buff);

    uv_buf_t buf = uv_buf_init(_send_buff, ss.str().size());
    int ret = uv_write(&_write, (uv_stream_t*)&_tcp, &buf, 1, write_cb);
    if (ret < 0)
    {
        Log::error("tcp send failed:%s-%s", uv_err_name(ret), uv_strerror(ret)); 
        _step_state = RTSP_STEP_FAILED;
		_play_cb(_option.ssid, RTSP_ERR_TCP_SEND_FAILED);
        return ret;
    }
    return 0;
}

int CRtspClient::send_describe()
{
    _step = RTSP_STEP_DESCRIBE;
    _step_state = RTSP_STEP_BEGIN;

    stringstream ss;
    ss << "DESCRIBE " << _uri << " RTSP/1.0\r\n"
        << "CSeq: " << _seq_++ << "\r\n";
    if(_need_auth == RTSP_AUTHORIZATION_DIGEST) {
        MD5 md5;
        string auth = "DESCRIBE:" + _uri;
        md5.ComputMd5(auth.c_str(), auth.size());
        string auth_md5 = md5.GetMd5();
        MD5 md5_2;
        string auth_2 = _auth_md5 + ":" + _nonce + ":" + auth_md5;
        md5_2.ComputMd5(auth_2.c_str(), auth_2.size());
        string response = md5_2.GetMd5();

        ss << "Authorization: Digest username=\"" << _option.user_name
            << "\", realm=\"" << _realm << "\", nonce=\"" << _nonce
            << "\", uri=\"" << _uri << "\", response=\"" << response << "\"\r\n";
    } else if(_need_auth == RTSP_AUTHORIZATION_BASIC) {
        ss << "Authorization: Basic " << _auth_md5 << "\r\n";
    }
    ss << "User-Agent: relay live rtsp\r\n"
        << "Accept: application/sdp\r\n"
        << "\r\n";
    memset(_send_buff, 0 , SOCKET_RECV_BUFF_LEN);
    strncpy(_send_buff, ss.str().c_str(), SOCKET_RECV_BUFF_LEN);
	Log::warning(_send_buff);

    uv_buf_t buf = uv_buf_init(_send_buff, ss.str().size());
    int ret = uv_write(&_write, (uv_stream_t*)&_tcp, &buf, 1, write_cb);
    if (ret < 0)
    {
        Log::error("tcp send failed:%s-%s", uv_err_name(ret), uv_strerror(ret)); 
		_play_cb(_option.ssid, RTSP_ERR_TCP_SEND_FAILED);
        return ret;
    }
    return 0;
}

int CRtspClient::send_setup()
{
    _step = RTSP_STEP_SETUP;
    _step_state = RTSP_STEP_BEGIN;

    stringstream ss;
    ss << "SETUP " << _uri << "/trackID=1 RTSP/1.0\r\n"
        << "CSeq: " << _seq_++ << "\r\n";
    if(_need_auth == RTSP_AUTHORIZATION_DIGEST) {
        MD5 md5;
        string auth = "SETUP:" + _uri + "/trackID=1";
        md5.ComputMd5(auth.c_str(), auth.size());
        string auth_md5 = md5.GetMd5();
        MD5 md5_2;
        string auth_2 = _auth_md5 + ":" + _nonce + ":" + auth_md5;
        md5_2.ComputMd5(auth_2.c_str(), auth_2.size());
        string response = md5_2.GetMd5();

        ss << "Authorization: Digest username=\"" << _option.user_name
            << "\", realm=\"" << _realm << "\", nonce=\"" << _nonce
            << "\", uri=\"" << _uri << "\", response=\"" << response << "\"\r\n";
    }
    ss << "User-Agent: relay live rtsp\r\n"
		<< "Transport: RTP/AVP;unicast;client_port=" << _option.rtp_port
        << "-" << _option.rtp_port+1 << "\r\n"
        << "\r\n";
    memset(_send_buff, 0 , SOCKET_RECV_BUFF_LEN);
    strncpy(_send_buff, ss.str().c_str(), SOCKET_RECV_BUFF_LEN);
	Log::warning(_send_buff);

    uv_buf_t buf = uv_buf_init(_send_buff, ss.str().size());
    int ret = uv_write(&_write, (uv_stream_t*)&_tcp, &buf, 1, write_cb);
    if (ret < 0)
    {
        Log::error("tcp send failed:%s-%s", uv_err_name(ret), uv_strerror(ret)); 
		_play_cb(_option.ssid, RTSP_ERR_TCP_SEND_FAILED);
        return ret;
    }
    return 0;
}

int CRtspClient::send_play()
{
    _step = RTSP_STEP_PLAY;
    _step_state = RTSP_STEP_BEGIN;

    stringstream ss;
    ss << "PLAY " << _uri << " RTSP/1.0\r\n"
        << "CSeq: " << _seq_++ << "\r\n";
    if(_need_auth == RTSP_AUTHORIZATION_DIGEST) {
        MD5 md5;
        string auth = "PLAY:" + _uri;
        md5.ComputMd5(auth.c_str(), auth.size());
        string auth_md5 = md5.GetMd5();
        MD5 md5_2;
        string auth_2 = _auth_md5 + ":" + _nonce + ":" + auth_md5;
        md5_2.ComputMd5(auth_2.c_str(), auth_2.size());
        string response = md5_2.GetMd5();

        ss << "Authorization: Digest username=\"" << _option.user_name
            << "\", realm=\"" << _realm << "\", nonce=\"" << _nonce
            << "\", uri=\"" << _uri << "\", response=\"" << response << "\"\r\n";
    }
    ss << "User-Agent: relay live rtsp\r\n"
        << "Session: " << _session << "\r\n"
        << "Range: npt=0.000-\r\n"
        << "\r\n";
    memset(_send_buff, 0 , SOCKET_RECV_BUFF_LEN);
    strncpy(_send_buff, ss.str().c_str(), SOCKET_RECV_BUFF_LEN);
	Log::warning(_send_buff);

    uv_buf_t buf = uv_buf_init(_send_buff, ss.str().size());
    int ret = uv_write(&_write, (uv_stream_t*)&_tcp, &buf, 1, write_cb);
    if (ret < 0)
    {
        Log::error("tcp send failed:%s-%s", uv_err_name(ret), uv_strerror(ret)); 
		_play_cb(_option.ssid, RTSP_ERR_TCP_SEND_FAILED);
        return ret;
    }
    return 0;
}

int CRtspClient::send_teardown()
{
    _step = RTSP_STEP_TEARDOWN;
    _step_state = RTSP_STEP_BEGIN;

    stringstream ss;
    ss << "TEARDOWN " << _uri << " RTSP/1.0\r\n"
        << "CSeq: " << _seq_++ << "\r\n";
    if(_need_auth == RTSP_AUTHORIZATION_DIGEST) {
        MD5 md5;
        string auth = "TEARDOWN:" + _uri;
        md5.ComputMd5(auth.c_str(), auth.size());
        string auth_md5 = md5.GetMd5();
        MD5 md5_2;
        string auth_2 = _auth_md5 + ":" + _nonce + ":" + auth_md5;
        md5_2.ComputMd5(auth_2.c_str(), auth_2.size());
        string response = md5_2.GetMd5();

        ss << "Authorization: Digest username=\"" << _option.user_name
            << "\", realm=\"" << _realm << "\", nonce=\"" << _nonce
            << "\", uri=\"" << _uri << "\", response=\"" << response << "\"\r\n";
    }
    ss << "User-Agent: relay live rtsp\r\n"
        << "Session: " << _session << "\r\n"
        << "\r\n";
    memset(_send_buff, 0 , SOCKET_RECV_BUFF_LEN);
    strncpy(_send_buff, ss.str().c_str(), SOCKET_RECV_BUFF_LEN);
	Log::warning(_send_buff);

    uv_buf_t buf = uv_buf_init(_send_buff, ss.str().size());
    int ret = uv_write(&_write, (uv_stream_t*)&_tcp, &buf, 1, write_cb);
    if (ret < 0)
    {
        Log::error("tcp send failed:%s-%s", uv_err_name(ret), uv_strerror(ret)); 
		_play_cb(_option.ssid, RTSP_ERR_TCP_SEND_FAILED);
        return ret;
    }
    return 0;
}

int CRtspClient::parse_options()
{
    if(strncasecmp(_recv_buff, "RTSP/1.0 200 OK\r\n", 17)) {
        Log::error("parse option error");
        _step_state = RTSP_STEP_FAILED;
		_play_cb(_option.ssid, RTSP_ERR_OPTION_FAILED);
        return -1;
    }

    _step_state = RTSP_STEP_SUCESS;
    return send_describe();
}

int CRtspClient::parse_describe()
{
    if(strncasecmp(_recv_buff, "RTSP/1.0 200 OK\r\n", 17)) {
        if(strncasecmp(_recv_buff, "RTSP/1.0 401 Unauthorized\r\n", 27)) {
            Log::error("parse describe error");
            _step_state = RTSP_STEP_FAILED;
			_play_cb(_option.ssid, RTSP_ERR_DESCRIBE_FAILED);
            return -1;
        }

        if(_need_auth > RTSP_AUTHORIZATION_NONE){
            _step_state = RTSP_STEP_FAILED;
            _play_cb(_option.ssid, RTSP_ERR_DESCRIBE_FAILED);
            Log::error("Authenticate failed %s:%s", _option.user_name.c_str(), _option.password.c_str());
            return -1;
        }

        do {
            char* p = strstr(_recv_buff, "\r\nWWW-Authenticate:");
            if(!p) break;

            char* Digest = strstr(p, " Digest ");
            char* Basic = strstr(p, " Basic ");
            if(Digest) {
                _need_auth = RTSP_AUTHORIZATION_DIGEST;

                char* realm = strstr(Digest, " realm=\"");
                if(!realm) break;
                realm += 8;
                char* end = strstr(realm, "\"");
                _realm = string(realm, end-realm);

                char* nonce = strstr(p, " nonce=\"");
                if(!nonce) break;
                nonce += 8;
                end = strstr(nonce, "\"");
                _nonce = string(nonce, end-nonce);

                MD5 md5;
                string auth = _option.user_name + ":" + _realm + ":" + _option.password;
                md5.ComputMd5(auth.c_str(), auth.size());
                _auth_md5 = md5.GetMd5();
            } else if(Basic){
                _need_auth = RTSP_AUTHORIZATION_BASIC;

                char* realm = strstr(Basic, " realm=\"");
                if(!realm) break;
                realm += 8;
                char* end = strstr(realm, "\"");
                _realm = string(realm, end-realm);

                string auth = _option.user_name + ":" + _option.password;
                _auth_md5 = CBase64::Encode((const unsigned char*)auth.c_str(), auth.size());
            } else {
                break;
            }

            return send_describe();
        }while(0);

        _step_state = RTSP_STEP_FAILED;
        _play_cb(_option.ssid, RTSP_ERR_DESCRIBE_FAILED);
        Log::error("Authenticate failed %s:%s", _option.user_name.c_str(), _option.password.c_str());
        return -1;
    }

    _step_state = RTSP_STEP_SUCESS;
    return send_setup();
}

int CRtspClient::parse_setup()
{
    if(strncasecmp(_recv_buff, "RTSP/1.0 200 OK\r\n", 17)) {
        Log::error("parse setup error");
        _step_state = RTSP_STEP_FAILED;
        _play_cb(_option.ssid, RTSP_ERR_SETUP_FAILED);
        return -1;
    }

    char* p = strstr(_recv_buff, "\r\nSession:");
    p += 10;
    while(*p == ' ') p++;
    char* session = p;
    char* e1 = strstr(p, ";");
    char* e2 = strstr(p, "\r\n");
    if(e1 && e2) {
        char* e;
        if(e1 < e2) e=e1; else e=e2;
        _session = string(session, e-session);
    } else if(e1 && !e2) {
        _session = string(session, e1-session);
    } else if(!e1 && e2) {
        _session = string(session, e2-session);
    } else {
        _step_state = RTSP_STEP_FAILED;
		_play_cb(_option.ssid, RTSP_ERR_SETUP_FAILED);
        return -1;
    }

    _step_state = RTSP_STEP_SUCESS;
    return send_play();
}

int CRtspClient::parse_play()
{
    if(strncasecmp(_recv_buff, "RTSP/1.0 200 OK\r\n", 17)) {
        Log::error("parse play error");
        _step_state = RTSP_STEP_FAILED;
		_play_cb(_option.ssid, RTSP_ERR_PLAY_FAILED);
        return -1;
    }

    _step_state = RTSP_STEP_SUCESS;
	_play_cb(_option.ssid, RTSP_ERR_SUCESS);
    return 0;
}

int CRtspClient::parse_teardown()
{
    if(strncasecmp(_recv_buff, "RTSP/1.0 200 OK\r\n", 17)) {
        Log::error("parse teardown error");
        _step_state = RTSP_STEP_FAILED;
        //_play_cb(_option, RTSP_ERR_TEARDOWN_FAILED);
        //return -1;
    }

    _step_state = RTSP_STEP_SUCESS;

    uv_shutdown(&_shutdown, (uv_stream_t*)&_tcp, shutdown_cb);
    return 0;
}