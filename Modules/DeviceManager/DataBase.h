#pragma once
#include "PublicDefine.h"


class CDataBase
{
public:
    CDataBase(void);
    ~CDataBase(void);
    void init();

    /** 从数据库查询得到已经存在的设备信息 */
    vector<DevInfo*> GetDevInfo();

    /** 更新设备的在线状态 */
    bool UpdateStatus(string code, bool online);

    /** 更新设备的经纬度 */
    bool UpdatePos(string code, string lat, string lon);

private:
    string                 m_strDB;
    string                 m_strGetDevsSql;
    string                 m_strUpdateStatSql;
    string                 m_strUpdatePosSql;
};

