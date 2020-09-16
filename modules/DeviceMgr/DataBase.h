#pragma once
#include "PublicDefine.h"


class CDataBase
{
public:
    CDataBase(void);
    ~CDataBase(void);
    void Init();

    /** �����ݿ��ѯ�õ��Ѿ����ڵ��豸��Ϣ */
    vector<DevInfo*> GetDevInfo();

    /** �����豸������״̬ */
    bool UpdateStatus(string code, bool online);

    /** �����豸�ľ�γ�� */
    bool UpdatePos(string code, string lat, string lon);

    /** ����в����µ��豸 */
    bool InsertDev(DevInfo* dev);

    /** ������ݿ��豸�� */
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

