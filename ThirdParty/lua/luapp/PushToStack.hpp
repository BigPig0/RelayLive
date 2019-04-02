
#pragma once

#include "luapp/LuaAPI.hpp"
#include "luapp/Table.hpp"

namespace lua{


//------------------------------------------------------------------------------
inline void PushVarToLua(lua::NativeState hLua,lua::Bool t)
{
	lua::PushBoolean(hLua,t);
}
//------------------------------------------------------------------------------
/*
inline void PushVarToLua(lua::NativeState hLua,lua::Int t)
{
	lua::PushInteger(hLua,t);
}*/
inline void PushVarToLua(lua::NativeState hLua,int t)
{
	lua::PushInteger(hLua,t);
}
inline void PushVarToLua(lua::NativeState hLua,long int t)
{
	lua::PushInteger(hLua,t);
}
inline void PushVarToLua(lua::NativeState hLua,long long int t)
{
	lua::PushInteger(hLua,t);
}
//------------------------------------------------------------------------------
inline void PushVarToLua(lua::NativeState hLua,lua::Num t)
{
	lua::PushNumber(hLua,t);
}
//------------------------------------------------------------------------------
inline void PushVarToLua(lua::NativeState hLua,lua::Str t)
{
	lua::PushString(hLua,t);
}
//------------------------------------------------------------------------------
inline void PushVarToLua(lua::NativeState hLua,lua::Ptr t)
{
	lua::PushPointer(hLua,t);
}
//------------------------------------------------------------------------------
inline void PushVarToLua(lua::NativeState hLua,lua::Nil)
{
	lua::PushNil(hLua);
}
//------------------------------------------------------------------------------
#ifdef _LUAPP_CPP11_
inline void PushVarToLua(lua::NativeState hLua,std::nullptr_t)
{
	lua::PushPointer(hLua,nullptr);
}
#endif
//------------------------------------------------------------------------------
inline void _PushValueToLuaTable(lua::NativeState hLua,lua::Table &table)
{
	lua::NewTable(hLua);                 // ... [T]

	lua::Table::Iterator   it = table.getBegin();

	lua::Var   key;
	lua::Var   value;

	for ( ; ! it.isEnd() ; it++ )
	{
		// ... [T]

		it.getKeyValue( &key, &value );

		if ( lua::VarType<lua::Str>(key) )
		{
			lua::Str   t_key = lua::VarCast<lua::Str>(key);
			PushVarToLua(hLua,t_key);                        // ... [T] [key]
		}
		else if ( lua::VarType<lua::Int>(key) )
		{
			lua::Int   t_key = lua::VarCast<lua::Int>(key);
			PushVarToLua(hLua,t_key);                        // ... [T] [key]
		}
		else if ( lua::VarType<lua::Num>(key) )
		{
			lua::Num   t_key = lua::VarCast<lua::Num>(key);
			PushVarToLua(hLua,t_key);                        // ... [T] [key]
		}
		#ifdef _LUAPP_ENABLE_BOOLEAN_INDEX_OF_TABLE_
		else if ( lua::VarType<lua::Bool>(key) )
		{
			lua::Bool  t_key = lua::VarCast<lua::Bool>(key);
			PushVarToLua(hLua,t_key);                        // ... [T] [key]
		}
		#endif
		else
		{
			continue;    // Just in case.
		}

		if ( lua::VarType<lua::Str>(value) )
		{
			lua::Str   t_value = lua::VarCast<lua::Str>(value);
			PushVarToLua(hLua,t_value);                               // ... [T] [key] [value]
		}
		else if ( lua::VarType<lua::Int>(value) )
		{
			lua::Int   t_value = lua::VarCast<lua::Int>(value);
			PushVarToLua(hLua,t_value);                               // ... [T] [key] [value]
		}
		else if ( lua::VarType<lua::Num>(value) )
		{
			lua::Num   t_value = lua::VarCast<lua::Num>(value);
			PushVarToLua(hLua,t_value);                               // ... [T] [key] [value]
		}
		else if ( lua::VarType<lua::Ptr>(value) )
		{
			lua::Ptr   t_value = lua::VarCast<lua::Ptr>(value);
			PushVarToLua(hLua,t_value);                               // ... [T] [key] [value]
		}
		else if ( lua::VarType<lua::Bool>(value) )
		{
			lua::Bool   t_value = lua::VarCast<lua::Bool>(value);
			PushVarToLua(hLua,t_value);                               // ... [T] [key] [value]
		}
		else if ( lua::VarType<lua::Table>(value) )
		{
			lua::Table   t_value = lua::VarCast<lua::Table>(value);
			_PushValueToLuaTable(hLua,t_value);                       // ... [T] [key] [value]
		}
		else if ( lua::VarType<lua::Func>(value) ||
		          lua::VarType<lua::Task>(value) ||
		          lua::VarType<lua::User>(value) )
		{
			lua::Log<<"error:ignore unsupported value"<<lua::End;
			lua::Pop(hLua, 1);           // ... [T]
			continue;
		}
		else
		{
			lua::Pop(hLua, 1);           // ... [T]
			continue;
		}

		lua::SetTable(hLua,-3);      // ... [T]
	}

	// ... [T]
}
//------------------------------------------------------------------------------
inline void PushVarToLua(lua::NativeState hLua,lua::Table &table)
{
	                                     // ...
	_PushValueToLuaTable(hLua,table);    // ... [T]
}
//------------------------------------------------------------------------------
inline void PushVarToLua(lua::NativeState hLua,lua::Var &t)
{
	if ( lua::VarType<lua::Str>(t) )
	{
		lua::Str   var = lua::VarCast<lua::Str>(t);
		PushVarToLua(hLua,var);
	}
	else if ( lua::VarType<lua::Num>(t) )
	{
		lua::Num   var = lua::VarCast<lua::Num>(t);
		PushVarToLua(hLua,var);
	}
	else if ( lua::VarType<lua::Int>(t) )
	{
		lua::Int   var = lua::VarCast<lua::Int>(t);
		PushVarToLua(hLua,var);
	}
	else if ( lua::VarType<lua::Bool>(t) )
	{
		lua::Bool  var = lua::VarCast<lua::Bool>(t);
		PushVarToLua(hLua,var);
	}
	else if ( lua::VarType<lua::Nil>(t) )
	{
		lua_pushnil(hLua);
	}
	else if ( lua::VarType<lua::Ptr>(t) )
	{
		lua::Ptr  var = lua::VarCast<lua::Ptr>(t);
		PushVarToLua(hLua,var);
	}
	else if ( lua::VarType<lua::Table>(t) )
	{
		lua::Table  var = lua::VarCast<lua::Table>(t);
		PushVarToLua(hLua,var);
	}
	else if ( lua::VarType<lua::Func>(t) ||
	          lua::VarType<lua::Task>(t) ||
	          lua::VarType<lua::User>(t) )
	{
		lua::Log<<"error:ignore unsupported data type"<<lua::End;
		lua_pushnil(hLua);
	}
	else
	{
		lua::Log<<"error:you push unknown data type"<<lua::End;
		lua_pushnil(hLua);
	}
}
//------------------------------------------------------------------------------

}
