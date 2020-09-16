#pragma once

#define LUAPP_VERSION_MAJOR "2"
#define LUAPP_VERSION_MINOR "2"
#define LUAPP_VERSION_PATCH "0"
#define LUAPP_AUTHOR "Yan Xin Wu"


#include "luapp/Environment.hpp"

// if ( version >= C++11 )
#if (__cplusplus > 201100L) || defined(TOY_VC_2012) || defined(TOY_VC_2013) || defined(TOY_VC_2015) || defined(TOY_VC_2017)
	#define _LUAPP_CPP11_
#endif


// Check data type before copy variable from lua.
#define _LUAPP_CHECK_DATA_TYPE_

// Spend more time to make sure everything was fine.
#define _LUAPP_CHECK_CAREFUL_

// You can keep lua variable at C++ side, but it make luapp slow down.
#define _LUAPP_KEEP_LOCAL_LUA_VARIABLE_

// Release shared pointer when lua::State deconstruction. It make luapp slow down.
#define _LUAPP_CLEAN_LUA_HANDLE_

// Enable boolean index for lua::Table.
#define _LUAPP_ENABLE_BOOLEAN_INDEX_OF_TABLE_

#include "luapp/Log.hpp"
