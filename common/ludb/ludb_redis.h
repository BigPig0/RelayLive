#pragma once

#include "utilc.h"
#include "ludb_private.h"

class ludb_redis_conn : public ludb_conn_t
{
public:
    ludb_redis_conn();
    ~ludb_redis_conn();

    virtual ludb_stmt_t* create_stmt();
    virtual bool ludb_commit();
};

class ludb_redis_stmt : public ludb_stmt_t
{
public:
    ludb_redis_stmt();
    ~ludb_redis_stmt();

    virtual bool execute(const char *sql);
    virtual bool prepare(const char *sql);
    virtual bool bind_int(const char *name, int *data);
    virtual bool bind_str(const char *name, const char *data, int len);
    virtual bool execute();
    virtual uint32_t affected_rows();
    virtual ludb_rest_t* result();
};

class ludb_redis_rest_t : public ludb_rest_t
{
public:
    ludb_redis_rest_t();
    ~ludb_redis_rest_t();

    virtual bool next();
    virtual string get_char(uint32_t i);
    virtual int get_int(uint32_t i);
    virtual string get_date(uint32_t i);
    virtual string get_blob(uint32_t i);
};


extern bool ludb_redis_init();

extern void ludb_redis_clean();

/**
 * 连接redis服务
 * @param database 服务地址，格式为host[:port],port默认为6397
 * @param usr 必须是一个数字，用来选择哪一个数据库
 * @param pwd 可以为空，需要鉴权时为密码
 */
extern ludb_conn_t* ludb_redis_connect(const char *database, const char *usr, const char *pwd);

extern bool ludb_redis_create_pool(const char *tag, const char *database, const char *usr, const char *pwd, uint32_t max, uint32_t min, uint32_t inc);

extern ludb_conn_t* ludb_redis_pool_connect(const char *tag);

//////////////////////////////////////////////////////////////////////////

class ludb_redis_batch : public ludb_batch_t
{
public:
    char*  buff;     //< 内存区域

    ludb_redis_batch(string Tag, string Sql, int RowNum, int Interval, bind_column_t* Binds);
    ~ludb_redis_batch();

    virtual bool insert();
};

