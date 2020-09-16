#include "ludb_private.h"
#include "ludb.h"
#include "ludb_oracle.h"
#include "ludb_mysql.h"
#include "ludb_mongo.h"
#include "ludb_redis.h"
#include "ludb_lua.h"
#include <stdio.h>


bool ludb_init_oracle(const char *path /*= NULL*/) {
    return ludb_oracle_init(path);
}

bool ludb_init_mysql() {
#ifdef DB_MYSQL
    return ludb_mysql_init();
#endif
    return false;
}

bool ludb_init_mongo() {
    return true;
}

bool ludb_init_redis() {
#ifdef DB_REDIS
    return ludb_redis_init();
#endif
    return false;
}

void ludb_use_lua(void* lua) {
    ludb_lua_init(lua);
}

void ludb_clean(ludb_db_type_t t) {
    if (t == ludb_db_oracle) {
#ifdef DB_ORACLE
        ludb_oracle_clean();
#endif
    } else if (t == ludb_db_mysql) {
#ifdef DB_MYSQL
        ludb_mysql_clean();
#endif
    }else if (t == ludb_db_redis) {
#ifdef DB_REDIS
        ludb_redis_clean();
#endif
    }
}

ludb_conn_t* ludb_connect(ludb_db_type_t t, const char *database, const char *usr, const char *pwd) {
    if(t == ludb_db_oracle) {
#ifdef DB_ORACLE
        return ludb_oracle_connect(database, usr, pwd);
#endif
    } else if(t == ludb_db_mongo) {
#ifdef DB_MONGO
        return ludb_mongo_connect(database, usr, pwd);
#endif
    } else if(t == ludb_db_redis) {
#ifdef DB_REDIS
        return ludb_redis_connect(database, usr, pwd);
#endif
    }
    return NULL;
}

bool ludb_create_pool(ludb_db_type_t t, const char *tag, const char *database, const char *usr, const char *pwd, uint32_t max, uint32_t min, uint32_t inc) {
    if(t == ludb_db_oracle) {
#ifdef DB_ORACLE
        return ludb_oracle_create_pool(tag, database, usr, pwd, max, min, inc);
#endif
    } else if (t == ludb_db_mongo) {
#ifdef DB_MONGO
        return ludb_mongo_create_pool(tag, database, usr, pwd, max, min, inc);
#endif
    } else if (t == ludb_db_redis) {
#ifdef DB_REDIS
        return ludb_redis_create_pool(tag, database, usr, pwd, max, min, inc);
#endif
    }
    return false;
}

ludb_conn_t* ludb_pool_connect(ludb_db_type_t t, const char *tag) {
    if(t == ludb_db_oracle) {
#ifdef DB_ORACLE
        return ludb_oracle_pool_connect(tag);
#endif
    } else if (t == ludb_db_mongo) {
#ifdef DB_MONGO
        return ludb_mongo_pool_connect(tag);
#endif
    } else if (t == ludb_db_redis) {
#ifdef DB_REDIS
        return ludb_redis_pool_connect(tag);
#endif
    }
    return NULL;
}

bool ludb_free_conn(ludb_conn_t *conn) {
    delete conn;
    return true;
}

ludb_stmt_t* ludb_create_stmt(ludb_conn_t *conn) {
    return conn->create_stmt();
}

bool ludb_free_stmt(ludb_stmt_t *stmt) {
    delete stmt;
    return true;
}

bool ludb_execute_stmt(ludb_stmt_t *stmt, const char *sql) {
    return stmt->execute(sql);
}

bool ludb_prepare(ludb_stmt_t *stmt, const char *sql) {
    return stmt->prepare(sql);
}

bool ludb_bind_int(ludb_stmt_t *stmt, const char *name, int *data) {
    return stmt->bind_int(name, data);
}

bool ludb_bind_str(ludb_stmt_t *stmt, const char *name, const char *data, int len) {
    return stmt->bind_str(name, data, len);
}

bool ludb_bind(ludb_stmt_t *stmt, uint32_t col_num, column_type_t *col_type, string *col_value) {
    return stmt->bind(col_num, col_type, col_value);
}

bool ludb_execute(ludb_stmt_t *stmt) {
    return stmt->execute();
}

uint32_t ludb_affected_rows(ludb_stmt_t *stmt) {
    return stmt->affected_rows();
}

bool ludb_commit(ludb_conn_t *conn) {
    return conn->ludb_commit();
}

ludb_rest_t* ludb_result(ludb_stmt_t *stmt) {
    return stmt->result();
}

void ludb_free_result(ludb_rest_t *res) {
    delete res;
}

bool ludb_result_next(ludb_rest_t *res) {
    return res->next();
}

string ludb_rest_get_char(ludb_rest_t *res, uint32_t i) {
    return res->get_char(i);
}

int ludb_rest_get_int(ludb_rest_t *res, uint32_t i) {
    return res->get_int(i);
}

string ludb_rest_get_date(ludb_rest_t *res, uint32_t i) {
    return res->get_date(i);
}

string ludb_rest_get_blob(ludb_rest_t *res, uint32_t i) {
    return res->get_blob(i);
}