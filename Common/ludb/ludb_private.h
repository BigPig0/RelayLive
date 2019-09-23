#pragma once

#define DB_ORACLE
//#define DB_MONGO
#define DB_REDIS

#include "common.h"
#include "ludb_public.h"
#include "uv.h"
#include <string>
#include <vector>
#include <queue>
using namespace std;

/** 连接句柄 */
struct ludb_conn_t {
    ludb_db_type_t type;
    bool           from_pool;
    void          *conn;

    virtual ~ludb_conn_t(){}
    virtual ludb_stmt_t* create_stmt() = 0;
    virtual bool ludb_commit() = 0;
};

/** statement句柄 */
struct ludb_stmt_t {
    ludb_db_type_t type;
    ludb_conn_t   *conn;
    void          *stmt;

    virtual ~ludb_stmt_t(){}
    virtual bool execute(const char *sql) = 0;
    virtual bool prepare(const char *sql) = 0;
    virtual bool bind_int(const char *name, int *data) = 0;
    virtual bool bind_str(const char *name, const char *data, int len) = 0;
    virtual bool execute() = 0;
    virtual uint32_t affected_rows() = 0;
    virtual ludb_rest_t* result() = 0;
};

/** resultset句柄 */
struct ludb_rest_t {
    ludb_db_type_t type;
    ludb_conn_t   *conn;
    ludb_stmt_t   *stmt;
    void          *rest;

    virtual ~ludb_rest_t(){};
    virtual bool next() = 0;
    virtual string get_char(uint32_t i) = 0;
    virtual int get_int(uint32_t i) = 0;
    virtual string get_date(uint32_t i) = 0;
    virtual string get_blob(uint32_t i) = 0;
};

/** 连接池句柄 */
struct ludb_pool_t {
    ludb_db_type_t type;
    void          *pool;
};

/** 字段定义 */
struct bind_column_pri_t {
    string           name;        //< 绑定标记名称或列名称
    column_type_t    type;        //< 列类型
    int              max_len;     //< 最大长度，类型是字符串时必须设置
    bool             nullable;    //< 是否可为空
    string           default_value;     //< 默认值，偶尔需要
};

/** 批量处理句柄 */
struct ludb_batch_t {
    ludb_db_type_t type;      //< 数据库类型
    string         tag;       //< 连接池标签
    string         sql;       //< 执行的sql语句
    int            row_num;   //< 数据缓存达到这么多行就执行
    int            interval;  //< 从上次执行经过这么多秒，缓存数据不为空就执行。0 不启动定时器

    vector<bind_column_pri_t>     binds;     //< vector<bind_column_t>
    queue<vector<string>>         recvs;     //< queue<vector<string>> 数据接收队列
    vector<vector<string>>        insts;     //< vector<vector<string>> 待入库的数据
    uv_mutex_t     mutex;     //< 上面这两个队列切换的锁
    bool           running;   //< 是否正在执行
    time_t         ins_time;  //< 上一次提交的时间
    uv_thread_t    tid;       //< 执行线程ID

    ludb_batch_t(string Tag, string Sql, int RowNum, int Interval, bind_column_t* Binds);
    virtual ~ludb_batch_t();
    virtual bool insert() = 0;
};

extern int pointLen;
