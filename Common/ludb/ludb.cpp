#include "ludb_private.h"
#include "ludb.h"
#include "ludb_oracle.h"
#include "ludb_mongo.h"
#include "ludb_redis.h"
#include "ludb_lua.h"
#include <stdio.h>


bool ludb_init_oracle(const char *path /*= NULL*/) {
    return ludb_oracle_init(path);
}

bool ludb_init_mongo() {
    return true;
}

bool ludb_init_redis() {
    return ludb_redis_init();
}

void ludb_use_lua(void* lua) {
    ludb_lua_init(lua);
}

void ludb_clean(ludb_db_type_t t) {
    if (t == ludb_db_oracle) {
        ludb_oracle_clean();
    } else if (t == ludb_db_redis) {
        ludb_redis_clean();
    }
}

ludb_conn_t* ludb_connect(ludb_db_type_t t, const char *database, const char *usr, const char *pwd) {
    if(t == ludb_db_oracle) {
        return ludb_oracle_connect(database, usr, pwd);
    } else if(t == ludb_db_mongo) {
        return ludb_mongo_connect(database, usr, pwd);
    } else if(t == ludb_db_redis) {
        return ludb_redis_connect(database, usr, pwd);
    }
    return NULL;
}

bool ludb_create_pool(ludb_db_type_t t, const char *tag, const char *database, const char *usr, const char *pwd, uint32_t max, uint32_t min, uint32_t inc) {
    if(t == ludb_db_oracle) {
        return ludb_oracle_create_pool(tag, database, usr, pwd, max, min, inc);
    } else if (t == ludb_db_mongo) {
        return ludb_mongo_create_pool(tag, database, usr, pwd, max, min, inc);
    } else if (t == ludb_db_redis) {
        return ludb_redis_create_pool(tag, database, usr, pwd, max, min, inc);
    }
    return false;
}

ludb_conn_t* ludb_pool_connect(ludb_db_type_t t, const char *tag) {
    if(t == ludb_db_oracle) {
        return ludb_oracle_pool_connect(tag);
    } else if (t == ludb_db_mongo) {
        return ludb_mongo_pool_connect(tag);
    } else if (t == ludb_db_redis) {
        return ludb_redis_pool_connect(tag);
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

string ludb_rest_get_date(ludb_rest_t *res, uint32_t i, char *buff) {
    return res->get_date(i);
}

string ludb_rest_get_blob(ludb_rest_t *res, uint32_t i) {
    return res->get_blob(i);
}