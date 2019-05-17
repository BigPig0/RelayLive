#pragma once

#include "ocilib.h"

//获取连接
#define DBTOOL_CREATE_POOL(t,s) dbTool::Connect(t,s)
#define DBTOOL_GET_CONNECT(t)   dbTool::GetConnection(t)

//查询
#define DBTOOL_GET_STRING(rs,i) dbTool::oci_get_string(rs, i).c_str()
#define DBTOOL_GET_INT(rs,i)    dbTool::oci_get_int(rs, i)
#define DBTOOL_GET_ODT(rs,i)    dbTool::oci_get_date(rs, i)
#define DBTOOL_GET_BLOB(rs,i)   dbTool::oci_get_blob(rs,i)

namespace dbTool
{
    struct ConPool_Setting
    {
        std::string database;
        std::string username;
        std::string password;
        int max_conns;
        int min_conns;
        int inc_conns;  //调整时一次增减的连接数
    };

    bool Init(const char *path = NULL, void *lua = NULL);

    void Cleanup();

    bool Connect(std::string tag, ConPool_Setting settings);

    OCI_Connection* GetConnection(std::string tag);

    //查询记录中的字符串
    string oci_get_string(OCI_Resultset* rs, unsigned int i);

    //查询记录中的整数
    int oci_get_int(OCI_Resultset* rs, unsigned int i);

    //查询记录中的时间
    string oci_get_date(OCI_Resultset* rs, unsigned int i);

    //查询记录中读取二进制数据
    string oci_get_blob(OCI_Resultset* rs, unsigned int i);
}