#include "ludb_lua.h"
#include "luapp.hpp"
#include "ludb.h"
#include "ludb_batch.h"
#include "ludb_private.h"
#include <string>
using namespace std;

ludb_db_type_t lu_get_type(lua::Str type){
    if(type == "oracle") {
#ifdef DB_ORACLE
        return ludb_db_oracle;
#endif
    } else if(type == "mongodb") {
#ifdef DB_MONGO
        return ludb_db_mongo;
#endif
    } else if(type == "redis") {
#ifdef DB_REDIS
        return ludb_db_redis;
#endif
    }
    return ludb_db_unknow;
}

lua::Bool luaDbInit(lua::Table pra){
    ludb_db_type_t t = lu_get_type(pra["dbtype"]);
    if(t == ludb_db_oracle){
#ifdef DB_ORACLE
        if(pra.isExist(string("path"))){
            lua::Str path = pra["path"];
            return ludb_init_oracle(path.c_str());
        } else {
            return ludb_init_oracle(NULL);
        }
#endif
    } else if(t == ludb_db_mongo) {
#ifdef DB_MONGO
        return ludb_init_mongo();
#endif
    } else if(t == ludb_db_redis) {
#ifdef DB_REDIS
        return ludb_init_redis();
#endif
    }
    return false;
}
lua::Bool luaDbClean(lua::Str type){
    ludb_db_type_t t = lu_get_type(type);
    if(t == ludb_db_unknow)
        return false;
    ludb_clean(t);
    return true;
}

lua::Ptr luaConnect(lua::Table pra) {
    lua::Str database = pra["dbpath"];
    lua::Str username = pra["user"];
    lua::Str password = pra["pwd"];
    ludb_db_type_t t = lu_get_type(pra["dbtype"]);
    if(t == ludb_db_unknow){
        return false;
    }
    return ludb_connect(t, database.c_str(), username.c_str(), password.c_str());
}
lua::Bool luaCreatePool(lua::Table pra){
    lua::Str tag = pra["tag"];
    lua::Str database = pra["dbpath"];
    lua::Str username = pra["user"];
    lua::Str password = pra["pwd"];
    int max_conns = pra.isExist(string("max"))?lua::VarCast<lua::Int>(pra["max"]):1;
    int min_conns = pra.isExist(string("min"))?lua::VarCast<lua::Int>(pra["min"]):0;
    int inc_conns = pra.isExist(string("inc"))?lua::VarCast<lua::Int>(pra["inc"]):1;
    ludb_db_type_t t = lu_get_type(pra["dbtype"]);
    if(t == ludb_db_unknow){
        return false;
    }
    return ludb_create_pool(t
        , (char*)tag.c_str()
        , (char*)database.c_str()
        , (char*)username.c_str()
        , (char*)password.c_str()
        , max_conns, min_conns, inc_conns);
}
lua::Ptr luaPoolConnect(lua::Str type, lua::Str tag){
    ludb_db_type_t t = lu_get_type(type);
    if(t == ludb_db_unknow){
        return false;
    }
    return (void*)ludb_pool_connect(t, (char*)tag.c_str());
}
lua::Bool luaFreeConnect(lua::Ptr con){
    return ludb_free_conn((ludb_conn_t *)con);
}
lua::Ptr luaCreateStatement(lua::Ptr con){
    return (void*)ludb_create_stmt((ludb_conn_t *)con);
}
lua::Bool luaFreeStatement(lua::Ptr stmt){
    return ludb_free_stmt((ludb_stmt_t *)stmt);
}
lua::Bool luaExecuteStmt(lua::Ptr stmt, lua::Str sql){
    return ludb_execute_stmt((ludb_stmt_t *)stmt, (char*)sql.c_str());
}
lua::Bool luaPrepare(lua::Ptr stmt, lua::Str sql){
    return ludb_prepare((ludb_stmt_t *)stmt, (char*)sql.c_str());
}
lua::Bool luaBindInt(lua::Ptr stmt, lua::Str name, lua::Ptr data){
    return ludb_bind_int((ludb_stmt_t *)stmt, (char*)name.c_str(), (int*)data);
}
lua::Bool luaBindString(lua::Ptr stmt, lua::Str name, lua::Ptr data, lua::Int maxLen){
    return ludb_bind_str((ludb_stmt_t *)stmt, (char*)name.c_str(), (char*)data, maxLen);
}
lua::Bool luaExecute(lua::Ptr stmt){
    return ludb_execute((ludb_stmt_t *)stmt);
}
lua::Int luaGetAffectedRows(lua::Ptr stmt){
    return ludb_affected_rows((ludb_stmt_t *)stmt);
}
lua::Int luaCommit(lua::Ptr con){
    return ludb_commit((ludb_conn_t *)con);
}
lua::Ptr luaGetResultset(lua::Ptr stmt){
    return ludb_result((ludb_stmt_t *)stmt);
}
lua::Bool luaFreeResultset(lua::Ptr rs){
    ludb_free_result((ludb_rest_t*)rs);
    return true;
}
lua::Bool luaFetchNext(lua::Ptr rs){
    return ludb_result_next((ludb_rest_t *)rs);
}
lua::Str luaGetString(lua::Ptr rs, lua::Int i){
    return ludb_rest_get_char((ludb_rest_t*)rs, i);
}
lua::Int luaGetInt(lua::Ptr rs, lua::Int i){
    return ludb_rest_get_int((ludb_rest_t*)rs, i);
}
lua::Str luaGetOdt(lua::Ptr rs, lua::Int i){
    return ludb_rest_get_date((ludb_rest_t*)rs, i);
}
lua::Str luaGetBlob(lua::Ptr rs, lua::Int i){
    string blob = ludb_rest_get_blob((ludb_rest_t*)rs, i);
    return blob;
}
lua::Ptr luaBatchInit(lua::Str type, lua::Str tag, lua::Str sql, lua::Int rnum, lua::Int interval, lua::Table binds) {
    ludb_db_type_t t = lu_get_type(type);
    if(t == ludb_db_unknow)
        return NULL;

    bind_column_t *param = (bind_column_t*)calloc(sizeof(bind_column_t), binds.size()+1);
    int i = 0;
    for(auto it = binds.getBegin(); !it.isEnd(); it++,i++){
        lua::Var k,v;
        it.getKeyValue(&k, &v);
        if(!lua::VarType<lua::Table>(v))
            continue;
        lua::Table col = lua::VarCast<lua::Table>(v);
        if(!col.isExist(lua::Str("bindname")) || !lua::VarType<lua::Str>(col["bindname"])){
            Log::error("this column lost bindname");
            continue;
        }
        if(!col.isExist(lua::Str("coltype")) || !lua::VarType<lua::Int>(col["coltype"])){
            Log::error("this column lost coltype");
            continue;
        }

        string &bindName    = lua::VarCast<lua::Str>(col["bindname"]);
        char *pname         = (char*)malloc(bindName.size()+1);
        memcpy(pname, bindName.c_str(), bindName.size());
        pname[bindName.size()] = 0;
        param[i].name = pname;

        param[i].type = (column_type_t)lua::VarCast<lua::Int>(col["coltype"]);

        param[i].max_len     = 16;
        if(col.isExist(lua::Str("maxlen"))){
            if (lua::VarType<lua::Int>(col["maxlen"]))
                param[i].max_len = (uint32_t)lua::VarCast<lua::Int>(col["maxlen"]);
        }

        param[i].nullable =  false;
        if(col.isExist(lua::Str("nullable"))){
            if(lua::VarType<lua::Bool>(col["nullable"]))
                param[i].nullable = lua::VarCast<lua::Bool>(col["nullable"]);
        }

        param[i].default_value = NULL;
        if(col.isExist(lua::Str("def"))){
            if(lua::VarType<lua::Str>(col["def"])) {
                string &strDefault = lua::VarCast<lua::Str>(col["def"]);
                char *pname = (char*)malloc(strDefault.size()+1);
                memcpy(pname, strDefault.c_str(), strDefault.size());
                pname[strDefault.size()] = 0;
                param[i].default_value = pname;
            }
        }
    }
    param[i].name = NULL;
    ludb_batch_t *dbInster = create_ludb_batch(t, (char*)tag.c_str(), (char*)sql.c_str(), rnum, interval, param);

    for(int j=0; j<i; j++){
        if(param[j].name)
            free((char*)param[j].name);
        if(param[j].default_value)
            free((char*)param[j].default_value);
    }
    SAFE_FREE(param);
    return (void*)dbInster;
}
lua::Bool luaAddRow(lua::Ptr rc, lua::Table row){
    ludb_batch_t *h = (ludb_batch_t*)rc;
    int size = row.size();
    char** values = (char**)calloc(size+1, sizeof(char*));
    int i=0;
    for(; i<size; i++) {
		if(lua::VarType<lua::Str>(row[i+1])){
			lua::Str &col = lua::VarCast<lua::Str>(row[i+1]);
			values[i] = (char*)calloc(1, col.size()+1);
			memcpy(values[i], col.c_str(), col.size());
		} else if(lua::VarType<lua::Int>(row[i+1])){
			values[i] = (char*)calloc(1, 20);
			sprintf(values[i], "%d", lua::VarCast<lua::Int>(row[i+1]));
		} else {
			values[i] =  (char*)calloc(1, 1);
		}
    }
    ludb_batch_add_row(h, (const char**)values);
	for(; i<size; i++) {
		free(values[i]);
	}
    free(values);
    return true;
}

void ludb_lua_init(void *lua) {

    lua::State<> *luastate = (lua::State<> *)lua;
    luastate->setFunc("LUDB_INIT",         &luaDbInit);
    luastate->setFunc("LUDB_CLEAN",        &luaDbClean);

    luastate->setFunc("LUDB_CONN",         &luaConnect);
    luastate->setFunc("LUDB_CREAT_POOL",   &luaCreatePool);
    luastate->setFunc("LUDB_POOL_CONN",    &luaPoolConnect);
    luastate->setFunc("LUDB_FREE_CONN",    &luaFreeConnect);
    luastate->setFunc("LUDB_COMMIT",       &luaCommit);

    luastate->setFunc("LUDB_CREATE_STMT",  &luaCreateStatement);
    luastate->setFunc("LUDB_FREE_STMT",    &luaFreeStatement);
    luastate->setFunc("LUDB_EXECUTE_STMT", &luaExecuteStmt);
    luastate->setFunc("LUDB_PREPARE",      &luaPrepare);
    luastate->setFunc("LUDB_BIND_INT",     &luaBindInt);
    luastate->setFunc("LUDB_BIND_STRING",  &luaBindString);
    luastate->setFunc("LUDB_EXECUTE",      &luaExecute);
    luastate->setFunc("LUDB_GET_AFFECT",   &luaGetAffectedRows);
    luastate->setFunc("LUDB_GET_RES",      &luaGetResultset);

    luastate->setFunc("LUDB_FREE_RES",     &luaFreeResultset);
    luastate->setFunc("LUDB_FETCH_NEXT",   &luaFetchNext);

    luastate->setFunc("LUDB_GET_STR",      &luaGetString);
    luastate->setFunc("LUDB_GET_INT",      &luaGetInt);
    luastate->setFunc("LUDB_GET_ODT",      &luaGetOdt);
    luastate->setFunc("LUDB_GET_BLOB",     &luaGetBlob);

    luastate->setFunc("LUDB_BATCH_INIT",   &luaBatchInit);
    luastate->setFunc("LUDB_ADD_ROW",      &luaAddRow);

    luastate->setGlobal("LUDB_TYPE_CHR",   column_type_char);
    luastate->setGlobal("LUDB_TYPE_INT",   column_type_int);
    luastate->setGlobal("LUDB_TYPE_FLT",   column_type_float);
    luastate->setGlobal("LUDB_TYPE_LNG",   column_type_long);
    luastate->setGlobal("LUDB_TYPE_UIN",   column_type_uint);
    luastate->setGlobal("LUDB_TYPE_BLOB",  column_type_blob);
    luastate->setGlobal("LUDB_TYPE_ODT",   column_type_date);
}