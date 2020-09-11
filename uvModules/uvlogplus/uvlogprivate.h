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
        consol = 0,         //����̨
        file,               //�ļ�
        rolling_file,       //�ļ���С����ָ���ߴ��ʱ�����һ���µ��ļ�
    };

    enum ConsolTarget {
        SYSTEM_OUT = 0,
        SYSTEM_ERR
    };

    enum FilterMatch {
        ACCEPT = 0,     //����
        NEUTRAL,        //����
        DENY            //�ܾ�
    };

    /** һ����־���� */
    struct LogMsg {
        uint32_t    tid;            //�߳�ID
        Level       level;          //��־�ȼ�
        const char *file_name;      //�����ļ�����
        const char *func_name;      //���ں�������
        int         line;           //�к�
        time_t      msg_time;       //����ʱ��
        std::string msg;            //����
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
     * OnMatch = "ACCEPT" ƥ��ü�������
     * OnMatch = "DENY" ��ƥ��ü�������
     * OnMatch = "NEUTRAL" �ü������ϵģ�����һ��filter���������ǰ�����һ������ƥ��
     * OnMismatch = "ACCEPT" ƥ��ü�������
     * OnMismatch = "DENY" ��ƥ��ü�������
     * OnMismatch = "NEUTRAL" �ü������µģ�����һ��filter���������ǰ�����һ������ƥ��
     */
    struct Filter {
        Level        level;
        FilterMatch  on_match;
        FilterMatch  mis_match;
    };

    struct TimeBasedTriggeringPolicy {
        int          interval;           //ָ����ù���һ�Σ�Ĭ����1 hour
        bool         modulate;           //��������ʱ�䣺��������������3am��interval��4����ô��һ�ι�������4am��������8am��12am...������7am
    };

    struct SizeBasedTriggeringPolicy {
        uint64_t          size;               //����ÿ����־�ļ��Ĵ�С
    };

    struct Policies {
        TimeBasedTriggeringPolicy  time_policy;
        SizeBasedTriggeringPolicy  size_policy;
    };

    /** appender�������� */
    class Appender {
    public:
        AppenderType   type;    // ָ��appender������
        std::string    name;    // ָ��Appender������
        std::string    pattern_layout;  // �����ʽ��������Ĭ��Ϊ:%m%n
        std::list<Filter>  filter;      // �Բ�ͬ�ȼ�����־�Ĵ���ʽ
        moodycamel::ConcurrentQueue<std::shared_ptr<LogMsg>> 
                       msg_queue;
        uv_loop_t     *uv_loop;
        uv_async_t     uv_async;

        Appender(){};
        virtual ~Appender(){};
        virtual void Init(uv_loop_t *uv) = 0;
        virtual void Write() = 0;
    };

    /** ����̨���appender */
    class ConsolAppender : public Appender {
    public:
        ConsolTarget    target;             //һ��ֻ����Ĭ��:SYSTEM_OUT
        bool            opening;            //���ڴ򿪿���̨
        bool            opened;             //����̨�Ѿ���
        uv_tty_t        tty_handle;         //����̨���

        ConsolAppender();
        virtual ~ConsolAppender();
        virtual void Init(uv_loop_t *uv);
        virtual void Write();
    };

    /** ���ļ����appender */
    class FileAppender : public Appender {
    public:
        std::string     file_name;         //ָ�������־��Ŀ���ļ���ȫ·�����ļ���
        bool            append;            //�Ƿ�׷�ӣ�Ĭ��false
        bool            opening;           //���ڴ��ļ�
        bool            opened;            //�ļ��Ѿ���
        bool            writing;           //����д������
        uv_file         file_handle;       //�򿪵��ļ����

        FileAppender();
        virtual ~FileAppender();
        virtual void Init(uv_loop_t *uv);
        virtual void Write();
        virtual void WriteCB();
    };

    /** ��̬�ļ����appender */
    class RollingFileAppender : public FileAppender {
    public:
        std::string     filePattern;       //ָ���½���־�ļ������Ƹ�ʽ.
        Policies        policies;          //ָ��������־�Ĳ��ԣ�����ʲôʱ������½���־�ļ������־.
        int             max;               //����ָ��ͬһ���ļ���������м�����־�ļ�ʱ��ʼɾ����ɵģ������µ�

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
        Level                     level;              //�ü������ϵ���־���ύ��appender
        std::string               name;               //��־����
        std::list<std::string>    appender_ref;       //д����Щappender
        bool                      additivity;         //appender_ref��Ϊ��ʱ���Ƿ���Ȼ�����root
    };

    /** ȫ������ */
    struct Configuration {
        //int             monitorinterval;    //ָ��log4j�Զ��������õļ����ʱ�䣬��λ��s,��С��5s
        std::unordered_map<std::string, Appender*>
                        appenders;
        Logger         *root;
        std::unordered_map<std::string, Logger*>
                        loggers;
    };

    /** д��־���� */
    struct LogMsgReq {
        Appender        *appender;
        std::shared_ptr<LogMsg> item;
        char            *buff;
    };
};