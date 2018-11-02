#include "stdafx.h"
#include "OracleClient.h"


OracleClient::OracleClient(void)
{
}


OracleClient::~OracleClient(void)
{
    conn_pools_.clear();
}

bool OracleClient::connect(std::string tag, ConPool_Setting settings)
{
    Log::debug("database:%s",tag.c_str());
    if(conn_pools_.count(tag))
    {
        Log::error("tag already exist");
        return false;
    }

    conn_pool_ptr cp = conn_pool_ptr(new ConnPool(settings.database, 
        settings.username, 
        settings.password, 
        settings.min_conns, 
        settings.max_conns, 
        settings.inc_conns));
    cp->run(); 
    conn_pools_[tag] = cp;
    return true;
}

conn_pool_ptr OracleClient::get_conn_pool(string tag)
{
    if (conn_pools_.count(tag))
        return conn_pools_[tag];
    return NULL;
}

string OracleClient::oci_get_date(OCI_Resultset* rs,unsigned int i)
{
    if(OCI_IsNull(rs, i))
        return "";
    OCI_Date * dat = OCI_GetDate(rs, i);
    char szDate[20]={0};
    if(!OCI_DateToText(dat,"yyyymmddhh24miss",20,szDate))
        return "";
    return szDate;
}

string OracleClient::oci_get_blob(OCI_Resultset* rs,unsigned int i)
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