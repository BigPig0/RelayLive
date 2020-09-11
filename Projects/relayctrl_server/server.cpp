
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

    bool ParseHeader();    //��������ͷ
    bool ParsePath();      //��������ͷ�е�uri
    void WriteFailResponse(); //���Ĳ��Ϸ�ʱ��Ӧ��

    string                strRemoteIP;          //�ͻ���IP��ַ
    uint32_t              nRemotePort;          //�ͻ���port
    uv_tcp_t              socket;               //tcp����
    char                  readBuff[1024];       //��ȡ�ͻ������ݻ���
    string                dataCatch;            //��ȡ��������
    bool                  parseHeader;          //�Ƿ����Э��ͷ
    bool                  connected;

    string                method;               //����������ķ���
    string                path;                 //�����������uri
    string                version;              //�����������Э��汾
    string                Connection;           //����ͷ�е��ֶ�ֵ
    uint32_t              ContentLen;           //�������ݳ���

    string                rsBuff;               //Ӧ��Э������
    string                writeBuff;            //����Ҫ���͵�����
    bool                  writing;              //���ڷ���
    uv_write_t            writeReq;             //��������
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
            //�Զ˷�����FIN
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

/** ���յ��ͻ��˷����������� */
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

    //socketԶ��ip�Ͷ˿�
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

    // httpͷ����
    if(!parseHeader && dataCatch.find("\r\n\r\n") != std::string::npos) {
        if(!ParseHeader()) {
            Log::error("error request");
            WriteFailResponse();
            return;
        }
        parseHeader = true;

        Log::debug("Http req: %s", path.c_str());

        //��������
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
    size_t pos1 = dataCatch.find("\r\n");        //��һ�еĽ�β
    size_t pos2 = dataCatch.find("\r\n\r\n");    //ͷ�Ľ�βλ��
    string reqline = dataCatch.substr(0, pos1);  //��һ�е�����
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