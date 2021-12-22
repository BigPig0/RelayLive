#include "util.h"
#include "utilc.h"
#include "easylog.h"
#include "uv.h"

using namespace util;

uv_loop_t m_uvLoop;
uv_timer_t _timer;
std::string   _strSavePath;
uint64_t      _cleanTime;

static bool scanDayDir(string dir, int year, int month, int day) {
    bool ret = true; //路径为空

    //判断是否超出日期
    struct tm tm;
    tm.tm_year = year - 1900;
    tm.tm_mon  = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = 0;
    tm.tm_min  = 0;
    tm.tm_sec  = 0;
    tm.tm_isdst = -1;

    time_t dirTime = mktime(&tm);
    if(difftime(time(NULL), dirTime) < _cleanTime) {
        return false;
    }

    uv_fs_t req;
    uv_dirent_t dent;
    int sr = uv_fs_scandir(NULL, &req, dir.c_str(), 0, NULL);
    if(sr < 0) {
        return ret;
    }
    while (uv_fs_scandir_next(&req, &dent) != UV_EOF) {
        Log::debug("find file:%s", dent.name);
        // 跳过 . 文件
        if( strcmp(dent.name, ".") == 0 || 0 == strcmp(dent.name, "..") )
            continue;
        ret = false;

        if (dent.type & UV_DIRENT_FILE) {
            string subfile = dir + "/" + dent.name;
            //删除文件
            uv_fs_t *req = new uv_fs_t();
            uv_fs_unlink(NULL, req, subfile.c_str(), NULL);
            delete req;
            Log::debug("delete file: %s", subfile.c_str());
        }
    }
    uv_fs_req_cleanup(&req);

    return ret;
}

static bool scanMonthDir(string dir, int year, int month) {
    bool ret = true; //路径为空

    //判断是否超出日期
    struct tm tm;
    tm.tm_year = year - 1900;
    tm.tm_mon  = month - 1;
    tm.tm_mday = 1;
    tm.tm_hour = 0;
    tm.tm_min  = 0;
    tm.tm_sec  = 0;
    tm.tm_isdst = -1;

    time_t dirTime = mktime(&tm);
    if(difftime(time(NULL), dirTime) < _cleanTime) {
        return false;
    }

    uv_fs_t req;
    uv_dirent_t dent;
    int sr = uv_fs_scandir(NULL, &req, dir.c_str(), 0, NULL);
    if(sr < 0) {
        return ret;
    }
    while (uv_fs_scandir_next(&req, &dent) != UV_EOF) {
        Log::debug("find dir:%s", dent.name);
        // 跳过 . 文件
        if( strcmp(dent.name, ".") == 0 || 0 == strcmp(dent.name, "..") )
            continue;
        ret = false;

        if (dent.type & UV_DIRENT_DIR) {
            string subdir = dir + "/" + dent.name;
            bool empty = scanDayDir(subdir, year, month, stoi(dent.name));
            if(empty) {
                //删除天文件夹
                uv_fs_t *req = new uv_fs_t();
                uv_fs_rmdir(NULL, req, subdir.c_str(), NULL);
                delete req;
                Log::debug("delete dir: %s", subdir.c_str());
            }
        }
    }
    uv_fs_req_cleanup(&req);

    return ret;
}

static bool scanYearDir(string dir, int year) {
    bool ret = true; //路径为空

    //判断是否超出日期
    struct tm tm;
    tm.tm_year = year - 1900;
    tm.tm_mon  = 0;
    tm.tm_mday = 1;
    tm.tm_hour = 0;
    tm.tm_min  = 0;
    tm.tm_sec  = 0;
    tm.tm_isdst = -1;

    time_t dirTime = mktime(&tm);
    if(difftime(time(NULL), dirTime) < _cleanTime) {
        return false;
    }

    uv_fs_t req;
    uv_dirent_t dent;
    int sr = uv_fs_scandir(NULL, &req, dir.c_str(), 0, NULL);
    if(sr < 0) {
        return ret;
    }
    while (uv_fs_scandir_next(&req, &dent) != UV_EOF) {
        Log::debug("find dir:%s", dent.name);
        // 跳过 . 文件
        if( strcmp(dent.name, ".") == 0 || 0 == strcmp(dent.name, "..") )
            continue;
        ret = false;

        if (dent.type & UV_DIRENT_DIR) {
            string subdir = dir + "/" + dent.name;
            bool empty = scanMonthDir(subdir, year, stoi(dent.name));
            if(empty) {
                //删除月文件夹
                uv_fs_t *req = new uv_fs_t();
                uv_fs_rmdir(NULL, req, subdir.c_str(), NULL);
                delete req;
                Log::debug("delete dir: %s", subdir.c_str());
            }
        }
    }
    uv_fs_req_cleanup(&req);

    return ret;
}

static bool scanDir(string dir) {
    bool ret = true; //路径为空
    uv_fs_t req;
    uv_dirent_t dent;
    int sr = uv_fs_scandir(NULL, &req, dir.c_str(), 0, NULL);
    if(sr < 0) {
        return ret;
    }

    while (uv_fs_scandir_next(&req, &dent) != UV_EOF) {
        Log::debug("find dir:%s", dent.name);
        // 跳过 . 文件
        if( strcmp(dent.name, ".") == 0 || 0 == strcmp(dent.name, "..") )
            continue;
            ret = false;

        if (dent.type & UV_DIRENT_DIR) {
            string subdir = dir + "/" + dent.name;
            bool empty = scanYearDir(subdir, stoi(dent.name));
            if(empty) {
                //删除年文件夹
                uv_fs_t *req = new uv_fs_t();
                uv_fs_rmdir(NULL, req, subdir.c_str(), NULL);
                delete req;
                Log::debug("delete dir: %s", subdir.c_str());
            }
        }
    }
    uv_fs_req_cleanup(&req);

    return ret;
}

static void on_timer_cb(uv_timer_t* handle) {
    scanDir(_strSavePath + "/image");
    scanDir(_strSavePath + "/video");
}

static void run_loop_thread(void* arg) {
    while (true) {
        uv_run(&m_uvLoop, UV_RUN_DEFAULT);
        sleep(10);
    }
}

void CleanStart() {
    _strSavePath = Settings::getValue("Capture","save","/");
    _cleanTime   = Settings::getValue("Capture","clean", 365) * 24 * 60 * 60;

    uv_loop_init(&m_uvLoop);
    uv_timer_init(&m_uvLoop, &_timer);
    uv_timer_start(&_timer, on_timer_cb, 30000, 300000);

    uv_thread_t tid;
    uv_thread_create(&tid, run_loop_thread, NULL);
}