
#pragma once

#include "luapp/LuaAPI.hpp"
#include "luapp/PushToStack.hpp"
#include "luapp/CopyFromStack.hpp"
#include "luapp/VarBridgeExtra.hpp"

namespace lua{

#ifdef _LUAPP_KEEP_LOCAL_LUA_VARIABLE_

class Register;

class HandleClass
{
	public:

		// They implement at State.hpp
		HandleClass(lua::NativeState h);
		HandleClass();

		~HandleClass()
		{
			if ( ! _moduleMode )
			{
				lua::DestroyHandle(_lua);
				_lua = (lua::NativeState)0;
			}
		}

		lua::NativeState      _lua;
		bool                  _moduleMode;

		std::shared_ptr<lua::Register>        _register;
};


typedef std::shared_ptr<HandleClass> Handle;


//------------------------------Just a wrapper of LuaAPI.hpp------------------------------start
inline void NewModule(Handle h,FuncReg &reg) { NewModule(h->_lua,reg); }
inline void DestroyHandle(Handle h) { DestroyHandle(h->_lua); }
inline int PCall(Handle h,int num01,int num02,int num03) { return PCall(h->_lua,num01,num02,num03); }
inline void OpenLibs(Handle h) { OpenLibs(h->_lua); }
inline int TypeCast(Handle h,int index) { return TypeCast(h->_lua,index); }
inline void DoString(Handle h,lua::Str code) { DoString(h->_lua,code); }
inline int LoadScript(Handle h,lua::Str name,lua::Str& code) { return LoadScript(h->_lua,name,code); }
inline int LoadScript(Handle h,lua::Str filename) { return LoadScript(h->_lua,filename); }
inline int DoScript(Handle h,lua::Str name,lua::Str& code) { return DoScript(h->_lua,name,code); }
inline int DoScript(Handle h,lua::Str filename) { return DoScript(h->_lua,filename); }
inline void NewTable(Handle h) { NewTable(h->_lua); }
inline int NewMetaTable(Handle h,lua::Str tname) { return NewMetaTable(h->_lua,tname); }
inline void* NewUserData(Handle h,size_t size) { return NewUserData(h->_lua,size); }
inline void RemoveGlobal(Handle h,lua::Str var) { RemoveGlobal(h->_lua,var); }
inline void SetGlobal(Handle h,lua::Str var) { SetGlobal(h->_lua,var); }
inline bool IsGlobal(Handle h,lua::Str var) { return IsGlobal(h->_lua,var); }
inline void GetGlobal(Handle h,lua::Str var) { GetGlobal(h->_lua,var); }
inline void SetTable(Handle h,int index) { SetTable(h->_lua,index); }
inline void GetTable(Handle h,int index) { GetTable(h->_lua,index); }
inline void SetField(Handle h,int index, lua::Str name) { SetField(h->_lua,index,name); }
inline void GetField(Handle h,int index, lua::Str k) { GetField(h->_lua,index,k); }
inline int SetMetaTable(Handle h,int index) { return SetMetaTable(h->_lua,index); }
inline void GetMetaTable(Handle h,lua::Str name) { GetMetaTable(h->_lua,name); }
inline void PushNil(Handle h) { PushNil(h->_lua); }
inline void PushClosure(Handle h,CFunction fn,int n) { PushClosure(h->_lua,fn,n); }
inline void PushFunction(Handle h,CFunction fn) { PushFunction(h->_lua,fn); }
inline void PushString(Handle h,lua::Str str) { PushString(h->_lua,str); }
inline void PushValue(Handle h,int index) { PushValue(h->_lua,index); }
inline void PushNumber(Handle h,double n) { PushNumber(h->_lua,n); }
inline void PushBoolean(Handle h,bool num) { PushBoolean(h->_lua,num); }
inline void PushInteger(Handle h,int num) { PushInteger(h->_lua,num); }
template<typename S>
inline void PushUserData(Handle h,S ud) { PushUserData(h->_lua,ud); }
template<typename S>
inline void PushUserData(Handle h,S ud, lua::Str tname) { PushUserData(h->_lua,ud,tname); }
inline double CheckNumber(Handle h,int index) { return CheckNumber(h->_lua,index); }
inline bool CheckBoolean(Handle h,int index) { return CheckBoolean(h->_lua,index); }
inline int CheckInteger(Handle h,int index) { return CheckInteger(h->_lua,index); }
inline Str CheckString(Handle h,int index) { return CheckString(h->_lua,index); }
inline void* CheckUserData(Handle h,int index) { return CheckUserData(h->_lua,index); }
inline void* CheckUserData(Handle h,int index, lua::Str tname) { return CheckUserData(h->_lua,index,tname); }
inline void PushPointer(Handle h,Ptr ptr) { PushPointer(h->_lua,ptr); }
#ifdef _LUAPP_CPP11_
inline void PushPointer(Handle h,std::nullptr_t ptr) { PushPointer(h->_lua,ptr); }
#endif
inline Ptr CheckPointer(Handle h,int index) { return CheckPointer(h->_lua,index); }
inline void Pop(Handle h,int num) { Pop(h->_lua,num); }
inline void Replace(Handle h,int num) { Replace(h->_lua,num); }
inline void SetTop(Handle h,int num) { SetTop(h->_lua,num); }
inline int GetTop(Handle h) { return GetTop(h->_lua); }
//------------------------------Just a wrapper of LuaAPI.hpp------------------------------end


inline void CheckVarFromLua(Handle h,lua::Bool *t, int i) { CheckVarFromLua(h->_lua,t,i); }
inline void CheckVarFromLua(Handle h,lua::Int *t,  int i) { CheckVarFromLua(h->_lua,t,i); }
inline void CheckVarFromLua(Handle h,lua::Num *t,  int i) { CheckVarFromLua(h->_lua,t,i); }
inline void CheckVarFromLua(Handle h,lua::Str *t,  int i) { CheckVarFromLua(h->_lua,t,i); }
inline void CheckVarFromLua(Handle h,lua::Ptr *t,  int i) { CheckVarFromLua(h->_lua,t,i); }

inline void PushVarToLua(lua::Handle h,lua::Bool   t) { PushVarToLua(h->_lua,t); }
inline void PushVarToLua(lua::Handle h,lua::Num    t) { PushVarToLua(h->_lua,t); }
inline void PushVarToLua(lua::Handle h,lua::Str    t) { PushVarToLua(h->_lua,t); }
inline void PushVarToLua(lua::Handle h,lua::Ptr    t) { PushVarToLua(h->_lua,t); }
inline void PushVarToLua(lua::Handle h,lua::Nil    t) { PushVarToLua(h->_lua,t); }
/*
inline void PushVarToLua(lua::Handle h,lua::Int    t) { PushVarToLua(h->_lua,t); }
*/
inline void PushVarToLua(lua::Handle h,int         t) { PushVarToLua(h->_lua,t); }
inline void PushVarToLua(lua::Handle h,long int    t) { PushVarToLua(h->_lua,t); }
inline void PushVarToLua(lua::Handle h,long long   t) { PushVarToLua(h->_lua,t); }

template<typename T>
inline void PushVarToLua(lua::Handle h,lua::Type<T> t) { PushVarToLua(h->_lua,t); }

template<typename T>
inline void CheckVarFromLua(Handle h,lua::Type<T> *t, int i) { CheckVarFromLua(h->_lua,t,i); }

template<typename T>
inline void PushVarToLua(lua::Handle h,lua::Object<T> t) { PushVarToLua(h->_lua,t); }

template<typename C>
inline void CheckVarFromLua(Handle h,lua::Object<C> *t, int i) { CheckVarFromLua(h->_lua,t,i); }

template<typename C>
inline void CheckVarFromLua(Handle h,lua::Obj<C> *t, int i) { CheckVarFromLua(h->_lua,t,i); }

#else

typedef lua_State* Handle;

#endif


}//namespace lua
