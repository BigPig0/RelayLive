/**
 * @file   Adapter.hpp
 * @brief  Help lua and C++.
 */

#pragma once

#include "luapp/AdapterProxy.hpp"


namespace lua{
namespace adapter{

class StaticBool
{
	public:

		StaticBool():_flag(false)
		{
			;
		}

		~StaticBool()
		{
			;
		}

		void set(bool flag)
		{
			_flag = flag;
		}

		bool isTrue()
		{
			return _flag;
		}

		bool isFalse()
		{
			return ! _flag;
		}

	private:

		bool  _flag;
};

template<typename C,int N>
class Adapter
{
	public:

		struct NFunc
		{
			NFunc(Str name,lua::CFunction func):_name(name),_func(func){}

			Str            _name;
			lua::CFunction _func;
		};

		// It's a general way to register class.
		static void registerClass(lua::Handle L,const lua::Str& className)
		{
			Adapter<C,N>::_isGeneralConstructorEnable.set(true);

			//--------Setup a global function to lua--------
			lua::PushFunction(L, &Adapter<C,N>::constructor);  // ... [F]
			lua::SetGlobal(L, className);                      // ...

			buildMetaTableForUserdata(L);
		}

		// Call it after every member function was registed at _listName[] & _listProxy[].
		static void registerClassEx(lua::Handle L,const lua::Str& className)
		{
			//--------Setup a global function to lua--------
			lua::PushFunction(L, &Adapter<C,N>::constructorEx);// ... [F]
			lua::SetGlobal(L, className);                      // ...

			buildMetaTableForUserdata(L);
			buildMetaTableForMemberFunction(L);
		}

		template<typename A1>
		static void registerClass1ArgEx(lua::Handle L,lua::Str& className,A1*)
		{
			//--------Setup a global function to lua--------
			lua::PushFunction(L, &Adapter<C,N>::constructor1ArgEx<A1>); // ... [F]
			lua::SetGlobal(L, className);                               // ...

			buildMetaTableForUserdata(L);
			buildMetaTableForMemberFunction(L);
		}

		template<typename A1,typename A2>
		static void registerClass2ArgEx(lua::Handle L,lua::Str& className,A1*,A2*)
		{
			//--------Setup a global function to lua--------
			lua::PushFunction(L, &Adapter<C,N>::constructor2ArgEx<A1,A2>); // ... [F]
			lua::SetGlobal(L, className);                                  // ...

			buildMetaTableForUserdata(L);
			buildMetaTableForMemberFunction(L);
		}

		static lua::CFunction getConstructor(lua::Handle L)
		{
			Adapter<C,N>::_isGeneralConstructorEnable.set(true);
			buildMetaTableForUserdata(L);
			return &Adapter<C,N>::constructor;
		}

		static lua::CFunction getConstructorEx(lua::Handle L)
		{
			buildMetaTableForUserdata(L);
			buildMetaTableForMemberFunction(L);

			return &Adapter<C,N>::constructorEx;
		}

		template<typename A1>
		static lua::CFunction getConstructor1ArgEx(lua::Handle L,A1*)
		{
			buildMetaTableForUserdata(L);
			buildMetaTableForMemberFunction(L);

			return &Adapter<C,N>::constructor1ArgEx<A1>;
		}

		template<typename A1,typename A2>
		static lua::CFunction getConstructor2ArgEx(lua::Handle L,A1*,A2*)
		{
			buildMetaTableForUserdata(L);
			buildMetaTableForMemberFunction(L);

			return &Adapter<C,N>::constructor2ArgEx<A1,A2>;
		}

		static void pushPack( ::lua::Str str, ::lua::adapter::Proxy<C>* proxy)
		{
			Adapter<C,N>::_listName.push_back(str);
			Adapter<C,N>::_listProxy.push_back(proxy);
		}

		static void pushNFunc(struct NFunc func)
		{
			Adapter<C,N>::_nlist.push_back(func);
		}

		static void cleanUpUnusedResource()
		{
			if ( Adapter<C,N>::_isGeneralConstructorEnable.isFalse() )
			{
				NFuncList().swap( Adapter<C,N>::_nlist );
				StringList().swap( Adapter<C,N>::_listName );
			}
		}

		#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_
		#ifdef _LUAPP_CLEAN_LUA_HANDLE_
		static void cleanPtr()
		{
			Adapter<C,N>::_lua = NULL;
		}
		#endif
		static lua::Handle _lua;
		#endif

	private:

		class PackList : public std::vector< ::lua::adapter::Proxy<C>*>
		{
			public:

				~PackList()
				{
					for(int i =this->size()-1;i>=0;i--)
					{
						delete ((*this)[i]);
					}
				}
		};

		typedef std::vector< ::lua::Str>  StringList;
		typedef std::vector<struct NFunc> NFuncList;

		static Str         _classNameUD;   // For user data.
		static Str         _classNameMT;   // For meta table.
		static StringList  _listName;
		static PackList    _listProxy;
		static NFuncList   _nlist;

		static StaticBool  _isGeneralConstructorEnable;

		static void buildMetaTableForUserdata(lua::Handle L)
		{
			if ( ! _classNameUD.empty() ) return;

			_classNameUD = lua::CreateBindingCoreName<C>();

			lua::NewMetaTable(L, _classNameUD);                // ... [T]
			lua::PushString(L, "__gc");                        // ... [T] ["__gc"]
			lua::PushFunction(L, &Adapter<C,N>::gc_obj);       // ... [T] ["__gc"] [F]
			lua::SetTable(L, -3);                              // ... [T]
			lua::Pop(L,1);                                     // ...
		}

		static void buildMetaTableForMemberFunction(lua::Handle L)
		{
			if ( Adapter<C,N>::_listProxy.empty() && Adapter<C,N>::_nlist.empty() ) return;

			if ( ! _classNameMT.empty() ) return;

			_classNameMT = lua::CreateBindingMethodName<C>();

			lua::NewMetaTable(L, _classNameMT);                // ... [T]
			lua::PushString(L, "__index");                     // ... [T] ["__index"]
			lua::PushValue(L,-2);                              // ... [T] ["__index"] [T]
			lua::SetTable(L,-3);                               // ... [T]

			if ( Adapter<C,N>::_listName.size() != Adapter<C,N>::_listProxy.size() )
			{
				lua::Log<<"error:Something is wrong when building meta table."<<lua::End;
			}
			else
			{
				if ( ! Adapter<C,N>::_listName.empty() )
				{
					for (int i = Adapter<C,N>::_listName.size()-1; i>=0; i--)
					{
						lua::PushString(L, Adapter<C,N>::_listName[i]);    // ... [T] [member func name]
						lua::PushInteger(L, i);                            // ... [T] [member func name] [member func ID]
						lua::PushClosure(L, &Adapter<C,N>::thunk, 1);      // ... [T] [member func name] [closure]
						lua::SetTable(L, -3);                              // ... [T]
					}
				}
			}

			if ( ! Adapter<C,N>::_nlist.empty() )
			{
				for (int i = Adapter<C,N>::_nlist.size()-1; i>=0; i--)
				{
					lua::PushString(L, Adapter<C,N>::_nlist[i]._name);
					lua::PushFunction(L, Adapter<C,N>::_nlist[i]._func);
					lua::SetTable(L, -3);
				}
			}

			lua::Pop(L,1);                                     // ...
		}

		// As destructor.
		static int gc_obj(lua::NativeState L)
		{
			C** obj = static_cast<C**>(lua::CheckUserData(L, -1, _classNameUD));
			delete (*obj);

			return 0;
		}

		// It's a general way to build object(table).
		// Lua just only need to store constructor and a small metatable of userdata.
		static int constructor(lua::NativeState L)
		{
			lua::NewTable(L);                                  // ... [T]
			lua::_PushCoreKey(L);                              // ... [T] [key]
			C** a = (C**)lua::NewUserData(L, sizeof(C*));      // ... [T] [key] [UD]
			*a = new C;
			lua::GetMetaTable(L, _classNameUD);                // ... [T] [key] [UD] [MT]
			lua::SetMetaTable(L, -2);                          // ... [T] [key] [UD]
			lua::SetTable(L, -3);                              // ... [T]

			if ( Adapter<C,N>::_listName.size() != Adapter<C,N>::_listProxy.size() )
			{
				lua::Log<<"error:Something is wrong when building meta table in Adapter::constructor."<<lua::End;
			}
			else
			{
				if ( ! Adapter<C,N>::_listName.empty() )
				{
					for (int i = Adapter<C,N>::_listName.size()-1; i>=0; i--)
					{
						lua::PushString(L, Adapter<C,N>::_listName[i]);    // ... [T] [member func name]
						lua::PushInteger(L, i);                            // ... [T] [member func name] [member func ID]
						lua::PushClosure(L, &Adapter<C,N>::thunk, 1);      // ... [T] [member func name] [closure]
						lua::SetTable(L, -3);                              // ... [T]
					}
				}
			}

			if ( ! Adapter<C,N>::_nlist.empty() )
			{
				for (int i = Adapter<C,N>::_nlist.size()-1; i>=0; i--)
				{
					lua::PushString(L, Adapter<C,N>::_nlist[i]._name);
					lua::PushFunction(L, Adapter<C,N>::_nlist[i]._func);
					lua::SetTable(L, -3);
				}
			}

			return 1;
		}

		// It's a faster way to build object(table).
		// Lua have to store a big metatable that include every each member function.
		static int constructorEx(lua::NativeState L)
		{
			lua::NewTable(L);                                  // ... [T]

			//-----------Setup member function-----------
			lua::GetMetaTable(L, _classNameMT);                // ... [T] [MT]
			lua::SetMetaTable(L, -2);                          // ... [T]

			//-----------New a object and setup destructor-----------
			lua::_PushCoreKey(L);                              // ... [T] [key]
			C** a = (C**)lua::NewUserData(L, sizeof(C*));      // ... [T] [key] [UD]
			*a = new C;
			lua::GetMetaTable(L, _classNameUD);                // ... [T] [key] [UD] [MT]
			lua::SetMetaTable(L, -2);                          // ... [T] [key] [UD]
			lua::SetTable(L, -3);                              // ... [T]

			return 1;
		}

		template<typename A1>
		static int constructor1ArgEx(lua::NativeState L)
		{
			                                                   // [Arg] [LUA_TNONE]   I don't know why.
			A1   arg1;
			lua::CheckVarFromLua(L,&arg1,1);
			lua::Pop(L,2);                                     // ...

			lua::NewTable(L);                                  // ... [T]

			//-----------Setup member function-----------
			lua::GetMetaTable(L, _classNameMT);                // ... [T] [MT]
			lua::SetMetaTable(L, -2);                          // ... [T]

			//-----------New a object and setup destructor-----------
			lua::_PushCoreKey(L);                              // ... [T] [key]
			C** a = (C**)lua::NewUserData(L, sizeof(C*));      // ... [T] [key] [UD]

			*a = new C(arg1);
			lua::GetMetaTable(L, _classNameUD);                // ... [T] [key] [UD] [MT]
			lua::SetMetaTable(L, -2);                          // ... [T] [key] [UD]
			lua::SetTable(L, -3);                              // ... [T]

			return 1;
		}

		template<typename A1,typename A2>
		static int constructor2ArgEx(lua::NativeState L)
		{
			                                                   // [Arg1] [Arg2] [LUA_TNONE]   I don't know why.
			A1   arg1;
			A2   arg2;
			lua::CheckVarFromLua(L,&arg1,1);
			lua::CheckVarFromLua(L,&arg2,2);
			lua::Pop(L,3);                                     // ...

			lua::NewTable(L);                                  // ... [T]

			//-----------Setup member function-----------
			lua::GetMetaTable(L, _classNameMT);                // ... [T] [MT]
			lua::SetMetaTable(L, -2);                          // ... [T]

			//-----------New a object and setup destructor-----------
			lua::_PushCoreKey(L);                              // ... [T] [key]
			C** a = (C**)lua::NewUserData(L, sizeof(C*));      // ... [T] [key] [UD]

			*a = new C(arg1,arg2);
			lua::GetMetaTable(L, _classNameUD);                // ... [T] [key] [UD] [MT]
			lua::SetMetaTable(L, -2);                          // ... [T] [key] [UD]
			lua::SetTable(L, -3);                              // ... [T]

			return 1;
		}

		static int thunk(lua::NativeState L)
		{
			                                                        // [this] [arg1] [arg2] ... [argN]
			int id = lua::CheckInteger(L, lua::UpValueIndex(1));
			lua::_PushCoreKey(L);                                   // [this] [arg1] [arg2] ... [argN] [key]
			lua::GetTable(L, 1);                                    // [this] [arg1] [arg2] ... [argN] [UD]
			C** obj = static_cast<C**>(lua::CheckUserData(L, -1, _classNameUD));
			lua::Pop(L, 1);                                         // [this] [arg1] [arg2] ... [argN]

			#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_
			return	(Adapter<C,N>::_listProxy[id])->Do(Adapter<C,N>::_lua,*obj);
			#else
			return	(Adapter<C,N>::_listProxy[id])->Do(L,*obj);
			#endif
		}
};

template <typename C,int N>StaticBool                           Adapter<C,N>::_isGeneralConstructorEnable;
template <typename C,int N>Str                                  Adapter<C,N>::_classNameUD;
template <typename C,int N>Str                                  Adapter<C,N>::_classNameMT;
template <typename C,int N>typename Adapter<C,N>::StringList    Adapter<C,N>::_listName;
template <typename C,int N>typename Adapter<C,N>::PackList      Adapter<C,N>::_listProxy;
template <typename C,int N>typename Adapter<C,N>::NFuncList     Adapter<C,N>::_nlist;

#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_
template <typename C,int N>Handle                               Adapter<C,N>::_lua;
#endif


}//namespace adapter
}//namespace lua
