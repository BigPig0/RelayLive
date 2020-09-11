#include "util_log.h"
#include "uvlogplus.h"
#include "util_string.h"
#include <sstream>
#include <stdarg.h>

using namespace std;
using namespace uvLogPlus;

namespace Log {

const int LOG_SIZE = 4 * 1024;
static CLog *log = NULL;

bool open(Print print, Level level, const char *pathFileName) {
    string strFilePath = pathFileName;
    strFilePath = util::String::replace(strFilePath, "\\", "\\\\");
    stringstream ss;
    ss << "{\"configuration\":{"
        "\"appenders\":{";
    bool apd = false;
    if ((int)print & (int)Print::file) {
        ss << "\"RollingFile\":{\"name\":\"RollingFileAppd\",\"fileName\":\"" << strFilePath << "\", \"Policies\":{\"size\":\"10MB\",\"max\":100}}";
        apd = true;
    }

    if ((int)print & (int)Print::console) {
        if(apd)
            ss << ",";
        ss << "\"console\":{\"name\":\"ConsoleAppd\",\"target\":\"SYSTEM_OUT\"}";
    }

    ss << "},\"loggers\":{"
           "\"root\":{\"level\":\"DEBUG\",\"appender-ref\":{\"ref\":\"ConsoleAppd\"}"
           "},\"commonLog\":{\"level\":\"DEBUG\",";
    apd = false;
    if((int)print & (int)Print::console) {
        ss << "\"appender-ref\":{\"ref\":\"ConsoleAppd\"}";
        apd = true;
    }
    if((int)print & (int)Print::file) {
        if(apd)
            ss << ",";
        ss << "\"appender-ref\":{\"ref\":\"RollingFileAppd\"}";
    }
    ss <<"}}}}";

    log = CLog::Create(ss.str().c_str());
    return log!=NULL;
}

void close()
{
    if(log != NULL) {
        delete log;
        log = NULL;
    }
}

void print(Level level, const char *file, int line, const char *func, const char *fmt, ...)
{
    if(log != NULL) {
        uvLogPlus::Level lv = uvLogPlus::Level::Debug;
        if(level == Level::info)
            lv = uvLogPlus::Level::Info;
        else if(level == Level::warning)
            lv = uvLogPlus::Level::Warn;
        else if(level == Level::error)
            lv = uvLogPlus::Level::Error;

        size_t size = 4096;
        std::string buffer(size, '\0');
        char* buffer_p = const_cast<char*>(buffer.data());
        va_list arg;
        va_start(arg, fmt);
        vsnprintf(buffer_p, size, fmt, arg);
        va_end(arg);
        log->Write("commonLog", lv, file, line, func, buffer_p);
    }
}

}
