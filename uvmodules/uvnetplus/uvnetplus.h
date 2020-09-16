#pragma once
#include "uvnetpuclic.h"
#include <string>
#include <list>
#include <vector>
#include <unordered_map>
#include <map>
#include <stdint.h>

namespace uvNetPlus {

/** 事件循环eventloop执行线程，封装uv_loop */
class CNet
{
public:
    static CNet* Create();
    virtual ~CNet(){};
    virtual void* Loop() = 0;
protected:
    CNet(){};
};

//声明用到的所有类
class CTcpSocket;
class CTcpServer;
class CTcpAgent;
class CTcpConnPool;

class CHttpMsg;
class CHttpRequest;
class CHttpResponse;
class CHttpClient;
class CHttpServer;

//////////////////////////////////////////////////////////////////////////

/** TCP客户端 */
class CTcpSocket
{
    typedef void (*EventCB)(CTcpSocket* skt);
    typedef void (*RecvCB)(CTcpSocket* skt, char *data, int len);
    typedef void (*ErrorCB)(CTcpSocket* skt, std::string error);
public:
    EventCB      OnReady;     //socket创建完成
    ErrorCB      OnConnect;   //连接完成
    RecvCB       OnRecv;      //收到数据
    EventCB      OnDrain;     //发送队列全部完成
    EventCB      OnCLose;     //socket关闭
    EventCB      OnEnd;       //收到对方fin,读到eof
    EventCB      OnTimeout;   //超时回调
    ErrorCB      OnError;     //错误回调

    bool         autoRecv;    //连接建立后是否立即自动接收数据。默认true
    bool         copy;        //发送的数据拷贝到临时区域
    void        *userData;    //用户绑定自定义数据
    uint64_t     fd;          // SOCKET的值(测试用)

    /**
     * 创建一个tcp连接客户端实例
     * @param net 环境句柄
     * @param usr 设定用户自定义数据
     * @param copy 调用发送接口时，是否将数据拷贝到缓存由内部进行管理
     */
    static CTcpSocket* Create(CNet* net, void *usr=nullptr, bool copy=true);

    /**
     * 异步删除这个实例
     */
    virtual void Delete() = 0;

    /**
     * 连接服务器，连接完成后调用OnConnect回调
     */
    virtual void Connect(std::string strIP, uint32_t nPort) = 0;

    /**
     * 设置socket的本地端口，如果不指定，将有系统自动分配
     * @param strIP 本地IP，用来指定本定使用哪一个网卡。空表示不指定。
     * @param nPort 本定端口，0表示不指定
     */
    virtual void SetLocal(std::string strIP, uint32_t nPort) = 0; 

    /**
     * 建立一个socket后，可以获取其本地地址
     */
    virtual void GetLocal(std::string &strIP, uint32_t &nPort) = 0;

    /**
     * 发送数据。将数据放到本地缓存起来
     */
    virtual void Send(const char *pData, uint32_t nLen) = 0;

protected:
    CTcpSocket();
    virtual ~CTcpSocket() = 0;
};

/** TCP服务端 */
class CTcpServer
{
    typedef void (*EventCB)(CTcpServer* svr, std::string err);
    typedef void (*ConnCB)(CTcpServer* svr, std::string err, CTcpSocket* client);
public:

    EventCB          OnListen;       // 开启监听完成回调，错误时上抛错误消息
    ConnCB           OnConnection;   // 新连接回调
    EventCB          OnClose;        // 监听socket关闭完成回调
    EventCB          OnError;        // 发生错误回调

    void            *userData;

    /**
     * 创建一个tcp服务端实例
     * @param net 环境句柄
     * @param onConnection 指定收到新连接时的回调
     * @param usr 设定用户自定义数据
     */
    static CTcpServer* Create(CNet* net, ConnCB onConnection, void *usr=nullptr);

    /**
     * 异步删除当前实例
     */
    virtual void Delete() = 0;

    /**
     * 启动监听
     * @param strIP 本地IP，用来指定本定使用哪一个网卡
     * @param nPort 本地监听端口
     */
    virtual bool Listen(std::string strIP, uint32_t nPort) = 0;

    /** 服务器是否在监听连接 */
    virtual bool Listening() = 0;
protected:
    CTcpServer();
    virtual ~CTcpServer() = 0;
};

/** 简单的TCP连接池管理 */
class CTcpAgent
{
    typedef void (*EventCB)(CTcpAgent *agent, CTcpSocket *skt);
public:
    /** 创建连接池 */
    static CTcpAgent* Create(CNet* net);

    /** 向连接池放入一个socket */
    virtual bool Put(CTcpSocket *skt) = 0;

    /** 从连接池移除一个socket */
    virtual bool Remove(CTcpSocket *skt) = 0;

    /** 销毁连接池 */
    virtual void Delete() = 0;

    EventCB    onTimeOut;   //超时回调
    uint32_t   timeOut;     //空闲连接超时时间 秒 默认20s 0为永不超时
protected:
    CTcpAgent();
    virtual ~CTcpAgent() = 0;
};

//////////////////////////////////////////////////////////////////////////

/** TCP连接池 请求结构 */
struct CTcpRequest {
    std::string     host;   //请求目标域名或ip
    uint32_t        port;   //请求端口
    std::string     localaddr; //本地ip，表明使用哪一块网卡。默认空，不限制
    bool            copy;   //需要发送的数据是否拷贝到内部维护
    bool            recv;   //tcp请求是否需要接收数据
    void           *usr;    //用户自定义数据
    bool            autodel;//回调后自动删除，不需要用户手动删除。默认true。

    CTcpConnPool   *pool;   //从哪一个连接池获取连接
    CTcpRequest()
        : port(80)
        , copy(true)
        , recv(true)
        , usr(NULL)
        , autodel(true)
        , pool(NULL)
    {}
};

/** TCP客户端连接池，自动管理多个CTcpAgent */
class CTcpConnPool
{
    typedef void(*ErrorCB)(CTcpRequest *req, std::string error);
    typedef void (*ReqCB)(CTcpRequest* req, CTcpSocket* skt);
public:
    uint32_t   maxConns;    //最大连接数 默认512(busy+idle)
    uint32_t   maxIdle;     //最大空闲连接数 默认100
    uint32_t   timeOut;     //空闲连接超时时间 秒 默认20s 0为永不超时
    uint32_t   maxRequest;  //连接达到最大时能存放的请求数 默认0 不限制

    ErrorCB    OnError;     //获取连接失败回调
    ReqCB      OnRequest;   //获取TCP客户端连接回调

    /**
     * 创建连接池
     * @param net loop实例
     * @param onReq 获取TCP客户端连接回调
     */
    static CTcpConnPool* Create(CNet* net, ReqCB onReq);

    /**
     * 异步删除连接池
     */
    virtual void Delete() = 0;

    /**
     * 从连接池获取一个socket。内部申请的对象，需要用户删除
     * @param host 请求目标域名或端口
     * @param port 请求目标端口
     * @param localaddr 本地ip，指定网卡，为空表示不指定
     * @param usr 绑定一个用户数据，回调时作为参数输出
     * @param copy 发送的数据是否拷贝到内部
     * @param recv 是否需要接收应答
     * @return 返回新的请求实例
     */
    virtual bool Request(std::string host, uint32_t port, std::string localaddr
        , void *usr=nullptr, bool copy=true, bool recv=true) = 0;

    /**
     * 从连接池获取一个socket
     * @param req 请求参数结构
     */
    virtual bool Request(CTcpRequest *req) = 0;

protected:
    CTcpConnPool();
    virtual ~CTcpConnPool() = 0;
};

//////////////////////////////////////////////////////////////////////////
namespace Http {
#ifdef WIN32
typedef std::unordered_multimap<std::string, std::string> hash_list;
#else
typedef std::multimap<std::string, std::string> hash_list;
#endif

enum METHOD {
    OPTIONS = 0,
    HEAD,
    GET,
    POST,
    PUT,
    DEL,
    TRACE,
    CONNECT
};

enum VERSION {
    HTTP1_0 = 0,
    HTTP1_1,
    HTTP2,
    HTTP3
};

class CHttpMsg {
public:
    bool aborted;   //请求终止时设置为true
    bool complete;  //http消息接收完整时设置为true

    METHOD      method;     // 请求方法
    std::string path;        // 请求路径

    int         statusCode;     //应答状态码
    std::string statusMessage;  //应答状态消息

    VERSION     version;        //http版本号 1.1或1.0
    std::string rawHeaders;     //完整的头部字符串
    std::string rawTrailers;    //完整的尾部字符串
    hash_list   headers;        //解析好的http头部键值对
    hash_list   trailers;       //解析好的http尾部键值对
    bool        keepAlive;      // 是否使用长连接, true时，使用CTcpConnPool管理连接
    bool        chunked;        // Transfer-Encoding: chunked
    uint32_t    contentLen;     // chunked为false时：内容长度；chunked为true时，块长度
    std::string content;        // 一次的接收内容

    CHttpMsg();
    ~CHttpMsg();
};

class CHttpConnect {
public:
    CHttpConnect();
    ~CHttpConnect();

    /**
     * 显示填写http头，调用后隐式http头的接口就无效了
     * @param headers http头域完整字符串，包含每一行结尾的"\r\n"
     */
    virtual void WriteHead(std::string headers);

    /**
     * 获取已经设定的隐式头
     */
    virtual std::vector<std::string> GetHeader(std::string name);

    /**
     * 获取所有设定的隐式头的key
     */
    virtual std::vector<std::string> GetHeaderNames();

    /**
     * 隐式头是否已经包含一个名称
     */
    virtual bool HasHeader(std::string name);

    /**
     * 移除一个隐式头
     */
    virtual void RemoveHeader(std::string name);

    /**
     * 重新设置一个头的值，或者新增一个隐式头
     * param name field name
     * param value 单个field value
     * param values 以NULL结尾的字符串数组，多个field value
     */
    virtual void SetHeader(std::string name, std::string value);
    virtual void SetHeader(std::string name, char **values);

    /**
     * 设置内容长度。内容分多次发送，且不使用chunked时使用。
     */
    virtual void SetContentLen(uint32_t len);

    /**
     * 查看是否完成
     */
    virtual bool Finished();

public:
     CTcpSocket        *tcpSocket;

protected:
    /** 由隐式头组成字符串 */
    std::string getImHeaderString();

protected:
    std::string         m_strHeaders;   // 显式的头
    hash_list           m_Headers;      // 隐式头的内容
    bool                m_bHeadersSent; // header是否已经发送
    bool                m_bFinished;    // 发送是否完成
    uint32_t            m_nContentLen;  // 设置内容的长度
};

class CHttpRequest : public CHttpConnect {
    typedef void(*ErrorCB)(CHttpRequest *req, std::string error);
    typedef void(*ResCB)(CHttpRequest *req, CHttpMsg* response);
public:
    typedef void(*DrainCB)(CHttpRequest *req);

    PROTOCOL            protocol;  // 协议,http或https
    METHOD              method;    // 方法
    std::string         path;      // 请求路径
    VERSION             version;   // http版本号 1.0或1.1
    std::string         host;      // 域名或IP
    int                 port;      // 端口
    std::string         localaddr; // 指定本地IP，默认为空
    int                 localport; // 指定本地端口， 默认为0。只有很特殊的情形需要设置，正常都不需要
    bool                keepAlive; // 是否使用长连接, true时，使用CTcpConnPool管理连接
    bool                chunked;   // Transfer-Encoding: chunked
    void               *usrData;   // 用户自定义数据
    bool                autodel;   // 接收完成后自动删除，不需要手动释放。
    uint64_t            fd;        // SOCKET的值(测试用)


    /** 客户端收到connect方法的应答时回调 */
    ResCB OnConnect;
    /** 客户端收到1xx应答(101除外)时回调 */
    ResCB OnInformation;
    /** 客户端收到101 upgrade 时回调 */
    ResCB OnUpgrade;
    /** 客户端收到应答时回调，如果是其他指定回调，则不会再次进入这里 */
    ResCB OnResponse;


    ErrorCB     OnError;        // 发生错误
    DrainCB     OnDrain;        // 发送数据完成

    /** 删除实例 */
    virtual void Delete() = 0;

    /**
     * 用来发送一块数据，如果chunked=true，发送一个chunk的数据
     * 如果chunked=false，使用这个方法多次发送数据，必须自己在设置头里设置length
     * @param chunk 需要发送的数据
     * @param len 发送的数据长度
     * @param cb 数据写入缓存后调用
     */
    virtual bool Write(const char* chunk, int len, DrainCB cb = NULL) = 0;

    /**
     * 完成一个发送请求，如果有未发送的部分则将其发送，如果chunked=true，额外发送结束段'0\r\n\r\n'
     * 如果chunked=false,协议头没有发送，则自动添加length
     */
    virtual bool End() = 0;

    /**
     * 相当于Write(data, len, cb);end();
     */
    virtual void End(const char* data, int len) = 0;
protected:
    CHttpRequest();
    virtual ~CHttpRequest() = 0;
};

class CHttpClient {
public:
    typedef void(*ReqCB)(CHttpRequest *req, void* usr, std::string error);

    /**
     * 创建一个http客户端环境
     * @param net 环境句柄
     * @param maxConns 同一个地址最大连接数
     * @param maxIdle 同一个地址最大空闲连接
     * @param timeOut 空闲连接超时时间
     * @param maxRequest 同一个地址请求最大缓存
     */
    CHttpClient(CNet* net, uint32_t maxConns=512, uint32_t maxIdle=100, uint32_t timeOut=20, uint32_t maxRequest=0);
    ~CHttpClient();
    bool Request(std::string host, int port, void* usr = NULL, ReqCB cb = NULL);

    /**
     * 默认请求获取成功回调函数，如果Request设置了指定回调，则优先使用指定的回调
     */
    ReqCB                OnRequest;
    CTcpConnPool        *connPool;
};

class CHttpResponse : public CHttpConnect {
public:
    typedef void(*ResCb)(CHttpResponse *response);

    bool                sendDate;      // 默认true，在发送头时自动添加Date头(已存在则不会添加)
    int                 statusCode;    // 状态码
    std::string         statusMessage; //自定义的状态消息，如果为空，发送时会取标准消息
    VERSION             version;       // http版本号 1.0或1.1
    bool                keepAlive; // 是否使用长连接, true时，使用CTcpConnPool管理连接
    bool                chunked;   // Transfer-Encoding: chunked

    /** 发送完成前,socket中断了会回调该方法 */
    ResCb OnClose;
    /** 应答发送完成时回调，所有数据都已经发送 */
    ResCb OnFinish;

    /**
     * 添加一个尾部数据
     * @param key 尾部数据的field name，这个值已经在header中的Trailer里定义了
     * @param value 尾部数据的field value
     */
    virtual void AddTrailers(std::string key, std::string value) = 0;

    /**
     * Sends a HTTP/1.1 100 Continue message。包括write和end的功能
     */
    virtual void WriteContinue() = 0;

    /**
     * Sends a HTTP/1.1 102 Processing message to the client
     */
    virtual void WriteProcessing() = 0;

    /**
     * 显示填写http头，调用后隐式http头的接口就无效了
     * @param statusCode 响应状态码
     * @param statusMessage 自定义状态消息，可以为空，则使用标准消息
     * @param headers http头域完整字符串，每行都要包含"\r\n"
     */
    virtual void WriteHead(int statusCode, std::string statusMessage, std::string headers) = 0;

    /**
     * 如果调用了此方法，但没有调用writeHead()，则使用隐式头并立即发送头
     */
    virtual void Write(const char* chunk, int len, ResCb cb = NULL) = 0;

    /**
     * 表明应答的所有数据都已经发送。每个实例都需要调用一次end。执行后会触发OnFinish
     */
    virtual void End() = 0;

    /**
     * 相当于调用write(data, len, cb) ; end()
     */
    virtual void End(const char* data, int len, ResCb cb = NULL) = 0;

protected:
    CHttpResponse();
    virtual ~CHttpResponse() = 0;
};

class CHttpServer {
    typedef void(*ReqCb)(CHttpServer *server, CHttpMsg *request, CHttpResponse *response);
public:
    /** 接受到一个包含'Expect: 100-continue'的请求时调用，如果没有指定，自动发送'100 Continue' */
    ReqCb OnCheckContinue;
    /** 接收到一个包含Expect头，但不是100的请求时调用，如果没有指定，自动发送'417 Expectation Failed' */
    ReqCb OnCheckExpectation;
    /** 收到upgrade请求时调用 */
    ReqCb OnUpgrade;
    /** 接收到一个请求，如果是其他指定的回调，就不会进入这里 */
    ReqCb OnRequest;

    /** 创建一个实例 */
    static CHttpServer* Create(CNet* net);

    /** 设备长连接保活时间，超过保活时间而没有新请求则断开连接 */
    virtual void SetKeepAlive(uint32_t secends) = 0;
    /** 服务器启动监听 */
    virtual bool Listen(std::string strIP, uint32_t nPort) = 0;
    /** 服务器关闭 */
    virtual void Close() = 0;
    /** 服务器是否在监听连接 */
    virtual bool Listening() = 0;
protected:
    CHttpServer();
    virtual ~CHttpServer() = 0;
};
}; //namespace Http

//////////////////////////////////////////////////////////////////////////
namespace Ftp {
enum FTP_CMD {
    FTP_CMD_ABOR = 0, //中断数据连接程序
    FTP_CMD_ACCT, //系统特权帐号
    FTP_CMD_ALLO, //为服务器上的文件存储器分配字节
    FTP_CMD_APPE, //添加文件到服务器同名文件
    FTP_CMD_CDUP, //改变服务器上的父目录
    FTP_CMD_CWD, //改变服务器上的工作目录
    FTP_CMD_DELE, //删除服务器上的指定文件
    FTP_CMD_HELP, //返回指定命令信息
    FTP_CMD_LIST, //如果是文件名列出文件信息，如果是目录则列出文件列表
    FTP_CMD_MODE, //传输模式（S=流模式，B=块模式，C=压缩模式）
    FTP_CMD_MKD, //在服务器上建立指定目录
    FTP_CMD_NLST, //列出指定目录内容
    FTP_CMD_NOOP, //无动作，除了来自服务器上的承认
    FTP_CMD_PASS, //系统登录密码
    FTP_CMD_PASV, //请求服务器等待数据连接
    FTP_CMD_PORT, //IP 地址和两字节的端口 ID
    FTP_CMD_PWD, //显示当前工作目录
    FTP_CMD_QUIT, //从 FTP 服务器上退出登录
    FTP_CMD_REIN, //重新初始化登录状态连接
    FTP_CMD_REST, //由特定偏移量重启文件传递
    FTP_CMD_RETR, //从服务器上找回（复制）文件
    FTP_CMD_RMD, //在服务器上删除指定目录
    FTP_CMD_RNFR, //对旧路径重命名
    FTP_CMD_RNTO, //对新路径重命名
    FTP_CMD_SITE, //由服务器提供的站点特殊参数
    FTP_CMD_SMNT, //挂载指定文件结构
    FTP_CMD_STAT, //在当前程序或目录上返回信息
    FTP_CMD_STOR, //储存（复制）文件到服务器上
    FTP_CMD_STOU, //储存文件到服务器名称上
    FTP_CMD_STRU, //数据结构（F=文件，R=记录，P=页面）
    FTP_CMD_SYST, //返回服务器使用的操作系统
    FTP_CMD_TYPE, //数据类型（A=ASCII，E=EBCDIC，I=binary）
    FTP_CMD_USER, //系统登录的用户名
    FTP_CMD_OPTS, //设置选项
    FTP_CONN      //tcp连接
};

enum FTP_DATA_MOD {
    FTP_DATA_MOD_PASV = 0,
    FTP_DATA_MOD_PORT
};

enum FTP_FILE_TYPE {
    FTP_FILE_TYPE_ASCII = 0,
    FTP_FILE_TYPE_EBCDIC,
    FTP_FILE_TYPE_BINARY
};

struct CFtpFile {
    std::string rawData;    //服务器返回的数据，没有解析
    bool        isDir;      //是否为目录
    std::string permission; //权限字符串,10位，第一位d代表目录，后面代表用户、组、其他的rwx权限
    std::string owner;
    std::string group;
    uint64_t    size;
    std::string date;
    time_t      dateTime;
    std::string name;
};

struct CFtpMsg {
    FTP_CMD cmd;          //请求命令
    std::string cmdParam; //参数
    std::string cmdStr;   //完整请求命令
    int replyCode;        //服务器返回码
    std::string replyStr; //返回消息内容
    CFtpMsg():replyCode(0){}
    void Init(FTP_CMD c, std::string p);
};

class CFtpRequest {
public:
    typedef void(*ReqCB)(CFtpRequest *req, CFtpMsg *msg);
    typedef void(*SuccessCB)(CFtpRequest *req);
    typedef void(*NameListCB)(CFtpRequest *req, std::list<std::string> names);
    typedef void(*ListCB)(CFtpRequest *req, std::list<CFtpFile> files);
    typedef void(*DownloadCB)(CFtpRequest *req, char* data, uint32_t size);
    std::string         host;      // 域名或IP
    int                 port;      // 端口
    std::string         user;      // 用户名
    std::string         pwd;       // 密码
    void               *usrData;   // 用户自定义数据
    std::string         path;      // 当前目录
    FTP_DATA_MOD        dataMod;   // 被动模式还是主动模式

    /** 删除实例 */
    virtual void Delete() = 0;

    /**
     * 获取当前工作目录
     */
    virtual void GetWorkingDirectory(SuccessCB cb) = 0;

    /**
     * 改变服务器上的工作目录CWD
     */
    virtual void ChangeWorkingDirectory(std::string path, SuccessCB cb) = 0;

    /**
     * 切换文件类型
     */
    virtual void SetFileType(FTP_FILE_TYPE t, SuccessCB cb) = 0;

    /**
     * 获取服务器文件名称列表NLST
     */
    virtual void NameList(NameListCB cb) = 0;

    /**
     * 获取文件信息或文件列表LIST
     */
    virtual void List(ListCB cb) = 0;

    /**
     * 下载文件
     */
    virtual void Download(std::string file, DownloadCB cb) = 0;

    /**
     * 上传文件
     */
    virtual void Upload(std::string file, char *data, int size, SuccessCB cb) = 0;

    /**
     * 创建目录
     */
    virtual void MakeDirectory(std::string path, SuccessCB cb) = 0;

    /**
     * 删除目录
     */
    virtual void RmDirectory(std::string path, SuccessCB cb) = 0;

    /**
     * 删除文件
     */
    virtual void DelFile(std::string path, SuccessCB cb) = 0;
protected:
    CFtpRequest();
    virtual ~CFtpRequest() = 0;
};

class CFtpClient {
public:
    /**
     * 创建一个ftp客户端环境
     * @param net 环境句柄
     */
    static CFtpClient* Create(CNet* net);
    ~CFtpClient();

    /**
     * 创建一个ftp客户端连接，并进行登陆
     * @param host 服务器地址
     * @param port 服务器端口
     * @param user 用户名
     * @param pwd  密码
     * @param onLogin ftp登陆成功后回调
     * @param onError 异常时的回调
     * @param usrData 回调中返回的CFtpRequest实例绑定的一个用户数据
     */
    virtual void Request(std::string host, int port, std::string user, std::string pwd, CFtpRequest::SuccessCB onLogin, CFtpRequest::ReqCB onError, void* usrData = NULL) = 0;
protected:
    CFtpClient();
};

class CFtpResponse
{

};

class CFtpServer
{
    typedef void(*ReqCb)(CFtpServer *server, CFtpMsg *request, CFtpResponse *response);
public:
    ReqCb OnRequest;

    CFtpServer* Create(CNet* net);
    /** 服务器启动监听 */
    virtual bool Listen(std::string strIP, uint32_t nPort) = 0;
    /** 服务器关闭 */
    virtual void Close() = 0;
protected:
    CFtpServer();
    virtual ~CFtpServer() = 0;
};
}
}

