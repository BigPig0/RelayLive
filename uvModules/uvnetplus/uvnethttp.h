#pragma once
#include "uvnetpuclic.h"
#include "uvnettcp.h"
#include <string>
#include <stdint.h>

namespace uvNetPlus {
namespace Http {

class CUNHttpRequest;
class CUNHttpResponse;
class CUNHttpServer;


/** HTTP客户端请求,用于客户端组织数据并发送 */
class CUNHttpRequest : public CHttpRequest
{
public:
    CUNHttpRequest(/*CTcpConnPool *pool*/);
    ~CUNHttpRequest();

    /** 删除实例 */
    virtual void Delete();

    /**
     * 用来发送一块数据，如果chunked=true，发送一个chunk的数据
     * 如果chunked=false，使用这个方法多次发送数据，必须自己在设置头里设置length
     * @param chunk 需要发送的数据
     * @param len 发送的数据长度
     * @param cb 数据写入缓存后调用
     */
    virtual bool Write(const char* chunk, int len, DrainCB cb = NULL);

    /**
     * 完成一个发送请求，如果有未发送的部分则将其发送，如果chunked=true，额外发送结束段'0\r\n\r\n'
     * 如果chunked=false,协议头没有发送，则自动添加length
     */
    virtual bool End();

    /**
     * 相当于Write(data, len, cb);end();
     * @remark Write和End方法只能直接调用一次，多次发送需要在回调中执行
     */
    virtual void End(const char* chunk, int len);

    /** 这几个Do函数是内部使用的 */
    /** 从连接池获取socket完成 */
    //void DoGetSocket(CTcpSocket *skt);

    /** 新建立的连接完成 */
    //void DoConnected(string err);

    /* 收到的数据处理 */
    void DoReceive(const char *data, int len);

    /** 发生错误处理 */
    void DoError(string err);

    /** 客户端数据全部发送 */
    void DoDrain();

private:
    std::string GetHeadersString();

    /** 解析http头，成功返回true，不是http头返回false */
    bool ParseHeader();
    /** 解析内容，已经接收完整内容或块返回true，否则false */
    bool ParseContent();

    CHttpMsg    *incMsg;        //解析出的应答数据
    bool                 parseHeader;   //请求报文中解析出http头。默认false

    uv_mutex_t           mutex;         //write和end线程安全
    std::string          recvBuff;      //接收数据缓存
};

/** 服务端生成应答数据并发送 */
class CUNHttpResponse : public CHttpResponse
{
public:
    CUNHttpResponse();
    ~CUNHttpResponse();

    /**
     * 添加一个尾部数据
     * @param key 尾部数据的field name，这个值已经在header中的Trailer里定义了
     * @param value 尾部数据的field value
     */
    virtual void AddTrailers(std::string key, std::string value);

    /**
     * Sends a HTTP/1.1 100 Continue message。包括write和end的功能
     */
    virtual void WriteContinue();

    /**
     * Sends a HTTP/1.1 102 Processing message to the client
     */
    virtual void WriteProcessing();

    /**
     * 显示填写http头，调用后隐式http头的接口就无效了
     * @param statusCode 响应状态码
     * @param statusMessage 自定义状态消息，可以为空，则使用标准消息
     * @param headers http头域完整字符串，每行都要包含"\r\n"
     */
    virtual void WriteHead(int statusCode, std::string statusMessage, std::string headers);

    /**
     * 如果调用了此方法，但没有调用writeHead()，则使用隐式头并立即发送头
     */
    virtual void Write(const char* chunk, int len, ResCb cb = NULL);

    /**
     * 表明应答的所有数据都已经发送。每个实例都需要调用一次end。执行后会触发OnFinish
     */
    virtual void End();

    /**
     * 相当于调用write(data, len, cb) ; end()
     */
    virtual void End(const char* data, int len, ResCb cb = NULL);

private:
    std::string GetHeadersString();

    hash_list   m_Trailers;
};


/** http服务端连接 */
class CSvrConn {
public:
    CSvrConn();
    /** 解析http头，成功返回true，不是http头返回false */
    bool ParseHeader();
    /** 解析内容，已经接收完整内容或块返回true，否则false */
    bool ParseContent();

    CUNHttpServer   *http;
    CTcpServer      *server;
    CTcpSocket      *client;
    std::string      buff;   //接收数据缓存
    CHttpMsg        *inc;    //保存解析到的请求数据
    CUNHttpResponse *res;    //应答
    bool             parseHeader;   //请求报文中解析出http头。默认false，请求完成后要重置为false。
};

/** http服务 */
class CUNHttpServer : public CHttpServer
{
public:
    CUNHttpServer(CNet* net);
    ~CUNHttpServer();

    /** 设备长连接保活时间，超过保活时间而没有新请求则断开连接 */
    virtual void SetKeepAlive(uint32_t secends);
    /** 服务器启动监听 */
    virtual bool Listen(std::string strIP, uint32_t nPort);
    /** 服务器关闭 */
    virtual void Close();
    /** 服务器是否在监听连接 */
    virtual bool Listening();

private:
    static void OnTimeOut(CTcpAgent *agent, CTcpSocket *skt);
    static void OnListen(CTcpServer* svr, std::string err);
    static void OnTcpConnection(CTcpServer* svr, std::string err, CTcpSocket* client);
    static void OnSvrCltRecv(CTcpSocket* skt, char *data, int len);
    static void OnSvrCltDrain(CTcpSocket* skt);
    static void OnSvrCltClose(CTcpSocket* skt);
    static void OnSvrCltEnd(CTcpSocket* skt);
    static void OnSvrCltError(CTcpSocket* skt, string err);

private:
    int           m_nPort;      //服务监听端口
    CTcpServer   *m_pTcpSvr;    //tcp监听服务
    CTcpAgent    *m_pAgent;     //socket连接池
#ifdef WIN32
    //std::unordered_multimap<std::string,CSvrConn*> m_pConns;   //所有连接的客户端请求
#else
    std::multimap<std::string,CSvrConn*> m_pConns;   //所有连接的客户端请求
#endif
};

};

}