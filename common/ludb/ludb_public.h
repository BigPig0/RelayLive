#pragma once

#include "utilc.h"


/** ���ݿ����� */
typedef enum _ludb_db_type {
    ludb_db_unknow = 0,
    ludb_db_oracle,
    ludb_db_mongo,
    ludb_db_redis
} ludb_db_type_t;

/** connect��� */
struct ludb_conn_t;

/** statement��� */
struct ludb_stmt_t;

/** resultset��� */
struct ludb_rest_t;

/** ���������� */
struct ludb_batch_t;

/** �ֶ����Ͷ��� */
typedef enum _column_type_ {
    column_type_char = 0,
    column_type_int,
    column_type_float,
    column_type_long,
    column_type_uint,
    column_type_blob,
    column_type_date
}column_type_t;

/** �ֶζ��� */
struct bind_column_t {
    const char      *name;        //< �󶨱�����ƻ�������
    column_type_t    type;        //< ������
    int              max_len;     //< ��󳤶ȣ��������ַ���ʱ��������
    bool             nullable;    //< �Ƿ��Ϊ��
    const char      *default_value;     //< Ĭ��ֵ��ż����Ҫ
};
