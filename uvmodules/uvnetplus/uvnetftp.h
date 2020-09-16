/**
 * FTP（FILE TRANSFER PROTOCOL）：文件传输协议
 * PI（protocol interpreter）：协议解析器 用户和服务器用其来解析协议，它们的具体实现分别称为用户 PI （USER-PI）和服务器PI（SERVER-PI）
 * 服务器PI（server-PI）：服务器 PI 在 L 端口“监听”用户协议解析器的连接请求并建立控制连接。它从用户 PI接收标准的 FTP 命令，发送响应，并管理服务器 DTP
 * 服务器DTP（server-DTP）：数据传输过程，在通常的“主动”状态下是用“监听”的数据端口建立数据连接。它建立传输和存储参数，并在服务器端 PI 的命令下传输数据。服务器端 DTP 也可以用于“被动”模式，而不是主动在数据端口建立连接。
 * 用户PI（user-PI）：用户协议解析器用 U 端口建立到服务器 FTP 过程的控制连接，并在文件传输时管理用户 DTP。
 * 用户DTP（user-DTP）：数据传输过程在数据端口“监听”服务器 FTP 过程的连接。
 * 控制连接：用户PI 与服务器PI 用来交换命令和响应的信息传输通道。
 * 数据连接：通过控制连接协商的模式和类型进行数据传输。
 */
#pragma once
#include "uvnetpuclic.h"
#include "uvnettcp.h"
#include <string>
#include <stdint.h>

namespace uvNetPlus {
namespace Ftp {
class CUNFtpRequest : public CFtpRequest 
{
public:
    CUNFtpRequest();

    ~CUNFtpRequest();

    /** 删除实例 */
    virtual void Delete();

    /**
     * 获取当前工作目录
     */
    virtual void GetWorkingDirectory(SuccessCB cb);

    /**
     * 改变服务器上的工作目录CWD
     */
    virtual void ChangeWorkingDirectory(std::string path, SuccessCB cb);

    /**
     * 切换文件类型
     */
    virtual void SetFileType(FTP_FILE_TYPE t, SuccessCB cb);

    /**
     * 获取服务器文件列表NLST
     */
    virtual void NameList(NameListCB cb);

    /**
     * 获取文件信息或文件列表LIST
     */
    virtual void List(ListCB cb);

    /**
     * 下载文件
     */
    virtual void Download(string file, DownloadCB cb);

    /**
     * 上传文件
     */
    virtual void Upload(string file, char *data, int size, SuccessCB cb);

    /**
     * 创建目录
     */
    virtual void MakeDirectory(std::string path, SuccessCB cb);

    /**
     * 删除目录
     */
    virtual void RmDirectory(std::string path, SuccessCB cb);

    /**
     * 删除文件
     */
    virtual void DelFile(std::string path, SuccessCB cb);

    /** 开启数据传输 */
    void BeginDataTrans();

    /** 完成数据传输 */
    void FinishDataTrans();

    /* User-Pi收到的应答数据处理 */
    void DoReceive(const char *data, int len);

    /** User-Pi连接发生错误处理 */
    void DoError(string err);

    /** User-Pi数据全部发送 */
    void DoDrain();

    /** DTP连接成功，发送传输控制命令 */
    void DoDtp();

public:
    CUVNetPlus          *m_pNet;         // 事件线程句柄
    CUNTcpSocket        *m_tcpUserPI;    // user-PI
    CFtpMsg              m_ftpMsg;       // 控制命令
    std::string          m_strRecvBuff;  // 接收数据缓存
    std::string          m_strPiHost;
    uint32_t             m_nPiPort;

    CUNTcpSocket        *m_tcpUserDTP;   // user-DTP
    CFtpMsg              m_ftpData;      // 传输命令
    char                *m_dtpUploadData;// DTP上传内容
    int                  m_dtpUploadSize;// DTP上传大小
    std::string          m_strRecvData;  // 接收数据缓存
    uint32_t             m_nDataPort;    // 数据传输端口

    bool                 m_bLoginCb;     // 是否已经调用的登陆回调，在未调用之前，有一些命令收到应答后自动发起新的命令来完成初始话，否则回调

    /** 异常回调 */
    ReqCB                OnCB;
    /** 登陆,上传文件 成功的回调 */
    SuccessCB            OnSuccess;
    /** 获取文件名称列表应答 */
    NameListCB           OnNameListCB;
    /** 获取文件列表应答 */
    ListCB               OnListCB;
    /** 下载成功回调 */
    DownloadCB           OnDownloadCB;
};

class CUNFtpClient: public CFtpClient
{
public:
    CUVNetPlus       *m_pNet;      //事件线程句柄

    virtual void Request(std::string host, int port, std::string user, std::string pwd, CFtpRequest::SuccessCB onLogin, CFtpRequest::ReqCB onError, void* usrData = NULL);
};

}
}