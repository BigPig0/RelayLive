
#include "uv.h"
#include "util.h"
#include "utilc.h"
#include "ipc.h"
#include <sstream>

using namespace util;

uv_loop_t  uvLoopLive;
uv_tcp_t   uvTcpServer;

class CHttpSession
{
public:
    CHttpSession();
    ~CHttpSession();

    void OnRecv(char* data, int len);
    void OnClose();

    bool ParseHeader();    //解析报文头
    bool ParsePath();      //解析报文头中的uri
    void WriteFailResponse(); //报文不合法时的应答

    string                strRemoteIP;          //客户端IP地址
    uint32_t              nRemotePort;          //客户端port
    uv_tcp_t              socket;               //tcp连接
    char                  readBuff[1024];       //读取客户端内容缓存
    string                dataCatch;            //读取到的数据
    bool                  parseHeader;          //是否解析协议头
    bool                  connected;

    string                method;               //解析出请求的方法
    string                path;                 //解析出请求的uri
    string                version;              //解析出请求的协议版本
    string                Connection;           //报文头中的字段值
    uint32_t              ContentLen;           //报文内容长度

    string                rsBuff;               //应答协议数据
    string                writeBuff;            //缓存要发送的数据
    bool                  writing;              //正在发送
    uv_write_t            writeReq;             //发送请求
};

//////////////////////////////////////////////////////////////////////////

static void on_uv_close(uv_handle_t* handle) {
    CHttpSession *skt = (CHttpSession*)handle->data;
    Log::warning("on close client [%s:%u]", skt->strRemoteIP.c_str(), skt->nRemotePort);
    delete skt;
}

static void on_uv_shutdown(uv_shutdown_t* req, int status) {
    CHttpSession *skt = (CHttpSession*)req->data;
    Log::warning("on shutdown client [%s:%d]", skt->strRemoteIP.c_str(), skt->nRemotePort);
    delete req;
    uv_close((uv_handle_t*)&skt->socket, on_uv_close);
}

static void on_uv_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf){
    CHttpSession *skt = (CHttpSession*)handle->data;
    *buf = uv_buf_init(skt->readBuff, 1024);
}

static void on_uv_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    CHttpSession *skt = (CHttpSession*)stream->data;
    if(nread < 0) {
        skt->connected = false;
        if(nread == UV__ECONNRESET || nread == UV_EOF) {
            //对端发送了FIN
            Log::warning("remote close socket [%s:%u]", skt->strRemoteIP.c_str(), skt->nRemotePort);
            uv_close((uv_handle_t*)&skt->socket, on_uv_close);
        } else {
            uv_shutdown_t* req = new uv_shutdown_t;
            req->data = skt;
            Log::error("Read error %s", uv_strerror((int)nread));
            Log::warning("remote shutdown socket [%s:%u]", skt->strRemoteIP.c_str(), skt->nRemotePort);
            uv_shutdown(req, stream, on_uv_shutdown);
        }
        return;
    }

    skt->OnRecv(buf->base, (int)nread);
}

static void on_uv_write(uv_write_t* req, int status) {
    CHttpSession *skt = (CHttpSession*)req->data;
    skt->writing = false;
}

/** 接收到客户端发送来的连接 */
static void on_connection(uv_stream_t* server, int status) {
    if(status != 0) {
        Log::error("status:%d %s", status, uv_strerror(status));
        return;
    }

    CHttpSession *sess = new CHttpSession();
    int ret = uv_accept(server, (uv_stream_t*)(&sess->socket));
    if(ret != 0) {
        delete sess;
        return;
    }

    //socket远端ip和端口
    struct sockaddr peername;
    int namelen = sizeof(struct sockaddr);
    ret = uv_tcp_getpeername(&sess->socket, &peername, &namelen);
    if(peername.sa_family == AF_INET) {
        struct sockaddr_in* sin = (struct sockaddr_in*)&peername;
        char addr[46] = {0};
        uv_ip4_name(sin, addr, 46);
        sess->strRemoteIP = addr;
        sess->nRemotePort = sin->sin_port;
    } else if(peername.sa_family == AF_INET6) {
        struct sockaddr_in6* sin6 = (struct sockaddr_in6*)&peername;
        char addr[46] = {0};
        uv_ip6_name(sin6, addr, 46);
        sess->strRemoteIP = addr;
        sess->nRemotePort = sin6->sin6_port;
    }

    uv_read_start((uv_stream_t*)&sess->socket, on_uv_alloc, on_uv_read);
}

static void run_loop_thread(void* arg) {
    uv_run(&uvLoopLive, UV_RUN_DEFAULT);
    uv_loop_close(&uvLoopLive);
}

//////////////////////////////////////////////////////////////////////////

CHttpSession::CHttpSession()
    : parseHeader(false)
    , nRemotePort(0)
    , writing(false)
    , connected(true)
    , ContentLen(0)
{
    socket.data = this;
    uv_tcp_init(&uvLoopLive, &socket);
    writeReq.data = this;
}

CHttpSession::~CHttpSession() {
    Log::debug("~CHttpSession()");
}

void CHttpSession::OnRecv(char* data, int len) {
    dataCatch.append(data, len);

    // http头解析
    if(!parseHeader && dataCatch.find("\r\n\r\n") != std::string::npos) {
        if(!ParseHeader()) {
            Log::error("error request");
            WriteFailResponse();
            return;
        }
        parseHeader = true;

        Log::debug("Http req: %s", path.c_str());

        //解析请求
        if(!ParsePath()) {
            Log::error("error path:%s", path.c_str());
            WriteFailResponse();
            return;
        } else {
            stringstream ss;
            ss << version << " 200 OK\r\n"
                "Connection: close\r\n"
                "Access-Control-Allow-Origin: *\r\n"
                "Content-Length: " << writeBuff.size() << "\r\n"
                "\r\n";
            rsBuff = ss.str();

            writing = true;
            uv_buf_t buff[] = {
                uv_buf_init((char*)rsBuff.c_str(), rsBuff.size()),
                uv_buf_init((char*)writeBuff.c_str(), writeBuff.size()),
            };
            uv_write(&writeReq, (uv_stream_t*)&socket, buff, 2, on_uv_write);
        }
    } else {
        if(dataCatch.size() >= ContentLen) {
            parseHeader = false;
        }
    }
}

bool CHttpSession::ParseHeader() {
    size_t pos1 = dataCatch.find("\r\n");        //第一行的结尾
    size_t pos2 = dataCatch.find("\r\n\r\n");    //头的结尾位置
    string reqline = dataCatch.substr(0, pos1);  //第一行的内容
    vector<string> reqlines = util::String::split(reqline, ' ');
    if(reqlines.size() != 3)
        return false;


    method = reqlines[0].c_str();
    path = reqlines[1];
    version = reqlines[2].c_str();
    string rawHeaders = dataCatch.substr(pos1+2, pos2-pos1);
    dataCatch = dataCatch.substr(pos2+4, dataCatch.size()-pos2-4);

    vector<string> headers = util::String::split(rawHeaders, "\r\n");
    for(auto &hh : headers) {
        string name, value;
        bool b = false;
        for(auto &c:hh){
            if(!b) {
                if(c == ':'){
                    b = true;
                } else {
                    name.push_back(c);
                }
            } else {
                if(!value.empty() || c != ' ')
                    value.push_back(c);
            }
        }

        if(!strcasecmp(name.c_str(), "Connection")) {
            Connection = value;
        } else if(!strcasecmp(name.c_str(), "Content-Length")) {
            ContentLen = stoi(value);
        }
    }
    return true;
}

bool CHttpSession::ParsePath() {
    vector<string> uri = util::String::split(path, '?');
    if(uri.size() != 2 && uri.size() != 1)
        return false;

    if(!strcasecmp(uri[0].c_str(), "/device/clients")) {
        writeBuff = IPC::GetClientsJson();
    } else {
        return false;
    }

    return true;
}

void CHttpSession::WriteFailResponse() {
    stringstream ss;
    ss << version << " 400 Bad request\r\n"
        "Connection: close\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Content-Length: 0\r\n"
        "\r\n";
    rsBuff = ss.str();

    writing = true;
    uv_buf_t buff = uv_buf_init((char*)rsBuff.c_str(), rsBuff.size());
    uv_write(&writeReq, (uv_stream_t*)&socket, &buff, 1, on_uv_write);
}

//////////////////////////////////////////////////////////////////////////

void start_service(uint32_t port) {
    uv_loop_init(&uvLoopLive);
    uv_tcp_init(&uvLoopLive, &uvTcpServer);
    struct sockaddr_in addr;
    uv_ip4_addr("0.0.0.0", port, &addr);
    uv_tcp_bind(&uvTcpServer, (const sockaddr*)&addr, 0);
    uv_listen((uv_stream_t*)&uvTcpServer, 512, on_connection);
    uv_thread_t tid;
    uv_thread_create(&tid, run_loop_thread, NULL);
}

namespace Server
{
    int Init(int port) {
        start_service(port);
        return 0;
    }

    int Cleanup() {
        return 0;
    }
};