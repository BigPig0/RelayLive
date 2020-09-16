
#pragma once

#include "luapp/Handle.hpp"
#include "luapp/Register.hpp"

namespace lua{

// Work for lua::Map::operator >>
struct _Map_Address
{
	_Map_Address(){}
	~_Map_Address(){}

	// They implemented at luapp/MorePushAndPull.hpp
	void _checkVar(lua::Var *var);
	template<typename T>
	_Map_Address& operator >> (const T key);
	_Map_Address& operator >> (const char* key);

	lua::Handle          _lua;
};

class Map
{
	public:

		// Work for lua::Map::operator []
		struct _Value
		{
			_Value(Map *m):_map(m),_level(0){}
			~_Value(){}

			// They implemented at luapp/MorePushAndPull.hpp
			template<typename T>
			_Value& operator [] (const T key);
			_Value& operator [] (const char* key);
			lua::Nil operator = (lua::Nil value);
			lua::Str operator = (lua::Str value);
			lua::Int operator = (lua::Int value);
			lua::Num operator = (lua::Num value);
			lua::Ptr operator = (lua::Ptr value);
			lua::Var operator = (lua::Var value);
			lua::Map operator = (lua::Map value);
			lua::Bool operator = (lua::Bool value);
			lua::Func operator = (lua::Func value);
			lua::Task operator = (lua::Task value);
			lua::User operator = (lua::User value);
			lua::Table operator = (lua::Table value);

			Map   *_map;
			int   _level;
		};

		Map():_temp(this){}

		~Map()
		{
			_item = NULL;   // Just make sure it released before this->_lua.
		}

		Map(const Map& bro):_temp(this)
		{
			copy_my_kind(bro);
		}

		Map& operator = (const Map& bro)
		{
			copy_my_kind(bro);
			return *this;
		}

		// It implemented at luapp/MorePushAndPull.hpp
		template<typename T>
		lua::_Map_Address& operator >> (const T key);
		lua::_Map_Address& operator >> (const char* key);
		template<typename T>
		_Value& operator [] (const T key);
		_Value& operator [] (const char* key);

		void _set(lua::Handle h,lua::Register::Item i)
		{
			if ( _lua ) lua::Log<<"warning:why you set handle of function again?"<<lua::End;
			_item = i;
			_lua = h;

			_temp2._lua = _lua;
		}

		lua::Register::Item _getItem()
		{
			return this->_item;
		}

	private:

		void copy_my_kind(const Map& _bro)
		{
			Map   &bro = const_cast<Map&>(_bro);
			this->_lua  = bro._lua;
			this->_item = bro._item;
			this->_temp._map = bro._temp._map;
			this->_temp2._lua = bro._lua;
		}

		lua::Handle          _lua;
		lua::Register::Item  _item;
		_Value               _temp;
		lua::_Map_Address    _temp2;
};

inline Var::Var(const lua::Map &t):_ptr(0)
{
	this->_ptr = new ::lua::_VarType<lua::Map>(t);
}

inline Var& Var::operator = (const lua::Map &t)
{
	this->free_ptr();
	this->_ptr = new ::lua::_VarType<lua::Map>(t);
	return *this;
}


}
