#include "uvlogprivate.h"
#include "uv.h"

namespace uvLogPlus {

//字符颜色的格式为：\e[F;Bm
//其中"F"为字体颜色, 编号为30-37, "B"为背景颜色, 编号为40-47, 依次为 黑、红、绿、黄、蓝、紫红、青蓝、白。
//用 \e[m 结束颜色设置
const char *color_fatal = "\033[35;1;1m"; //紫色
const char *color_error = "\033[31;1;1m"; //红色
const char *color_warn  = "\033[33;1;1m"; //黄色
const char *color_info  = "\033[32;1;1m"; //绿色
const char *color_end   = "\033[0m";

static void _write_task_cb(uv_write_t* req, int status) {
    LogMsgReq *msg_req = (LogMsgReq*)req->data;
    ConsolAppender* apd = (ConsolAppender*)msg_req->appender;
    delete msg_req;
    delete req;

    //写下一条日志
    apd->Write();
}

ConsolAppender::ConsolAppender()
    : target(ConsolTarget::SYSTEM_OUT)
    , opening(false)
    , opened(false)
{
    type = AppenderType::consol;
}

ConsolAppender::~ConsolAppender() {

}

void ConsolAppender::Init(uv_loop_t *uv) {
    uv_loop = uv;
    if(!opened && !opening){
        int ret = 0;
        uv_file fd = 1; //stdout
        if(target == ConsolTarget::SYSTEM_ERR)
            fd = 2;
        opening = true;
        ret = uv_tty_init(uv, &tty_handle, fd, 0);
        if(ret < 0) {
            printf("tty init failed: %s\r\n", uv_strerror(ret));
        }
        opened = true;
    }
}

void ConsolAppender::Write() {
    if(opened){
        std::shared_ptr<LogMsg> item;
        bool found = msg_queue.try_dequeue(item);
        if(!found)
            return;
        
        LogMsgReq *msg_req = new LogMsgReq;
        msg_req->appender = this;
        msg_req->item = item;
        msg_req->buff = NULL;

        uv_write_t *req = new uv_write_t;
        req->data = msg_req;
        if(msg_req->item->level > Level::Debug){
            uv_buf_t buff[4];
            if(msg_req->item->level == Level::Fatal)
                buff[0] = uv_buf_init((char*)color_fatal, 9);
            else if(msg_req->item->level == Level::Error)
                buff[0] = uv_buf_init((char*)color_error, 9);
            else if(msg_req->item->level == Level::Warn)
                buff[0] = uv_buf_init((char*)color_warn, 9);
            else if(msg_req->item->level == Level::Info)
                buff[0] = uv_buf_init((char*)color_info, 9);
            buff[1] = uv_buf_init((char*)item->msg.c_str(), item->msg.size());
            buff[2] = uv_buf_init((char*)color_end, 4);
            buff[3] = uv_buf_init((char*)"\n", 1);
            uv_write(req, (uv_stream_t*)&tty_handle, buff, 4, _write_task_cb);
        } else {
            uv_buf_t buff[2];
            buff[0] = uv_buf_init((char*)item->msg.c_str(), item->msg.size());
            buff[1] = uv_buf_init((char*)"\n", 1);
            uv_write(req, (uv_stream_t*)&tty_handle, buff, 2, _write_task_cb);
        }
    }
}

}