#pragma once
#include "uvnetpuclic.h"
#include <string>
#include <list>
#include <vector>
#include <unordered_map>
#include <map>
#include <stdint.h>

namespace uvNetPlus {

/** �¼�ѭ��eventloopִ���̣߳���װuv_loop */
class CNet
{
public:
    static CNet* Create();
    virtual ~CNet(){};
    virtual void* Loop() = 0;
protected:
    CNet(){};
};

//�����õ���������
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

/** TCP�ͻ��� */
class CTcpSocket
{
    typedef void (*EventCB)(CTcpSocket* skt);
    typedef void (*RecvCB)(CTcpSocket* skt, char *data, int len);
    typedef void (*ErrorCB)(CTcpSocket* skt, std::string error);
public:
    EventCB      OnReady;     //socket�������
    ErrorCB      OnConnect;   //�������
    RecvCB       OnRecv;      //�յ�����
    EventCB      OnDrain;     //���Ͷ���ȫ�����
    EventCB      OnCLose;     //socket�ر�
    EventCB      OnEnd;       //�յ��Է�fin,����eof
    EventCB      OnTimeout;   //��ʱ�ص�
    ErrorCB      OnError;     //����ص�

    bool         autoRecv;    //���ӽ������Ƿ������Զ��������ݡ�Ĭ��true
    bool         copy;        //���͵����ݿ�������ʱ����
    void        *userData;    //�û����Զ�������
    uint64_t     fd;          // SOCKET��ֵ(������)

    /**
     * ����һ��tcp���ӿͻ���ʵ��
     * @param net �������
     * @param usr �趨�û��Զ�������
     * @param copy ���÷��ͽӿ�ʱ���Ƿ����ݿ������������ڲ����й���
     */
    static CTcpSocket* Create(CNet* net, void *usr=nullptr, bool copy=true);

    /**
     * �첽ɾ�����ʵ��
     */
    virtual void Delete() = 0;

    /**
     * ���ӷ�������������ɺ����OnConnect�ص�
     */
    virtual void Connect(std::string strIP, uint32_t nPort) = 0;

    /**
     * ����socket�ı��ض˿ڣ������ָ��������ϵͳ�Զ�����
     * @param strIP ����IP������ָ������ʹ����һ���������ձ�ʾ��ָ����
     * @param nPort �����˿ڣ�0��ʾ��ָ��
     */
    virtual void SetLocal(std::string strIP, uint32_t nPort) = 0; 

    /**
     * ����һ��socket�󣬿��Ի�ȡ�䱾�ص�ַ
     */
    virtual void GetLocal(std::string &strIP, uint32_t &nPort) = 0;

    /**
     * �������ݡ������ݷŵ����ػ�������
     */
    virtual void Send(const char *pData, uint32_t nLen) = 0;

protected:
    CTcpSocket();
    virtual ~CTcpSocket() = 0;
};

/** TCP����� */
class CTcpServer
{
    typedef void (*EventCB)(CTcpServer* svr, std::string err);
    typedef void (*ConnCB)(CTcpServer* svr, std::string err, CTcpSocket* client);
public:

    EventCB          OnListen;       // ����������ɻص�������ʱ���״�����Ϣ
    ConnCB           OnConnection;   // �����ӻص�
    EventCB          OnClose;        // ����socket�ر���ɻص�
    EventCB          OnError;        // ��������ص�

    void            *userData;

    /**
     * ����һ��tcp�����ʵ��
     * @param net �������
     * @param onConnection ָ���յ�������ʱ�Ļص�
     * @param usr �趨�û��Զ�������
     */
    static CTcpServer* Create(CNet* net, ConnCB onConnection, void *usr=nullptr);

    /**
     * �첽ɾ����ǰʵ��
     */
    virtual void Delete() = 0;

    /**
     * ��������
     * @param strIP ����IP������ָ������ʹ����һ������
     * @param nPort ���ؼ����˿�
     */
    virtual bool Listen(std::string strIP, uint32_t nPort) = 0;

    /** �������Ƿ��ڼ������� */
    virtual bool Listening() = 0;
protected:
    CTcpServer();
    virtual ~CTcpServer() = 0;
};

/** �򵥵�TCP���ӳع��� */
class CTcpAgent
{
    typedef void (*EventCB)(CTcpAgent *agent, CTcpSocket *skt);
public:
    /** �������ӳ� */
    static CTcpAgent* Create(CNet* net);

    /** �����ӳط���һ��socket */
    virtual bool Put(CTcpSocket *skt) = 0;

    /** �����ӳ��Ƴ�һ��socket */
    virtual bool Remove(CTcpSocket *skt) = 0;

    /** �������ӳ� */
    virtual void Delete() = 0;

    EventCB    onTimeOut;   //��ʱ�ص�
    uint32_t   timeOut;     //�������ӳ�ʱʱ�� �� Ĭ��20s 0Ϊ������ʱ
protected:
    CTcpAgent();
    virtual ~CTcpAgent() = 0;
};

//////////////////////////////////////////////////////////////////////////

/** TCP���ӳ� ����ṹ */
struct CTcpRequest {
    std::string     host;   //����Ŀ��������ip
    uint32_t        port;   //����˿�
    std::string     localaddr; //����ip������ʹ����һ��������Ĭ�Ͽգ�������
    bool            copy;   //��Ҫ���͵������Ƿ񿽱����ڲ�ά��
    bool            recv;   //tcp�����Ƿ���Ҫ��������
    void           *usr;    //�û��Զ�������
    bool            autodel;//�ص����Զ�ɾ��������Ҫ�û��ֶ�ɾ����Ĭ��true��

    CTcpConnPool   *pool;   //����һ�����ӳػ�ȡ����
    CTcpRequest()
        : port(80)
        , copy(true)
        , recv(true)
        , usr(NULL)
        , autodel(true)
        , pool(NULL)
    {}
};

/** TCP�ͻ������ӳأ��Զ�������CTcpAgent */
class CTcpConnPool
{
    typedef void(*ErrorCB)(CTcpRequest *req, std::string error);
    typedef void (*ReqCB)(CTcpRequest* req, CTcpSocket* skt);
public:
    uint32_t   maxConns;    //��������� Ĭ��512(busy+idle)
    uint32_t   maxIdle;     //������������ Ĭ��100
    uint32_t   timeOut;     //�������ӳ�ʱʱ�� �� Ĭ��20s 0Ϊ������ʱ
    uint32_t   maxRequest;  //���Ӵﵽ���ʱ�ܴ�ŵ������� Ĭ��0 ������

    ErrorCB    OnError;     //��ȡ����ʧ�ܻص�
    ReqCB      OnRequest;   //��ȡTCP�ͻ������ӻص�

    /**
     * �������ӳ�
     * @param net loopʵ��
     * @param onReq ��ȡTCP�ͻ������ӻص�
     */
    static CTcpConnPool* Create(CNet* net, ReqCB onReq);

    /**
     * �첽ɾ�����ӳ�
     */
    virtual void Delete() = 0;

    /**
     * �����ӳػ�ȡһ��socket���ڲ�����Ķ�����Ҫ�û�ɾ��
     * @param host ����Ŀ��������˿�
     * @param port ����Ŀ��˿�
     * @param localaddr ����ip��ָ��������Ϊ�ձ�ʾ��ָ��
     * @param usr ��һ���û����ݣ��ص�ʱ��Ϊ�������
     * @param copy ���͵������Ƿ񿽱����ڲ�
     * @param recv �Ƿ���Ҫ����Ӧ��
     * @return �����µ�����ʵ��
     */
    virtual bool Request(std::string host, uint32_t port, std::string localaddr
        , void *usr=nullptr, bool copy=true, bool recv=true) = 0;

    /**
     * �����ӳػ�ȡһ��socket
     * @param req ��������ṹ
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
    bool aborted;   //������ֹʱ����Ϊtrue
    bool complete;  //http��Ϣ��������ʱ����Ϊtrue

    METHOD      method;     // ���󷽷�
    std::string path;        // ����·��

    int         statusCode;     //Ӧ��״̬��
    std::string statusMessage;  //Ӧ��״̬��Ϣ

    VERSION     version;        //http�汾�� 1.1��1.0
    std::string rawHeaders;     //������ͷ���ַ���
    std::string rawTrailers;    //������β���ַ���
    hash_list   headers;        //�����õ�httpͷ����ֵ��
    hash_list   trailers;       //�����õ�httpβ����ֵ��
    bool        keepAlive;      // �Ƿ�ʹ�ó�����, trueʱ��ʹ��CTcpConnPool��������
    bool        chunked;        // Transfer-Encoding: chunked
    uint32_t    contentLen;     // chunkedΪfalseʱ�����ݳ��ȣ�chunkedΪtrueʱ���鳤��
    std::string content;        // һ�εĽ�������

    CHttpMsg();
    ~CHttpMsg();
};

class CHttpConnect {
public:
    CHttpConnect();
    ~CHttpConnect();

    /**
     * ��ʾ��дhttpͷ�����ú���ʽhttpͷ�Ľӿھ���Ч��
     * @param headers httpͷ�������ַ���������ÿһ�н�β��"\r\n"
     */
    virtual void WriteHead(std::string headers);

    /**
     * ��ȡ�Ѿ��趨����ʽͷ
     */
    virtual std::vector<std::string> GetHeader(std::string name);

    /**
     * ��ȡ�����趨����ʽͷ��key
     */
    virtual std::vector<std::string> GetHeaderNames();

    /**
     * ��ʽͷ�Ƿ��Ѿ�����һ������
     */
    virtual bool HasHeader(std::string name);

    /**
     * �Ƴ�һ����ʽͷ
     */
    virtual void RemoveHeader(std::string name);

    /**
     * ��������һ��ͷ��ֵ����������һ����ʽͷ
     * param name field name
     * param value ����field value
     * param values ��NULL��β���ַ������飬���field value
     */
    virtual void SetHeader(std::string name, std::string value);
    virtual void SetHeader(std::string name, char **values);

    /**
     * �������ݳ��ȡ����ݷֶ�η��ͣ��Ҳ�ʹ��chunkedʱʹ�á�
     */
    virtual void SetContentLen(uint32_t len);

    /**
     * �鿴�Ƿ����
     */
    virtual bool Finished();

public:
     CTcpSocket        *tcpSocket;

protected:
    /** ����ʽͷ����ַ��� */
    std::string getImHeaderString();

protected:
    std::string         m_strHeaders;   // ��ʽ��ͷ
    hash_list           m_Headers;      // ��ʽͷ������
    bool                m_bHeadersSent; // header�Ƿ��Ѿ�����
    bool                m_bFinished;    // �����Ƿ����
    uint32_t            m_nContentLen;  // �������ݵĳ���
};

class CHttpRequest : public CHttpConnect {
    typedef void(*ErrorCB)(CHttpRequest *req, std::string error);
    typedef void(*ResCB)(CHttpRequest *req, CHttpMsg* response);
public:
    typedef void(*DrainCB)(CHttpRequest *req);

    PROTOCOL            protocol;  // Э��,http��https
    METHOD              method;    // ����
    std::string         path;      // ����·��
    VERSION             version;   // http�汾�� 1.0��1.1
    std::string         host;      // ������IP
    int                 port;      // �˿�
    std::string         localaddr; // ָ������IP��Ĭ��Ϊ��
    int                 localport; // ָ�����ض˿ڣ� Ĭ��Ϊ0��ֻ�к������������Ҫ���ã�����������Ҫ
    bool                keepAlive; // �Ƿ�ʹ�ó�����, trueʱ��ʹ��CTcpConnPool��������
    bool                chunked;   // Transfer-Encoding: chunked
    void               *usrData;   // �û��Զ�������
    bool                autodel;   // ������ɺ��Զ�ɾ��������Ҫ�ֶ��ͷš�
    uint64_t            fd;        // SOCKET��ֵ(������)


    /** �ͻ����յ�connect������Ӧ��ʱ�ص� */
    ResCB OnConnect;
    /** �ͻ����յ�1xxӦ��(101����)ʱ�ص� */
    ResCB OnInformation;
    /** �ͻ����յ�101 upgrade ʱ�ص� */
    ResCB OnUpgrade;
    /** �ͻ����յ�Ӧ��ʱ�ص������������ָ���ص����򲻻��ٴν������� */
    ResCB OnResponse;


    ErrorCB     OnError;        // ��������
    DrainCB     OnDrain;        // �����������

    /** ɾ��ʵ�� */
    virtual void Delete() = 0;

    /**
     * ��������һ�����ݣ����chunked=true������һ��chunk������
     * ���chunked=false��ʹ�����������η������ݣ������Լ�������ͷ������length
     * @param chunk ��Ҫ���͵�����
     * @param len ���͵����ݳ���
     * @param cb ����д�뻺������
     */
    virtual bool Write(const char* chunk, int len, DrainCB cb = NULL) = 0;

    /**
     * ���һ���������������δ���͵Ĳ������䷢�ͣ����chunked=true�����ⷢ�ͽ�����'0\r\n\r\n'
     * ���chunked=false,Э��ͷû�з��ͣ����Զ����length
     */
    virtual bool End() = 0;

    /**
     * �൱��Write(data, len, cb);end();
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
     * ����һ��http�ͻ��˻���
     * @param net �������
     * @param maxConns ͬһ����ַ���������
     * @param maxIdle ͬһ����ַ����������
     * @param timeOut �������ӳ�ʱʱ��
     * @param maxRequest ͬһ����ַ������󻺴�
     */
    CHttpClient(CNet* net, uint32_t maxConns=512, uint32_t maxIdle=100, uint32_t timeOut=20, uint32_t maxRequest=0);
    ~CHttpClient();
    bool Request(std::string host, int port, void* usr = NULL, ReqCB cb = NULL);

    /**
     * Ĭ�������ȡ�ɹ��ص����������Request������ָ���ص���������ʹ��ָ���Ļص�
     */
    ReqCB                OnRequest;
    CTcpConnPool        *connPool;
};

class CHttpResponse : public CHttpConnect {
public:
    typedef void(*ResCb)(CHttpResponse *response);

    bool                sendDate;      // Ĭ��true���ڷ���ͷʱ�Զ����Dateͷ(�Ѵ����򲻻����)
    int                 statusCode;    // ״̬��
    std::string         statusMessage; //�Զ����״̬��Ϣ�����Ϊ�գ�����ʱ��ȡ��׼��Ϣ
    VERSION             version;       // http�汾�� 1.0��1.1
    bool                keepAlive; // �Ƿ�ʹ�ó�����, trueʱ��ʹ��CTcpConnPool��������
    bool                chunked;   // Transfer-Encoding: chunked

    /** �������ǰ,socket�ж��˻�ص��÷��� */
    ResCb OnClose;
    /** Ӧ�������ʱ�ص����������ݶ��Ѿ����� */
    ResCb OnFinish;

    /**
     * ���һ��β������
     * @param key β�����ݵ�field name�����ֵ�Ѿ���header�е�Trailer�ﶨ����
     * @param value β�����ݵ�field value
     */
    virtual void AddTrailers(std::string key, std::string value) = 0;

    /**
     * Sends a HTTP/1.1 100 Continue message������write��end�Ĺ���
     */
    virtual void WriteContinue() = 0;

    /**
     * Sends a HTTP/1.1 102 Processing message to the client
     */
    virtual void WriteProcessing() = 0;

    /**
     * ��ʾ��дhttpͷ�����ú���ʽhttpͷ�Ľӿھ���Ч��
     * @param statusCode ��Ӧ״̬��
     * @param statusMessage �Զ���״̬��Ϣ������Ϊ�գ���ʹ�ñ�׼��Ϣ
     * @param headers httpͷ�������ַ�����ÿ�ж�Ҫ����"\r\n"
     */
    virtual void WriteHead(int statusCode, std::string statusMessage, std::string headers) = 0;

    /**
     * ��������˴˷�������û�е���writeHead()����ʹ����ʽͷ����������ͷ
     */
    virtual void Write(const char* chunk, int len, ResCb cb = NULL) = 0;

    /**
     * ����Ӧ����������ݶ��Ѿ����͡�ÿ��ʵ������Ҫ����һ��end��ִ�к�ᴥ��OnFinish
     */
    virtual void End() = 0;

    /**
     * �൱�ڵ���write(data, len, cb) ; end()
     */
    virtual void End(const char* data, int len, ResCb cb = NULL) = 0;

protected:
    CHttpResponse();
    virtual ~CHttpResponse() = 0;
};

class CHttpServer {
    typedef void(*ReqCb)(CHttpServer *server, CHttpMsg *request, CHttpResponse *response);
public:
    /** ���ܵ�һ������'Expect: 100-continue'������ʱ���ã����û��ָ�����Զ�����'100 Continue' */
    ReqCb OnCheckContinue;
    /** ���յ�һ������Expectͷ��������100������ʱ���ã����û��ָ�����Զ�����'417 Expectation Failed' */
    ReqCb OnCheckExpectation;
    /** �յ�upgrade����ʱ���� */
    ReqCb OnUpgrade;
    /** ���յ�һ���������������ָ���Ļص����Ͳ���������� */
    ReqCb OnRequest;

    /** ����һ��ʵ�� */
    static CHttpServer* Create(CNet* net);

    /** �豸�����ӱ���ʱ�䣬��������ʱ���û����������Ͽ����� */
    virtual void SetKeepAlive(uint32_t secends) = 0;
    /** �������������� */
    virtual bool Listen(std::string strIP, uint32_t nPort) = 0;
    /** �������ر� */
    virtual void Close() = 0;
    /** �������Ƿ��ڼ������� */
    virtual bool Listening() = 0;
protected:
    CHttpServer();
    virtual ~CHttpServer() = 0;
};
}; //namespace Http

//////////////////////////////////////////////////////////////////////////
namespace Ftp {
enum FTP_CMD {
    FTP_CMD_ABOR = 0, //�ж��������ӳ���
    FTP_CMD_ACCT, //ϵͳ��Ȩ�ʺ�
    FTP_CMD_ALLO, //Ϊ�������ϵ��ļ��洢�������ֽ�
    FTP_CMD_APPE, //����ļ���������ͬ���ļ�
    FTP_CMD_CDUP, //�ı�������ϵĸ�Ŀ¼
    FTP_CMD_CWD, //�ı�������ϵĹ���Ŀ¼
    FTP_CMD_DELE, //ɾ���������ϵ�ָ���ļ�
    FTP_CMD_HELP, //����ָ��������Ϣ
    FTP_CMD_LIST, //������ļ����г��ļ���Ϣ�������Ŀ¼���г��ļ��б�
    FTP_CMD_MODE, //����ģʽ��S=��ģʽ��B=��ģʽ��C=ѹ��ģʽ��
    FTP_CMD_MKD, //�ڷ������Ͻ���ָ��Ŀ¼
    FTP_CMD_NLST, //�г�ָ��Ŀ¼����
    FTP_CMD_NOOP, //�޶������������Է������ϵĳ���
    FTP_CMD_PASS, //ϵͳ��¼����
    FTP_CMD_PASV, //����������ȴ���������
    FTP_CMD_PORT, //IP ��ַ�����ֽڵĶ˿� ID
    FTP_CMD_PWD, //��ʾ��ǰ����Ŀ¼
    FTP_CMD_QUIT, //�� FTP ���������˳���¼
    FTP_CMD_REIN, //���³�ʼ����¼״̬����
    FTP_CMD_REST, //���ض�ƫ���������ļ�����
    FTP_CMD_RETR, //�ӷ��������һأ����ƣ��ļ�
    FTP_CMD_RMD, //�ڷ�������ɾ��ָ��Ŀ¼
    FTP_CMD_RNFR, //�Ծ�·��������
    FTP_CMD_RNTO, //����·��������
    FTP_CMD_SITE, //�ɷ������ṩ��վ���������
    FTP_CMD_SMNT, //����ָ���ļ��ṹ
    FTP_CMD_STAT, //�ڵ�ǰ�����Ŀ¼�Ϸ�����Ϣ
    FTP_CMD_STOR, //���棨���ƣ��ļ�����������
    FTP_CMD_STOU, //�����ļ���������������
    FTP_CMD_STRU, //���ݽṹ��F=�ļ���R=��¼��P=ҳ�棩
    FTP_CMD_SYST, //���ط�����ʹ�õĲ���ϵͳ
    FTP_CMD_TYPE, //�������ͣ�A=ASCII��E=EBCDIC��I=binary��
    FTP_CMD_USER, //ϵͳ��¼���û���
    FTP_CMD_OPTS, //����ѡ��
    FTP_CONN      //tcp����
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
    std::string rawData;    //���������ص����ݣ�û�н���
    bool        isDir;      //�Ƿ�ΪĿ¼
    std::string permission; //Ȩ���ַ���,10λ����һλd����Ŀ¼����������û����顢������rwxȨ��
    std::string owner;
    std::string group;
    uint64_t    size;
    std::string date;
    time_t      dateTime;
    std::string name;
};

struct CFtpMsg {
    FTP_CMD cmd;          //��������
    std::string cmdParam; //����
    std::string cmdStr;   //������������
    int replyCode;        //������������
    std::string replyStr; //������Ϣ����
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
    std::string         host;      // ������IP
    int                 port;      // �˿�
    std::string         user;      // �û���
    std::string         pwd;       // ����
    void               *usrData;   // �û��Զ�������
    std::string         path;      // ��ǰĿ¼
    FTP_DATA_MOD        dataMod;   // ����ģʽ��������ģʽ

    /** ɾ��ʵ�� */
    virtual void Delete() = 0;

    /**
     * ��ȡ��ǰ����Ŀ¼
     */
    virtual void GetWorkingDirectory(SuccessCB cb) = 0;

    /**
     * �ı�������ϵĹ���Ŀ¼CWD
     */
    virtual void ChangeWorkingDirectory(std::string path, SuccessCB cb) = 0;

    /**
     * �л��ļ�����
     */
    virtual void SetFileType(FTP_FILE_TYPE t, SuccessCB cb) = 0;

    /**
     * ��ȡ�������ļ������б�NLST
     */
    virtual void NameList(NameListCB cb) = 0;

    /**
     * ��ȡ�ļ���Ϣ���ļ��б�LIST
     */
    virtual void List(ListCB cb) = 0;

    /**
     * �����ļ�
     */
    virtual void Download(std::string file, DownloadCB cb) = 0;

    /**
     * �ϴ��ļ�
     */
    virtual void Upload(std::string file, char *data, int size, SuccessCB cb) = 0;

    /**
     * ����Ŀ¼
     */
    virtual void MakeDirectory(std::string path, SuccessCB cb) = 0;

    /**
     * ɾ��Ŀ¼
     */
    virtual void RmDirectory(std::string path, SuccessCB cb) = 0;

    /**
     * ɾ���ļ�
     */
    virtual void DelFile(std::string path, SuccessCB cb) = 0;
protected:
    CFtpRequest();
    virtual ~CFtpRequest() = 0;
};

class CFtpClient {
public:
    /**
     * ����һ��ftp�ͻ��˻���
     * @param net �������
     */
    static CFtpClient* Create(CNet* net);
    ~CFtpClient();

    /**
     * ����һ��ftp�ͻ������ӣ������е�½
     * @param host ��������ַ
     * @param port �������˿�
     * @param user �û���
     * @param pwd  ����
     * @param onLogin ftp��½�ɹ���ص�
     * @param onError �쳣ʱ�Ļص�
     * @param usrData �ص��з��ص�CFtpRequestʵ���󶨵�һ���û�����
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
    /** �������������� */
    virtual bool Listen(std::string strIP, uint32_t nPort) = 0;
    /** �������ر� */
    virtual void Close() = 0;
protected:
    CFtpServer();
    virtual ~CFtpServer() = 0;
};
}
}

