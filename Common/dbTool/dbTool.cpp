#include "stdafx.h"
#include "dbTool.h"
#include "luapp.hpp"

namespace dbTool
{
    std::map<std::string, OCI_ConnPool*> conn_pools_;

    lua::Bool luaPoolConnect(lua::Table pra){
        string tag = pra["tag"];
        ConPool_Setting settings;
        settings.database = pra["dbpath"];
        settings.username = pra["user"];
        settings.password = pra["pwd"];
        settings.max_conns = pra.isExist("max")?lua::VarCast<lua::Int>(pra["max"]):1;
        settings.min_conns = pra.isExist("min")?lua::VarCast<lua::Int>(pra["min"]):0;
        settings.inc_conns = pra.isExist("inc")?lua::VarCast<lua::Int>(pra["inc"]):1;
        return Connect(tag, settings);
    }
    lua::Ptr luaGetConnect(lua::Str tag){
        return (void*)GetConnection(tag);
    }
    lua::Bool luaFreeConnect(lua::Ptr con){
        return OCI_ConnectionFree((OCI_Connection *)con);
    }
    lua::Ptr luaCreateStatement(lua::Ptr con){
        return (void*)OCI_CreateStatement((OCI_Connection *)con);
    }
    lua::Bool luaFreeStatement(lua::Ptr stmt){
        return OCI_FreeStatement((OCI_Statement *)stmt);
    }
    lua::Bool luaExecuteStmt(lua::Ptr stmt, lua::Str sql){
        return OCI_ExecuteStmt((OCI_Statement *)stmt, sql.c_str());
    }
    lua::Bool luaPrepare(lua::Ptr stmt, lua::Str sql){
        return OCI_Prepare((OCI_Statement *)stmt, sql.c_str());
    }
    lua::Bool luaBindInt(lua::Ptr stmt, lua::Str name, lua::Ptr data){
        return OCI_BindInt((OCI_Statement *)stmt, name.c_str(), (int*)data);
    }
    lua::Bool luaBindString(lua::Ptr stmt, lua::Str name, lua::Ptr data, lua::Int maxLen){
        return OCI_BindString((OCI_Statement *)stmt, name.c_str(), (char*)data, maxLen);
    }
    lua::Bool luaExecute(lua::Ptr stmt){
        return OCI_Execute((OCI_Statement *)stmt);
    }
    lua::Int luaGetAffectedRows(lua::Ptr stmt){
        return OCI_GetAffectedRows((OCI_Statement *)stmt);
    }
    lua::Int luaCommit(lua::Ptr con){
        return OCI_Commit((OCI_Connection *)con);
    }
    lua::Ptr luaGetResultset(lua::Ptr stmt){
        return OCI_GetResultset((OCI_Statement *)stmt);
    }
    lua::Bool luaFetchNext(lua::Ptr rs){
        return OCI_FetchNext((OCI_Resultset *)rs);
    }
    lua::Str luaGetString(lua::Ptr rs, lua::Int i){
        return oci_get_string((OCI_Resultset*)rs, i);
    }
    lua::Int luaGetInt(lua::Ptr rs, lua::Int i){
        return oci_get_int((OCI_Resultset*)rs, i);
    }
    lua::Str luaGetOdt(lua::Ptr rs, lua::Int i){
        return oci_get_date((OCI_Resultset*)rs, i);
    }
    lua::Str luaGetBlob(lua::Ptr rs, lua::Int i){
        return oci_get_blob((OCI_Resultset*)rs, i);
    }
    lua::Ptr luaInsertInit(lua::Str tag, lua::Str tbl, lua::Int rnum, lua::Table columns) {
        DBTOOL_INSERTER *dbInster = new DBTOOL_INSERTER(tag, tbl, rnum);
        for(auto it = columns.getBegin(); !it.isEnd(); it++){
            lua::Var k,v;
            it.getKeyValue(&k, &v);
            if(!lua::VarType<lua::Table>(v))
                continue;
            lua::Table col = lua::VarCast<lua::Table>(v);
            if(!col.isExist(lua::Str("colname")) || !col.isExist(lua::Str("coltype")))
				continue;
			if( !lua::VarType<lua::Str>(col["colname"])
                || !lua::VarType<lua::Int>(col["coltype"]))
                continue;
            string strColname = lua::VarCast<lua::Str>(col["colname"]);
            int nColtype = lua::VarCast<lua::Int>(col["coltype"]);
            uint32_t nMaxLength = 16;
            if(col.isExist(lua::Str("maxlen"))){
				if (lua::VarType<lua::Int>(col["maxlen"]))
					nMaxLength = (uint32_t)lua::VarCast<lua::Int>(col["maxlen"]);
			}
            bool bNullable =  false;
            if(col.isExist(lua::Str("nullable"))){
				if(lua::VarType<lua::Bool>(col["nullable"]));
					bNullable = lua::VarCast<lua::Bool>(col["nullable"]);
			}
            string strDefault;
			if(col.isExist(lua::Str("def"))){
				if(lua::VarType<lua::Str>(col["def"]))
					strDefault = lua::VarCast<lua::Str>(col["def"]);
			}
            dbInster->AddCloumn(strColname,nColtype,nMaxLength, bNullable,strDefault);
        }
        dbInster->Prepair();
        return (void*)dbInster;
    }
    lua::Ptr luaRowInit(lua::Int rows, lua::Int interval){
        RowCollector *ret = new RowCollector(rows, interval);
        return ret;
    }
    lua::Bool luaRowInsertHandle(lua::Ptr ins, lua::Ptr row){
        DBTOOL_INSERTER *dbInster = (DBTOOL_INSERTER*)ins;
        RowCollector *rowList = (RowCollector*)row;
        rowList->add_insertion_handler([&](vector<vector<string>> v){
            dbInster->Insert(v);
        });
        return true;
    }
    lua::Bool luaAddRow(lua::Ptr rc, lua::Table row){
        RowCollector *rowList = (RowCollector*)rc;
        vector<string> values;
        for(auto it = row.getBegin(); !it.isEnd(); it++){
            lua::Var k,v;
            it.getKeyValue(&k, &v);
            lua::Str col = lua::VarCast<lua::Str>(v);
            values.push_back(col);
        }
        rowList->add_row(values);
        return true;
    }

    void InitLua(lua::State<> *lua) {
        lua->setFunc("DBTOOL_POOL_CONN",    &luaPoolConnect);
        lua->setFunc("DBTOOL_GET_CONN",     &luaGetConnect);
        lua->setFunc("DBTOOL_FREE_CONN",    &luaFreeConnect);
        lua->setFunc("DBTOOL_CREATE_STMT",  &luaCreateStatement);
        lua->setFunc("DBTOOL_FREE_STMT",    &luaFreeStatement);
        lua->setFunc("DBTOOL_EXECUTE_STMT", &luaExecuteStmt);
        lua->setFunc("DBTOOL_PREPARE",      &luaPrepare);
        lua->setFunc("DBTOOL_BIND_INT",     &luaBindInt);
        lua->setFunc("DBTOOL_BIND_STRING",  &luaBindString);
        lua->setFunc("DBTOOL_EXECUTE",      &luaExecute);
        lua->setFunc("DBTOOL_GET_AFFECT",   &luaGetAffectedRows);
        lua->setFunc("DBTOOL_COMMIT",       &luaCommit);
        lua->setFunc("DBTOOL_GET_RES",      &luaGetResultset);
        lua->setFunc("DBTOOL_FETCH_NEXT",   &luaFetchNext);

        lua->setFunc("DBTOOL_GET_STR",     &luaGetString);
        lua->setFunc("DBTOOL_GET_INT",     &luaGetInt);
        lua->setFunc("DBTOOL_GET_ODT",     &luaGetOdt);
        lua->setFunc("DBTOOL_GET_BLOB",    &luaGetBlob);

        lua->setFunc("DBTOOL_INSERT_INIT", &luaInsertInit);
        lua->setFunc("DBTOOL_ROWER_INIT",  &luaRowInit);
        lua->setFunc("DBTOOL_ROW_INS",     &luaRowInsertHandle);
        lua->setFunc("DBTOOL_ADD_ROW",     &luaAddRow);

        lua->setGlobal("DBTOOL_TYPE_CHR", SQLT_CHR);
        lua->setGlobal("DBTOOL_TYPE_INT", SQLT_INT);
        lua->setGlobal("DBTOOL_TYPE_FLT", SQLT_FLT);
        lua->setGlobal("DBTOOL_TYPE_LNG", SQLT_LNG);
        lua->setGlobal("DBTOOL_TYPE_UIN", SQLT_UIN);
        lua->setGlobal("DBTOOL_TYPE_BLOB", SQLT_BLOB);
        lua->setGlobal("DBTOOL_TYPE_ODT", SQLT_ODT);
    }

    bool Init(const char *path, void *lua)
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

        if(!OCI_Initialize(oci_error_handler, path, OCI_ENV_DEFAULT | OCI_ENV_THREADED))
            return false;

        InitLua((lua::State<>*)lua);

        return true;
    }

    void Cleanup()
    {
        for(auto poolpair:conn_pools_){
            OCI_PoolFree(poolpair.second);
        }
        conn_pools_.clear();
        OCI_Cleanup();
    }

    bool Connect(std::string tag, ConPool_Setting settings)
    {
        Log::debug("database:%s",tag.c_str());
        if(conn_pools_.count(tag))
        {
            Log::error("tag already exist");
            return false;
        }

        OCI_ConnPool *pool = OCI_PoolCreate(settings.database.c_str(), 
            settings.username.c_str(), 
            settings.password.c_str(), 
            OCI_POOL_SESSION, 
            OCI_SESSION_DEFAULT, 
            settings.min_conns, 
            settings.max_conns, 
            settings.inc_conns);
        conn_pools_[tag] = pool;
        return true;
    }

    OCI_Connection* GetConnection(std::string tag)
    {
        if (!conn_pools_.count(tag))
            return NULL;
        OCI_ConnPool* pool = conn_pools_[tag];
        OCI_Connection *cn = OCI_PoolGetConnection(pool, NULL);
        return cn;
    }


    string oci_get_string(OCI_Resultset* rs,unsigned int i)
    {
        if(OCI_IsNull(rs, i))
            return "";
        const char* pTmp = OCI_GetString(rs, i);
        if (pTmp)
            return pTmp;
        return "";
    }

    int oci_get_int(OCI_Resultset* rs, unsigned int i){
        return OCI_IsNull(rs, i)?0:OCI_GetInt(rs, i);
    }

    string oci_get_date(OCI_Resultset* rs,unsigned int i)
    {
        if(OCI_IsNull(rs, i))
            return "";
        OCI_Date * dat = OCI_GetDate(rs, i);
        char szDate[20]={0};
        if(!OCI_DateToText(dat,"yyyymmddhh24miss",20,szDate))
            return "";
        return szDate;
    }

    string oci_get_blob(OCI_Resultset* rs,unsigned int i)
    {
        if(OCI_IsNull(rs, i))
            return "";
        string strRet;
        OCI_Lob *lob = OCI_GetLob(rs,i);
        big_uint len = OCI_LobGetLength(lob);
        char* buff = new char[len+1];
        buff[len] = 0;
        if(0 == OCI_LobRead(lob, buff, len))
            return "";
        strRet = buff;
        delete[] buff;
        return strRet;
    }
}