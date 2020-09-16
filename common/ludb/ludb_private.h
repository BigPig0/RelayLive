#pragma once

#define DB_ORACLE
//#define DB_MONGO
#define DB_REDIS

#include "util.h"
#include "ludb_public.h"
#include "uv.h"
#include <string>
#include <vector>
#include <queue>
#include <map>
using namespace std;

/** ���Ӿ�� */
struct ludb_conn_t {
    ludb_db_type_t type;
    bool           from_pool;
    void          *conn;

    virtual ~ludb_conn_t(){}
    virtual ludb_stmt_t* create_stmt() = 0;
    virtual bool ludb_commit() = 0;
};

/** statement��� */
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

/** resultset��� */
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

/** ���ӳؾ�� */
struct ludb_pool_t {
    ludb_db_type_t type;
    void          *pool;
};

/** �ֶζ��� */
struct bind_column_pri_t {
    string           name;        //< �󶨱�����ƻ�������
    column_type_t    type;        //< ������
    int              max_len;     //< ��󳤶ȣ��������ַ���ʱ��������
    bool             nullable;    //< �Ƿ��Ϊ��
    string           default_value;     //< Ĭ��ֵ��ż����Ҫ
};

/** ���������� */
struct ludb_batch_t {
    ludb_db_type_t type;      //< ���ݿ�����
    string         tag;       //< ���ӳر�ǩ
    string         sql;       //< ִ�е�sql���
    int            row_num;   //< ���ݻ���ﵽ��ô���о�ִ��
    int            interval;  //< ���ϴ�ִ�о�����ô���룬�������ݲ�Ϊ�վ�ִ�С�0 ��������ʱ��

    vector<bind_column_pri_t>     binds;     //< vector<bind_column_t>
    queue<vector<string>>         recvs;     //< queue<vector<string>> ���ݽ��ն���
    vector<vector<string>>        insts;     //< vector<vector<string>> ����������
    uv_mutex_t     mutex;     //< ���������������л�����
    bool           running;   //< �Ƿ�����ִ��
    time_t         ins_time;  //< ��һ���ύ��ʱ��
    uv_thread_t    tid;       //< ִ���߳�ID

    ludb_batch_t(string Tag, string Sql, int RowNum, int Interval, bind_column_t* Binds);
    virtual ~ludb_batch_t();
    virtual bool insert() = 0;
};

extern int pointLen;
