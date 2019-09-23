#include "ludb_mongo.h"
#include "ludb_private.h"
#include "mongoc.h"

//#pragma comment(lib, "libmongoc.lib")

static map<string,mongoc_client_t*> mongo_conn_pools; //map<string, mongoc_client_t*>

/** 操作类型 */
typedef enum _stmt_type_ {
    stmt_type_unknown = 0,
    stmt_type_insert,
    stmt_type_delete,
    stmt_type_update,
    stmt_type_select
}stmt_type_t;

/** 将mongo执行方式转为类似rmdb sql */
struct mongo_stmt_t {
    stmt_type_t     type;           //操作类型
    string          collection;    //集合名称
    queue<string>   queue_cols;    //操作影响的所有field
    map<string, string> bind_cols;     //操作名称-对应-绑定标记或者固定值
    map<string, string> bind_values;   //绑定标记-对应-值
    mongoc_cursor_t *cursor;        //查询结果
};

bool ludb_mongo_init() {
    mongoc_init();
    return true;
}

void ludb_mongo_clean() {
    mongoc_cleanup();
}

ludb_conn_t* ludb_mongo_connect(const char *database, const char *usr, const char *pwd) {
    char path[MAX_PATH] = {0};
    sprintf(path, "mongodb://%s:%s@%s", usr, pwd, database);
    mongoc_client_t *client = mongoc_client_new(path);
    if(client) {
        ludb_mongo_conn *conn = new ludb_mongo_conn();
        conn->from_pool = false;
        conn->type = ludb_db_mongo;
        conn->conn = (void *)client;
        return conn;
    }

    return NULL;
}

bool ludb_mongo_create_pool(const char *tag, const char *database, const char *usr, const char *pwd, uint32_t max, uint32_t min, uint32_t inc) {
    if(mongo_conn_pools.count(tag)) {
        Log::error("mongo pool tag already exist");
        return false;
    }

        char path[MAX_PATH] = {0};
        sprintf(path, "mongodb://%s:%s@%s?maxPoolSize=%d&minPoolSize=%d", usr, pwd, database, max, min);
        mongoc_client_t *client = mongoc_client_new(path);
        if(client) {
            mongo_conn_pools.insert(make_pair(tag, client));
            return true;
        }
    return false;
}

ludb_conn_t* ludb_mongo_pool_connect(const char *tag){
    if(!mongo_conn_pools.count(tag)){
        Log::error("mongo connect of tag[%s] is not exist", tag);
        return NULL;
    }
    mongoc_client_t *client = mongo_conn_pools[tag];
    if(client){
        ludb_mongo_conn *conn = new ludb_mongo_conn();
        conn->from_pool = true;
        conn->type = ludb_db_mongo;
        conn->conn = (void *)client;
        return conn;
    }
    return NULL;
}

//////////////////////////////////////////////////////////////////////////

ludb_mongo_conn::ludb_mongo_conn() {

}

ludb_mongo_conn::~ludb_mongo_conn() {
    if(!from_pool) {
        mongoc_client_destroy((mongoc_client_t*)conn);
    }
}

ludb_stmt_t* ludb_mongo_conn::create_stmt() {
    mongo_stmt_t *most = new mongo_stmt_t();
    ludb_mongo_stmt* stmt = new ludb_mongo_stmt();
    stmt->type = ludb_db_mongo;
    stmt->conn = this;
    stmt->stmt = (void*)most;
    return stmt;
}

bool ludb_mongo_conn::ludb_commit() {
    return true;
}

//////////////////////////////////////////////////////////////////////////

ludb_mongo_stmt::ludb_mongo_stmt() {

}

ludb_mongo_stmt::~ludb_mongo_stmt() {
    mongo_stmt_t *st = (mongo_stmt_t*)stmt;
    delete st;
}

bool ludb_mongo_stmt::execute(const char *sql) {
    if(!prepare(sql))
        return false;
    return execute();
}

bool ludb_mongo_stmt::prepare(const char *sql) {
    char *p1=NULL, *p2=NULL, *p3=NULL;
    mongo_stmt_t *st = (mongo_stmt_t*)stmt;
    if(!strncasecmp(sql,"insert", 6)){
        st->type = stmt_type_insert;
        p1 = (char*)sql+7;
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

bool ludb_mongo_stmt::bind_int(const char *name, int *data) {
    mongo_stmt_t *st = (mongo_stmt_t*)stmt;

    char str[20] = {0};
    sprintf(str, "%d", *data);
    st->bind_values.insert(make_pair(name, str));

    return true;
}

bool ludb_mongo_stmt::bind_str(const char *name, const char *data, int len) {
    mongo_stmt_t *st = (mongo_stmt_t*)stmt;
    string str(data, len);
    st->bind_values.insert(make_pair(name, str));

    return true;
}

bool ludb_mongo_stmt::execute() {
    mongo_stmt_t *st = (mongo_stmt_t*)stmt;
    mongoc_client_t *client = (mongoc_client_t*)conn->conn;
    mongoc_collection_t *collection = mongoc_client_get_collection (client, "mydb", st->collection.c_str());
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
        while (mongoc_cursor_next (cursor, (const bson_t**)&doc)) {
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
    mongoc_collection_destroy(collection);
    return true;
}

uint32_t ludb_mongo_stmt::affected_rows() {
    return 0;
}

ludb_rest_t* ludb_mongo_stmt::result() {
    mongo_stmt_t *st = (mongo_stmt_t*)stmt;
    if(st->cursor) {
        ludb_mongo_rest_t *rest = new ludb_mongo_rest_t();
        rest->type = ludb_db_mongo;
        rest->stmt = this;
        rest->conn = conn;
        rest->rest = (void*)st->cursor;
        return rest;
    }
    return NULL;
}

//////////////////////////////////////////////////////////////////////////

ludb_mongo_rest_t::ludb_mongo_rest_t() {

}

ludb_mongo_rest_t::~ludb_mongo_rest_t() {

}

bool ludb_mongo_rest_t::next() {
    mongoc_cursor_t *cursor = (mongoc_cursor_t*)rest;
    const bson_t *doc;
    bool ret = mongoc_cursor_next(cursor, &doc);
    char *str = bson_as_json (doc, NULL);
    printf ("%s\n", str);
    bson_free (str);
    return ret;
}

string ludb_mongo_rest_t::get_char(uint32_t i){
    return "";
}

int ludb_mongo_rest_t::get_int(uint32_t i){
    return 0;
}

string ludb_mongo_rest_t::get_date(uint32_t i){
    return "";
}

string ludb_mongo_rest_t::get_blob(uint32_t i){
    return "";
}

//////////////////////////////////////////////////////////////////////////

/** 以下是批量处理方法 */

ludb_mongo_batch::ludb_mongo_batch(string Tag, string Sql, int RowNum, int Interval, bind_column_t* Binds)
    : ludb_batch_t(Tag, Sql, RowNum, Interval, Binds)
{
}

ludb_mongo_batch::~ludb_mongo_batch() {

}

bool ludb_mongo_batch::insert() {
    return true;
}