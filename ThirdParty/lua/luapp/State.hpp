/**
 * @file   State.hpp
 * @brief  The main interface of luapp.
 */


#pragma once

#include "luapp/Adapter.hpp"
#include "luapp/Wrapper.hpp"
#include "luapp/GlobalFunction.hpp"
#include "luapp/Searcher.hpp"
#include "luapp/PointerTypeFilter.hpp"

#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_
#include "luapp/Func.hpp"
#include "luapp/Register.hpp"
#endif

namespace lua{

/// The main interface of luapp.
template<int N=0>
class State
{
	public:

		State(lua::NativeState h = (lua::NativeState)0):_moduleMode(false)
		{
			#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_
				if ( h )
				{
					_moduleMode = true;
					_lua = std::make_shared<HandleClass>(h);
				}
				else
				{
					_lua = std::make_shared<HandleClass>();
				}

				wrapper::Wrapper<N>::_lua = this->_lua;
			#else
				if ( h )
				{
					_moduleMode = true;
					_lua = h;
				}
				else
				{
					_lua = lua::CreateHandle();

					if ( ! _lua )
					{
						lua::Log<<"error:can't get lua_State."<<lua::End;
					}

					lua::OpenLibs(_lua);
				}
			#endif
		}

		~State()
		{
			#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_
			wrapper::Wrapper<N>::_lua = NULL;
			#endif

			if(_lua)drop();
		}

		/// Let lua script could use given class type.
		template<typename C>
		void bindClass(lua::Str class_name)
		{
			#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_
			adapter::Adapter<C,N>::_lua = this->_lua;
			#ifdef _LUAPP_CLEAN_LUA_HANDLE_
			pushClean(&adapter::Adapter<C,N>::cleanPtr);
			#endif
			#endif

			if ( _moduleMode )
			{
				_funcReg.add(class_name,adapter::Adapter<C,N>::getConstructor(_lua));
			}
			else
			{
				adapter::Adapter<C,N>::registerClass(_lua,class_name);
			}
		}

		/**
		 * Let lua script could use given class type.
		 * It have a faster constructor. But lua need to store more information.
		 * Always call it after all bindMethod().
		 */
		template<typename C>
		void bindClassEx(lua::Str class_name)
		{
			#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_
			adapter::Adapter<C,N>::_lua = this->_lua;
			#ifdef _LUAPP_CLEAN_LUA_HANDLE_
			pushClean(&adapter::Adapter<C,N>::cleanPtr);
			#endif
			#endif

			if ( _moduleMode )
			{
				_funcReg.add(class_name,adapter::Adapter<C,N>::getConstructorEx(_lua));
			}
			else
			{
				adapter::Adapter<C,N>::registerClassEx(_lua,class_name);
			}
		}

		template<typename C>
		lua::CFunction bindClassEx()
		{
			#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_
			adapter::Adapter<C,N>::_lua = this->_lua;
			#ifdef _LUAPP_CLEAN_LUA_HANDLE_
			pushClean(&adapter::Adapter<C,N>::cleanPtr);
			#endif
			#endif

			return adapter::Adapter<C,N>::getConstructorEx(_lua);
		}

		template<typename C,typename A1>
		void bindClass1ArgEx(lua::Str class_name)
		{
			#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_
			adapter::Adapter<C,N>::_lua = this->_lua;
			#ifdef _LUAPP_CLEAN_LUA_HANDLE_
			pushClean(&adapter::Adapter<C,N>::cleanPtr);
			#endif
			#endif

			if ( _moduleMode )
			{
				_funcReg.add(class_name,adapter::Adapter<C,N>::getConstructor1ArgEx(_lua,(A1*)0));
			}
			else
			{
				adapter::Adapter<C,N>::registerClass1ArgEx(_lua,class_name,(A1*)0);
			}
		}

		template<typename C,typename A1>
		lua::CFunction bindClass1ArgEx()
		{
			#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_
			adapter::Adapter<C,N>::_lua = this->_lua;
			#ifdef _LUAPP_CLEAN_LUA_HANDLE_
			pushClean(&adapter::Adapter<C,N>::cleanPtr);
			#endif
			#endif

			return adapter::Adapter<C,N>::getConstructor1ArgEx(_lua,(A1*)0);
		}

		template<typename C,typename A1,typename A2>
		void bindClass2ArgEx(lua::Str class_name)
		{
			#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_
			adapter::Adapter<C,N>::_lua = this->_lua;
			#ifdef _LUAPP_CLEAN_LUA_HANDLE_
			pushClean(&adapter::Adapter<C,N>::cleanPtr);
			#endif
			#endif

			if ( _moduleMode )
			{
				_funcReg.add(class_name,adapter::Adapter<C,N>::getConstructor2ArgEx(_lua,(A1*)0,(A2*)0));
			}
			else
			{
				adapter::Adapter<C,N>::registerClass2ArgEx(_lua,class_name,(A1*)0,(A2*)0);
			}
		}

		template<typename C,typename A1,typename A2>
		lua::CFunction bindClass2ArgEx()
		{
			#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_
			adapter::Adapter<C,N>::_lua = this->_lua;
			#ifdef _LUAPP_CLEAN_LUA_HANDLE_
			pushClean(&adapter::Adapter<C,N>::cleanPtr);
			#endif
			#endif

			return adapter::Adapter<C,N>::getConstructor2ArgEx(_lua,(A1*)0,(A2*)0);
		}

		/**
		 * Let lua script could use given member function.
		 * Don't use it without bindClass/bindClassEx/bindClass(n)ArgEx.
		 */
		template<typename F>
		void bindMethod(lua::Str name,F fn)
		{
			typedef typename lua::PointerTypeFilter<F>::ClassType C;

			#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_
			adapter::Adapter<C,N>::_lua = this->_lua;
			#ifdef _LUAPP_CLEAN_LUA_HANDLE_
			pushClean(&adapter::Adapter<C,N>::cleanPtr);
			#endif
			#endif

			adapter::Adapter<C,N>::pushPack( name,adapter::GetProxy(fn));
		}

		template<typename C>
		void bindFunc(lua::Str name,lua::CFunction func)
		{
			struct adapter::Adapter<C,N>::NFunc     myF( name, func );
			adapter::Adapter<C,N>::pushNFunc(myF);
		}

		/// Convert the standard lua C function to lua global function.
		void setFunc(lua::Str name,lua::CFunction func)
		{
			#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_
			wrapper::Wrapper<N>::_lua = this->_lua;
			#endif

			if ( _moduleMode )
			{
				_funcReg.add(name,func);
			}
			else
			{
				lua::PushFunction(_lua, func);
				lua::SetGlobal(_lua, name);
			}
		}

		/// Convert C++ function to lua global function.
		template<typename F>
		void setFunc(lua::Str name,F fn)
		{
			if ( _moduleMode )
			{
				lua::Log<<"error:setFunc(lua::Str name,F fn) not support module mode."<<lua::End;
				return;
			}

			#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_
			wrapper::Wrapper<N>::_lua = this->_lua;
			#endif

			wrapper::Wrapper<N>::registerFunction(_lua,name,fn);
		}

		/// Convert C++ member function to lua global function.
		template<typename F,typename C>
		void setFunc(lua::Str name,F fn,C *obj)
		{
			#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_
			adapter::Adapter<C,N>::_lua = this->_lua;
			#ifdef _LUAPP_CLEAN_LUA_HANDLE_
			pushClean(&adapter::Adapter<C,N>::cleanPtr);
			#endif
			#endif

			if ( _moduleMode )
			{
				lua::Log<<"error:setFunc(lua::Str name,F fn,C *obj) not support module mode."<<lua::End;
				return;
			}

			// Add class type checked here some times later.
			wrapper::Wrapper<N>::registerFunction(_lua,name,fn,obj);
		}

		void drop()
		{
			#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_
				if ( _moduleMode )
				{
					this->build_module();
				}
				else
				{
					#ifdef _LUAPP_CLEAN_LUA_HANDLE_
					this->cleanHandle();
					#endif
				}
			#else
				if ( _lua )
				{
					if ( _moduleMode )
					{
						this->build_module();
					}
					else
					{
						lua::DestroyHandle(_lua);
					}
				}
			#endif

			_lua = NULL;
		}

		template<typename C>
		void cleanUnusedResource()
		{
			adapter::Adapter<C,N>::cleanUpUnusedResource();
		}

		int load(lua::Str name,lua::Str& code)
		{
			int result = lua::LoadScript(_lua,name,code);

			if ( ! result )
			{
				lua::Log<<"lua::State::load(name,code)"<<lua::End;
			}

			return result;
		}

		int load(lua::Str filename)
		{
			int result = lua::LoadScript(_lua,filename);

			if ( ! result )
			{
				lua::Log<<"lua::State::load(filename)"<<lua::End;
			}

			return result;
		}

		int runScriptFile(lua::Str script)
		{
			if ( _moduleMode )
			{
				lua::Log<<"error:You can't do this. Because module mode didn't run its own script."<<lua::End;
				return (int)0;
			}

			int result = lua::DoScript(_lua,script);

			if ( ! result )
			{
				lua::Log<<"lua::State::run()"<<lua::End;
			}

			return result;
		}

		int run()
		{
			if ( _moduleMode )
			{
				lua::Log<<"error:You can't do this. Because module mode didn't run its own script."<<lua::End;
				return (int)0;
			}

			if ( lua::PCall(_lua,0,0,0) )
			{
				return 0;
			}

			return 1;
		}

		/*
		lua.searcher(C) + lua.load(A,B) + lua.run()
		same as
		lua.run(A,B,C)
		*/
		int run(lua::Str name,lua::Str& code,std::function<lua::Str&(lua::Str)> loader)
		{
			if ( _moduleMode )
			{
				lua::Log<<"error:You can't do this. Because module mode didn't run its own script."<<lua::End;
				return (int)0;
			}

			if ( loader )
			{
				this->searcher(loader);
			}

			int result = lua::DoScript(_lua,name,code);

			if ( ! result )
			{
				lua::Log<<"lua::State::run(name,code,loader)"<<lua::End;
			}

			return result;
		}

		int run(lua::Str path,lua::Str script)
		{
			this->path(path);
			return this->runScriptFile(path+"/"+script);
		}

		void run(lua::Str cmd)
		{
			lua::DoString(_lua,cmd);
		}

		/// Call a standard lua C function without arguments and return values.
		void run( lua::CFunction func )
		{
			lua::PushFunction(_lua, func);
			lua::PCall(_lua,0,0,0);
		}

		/// Let luapp know where to read more lua scripts.
		void path(lua::Str path)
		{
			if(_lua)
			{
				path+="/";
				add_script_path_to_lua(path);
			}
		}

		/// Set global variable to lua script. Don't try to send function.
		template<typename T>
		void setGlobal(lua::Str name,T t)
		{
			lua::PushVarToLua(_lua,t);
			lua::SetGlobal(_lua,name);
		}

		/// Get global variable from lua script.
		template<typename T>
		void getGlobal(lua::Str name,T t)
		{
			lua::GetGlobal(_lua,name);
			lua::CheckVarFromLua(_lua,t,-1);
			lua::Pop(_lua,1);
		}

		#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_
		lua::Func getFunc(lua::Str name)
		{
			lua::Func  func;
			this->getGlobal(name,&func);
			return func;
		}
		#endif

		/// Get global function(one or no return value).
		template<typename F>
		void getFunc(const char *name,lua::GlobalFunction<F> *func)
		{
			func->_lua=_lua;
			func->_funcName=name;
		}

		/// Call global function that don't have return value.
		void call(lua::Str name)
		{
			lua::GetGlobal(_lua,name);
			lua::PCall(_lua,0,0,0);
		}

		template<typename A1>
		void call(lua::Str name,A1 a1)
		{
			lua::GetGlobal(_lua,name);
			lua::PushVarToLua(_lua,a1);
			lua::PCall(_lua,1,0,0);
		}

		template<typename A1,typename A2>
		void call(lua::Str name,A1 a1,A2 a2)
		{
			lua::GetGlobal(_lua,name);
			lua::PushVarToLua(_lua,a1);
			lua::PushVarToLua(_lua,a2);
			lua::PCall(_lua,2,0,0);
		}

		template<typename A1,typename A2,typename A3>
		void call(lua::Str name,A1 a1,A2 a2,A3 a3)
		{
			lua::GetGlobal(_lua,name);
			lua::PushVarToLua(_lua,a1);
			lua::PushVarToLua(_lua,a2);
			lua::PushVarToLua(_lua,a3);
			lua::PCall(_lua,3,0,0);
		}

		template<typename A1,typename A2,typename A3,typename A4>
		void call(lua::Str name,A1 a1,A2 a2,A3 a3,A4 a4)
		{
			lua::GetGlobal(_lua,name);
			lua::PushVarToLua(_lua,a1);
			lua::PushVarToLua(_lua,a2);
			lua::PushVarToLua(_lua,a3);
			lua::PushVarToLua(_lua,a4);
			lua::PCall(_lua,4,0,0);
		}

		template<typename A1,typename A2,typename A3,typename A4,typename A5>
		void call(lua::Str name,A1 a1,A2 a2,A3 a3,A4 a4,A5 a5)
		{
			lua::GetGlobal(_lua,name);
			lua::PushVarToLua(_lua,a1);
			lua::PushVarToLua(_lua,a2);
			lua::PushVarToLua(_lua,a3);
			lua::PushVarToLua(_lua,a4);
			lua::PushVarToLua(_lua,a5);
			lua::PCall(_lua,5,0,0);
		}

		template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6>
		void call(lua::Str name,A1 a1,A2 a2,A3 a3,A4 a4,A5 a5,A6 a6)
		{
			lua::GetGlobal(_lua,name);
			lua::PushVarToLua(_lua,a1);
			lua::PushVarToLua(_lua,a2);
			lua::PushVarToLua(_lua,a3);
			lua::PushVarToLua(_lua,a4);
			lua::PushVarToLua(_lua,a5);
			lua::PushVarToLua(_lua,a6);
			lua::PCall(_lua,6,0,0);
		}

		template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7>
		void call(lua::Str name,A1 a1,A2 a2,A3 a3,A4 a4,A5 a5,A6 a6,A7 a7)
		{
			lua::GetGlobal(_lua,name);
			lua::PushVarToLua(_lua,a1);
			lua::PushVarToLua(_lua,a2);
			lua::PushVarToLua(_lua,a3);
			lua::PushVarToLua(_lua,a4);
			lua::PushVarToLua(_lua,a5);
			lua::PushVarToLua(_lua,a6);
			lua::PushVarToLua(_lua,a7);
			lua::PCall(_lua,7,0,0);
		}

		template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8>
		void call(lua::Str name,A1 a1,A2 a2,A3 a3,A4 a4,A5 a5,A6 a6,A7 a7,A8 a8)
		{
			lua::GetGlobal(_lua,name);
			lua::PushVarToLua(_lua,a1);
			lua::PushVarToLua(_lua,a2);
			lua::PushVarToLua(_lua,a3);
			lua::PushVarToLua(_lua,a4);
			lua::PushVarToLua(_lua,a5);
			lua::PushVarToLua(_lua,a6);
			lua::PushVarToLua(_lua,a7);
			lua::PushVarToLua(_lua,a8);
			lua::PCall(_lua,8,0,0);
		}

		template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8,typename A9>
		void call(lua::Str name,A1 a1,A2 a2,A3 a3,A4 a4,A5 a5,A6 a6,A7 a7,A8 a8,A9 a9)
		{
			lua::GetGlobal(_lua,name);
			lua::PushVarToLua(_lua,a1);
			lua::PushVarToLua(_lua,a2);
			lua::PushVarToLua(_lua,a3);
			lua::PushVarToLua(_lua,a4);
			lua::PushVarToLua(_lua,a5);
			lua::PushVarToLua(_lua,a6);
			lua::PushVarToLua(_lua,a7);
			lua::PushVarToLua(_lua,a8);
			lua::PushVarToLua(_lua,a9);
			lua::PCall(_lua,9,0,0);
		}

		void searcher(std::function<lua::Str&(lua::Str)> loader)
		{
			Searcher<N>::setup(_lua,loader);
		}

		#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_
		/// Bind lua native function.
		lua::Func bind(lua::CFunction func)
		{
			lua::Func   fu;
			lua::PushFunction(_lua, func);

			lua::Register::Item   item = _lua->_register->newItem();

			item->setVar();
			fu._set(_lua,item);

			return fu;
		}

		/// Bind C++ function.
		template<typename F>
		lua::Func bind(F func)
		{
			lua::Func   fu;
			lua::Str    name("luapp_temp");
			this->setFunc(name,func);
			this->getGlobal(name,&fu);
			lua::RemoveGlobal(_lua,name);

			return fu;
		}

		/// Bind C++ member function.
		template<typename F,typename C>
		lua::Func bind(F func,C *obj)
		{
			lua::Func   fu;
			lua::Str    name("luapp_temp");
			this->setFunc(name,func,obj);
			this->getGlobal(name,&fu);
			lua::RemoveGlobal(_lua,name);

			return fu;
		}
		#endif

	private:

		void add_script_path_to_lua(lua::Str path)
		{
			                                 // ...
			lua::GetGlobal(_lua,"package");  // ... package
			lua::GetField(_lua,-1, "path");  // ... package old_path

			path += "?.lua;" + lua::CheckString(_lua,-1);
			lua::PushString(_lua,path);      // ... package old_path new_path

			lua::SetField(_lua,-3, "path");  // ... package old_path
			lua::Pop(_lua,2);                // ...
		}

		void build_module()
		{
			lua::NewModule(_lua,_funcReg);
		}

		#if defined(_LUAPP_KEEP_LOCAL_LUA_VARIABLE_) && defined(_LUAPP_CLEAN_LUA_HANDLE_)
		void pushClean(void(*func)())
		{
			int     count = _cleanHandle.size();
			int     not_exist = 1;

			for ( int i = 0 ; i < count ; i++)
			{
				if ( _cleanHandle[i]==func )
				{
					not_exist = 0;
					break;
				}
			}

			if ( not_exist )
			{
				_cleanHandle.push_back(func);
			}
		}

		void cleanHandle()
		{
			int     count = _cleanHandle.size();

			for ( int i = 0 ; i < count ; i++)
			{
				_cleanHandle[i]();
			}

			_cleanHandle.clear();
		}
		#endif

		lua::Handle      _lua;
		bool             _moduleMode;
		lua::FuncReg     _funcReg;      // Only work for module mode.

		#if defined(_LUAPP_KEEP_LOCAL_LUA_VARIABLE_) && defined(_LUAPP_CLEAN_LUA_HANDLE_)
		std::vector<void(*)()>    _cleanHandle;    // To write down all the shared pointer must free.
		#endif
};

#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_

inline HandleClass::HandleClass(lua::NativeState h)
{
	_lua = h;
	_register = std::make_shared<lua::Register>(_lua);
	_moduleMode = true;
}

inline HandleClass::HandleClass()
{
	_moduleMode = false;
	_lua = lua::CreateHandle();

	if ( ! _lua )
	{
		lua::Log<<"error:can't get lua_State."<<lua::End;
	}

	lua::OpenLibs(_lua);
	_register = std::make_shared<lua::Register>(_lua);
}

#endif

}//namespace lua
