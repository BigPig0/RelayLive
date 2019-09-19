#include "ludb_oracle.h"
#include "ludb_private.h"
#include "ocilib.h"

static map_t *oracle_conn_pools = NULL; //map<string, OCI_ConnPool*>

static void oci_error_handler( OCI_Error *err ) {
    OCI_Connection * conn = OCI_ErrorGetConnection(err);
    char error_info[2000] = {0};
    sprintf(error_info, "Error[ORA-%05d] - msg[%s] - database[%s] - user[%s] - sql[%s]"
        , OCI_ErrorGetOCICode(err)
        , OCI_ErrorGetString(err)
        , OCI_GetDatabase(conn)
        , OCI_GetUserName(conn)
        , OCI_GetSql(OCI_ErrorGetStatement(err)));
    g_log_hook(error_info);
};

bool ludb_oracle_init(const char *path /*= NULL*/) {
    if(!OCI_Initialize(oci_error_handler, path, OCI_ENV_DEFAULT | OCI_ENV_THREADED))
        return false;

    return true;
}

void ludb_oracle_clean() {
    MAP_DESTORY(oracle_conn_pools, string_t*, OCI_Pool*, string_destroy, OCI_PoolFree);
    OCI_Cleanup();
}

ludb_conn_t* ludb_oracle_connect(char *database, char *usr, char *pwd) {
    OCI_Connection *cn = OCI_CreateConnection(database, usr, pwd, OCI_SESSION_DEFAULT);
    if(cn){
        SAFE_MALLOC(ludb_conn_t, conn);
        conn->from_pool = false;
        conn->type = ludb_db_oracle;
        conn->conn = (void *)cn;
        return conn;
    }
    return NULL;
}

bool ludb_oracle_create_pool(char *tag, char *database, char *usr, char *pwd, uint32_t max, uint32_t min, uint32_t inc) {
    if(oracle_conn_pools == NULL) {
        oracle_conn_pools = create_map(void*,void*);
        map_init_ex(oracle_conn_pools, string_map_compare);
    }
    if(map_find_easy_str(oracle_conn_pools, tag)) {
        g_log_hook("oracle pool tag already exist");
        return false;
    } else {
        OCI_ConnPool *pool = OCI_PoolCreate(database, usr, pwd, OCI_POOL_SESSION, OCI_SESSION_DEFAULT, min, max, inc);
        if(pool){
            string_t *key = create_string();
            string_init_cstr(key, tag);
            map_insert_easy(oracle_conn_pools, key, pool);
            return true;
        }
    }
    return false;
}

ludb_conn_t* ludb_oracle_pool_connect(char *tag){
    OCI_ConnPool *pool = (OCI_ConnPool*)map_find_easy_str(oracle_conn_pools, tag);
    if(pool){
        OCI_Connection *cn = OCI_PoolGetConnection(pool, NULL);
        if(cn){
            SAFE_MALLOC(ludb_conn_t, conn);
            conn->from_pool = true;
            conn->type = ludb_db_oracle;
            conn->conn = (void *)cn;
            return conn;
        }
    }
    return NULL;
}

bool ludb_oracle_free_conn(ludb_conn_t *conn) {
    OCI_Connection *cn = (OCI_Connection *)conn->conn;
    free(conn);
    return OCI_ConnectionFree(cn);
}

ludb_stmt_t* ludb_oracle_create_stmt(ludb_conn_t *conn) {
    OCI_Statement *sm = OCI_CreateStatement((OCI_Connection *)conn->conn);
    if(sm){
        SAFE_MALLOC(ludb_stmt_t, stmt);
        stmt->type = ludb_db_oracle;
        stmt->conn = conn;
        stmt->stmt = (void*)sm;
		return stmt;
    }
    return NULL;
}

bool ludb_oracle_free_stmt(ludb_stmt_t *stmt) {
    OCI_Statement *sm = (OCI_Statement *)stmt->stmt;
    free(stmt);
    return OCI_FreeStatement(sm);
}

bool ludb_oracle_execute_stmt(ludb_stmt_t *stmt, char *sql) {
    return OCI_ExecuteStmt((OCI_Statement *)stmt->stmt, sql);
}

bool ludb_oracle_prepare(ludb_stmt_t *stmt, char *sql) {
    return OCI_Prepare((OCI_Statement *)stmt->stmt, sql);
}

bool ludb_oracle_bind_int(ludb_stmt_t *stmt, char *name, int *data) {
    return OCI_BindInt((OCI_Statement *)stmt->stmt, name, data);
}

bool ludb_oracle_bind_str(ludb_stmt_t *stmt, char *name, char *data, int len) {
    return OCI_BindString((OCI_Statement *)stmt->stmt, name, data, len);
}

bool ludb_oracle_execute(ludb_stmt_t *stmt) {
    return OCI_Execute((OCI_Statement *)stmt->stmt);
}

uint32_t ludb_oracle_affected_rows(ludb_stmt_t *stmt) {
    return OCI_GetAffectedRows((OCI_Statement *)stmt->stmt);
}

bool ludb_oracle_commit(ludb_conn_t *conn) {
    return OCI_Commit((OCI_Connection *)conn->conn);
}

ludb_rest_t* ludb_oracle_result(ludb_stmt_t *stmt) {
    OCI_Resultset *rs = OCI_GetResultset((OCI_Statement *)stmt->stmt);
    if(rs) {
        SAFE_MALLOC(ludb_rest_t, rest);
        rest->type = ludb_db_oracle;
        rest->stmt = stmt;
        rest->conn = stmt->conn;
        rest->rest = (void*)rs;
        return rest;
    }
    return NULL;
}

bool ludb_oracle_result_next(ludb_rest_t *res) {
    return OCI_FetchNext((OCI_Resultset *)res->rest);
}

char* ludb_oracle_rest_get_char(ludb_rest_t *res, uint32_t i) {
    OCI_Resultset *rs = (OCI_Resultset *)res->rest;
    char* pTmp;
    if(OCI_IsNull(rs, i))
        return "";
    pTmp = (char*)OCI_GetString(rs, i);
    if (pTmp)
        return pTmp;
    return "";
}

int ludb_oracle_rest_get_int(ludb_rest_t *res, uint32_t i) {
    OCI_Resultset *rs = (OCI_Resultset *)res->rest;
    return OCI_IsNull(rs, i)?0:OCI_GetInt(rs, i);
}

char* ludb_oracle_rest_get_date(ludb_rest_t *res, uint32_t i, char *buff) {
    OCI_Resultset *rs = (OCI_Resultset *)res->rest;
    OCI_Date * dat;
    if(OCI_IsNull(rs, i))
        return "";
    dat = OCI_GetDate(rs, i);
    if(!OCI_DateToText(dat,"yyyymmddhh24miss",20,buff))
        return "";

    OCI_DateFree(dat);
    return buff;
}

char* ludb_oracle_rest_get_blob(ludb_rest_t *res, uint32_t i) {
    OCI_Resultset *rs = (OCI_Resultset *)res->rest;
    OCI_Lob *lob;
    big_uint len;
	big_uint readlen;
    char* buff;
    if(OCI_IsNull(rs, i))
        return "";

    lob = OCI_GetLob(rs,i);
    len = OCI_LobGetLength(lob)*2;
    buff = (char*)calloc(1,len+1);

	readlen = OCI_LobRead(lob, buff, len);
    if(0 == readlen)
        return buff;

    OCI_LobFree(lob);
    return buff;
}

//////////////////////////////////////////////////////////////////////////

/** 以下是批量处理方法 */

typedef struct _ludb_batch_oracle_ {
    char*                   buff;     //< 内存区域
}ludb_batch_oracle_t;

static void str_set(char *target, int index, int len, const char *source) {
    int offset = index * (len + 1);
    char *p = target + offset;
    const char *q = source;
    int count = 0;
    memset(p, 0, len + 1);
	p[0] = ' ';
    while (q && *q != '\0' && count < len){
        *p++ = *q++;
        ++count;
    }
}

static void bind_set_data(OCI_Bind *bind, int index, int len, string_t *val, bool is_null /*= false*/){
    if(!is_null) {
        char *str = (char *)OCI_BindGetData(bind);
        str_set(str, index, len, string_c_str(val));
    } else {
        OCI_BindSetNullAtPos(bind, index + 1);
    }
}

static void bind_set_date(OCI_Bind *bind, int index, string_t *val, char *date_fmt, bool is_null /*= false*/){
    if(!is_null){
        OCI_Date **dates = (OCI_Date **)OCI_BindGetData(bind);
        OCI_DateFromText(dates[index], string_c_str(val), date_fmt);
    } else {
        OCI_BindSetNullAtPos(bind, index + 1); //下标从1开始
    }
}

static void bind_set_int16(OCI_Bind *bind, int index, int16_t val, bool is_null /*= false*/){
    if(!is_null) {
        int16_t *numbers = (int16_t *)OCI_BindGetData(bind);
        numbers[index] = val;
    } else {
        OCI_BindSetNullAtPos(bind, index + 1); //下标从1开始
    }
}

static void bind_set_int32(OCI_Bind *bind, int index, int32_t val, bool is_null /*= false*/){
    if(!is_null) {
        int32_t *numbers = (int32_t *)OCI_BindGetData(bind);
        numbers[index] = val;
    } else {
        OCI_BindSetNullAtPos(bind, index + 1); //下标从1开始
    }
}

static void bind_set_int64(OCI_Bind *bind, int index, int64_t val, bool is_null /*= false*/){
    if(!is_null) {
        int64_t *numbers = (int64_t *)OCI_BindGetData(bind);
        numbers[index] = val;
    } else {
        OCI_BindSetNullAtPos(bind, index + 1); //下标从1开始
    }
}

bool create_ludb_batch_oracle(ludb_batch_t *h) {
    char *pBindBegin = NULL;
    char *pDataBegin = NULL;
    int32_t nOffsetBind = 0;
    int32_t nOffsetData = 0;
    int32_t colNum = vector_size(h->binds);    // 列数
    int32_t nBindSize = colNum * pointLen;    //绑定区域的大小
    int32_t allSize = 0;    //数据区域的大小
    SAFE_MALLOC(ludb_batch_oracle_t, ho);
    VECTOR_FOR_BEGIN(h->binds, bind_column_pri_t*, col){
        int Length = col->max_len;
        if(col->type==column_type_char)
            Length++;
        allSize += Length * h->row_num;
    }VECTOR_FOR_END

    h->handle = (void*)ho;

    //申请内存
    ho->buff = (char*)calloc(1, nBindSize + allSize);
    CHECK_POINT_BOOL(ho->buff);

    //绑定区域
    pBindBegin = ho->buff;
    //数据区域
    pDataBegin = pBindBegin + nBindSize;

    //将绑定数据地址指向数据区域
    VECTOR_FOR_BEGIN(h->binds, bind_column_pri_t*, col){
        int Length;
        char* tmp = pDataBegin + nOffsetData;
        memcpy(pBindBegin+nOffsetBind, &tmp, pointLen);

        if (col->type == column_type_blob) {
            OCI_Lob **blob = (OCI_Lob **)(pDataBegin+nOffsetData);
            int i = 0;
            for (; i < h->row_num; i++) {
                blob[i] = OCI_LobCreate(NULL,OCI_BLOB);
            }
        } else if (col->type == column_type_date) {
            OCI_Date **da = (OCI_Date **)(pDataBegin+nOffsetData);
            int i = 0;
            for (; i < h->row_num; i++) {
                da[i] = OCI_DateCreate(NULL);
            }
        }

        nOffsetBind += pointLen;
        Length = col->type==column_type_char?col->max_len+1:col->max_len;
        nOffsetData += Length * h->row_num;
    }VECTOR_FOR_END
    return true;
}

void destory_ludb_batch_oracle(ludb_batch_t* h) {
    ludb_batch_oracle_t *ho = (ludb_batch_oracle_t*)h->handle;
    char* pDataBegin = ho->buff + vector_size(h->binds) * pointLen;
    int32_t nOffsetData = 0;
    VECTOR_FOR_BEGIN(h->binds, bind_column_pri_t*, col){
        int Length = col->max_len;
        if(col->type==column_type_char)
            Length++;
        if (col->type == column_type_blob) {
            OCI_Lob **blob = (OCI_Lob **)(pDataBegin+nOffsetData);
            int i = 0;
            for (; i < h->row_num; i++) {
                OCI_LobFree(blob[i]);
            }
        } else if (col->type == column_type_date) {
            OCI_Date **da = (OCI_Date **)(pDataBegin+nOffsetData);
            int i = 0;
            for (; i < h->row_num; i++) {
                OCI_DateFree(da[i]);
            }
        }
        nOffsetData += Length * h->row_num;
    }VECTOR_FOR_END
    free(ho->buff);
    free(ho);
}

bool ludb_batch_insert_oracle(ludb_batch_t* h) {
    ludb_batch_oracle_t *ho = (ludb_batch_oracle_t*)h->handle;
    OCI_Connection *cn;
    ludb_conn_t *conn;
    OCI_Statement *st;
    int nRowSize = vector_size(h->insts);
    if (nRowSize > h->row_num) nRowSize = h->row_num; //正常不会走到这一步
    if(true){
        char buff[100]={0};
        sprintf(buff,"insert %d rows to db", nRowSize);
        g_log_hook(buff);
    }

    // 获取数据库连接
    conn = ludb_oracle_pool_connect((char*)string_c_str(h->tag));
    if(!conn || !conn->conn){
        char buff[100]={0};
        sprintf(buff, "fail to get connection: %s", string_c_str(h->tag));
        g_log_hook(buff);
        return false;
    }
    cn = (OCI_Connection*)conn->conn;
    st = OCI_CreateStatement(cn);

    if(vector_empty(h->binds)){
        VECTOR_FOR_BEGIN(h->insts, vector_t*, row)
            VECTOR_FOR_BEGIN(row, string_t*, sql)
                OCI_ExecuteStmt(st, string_c_str(sql));
            VECTOR_FOR_END
        VECTOR_FOR_END
        OCI_Commit(cn);
    } else {
        int column_num = vector_size(h->binds);
        int nOffsetBind = 0;
        char* pBindBegin = ho->buff;
        int i,j;

        //准备执行sql
        OCI_SetBindAllocation(st, OCI_BAM_EXTERNAL);
        OCI_Prepare(st, string_c_str(h->sql));
        OCI_BindArraySetSize(st, nRowSize);

        //绑定
        VECTOR_FOR_BEGIN(h->binds, bind_column_pri_t*, col)
            if (col->type == column_type_char) {
                    char* pBuff;
                    memcpy(&pBuff, pBindBegin+nOffsetBind, pointLen);
                    OCI_BindArrayOfStrings(st, string_c_str(col->name), pBuff, col->max_len, 0);
            } else if(col->type == column_type_int) {
                    int32_t* pBuff;
                    memcpy(&pBuff, pBindBegin+nOffsetBind, pointLen);
                    OCI_BindArrayOfInts(st, string_c_str(col->name), pBuff, 0);
            } else if(col->type == column_type_float) {
                    double* pBuff;
                    memcpy(&pBuff, pBindBegin+nOffsetBind, pointLen);
                    OCI_BindArrayOfDoubles(st, string_c_str(col->name), pBuff, 0);
            } else if(col->type == column_type_long) {
                    big_int* pBuff;
                    memcpy(&pBuff, pBindBegin+nOffsetBind, pointLen);
                    OCI_BindArrayOfBigInts(st, string_c_str(col->name), pBuff, 0);
            } else if(col->type == column_type_uint) {
                    uint32_t* pBuff;
                    memcpy(&pBuff, pBindBegin+nOffsetBind, pointLen);
                    OCI_BindArrayOfUnsignedInts(st, string_c_str(col->name), pBuff, 0);
            } else if(col->type == column_type_date) {
                    OCI_Date** pBuff;
                    memcpy(&pBuff, pBindBegin+nOffsetBind, pointLen);
                    OCI_BindArrayOfDates(st, string_c_str(col->name), pBuff, 0);
            } else if(col->type == column_type_blob) {
                    OCI_Lob** pBuff;
                    memcpy(&pBuff, pBindBegin+nOffsetBind, pointLen);
                    OCI_BindArrayOfLobs(st, string_c_str(col->name), pBuff, OCI_BLOB, 0);
            }
            nOffsetBind += pointLen;
        VECTOR_FOR_END
        for(i = 0; i < nRowSize; ++i) {
            vector_t *row = *(vector_t**)vector_at(h->insts, i);
            int nLineColumnSize = vector_size(row);
            for (j = 0; j<column_num; ++j) {
                bind_column_pri_t *col_info = *(bind_column_pri_t**)vector_at(h->binds, j);
                string_t *value = j<nLineColumnSize?*(string_t**)vector_at(row,j):col_info->default_value;
                bool isnull = col_info->nullable?string_empty(value):false;

                if(col_info->type == column_type_char) {
                    bind_set_data(OCI_GetBind(st, j+1), i, col_info->max_len, value, isnull);
                } else if (col_info->type == column_type_int 
                    || col_info->type == column_type_uint
                    || col_info->type == column_type_float) {
                    int32_t n = string_empty(value)?0:atoi(string_c_str(value));
                    bind_set_int32(OCI_GetBind(st, j+1), i, n, isnull);
                } else if (col_info->type == column_type_long) {
                    int64_t n = string_empty(value)?0:_atoi64(string_c_str(value));
                    bind_set_int64(OCI_GetBind(st, j+1), i, n, isnull);
                } else if (col_info->type == column_type_date) {
					string_t *val = string_substr(value, 0, 14);
                    bind_set_date(OCI_GetBind(st, j+1), i, val, "yyyymmddhh24miss", isnull);
					string_destroy(val);
                }
            }
        }

        if(true){
            boolean bExcute = OCI_Execute(st);
            boolean bCommit = OCI_Commit(cn);
            unsigned int count = OCI_GetAffectedRows(st);    //某一行插入失败，不会回滚所有数据，但是出错后count为0，这是ocilib的一个bug
            char buff[100]={0};
            if (!bExcute || !bCommit || 0 == count){
                sprintf(buff, "Execute %s fail bExcute: %d; bCommit: %d; count: %d" ,string_c_str(h->tag), bExcute, bCommit, count);
            } else {
                sprintf(buff, "Execute %s sucess bExcute: %d; bCommit: %d; count: %d" ,string_c_str(h->tag), bExcute, bCommit, count);
            }
            g_log_hook(buff);
        }
    }
    OCI_FreeStatement(st);
    ludb_oracle_free_conn(conn);
    return true;
}