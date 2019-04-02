
#pragma once

#include <cstring>

#include "luapp/LuaAPI.hpp"
#include "luapp/TypeString.hpp"

namespace lua{


//------------------------------------------------------------------------------

template<typename T>
class _ClassZone
{
	public:

		_ClassZone(){}
		~_ClassZone(){}

		static int destructor(lua::NativeState L)
		{
			T* obj = static_cast<T*>(lua::CheckUserData(L, -1, lua::CreateUserType<T>()));

			obj->~T();

			return 0;
		}

		static void registerType(lua::NativeState hLua,lua::Str &userType)
		{
			lua::GetMetaTable(hLua,userType);                                // ... [?]

			if ( lua::TypeCast(hLua,-1)==LUA_TNIL )
			{
				lua::NewMetaTable(hLua, userType);                           // ... [nil] [T]

				lua::PushString(hLua, "__gc");                               // ... [nil] [T] ["__gc"]
				lua::PushFunction(hLua, &lua::_ClassZone<T>::destructor);    // ... [nil] [T] ["__gc"] [F]
				lua::SetTable(hLua, -3);                                     // ... [nil] [T]
				lua::Pop(hLua,2);                                            // ...
			}
			else
			{
				lua::Pop(hLua,1);                                            // ...
			}
		}
};

template<typename T>
inline void PushClassToLua(lua::NativeState hLua)
{
	lua::Str   userType = lua::CreateUserType<T>();

	lua::_ClassZone<T>::registerType(hLua,userType);

	T*  ptr = static_cast<T*>(lua::NewUserData(hLua, sizeof(T)));    // ... [UD]

	new (ptr) T();

	lua::GetMetaTable(hLua, userType);                               // ... [UD] [MT]
	lua::SetMetaTable(hLua, -2);                                     // ... [UD]
}

template<typename T,typename A1>
inline void PushClassToLua(lua::NativeState hLua,A1 a1)
{
	lua::Str   userType = lua::CreateUserType<T>();

	lua::_ClassZone<T>::registerType(hLua,userType);

	T*  ptr = static_cast<T*>(lua::NewUserData(hLua, sizeof(T)));    // ... [UD]

	new (ptr) T(a1);

	lua::GetMetaTable(hLua, userType);                               // ... [UD] [MT]
	lua::SetMetaTable(hLua, -2);                                     // ... [UD]
}

template<typename T,typename A1,typename A2>
inline void PushClassToLua(lua::NativeState hLua,A1 a1,A2 a2)
{
	lua::Str   userType = lua::CreateUserType<T>();

	lua::_ClassZone<T>::registerType(hLua,userType);

	T*  ptr = static_cast<T*>(lua::NewUserData(hLua, sizeof(T)));    // ... [UD]

	new (ptr) T(a1,a2);

	lua::GetMetaTable(hLua, userType);                               // ... [UD] [MT]
	lua::SetMetaTable(hLua, -2);                                     // ... [UD]
}

template<typename T,typename A1,typename A2,typename A3>
inline void PushClassToLua(lua::NativeState hLua,A1 a1,A2 a2,A3 a3)
{
	lua::Str   userType = lua::CreateUserType<T>();

	lua::_ClassZone<T>::registerType(hLua,userType);

	T*  ptr = static_cast<T*>(lua::NewUserData(hLua, sizeof(T)));    // ... [UD]

	new (ptr) T(a1,a2,a3);

	lua::GetMetaTable(hLua, userType);                               // ... [UD] [MT]
	lua::SetMetaTable(hLua, -2);                                     // ... [UD]
}

template<typename T,typename A1,typename A2,typename A3,typename A4>
inline void PushClassToLua(lua::NativeState hLua,A1 a1,A2 a2,A3 a3,A4 a4)
{
	lua::Str   userType = lua::CreateUserType<T>();

	lua::_ClassZone<T>::registerType(hLua,userType);

	T*  ptr = static_cast<T*>(lua::NewUserData(hLua, sizeof(T)));    // ... [UD]

	new (ptr) T(a1,a2,a3,a4);

	lua::GetMetaTable(hLua, userType);                               // ... [UD] [MT]
	lua::SetMetaTable(hLua, -2);                                     // ... [UD]
}

// Not every class could copy new one. It's why output pointer here.
template<typename T>
inline void CheckClassFromLua(lua::NativeState hLua,T **t,int i)
{
	T*  obj = static_cast<T*>(lua::CheckUserData(hLua, i, lua::CreateUserType<T>()));
	*t = obj;
}

//------------------------------------------------------------------------------

template<typename T>
inline void PushStructToLua(lua::NativeState hLua,const T &t)
{
	lua::Str   userType = lua::CreateUserType<T>();

	lua::GetMetaTable(hLua,userType);                              // ... [?]

	if ( lua::TypeCast(hLua,-1)==LUA_TNIL )
	{
		lua::NewMetaTable(hLua, userType);                         // ... [nil] [T]
		lua::Pop(hLua,2);                                          // ...
	}
	else
	{
		lua::Pop(hLua,1);                                          // ...
	}

	T*  ptr = static_cast<T*>(lua::NewUserData(hLua, sizeof(T)));  // ... [UD]
	*ptr = t;
	lua::GetMetaTable(hLua, userType);                             // ... [UD] [MT]
	lua::SetMetaTable(hLua, -2);                                   // ... [UD]
}

template<typename T>
inline void CheckStructFromLua(lua::NativeState hLua,T *t,int i)
{
	T*  obj = static_cast<T*>(lua::CheckUserData(hLua, i, lua::CreateUserType<T>()));
	*t = *obj;
}

//------------------------------------------------------------------------------

inline void PushUserDataToLua(lua::NativeState hLua,void *input,size_t size,lua::Str userType)
{
	lua::GetMetaTable(hLua,userType);            // ... [?]

	if ( lua::TypeCast(hLua,-1)==LUA_TNIL )
	{
		lua::NewMetaTable(hLua, userType);       // ... [nil] [T]
		lua::Pop(hLua,2);                        // ...
	}
	else
	{
		lua::Pop(hLua,1);                        // ...
	}

	void*  ptr = lua::NewUserData(hLua, size);   // ... [UD]

	std::memcpy(ptr,input,size);

	lua::GetMetaTable(hLua, userType);           // ... [UD] [MT]
	lua::SetMetaTable(hLua, -2);                 // ... [UD]
}

inline void CheckUserDataFromLua(lua::NativeState hLua,void *output,size_t size,int i,lua::Str userType)
{
	void*  obj = lua::CheckUserData(hLua, i, userType);

	std::memcpy(output,obj,size);
}

//------------------------------------------------------------------------------

// Type T must be able to copied by std::memcpy().
template<typename T>
class Type
{
	public:

		Type(){}
		~Type(){}

		T   data;
};

template<typename T>
inline void PushVarToLua(lua::NativeState hLua,lua::Type<T> t)
{
	lua::PushStructToLua(hLua,t.data);
}

template<typename T>
inline void CheckVarFromLua(lua::NativeState hLua,lua::Type<T> *t, int i)
{
	lua::CheckStructFromLua(hLua,&(t->data),i);
}

//------------------------------------------------------------------------------

// May be I should remove it.
template<typename T>
class Obj
{
	public:

		Obj(){}
		~Obj(){}

		T   *ptr;
};

template<typename C>
inline void CheckVarFromLua(lua::NativeState hLua,lua::Obj<C> *obj, int i)
{
	lua::CheckClassFromLua(hLua,&(obj->ptr),i);
}

//------------------------------------------------------------------------------

inline void _PushCoreKey(lua::NativeState L)
{
	lua::PushInteger(L, 0);
//	lua::PushInteger(L, 1000);
//	lua::PushNumber(L, 0.0001f);
//	lua::PushString(L, "__object_from_cpp");
}

template<typename T>
class Object
{
	public:

		Object(){}
		~Object(){}

		T   *ptr;
};

template<typename C>
inline void PushVarToLua(lua::NativeState L,lua::Object<C> t)
{
	lua::Str  name = lua::CreateBindingCoreName<C>();

	lua::NewTable(L);                                  // ... [T]

	//-----------New a object and setup destructor-----------
	lua::_PushCoreKey(L);                              // ... [T] [key]
	C** a = (C**)lua::NewUserData(L, sizeof(C*));      // ... [T] [key] [UD]
	*a = t.ptr;

	lua::_ClassZone<C>::registerType(L,name);

	lua::GetMetaTable(L, name);                        // ... [T] [key] [UD] [MT]
	lua::SetMetaTable(L, -2);                          // ... [T] [key] [UD]
	lua::SetTable(L, -3);                              // ... [T]
}

template<typename C>
inline void CheckVarFromLua(lua::NativeState hLua,lua::Object<C> *obj, int i)
{
	                           // ... [var] ...
	lua::_PushCoreKey(hLua);   // ... [var] ... [key]
	lua::GetTable(hLua, i);    // ... [var] ... [UD]

	lua::Str    name = lua::CreateBindingCoreName<C>();

	C** ptr = static_cast<C**>(lua::CheckUserData(hLua, -1, name));
	obj->ptr = *ptr;

	lua::Pop(hLua, 1);         // ... [var] ...
}

//------------------------------------------------------------------------------


}//namespace lua
