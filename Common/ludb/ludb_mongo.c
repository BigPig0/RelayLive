#include "ludb_mongo.h"
#include "ludb_private.h"
#include "mongoc.h"

static map_t *mongo_conn_pools = NULL; //map<string, OCI_ConnPool*>

/** 操作类型 */
typedef enum _stmt_type_ {
    stmt_type_unknown = 0,
    stmt_type_insert,
    stmt_type_delete,
    stmt_type_update,
    stmt_type_select
}stmt_type_t;

/** 将mongo执行方式转为类似rmdb sql */
typedef struct _mongo_stmt_ {
    stmt_type_t     type;           //操作类型
    string_t        *collection;    //集合名称
    queue_t         *queue_cols;    //操作影响的所有field
    map_t           *bind_cols;     //操作名称-对应-绑定标记或者固定值
    map_t           *bind_values;   //绑定标记-对应-值
    mongoc_cursor_t *cursor;        //查询结果
}monogo_stmt_t;

bool ludb_mongo_init() {
    mongoc_init();
    return true;
}

void ludb_mongo_clean() {
    mongoc_cleanup();
}

ludb_conn_t* ludb_mongo_connect(char *database, char *usr, char *pwd) {
    mongoc_client_t *client;
    char path[MAX_PATH] = {0};
    sprintf(path, "mongodb://%s:%s@%s", usr, pwd, database);
    client = mongoc_client_new(path);
    if(client) {
        SAFE_MALLOC(ludb_conn_t, conn);
        conn->from_pool = false;
        conn->type = ludb_db_mongo;
        conn->conn = (void *)client;
        return conn;
    }

    return NULL;
}

bool ludb_mongo_create_pool(char *tag, char *database, char *usr, char *pwd, uint32_t max, uint32_t min, uint32_t inc) {
    if(mongo_conn_pools == NULL) {
        mongo_conn_pools = create_map(void*,void*);
        map_init_ex(mongo_conn_pools, string_map_compare);
    }
    if(map_find_easy_str(mongo_conn_pools, tag)) {
        g_log_hook("mongo pool tag already exist");
        return false;
    } else {
        mongoc_client_t *client;
        char path[MAX_PATH] = {0};
        sprintf(path, "mongodb://%s:%s@%s?maxPoolSize=%d&minPoolSize=%d", usr, pwd, database, max, min);
        client = mongoc_client_new(path);
        if(client) {
            string_t *key = create_string();
            string_init_cstr(key, tag);
            map_insert_easy(mongo_conn_pools, key, client);
            return true;
        }
    }
    return false;
}

ludb_conn_t* ludb_mongo_pool_connect(char *tag){
    mongoc_client_t *client = (mongoc_client_t*)map_find_easy_str(mongo_conn_pools, tag);
    if(client){
        SAFE_MALLOC(ludb_conn_t, conn);
        conn->from_pool = true;
        conn->type = ludb_db_oracle;
        conn->conn = (void *)client;
        return conn;
    }
    return NULL;
}

bool ludb_mongo_free_conn(ludb_conn_t *conn) {
    if(conn->from_pool) {
        mongoc_client_destroy((mongoc_client_t*)conn->conn);
    } else {
    }
    free(conn);
    return true;
}

ludb_stmt_t* ludb_mongo_create_stmt(ludb_conn_t *conn) {
    SAFE_MALLOC(monogo_stmt_t, st);
    SAFE_MALLOC(ludb_stmt_t, stmt);
    stmt->type = ludb_db_mongo;
    stmt->conn = conn;
    stmt->stmt = (void*)st;

    return stmt;
}

bool ludb_mongo_free_stmt(ludb_stmt_t *stmt) {
    monogo_stmt_t *st = (monogo_stmt_t*)stmt->stmt;
    if(st){
        QUEUE_DESTORY(st->queue_cols, string_t*, string_destroy);
        MAP_DESTORY(st->bind_cols, string_t*, string_t*, string_destroy, string_destroy);
        MAP_DESTORY(st->bind_values, string_t*, string_t*, string_destroy, string_destroy);
        free(st);
    }
    free(stmt);
    return true;
}

bool ludb_mongo_execute_stmt(ludb_stmt_t *stmt, char *sql) {
    ludb_mongo_prepare(stmt, sql);
    ludb_mongo_execute(stmt);
    return true;
}

bool ludb_mongo_prepare(ludb_stmt_t *stmt, char *sql) {
    char *p1=NULL, *p2=NULL, *p3=NULL;
    monogo_stmt_t *st = (monogo_stmt_t*)stmt->stmt;
    if(!strncasecmp(sql,"insert", 6)){
        st->type = stmt_type_insert;
        p1 = sql+7;
        while(*p1!=' '){
            if(p3==NULL){
            }
        }
    }else if(!strncasecmp(sql,"update",6)){
        st->type = stmt_type_update;
    }else {
        return false;
    }
    return true;
}

bool ludb_mongo_bind_int(ludb_stmt_t *stmt, char *name, int *data) {
    monogo_stmt_t *st = (monogo_stmt_t*)stmt->stmt;
    string_t *key = create_string();
    string_t *val = create_string();
    char str[20] = {0};
    sprintf(str, "%d", *data);
    string_init_cstr(key, name);
    string_init_cstr(val, str);

    MAP_INSERT(st->bind_values, string_t*, key, string_t, val);
    return true;
}

bool ludb_mongo_bind_str(ludb_stmt_t *stmt, char *name, char *data, int len) {
    monogo_stmt_t *st = (monogo_stmt_t*)stmt->stmt;
    string_t *key = create_string();
    string_t *val = create_string();
    string_init_cstr(key, name);
    string_init_subcstr(val, data, len);

    MAP_INSERT(st->bind_values, string_t*, key, string_t, val);
    return true;
}

bool ludb_mongo_execute(ludb_stmt_t *stmt) {
    monogo_stmt_t *st = (monogo_stmt_t*)stmt->stmt;
    mongoc_client_t *client = (mongoc_client_t*)stmt->conn->conn;
    mongoc_collection_t *collection = mongoc_client_get_collection (client, "mydb", string_c_str(st->collection));
    if (st->type == stmt_type_insert) {
        bson_error_t error;
        bson_oid_t oid;
        bson_t *doc;
        doc = bson_new();
        bson_oid_init(&oid, NULL);
        BSON_APPEND_OID(doc, "_id", &oid);
        BSON_APPEND_UTF8(doc, "hello", "world");
        if (!mongoc_collection_insert (collection, MONGOC_INSERT_NONE, doc, NULL, &error)) {
            fprintf(stderr, "%s\n", error.message);
        }
        bson_destroy (doc);
    } else if(st->type == stmt_type_update){
        bson_error_t error;
        bson_oid_t oid;
        bson_t *doc = NULL;
        bson_t *update = NULL;
        bson_t *query = NULL;
        doc = BCON_NEW ("_id", BCON_OID (&oid), "key", BCON_UTF8 ("old_value"));
        if (!mongoc_collection_insert (collection, MONGOC_INSERT_NONE, doc, NULL, &error)) {
            fprintf (stderr, "%s\n", error.message);
            goto fail;
        }
        query = BCON_NEW ("_id", BCON_OID (&oid));
        update = BCON_NEW ("$set",
            "{",
            "key",
            BCON_UTF8 ("new_value"),
            "updated",
            BCON_BOOL (true),
            "}");
        if (!mongoc_collection_update (collection, MONGOC_UPDATE_NONE, query, update, NULL, &error)) {
            fprintf (stderr, "%s\n", error.message);
            goto fail;
        }
fail:
        if (doc)
            bson_destroy (doc);
        if (query)
            bson_destroy (query);
        if (update)
            bson_destroy (update);
    } else if(st->type == stmt_type_delete){
        mongoc_cursor_t *cursor;
        bson_error_t error;
        bson_oid_t oid;
        bson_t *doc;
        bson_t *query;
        char *str;
        query = bson_new();
        BSON_APPEND_UTF8 (query, "hello", "world");
        cursor = mongoc_collection_find_with_opts (collection, query, NULL, NULL);
        while (mongoc_cursor_next (cursor, &doc)) {
            if (!mongoc_collection_remove (collection, MONGOC_REMOVE_SINGLE_REMOVE, doc, NULL, &error)) {
                fprintf (stderr, "Delete failed: %s\n", error.message);
                printf ("Delete failed: %s\n", error.message);
            }
            str = bson_as_json (doc, NULL);
            printf ("%s\n", str);
            bson_free (str);
        }
        bson_destroy (doc);
        mongoc_cursor_destroy (cursor);
    } else if(st->type == stmt_type_select) {
        const bson_t *doc;
        bson_t *query;

        query = bson_new ();
        st->cursor = mongoc_collection_find_with_opts (collection, query, NULL, NULL);
        bson_destroy (query);
    }
    mongoc_collection_destroy (collection);
    return true;
}

uint32_t ludb_mongo_affected_rows(ludb_stmt_t *stmt) {
    return 0;
}

bool ludb_mongo_commit(ludb_conn_t *conn) {
    return true;
}

ludb_rest_t* ludb_mongo_result(ludb_stmt_t *stmt) {
    monogo_stmt_t *st = (monogo_stmt_t*)stmt->stmt;
    if(st->cursor) {
        SAFE_MALLOC(ludb_rest_t, rest);
        rest->type = ludb_db_oracle;
        rest->stmt = stmt;
        rest->conn = stmt->conn;
        rest->rest = (void*)st->cursor;
        return rest;
    }
    return NULL;
}

bool ludb_mongo_result_next(ludb_rest_t *res) {
    mongoc_cursor_t *cursor = (mongoc_cursor_t*)res->rest;
    const bson_t *doc;
    bool ret = mongoc_cursor_next(cursor, &doc);
    char *str = bson_as_json (doc, NULL);
    printf ("%s\n", str);
    bson_free (str);
    return ret;
}

char* ludb_mongo_rest_get_char(ludb_rest_t *res, uint32_t i) {
    
    return "";
}

int ludb_mongo_rest_get_int(ludb_rest_t *res, uint32_t i) {
    return 0;
}

char* ludb_mongo_rest_get_date(ludb_rest_t *res, uint32_t i, char *buff) {
    return buff;
}

char* ludb_mongo_rest_get_blob(ludb_rest_t *res, uint32_t i) {
    return NULL;
}

//////////////////////////////////////////////////////////////////////////

/** 以下是批量处理方法 */

typedef struct _ludb_batch_mongo_ {
    char*                   buff;     //< 内存区域
}ludb_batch_mongo_t;


bool create_ludb_batch_mongo(ludb_batch_t *h) {
    return true;
}

void destory_ludb_batch_mongo(ludb_batch_t* h) {
    
}

bool ludb_batch_insert_mongo(ludb_batch_t* h) {
    return true;
}