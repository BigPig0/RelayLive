#include "stdafx.h"
#include "DataBase.h"
#include "dbTool.h"
#include "luapp.hpp"

DBTOOL_INSERTER             _inserter; 
RowCollector                _rows;

CDataBase::CDataBase(void)
    : m_strDB("DB")
{
}

CDataBase::~CDataBase(void)
{
    lua::GlobalFunction<lua::Nil()> luafCleanup;
    m_lua.getFunc("Init",&luafCleanup);
    luafCleanup();
}

void CDataBase::Init()
{
    string path = Settings::getValue("DataBase", "Path");
    if(!dbTool::Init(path.c_str(), &m_lua))
        return;

    dbTool::ConPool_Setting dbset;
    dbset.database = Settings::getValue("DataBase","Addr");
    dbset.username = Settings::getValue("DataBase","User");
    dbset.password = Settings::getValue("DataBase","PassWord");
    dbset.max_conns = 5;
    dbset.min_conns = 0;
    dbset.inc_conns = 2;

    DBTOOL_CREATE_POOL(m_strDB, dbset);

    string tableName = Settings::getValue("DataBase","TableName");
    string colCode = Settings::getValue("DataBase","ColumnCODE");
    string colStat = Settings::getValue("DataBase","ColumnSTATUS");
    string colLon = Settings::getValue("DataBase","ColumnLON");
    string colLat = Settings::getValue("DataBase","ColumnLAT");

    stringstream ss;
    ss << "select " << colCode << "," << colStat << " from  " << tableName;
    m_strGetDevsSql = ss.str();
    ss.clear();
    ss.str();
    ss << "update " << tableName << " set " << colStat << " = :status where " << colCode << " = :code";
    m_strUpdateStatSql = ss.str();
    ss.clear();
    ss.str();
    ss << "update " << tableName << " set " << colLat << " = :lat, " 
        << colLon << " = :lon where " << colCode << " = :code";
    m_strUpdatePosSql == ss.str();

    _inserter.Init(m_strDB, tableName, 10);
    _inserter.AddCloumn(colCode, SQLT_CHR, 20);
    _inserter.AddCloumn(colStat, SQLT_INT);
    _inserter.AddCloumn(colLat,  SQLT_FLT);
    _inserter.AddCloumn(colLon,  SQLT_FLT);
    _inserter.Prepair();
    _rows.set_info(50, 10);
    _rows.add_insertion_handler([&](vector<vector<string>> v){
        _inserter.Insert(v);
    });

    m_lua.run(".","DeviceMgr.lua");
    lua::GlobalFunction<lua::Bool()> luafInit;
    m_lua.getFunc("Init",&luafInit);
    luafInit();
}

vector<DevInfo*> CDataBase::GetDevInfo()
{
    vector<DevInfo*> vecRet;
    OCI_Connection *cn = DBTOOL_GET_CONNECT(m_strDB);
    if (!cn) {
        Log::error("fail to get connection: %s", m_strDB.c_str());
        return vecRet;
    }
    OCI_Statement *st = OCI_CreateStatement(cn);
    OCI_ExecuteStmt(st, m_strGetDevsSql.c_str());
    OCI_Resultset *rs = OCI_GetResultset(st);
    while (OCI_FetchNext(rs)) 
    {
        DevInfo* dev = new DevInfo;
        dev->strDevID     = DBTOOL_GET_STRING(rs,1);
        dev->strStatus    = DBTOOL_GET_INT(rs,2)>0?"ON":"OFF";
        vecRet.push_back(dev);
    }
    OCI_FreeStatement(st);
    OCI_ConnectionFree(cn);
    return vecRet;
}

bool CDataBase::UpdateStatus(string code, bool online)
{
    int nStateValue = online?1:0;
    OCI_Connection *cn = DBTOOL_GET_CONNECT(m_strDB);
    if (!cn) {
        Log::error("fail to get connection: %s", m_strDB.c_str());
        return false;
    }
    OCI_Statement *st = OCI_CreateStatement(cn);
    OCI_Prepare(st, m_strUpdateStatSql.c_str());
    OCI_BindInt(st, ":status",   &nStateValue);
    OCI_BindString(st, ":code", (char*)code.c_str(), 30);
    OCI_Execute(st);
    int count = OCI_GetAffectedRows(st);
    OCI_Commit(cn);
    OCI_FreeStatement(st);
    OCI_ConnectionFree(cn);
    return true;
}

bool CDataBase::UpdatePos(string code, string lat, string lon)
{
    if(lat.size() > 9) lat = lat.substr(0, 9);
    if(lon.size() > 9) lon = lon.substr(0, 9);
    OCI_Connection *cn = DBTOOL_GET_CONNECT(m_strDB);
    if (!cn) {
        Log::error("fail to get connection: %s", m_strDB.c_str());
        return false;
    }
    OCI_Statement *st = OCI_CreateStatement(cn);
    OCI_Prepare(st, m_strUpdatePosSql.c_str());
    OCI_BindString(st, ":lat", (char*)lat.c_str(), 10);
    OCI_BindString(st, ":lon", (char*)lon.c_str(), 10);
    OCI_BindString(st, ":code", (char*)code.c_str(), 30);
    OCI_Execute(st);
    int count = OCI_GetAffectedRows(st);
    OCI_Commit(cn);
    OCI_FreeStatement(st);
    OCI_ConnectionFree(cn);
    return true;
}

bool CDataBase::InsertDev(DevInfo* dev)
{
    vector<string> values;
    values.push_back(dev->strDevID);
    values.push_back(dev->strStatus=="ON"?"1":"0");
    values.push_back(dev->strLatitude);
    values.push_back(dev->strLongitude);
    _rows.add_row(values);
}