#include "utilc.h"
#include "util.h"
#include "uvnetftp.h"
#include <time.h>

namespace uvNetPlus {
namespace Ftp {
    const char* szFtpCmd[] = {
        "ABOR", //�ж��������ӳ���
        "ACCT", //ϵͳ��Ȩ�ʺ�
        "ALLO", //Ϊ�������ϵ��ļ��洢�������ֽ�
        "APPE", //����ļ���������ͬ���ļ�
        "CDUP", //�ı�������ϵĸ�Ŀ¼
        "CWD",  //�ı�������ϵĹ���Ŀ¼
        "DELE", //ɾ���������ϵ�ָ���ļ�
        "HELP", //����ָ��������Ϣ
        "LIST", //������ļ����г��ļ���Ϣ�������Ŀ¼���г��ļ��б�
        "MODE", //����ģʽ��S=��ģʽ��B=��ģʽ��C=ѹ��ģʽ��
        "MKD",  //�ڷ������Ͻ���ָ��Ŀ¼
        "NLST", //�г�ָ��Ŀ¼����
        "NOOP", //�޶������������Է������ϵĳ���
        "PASS", //ϵͳ��¼����
        "PASV", //����������ȴ���������
        "PORT", //IP ��ַ�����ֽڵĶ˿� ID
        "PWD",  //��ʾ��ǰ����Ŀ¼
        "QUIT", //�� FTP ���������˳���¼
        "REIN", //���³�ʼ����¼״̬����
        "REST", //���ض�ƫ���������ļ�����
        "RETR", //�ӷ��������һأ����ƣ��ļ�
        "RMD",  //�ڷ�������ɾ��ָ��Ŀ¼
        "RNFR", //�Ծ�·��������
        "RNTO", //����·��������
        "SITE", //�ɷ������ṩ��վ���������
        "SMNT", //����ָ���ļ��ṹ
        "STAT", //�ڵ�ǰ�����Ŀ¼�Ϸ�����Ϣ
        "STOR", //���棨���ƣ��ļ�����������
        "STOU", //�����ļ���������������
        "STRU", //���ݽṹ��F=�ļ���R=��¼��P=ҳ�棩
        "SYST", //���ط�����ʹ�õĲ���ϵͳ
        "TYPE", //�������ͣ�A=ASCII��E=EBCDIC��I=binary��
        "USER", //ϵͳ��¼���û���
        "OPTS"
    };

    const char* szFtpFileType[] = {"A","E","I"};

    const char* szMon[] ={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

    //////////////////////////////////////////////////////////////////////////
    /** �������ӵĻص����� */

    static void OnDataConnect(CTcpSocket* skt, std::string error) {
        CUNFtpRequest *ftp = (CUNFtpRequest*)skt->userData;
        if(!error.empty()) {
            //����ʧ��
            ftp->m_ftpData.replyCode = -1;
            ftp->m_ftpData.replyStr = error;
            ftp->OnCB(ftp, &ftp->m_ftpData);
            return;
        }
    }

    static void OnDataRecv(CTcpSocket* skt, char *data, int len) {
        CUNFtpRequest *ftp = (CUNFtpRequest*)skt->userData;
        ftp->m_strRecvData.append(data, len);
    }

    static void OnDataDrain(CTcpSocket* skt) {
        CUNFtpRequest *ftp = (CUNFtpRequest*)skt->userData;
        ftp->m_tcpUserDTP->Delete();
        ftp->m_tcpUserDTP = NULL;
    }

    static void OnDataClose(CTcpSocket* skt) {
        CUNFtpRequest *ftp = (CUNFtpRequest*)skt->userData;
        ftp->m_tcpUserDTP->Delete();
        ftp->m_tcpUserDTP = NULL;

        ftp->FinishDataTrans();
    }

    static void OnDataError(CTcpSocket* skt, string err) {
        CUNFtpRequest *ftp = (CUNFtpRequest*)skt->userData;
        ftp->m_ftpData.replyCode = -1;
        ftp->m_ftpData.replyStr = err;
        ftp->OnCB(ftp, &ftp->m_ftpData);
    }

    //////////////////////////////////////////////////////////////////////////
    /** �������ӵĻص����� */

    static void OnClientConnect(CTcpSocket* skt, std::string error) {
        CUNFtpRequest *ftp = (CUNFtpRequest*)skt->userData;
        if(!error.empty()) {
            //����ʧ��
            ftp->m_ftpMsg.replyCode = -1;
            ftp->m_ftpMsg.replyStr = error;
            ftp->OnCB(ftp, &ftp->m_ftpMsg);
            return;
        }
        ftp->m_tcpUserPI->GetLocal(ftp->m_strPiHost, ftp->m_nPiPort);
        ftp->m_nDataPort = ftp->m_nPiPort + 1;
    }

    static void OnClientRecv(CTcpSocket* skt, char *data, int len){
        CUNFtpRequest* req = (CUNFtpRequest*)skt->userData;
        req->DoReceive(data, len);
    }

    static void OnClientDrain(CTcpSocket* skt){
        //Log::debug("client drain");
        CUNFtpRequest* req = (CUNFtpRequest*)skt->userData;
        req->DoDrain();
    }

    static void OnClientClose(CTcpSocket* skt){
        //Log::debug("client close");
        CUNFtpRequest* req = (CUNFtpRequest*)skt->userData;
    }

    static void OnClientError(CTcpSocket* skt, string err){
        Log::error("client error: %s ", err.c_str());
        CUNFtpRequest* req = (CUNFtpRequest*)skt->userData;
        req->DoError(err);
    }


    void CFtpMsg::Init(FTP_CMD c, std::string p) {
        cmd = c;
        cmdParam = p;
        if(p.empty()) {
            cmdStr = std::string(szFtpCmd[cmd]) + "\r\n";
        } else {
            cmdStr = std::string(szFtpCmd[cmd]) + " " + p + "\r\n";
        }
        replyCode = 0;
        replyStr = "";
    }

    //////////////////////////////////////////////////////////////////////////
    CFtpRequest::CFtpRequest()
        : port(21)
        , usrData(NULL)
        , path("/")
        , dataMod(FTP_DATA_MOD_PASV)
    {}

    CFtpRequest::~CFtpRequest() {}

    CUNFtpRequest::CUNFtpRequest()
        : m_pNet(NULL)
        , m_tcpUserPI(NULL)
        , m_tcpUserDTP(NULL)
        , m_dtpUploadData(NULL)
        , m_dtpUploadSize(0)
        , m_nDataPort(20)
        , m_bLoginCb(false)
        , OnCB(NULL)
        , OnSuccess(NULL)
        , OnNameListCB(NULL)
        , OnListCB(NULL)
    {
    }

    CUNFtpRequest::~CUNFtpRequest() {
    }

    /** ɾ��ʵ�� */
    void CUNFtpRequest::Delete() {
        delete this;
    }

    /**
     * ��ȡ��ǰ����Ŀ¼
     */
    void CUNFtpRequest::GetWorkingDirectory(SuccessCB cb) {
        OnSuccess = cb;
        m_ftpMsg.Init(FTP_CMD::FTP_CMD_PWD, ""); //��ȡ��ǰĿ¼
        Log::info("%s %s", szFtpCmd[m_ftpMsg.cmd], m_ftpMsg.cmdParam.c_str());
        m_tcpUserPI->Send(m_ftpMsg.cmdStr.c_str(), m_ftpMsg.cmdStr.size());
    }

    /**
     * �ı�������ϵĹ���Ŀ¼CWD
     */
    void CUNFtpRequest::ChangeWorkingDirectory(std::string p, SuccessCB cb) {
        OnSuccess = cb;
        m_ftpMsg.Init(FTP_CMD::FTP_CMD_CWD, p);
        Log::info("%s %s", szFtpCmd[m_ftpMsg.cmd], m_ftpMsg.cmdParam.c_str());
        m_tcpUserPI->Send(m_ftpMsg.cmdStr.c_str(), m_ftpMsg.cmdStr.size());
    }

    /**
     * �л��ļ�����
     */
    void CUNFtpRequest::SetFileType(FTP_FILE_TYPE t, SuccessCB cb) {
        OnSuccess = cb;
        m_ftpMsg.Init(FTP_CMD::FTP_CMD_TYPE, szFtpFileType[t]);
        Log::info("%s %s", szFtpCmd[m_ftpMsg.cmd], m_ftpMsg.cmdParam.c_str());
        m_tcpUserPI->Send(m_ftpMsg.cmdStr.c_str(), m_ftpMsg.cmdStr.size());
    }

    /**
     * ��ȡ�������ļ������б�NLST
     */
    void CUNFtpRequest::NameList(NameListCB cb) {
        OnNameListCB = cb;
        m_ftpData.Init(FTP_CMD::FTP_CMD_NLST, "");
        BeginDataTrans();
    }

    /**
     * ��ȡ�ļ���Ϣ���ļ��б�LIST
     */
    void CUNFtpRequest::List(ListCB cb) {
        OnListCB = cb;
        m_ftpData.Init(FTP_CMD::FTP_CMD_LIST, "");
        BeginDataTrans();
    }

    /**
     * �����ļ�
     */
    void CUNFtpRequest::Download(string file, DownloadCB cb) {
        OnDownloadCB = cb;
        m_ftpData.Init(FTP_CMD::FTP_CMD_RETR, file);
        BeginDataTrans();
    }

    /**
     * �ϴ��ļ�
     */
    void CUNFtpRequest::Upload(string file, char *data, int size, SuccessCB cb) {
        OnSuccess = cb;
        m_dtpUploadData = data;
        m_dtpUploadSize = size;
        m_ftpData.Init(FTP_CMD::FTP_CMD_STOR, file);
        BeginDataTrans();
    }

    /**
     * ����Ŀ¼
     */
    void CUNFtpRequest::MakeDirectory(std::string path, SuccessCB cb) {
        OnSuccess = cb;
        m_ftpMsg.Init(FTP_CMD::FTP_CMD_MKD, path); //����Ŀ¼
        Log::info("%s %s", szFtpCmd[m_ftpMsg.cmd], m_ftpMsg.cmdParam.c_str());
        m_tcpUserPI->Send(m_ftpMsg.cmdStr.c_str(), m_ftpMsg.cmdStr.size());
    }

    /**
     * ɾ��Ŀ¼
     */
    void CUNFtpRequest::RmDirectory(std::string path, SuccessCB cb) {
        OnSuccess = cb;
        m_ftpMsg.Init(FTP_CMD::FTP_CMD_RMD, path); //ɾ��Ŀ¼
        Log::info("%s %s", szFtpCmd[m_ftpMsg.cmd], m_ftpMsg.cmdParam.c_str());
        m_tcpUserPI->Send(m_ftpMsg.cmdStr.c_str(), m_ftpMsg.cmdStr.size());
    }

    /**
     * ɾ���ļ�
     */
    void CUNFtpRequest::DelFile(std::string path, SuccessCB cb) {
        OnSuccess = cb;
        m_ftpMsg.Init(FTP_CMD::FTP_CMD_DELE, path); //ɾ���ļ�
        Log::info("%s %s", szFtpCmd[m_ftpMsg.cmd], m_ftpMsg.cmdParam.c_str());
        m_tcpUserPI->Send(m_ftpMsg.cmdStr.c_str(), m_ftpMsg.cmdStr.size());
    }

    /** �������ݴ��� */
    void CUNFtpRequest::BeginDataTrans() {
        if(dataMod == FTP_DATA_MOD_PASV) {
            // ��������ģʽ
            m_ftpMsg.Init(FTP_CMD::FTP_CMD_PASV, "");
        } else {
            //��������ģʽ
            m_ftpMsg.Init(FTP_CMD::FTP_CMD_PORT, "");
        }
        Log::info("%s %s", szFtpCmd[m_ftpMsg.cmd], m_ftpMsg.cmdParam.c_str());
        m_tcpUserPI->Send(m_ftpMsg.cmdStr.c_str(), m_ftpMsg.cmdStr.size());
    }

    void CUNFtpRequest::FinishDataTrans() {
        if(m_ftpData.cmd == FTP_CMD::FTP_CMD_NLST) { //��ȡ�����б�
            vector<string> fileStrs = util::String::split(m_strRecvData, "\r\n");
            list<string> nameList;
            for(auto str:fileStrs) {
                nameList.push_back(str);
            }
            m_strRecvData.clear();
            OnNameListCB(this, nameList);
            return;
        } else if(m_ftpData.cmd == FTP_CMD::FTP_CMD_LIST) { //��ȡ�б�
            //Log::debug(m_strRecvData.c_str());
            list<CFtpFile> fileList;
            vector<string> fileStrs = util::String::split(m_strRecvData, "\r\n");
            for(auto str:fileStrs) {
                
                CFtpFile f;
                f.rawData = str;
                f.isDir = str[0]=='d';
                char szPermission[20]={0}, szMonth[5]={0}, szYearOrTime[20]={0}, szFileName[MAX_PATH]={0};
                uint64_t fileSize = 0;
                int owner=0, group=0, year=0, month=0, day = 0, hour=0, minute=0, second=0;
                sscanf(str.c_str(), "%[^ \t]%*[ \t]%*d%*[ \t]%d%*[ \t]%d%*[ \t]%llu%*[ \t]%[^ \t]%*[ \t]%d%*[ \t]%[^ \t]%*[ \t]%s"
                    ,szPermission, &owner, &group, &fileSize, szMonth, &day, szYearOrTime, szFileName);
                f.permission = szPermission;
                f.owner = to_string(owner);
                f.group = to_string(group);
                f.size = fileSize;
                for(int i = 0;i < 12; i++){
                    if(!strcasecmp(szMonth, szMon[i])){
                        month = i;//ȡ���ļ���Ϣ�е��·�
                        break;
                    }
                }
                if(strstr(szYearOrTime, ":")) { //����ʱ��
                    sscanf(szYearOrTime, "%d:%d:%d", &hour, &minute, &second);
                    //��ȡ��ǰʱ����·�
                    struct tm now = util::CTimeFormat::getTimeInfo(time(NULL));
                    if(now.tm_mon < month) {
                        year = now.tm_year - 1;
                    } else {
                        year = now.tm_year;
                    }
                } else { //��
                    year = atoi(szYearOrTime) - 1900;
                }
                struct tm fileTime;
                fileTime.tm_year = year;
                fileTime.tm_mon = month;
                fileTime.tm_mday = day;
                fileTime.tm_hour = hour;
                fileTime.tm_min = minute;
                fileTime.tm_sec = second;
                fileTime.tm_isdst = -1;
                f.dateTime = timegm(&fileTime); //FTP�ϵ�ʱ����UTCʱ�䡣ftp�������ǿ���ѡ����UTCʱ�仹�Ǳ���ʱ�䣬��UTC��ͨ�á�
                f.date = util::CTimeFormat::printTime(f.dateTime, "%Y-%m-%d %H:%M:%S");
                f.name = szFileName;
                fileList.push_back(f);
                /*int i=0, count = 0;
                bool curEmpty = true;
                size_t len = str.size();
                string strTmp;
                vector<string> vecSection;
                f.permission = str.substr(0, 10);
                for(i=10; i<len; ++i) {
                    if(str[i] == ' ' || str[i] == '\t' || str[i] == '\r' || str[i] == '\n') {
                        if(curEmpty) {
                            continue;
                        } else {
                            curEmpty = true;
                            if(!strTmp.empty()) {
                                if(count == 1) {
                                    f.owner = strTmp;
                                } else if(count == 2){
                                    f.group = strTmp;
                                } else if(count == 3) {
                                    f.size = stoll(strTmp);
                                } else if(count == 4) {

                                }
                                strTmp.clear();
                                ++count;
                            }
                        }
                    } else {
                        if(curEmpty) {

                        } else {

                        }
                    }
                }*/
            }
            m_strRecvData.clear();
            OnListCB(this, fileList);
            return;
        } else if(m_ftpData.cmd == FTP_CMD::FTP_CMD_RETR) { //�����ļ�
            OnDownloadCB(this, (char*)m_strRecvData.c_str(), m_strRecvData.size());
            m_strRecvData.clear();
            return;
        }
    }

    /* �յ������ݴ��� */
    void CUNFtpRequest::DoReceive(const char *data, int len) {
        if(data == NULL || len == 0)
            return;

        m_strRecvBuff.append(data, len);
        size_t endPos = m_strRecvBuff.find("\r\n");

        if(endPos == std::string::npos)
            return; //����δ���


        m_ftpMsg.replyStr = m_strRecvBuff.substr(0, endPos);
        m_strRecvBuff = m_strRecvBuff.substr(endPos+2, m_strRecvBuff.size()-endPos-2);        //�ص��Ѿ�����������

        if( m_ftpMsg.replyStr[0] < '0' || m_ftpMsg.replyStr[0] > '9'
         || m_ftpMsg.replyStr[1] < '0' || m_ftpMsg.replyStr[1] > '9'
         || m_ftpMsg.replyStr[2] < '0' || m_ftpMsg.replyStr[2] > '9'
         || m_ftpMsg.replyStr[3] != ' ') {
              Log::error(m_ftpMsg.replyStr.c_str());
              return;
        }
        m_ftpMsg.replyCode = stoi(m_ftpMsg.replyStr.substr(0,3));
        Log::debug(m_ftpMsg.replyStr.c_str());

        //220 ����ftp�������ɹ�Ӧ��  530 ������Ҫ���½
        if(m_ftpMsg.replyCode == 220 ||  m_ftpMsg.replyCode == 530) {
            m_ftpMsg.Init(FTP_CMD::FTP_CMD_USER, user);
            Log::info("%s %s", szFtpCmd[m_ftpMsg.cmd], m_ftpMsg.cmdParam.c_str());
            m_tcpUserPI->Send(m_ftpMsg.cmdStr.c_str(), m_ftpMsg.cmdStr.size());
            return;
        } 
        
        if(m_ftpMsg.cmd == FTP_CMD::FTP_CMD_USER) {
            if(m_ftpMsg.replyCode == 331) { //������Ҫ����������
                m_ftpMsg.Init(FTP_CMD::FTP_CMD_PASS, pwd);
                Log::info("%s %s", szFtpCmd[m_ftpMsg.cmd], m_ftpMsg.cmdParam.c_str());
                m_tcpUserPI->Send(m_ftpMsg.cmdStr.c_str(), m_ftpMsg.cmdStr.size());
                return;
            } 
        } else if(m_ftpMsg.cmd == FTP_CMD::FTP_CMD_PASS) {
            if(m_ftpMsg.replyCode == 230) { //��½�ɹ�
                m_ftpMsg.Init(FTP_CMD::FTP_CMD_OPTS, "UTF8 ON");
                Log::info("%s %s", szFtpCmd[m_ftpMsg.cmd], m_ftpMsg.cmdParam.c_str());
                m_tcpUserPI->Send(m_ftpMsg.cmdStr.c_str(), m_ftpMsg.cmdStr.size());
                return;
            } 
        } else if(m_ftpMsg.cmd == FTP_CMD::FTP_CMD_OPTS) {
            if(m_ftpMsg.replyCode == 200) { //UTF8ģʽ�ɹ�
                if(!m_bLoginCb) {
                    m_ftpMsg.Init(FTP_CMD::FTP_CMD_PWD, ""); //��ȡ��ǰĿ¼
                    Log::info("%s %s", szFtpCmd[m_ftpMsg.cmd], m_ftpMsg.cmdParam.c_str());
                    m_tcpUserPI->Send(m_ftpMsg.cmdStr.c_str(), m_ftpMsg.cmdStr.size());
                    return;
                }
            }
        } else if(m_ftpMsg.cmd == FTP_CMD::FTP_CMD_PWD) {
            if(m_ftpMsg.replyCode == 257) { //��ȡ��ǰĿ¼�ɹ�
                path = m_ftpMsg.replyStr.substr(4, m_ftpMsg.replyStr.size() - 4);
                if(path.size() >2 && path.at(0) == '"' && path.at(path.size()-1) == '"') {
                    path = path.substr(1, path.size()-2);
                }

                if(!m_bLoginCb) {
                    m_ftpMsg.Init(FTP_CMD::FTP_CMD_TYPE, "I"); //���ô�������
                    Log::info("%s %s", szFtpCmd[m_ftpMsg.cmd], m_ftpMsg.cmdParam.c_str());
                    m_tcpUserPI->Send(m_ftpMsg.cmdStr.c_str(), m_ftpMsg.cmdStr.size());
                    return;
                }

                OnSuccess(this);
            }
        } else if(m_ftpMsg.cmd == FTP_CMD::FTP_CMD_CWD) {
            if(m_ftpMsg.replyCode == 250) {
                OnSuccess(this);
            }
        } else if(m_ftpMsg.cmd == FTP_CMD::FTP_CMD_TYPE) {
            if(m_ftpMsg.replyCode == 200) {
                m_bLoginCb = true;
                OnSuccess(this);
            }
        } else if(m_ftpMsg.cmd == FTP_CMD::FTP_CMD_PASV) { // ����ģʽ�����յ��ɹ�Ӧ��user-DTP��������
            if(m_ftpMsg.replyCode == 227) { //���뱻������
                int ip1, ip2, ip3, ip4, port1, port2;
                int scanNum = sscanf(m_ftpMsg.replyStr.c_str(),"%*[^(](%d,%d,%d,%d,%d,%d)", &ip1, &ip2, &ip3, &ip4, &port1, &port2);
                if(scanNum != 6) {
                    Log::error(m_ftpMsg.replyStr.c_str());
                    return;
                }

                //�������ݴ�������, �������ڵ���PASVʱ��������
                Log::info("%s %s", szFtpCmd[m_ftpData.cmd], m_ftpData.cmdParam.c_str());
                m_tcpUserPI->Send(m_ftpData.cmdStr.c_str(), m_ftpData.cmdStr.size());

                //���𱻶�ģʽ����
                char dtpIp[20]={0};
                sprintf(dtpIp, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
                int dtpPort = port1*256 + port2;
                m_tcpUserDTP = (CUNTcpSocket*)CTcpSocket::Create(m_pNet, this, true);
                m_tcpUserDTP->copy       = false;
                m_tcpUserDTP->OnConnect  = OnDataConnect;
                m_tcpUserDTP->OnRecv     = OnDataRecv;
                m_tcpUserDTP->OnDrain    = OnDataDrain;
                m_tcpUserDTP->OnCLose    = OnDataClose;
                m_tcpUserDTP->OnError    = OnClientError;
                //m_tcpUserDTP->SetLocal(m_strPiHost, m_nDataPort);
                m_tcpUserDTP->Connect(dtpIp, dtpPort);
                return;
            }
            if(m_ftpData.cmd == FTP_CMD::FTP_CMD_NLST) { // ��ȡ�ļ��б���Ϣ
                if(m_ftpMsg.replyCode == 150) { //��ʼ����
                    return;
                } else if(m_ftpMsg.replyCode == 226) { //������ɣ���DTP����һ���Ѿ�������ɣ������DTP�Ͽ�ʱ�ص����׽��յ�����
                    return;
                }
            } else if(m_ftpData.cmd == FTP_CMD::FTP_CMD_LIST) { // ��ȡ�ļ��б���Ϣ
                if(m_ftpMsg.replyCode == 150) { //��ʼ����
                    return;
                } else if(m_ftpMsg.replyCode == 226) { //������ɣ���DTP����һ���Ѿ�������ɣ������DTP�Ͽ�ʱ�ص����׽��յ�����
                    return;
                }
            } else if(m_ftpData.cmd == FTP_CMD::FTP_CMD_STOR) { //�ϴ��ļ�
                if(m_ftpMsg.replyCode == 150) { // ���ݴ��俪ʼ
                    m_tcpUserDTP->Send(m_dtpUploadData, m_dtpUploadSize);
                    return;
                } else if(m_ftpMsg.replyCode == 226) { //�������
                    OnSuccess(this);
                    return;
                }
                return;
            } else if(m_ftpData.cmd == FTP_CMD::FTP_CMD_RETR) { // �����ļ�
                if(m_ftpMsg.replyCode == 150) { //��ʼ����
                    return;
                } else if(m_ftpMsg.replyCode == 226) { //������ɣ���DTP����һ���Ѿ�������ɣ������DTP�Ͽ�ʱ�ص����׽��յ�����
                    return;
                } else if(m_ftpMsg.replyCode == 550) { //���ļ�ʧ��
                    m_tcpUserDTP->Delete();
                    m_tcpUserDTP=NULL;
                }
            } 
        } else if(m_ftpMsg.cmd == FTP_CMD::FTP_CMD_MKD) { //����Ŀ¼
            if(m_ftpMsg.replyCode == 257) {
                OnSuccess(this);
                return;
            } else if(m_ftpMsg.replyCode == 550) { //����Ŀ¼ʧ��
            }
        } else if(m_ftpMsg.cmd == FTP_CMD::FTP_CMD_RMD) { //ɾ��Ŀ¼
            if(m_ftpMsg.replyCode == 250) {
                OnSuccess(this);
                return;
            } else if(m_ftpMsg.replyCode == 550) { //ɾ��Ŀ¼ʧ��
            }
        } else if(m_ftpMsg.cmd == FTP_CMD::FTP_CMD_DELE) { //ɾ���ļ�
            if(m_ftpMsg.replyCode == 250) {
                OnSuccess(this);
                return;
            } else if(m_ftpMsg.replyCode == 550) { //ɾ���ļ�ʧ��
            }
        }

        //����Ϊ�������Ļص�
        OnCB(this, &m_ftpMsg);
    }

    /** ���������� */
    void CUNFtpRequest::DoError(string err) {

    }

    /** �ͻ�������ȫ������ */
    void CUNFtpRequest::DoDrain() {

    }

    void CUNFtpRequest::DoDtp() {
        m_tcpUserPI->Send(m_ftpData.cmdStr.c_str(), m_ftpData.cmdStr.size());
    }

    //////////////////////////////////////////////////////////////////////////

    CFtpClient::CFtpClient()
    {
    }

    CFtpClient::~CFtpClient() {
    }

    CFtpClient* CFtpClient::Create(CNet* net)
    {
        CUNFtpClient* ftpClient = new CUNFtpClient();
        ftpClient->m_pNet = (CUVNetPlus *)net;
        return ftpClient;
    }

    void CUNFtpClient::Request(std::string host, int port, std::string user, std::string pwd, CFtpRequest::SuccessCB onLogin, CFtpRequest::ReqCB onError, void* usrData /*= NULL*/) {
        CUNFtpRequest *ftp = new CUNFtpRequest();
        ftp->m_pNet = m_pNet;
        ftp->host = host;
        ftp->port = port;
        ftp->user = user;
        ftp->pwd  = pwd;
        ftp->usrData = usrData;
        ftp->OnSuccess = onLogin;
        ftp->OnCB = onError;

        // user pi ���ӷ�����
        ftp->m_ftpMsg.cmd = FTP_CMD::FTP_CONN;
        ftp->m_tcpUserPI = (CUNTcpSocket*)CTcpSocket::Create(m_pNet, ftp, true);
        ftp->m_tcpUserPI->OnConnect  = OnClientConnect;
        ftp->m_tcpUserPI->OnRecv     = OnClientRecv;
        ftp->m_tcpUserPI->OnDrain    = OnClientDrain;
        ftp->m_tcpUserPI->OnCLose    = OnClientClose;
        ftp->m_tcpUserPI->OnError    = OnClientError;
        ftp->m_tcpUserPI->Connect(host, port);
    }
}
}