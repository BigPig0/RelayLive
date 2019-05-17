#pragma once
#include "PublicDefine.h"


class CDataBase
{
public:
    CDataBase(void);
    ~CDataBase(void);
    void Init();

    /** 从数据库查询得到已经存在的设备信息 */
    vector<DevInfo*> GetDevInfo();

    /** 更新设备的在线状态 */
    bool UpdateStatus(string code, bool online);

    /** 更新设备的经纬度 */
    bool UpdatePos(string code, string lat, string lon);

    /** 向库中插入新的设备 */
    bool InsertDev(DevInfo* dev);

    /** 清空数据库设备表 */
    bool CleanDev();

	bool UpdateDevPos(DevInfo* dev);

private:
    string                 m_strDB;
    string                 m_strGetDevsSql;
    string                 m_strUpdateStatSql;
    string                 m_strUpdatePosSql;

    lua::State<>           m_lua;
    lua::GlobalFunction<lua::Table()> 
                           luafGetDevInfo;
    lua::GlobalFunction<lua::Bool(lua::Str, lua::Bool)> 
                           luafUpdateStatus;
    lua::GlobalFunction<lua::Bool(lua::Str, lua::Str, lua::Str)> 
                           luafUpdatePos;
    lua::GlobalFunction<lua::Bool(lua::Table)> 
                           luafInsertDev;
    lua::GlobalFunction<lua::Bool()>
                           luafDeleteDev;
	lua::GlobalFunction<lua::Table(lua::Table)>
		                   luafTransDevPos;
};

