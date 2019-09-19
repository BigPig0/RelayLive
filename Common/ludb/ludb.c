#include "ludb_private.h"
#include "ludb.h"
#include "ludb_oracle.h"
#include "ludb_mongo.h"
#include "ludb_lua.h"
#include <stdio.h>

//Á¬½Ó³Ø
map_t *g_conn_pools = NULL;

static void log_print(char *info) {
    printf(info);
}

LOG_HANDLE g_log_hook = log_print;

void ludb_hook_log(LOG_HANDLE fuc) {
    g_log_hook = fuc;
}

bool ludb_init_oracle(const char *path /*= NULL*/) {
    return ludb_oracle_init(path);
}

bool ludb_init_mongo() {
    return true;
}

void ludb_use_lua(void* lua) {
    ludb_lua_init(lua);
}

void ludb_clean(ludb_db_type_t t) {
    if (t == ludb_db_oracle) {
        ludb_oracle_clean();
    }
}

ludb_conn_t* ludb_connect(ludb_db_type_t t, const char *database, const char *usr, const char *pwd) {
    if(t == ludb_db_oracle) {
        return ludb_oracle_connect(database, usr, pwd);
    }
    return NULL;
}

bool ludb_create_pool(ludb_db_type_t t, const char *tag, const char *database, const char *usr, const char *pwd, uint32_t max, uint32_t min, uint32_t inc) {
    if(t == ludb_db_oracle) {
        return ludb_oracle_create_pool(tag, database, usr, pwd, max, min, inc);
    } else if (t == ludb_db_mongo) {

    }
    return false;
}

ludb_conn_t* ludb_pool_connect(ludb_db_type_t t, const char *tag) {
    if(t == ludb_db_oracle) {
        return ludb_oracle_pool_connect(tag);
    } else if (t == ludb_db_mongo) {

    }
    return NULL;
}

bool ludb_free_conn(ludb_conn_t *conn) {
    if(conn->type == ludb_db_oracle) {
        return ludb_oracle_free_conn(conn);
    } else if(conn->type == ludb_db_mongo) {

    }
    return false;
}

ludb_stmt_t* ludb_create_stmt(ludb_conn_t *conn) {
    if(conn->type == ludb_db_oracle) {
        return ludb_oracle_create_stmt(conn);
    } else if(conn->type == ludb_db_mongo) {

    }
    return NULL;
}

bool ludb_free_stmt(ludb_stmt_t *stmt) {
    if(stmt->type == ludb_db_oracle) {
        return ludb_oracle_free_stmt(stmt);
    } else if(stmt->type == ludb_db_mongo) {

    }
    return false;
}

bool ludb_execute_stmt(ludb_stmt_t *stmt, const char *sql) {
    if(stmt->type == ludb_db_oracle) {
        return ludb_oracle_execute_stmt(stmt, sql);
    } else if(stmt->type == ludb_db_mongo) {

    }
    return false;
}

bool ludb_prepare(ludb_stmt_t *stmt, const char *sql) {
    if(stmt->type == ludb_db_oracle) {
        return ludb_oracle_prepare(stmt, sql);
    } else if(stmt->type == ludb_db_mongo) {

    }
    return false;
}

bool ludb_bind_int(ludb_stmt_t *stmt, const char *name, int *data) {
    if(stmt->type == ludb_db_oracle) {
        return ludb_oracle_bind_int(stmt, name, data);
    } else if(stmt->type == ludb_db_mongo) {

    }
    return false;
}

bool ludb_bind_str(ludb_stmt_t *stmt, const char *name, const char *data, int len) {
    if(stmt->type == ludb_db_oracle) {
        return ludb_oracle_bind_str(stmt, name, data, len);
    } else if(stmt->type == ludb_db_mongo) {

    }
    return false;
}

bool ludb_execute(ludb_stmt_t *stmt) {
    if(stmt->type == ludb_db_oracle) {
        return ludb_oracle_execute(stmt);
    } else if(stmt->type == ludb_db_mongo) {

    }
    return false;
}

uint32_t ludb_affected_rows(ludb_stmt_t *stmt) {
    if(stmt->type == ludb_db_oracle) {
        return ludb_oracle_affected_rows(stmt);
    } else if(stmt->type == ludb_db_mongo) {

    }
    return 0;
}

bool ludb_commit(ludb_conn_t *conn) {
    if(conn->type == ludb_db_oracle) {
        return ludb_oracle_commit(conn);
    } else if(conn->type == ludb_db_mongo) {

    }
    return false;
}

ludb_rest_t* ludb_result(ludb_stmt_t *stmt) {
    if(stmt->type == ludb_db_oracle) {
        return ludb_oracle_result(stmt);
    } else if(stmt->type == ludb_db_mongo) {

    }
    return NULL;
}

void ludb_free_result(ludb_rest_t *res) {
    free(res);
}

bool ludb_result_next(ludb_rest_t *res) {
    if(res->type == ludb_db_oracle) {
        return ludb_oracle_result_next(res);
    } else if(res->type == ludb_db_mongo) {

    }
    return false;
}

char* ludb_rest_get_char(ludb_rest_t *res, uint32_t i) {
    if(res->type == ludb_db_oracle) {
        return ludb_oracle_rest_get_char(res, i);
    } else if(res->type == ludb_db_mongo) {

    }
    return "";
}

int ludb_rest_get_int(ludb_rest_t *res, uint32_t i) {
    if(res->type == ludb_db_oracle) {
        return ludb_oracle_rest_get_int(res, i);
    } else if(res->type == ludb_db_mongo) {

    }
    return 0;
}

char* ludb_rest_get_date(ludb_rest_t *res, uint32_t i, char *buff) {
    if(res->type == ludb_db_oracle) {
        return ludb_oracle_rest_get_date(res, i, buff);
    } else if(res->type == ludb_db_mongo) {

    }
    return "";
}

char* ludb_rest_get_blob(ludb_rest_t *res, uint32_t i) {
    if(res->type == ludb_db_oracle) {
        return ludb_oracle_rest_get_blob(res, i);
    } else if(res->type == ludb_db_mongo) {

    }
    return "";
}