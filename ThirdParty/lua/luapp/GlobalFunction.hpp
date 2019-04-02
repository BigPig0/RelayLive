/**
 * @file   GlobalFunction.hpp
 * @brief  Let you call the function come from lua.
 */

#pragma once

#include "luapp/Standard.hpp"


namespace lua{



template<typename R>
struct GlobalFunction{};

template<>
struct GlobalFunction<void()>
{
	GlobalFunction(){}

	void operator()() const
	{
		lua::GetGlobal(_lua,_funcName.c_str());
		lua::PCall(_lua,0,0,0);
	}
	lua::Handle      _lua;
	lua::Str         _funcName;
};

template<typename R>
struct GlobalFunction<R()>
{
	GlobalFunction(){}

	R operator()() const
	{
		lua::GetGlobal(_lua,_funcName.c_str());
		lua::PCall(_lua,0,1,0);

		R   result;
		CheckVarFromLua(_lua,&result,-1);
		lua::Pop(_lua,1);

		return result;
	}
	lua::Handle      _lua;
	lua::Str         _funcName;
};

template<typename A1>
struct GlobalFunction<void(A1)>
{
	void operator()(A1 a1) const
	{
		lua::GetGlobal(_lua,_funcName.c_str());
		PushVarToLua(_lua,a1);
		lua::PCall(_lua,1,0,0);
	}
	lua::Handle      _lua;
	lua::Str         _funcName;
};

template<typename R,typename A1>
struct GlobalFunction<R(A1)>
{
	R operator()(A1 a1) const
	{
		lua::GetGlobal(_lua,_funcName.c_str());
		PushVarToLua(_lua,a1);
		lua::PCall(_lua,1,1,0);

		R   result;
		CheckVarFromLua(_lua,&result,-1);

		lua::Pop(_lua,1);

		return result;
	}
	lua::Handle      _lua;
	lua::Str         _funcName;
};

template<typename A1,typename A2>
struct GlobalFunction<void(A1,A2)>
{
	void operator()(A1 a1,A2 a2) const
	{
		lua::GetGlobal(_lua,_funcName.c_str());
		PushVarToLua(_lua,a1);
		PushVarToLua(_lua,a2);
		lua::PCall(_lua,2,0,0);
	}
	lua::Handle      _lua;
	lua::Str         _funcName;
};

template<typename R,typename A1,typename A2>
struct GlobalFunction<R(A1,A2)>
{
	R operator()(A1 a1,A2 a2) const
	{
		lua::GetGlobal(_lua,_funcName.c_str());
		PushVarToLua(_lua,a1);
		PushVarToLua(_lua,a2);
		lua::PCall(_lua,2,1,0);

		R   result;
		CheckVarFromLua(_lua,&result,-1);

		lua::Pop(_lua,1);

		return result;
	}
	lua::Handle      _lua;
	lua::Str         _funcName;
};

template<typename A1,typename A2,typename A3>
struct GlobalFunction<void(A1,A2,A3)>
{
	void operator()(A1 a1,A2 a2,A3 a3) const
	{
		lua::GetGlobal(_lua,_funcName.c_str());
		PushVarToLua(_lua,a1);
		PushVarToLua(_lua,a2);
		PushVarToLua(_lua,a3);
		lua::PCall(_lua,3,0,0);
	}
	lua::Handle      _lua;
	lua::Str         _funcName;
};

template<typename R,typename A1,typename A2,typename A3>
struct GlobalFunction<R(A1,A2,A3)>
{
	R operator()(A1 a1,A2 a2,A3 a3) const
	{
		lua::GetGlobal(_lua,_funcName.c_str());
		PushVarToLua(_lua,a1);
		PushVarToLua(_lua,a2);
		PushVarToLua(_lua,a3);
		lua::PCall(_lua,3,1,0);

		R   result;
		CheckVarFromLua(_lua,&result,-1);

		lua::Pop(_lua,1);

		return result;
	}
	lua::Handle      _lua;
	lua::Str         _funcName;
};

template<typename A1,typename A2,typename A3,typename A4>
struct GlobalFunction<void(A1,A2,A3,A4)>
{
	void operator()(A1 a1,A2 a2,A3 a3,A4 a4) const
	{
		lua::GetGlobal(_lua,_funcName.c_str());
		PushVarToLua(_lua,a1);
		PushVarToLua(_lua,a2);
		PushVarToLua(_lua,a3);
		PushVarToLua(_lua,a4);
		lua::PCall(_lua,4,0,0);
	}
	lua::Handle      _lua;
	lua::Str         _funcName;
};

template<typename R,typename A1,typename A2,typename A3,typename A4>
struct GlobalFunction<R(A1,A2,A3,A4)>
{
	R operator()(A1 a1,A2 a2,A3 a3,A4 a4) const
	{
		lua::GetGlobal(_lua,_funcName.c_str());
		PushVarToLua(_lua,a1);
		PushVarToLua(_lua,a2);
		PushVarToLua(_lua,a3);
		PushVarToLua(_lua,a4);
		lua::PCall(_lua,4,1,0);

		R   result;
		CheckVarFromLua(_lua,&result,-1);

		lua::Pop(_lua,1);

		return result;
	}
	lua::Handle      _lua;
	lua::Str         _funcName;
};

template<typename A1,typename A2,typename A3,typename A4,typename A5>
struct GlobalFunction<void(A1,A2,A3,A4,A5)>
{
	void operator()(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) const
	{
		lua::GetGlobal(_lua,_funcName.c_str());
		PushVarToLua(_lua,a1);
		PushVarToLua(_lua,a2);
		PushVarToLua(_lua,a3);
		PushVarToLua(_lua,a4);
		PushVarToLua(_lua,a5);
		lua::PCall(_lua,5,0,0);
	}
	lua::Handle      _lua;
	lua::Str         _funcName;
};

template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5>
struct GlobalFunction<R(A1,A2,A3,A4,A5)>
{
	R operator()(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5) const
	{
		lua::GetGlobal(_lua,_funcName.c_str());
		PushVarToLua(_lua,a1);
		PushVarToLua(_lua,a2);
		PushVarToLua(_lua,a3);
		PushVarToLua(_lua,a4);
		PushVarToLua(_lua,a5);
		lua::PCall(_lua,5,1,0);

		R   result;
		CheckVarFromLua(_lua,&result,-1);

		lua::Pop(_lua,1);

		return result;
	}
	lua::Handle      _lua;
	lua::Str         _funcName;
};

template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6>
struct GlobalFunction<void(A1,A2,A3,A4,A5,A6)>
{
	void operator()(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5,A6 a6) const
	{
		lua::GetGlobal(_lua,_funcName.c_str());
		PushVarToLua(_lua,a1);
		PushVarToLua(_lua,a2);
		PushVarToLua(_lua,a3);
		PushVarToLua(_lua,a4);
		PushVarToLua(_lua,a5);
		PushVarToLua(_lua,a6);
		lua::PCall(_lua,6,0,0);
	}
	lua::Handle      _lua;
	lua::Str         _funcName;
};

template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6>
struct GlobalFunction<R(A1,A2,A3,A4,A5,A6)>
{
	R operator()(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5,A6 a6) const
	{
		lua::GetGlobal(_lua,_funcName.c_str());
		PushVarToLua(_lua,a1);
		PushVarToLua(_lua,a2);
		PushVarToLua(_lua,a3);
		PushVarToLua(_lua,a4);
		PushVarToLua(_lua,a5);
		PushVarToLua(_lua,a6);
		lua::PCall(_lua,6,1,0);

		R   result;
		CheckVarFromLua(_lua,&result,-1);

		lua::Pop(_lua,1);

		return result;
	}
	lua::Handle      _lua;
	lua::Str         _funcName;
};

template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7>
struct GlobalFunction<R(A1,A2,A3,A4,A5,A6,A7)>
{
	R operator()(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5,A6 a6,A7 a7) const
	{
		lua::GetGlobal(_lua,_funcName.c_str());
		PushVarToLua(_lua,a1);
		PushVarToLua(_lua,a2);
		PushVarToLua(_lua,a3);
		PushVarToLua(_lua,a4);
		PushVarToLua(_lua,a5);
		PushVarToLua(_lua,a6);
		PushVarToLua(_lua,a7);
		lua::PCall(_lua,7,1,0);

		R   result;
		CheckVarFromLua(_lua,&result,-1);

		lua::Pop(_lua,1);

		return result;
	}
	lua::Handle      _lua;
	lua::Str         _funcName;
};

template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8>
struct GlobalFunction<R(A1,A2,A3,A4,A5,A6,A7,A8)>
{
	R operator()(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5,A6 a6,A7 a7,A8 a8) const
	{
		lua::GetGlobal(_lua,_funcName.c_str());
		PushVarToLua(_lua,a1);
		PushVarToLua(_lua,a2);
		PushVarToLua(_lua,a3);
		PushVarToLua(_lua,a4);
		PushVarToLua(_lua,a5);
		PushVarToLua(_lua,a6);
		PushVarToLua(_lua,a7);
		PushVarToLua(_lua,a8);
		lua::PCall(_lua,8,1,0);

		R   result;
		CheckVarFromLua(_lua,&result,-1);

		lua::Pop(_lua,1);

		return result;
	}
	lua::Handle      _lua;
	lua::Str         _funcName;
};

template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8,typename A9>
struct GlobalFunction<R(A1,A2,A3,A4,A5,A6,A7,A8,A9)>
{
	R operator()(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5,A6 a6,A7 a7,A8 a8,A9 a9) const
	{
		lua::GetGlobal(_lua,_funcName.c_str());
		PushVarToLua(_lua,a1);
		PushVarToLua(_lua,a2);
		PushVarToLua(_lua,a3);
		PushVarToLua(_lua,a4);
		PushVarToLua(_lua,a5);
		PushVarToLua(_lua,a6);
		PushVarToLua(_lua,a7);
		PushVarToLua(_lua,a8);
		PushVarToLua(_lua,a9);
		lua::PCall(_lua,9,1,0);

		R   result;
		CheckVarFromLua(_lua,&result,-1);

		lua::Pop(_lua,1);

		return result;
	}
	lua::Handle      _lua;
	lua::Str         _funcName;
};


}//namespace lua
