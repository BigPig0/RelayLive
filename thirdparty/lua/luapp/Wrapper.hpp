/**
@file   Wrapper.hpp
@brief  Help lua and C++.
*/


#pragma once


#include "luapp/WrapperProxy.hpp"



namespace lua{
namespace wrapper{


template<int N>
class Wrapper
{
	public:

		class PackList : public std::vector< ::lua::wrapper::Proxy*>
		{
			public:

				~PackList()
				{
					for(int i =this->size()-1;i>=0;i--)
					{
						delete (*this)[i];
					}
				}
		};

		// For global function.
		template <typename F>
		static void registerFunction( lua::Handle    L,
		                              lua::Str       name,
		                              F              fn
		                              )
		{
			_funcList.push_back(GetProxy(fn));

			lua::PushInteger(L, _funcList.size()-1);
			lua::PushClosure(L, &thunk, 1);
			lua::SetGlobal(L, name);
		}

		// For member function.
		template <typename F,typename C>
		static void registerFunction( lua::Handle    L,
		                              lua::Str       name,
		                              F              fn,
		                              C*             obj
		                              )
		{
			_funcList.push_back(GetProxy(fn,obj));

			lua::PushInteger(L, _funcList.size()-1);
			lua::PushClosure(L, &thunk, 1);
			lua::SetGlobal(L, name);
		}

		#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_
		static lua::Handle     _lua;
		#endif

	private:

		static PackList        _funcList;

		static int thunk(lua::NativeState L)
		{
			int id = lua::CheckInteger(L, lua::UpValueIndex(1));

			#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_
			return _funcList[id]->Do(Wrapper<N>::_lua);
			#else
			return _funcList[id]->Do(L);
			#endif
		}
};

template<int N>typename Wrapper<N>::PackList        Wrapper<N>::_funcList;

#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_
template<int N>Handle                               Wrapper<N>::_lua;
#endif

}//namespace wrapper
}//namespace lua
