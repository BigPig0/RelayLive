#include "utilc.h"
#include "util.h"
#include "uvnetftp.h"
#include <time.h>

namespace uvNetPlus {
namespace Ftp {
    const char* szFtpCmd[] = {
        "ABOR", //中断数据连接程序
        "ACCT", //系统特权帐号
        "ALLO", //为服务器上的文件存储器分配字节
        "APPE", //添加文件到服务器同名文件
        "CDUP", //改变服务器上的父目录
        "CWD",  //改变服务器上的工作目录
        "DELE", //删除服务器上的指定文件
        "HELP", //返回指定命令信息
        "LIST", //如果是文件名列出文件信息，如果是目录则列出文件列表
        "MODE", //传输模式（S=流模式，B=块模式，C=压缩模式）
        "MKD",  //在服务器上建立指定目录
        "NLST", //列出指定目录内容
        "NOOP", //无动作，除了来自服务器上的承认
        "PASS", //系统登录密码
        "PASV", //请求服务器等待数据连接
        "PORT", //IP 地址和两字节的端口 ID
        "PWD",  //显示当前工作目录
        "QUIT", //从 FTP 服务器上退出登录
        "REIN", //重新初始化登录状态连接
        "REST", //由特定偏移量重启文件传递
        "RETR", //从服务器上找回（复制）文件
        "RMD",  //在服务器上删除指定目录
        "RNFR", //对旧路径重命名
        "RNTO", //对新路径重命名
        "SITE", //由服务器提供的站点特殊参数
        "SMNT", //挂载指定文件结构
        "STAT", //在当前程序或目录上返回信息
        "STOR", //储存（复制）文件到服务器上
        "STOU", //储存文件到服务器名称上
        "STRU", //数据结构（F=文件，R=记录，P=页面）
        "SYST", //返回服务器使用的操作系统
        "TYPE", //数据类型（A=ASCII，E=EBCDIC，I=binary）
        "USER", //系统登录的用户名
        "OPTS"
    };

    const char* szFtpFileType[] = {"A","E","I"};

    const char* szMon[] ={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

    //////////////////////////////////////////////////////////////////////////
    /** 数据连接的回调方法 */

    static void OnDataConnect(CTcpSocket* skt, std::string error) {
        CUNFtpRequest *ftp = (CUNFtpRequest*)skt->userData;
        if(!error.empty()) {
            //连接失败
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
    /** 控制连接的回调方法 */

    static void OnClientConnect(CTcpSocket* skt, std::string error) {
        CUNFtpRequest *ftp = (CUNFtpRequest*)skt->userData;
        if(!error.empty()) {
            //连接失败
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

    /** 删除实例 */
    void CUNFtpRequest::Delete() {
        delete this;
    }

    /**
     * 获取当前工作目录
     */
    void CUNFtpRequest::GetWorkingDirectory(SuccessCB cb) {
        OnSuccess = cb;
        m_ftpMsg.Init(FTP_CMD::FTP_CMD_PWD, ""); //获取当前目录
        Log::info("%s %s", szFtpCmd[m_ftpMsg.cmd], m_ftpMsg.cmdParam.c_str());
        m_tcpUserPI->Send(m_ftpMsg.cmdStr.c_str(), m_ftpMsg.cmdStr.size());
    }

    /**
     * 改变服务器上的工作目录CWD
     */
    void CUNFtpRequest::ChangeWorkingDirectory(std::string p, SuccessCB cb) {
        OnSuccess = cb;
        m_ftpMsg.Init(FTP_CMD::FTP_CMD_CWD, p);
        Log::info("%s %s", szFtpCmd[m_ftpMsg.cmd], m_ftpMsg.cmdParam.c_str());
        m_tcpUserPI->Send(m_ftpMsg.cmdStr.c_str(), m_ftpMsg.cmdStr.size());
    }

    /**
     * 切换文件类型
     */
    void CUNFtpRequest::SetFileType(FTP_FILE_TYPE t, SuccessCB cb) {
        OnSuccess = cb;
        m_ftpMsg.Init(FTP_CMD::FTP_CMD_TYPE, szFtpFileType[t]);
        Log::info("%s %s", szFtpCmd[m_ftpMsg.cmd], m_ftpMsg.cmdParam.c_str());
        m_tcpUserPI->Send(m_ftpMsg.cmdStr.c_str(), m_ftpMsg.cmdStr.size());
    }

    /**
     * 获取服务器文件名称列表NLST
     */
    void CUNFtpRequest::NameList(NameListCB cb) {
        OnNameListCB = cb;
        m_ftpData.Init(FTP_CMD::FTP_CMD_NLST, "");
        BeginDataTrans();
    }

    /**
     * 获取文件信息或文件列表LIST
     */
    void CUNFtpRequest::List(ListCB cb) {
        OnListCB = cb;
        m_ftpData.Init(FTP_CMD::FTP_CMD_LIST, "");
        BeginDataTrans();
    }

    /**
     * 下载文件
     */
    void CUNFtpRequest::Download(string file, DownloadCB cb) {
        OnDownloadCB = cb;
        m_ftpData.Init(FTP_CMD::FTP_CMD_RETR, file);
        BeginDataTrans();
    }

    /**
     * 上传文件
     */
    void CUNFtpRequest::Upload(string file, char *data, int size, SuccessCB cb) {
        OnSuccess = cb;
        m_dtpUploadData = data;
        m_dtpUploadSize = size;
        m_ftpData.Init(FTP_CMD::FTP_CMD_STOR, file);
        BeginDataTrans();
    }

    /**
     * 创建目录
     */
    void CUNFtpRequest::MakeDirectory(std::string path, SuccessCB cb) {
        OnSuccess = cb;
        m_ftpMsg.Init(FTP_CMD::FTP_CMD_MKD, path); //创建目录
        Log::info("%s %s", szFtpCmd[m_ftpMsg.cmd], m_ftpMsg.cmdParam.c_str());
        m_tcpUserPI->Send(m_ftpMsg.cmdStr.c_str(), m_ftpMsg.cmdStr.size());
    }

    /**
     * 删除目录
     */
    void CUNFtpRequest::RmDirectory(std::string path, SuccessCB cb) {
        OnSuccess = cb;
        m_ftpMsg.Init(FTP_CMD::FTP_CMD_RMD, path); //删除目录
        Log::info("%s %s", szFtpCmd[m_ftpMsg.cmd], m_ftpMsg.cmdParam.c_str());
        m_tcpUserPI->Send(m_ftpMsg.cmdStr.c_str(), m_ftpMsg.cmdStr.size());
    }

    /**
     * 删除文件
     */
    void CUNFtpRequest::DelFile(std::string path, SuccessCB cb) {
        OnSuccess = cb;
        m_ftpMsg.Init(FTP_CMD::FTP_CMD_DELE, path); //删除文件
        Log::info("%s %s", szFtpCmd[m_ftpMsg.cmd], m_ftpMsg.cmdParam.c_str());
        m_tcpUserPI->Send(m_ftpMsg.cmdStr.c_str(), m_ftpMsg.cmdStr.size());
    }

    /** 开启数据传输 */
    void CUNFtpRequest::BeginDataTrans() {
        if(dataMod == FTP_DATA_MOD_PASV) {
            // 开启被动模式
            m_ftpMsg.Init(FTP_CMD::FTP_CMD_PASV, "");
        } else {
            //开启主动模式
            m_ftpMsg.Init(FTP_CMD::FTP_CMD_PORT, "");
        }
        Log::info("%s %s", szFtpCmd[m_ftpMsg.cmd], m_ftpMsg.cmdParam.c_str());
        m_tcpUserPI->Send(m_ftpMsg.cmdStr.c_str(), m_ftpMsg.cmdStr.size());
    }

    void CUNFtpRequest::FinishDataTrans() {
        if(m_ftpData.cmd == FTP_CMD::FTP_CMD_NLST) { //获取名称列表
            vector<string> fileStrs = util::String::split(m_strRecvData, "\r\n");
            list<string> nameList;
            for(auto str:fileStrs) {
                nameList.push_back(str);
            }
            m_strRecvData.clear();
            OnNameListCB(this, nameList);
            return;
        } else if(m_ftpData.cmd == FTP_CMD::FTP_CMD_LIST) { //获取列表
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
                        month = i;//取得文件信息中的月份
                        break;
                    }
                }
                if(strstr(szYearOrTime, ":")) { //这是时间
                    sscanf(szYearOrTime, "%d:%d:%d", &hour, &minute, &second);
                    //获取当前时间的月份
                    struct tm now = util::CTimeFormat::getTimeInfo(time(NULL));
                    if(now.tm_mon < month) {
                        year = now.tm_year - 1;
                    } else {
                        year = now.tm_year;
                    }
                } else { //年
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
                f.dateTime = timegm(&fileTime); //FTP上的时间是UTC时间。ftp服务器是可以选择用UTC时间还是本地时间，但UTC更通用。
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
        } else if(m_ftpData.cmd == FTP_CMD::FTP_CMD_RETR) { //下载文件
            OnDownloadCB(this, (char*)m_strRecvData.c_str(), m_strRecvData.size());
            m_strRecvData.clear();
            return;
        }
    }

    /* 收到的数据处理 */
    void CUNFtpRequest::DoReceive(const char *data, int len) {
        if(data == NULL || len == 0)
            return;

        m_strRecvBuff.append(data, len);
        size_t endPos = m_strRecvBuff.find("\r\n");

        if(endPos == std::string::npos)
            return; //接收未完成


        m_ftpMsg.replyStr = m_strRecvBuff.substr(0, endPos);
        m_strRecvBuff = m_strRecvBuff.substr(endPos+2, m_strRecvBuff.size()-endPos-2);        //截掉已经解析的内容

        if( m_ftpMsg.replyStr[0] < '0' || m_ftpMsg.replyStr[0] > '9'
         || m_ftpMsg.replyStr[1] < '0' || m_ftpMsg.replyStr[1] > '9'
         || m_ftpMsg.replyStr[2] < '0' || m_ftpMsg.replyStr[2] > '9'
         || m_ftpMsg.replyStr[3] != ' ') {
              Log::error(m_ftpMsg.replyStr.c_str());
              return;
        }
        m_ftpMsg.replyCode = stoi(m_ftpMsg.replyStr.substr(0,3));
        Log::debug(m_ftpMsg.replyStr.c_str());

        //220 连接ftp服务器成功应答  530 服务器要求登陆
        if(m_ftpMsg.replyCode == 220 ||  m_ftpMsg.replyCode == 530) {
            m_ftpMsg.Init(FTP_CMD::FTP_CMD_USER, user);
            Log::info("%s %s", szFtpCmd[m_ftpMsg.cmd], m_ftpMsg.cmdParam.c_str());
            m_tcpUserPI->Send(m_ftpMsg.cmdStr.c_str(), m_ftpMsg.cmdStr.size());
            return;
        } 
        
        if(m_ftpMsg.cmd == FTP_CMD::FTP_CMD_USER) {
            if(m_ftpMsg.replyCode == 331) { //服务器要求输入密码
                m_ftpMsg.Init(FTP_CMD::FTP_CMD_PASS, pwd);
                Log::info("%s %s", szFtpCmd[m_ftpMsg.cmd], m_ftpMsg.cmdParam.c_str());
                m_tcpUserPI->Send(m_ftpMsg.cmdStr.c_str(), m_ftpMsg.cmdStr.size());
                return;
            } 
        } else if(m_ftpMsg.cmd == FTP_CMD::FTP_CMD_PASS) {
            if(m_ftpMsg.replyCode == 230) { //登陆成功
                m_ftpMsg.Init(FTP_CMD::FTP_CMD_OPTS, "UTF8 ON");
                Log::info("%s %s", szFtpCmd[m_ftpMsg.cmd], m_ftpMsg.cmdParam.c_str());
                m_tcpUserPI->Send(m_ftpMsg.cmdStr.c_str(), m_ftpMsg.cmdStr.size());
                return;
            } 
        } else if(m_ftpMsg.cmd == FTP_CMD::FTP_CMD_OPTS) {
            if(m_ftpMsg.replyCode == 200) { //UTF8模式成功
                if(!m_bLoginCb) {
                    m_ftpMsg.Init(FTP_CMD::FTP_CMD_PWD, ""); //获取当前目录
                    Log::info("%s %s", szFtpCmd[m_ftpMsg.cmd], m_ftpMsg.cmdParam.c_str());
                    m_tcpUserPI->Send(m_ftpMsg.cmdStr.c_str(), m_ftpMsg.cmdStr.size());
                    return;
                }
            }
        } else if(m_ftpMsg.cmd == FTP_CMD::FTP_CMD_PWD) {
            if(m_ftpMsg.replyCode == 257) { //获取当前目录成功
                path = m_ftpMsg.replyStr.substr(4, m_ftpMsg.replyStr.size() - 4);
                if(path.size() >2 && path.at(0) == '"' && path.at(path.size()-1) == '"') {
                    path = path.substr(1, path.size()-2);
                }

                if(!m_bLoginCb) {
                    m_ftpMsg.Init(FTP_CMD::FTP_CMD_TYPE, "I"); //设置传输类型
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
        } else if(m_ftpMsg.cmd == FTP_CMD::FTP_CMD_PASV) { // 被动模式请求收到成功应答，user-DTP发起连接
            if(m_ftpMsg.replyCode == 227) { //进入被动监听
                int ip1, ip2, ip3, ip4, port1, port2;
                int scanNum = sscanf(m_ftpMsg.replyStr.c_str(),"%*[^(](%d,%d,%d,%d,%d,%d)", &ip1, &ip2, &ip3, &ip4, &port1, &port2);
                if(scanNum != 6) {
                    Log::error(m_ftpMsg.replyStr.c_str());
                    return;
                }

                //发送数据传送命令, 该命令在调用PASV时缓存下载
                Log::info("%s %s", szFtpCmd[m_ftpData.cmd], m_ftpData.cmdParam.c_str());
                m_tcpUserPI->Send(m_ftpData.cmdStr.c_str(), m_ftpData.cmdStr.size());

                //发起被动模式连接
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
            if(m_ftpData.cmd == FTP_CMD::FTP_CMD_NLST) { // 获取文件列表信息
                if(m_ftpMsg.replyCode == 150) { //开始传输
                    return;
                } else if(m_ftpMsg.replyCode == 226) { //传输完成，但DTP并不一定已经接收完成，因此在DTP断开时回调上抛接收的数据
                    return;
                }
            } else if(m_ftpData.cmd == FTP_CMD::FTP_CMD_LIST) { // 获取文件列表信息
                if(m_ftpMsg.replyCode == 150) { //开始传输
                    return;
                } else if(m_ftpMsg.replyCode == 226) { //传输完成，但DTP并不一定已经接收完成，因此在DTP断开时回调上抛接收的数据
                    return;
                }
            } else if(m_ftpData.cmd == FTP_CMD::FTP_CMD_STOR) { //上传文件
                if(m_ftpMsg.replyCode == 150) { // 数据传输开始
                    m_tcpUserDTP->Send(m_dtpUploadData, m_dtpUploadSize);
                    return;
                } else if(m_ftpMsg.replyCode == 226) { //传输完成
                    OnSuccess(this);
                    return;
                }
                return;
            } else if(m_ftpData.cmd == FTP_CMD::FTP_CMD_RETR) { // 下载文件
                if(m_ftpMsg.replyCode == 150) { //开始传输
                    return;
                } else if(m_ftpMsg.replyCode == 226) { //传输完成，但DTP并不一定已经接收完成，因此在DTP断开时回调上抛接收的数据
                    return;
                } else if(m_ftpMsg.replyCode == 550) { //打开文件失败
                    m_tcpUserDTP->Delete();
                    m_tcpUserDTP=NULL;
                }
            } 
        } else if(m_ftpMsg.cmd == FTP_CMD::FTP_CMD_MKD) { //创建目录
            if(m_ftpMsg.replyCode == 257) {
                OnSuccess(this);
                return;
            } else if(m_ftpMsg.replyCode == 550) { //创建目录失败
            }
        } else if(m_ftpMsg.cmd == FTP_CMD::FTP_CMD_RMD) { //删除目录
            if(m_ftpMsg.replyCode == 250) {
                OnSuccess(this);
                return;
            } else if(m_ftpMsg.replyCode == 550) { //删除目录失败
            }
        } else if(m_ftpMsg.cmd == FTP_CMD::FTP_CMD_DELE) { //删除文件
            if(m_ftpMsg.replyCode == 250) {
                OnSuccess(this);
                return;
            } else if(m_ftpMsg.replyCode == 550) { //删除文件失败
            }
        }

        //其他为解析到的回调
        OnCB(this, &m_ftpMsg);
    }

    /** 发生错误处理 */
    void CUNFtpRequest::DoError(string err) {

    }

    /** 客户端数据全部发送 */
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

        // user pi 连接服务器
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