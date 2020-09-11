#pragma once

#include "utilc.h"
#include "ludb_private.h"

#ifdef DB_MONGO

class ludb_mongo_conn : public ludb_conn_t
{
public:
    ludb_mongo_conn();
    ~ludb_mongo_conn();

    virtual ludb_stmt_t* create_stmt();
    virtual bool ludb_commit();
};

class ludb_mongo_stmt : public ludb_stmt_t
{
public:
    ludb_mongo_stmt();
    ~ludb_mongo_stmt();

    virtual bool execute(const char *sql);
    virtual bool prepare(const char *sql);
    virtual bool bind_int(const char *name, int *data);
    virtual bool bind_str(const char *name, const char *data, int len);
    virtual bool execute();
    virtual uint32_t affected_rows();
    virtual ludb_rest_t* result();
};

class ludb_mongo_rest_t : public ludb_rest_t
{
public:
    ludb_mongo_rest_t();
    ~ludb_mongo_rest_t();

    virtual bool next();
    virtual string get_char(uint32_t i);
    virtual int get_int(uint32_t i);
    virtual string get_date(uint32_t i);
    virtual string get_blob(uint32_t i);
};


extern bool ludb_mongo_init(const char *path /*= NULL*/);

extern void ludb_mongo_clean();

extern ludb_conn_t* ludb_mongo_connect(const char *database, const char *usr, const char *pwd);

extern bool ludb_mongo_create_pool(const char *tag, const char *database, const char *usr, const char *pwd, uint32_t max, uint32_t min, uint32_t inc);

extern ludb_conn_t* ludb_mongo_pool_connect(const char *tag);

//////////////////////////////////////////////////////////////////////////

class ludb_mongo_batch : public ludb_batch_t
{
public:
    char*  buff;     //< ÄÚ´æÇøÓò

    ludb_mongo_batch(string Tag, string Sql, int RowNum, int Interval, bind_column_t* Binds);
    ~ludb_mongo_batch();

    virtual bool insert();
};
#endif