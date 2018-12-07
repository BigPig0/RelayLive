#pragma once
#include "PublicDefine.h"


class CDataBase
{
public:
    CDataBase(void);
    ~CDataBase(void);
    void init();

    vector<DevInfo*> GetDevInfo();
    bool UpdateStatus(string code, bool online);
    bool UpdatePos(string code, string lat, string lon);
private:
    string     m_strDB;
};

