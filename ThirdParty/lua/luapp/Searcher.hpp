/**
 * @file   Searcher.hpp
 * @brief  To find script by custom way.
 */

#pragma once

#include "luapp/LuaAPI.hpp"
#include "luapp/stl/functional.hpp"

namespace lua{

template<int N>
class Searcher
{
	public:

		static void setup(lua::Handle L,std::function<lua::Str&(lua::Str)> searcher)
		{
			Searcher<N>::_findScriptFromBuffer = searcher;
			lua::PushFunction(L,&Searcher<N>::thunk);
			lua::SetGlobal(L,"luapp_searcher");
			lua::DoString(L,"table.insert(package.searchers, 2, _G.luapp_searcher);_G.luapp_searcher=nil\n");
		}

	private:

		static int thunk(lua::NativeState L)
		{
			lua::Str   name = lua::CheckString(L,1);

			if ( ! Searcher<N>::_findScriptFromBuffer )
			{
				lua::Log<<"error:no one call Searcher::setup()"<<lua::End;
				return 1;
			}

			lua::Str&  code = Searcher<N>::_findScriptFromBuffer(name);

			if ( code.empty() )
			{
				lua::Log<<"error:script not find"<<lua::End;
				return 1;
			}

			if ( ! lua::LoadScript(L,name,code) )
			{
				lua::Log<<"error:Searcher load script failed"<<lua::End;
			}

			return 1;
		}

		static std::function<lua::Str&(lua::Str)>   _findScriptFromBuffer;
};

template <int N> std::function<lua::Str&(lua::Str)> Searcher<N>::_findScriptFromBuffer;

}//namespace lua
