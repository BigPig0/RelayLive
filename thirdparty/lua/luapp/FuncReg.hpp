
#pragma once


#include <vector>
#include <string>
#include "luapp/Config.hpp"
#include "luapp/ShortName.hpp"

namespace lua{

/**
 * It's a function register for lua.
 * Basically, this class just a manager of luaL_Reg.
 */
class FuncReg
{
	public:

		// lua::FuncReg::Item as luaL_Reg.
		#ifdef _LUAPP_CPP11_2
		struct Item
		{
			Name       name = nullptr;
			CFunction  func = nullptr;
		};
		#else
		struct Item
		{
			Item():name(NULL),func(NULL){}
			Name       name;
			CFunction  func;
		};
		#endif

		FuncReg():_index(0),_size(4),_data(0)
		{
			_data = new Item [_size];
		}

		~FuncReg()
		{
			delete [] _data;
		}

		void add(std::string name, CFunction func)
		{
			#ifdef _LUAPP_CHECK_CAREFUL_
			if ( func ==NULL )
			{
				lua::Log<<"error:never enter null pointer to lua::FuncReg"<<lua::End;
				return;
			}
			#endif

			_nameList.push_back(name);

			_data[_index].func = func;
			_index++;

			// If it get n elements in list, then list size alway bigger than n+1.
			if ( _index+1 == _size )
			{
				get_more_memory();
			}
		}

		Item* _get()
		{
			refresh();
			return _data;
		}

	private:

		void refresh()
		{
			for ( int i=0 ; i<_index ; i++ )
			{
				_data[i].name = _nameList[i].c_str();
			}
		}

		void get_more_memory()
		{
			_size = _size*2;

			Item    *new_block = new Item [_size];

			for ( int i=0 ; _data[i].func!=NULL ; i++ )
			{
				new_block[i] = _data[i];
			}

			delete [] _data;
			_data = new_block;
		}

		int     _index;
		int     _size;
		Item*   _data;
		std::vector<std::string>   _nameList;
};

}
