/**
 * FTP��FILE TRANSFER PROTOCOL�����ļ�����Э��
 * PI��protocol interpreter����Э������� �û��ͷ���������������Э�飬���ǵľ���ʵ�ֱַ��Ϊ�û� PI ��USER-PI���ͷ�����PI��SERVER-PI��
 * ������PI��server-PI���������� PI �� L �˿ڡ��������û�Э����������������󲢽����������ӡ������û� PI���ձ�׼�� FTP ���������Ӧ������������� DTP
 * ������DTP��server-DTP�������ݴ�����̣���ͨ���ġ�������״̬�����á������������ݶ˿ڽ����������ӡ�����������ʹ洢���������ڷ������� PI �������´������ݡ��������� DTP Ҳ�������ڡ�������ģʽ�����������������ݶ˿ڽ������ӡ�
 * �û�PI��user-PI�����û�Э��������� U �˿ڽ����������� FTP ���̵Ŀ������ӣ������ļ�����ʱ�����û� DTP��
 * �û�DTP��user-DTP�������ݴ�����������ݶ˿ڡ������������� FTP ���̵����ӡ�
 * �������ӣ��û�PI �������PI ���������������Ӧ����Ϣ����ͨ����
 * �������ӣ�ͨ����������Э�̵�ģʽ�����ͽ������ݴ��䡣
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

    /** ɾ��ʵ�� */
    virtual void Delete();

    /**
     * ��ȡ��ǰ����Ŀ¼
     */
    virtual void GetWorkingDirectory(SuccessCB cb);

    /**
     * �ı�������ϵĹ���Ŀ¼CWD
     */
    virtual void ChangeWorkingDirectory(std::string path, SuccessCB cb);

    /**
     * �л��ļ�����
     */
    virtual void SetFileType(FTP_FILE_TYPE t, SuccessCB cb);

    /**
     * ��ȡ�������ļ��б�NLST
     */
    virtual void NameList(NameListCB cb);

    /**
     * ��ȡ�ļ���Ϣ���ļ��б�LIST
     */
    virtual void List(ListCB cb);

    /**
     * �����ļ�
     */
    virtual void Download(string file, DownloadCB cb);

    /**
     * �ϴ��ļ�
     */
    virtual void Upload(string file, char *data, int size, SuccessCB cb);

    /**
     * ����Ŀ¼
     */
    virtual void MakeDirectory(std::string path, SuccessCB cb);

    /**
     * ɾ��Ŀ¼
     */
    virtual void RmDirectory(std::string path, SuccessCB cb);

    /**
     * ɾ���ļ�
     */
    virtual void DelFile(std::string path, SuccessCB cb);

    /** �������ݴ��� */
    void BeginDataTrans();

    /** ������ݴ��� */
    void FinishDataTrans();

    /* User-Pi�յ���Ӧ�����ݴ��� */
    void DoReceive(const char *data, int len);

    /** User-Pi���ӷ��������� */
    void DoError(string err);

    /** User-Pi����ȫ������ */
    void DoDrain();

    /** DTP���ӳɹ������ʹ���������� */
    void DoDtp();

public:
    CUVNetPlus          *m_pNet;         // �¼��߳̾��
    CUNTcpSocket        *m_tcpUserPI;    // user-PI
    CFtpMsg              m_ftpMsg;       // ��������
    std::string          m_strRecvBuff;  // �������ݻ���
    std::string          m_strPiHost;
    uint32_t             m_nPiPort;

    CUNTcpSocket        *m_tcpUserDTP;   // user-DTP
    CFtpMsg              m_ftpData;      // ��������
    char                *m_dtpUploadData;// DTP�ϴ�����
    int                  m_dtpUploadSize;// DTP�ϴ���С
    std::string          m_strRecvData;  // �������ݻ���
    uint32_t             m_nDataPort;    // ���ݴ���˿�

    bool                 m_bLoginCb;     // �Ƿ��Ѿ����õĵ�½�ص�����δ����֮ǰ����һЩ�����յ�Ӧ����Զ������µ���������ɳ�ʼ��������ص�

    /** �쳣�ص� */
    ReqCB                OnCB;
    /** ��½,�ϴ��ļ� �ɹ��Ļص� */
    SuccessCB            OnSuccess;
    /** ��ȡ�ļ������б�Ӧ�� */
    NameListCB           OnNameListCB;
    /** ��ȡ�ļ��б�Ӧ�� */
    ListCB               OnListCB;
    /** ���سɹ��ص� */
    DownloadCB           OnDownloadCB;
};

class CUNFtpClient: public CFtpClient
{
public:
    CUVNetPlus       *m_pNet;      //�¼��߳̾��

    virtual void Request(std::string host, int port, std::string user, std::string pwd, CFtpRequest::SuccessCB onLogin, CFtpRequest::ReqCB onError, void* usrData = NULL);
};

}
}