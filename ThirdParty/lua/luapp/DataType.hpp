/**
 * @file   DataType.hpp
 * @brief  Only These date types could work in luapp and lua
 */

#pragma once

#include <string>
#include "lua.hpp"
#include "luapp/Config.hpp"

namespace lua{

typedef bool            Bool;
typedef lua_Number      Num;
typedef lua_Integer     Int;
/*
typedef double          Num;
typedef long long       Int;
typedef long int        Int;
typedef int             Int;
*/
typedef std::string     Str;
typedef void*           Ptr;


struct Nil {};

/*
 * It implemented at luapp/Table.hpp
 * I try to made it looks like lua table.
 */
class Table;

/*
 * It implemented at luapp/Var.hpp
 * It works like boost::any.
 */
class Var;


#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_
class Map;
class Tag;
class Func;
class Task;// lua thread
class User;// For userdata
#else
struct Func {};
struct Task {};// lua thread
struct User {};// For userdata
#endif

}//namespace lua
