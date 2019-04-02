
#pragma once

#include "luapp/State.hpp"
#include "luapp/PullArgs.hpp"

namespace lua{

template<typename T>
void CopyVar(T &target,const lua::Var &data)
{
	if ( lua::VarType<T>(data) )
	{
		target = lua::VarCast<T>(data);
	}
	else
	{
		lua::Log<<"error:wrong type! lua::CopyVar() at Auxiliary.hpp"<<lua::End;
	}
}

template<typename T>
void TryCopyVar(T &target,const lua::Var &data)
{
	if ( lua::VarType<T>(data) )
	{
		target = lua::VarCast<T>(data);
	}
}

inline lua::Str GetPrintTableFuncScript()
{
	lua::Str   str("= function(name,t)\n");
	str+="local indent_dev =\n";str+="{\n";str+="str = function (self)\n";
	str+="return string.rep(self._text,self._size)\n";str+="end,\n";
	str+="move = function (self,offset)\n";
	str+="self._size = self._size + offset\n";str+="end,\n";str+="_size = 0,\n";
	str+="_text=\"    \"\n";str+="}\n";str+="local function modify_key(key)\n";
	str+="if type(key)==[[string]] then\n";str+="key=[[\"]] .. key .. [[\"]]\n";
	str+="elseif type(key)==[[boolean]] then\n";str+="if key then\n";
	str+="key = [[true]]\n";str+="else\n";str+="key = [[false]]\n";str+="end\n";
	str+="end\n";str+="return key\n";str+="end\n";
	str+="local function print_table(tt,indent)\n";
	str+="print(indent:str() .. [[{]])\n";str+="indent:move(1)\n";
	str+="for key, value in pairs(tt) do\n";str+="key = modify_key(key)\n";
	str+="if type(value)==[[table]] then\n";
	str+="print(indent:str() .. key .. [[ =]])\n";
	str+="print_table(value,indent)\n";
	str+="elseif type(value)==[[string]] then\n";
	str+="print(indent:str() .. key .. [[ = \"]] .. value .. [[\"]])\n";
	str+="elseif type(value)==[[boolean]] then\n";str+="if value then\n";
	str+="print(indent:str() .. key .. [[ = ]] .. [[true]])\n";str+="else\n";
	str+="print(indent:str() .. key .. [[ = ]] .. [[false]])\n";str+="end\n";
	str+="elseif type(value)==[[userdata]] then\n";
	str+="print(indent:str() .. key .. \" = [userdata]\")\n";
	str+="elseif type(value)==[[thread]] then\n";
	str+="print(indent:str() .. key .. \" = [thread]\")\n";
	str+="elseif type(value)==[[function]] then\n";
	str+="print(indent:str() .. key .. \" = [function]\")\n";
	str+="else\n";str+="print(indent:str() .. key .. [[ = ]] .. value)\n";
	str+="end\n";str+="end\n";str+="indent:move(-1)\n";
	str+="print(indent:str() .. [[}]])\n";str+="end\n";
	str+="print(name .. [[ =]])\n";str+="print_table(t,indent_dev)\n";
	str+="end\n";

	return str;
}

template<int N>
bool SetPrintTableFunc(lua::State<N> *state,lua::Str name)
{
	lua::Var  var;
	state->getGlobal(name,&var);

	if ( lua::VarType<lua::Nil>(var) )
	{
		name += ::lua::GetPrintTableFuncScript();
		state->run(name);
		return true;
	}

	return false;
}

}//namespace lua
