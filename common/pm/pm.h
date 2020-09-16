#pragma once
#include "util.h"
#include <string>

class CProcessMgr
{
public:
    struct Options {
        std::string path;   //�����ļ�·��
        std::string args;   //ִ�в���
        bool        daemon; //�Ƿ��ػ�����
        std::string rstime; //������ʱ��㣬24Сʱ�ƣ�eg:"15:30:00"
        int         rsdur;  //���о�����û���������λ��
        std::string remark; //��ע˵��
        int         exedir; //����Ŀ¼ 0:�ػ����̵Ĺ���Ŀ¼ 1:�����ļ����ڵ�Ŀ¼ 2:�Զ���
        std::string usrdir; //�û��Զ���Ĺ���Ŀ¼
    };

    struct Process : public Options {
        uint64_t    pid;    //����ID
        time_t      startup;//����ʱ���
        bool        running;
        bool        needclose;
    };

    static CProcessMgr* Creat();

    virtual void Start() = 0;

    virtual void AddTask(Options opt) = 0;

    virtual void AddTasks(std::string path) = 0;

    virtual void Stop() = 0;

    virtual std::vector<Process> GetInfo() = 0;

    virtual ~CProcessMgr();
protected:
    CProcessMgr();
};