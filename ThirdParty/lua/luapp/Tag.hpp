
#pragma once

#include "luapp/Handle.hpp"
#include "luapp/Register.hpp"

namespace lua{

class Tag
{
	public:

		Tag(){}

		~Tag()
		{
			_item = NULL;   // Just make sure it released before this->_lua.
		}

		template<typename T>
		bool type();

		void _set(lua::Handle h,lua::Register::Item i)
		{
			if ( _lua ) lua::Log<<"warning:why you set handle of Tag again?"<<lua::End;
			_item = i;
			_lua = h;
		}

		lua::Register::Item _getItem()
		{
			return this->_item;
		}

		lua::Handle          _lua;

	private:

		lua::Register::Item  _item;
};

inline Var::Var(const lua::Tag &t):_ptr(0)
{
	this->_ptr = new ::lua::_VarType<lua::Tag>(t);
}

inline Var& Var::operator = (const lua::Tag &t)
{
	this->free_ptr();
	this->_ptr = new ::lua::_VarType<lua::Tag>(t);
	return *this;
}

}
