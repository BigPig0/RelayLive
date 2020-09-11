
#include "uv.h"
#include "cJSON.h"
#include "pm.h"
#include <thread>

class CPM : public CProcessMgr
{
public:
    CPM();
    ~CPM();

    virtual void Start();

    virtual void AddTask(Options opt);

    virtual void AddTasks(std::string path);

    virtual void Stop();

    virtual std::vector<Process> GetInfo();

    void Run();
    void Timer();
private:
    /** 定时器中执行进程守护任务 */
    void Protect();
    /** 查找进程是否存在 */
    bool Find(uint64_t pid);
    /** 强制结束一个进程 */
    bool Kill(uint64_t pid);
    /** 启动或重启动一个进程 */
    bool RunChild(Process* pro);
    /** 创建进程 */
    bool CreateChildProcess(string path, string args, uint64_t& pid);
    /** 进程到了重启时间 */
    bool NeedRestart(Process* pro, time_t now);

private:
    std::vector<Process*>   m_vecProcess;
    uv_mutex_t              m_mutex;
    uv_timer_t              m_timer;
    uv_loop_t               m_uvloop;

    bool                    m_bStart;
    bool                    m_bRunning; //loop线程是否在执行
    uint32_t                m_nNum;     //启动子进程的线程数
};

//////////////////////////////////////////////////////////////////////////

static void on_thread(void* arg) {
    CPM* pm = (CPM*)arg;
    pm->Run();
}

static void on_timer(uv_timer_t* handle) {
    CPM* pm = (CPM*)handle->data;
    pm->Timer();
}

//////////////////////////////////////////////////////////////////////////

CPM::CPM()
    : m_bStart(false)
    , m_bRunning(false)
{
    uv_mutex_init(&m_mutex);
    uv_loop_init(&m_uvloop);
    uv_timer_init(&m_uvloop, &m_timer);
    m_timer.data = this;
}

CPM::~CPM(){
    Stop();
    while (m_bRunning){
        Sleep(10);
    }
    uv_loop_close(&m_uvloop);
    uv_mutex_destroy(&m_mutex);
}

void CPM::Start() {
    if(m_bStart)
        return;

    m_bStart = true;
    uv_timer_start(&m_timer, on_timer, 1000, 1000);
    uv_thread_t tid;
    uv_thread_create(&tid, on_thread, this);
}

void CPM::AddTask(Options opt) {
    Process *p = new Process;
    p->pid = 0;
    p->startup = 0;
    p->running = false;
    p->needclose = false;
    p->path   = opt.path;
    p->args   = opt.args;
    p->daemon = opt.daemon;
    p->rstime = opt.rstime;
    p->rsdur  = opt.rsdur;
    p->exedir = opt.exedir;
    p->usrdir = opt.usrdir;
    uv_mutex_lock(&m_mutex);
    m_vecProcess.push_back(p);
    uv_mutex_unlock(&m_mutex);
}

void CPM::AddTasks(std::string path) {
    char buff[1024*500] = {0};
    FILE *f = fopen(path.c_str(), "r");
    if(f==NULL) {
        Log::debug("read config file failed");
        return;
    }
    fread(buff, 1, 1024*500, f);
    fclose(f);
    cJSON* jarray = cJSON_Parse(buff);
    if(NULL == jarray || jarray->type != cJSON_Array) {
        Log::debug("config file is error");
        cJSON_Delete(jarray);
        return;
    }
    int event_num = cJSON_GetArraySize(jarray);
    Log::debug("%d tasks", event_num);
    uv_mutex_lock(&m_mutex);
    for(int i=0; i<event_num; i++) {
        Process *p = new Process;
        p->pid = 0;
        p->startup = 0;
        p->running = false;
        p->needclose = false;
        p->exedir = 0;
        
        cJSON *js = cJSON_GetArrayItem(jarray, i);
        for (cJSON *attr = js->child; attr; attr = attr->next){
            if(!strcmp(attr->string, "path")){
                p->path = attr->valuestring;
            } else if(!strcmp(attr->string, "args")){
                p->args = attr->valuestring;
            } else if(!strcmp(attr->string, "daemon")){
                p->daemon = false;
                if(attr->type == cJSON_Number)
                    if(attr->valueint > 0)
                        p->daemon = true;
                if(attr->type == cJSON_String)
                    if(!strcmp(attr->valuestring, "yes"))
                        p->daemon = true;
            } else if(!strcmp(attr->string, "rstime")){
                p->rstime = attr->valuestring;
            } else if(!strcmp(attr->string, "rsdur")){
                p->rsdur = attr->valueint;
            } else if(!strcmp(attr->string, "remark")){
                p->remark = attr->valuestring;
            } else if(!strcmp(attr->string, "exedir")){
                p->exedir = attr->valueint;
            }  else if(!strcmp(attr->string, "usrdir")){
                p->usrdir = attr->valuestring;
            } 
        }
        m_vecProcess.push_back(p);
    }
    uv_mutex_unlock(&m_mutex);
    cJSON_Delete(jarray);
}

void CPM::Stop() {
    m_bStart = false;
    while (m_bRunning){
        Sleep(10);
    }
}

std::vector<CPM::Process> CPM::GetInfo() {
    std::vector<CPM::Process> ret;
    uv_mutex_lock(&m_mutex);
    for(auto& p:m_vecProcess) {
        ret.push_back(*p);
    }
    uv_mutex_unlock(&m_mutex);
    return ret;
}

void CPM::Run() {
    m_bRunning = true;
    while(m_bStart){
        uv_run(&m_uvloop, UV_RUN_DEFAULT);
        Sleep(10);
    }
    uv_mutex_lock(&m_mutex);
    for (auto& pProcess:m_vecProcess){
        if(pProcess->pid > 0){
            Kill(pProcess->pid);
        }
    }
    uv_mutex_unlock(&m_mutex);
    m_bRunning = false;
}

void CPM::Timer() {
    if(m_bStart) {
        Protect();
    } else {
        uv_timer_stop(&m_timer);
    }
}

void CPM::Protect() {
    struct tm timeinfo;
    time_t now = time(NULL);
    localtime_s(&timeinfo, &now);
    uv_mutex_lock(&m_mutex);
    m_nNum = 0;
    for (auto& pProcess:m_vecProcess) {
        if ( pProcess->pid == 0 ) { //< 尚未启动
            m_nNum++;
            Log::debug("start %s %s", pProcess->path.c_str(), pProcess->args.c_str());
            std::thread t([&](){
                RunChild(pProcess);
                m_nNum--;
            });
            t.detach();
        } else if(pProcess->daemon && !Find(pProcess->pid)) { //程序异常退出
            m_nNum++;
            Log::debug("protect %s %s", pProcess->path.c_str(), pProcess->args.c_str());
            std::thread t([&](){
                RunChild(pProcess);
                m_nNum--;
            });
            t.detach();
        } else if(NeedRestart(pProcess, now)) { //达到重启时间
            m_nNum++;
            Log::debug("restart 1 %s %s", pProcess->path.c_str(), pProcess->args.c_str());
            std::thread t([&](){
                if(Kill(pProcess->pid)) {
                    pProcess->pid = 0;
                    RunChild(pProcess);
                } else {
                    Log::error("kill process:%d failed[%s %s]", pProcess->pid, pProcess->path.c_str(), pProcess->args.c_str());
                }
                m_nNum--;
            });
            t.detach();
        } else if(pProcess->rsdur > 0 && difftime(now, pProcess->startup) > pProcess->rsdur) {
            //运行时间达到重启时长
            m_nNum++;
            Log::debug("restart 2 %s %s", pProcess->path.c_str(), pProcess->args.c_str());
            std::thread t([&](){
                if(Kill(pProcess->pid)) {
                    pProcess->pid = 0;
                    RunChild(pProcess);
                } else {
                    Log::error("kill process:%d failed[%s %s]", pProcess->pid, pProcess->path.c_str(), pProcess->args.c_str());
                }
                m_nNum--;
            });
            t.detach();
        }
    }
    uv_mutex_unlock(&m_mutex);
    while (m_nNum) {
        Sleep(10);
    }
}

bool CPM::Find(uint64_t pid) {
    HANDLE h = OpenProcess(PROCESS_ALL_ACCESS,FALSE,pid);
    if (NULL == h) {
        Log::debug("unfind process PID:%ld", pid);
        return false;
    }
    CloseHandle(h);
    return true;
}

bool CPM::Kill(uint64_t pid) {
    int i = 5;
    //Log::debug("begin kill PID:%ld",lPID);
    while(i--) {
        HANDLE h=OpenProcess(PROCESS_TERMINATE,FALSE,pid);
        if(NULL == h) {
            Log::debug("kill process %ld sucess", pid);
            return true;
        }
        BOOL ret = TerminateProcess(h,0);
        if(FALSE == ret) {
            DWORD dwError = GetLastError();
            Log::error("TerminateProcess failed:%d",dwError);
        } else {
            WaitForSingleObject(h, INFINITE);
            DWORD dwExitCode = 0;
            GetExitCodeProcess(h, &dwExitCode); 
            CloseHandle(h);
            i=0;
            Log::debug("kill process %ld sucess: %d", pid, dwExitCode);
            return true;
        }
        CloseHandle(h);
        Sleep(1000);
    }
    //Log::debug("end kill PID:%ld",lPID);
    //HANDLE h=OpenProcess(PROCESS_TERMINATE,FALSE,pid);
    //if(NULL == h) {
    //    Log::debug("kill process %ld sucess", pid);
    //    return true;
    //}
    //
    //CloseHandle(h);
    //Log::warning("process is still exist:%ld(handle:%d)", pid,h);
    return false;
}

bool CPM::RunChild(Process* pro) {
    //if(pro->pid != 0 && Find(pro->pid) && !Kill(pro->pid)) {
    //    Log::error("kill process:%d failed[%s %s]", pro->pid, pro->path.c_str(), pro->args.c_str());
    //    m_nNum--;
    //    return false;
    //}

    if(!CreateChildProcess(pro->path, pro->args, pro->pid)) {
        Log::error("restart process failed[%s %s]", pro->path.c_str(), pro->args.c_str());
        //m_nNum--;
        return false;
    }

    pro->startup = time(NULL);

    //m_nNum--;
    return true;
}

bool CPM::CreateChildProcess(string path, string args, uint64_t& pid) {
    //Log::debug("%s %s\n", path.c_str(), args.c_str());
    bool bRes = false;

    /** 创建管道，重定向子进程标准输入 */
    SECURITY_ATTRIBUTES sa;
    memset(&sa, 0, sizeof(sa));
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;

    HANDLE childStdInRead = INVALID_HANDLE_VALUE, 
        childStdInWrite = INVALID_HANDLE_VALUE;

    CreatePipe(&childStdInRead, &childStdInWrite, &sa, 0);
    SetHandleInformation(childStdInWrite, HANDLE_FLAG_INHERIT, 0);

    /** 创建子进程 */
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    si.hStdInput = childStdInRead;
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si.wShowWindow = SW_MINIMIZE;
    si.dwFlags |= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));
    pi.hProcess = INVALID_HANDLE_VALUE;
    pi.hThread = INVALID_HANDLE_VALUE;


    string strRun = path + " " + args;

    char szFullPath[MAX_PATH] = {0};
    char szDrive[_MAX_DRIVE];
    char szDir[_MAX_DIR];
    char szFileName[_MAX_FNAME];
    char szExt[_MAX_EXT];
    _splitpath(path.c_str(), szDrive, szDir, szFileName, szExt);
    sprintf(szFullPath, "%s%s", szDrive, szDir);
    //LogInfo(szFullPath);
    if (!CreateProcessA(NULL, (char*)strRun.c_str(), NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, szFullPath, &si, &pi)) {
        Log::error("CreateProcess failed:%d", GetLastError());
        goto end;
    }
    pid = pi.dwProcessId;
    bRes = true;
    WaitForInputIdle(pi.hProcess, 60000);
    //LogInfo(_T("CreateProcess sucess:%d"), lPID);
end:
    if(INVALID_HANDLE_VALUE != childStdInWrite)
        CloseHandle(childStdInWrite);
    if(INVALID_HANDLE_VALUE != pi.hProcess)
        CloseHandle(pi.hProcess);
    if(INVALID_HANDLE_VALUE != pi.hThread)
        CloseHandle(pi.hThread);

    if(bRes) {
        Log::debug("Run %s, PID:%ld success" ,strRun.c_str(),pi.dwProcessId);
    } else {
        Log::error("Run %s, PID:%ld failed",strRun.c_str(),pi.dwProcessId);
    }

    return bRes;
}

bool CPM::NeedRestart(Process* pro, time_t now) {
    vector<string> tms = util::String::split(pro->rstime, ":");
    if(tms.size() != 3)
        return false;
    uint32_t hour = stoi(tms[0]);
    uint32_t minute = stoi(tms[1]);
    uint32_t second = stoi(tms[2]);
    if(hour > 23 || minute > 59 || second > 59)
        return false;
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = second;
    time_t rstime = mktime(&timeinfo);

    //程序运行了多上时间
    double runtimespan = difftime(now, pro->startup);
    //过了重启时间点多久
    double rstimespan = difftime(now, rstime);
    if(now>rstime && runtimespan > rstimespan)
        return true;

    return false;
}

//////////////////////////////////////////////////////////////////////////

CProcessMgr::CProcessMgr(){}

CProcessMgr::~CProcessMgr(){}

CProcessMgr* CProcessMgr::Creat() {
    CPM *pm = new CPM();
    return pm;
}