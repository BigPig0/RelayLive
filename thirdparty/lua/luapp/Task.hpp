
#pragma once

#include "luapp/Handle.hpp"
#include "luapp/Register.hpp"


namespace lua{

class Task
{
	public:

		Task(){}

		~Task()
		{
			_item = NULL;   // Just make sure it released before this->_lua.
		}

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

inline Var::Var(const lua::Task &t):_ptr(0)
{
	this->_ptr = new ::lua::_VarType<lua::Task>(t);
}

inline Var& Var::operator = (const lua::Task &t)
{
	this->free_ptr();
	this->_ptr = new ::lua::_VarType<lua::Task>(t);
	return *this;
}


}
