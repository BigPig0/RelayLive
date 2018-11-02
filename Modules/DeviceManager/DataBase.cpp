#include "stdafx.h"
#include "DataBase.h"
#include "libOci.h"


CDataBase::CDataBase(void)
    : m_strDB("DB")
{
    auto oci_error_handler = [](OCI_Error *err){
        OCI_Connection * conn = OCI_ErrorGetConnection(err);
        Log::error("Error[ORA-%05d] - msg[%s] - database[%s] - user[%s] - sql[%s]"
            , OCI_ErrorGetOCICode(err)
            , OCI_ErrorGetString(err)
            , OCI_GetDatabase(conn)
            , OCI_GetUserName(conn)
            , OCI_GetSql(OCI_ErrorGetStatement(err)));
    };
    string path = Settings::getValue("DataBase", "path");

    if(!OCI_Initialize(oci_error_handler, path.c_str(), OCI_ENV_THREADED))
        return;

    ConPool_Setting dbset;
    dbset.database = Settings::getValue("DataBase","Addr");
    dbset.username = Settings::getValue("DataBase","User");
    dbset.password = Settings::getValue("DataBase","PassWord");
    dbset.max_conns = 5;
    dbset.min_conns = 2;
    dbset.inc_conns = 2;
    OracleClient* client = OracleClient::GetInstance();
    client->connect(m_strDB, dbset);
}


CDataBase::~CDataBase(void)
{
}

vector<DevInfo*> CDataBase::GetDevInfo()
{
    vector<DevInfo*> vecRet;
    conn_pool_ptr pool = OracleClient::GetInstance()->get_conn_pool(m_strDB);
    if (pool == NULL) {
        Log::error("fail to get pool: %s", m_strDB.c_str());
        return vecRet;
    }
    int index = pool->getConnection();
    if (index == -1) {
        Log::error("fail to get connection: %s", m_strDB.c_str());
        return vecRet;
    }
    OCI_Connection *cn = pool->at(index);
    OCI_Statement *st = OCI_CreateStatement(cn);
    const char *sql = "select t.RECORDER_CODE, t.STATUS from ENFORCEMENT_RECORDER t";
    OCI_ExecuteStmt(st, sql);
    OCI_Resultset *rs = OCI_GetResultset(st);
    while (OCI_FetchNext(rs)) 
    {
        DevInfo* dev = new DevInfo;
        dev->strDevID     = OCI_GET_STRING(rs,1);
        dev->strStatus    = OCI_GET_INT(rs,2)>0?"ON":"OFF";
        vecRet.push_back(dev);
    }
    return vecRet;
}

bool CDataBase::UpdateStatus(string code, bool online)
{
    conn_pool_ptr pool = OracleClient::GetInstance()->get_conn_pool(m_strDB);
    if (pool == NULL) {
        Log::error("fail to get pool: %s", m_strDB.c_str());
        return false;
    }
    int index = pool->getConnection();
    if (index == -1) {
        Log::error("fail to get connection: %s", m_strDB.c_str());
        return false;
    }
    int nStateValue = online?1:0;
    OCI_Connection *cn = pool->at(index);
    OCI_Statement *st = OCI_CreateStatement(cn);
    const char *sql = "update ENFORCEMENT_RECORDER set STATUS = :status where RECORDER_CODE = :code";
    OCI_Prepare(st, sql);
    OCI_BindInt(st, ":status",   &nStateValue);
    OCI_BindString(st, ":code", (char*)code.c_str(), 30);
    OCI_Execute(st);
    int count = OCI_GetAffectedRows(st);
    OCI_Commit(cn);
    OCI_FreeStatement(st);
    pool->releaseConnection(index);
    return true;
}

bool CDataBase::UpdatePos(string code, string lat, string lon)
{
    conn_pool_ptr pool = OracleClient::GetInstance()->get_conn_pool(m_strDB);
    if (pool == NULL) {
        Log::error("fail to get pool: %s", m_strDB.c_str());
        return false;
    }
    int index = pool->getConnection();
    if (index == -1) {
        Log::error("fail to get connection: %s", m_strDB.c_str());
        return false;
    }
    if(lat.size() > 9) lat = lat.substr(0, 9);
    if(lon.size() > 9) lon = lon.substr(0, 9);
    OCI_Connection *cn = pool->at(index);
    OCI_Statement *st = OCI_CreateStatement(cn);
    const char *sql = "update ENFORCEMENT_RECORDER set LAT = :lat, LON = :lon where RECORDER_CODE = :code";
    OCI_Prepare(st, sql);
    OCI_BindString(st, ":lat", (char*)lat.c_str(), 10);
    OCI_BindString(st, ":lon", (char*)lon.c_str(), 10);
    OCI_BindString(st, ":code", (char*)code.c_str(), 30);
    OCI_Execute(st);
    int count = OCI_GetAffectedRows(st);
    OCI_Commit(cn);
    OCI_FreeStatement(st);
    pool->releaseConnection(index);
    return true;
}