#pragma once
#include "Singleton.h"
#include "ConnPool.h"
#include "OciDefine.h"

//获取连接
#define GET_CONNECT_POOL(s)  OracleClient::GetInstance()->get_conn_pool(s)

//查询
#define OCI_GET_STRING(rs,i) OCI_IsNull(rs, i)?"":OCI_GetString(rs, i)
#define OCI_GET_INT(rs,i)    OCI_IsNull(rs, i)?0:OCI_GetInt(rs, i)
#define OCI_GET_ODT(rs,i)    OracleClient::oci_get_date(rs, i)
#define OCI_GET_BLOB(rs,i)   OracleClient::oci_get_blob(rs,i)

struct ConPool_Setting
{
    std::string database;
    std::string username;
    std::string password;
    int max_conns;
    int min_conns;
    int inc_conns;  //调整时一次增减的连接数
};

class LIBOCI_API OracleClient : public Singleton<OracleClient>
{
    friend class Singleton<OracleClient>;
    OracleClient(void);
public:
    ~OracleClient(void);

    /**
     * 根据配置去连接数据库
     * @param tag 数据库标签
     * @param settings数据库配置
     */
    bool connect(std::string tag, ConPool_Setting settings);

    conn_pool_ptr get_conn_pool(string tag);

    //查询记录中的时间
    static string oci_get_date(OCI_Resultset* rs,unsigned int i);

    //查询记录中读取二进制数据
    static string oci_get_blob(OCI_Resultset* rs,unsigned int i);

private:
    std::map<std::string, conn_pool_ptr> conn_pools_;
};

