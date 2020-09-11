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


/** HTTP�ͻ�������,���ڿͻ�����֯���ݲ����� */
class CUNHttpRequest : public CHttpRequest
{
public:
    CUNHttpRequest(/*CTcpConnPool *pool*/);
    ~CUNHttpRequest();

    /** ɾ��ʵ�� */
    virtual void Delete();

    /**
     * ��������һ�����ݣ����chunked=true������һ��chunk������
     * ���chunked=false��ʹ�����������η������ݣ������Լ�������ͷ������length
     * @param chunk ��Ҫ���͵�����
     * @param len ���͵����ݳ���
     * @param cb ����д�뻺������
     */
    virtual bool Write(const char* chunk, int len, DrainCB cb = NULL);

    /**
     * ���һ���������������δ���͵Ĳ������䷢�ͣ����chunked=true�����ⷢ�ͽ�����'0\r\n\r\n'
     * ���chunked=false,Э��ͷû�з��ͣ����Զ����length
     */
    virtual bool End();

    /**
     * �൱��Write(data, len, cb);end();
     * @remark Write��End����ֻ��ֱ�ӵ���һ�Σ���η�����Ҫ�ڻص���ִ��
     */
    virtual void End(const char* chunk, int len);

    /** �⼸��Do�������ڲ�ʹ�õ� */
    /** �����ӳػ�ȡsocket��� */
    //void DoGetSocket(CTcpSocket *skt);

    /** �½������������ */
    //void DoConnected(string err);

    /* �յ������ݴ��� */
    void DoReceive(const char *data, int len);

    /** ���������� */
    void DoError(string err);

    /** �ͻ�������ȫ������ */
    void DoDrain();

private:
    std::string GetHeadersString();

    /** ����httpͷ���ɹ�����true������httpͷ����false */
    bool ParseHeader();
    /** �������ݣ��Ѿ������������ݻ�鷵��true������false */
    bool ParseContent();

    CHttpMsg    *incMsg;        //��������Ӧ������
    bool                 parseHeader;   //�������н�����httpͷ��Ĭ��false

    uv_mutex_t           mutex;         //write��end�̰߳�ȫ
    std::string          recvBuff;      //�������ݻ���
};

/** ���������Ӧ�����ݲ����� */
class CUNHttpResponse : public CHttpResponse
{
public:
    CUNHttpResponse();
    ~CUNHttpResponse();

    /**
     * ���һ��β������
     * @param key β�����ݵ�field name�����ֵ�Ѿ���header�е�Trailer�ﶨ����
     * @param value β�����ݵ�field value
     */
    virtual void AddTrailers(std::string key, std::string value);

    /**
     * Sends a HTTP/1.1 100 Continue message������write��end�Ĺ���
     */
    virtual void WriteContinue();

    /**
     * Sends a HTTP/1.1 102 Processing message to the client
     */
    virtual void WriteProcessing();

    /**
     * ��ʾ��дhttpͷ�����ú���ʽhttpͷ�Ľӿھ���Ч��
     * @param statusCode ��Ӧ״̬��
     * @param statusMessage �Զ���״̬��Ϣ������Ϊ�գ���ʹ�ñ�׼��Ϣ
     * @param headers httpͷ�������ַ�����ÿ�ж�Ҫ����"\r\n"
     */
    virtual void WriteHead(int statusCode, std::string statusMessage, std::string headers);

    /**
     * ��������˴˷�������û�е���writeHead()����ʹ����ʽͷ����������ͷ
     */
    virtual void Write(const char* chunk, int len, ResCb cb = NULL);

    /**
     * ����Ӧ����������ݶ��Ѿ����͡�ÿ��ʵ������Ҫ����һ��end��ִ�к�ᴥ��OnFinish
     */
    virtual void End();

    /**
     * �൱�ڵ���write(data, len, cb) ; end()
     */
    virtual void End(const char* data, int len, ResCb cb = NULL);

private:
    std::string GetHeadersString();

    hash_list   m_Trailers;
};


/** http��������� */
class CSvrConn {
public:
    CSvrConn();
    /** ����httpͷ���ɹ�����true������httpͷ����false */
    bool ParseHeader();
    /** �������ݣ��Ѿ������������ݻ�鷵��true������false */
    bool ParseContent();

    CUNHttpServer   *http;
    CTcpServer      *server;
    CTcpSocket      *client;
    std::string      buff;   //�������ݻ���
    CHttpMsg        *inc;    //�������������������
    CUNHttpResponse *res;    //Ӧ��
    bool             parseHeader;   //�������н�����httpͷ��Ĭ��false��������ɺ�Ҫ����Ϊfalse��
};

/** http���� */
class CUNHttpServer : public CHttpServer
{
public:
    CUNHttpServer(CNet* net);
    ~CUNHttpServer();

    /** �豸�����ӱ���ʱ�䣬��������ʱ���û����������Ͽ����� */
    virtual void SetKeepAlive(uint32_t secends);
    /** �������������� */
    virtual bool Listen(std::string strIP, uint32_t nPort);
    /** �������ر� */
    virtual void Close();
    /** �������Ƿ��ڼ������� */
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
    int           m_nPort;      //��������˿�
    CTcpServer   *m_pTcpSvr;    //tcp��������
    CTcpAgent    *m_pAgent;     //socket���ӳ�
#ifdef WIN32
    //std::unordered_multimap<std::string,CSvrConn*> m_pConns;   //�������ӵĿͻ�������
#else
    std::multimap<std::string,CSvrConn*> m_pConns;   //�������ӵĿͻ�������
#endif
};

};

}