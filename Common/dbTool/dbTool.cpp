#include "stdafx.h"
#include "dbTool.h"

namespace dbTool
{
    std::map<std::string, OCI_ConnPool*> conn_pools_;

    bool Init(const char *path)
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