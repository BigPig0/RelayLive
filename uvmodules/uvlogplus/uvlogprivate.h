#pragma once
#include "uvlogpublic.h"
#include "lock_free/concurrentqueue.h"
#include "uv.h"
#include <string>
#include <list>
#include <unordered_map>
#include <memory>

namespace uvLogPlus {

    enum AppenderType {
        consol = 0,         //控制台
        file,               //文件
        rolling_file,       //文件大小到达指定尺寸的时候产生一个新的文件
    };

    enum ConsolTarget {
        SYSTEM_OUT = 0,
        SYSTEM_ERR
    };

    enum FilterMatch {
        ACCEPT = 0,     //接受
        NEUTRAL,        //中立
        DENY            //拒绝
    };

    /** 一条日志内容 */
    struct LogMsg {
        uint32_t    tid;            //线程ID
        Level       level;          //日志等级
        const char *file_name;      //所在文件名称
        const char *func_name;      //所在函数名称
        int         line;           //行号
        time_t      msg_time;       //产生时间
        std::string msg;            //内容
        LogMsg(uint32_t _tid, Level _level, const char *_file, const char *_func, int _line, time_t _t, std::string &_msg)
            : tid(_tid)
            , level(_level)
            , file_name(_file)
            , func_name(_func)
            , line(_line)
            , msg_time(_t)
            , msg(_msg){};
    };

    /**
     * OnMatch = "ACCEPT" 匹配该级别及以上
     * OnMatch = "DENY" 不匹配该级别及以上
     * OnMatch = "NEUTRAL" 该级别及以上的，由下一个filter处理；如果当前是最后一个，则匹配
     * OnMismatch = "ACCEPT" 匹配该级别及以下
     * OnMismatch = "DENY" 不匹配该级别及以下
     * OnMismatch = "NEUTRAL" 该级别及以下的，由下一个filter处理；如果当前是最后一个，则匹配
     */
    struct Filter {
        Level        level;
        FilterMatch  on_match;
        FilterMatch  mis_match;
    };

    struct TimeBasedTriggeringPolicy {
        int          interval;           //指定多久滚动一次，默认是1 hour
        bool         modulate;           //用来调整时间：比如现在是早上3am，interval是4，那么第一次滚动是在4am，接着是8am，12am...而不是7am
    };

    struct SizeBasedTriggeringPolicy {
        uint64_t          size;               //定义每个日志文件的大小
    };

    struct Policies {
        TimeBasedTriggeringPolicy  time_policy;
        SizeBasedTriggeringPolicy  size_policy;
    };

    /** appender基础类型 */
    class Appender {
    public:
        AppenderType   type;    // 指定appender的类型
        std::string    name;    // 指定Appender的名字
        std::string    pattern_layout;  // 输出格式，不设置默认为:%m%n
        std::list<Filter>  filter;      // 对不同等级的日志的处理方式
        moodycamel::ConcurrentQueue<std::shared_ptr<LogMsg>> 
                       msg_queue;
        uv_loop_t     *uv_loop;
        uv_async_t     uv_async;

        Appender(){};
        virtual ~Appender(){};
        virtual void Init(uv_loop_t *uv) = 0;
        virtual void Write() = 0;
    };

    /** 控制台输出appender */
    class ConsolAppender : public Appender {
    public:
        ConsolTarget    target;             //一般只设置默认:SYSTEM_OUT
        bool            opening;            //正在打开控制台
        bool            opened;             //控制台已经打开
        uv_tty_t        tty_handle;         //控制台句柄

        ConsolAppender();
        virtual ~ConsolAppender();
        virtual void Init(uv_loop_t *uv);
        virtual void Write();
    };

    /** 单文件输出appender */
    class FileAppender : public Appender {
    public:
        std::string     file_name;         //指定输出日志的目的文件带全路径的文件名
        bool            append;            //是否追加，默认false
        bool            opening;           //正在打开文件
        bool            opened;            //文件已经打开
        bool            writing;           //正在写入数据
        uv_file         file_handle;       //打开的文件句柄

        FileAppender();
        virtual ~FileAppender();
        virtual void Init(uv_loop_t *uv);
        virtual void Write();
        virtual void WriteCB();
    };

    /** 动态文件输出appender */
    class RollingFileAppender : public FileAppender {
    public:
        std::string     filePattern;       //指定新建日志文件的名称格式.
        Policies        policies;          //指定滚动日志的策略，就是什么时候进行新建日志文件输出日志.
        int             max;               //用来指定同一个文件夹下最多有几个日志文件时开始删除最旧的，创建新的

        RollingFileAppender();
        virtual ~RollingFileAppender();
        virtual void Init(uv_loop_t *uv);
        virtual void CheckFile(bool append);
        virtual void WriteCB();
        virtual void StatCB(uint64_t size);
    protected:
        virtual bool RenameFile(int apd);
    };

    struct Logger {
        Level                     level;              //该级或以上的日志才提交给appender
        std::string               name;               //日志名称
        std::list<std::string>    appender_ref;       //写到哪些appender
        bool                      additivity;         //appender_ref不为空时，是否仍然输出到root
    };

    /** 全局配置 */
    struct Configuration {
        //int             monitorinterval;    //指定log4j自动重新配置的监测间隔时间，单位是s,最小是5s
        std::unordered_map<std::string, Appender*>
                        appenders;
        Logger         *root;
        std::unordered_map<std::string, Logger*>
                        loggers;
    };

    /** 写日志请求 */
    struct LogMsgReq {
        Appender        *appender;
        std::shared_ptr<LogMsg> item;
        char            *buff;
    };
};