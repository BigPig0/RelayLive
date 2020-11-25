#include "server.h"
#include "sha1.h"
#include "base64.h"
#include "uv.h"
#include "util.h"
#include "utilc.h"
#include "worker.h"
#include <unordered_map>
#include <sstream>

#ifdef WINDOWS_IMPL
#include <direct.h>
#include <io.h>
#elif LINUX_IMPL
#include <stdarg.h>
#include <sys/stat.h>
#endif

using namespace util;

uv_loop_t  uvLoopLive;
uv_tcp_t   uvTcpServer;

int CreatDir(string strDirPath) {

    uint32_t iRet;
    size_t iLen = strDirPath.size();
    //��ĩβ��/
    if (strDirPath[iLen - 1] != '\\' && strDirPath[iLen - 1] != '/') {
        strDirPath += '/';
        iLen++;
    }

    char *pszDir = (char*)strDirPath.c_str();
    // ����Ŀ¼
    for (size_t i = 0;i < iLen; i++) {
        if (pszDir[i] == '\\' || pszDir[i] == '/') { 
            pszDir[i] = '\0';

            //���������,����
            iRet = _access(pszDir,0);
            if (iRet != 0) {
                iRet = _mkdir(pszDir);
                if (iRet != 0) {
                    return -1;
                } 
            }
            //֧��linux,������\����/
            pszDir[i] = '/';
        } 
    }

    return 0;
}

class CLiveSession
{
public:
    CLiveSession();
    ~CLiveSession();

    void OnRecv(char* data, int len);
	void OnClose();

    bool ParseHeader();    //��������ͷ
    bool ParsePath();      //��������ͷ�е�uri
    void WsAcceptKey();     //����websocketӦ���Sec-WebSocket-Accept�ֶ�
    void WriteFailResponse(); //���Ĳ��Ϸ�ʱ��Ӧ��

    string                strRemoteIP;          //�ͻ���IP��ַ
    uint32_t              nRemotePort;          //�ͻ���port
    uv_tcp_t              socket;               //tcp����
    char                  readBuff[1024*1024];  //��ȡ�ͻ������ݻ���
    string                dataCatch;            //��ȡ��������
    bool                  parseHeader;          //�Ƿ����Э��ͷ
	bool                  connected;
	int                   handlecount;

    string                method;               //����������ķ���
    string                path;                 //�����������uri
    RequestParam          Params;               //uri�н���������
    string                version;              //�����������Э��汾
    string                Connection;           //����ͷ�е��ֶ�ֵ
    string                Upgrade;              //����ͷ�е��ֶ�ֵ
    string                SecWebSocketKey;      //websocket����ͷ�е��ֶ�ֵ
    string                SecWebSocketAccept;   //�����websocketӦ����ͷ���ֶ�ֵ
    string                xForwardedFor;        //����ͷ�е��ֶ�ֵ ��������

    bool                  isWs;                 // �Ƿ�Ϊwebsocket
    CLiveWorker          *pWorker;              // worker����

	CNetStreamMaker       wsBuff;               //Э��ͷ����
    CNetStreamMaker       writeBuff;            //����Ҫ���͵�����
    bool                  writing;              //���ڷ���
    uv_write_t            writeReq;             //��������
};

//////////////////////////////////////////////////////////////////////////

static void on_close(uv_handle_t* handle) {
	CLiveSession *skt = (CLiveSession*)handle->data;
	skt->handlecount--;
	if(skt->handlecount == 0 && !skt->connected) {
		delete skt;
	}
}

static void on_uv_close(uv_handle_t* handle) {
    CLiveSession *skt = (CLiveSession*)handle->data;
    Log::warning("on close client [%s:%u]", skt->strRemoteIP.c_str(), skt->nRemotePort);
    delete skt;
}

static void on_uv_shutdown(uv_shutdown_t* req, int status) {
    CLiveSession *skt = (CLiveSession*)req->data;
    Log::warning("on shutdown client [%s:%d]", skt->strRemoteIP.c_str(), skt->nRemotePort);
    delete req;
    if (skt->socket.flags & 0x01) {
        return;
    }
    uv_close((uv_handle_t*)&skt->socket, on_uv_close);
}

static void on_uv_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf){
    CLiveSession *skt = (CLiveSession*)handle->data;
    *buf = uv_buf_init(skt->readBuff, 1024*1024);
}

static void on_uv_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    CLiveSession *skt = (CLiveSession*)stream->data;
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
    CLiveSession *skt = (CLiveSession*)req->data;
    skt->writing = false;
    skt->OnClose();
}

/** ���յ��ͻ��˷����������� */
static void on_connection(uv_stream_t* server, int status) {
    if(status != 0) {
        Log::error("status:%d %s", status, uv_strerror(status));
        return;
    }

    CLiveSession *sess = new CLiveSession();
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

    Log::debug("new connect:[%s:%d]", sess->strRemoteIP.c_str(), sess->nRemotePort);
    uv_read_start((uv_stream_t*)&sess->socket, on_uv_alloc, on_uv_read);
}

static void on_uv_async_close(uv_async_t* handle) {
    CLiveSession *skt = (CLiveSession*)handle->data;
}

static void run_loop_thread(void* arg) {
    uv_run(&uvLoopLive, UV_RUN_DEFAULT);
    uv_loop_close(&uvLoopLive);
}

//////////////////////////////////////////////////////////////////////////

RequestParam::RequestParam()
    : strType("mp4")
    , strImgType("jpg")
    , nWidth(0)
    , nHeight(0)
    , nProbSize(Settings::getValue("FFMPEG","probsize",25600))
    , nProbTime(Settings::getValue("FFMPEG","probtime",1))
    , nInCatch(Settings::getValue("FFMPEG","incatch",1024*16))
    , nOutCatch(Settings::getValue("FFMPEG","outcatch",1024*16))
    , nImageNumber(0)
    , nVideoDuration(0)
{
    strSavePath = Settings::getValue("Save","path","/");
    char endC = strSavePath.at(strSavePath.size()-1);
    if(endC != '/' && endC != '\\') {
        strSavePath += '/';
    }
}

CLiveSession::CLiveSession()
    : parseHeader(false)
    , isWs(false)
    , pWorker(NULL)
    , nRemotePort(0)
    , writing(false)
	, connected(true)
	, handlecount(2)
{
    socket.data = this;
    uv_tcp_init(&uvLoopLive, &socket);
    writeReq.data = this;
}

CLiveSession::~CLiveSession() {
	Log::debug("~CLiveSession()");
}

void CLiveSession::OnRecv(char* data, int len) {
    dataCatch.append(data, len);

    // httpͷ����
    if(!parseHeader && dataCatch.find("\r\n\r\n") != std::string::npos) {
        if(!ParseHeader()) {
            Log::error("error request");
            WriteFailResponse();
            return;
        }
		parseHeader = true;

        if(!strcasecmp(Connection.c_str(), "Upgrade") && !strcasecmp(Upgrade.c_str(), "websocket")) {
            isWs = true;
			Log::debug("Websocket req: %s", path.c_str());
        } else {
			Log::debug("Http req: %s", path.c_str());
		}

        //��������
        if(!ParsePath()) {
            Log::error("error path:%s", path.c_str());
            WriteFailResponse();
            return;
        }

        //����������Ҫ,����ͼƬ����Ƶ�ļ��ĵ�ַ
        std::string dirName = util::CTimeFormat::printNow("%Y");
        dirName += "/";
        dirName += util::CTimeFormat::printNow("%m");
        dirName += "/";
        dirName += util::CTimeFormat::printNow("%d");
        for(int i=0; i<Params.nImageNumber; i++) {
            std::string imagePath = "/image/" + dirName + "/" + util::Guid::guid() + "." + Params.strImgType;
            Params.lstImgPath.push_back(imagePath);
        }
        if(Params.nVideoDuration > 0) {
            Params.videoPath = "/video/" + dirName + "/" + util::Guid::guid() + "." + Params.strType;
        }

        //����Ӧ����
        std::string response = "{\"code\":0,\"message\":null,\"pics\":";
        if(Params.lstImgPath.empty()) {
            response += "null,";
        } else {
            response += "\"";
            bool first = true;
            for(auto imagePath : Params.lstImgPath) {
                if(first) {
                    first = false;
                } else {
                    response += ",";
                }
                response += imagePath;
            }
            response += "\",";
        }
        response += "\"video\":";
        if(Params.videoPath.empty()) {
            response += "null";
        } else {
            response += "\"";
            response += Params.videoPath;
            response += "\"";
        }
        response += "}\r\n";
        size_t resLen = response.size();

        //����Ӧ��
        stringstream ss;
        if(isWs){
            WsAcceptKey();
            ss << version << " 101 Switching Protocols\r\n"
                "Upgrade: WebSocket\r\n"
                "Connection: Upgrade\r\n"
                "Sec-WebSocket-Accept: " << SecWebSocketAccept << "\r\n"
                "\r\n";
            if(resLen <= 125) {
                ss << (char)(0x82) //10000010
                   << (char)(resLen);
            } else if(len <= 65535) {
                ss << (char)(0x82) //10000010
                   << (char)(126)
                   << (char)(resLen >> 8)
                   << (char)(resLen);
            } else {
                ss << (char)(0x82) //10000010
                   << (char)(127)
                   << (char)(resLen >> 54)
                   << (char)(resLen >> 48)
                   << (char)(resLen >> 40)
                   << (char)(resLen >> 32)
                   << (char)(resLen >> 24)
                   << (char)(resLen >> 16)
                   << (char)(resLen >> 8)
                   << (char)(resLen);
            }
            ss << response;
        } else {
            ss << version << " 200 OK\r\n"
                "Connection: close\r\n"
                "Access-Control-Allow-Origin: *\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: " << resLen << "\r\n"
                "Cache-Control: no-cache\r\n"
                "Pragma: no-cache\r\n"
                "\r\n" << response;
        }

        writeBuff.clear();
        writeBuff.append_string(ss.str().c_str());
        writing = true;
        uv_buf_t buff = uv_buf_init(writeBuff.get(), writeBuff.size());
        uv_write(&writeReq, (uv_stream_t*)&socket, &buff, 1, on_uv_write);

        //����ļ����Ƿ���ڲ�����
        if(Params.nImageNumber > 0) {
            string path = Params.strSavePath + "/image/" + dirName + "/";
            CreatDir(path);
        }
        if(Params.nVideoDuration > 0) {
            string path = Params.strSavePath + "/video/" + dirName + "/";
            CreatDir(path);
        }

        //����worker
        //pWorker = CreatLiveWorker(Params);

    } else {
		if(dataCatch.find("\r\n\r\n") != std::string::npos) {
			Log::debug("%s", dataCatch.c_str());
		}
    }
}

void CLiveSession::OnClose() {
	if(connected) {
		connected = false;
		uv_shutdown_t* req = new uv_shutdown_t;
		req->data = this;
		uv_shutdown(req, (uv_stream_t*)&socket, on_uv_shutdown);
	}
}

bool CLiveSession::ParseHeader() {
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
        } else if(!strcasecmp(name.c_str(), "Upgrade")) {
            Upgrade = value;
        } else if(!strcasecmp(name.c_str(), "Sec-WebSocket-Key")) {
            SecWebSocketKey = value;
        } else if(!strcasecmp(name.c_str(), "x-forwarded-for")) {
            xForwardedFor = value;
        } 
    }
    return true;
}

bool CLiveSession::ParsePath() {
    vector<string> uri = util::String::split(path, '?');
    if(uri.size() != 2 && uri[0] != "capture")
        return false;

    vector<string> param = util::String::split(uri[1], '&');
    for(auto p:param) {
        vector<string> kv = util::String::split(p, '=');
        if(kv.size() != 2)
            continue;

        if(!strcasecmp(kv[0].c_str(),"url")) 
            Params.strUrl = kv[1];
        else if(!strcasecmp(kv[0].c_str(),"videotype"))
            Params.strType = kv[1];
        else if(!strcasecmp(kv[0].c_str(),"imagetype"))
            Params.strImgType = kv[1];
        else if(!strcasecmp(kv[0].c_str(),"hw")){
            string strHw = kv[1];
            if(!strHw.empty() && strHw.find('*') != string::npos)
                sscanf(strHw.c_str(), "%d*%d", &Params.nWidth, &Params.nHeight);
        } 
        else if(!strcasecmp(kv[0].c_str(),"probsize"))
            Params.nProbSize = stoi(kv[1]);
        else if(!strcasecmp(kv[0].c_str(),"probtime"))
            Params.nProbTime = stoi(kv[1]);
        else if(!strcasecmp(kv[0].c_str(),"incatch"))
            Params.nInCatch = stoi(kv[1]);
        else if(!strcasecmp(kv[0].c_str(),"outcatch"))
            Params.nOutCatch = stoi(kv[1]);
        else if(!strcasecmp(kv[0].c_str(), "imageNumber"))
            Params.nImageNumber = stoi(kv[1]);
        else if(!strcasecmp(kv[0].c_str(), "videoDuration"))
            Params.nVideoDuration = stoi(kv[1]);
    }
    return true;
}

void CLiveSession::WsAcceptKey() {
    SHA1 sha1;
    string tmp = SecWebSocketKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    tmp = (char*)sha1.Comput((uint8_t*)tmp.c_str(), tmp.size());
    SecWebSocketAccept = Base64::Encode((uint8_t*)tmp.c_str(), tmp.size());
	Log::debug("%s --> %s", SecWebSocketKey.c_str(), SecWebSocketAccept.c_str());
}

void CLiveSession::WriteFailResponse() {
    stringstream ss;
    ss << version << " 400 Bad request\r\n"
        "Connection: " << Connection << "\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Content-Length: 0\r\n"
        "\r\n";

    writeBuff.clear();
    writeBuff.append_string(ss.str().c_str());

    writing = true;
    uv_buf_t buff = uv_buf_init(writeBuff.get(), writeBuff.size());
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

namespace Server {

int Init(int port) {
    InitFFmpeg();
        
    start_service(port);

    return 0;
}

int Cleanup() {
    return 0;
}
}