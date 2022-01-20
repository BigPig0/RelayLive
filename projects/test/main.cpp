// sever.cpp : 定义控制台应用程序的入口点。
//
#include "util.h"
#include "utilc.h"
#include "sha1.h"
#include "base64.h"
#include "easylog.h"

using namespace util;

int main(int argc, char* argv[])
{
    /** 创建日志文件 */
    char path[MAX_PATH];
#ifdef WINDOWS_IMPL
    sprintf(path, "./log/test/log.txt");
#else
    sprintf(path, "/var/log/test/log.txt");
#endif
    Log::open(Log::Print::both, uvLogPlus::Level::Debug, path);
    Log::debug("version: %s %s", __DATE__, __TIME__);

    SHA1 sha1;
    string tmp = "i9OOwyfnrV3D9ElE19JEYQ==258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    tmp = (char*)sha1.Comput((uint8_t*)tmp.c_str(), tmp.size());
    Log::debug(tmp.c_str());
    tmp = Base64::Encode((uint8_t*)tmp.c_str(), tmp.size());
    Log::debug(tmp.c_str());

    util_sleep2(INFINITE);
    return 0;
}