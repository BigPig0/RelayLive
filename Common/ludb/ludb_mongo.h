#ifndef _LUDB_MONGO_H_
#define _LUDB_MONGO_H_
#include "utilc.h"
#include "ludb_private.h"

#ifdef __cplusplus
extern "C" {
#endif

extern bool ludb_mongo_init(const char *path /*= NULL*/);

extern void ludb_mongo_clean();

extern ludb_conn_t* ludb_mongo_connect(char *database, char *usr, char *pwd);

extern bool ludb_mongo_create_pool(char *tag, char *database, char *usr, char *pwd, uint32_t max, uint32_t min, uint32_t inc);

extern ludb_conn_t* ludb_mongo_pool_connect(char *tag);

extern bool ludb_mongo_free_conn(ludb_conn_t *conn);

extern ludb_stmt_t* ludb_mongo_create_stmt(ludb_conn_t *conn);

extern bool ludb_mongo_free_stmt(ludb_stmt_t *stmt);

extern bool ludb_mongo_execute_stmt(ludb_stmt_t *stmt, char *sql);

extern bool ludb_mongo_prepare(ludb_stmt_t *stmt, char *sql);

extern bool ludb_mongo_bind_int(ludb_stmt_t *stmt, char *name, int *data);

extern bool ludb_mongo_bind_str(ludb_stmt_t *stmt, char *name, char *data, int len);

extern bool ludb_mongo_execute(ludb_stmt_t *stmt);

extern uint32_t ludb_mongo_affected_rows(ludb_stmt_t *stmt);

extern bool ludb_mongo_commit(ludb_conn_t *conn);

extern ludb_rest_t* ludb_mongo_result(ludb_stmt_t *stmt);

extern bool ludb_mongo_result_next(ludb_rest_t *res);

extern char* ludb_mongo_rest_get_char(ludb_rest_t *res, uint32_t i);

extern int ludb_mongo_rest_get_int(ludb_rest_t *res, uint32_t i);

extern char* ludb_mongo_rest_get_date(ludb_rest_t *res, uint32_t i, char *buff);

extern char* ludb_mongo_rest_get_blob(ludb_rest_t *res, uint32_t i);

//////////////////////////////////////////////////////////////////////////

extern bool create_ludb_batch_mongo(ludb_batch_t *h);

extern void destory_ludb_batch_mongo(ludb_batch_t* h);

extern bool ludb_batch_insert_mongo(ludb_batch_t* h);

#ifdef __cplusplus
}
#endif
#endif