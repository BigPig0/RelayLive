#pragma once

#include "utilc.h"
#include "ludb_private.h"

class ludb_oracle_conn : public ludb_conn_t
{
public:
    ludb_oracle_conn();
    ~ludb_oracle_conn();

    virtual ludb_stmt_t* create_stmt();
    virtual bool ludb_commit();
};

class ludb_oracle_stmt : public ludb_stmt_t
{
public:
    ludb_oracle_stmt();
    ~ludb_oracle_stmt();

    virtual bool execute(const char *sql);
    virtual bool prepare(const char *sql);
    virtual bool bind_int(const char *name, int *data);
    virtual bool bind_str(const char *name, const char *data, int len);
    virtual bool bind(uint32_t col_num, column_type_t *col_type, string *col_value);
    virtual bool execute();
    virtual uint32_t affected_rows();
    virtual ludb_rest_t* result();
};

class ludb_oracle_rest_t : public ludb_rest_t
{
public:
    ludb_oracle_rest_t();
    ~ludb_oracle_rest_t();

    virtual bool next();
    virtual string get_char(uint32_t i);
    virtual int get_int(uint32_t i);
    virtual string get_date(uint32_t i);
    virtual string get_blob(uint32_t i);
};


extern bool ludb_oracle_init(const char *path /*= NULL*/);

extern void ludb_oracle_clean();

extern ludb_conn_t* ludb_oracle_connect(const char *database, const char *usr, const char *pwd);

extern bool ludb_oracle_create_pool(const char *tag, const char *database, const char *usr, const char *pwd, uint32_t max, uint32_t min, uint32_t inc);

extern ludb_conn_t* ludb_oracle_pool_connect(const char *tag);

//////////////////////////////////////////////////////////////////////////

class ludb_oracle_batch : public ludb_batch_t
{
public:
    char*  buff;     //< ÄÚ´æÇøÓò

    ludb_oracle_batch(string Tag, string Sql, int RowNum, int Interval, bind_column_t* Binds);
    ~ludb_oracle_batch();

    virtual bool insert();
};
