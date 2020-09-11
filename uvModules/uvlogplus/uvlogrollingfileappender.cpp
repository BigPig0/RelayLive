#include "uvlogprivate.h"
#include "uvlogutil.h"

namespace uvLogPlus {

    static void _stat_task_fs_cb(uv_fs_t* req) {
        uint64_t size = req->statbuf.st_size;
        RollingFileAppender* apd = (RollingFileAppender*)req->data;
        uv_fs_req_cleanup(req);
        delete req;
        apd->StatCB(size);
    }

    RollingFileAppender::RollingFileAppender() {
        max = 10;
        policies.size_policy.size = 1024*1024;
        policies.time_policy.interval = 0;
        policies.time_policy.modulate = false;
        opening = false;
        opened = false;
    }

    RollingFileAppender::~RollingFileAppender() {

    }

    void RollingFileAppender::Init(uv_loop_t *uv) {
        uv_loop = uv;
        if(!opened && !opening){
            CheckFile(append);
        }
    }

    void RollingFileAppender::CheckFile(bool append) {
        opening = true;
        //确定上级目录已被创建
        if(file_sys_check_path(file_name.c_str())){
            printf("create log file dir failed");
            opening = false;
            opened = true;
            return;
        }
        if(!append){
            //判断文件是否存在，如果存在就重命名
            if(file_sys_exist(file_name.c_str()) == 0) {
                RenameFile(1);
                std::string newName = file_name + ".1";
                uv_fs_t req;
                uv_fs_rename(NULL, &req, file_name.c_str(), newName.c_str(), NULL);
                uv_fs_req_cleanup(&req);
            }
        }

        uv_fs_t req;
        int ret = 0;
        int tag = UV_FS_O_RDWR | UV_FS_O_CREAT | UV_FS_O_APPEND;
        ret = uv_fs_open(NULL, &req, file_name.c_str(), tag, 666, NULL);
        if(ret < 0) {
            printf("fs open %s failed: %s\r\n", file_name.c_str(), uv_strerror(ret));
        } else {
            file_handle = req.result;
            opened = true;
        }
        opening = false;
    }

    void RollingFileAppender::WriteCB(){
        if(policies.size_policy.size > 0) {
            uv_fs_t * req = new uv_fs_t;
            req->data = this;
            uv_fs_fstat(uv_loop, req, file_handle, _stat_task_fs_cb);
        } else if(policies.time_policy.interval > 0){

        } else {
            //写下一条日志
            Write();
        }
    }

    void RollingFileAppender::StatCB(uint64_t size) {
        if(size >= policies.size_policy.size) {
            uv_fs_t req;
            uv_fs_close(NULL, &req, file_handle, NULL);
            uv_fs_req_cleanup(&req);
            opened = false;
            //重新打开新文件
            CheckFile(false);
        }
        //写下一条日志
        writing = false;
        Write();
    }

    /**
     * 将第apd个文件重命名为apd+1。
     * apd被成功清理则返回true，否则为false
     */
    bool RollingFileAppender::RenameFile(int apd) {
        std::string name = file_name + "." + std::to_string(apd);
        if(file_sys_exist(name.c_str()) != 0){
            return true;
        }

        std::string newName = file_name + "." + std::to_string(apd+1);
        bool remove = true;
        int ret = 0;
        if(file_sys_exist(newName.c_str()) == 0) {
            remove =false;
            if(apd+1 >= max){
                uv_fs_t req;
                ret = uv_fs_unlink(NULL, &req, newName.c_str(), NULL);
                uv_fs_req_cleanup(&req);
                remove = ret >= 0;
            } else {
                remove = RenameFile(apd+1);
            }
        }
        if(remove){
            uv_fs_t req;
            ret = uv_fs_rename(NULL, &req, name.c_str(), newName.c_str(), NULL);
            uv_fs_req_cleanup(&req);
        } else {
            uv_fs_t req;
            ret = uv_fs_unlink(NULL, &req, name.c_str(), NULL);
            uv_fs_req_cleanup(&req);
        }

        return ret>=0;
    }
}