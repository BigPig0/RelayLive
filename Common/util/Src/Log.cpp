#include "stdafx.h"
#include "File.h"
#include "Mutex.h"
#include <share.h>
#include <algorithm>
#include <sys\stat.h>
#include <thread>

class LogRunnable : public FindRunnable
{
public:
    explicit LogRunnable() : maxSeq(0) {}

    void find(const char *fileName) override
    {
        char *p = strchr((char *)fileName, '-');
        int seq = 0;
        if (p)
            seq = atoi(p + 1);
        if (seq > maxSeq)
            maxSeq = seq;

        if (seq <= 100)
        {
            time_t mtime = 0;
            struct stat statbuf; 
            int nRes = stat(fileName, &statbuf); 
            if(nRes == 0)
                mtime = statbuf.st_mtime;

            id_modifytime.push_back(make_pair(seq,mtime));
        }
    }

    int getIndex()
    {
        if(maxSeq < 100)
            return maxSeq+1;

        sort(id_modifytime.begin(),id_modifytime.end(),[](const pair<int,time_t>& p1,const pair<int,time_t>& p2)
        {
            if (p1.second > p2.second)
                return false;
            if (p1.second == p2.second && p1.first > p2.first)
                return false;
            return true;
        });
        if (id_modifytime.size()>0)
            return id_modifytime[0].first;

        return 1;
    }

private:
    int maxSeq;
    vector<pair<int,time_t>> id_modifytime;
};

namespace Log
{

const int LOG_SIZE = 4 * 1024;
const int BUFFER_SIZE = 64 * 1024;

struct Log
{
    CriticalSection csConsole, csFile;
    char        *fileBuffer;

    Print       print;
    Level       level;

    char        fileName[MAX_PATH], fileSuffix[10];
    FILE        *stdFilePtr;
    //bool        printTimeStamp, withLn;

    int         logSize, rotatedIndex;

    HANDLE      stderrHandle;
    WORD        oldColor;
};

static Log g_log;

bool open(Print print, Level level, const char *pathFileName)
{
    if ((int)print & (int)Print::file)
    {
        bool usePath = false;
        char path[MAX_PATH];
        strcpy_s(path, MAX_PATH, pathFileName);

        // 创建日志所在的目录(如果不存在)，如果存在同名文件则删掉再创建
        char *p = strchr(path, '\\');
        if (p)
        {
            usePath = true;
            char *q = path;
            do
            {
                *p = '\0';
                DWORD attr = GetFileAttributes(path);
                if ((attr == INVALID_FILE_ATTRIBUTES && GetLastError() == ERROR_FILE_NOT_FOUND) || !(attr & FILE_ATTRIBUTE_DIRECTORY))
                {
                    DeleteFile(path);
                    if (!CreateDirectory(path, NULL))
                    {
                        fprintf(stderr, "CreateDirectory(%s) failed:%d", path, GetLastError());
                        exit(1);
                    }
                }
                *p = '\\';
                q = p + 1;
                p = strchr(q, '\\');
            }
            while (p);
            p = strrchr(path, '\\');
            *p = '\0';
        }

        strcpy_s(g_log.fileName, MAX_PATH, pathFileName);
        p = strrchr(g_log.fileName, '.');
        *p = '\0';
        strcpy_s(g_log.fileSuffix, 10, ++p);

        LogRunnable runnable;
        if (usePath)
        {
            File logDir;
            char pat[MAX_PATH];
            char* name = strrchr(g_log.fileName, '\\');
            sprintf_s(pat, "%s-*.%s", ++name, g_log.fileSuffix);
            if (!logDir.findAll(path, pat, &runnable) && logDir.errorCode() != ERROR_FILE_NOT_FOUND)
            {
                fprintf(stderr, "findAll failed:%d\n", logDir.errorCode());
                return false;
            }
        }
        g_log.rotatedIndex = runnable.getIndex();

        char fileName[MAX_PATH];
        sprintf_s(fileName, MAX_PATH, "%s-%03d.%s", g_log.fileName, g_log.rotatedIndex, g_log.fileSuffix);

        g_log.stdFilePtr = _fsopen(fileName, "w+", _SH_DENYNO);
        if (!g_log.stdFilePtr)
        {
            fprintf(stderr, "_fsopen failed:%d\n", GetLastError());
            return false;
        }

        fseek(g_log.stdFilePtr, 0, SEEK_END);
        g_log.logSize = ftell(g_log.stdFilePtr);

        g_log.fileBuffer = (char *)malloc(BUFFER_SIZE);
        setvbuf(g_log.stdFilePtr, g_log.fileBuffer, _IOFBF, BUFFER_SIZE);
    }

    if ((int)print & (int)Print::console)
    {
        g_log.stderrHandle = GetStdHandle(STD_ERROR_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO consoleinfo;
        GetConsoleScreenBufferInfo(g_log.stderrHandle, &consoleinfo);
        g_log.oldColor = consoleinfo.wAttributes;
    }

    g_log.print = print;
    g_log.level = level;
    return true;
}

void close()
{
    if (g_log.stdFilePtr)
    {
        fclose(g_log.stdFilePtr);
        g_log.stdFilePtr = NULL;
    }
    if (g_log.fileBuffer)
    {
        free(g_log.fileBuffer);
        g_log.fileBuffer = NULL;
    }
}

static char levelNote[] =
{
    0, 'D', 'W', 'E'
};

void print(Level level, const char *fmt, ...)
{
    if (level >= g_log.level)
    {
        va_list arg;
        va_start(arg, fmt);
        char* file = va_arg(arg,char*);   //文件名称
        int   line = va_arg(arg,int);     //代码行数
        char* func = va_arg(arg,char*);   //函数名称
        char text[LOG_SIZE];              //日志内容
        memset(text,0,LOG_SIZE);
        vsprintf_s(text, LOG_SIZE, fmt, arg);
        va_end(arg);

        if (((int)g_log.print & (int)Print::file) && g_log.stdFilePtr)
        {
            //文件名称截取最后50个字符
            string strFile = file;
            if(strFile.length() > 50)
                strFile = strFile.substr(strFile.length()-50,50);
            //函数名称截取前面50个字符
            string strFunc = func;
            if (strFunc.length() > 50)
                strFunc = strFunc.substr(0,50);
            // 当前时间
            SYSTEMTIME systime;
            GetLocalTime(&systime);

            char codeInfo[200]={0};           //代码位置信息
            sprintf_s(codeInfo, 200, "[%08d]%04d-%02d-%02d %02d:%02d:%02d\t%c\t%50s:%04d\t%50s:\t"
                      , GetCurrentThreadId()
                      , systime.wYear, systime.wMonth, systime.wDay
                      , systime.wHour, systime.wMinute, systime.wSecond
                      , levelNote[(int)level]
                      , strFile.c_str()
                      , line
                      , strFunc.c_str()
                     );

            g_log.csFile.lock();
            g_log.logSize += fprintf(g_log.stdFilePtr, "%s%s\n", codeInfo,text);
            fflush(g_log.stdFilePtr);
            if (g_log.logSize >= 10 * 1024 * 1024)
            {
                fclose(g_log.stdFilePtr);
                g_log.logSize = 0;

                char fileName[MAX_PATH];
                g_log.rotatedIndex++;
                if(g_log.rotatedIndex>100) g_log.rotatedIndex=1;
                sprintf_s(fileName, MAX_PATH, "%s-%03d.%s", g_log.fileName, g_log.rotatedIndex, g_log.fileSuffix);

                g_log.stdFilePtr = _fsopen(fileName, "w+", _SH_DENYNO);
                if (!g_log.stdFilePtr)
                {
                    fprintf(stderr, "_fsopen failed:%d\n", GetLastError());
                    exit(1);
                }
                setvbuf(g_log.stdFilePtr, g_log.fileBuffer, _IOFBF, BUFFER_SIZE);
            }
            g_log.csFile.unlock();
        }

        if ((int)g_log.print & (int)Print::console)
        {
            g_log.csConsole.lock();
            if (level > Level::debug)
            {
                if (level == Level::warning)
                    SetConsoleTextAttribute(g_log.stderrHandle, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
                else if (level == Level::error)
                    SetConsoleTextAttribute(g_log.stderrHandle, FOREGROUND_RED | FOREGROUND_INTENSITY);

                fprintf(stderr, "%s : %s\n", func, text);
                SetConsoleTextAttribute(g_log.stderrHandle, g_log.oldColor);
            }
            else
            {
                fprintf(stderr, "%s : %s\n", func, text);
            }
            g_log.csConsole.unlock();
        }
    }
}

}
