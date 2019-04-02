
#pragma once

#include "lua.hpp"

namespace lua{

typedef lua_State* NativeState;
typedef int (*CFunction) (NativeState);   // lua::CFunction as lua_CFunction.
typedef const char* Name;

}
