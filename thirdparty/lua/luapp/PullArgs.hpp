
#pragma once

#include "luapp/Standard.hpp"

namespace lua{

template<typename L,typename A1>
inline void PullArgs(L h,A1 &a1)
{
	lua::CheckVarFromLua(h,&a1,-1);
	lua::Pop(h,1);
}

template<typename L,typename A1,typename A2>
inline void PullArgs(L h,A1 &a1,A2 &a2)
{
	lua::CheckVarFromLua(h,&a1,-2);
	lua::CheckVarFromLua(h,&a2,-1);
	lua::Pop(h,2);
}

template<typename L,typename A1,typename A2,typename A3>
inline void PullArgs(L h,A1 &a1,A2 &a2,A3 &a3)
{
	lua::CheckVarFromLua(h,&a1,-3);
	lua::CheckVarFromLua(h,&a2,-2);
	lua::CheckVarFromLua(h,&a3,-1);
	lua::Pop(h,3);
}

template<typename L,typename A1,typename A2,typename A3,typename A4>
inline void PullArgs(L h,A1 &a1,A2 &a2,A3 &a3,A4 &a4)
{
	lua::CheckVarFromLua(h,&a1,-4);
	lua::CheckVarFromLua(h,&a2,-3);
	lua::CheckVarFromLua(h,&a3,-2);
	lua::CheckVarFromLua(h,&a4,-1);
	lua::Pop(h,4);
}

template<typename L,typename A1,typename A2,typename A3,typename A4,typename A5>
inline void PullArgs(L h,A1 &a1,A2 &a2,A3 &a3,A4 &a4,A5 &a5)
{
	lua::CheckVarFromLua(h,&a1,-5);
	lua::CheckVarFromLua(h,&a2,-4);
	lua::CheckVarFromLua(h,&a3,-3);
	lua::CheckVarFromLua(h,&a4,-2);
	lua::CheckVarFromLua(h,&a5,-1);
	lua::Pop(h,5);
}

template<typename L,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6>
inline void PullArgs(L h,A1 &a1,A2 &a2,A3 &a3,A4 &a4,A5 &a5,A6 &a6)
{
	lua::CheckVarFromLua(h,&a1,-6);
	lua::CheckVarFromLua(h,&a2,-5);
	lua::CheckVarFromLua(h,&a3,-4);
	lua::CheckVarFromLua(h,&a4,-3);
	lua::CheckVarFromLua(h,&a5,-2);
	lua::CheckVarFromLua(h,&a6,-1);
	lua::Pop(h,6);
}

template<typename L,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7>
inline void PullArgs(L h,A1 &a1,A2 &a2,A3 &a3,A4 &a4,A5 &a5,A6 &a6,A7 &a7)
{
	lua::CheckVarFromLua(h,&a1,-7);
	lua::CheckVarFromLua(h,&a2,-6);
	lua::CheckVarFromLua(h,&a3,-5);
	lua::CheckVarFromLua(h,&a4,-4);
	lua::CheckVarFromLua(h,&a5,-3);
	lua::CheckVarFromLua(h,&a6,-2);
	lua::CheckVarFromLua(h,&a7,-1);
	lua::Pop(h,7);
}

template<typename L,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8>
inline void PullArgs(L h,A1 &a1,A2 &a2,A3 &a3,A4 &a4,A5 &a5,A6 &a6,A7 &a7,A8 &a8)
{
	lua::CheckVarFromLua(h,&a1,-8);
	lua::CheckVarFromLua(h,&a2,-7);
	lua::CheckVarFromLua(h,&a3,-6);
	lua::CheckVarFromLua(h,&a4,-5);
	lua::CheckVarFromLua(h,&a5,-4);
	lua::CheckVarFromLua(h,&a6,-3);
	lua::CheckVarFromLua(h,&a7,-2);
	lua::CheckVarFromLua(h,&a8,-1);
	lua::Pop(h,8);
}

template<typename L,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8,typename A9>
inline void PullArgs(L h,A1 &a1,A2 &a2,A3 &a3,A4 &a4,A5 &a5,A6 &a6,A7 &a7,A8 &a8,A9 &a9)
{
	lua::CheckVarFromLua(h,&a1,-9);
	lua::CheckVarFromLua(h,&a2,-8);
	lua::CheckVarFromLua(h,&a3,-7);
	lua::CheckVarFromLua(h,&a4,-6);
	lua::CheckVarFromLua(h,&a5,-5);
	lua::CheckVarFromLua(h,&a6,-4);
	lua::CheckVarFromLua(h,&a7,-3);
	lua::CheckVarFromLua(h,&a8,-2);
	lua::CheckVarFromLua(h,&a9,-1);
	lua::Pop(h,9);
}

}
