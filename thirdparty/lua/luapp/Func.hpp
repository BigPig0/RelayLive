/**
 * @file   Func.cpp
 * @brief  A functor could work on C++ and lua.
 */

#pragma once

#include "luapp/Handle.hpp"
#include "luapp/Register.hpp"


namespace lua{

class Func
{
	public:

		Func(){}

		~Func()
		{
			_item = NULL;   // Just make sure it released before this->_lua.
		}

		void call();

		template<typename R>
		R call();

		template<typename A1>
		void call(A1 a1);

		template<typename R,typename A1>
		R call(A1 a1);

		template<typename A1,typename A2>
		void call(A1 a1, A2 a2);

		template<typename R,typename A1,typename A2>
		R call(A1 a1, A2 a2);

		template<typename A1,typename A2,typename A3>
		void call(A1 a1, A2 a2, A3 a3);

		template<typename R,typename A1,typename A2,typename A3>
		R call(A1 a1, A2 a2, A3 a3);

		template<typename A1,typename A2,typename A3,typename A4>
		void call(A1 a1, A2 a2, A3 a3, A4 a4);

		template<typename R,typename A1,typename A2,typename A3,typename A4>
		R call(A1 a1, A2 a2, A3 a3, A4 a4);

		template<typename A1,typename A2,typename A3,typename A4,typename A5>
		void call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5);

		template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5>
		R call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5);

		template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6>
		void call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6);

		template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6>
		R call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6);

		template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7>
		void call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7);

		template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7>
		R call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7);

		template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8>
		void call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8);

		template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8>
		R call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8);

		template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8,typename A9>
		void call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9);

		template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8,typename A9>
		R call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9);

		void _set(lua::Handle h,lua::Register::Item i)
		{
			if ( _lua ) lua::Log<<"warning:why you set handle of function again?"<<lua::End;
			_item = i;
			_lua = h;
		}

		lua::Register::Item _getItem()
		{
			return this->_item;
		}

	private:

		lua::Handle          _lua;
		lua::Register::Item  _item;

};

inline Var::Var(const lua::Func &t):_ptr(0)
{
	this->_ptr = new ::lua::_VarType<lua::Func>(t);
}

inline Var& Var::operator = (const lua::Func &t)
{
	this->free_ptr();
	this->_ptr = new ::lua::_VarType<lua::Func>(t);
	return *this;
}

//------------------------------------------------------------------------------
inline void lua::Var::operator () ()
{
	if ( lua::VarType<lua::Func>(*this) )
	{
		lua::VarCast<lua::Func>(*this).call();
	}
}
template<typename A1>
void lua::Var::operator () (A1 a1)
{
	if ( lua::VarType<lua::Func>(*this) )
	{
		lua::VarCast<lua::Func>(*this).call(a1);
	}
}
template<typename A1,typename A2>
void lua::Var::operator () (A1 a1,A2 a2)
{
	if ( lua::VarType<lua::Func>(*this) )
	{
		lua::VarCast<lua::Func>(*this).call(a1,a2);
	}
}
template<typename A1,typename A2,typename A3>
void lua::Var::operator () (A1 a1,A2 a2,A3 a3)
{
	if ( lua::VarType<lua::Func>(*this) )
	{
		lua::VarCast<lua::Func>(*this).call(a1,a2,a3);
	}
}
template<typename A1,typename A2,typename A3,typename A4>
void lua::Var::operator () (A1 a1,A2 a2,A3 a3,A4 a4)
{
	if ( lua::VarType<lua::Func>(*this) )
	{
		lua::VarCast<lua::Func>(*this).call(a1,a2,a3,a4);
	}
}
template<typename A1,typename A2,typename A3,typename A4,typename A5>
void lua::Var::operator () (A1 a1,A2 a2,A3 a3,A4 a4,A5 a5)
{
	if ( lua::VarType<lua::Func>(*this) )
	{
		lua::VarCast<lua::Func>(*this).call(a1,a2,a3,a4,a5);
	}
}
template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6>
void lua::Var::operator () (A1 a1,A2 a2,A3 a3,A4 a4,A5 a5,A6 a6)
{
	if ( lua::VarType<lua::Func>(*this) )
	{
		lua::VarCast<lua::Func>(*this).call(a1,a2,a3,a4,a5,a6);
	}
}
template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7>
void lua::Var::operator () (A1 a1,A2 a2,A3 a3,A4 a4,A5 a5,A6 a6,A7 a7)
{
	if ( lua::VarType<lua::Func>(*this) )
	{
		lua::VarCast<lua::Func>(*this).call(a1,a2,a3,a4,a5,a6,a7);
	}
}
template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8>
void lua::Var::operator () (A1 a1,A2 a2,A3 a3,A4 a4,A5 a5,A6 a6,A7 a7,A8 a8)
{
	if ( lua::VarType<lua::Func>(*this) )
	{
		lua::VarCast<lua::Func>(*this).call(a1,a2,a3,a4,a5,a6,a7,a8);
	}
}
template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8,typename A9>
void lua::Var::operator () (A1 a1,A2 a2,A3 a3,A4 a4,A5 a5,A6 a6,A7 a7,A8 a8,A9 a9)
{
	if ( lua::VarType<lua::Func>(*this) )
	{
		lua::VarCast<lua::Func>(*this).call(a1,a2,a3,a4,a5,a6,a7,a8,a9);
	}
}
//------------------------------------------------------------------------------

}
