
#pragma once

#include "luapp/Var.hpp"
#include "luapp/LuaAPI.hpp"
#include "luapp/stl/memory.hpp"

namespace lua{


// Only work inside lua::HandleClass.
class Register
{
	public:

		class ItemClass
		{
			public:

				ItemClass(lua::Int id,Register *r):_id(id),_mgr(r)
				{
					;
				}

				~ItemClass()
				{
					_mgr->_remove(_id);
				}

				void setVar()
				{
					_mgr->_setVar(_id);
				}

				void getVar()
				{
					_mgr->_getVar(_id);
				}

				lua::Int   _id;
				Register*  _mgr;
		};

		typedef std::shared_ptr<ItemClass> Item;

		Register(lua::NativeState h):_lua(h)
		{
			if ( ! lua::IsGlobal(h,"_luapp_data") )
			{
				buildDataBase();
			}
		}

		~Register(){}

		std::shared_ptr<ItemClass> newItem()
		{
			return std::shared_ptr<ItemClass>(new ItemClass(createUniqueID(),this));
		}

		void _setVar(lua::Int id)
		{
			                                            // ... [V]
			lua::GetGlobal(_lua, "_luapp_data");        // ... [V] [data]
			lua::GetField(_lua,-1, "temp_variable");    // ... [V] [data] [temp]
			lua::PushInteger(_lua,id);                  // ... [V] [data] [temp] [id]
			lua::PushValue(_lua,-4);                    // ... [V] [data] [temp] [id] [V]
			lua::SetTable(_lua,-3);                     // ... [V] [data] [temp]
			lua::Pop(_lua,3);                           // ...
		}

		void _getVar(lua::Int id)
		{
			lua::GetGlobal(_lua, "_luapp_data");        // ... [data]
			lua::GetField(_lua,-1, "temp_variable");    // ... [data] [temp]

			lua::PushInteger(_lua,id);                  // ... [data] [temp] [id]
			lua::GetTable(_lua,-2);                     // ... [data] [temp] [value]

			lua::Replace(_lua,-3);                      // ... [value] [temp]
			lua::Pop(_lua,1);                           // ... [value]
		}

		void _remove(lua::Int id)
		{
			lua::GetGlobal(_lua, "_luapp_data");        // ... [data]
			lua::GetField(_lua,-1, "temp_variable");    // ... [data] [temp]

			lua::PushInteger(_lua,id);                  // ... [data] [temp] [id]
			lua::PushNil(_lua);                         // ... [data] [temp] [id] [nil]
			lua::SetTable(_lua,-3);                     // ... [data] [temp]
			lua::Pop(_lua,1);                           // ... [data]

			lua::GetField(_lua,-1, "id_count");         // ... [data] [count]

			lua::Int    count = lua::CheckInteger(_lua,-1);
			lua::Pop(_lua,1);                           // ... [data]

			if ( id>count )
			{
				lua::Log<<"error:can't remove a id that doesn't exist."<<lua::End;
				lua::Pop(_lua,1);                       // ...
				return;
			}

			lua::GetField(_lua,-1, "unused_id_list");   // ... [data] [list]
			lua::PushInteger(_lua,id);                  // ... [data] [list] [key]
			lua::PushBoolean(_lua,true);                // ... [data] [list] [key] [value]
			lua::SetTable(_lua,-3);                     // ... [data] [list]
			lua::Pop(_lua,2);                           // ...
		}

	private:

		void buildDataBase()
		{
			lua::NewTable(_lua);                       // ... [T]

			lua::PushString(_lua,"temp_variable");     // ... [T] [temp]
			lua::NewTable(_lua);                       // ... [T] [temp] [value]
			lua::SetTable(_lua,-3);                    // ... [T]

			lua::PushString(_lua,"unused_id_list");    // ... [T] [name]
			lua::NewTable(_lua);                       // ... [T] [name] [list]
			lua::SetTable(_lua,-3);                    // ... [T]

			lua::PushString(_lua,"id_count");          // ... [T] [name]
			lua::PushInteger(_lua,0);                  // ... [T] [name] 0
			lua::SetTable(_lua,-3);                    // ... [T]

			lua::SetGlobal(_lua, "_luapp_data");       // ...
		}

		lua::Int createUniqueID()
		{
			lua::GetGlobal(_lua, "_luapp_data");       // ... [data]
			lua::GetField(_lua,-1, "unused_id_list");  // ... [data] [list]

			lua::Int    id = -1;

			lua::PushNil(_lua);                        // ... [data] [list] [nil]

			while ( lua_next(_lua, -2) != 0 )
			{
				                                       // ... [data] [list] [key] [value]
				id = lua::CheckInteger(_lua,-2);

				// Remove ID from unused list.
				lua::Pop(_lua,1);                      // ... [data] [list] [key]
				lua::PushNil(_lua);                    // ... [data] [list] [key] [nil]
				lua::SetTable(_lua,-3);                // ... [data] [list]
				break;
			}
			                                           // ... [data] [list]
			if ( id==-1 )
			{
				lua::Int    count = 0;
				lua::GetField(_lua,-2, "id_count");    // ... [data] [list] [count]
				count = lua::CheckInteger(_lua,-1);
				lua::Pop(_lua,1);                      // ... [data] [list]
				count++;
				id = count;

				lua::PushString(_lua,"id_count");      // ... [data] [list] [name]
				lua::PushInteger(_lua,count);          // ... [data] [list] [name] [count]
				lua::SetTable(_lua,-4);                // ... [data] [list]
			}

			lua::Pop(_lua,2);                          // ...

			return id;
		}

		lua::NativeState    _lua;
};



}//namespace lua
