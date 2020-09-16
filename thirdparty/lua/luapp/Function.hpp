
#pragma once

#include "luapp/MorePushAndPull.hpp"

namespace lua{

//---------------------------------------------------------------------

inline void Func::call()
{
	if ( ! _lua ) lua::Log<<"error:Func::void call()"<<lua::End;
	_item->getVar();         // ... [F]
	lua::PCall(_lua,0,0,0);  // ...
}

template<typename R>
R Func::call()
{
	if ( ! _lua ) lua::Log<<"error:Func::R call()"<<lua::End;
	_item->getVar();         // ... [F]
	lua::PCall(_lua,0,1,0);  // ... [R]
	R   result;
	lua::CheckVarFromLua(_lua,&result,-1);
	lua::Pop(_lua,1);
	return result;
}

//---------------------------------------------------------------------

template<typename A1>
void Func::call(A1 a1)
{
	if ( ! _lua ) lua::Log<<"error:Func::void call(A1 a1)"<<lua::End;
	_item->getVar();
	lua::PushVarToLua(_lua,a1);
	lua::PCall(_lua,1,0,0);
}

template<typename R,typename A1>
R Func::call(A1 a1)
{
	if ( ! _lua ) lua::Log<<"error:Func::R call(A1 a1)"<<lua::End;
	_item->getVar();
	lua::PushVarToLua(_lua,a1);
	lua::PCall(_lua,1,1,0);
	R   result;
	lua::CheckVarFromLua(_lua,&result,-1);
	lua::Pop(_lua,1);
	return result;
}

//---------------------------------------------------------------------

template<typename A1,typename A2>
void Func::call(A1 a1, A2 a2)
{
	if ( ! _lua ) lua::Log<<"error:Func::void call(A1 a1, A2 a2)"<<lua::End;
	_item->getVar();
	lua::PushVarToLua(_lua,a1);
	lua::PushVarToLua(_lua,a2);
	lua::PCall(_lua,2,0,0);
}

template<typename R,typename A1,typename A2>
R Func::call(A1 a1, A2 a2)
{
	if ( ! _lua ) lua::Log<<"error:Func::R call(A1 a1, A2 a2)"<<lua::End;
	_item->getVar();
	lua::PushVarToLua(_lua,a1);
	lua::PushVarToLua(_lua,a2);
	lua::PCall(_lua,2,1,0);
	R   result;
	lua::CheckVarFromLua(_lua,&result,-1);
	lua::Pop(_lua,1);
	return result;
}

//---------------------------------------------------------------------

template<typename A1,typename A2,typename A3>
void Func::call(A1 a1, A2 a2, A3 a3)
{
	if ( ! _lua ) lua::Log<<"error:Func::void call(A1 a1, A2 a2, A3 a3)"<<lua::End;
	_item->getVar();
	lua::PushVarToLua(_lua,a1);
	lua::PushVarToLua(_lua,a2);
	lua::PushVarToLua(_lua,a3);
	lua::PCall(_lua,3,0,0);
}

template<typename R,typename A1,typename A2,typename A3>
R Func::call(A1 a1, A2 a2, A3 a3)
{
	if ( ! _lua ) lua::Log<<"error:Func::R call(A1 a1, A2 a2, A3 a3)"<<lua::End;
	_item->getVar();
	lua::PushVarToLua(_lua,a1);
	lua::PushVarToLua(_lua,a2);
	lua::PushVarToLua(_lua,a3);
	lua::PCall(_lua,3,1,0);
	R   result;
	lua::CheckVarFromLua(_lua,&result,-1);
	lua::Pop(_lua,1);
	return result;
}

//---------------------------------------------------------------------

template<typename A1,typename A2,typename A3,typename A4>
void Func::call(A1 a1, A2 a2, A3 a3, A4 a4)
{
	if ( ! _lua ) lua::Log<<"error:Func::void call(A1 a1, A2 a2, A3 a3, A4 a4)"<<lua::End;
	_item->getVar();
	lua::PushVarToLua(_lua,a1);
	lua::PushVarToLua(_lua,a2);
	lua::PushVarToLua(_lua,a3);
	lua::PushVarToLua(_lua,a4);
	lua::PCall(_lua,4,0,0);
}

template<typename R,typename A1,typename A2,typename A3,typename A4>
R Func::call(A1 a1, A2 a2, A3 a3, A4 a4)
{
	if ( ! _lua ) lua::Log<<"error:Func::R call(A1 a1, A2 a2, A3 a3, A4 a4)"<<lua::End;
	_item->getVar();
	lua::PushVarToLua(_lua,a1);
	lua::PushVarToLua(_lua,a2);
	lua::PushVarToLua(_lua,a3);
	lua::PushVarToLua(_lua,a4);
	lua::PCall(_lua,4,1,0);
	R   result;
	lua::CheckVarFromLua(_lua,&result,-1);
	lua::Pop(_lua,1);
	return result;
}

//---------------------------------------------------------------------

template<typename A1,typename A2,typename A3,typename A4,typename A5>
void Func::call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
{
	if ( ! _lua ) lua::Log<<"error:Func::void call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)"<<lua::End;
	_item->getVar();
	lua::PushVarToLua(_lua,a1);
	lua::PushVarToLua(_lua,a2);
	lua::PushVarToLua(_lua,a3);
	lua::PushVarToLua(_lua,a4);
	lua::PushVarToLua(_lua,a5);
	lua::PCall(_lua,5,0,0);
}

template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5>
R Func::call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
{
	if ( ! _lua ) lua::Log<<"error:Func::R call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)"<<lua::End;
	_item->getVar();
	lua::PushVarToLua(_lua,a1);
	lua::PushVarToLua(_lua,a2);
	lua::PushVarToLua(_lua,a3);
	lua::PushVarToLua(_lua,a4);
	lua::PushVarToLua(_lua,a5);
	lua::PCall(_lua,5,1,0);
	R   result;
	lua::CheckVarFromLua(_lua,&result,-1);
	lua::Pop(_lua,1);
	return result;
}

//---------------------------------------------------------------------

template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6>
void Func::call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
{
	if ( ! _lua ) lua::Log<<"error:Func::void call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)"<<lua::End;
	_item->getVar();
	lua::PushVarToLua(_lua,a1);
	lua::PushVarToLua(_lua,a2);
	lua::PushVarToLua(_lua,a3);
	lua::PushVarToLua(_lua,a4);
	lua::PushVarToLua(_lua,a5);
	lua::PushVarToLua(_lua,a6);
	lua::PCall(_lua,6,0,0);
}

template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6>
R Func::call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
{
	if ( ! _lua ) lua::Log<<"error:Func::R call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)"<<lua::End;
	_item->getVar();
	lua::PushVarToLua(_lua,a1);
	lua::PushVarToLua(_lua,a2);
	lua::PushVarToLua(_lua,a3);
	lua::PushVarToLua(_lua,a4);
	lua::PushVarToLua(_lua,a5);
	lua::PushVarToLua(_lua,a6);
	lua::PCall(_lua,6,1,0);
	R   result;
	lua::CheckVarFromLua(_lua,&result,-1);
	lua::Pop(_lua,1);
	return result;
}

//---------------------------------------------------------------------

template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7>
void Func::call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
{
	if ( ! _lua ) lua::Log<<"error:Func::void call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)"<<lua::End;
	_item->getVar();
	lua::PushVarToLua(_lua,a1);
	lua::PushVarToLua(_lua,a2);
	lua::PushVarToLua(_lua,a3);
	lua::PushVarToLua(_lua,a4);
	lua::PushVarToLua(_lua,a5);
	lua::PushVarToLua(_lua,a6);
	lua::PushVarToLua(_lua,a7);
	lua::PCall(_lua,7,0,0);
}

template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7>
R Func::call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
{
	if ( ! _lua ) lua::Log<<"error:Func::R call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)"<<lua::End;
	_item->getVar();
	lua::PushVarToLua(_lua,a1);
	lua::PushVarToLua(_lua,a2);
	lua::PushVarToLua(_lua,a3);
	lua::PushVarToLua(_lua,a4);
	lua::PushVarToLua(_lua,a5);
	lua::PushVarToLua(_lua,a6);
	lua::PushVarToLua(_lua,a7);
	lua::PCall(_lua,7,1,0);
	R   result;
	lua::CheckVarFromLua(_lua,&result,-1);
	lua::Pop(_lua,1);
	return result;
}

//---------------------------------------------------------------------

template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8>
void Func::call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
{
	if ( ! _lua ) lua::Log<<"error:Func::void call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)"<<lua::End;
	_item->getVar();
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

template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8>
R Func::call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
{
	if ( ! _lua ) lua::Log<<"error:Func::R call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)"<<lua::End;
	_item->getVar();
	lua::PushVarToLua(_lua,a1);
	lua::PushVarToLua(_lua,a2);
	lua::PushVarToLua(_lua,a3);
	lua::PushVarToLua(_lua,a4);
	lua::PushVarToLua(_lua,a5);
	lua::PushVarToLua(_lua,a6);
	lua::PushVarToLua(_lua,a7);
	lua::PushVarToLua(_lua,a8);
	lua::PCall(_lua,8,1,0);
	R   result;
	lua::CheckVarFromLua(_lua,&result,-1);
	lua::Pop(_lua,1);
	return result;
}

//---------------------------------------------------------------------

template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8,typename A9>
void Func::call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
{
	if ( ! _lua ) lua::Log<<"error:Func::void call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)"<<lua::End;
	_item->getVar();
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

template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8,typename A9>
R Func::call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
{
	if ( ! _lua ) lua::Log<<"error:Func::R call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)"<<lua::End;
	_item->getVar();
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
	R   result;
	lua::CheckVarFromLua(_lua,&result,-1);
	lua::Pop(_lua,1);
	return result;
}

//------------------------------------------------------------------------------

template<typename T> struct Function{};

//---------------------------------------------------------------------

template<typename R, typename P1>
struct Function<R(P1)>
{
	public:

		Function(){}
		Function(const Func &f):_func(f){}
		~Function(){}

		R operator()(P1 p1)
		{
			return _func.call<R,P1>(p1);
		}

		lua::Func get() { return _func; }

	private:

		lua::Func   _func;
};

template<typename P1>
struct Function<void(P1)>
{
	public:

		Function(){}
		Function(const Func &f):_func(f){}
		~Function(){}

		void operator()(P1 p1)
		{
			_func.call(p1);
		}

		lua::Func get() { return _func; }

	private:

		lua::Func   _func;
};

//---------------------------------------------------------------------

template<typename R, typename P1, typename P2>
struct Function<R(P1,P2)>
{
	public:

		Function(){}
		Function(const Func &f):_func(f){}
		~Function(){}

		R operator()(P1 p1,P2 p2)
		{
			return _func.call<R,P1,P2>(p1,p2);
		}

		lua::Func get() { return _func; }

	private:

		lua::Func   _func;
};

template<typename P1, typename P2>
struct Function<void(P1,P2)>
{
	public:

		Function(){}
		Function(const Func &f):_func(f){}
		~Function(){}

		void operator()(P1 p1,P2 p2)
		{
			_func.call(p1,p2);
		}

		lua::Func get() { return _func; }

	private:

		lua::Func   _func;
};

//---------------------------------------------------------------------

template<typename R, typename P1, typename P2, typename P3>
struct Function<R(P1,P2,P3)>
{
	public:

		Function(){}
		Function(const Func &f):_func(f){}
		~Function(){}

		R operator()(P1 p1,P2 p2,P3 p3)
		{
			return _func.call<R,P1,P2,P3>(p1,p2,p3);
		}

		lua::Func get() { return _func; }

	private:

		lua::Func   _func;
};

template<typename P1, typename P2, typename P3>
struct Function<void(P1,P2,P3)>
{
	public:

		Function(){}
		Function(const Func &f):_func(f){}
		~Function(){}

		void operator()(P1 p1,P2 p2,P3 p3)
		{
			_func.call(p1,p2,p3);
		}

		lua::Func get() { return _func; }

	private:

		lua::Func   _func;
};

//---------------------------------------------------------------------

template<typename R, typename P1, typename P2, typename P3, typename P4>
struct Function<R(P1,P2,P3,P4)>
{
	public:

		Function(){}
		Function(const Func &f):_func(f){}
		~Function(){}

		R operator()(P1 p1,P2 p2,P3 p3,P4 p4)
		{
			return _func.call<R,P1,P2,P3,P4>(p1,p2,p3,p4);
		}

		lua::Func get() { return _func; }

	private:

		lua::Func   _func;
};

template<typename P1, typename P2, typename P3, typename P4>
struct Function<void(P1,P2,P3,P4)>
{
	public:

		Function(){}
		Function(const Func &f):_func(f){}
		~Function(){}

		void operator()(P1 p1,P2 p2,P3 p3,P4 p4)
		{
			_func.call(p1,p2,p3,p4);
		}

		lua::Func get() { return _func; }

	private:

		lua::Func   _func;
};

//---------------------------------------------------------------------

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
struct Function<R(P1,P2,P3,P4,P5)>
{
	public:

		Function(){}
		Function(const Func &f):_func(f){}
		~Function(){}

		R operator()(P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
		{
			return _func.call<R,P1,P2,P3,P4,P5>(p1,p2,p3,p4,p5);
		}

		lua::Func get() { return _func; }

	private:

		lua::Func   _func;
};

template<typename P1, typename P2, typename P3, typename P4, typename P5>
struct Function<void(P1,P2,P3,P4,P5)>
{
	public:

		Function(){}
		Function(const Func &f):_func(f){}
		~Function(){}

		void operator()(P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
		{
			_func.call(p1,p2,p3,p4,p5);
		}

		lua::Func get() { return _func; }

	private:

		lua::Func   _func;
};

//---------------------------------------------------------------------

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
struct Function<R(P1,P2,P3,P4,P5,P6)>
{
	public:

		Function(){}
		Function(const Func &f):_func(f){}
		~Function(){}

		R operator()(P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6)
		{
			return _func.call<R,P1,P2,P3,P4,P5,P6>(p1,p2,p3,p4,p5,p6);
		}

		lua::Func get() { return _func; }

	private:

		lua::Func   _func;
};

template<typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
struct Function<void(P1,P2,P3,P4,P5,P6)>
{
	public:

		Function(){}
		Function(const Func &f):_func(f){}
		~Function(){}

		void operator()(P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6)
		{
			_func.call(p1,p2,p3,p4,p5,p6);
		}

		lua::Func get() { return _func; }

	private:

		lua::Func   _func;
};

//---------------------------------------------------------------------

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
struct Function<R(P1,P2,P3,P4,P5,P6,P7)>
{
	public:

		Function(){}
		Function(const Func &f):_func(f){}
		~Function(){}

		R operator()(P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7)
		{
			return _func.call<R,P1,P2,P3,P4,P5,P6,P7>(p1,p2,p3,p4,p5,p6,p7);
		}

		lua::Func get() { return _func; }

	private:

		lua::Func   _func;
};

template<typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
struct Function<void(P1,P2,P3,P4,P5,P6,P7)>
{
	public:

		Function(){}
		Function(const Func &f):_func(f){}
		~Function(){}

		void operator()(P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7)
		{
			_func.call(p1,p2,p3,p4,p5,p6,p7);
		}

		lua::Func get() { return _func; }

	private:

		lua::Func   _func;
};

//---------------------------------------------------------------------

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
struct Function<R(P1,P2,P3,P4,P5,P6,P7,P8)>
{
	public:

		Function(){}
		Function(const Func &f):_func(f){}
		~Function(){}

		R operator()(P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7,P8 p8)
		{
			return _func.call<R,P1,P2,P3,P4,P5,P6,P7,P8>(p1,p2,p3,p4,p5,p6,p7,p8);
		}

		lua::Func get() { return _func; }

	private:

		lua::Func   _func;
};

template<typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
struct Function<void(P1,P2,P3,P4,P5,P6,P7,P8)>
{
	public:

		Function(){}
		Function(const Func &f):_func(f){}
		~Function(){}

		void operator()(P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7,P8 p8)
		{
			_func.call(p1,p2,p3,p4,p5,p6,p7,p8);
		}

		lua::Func get() { return _func; }

	private:

		lua::Func   _func;
};

//---------------------------------------------------------------------

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
struct Function<R(P1,P2,P3,P4,P5,P6,P7,P8,P9)>
{
	public:

		Function(){}
		Function(const Func &f):_func(f){}
		~Function(){}

		R operator()(P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7,P8 p8,P9 p9)
		{
			return _func.call<R,P1,P2,P3,P4,P5,P6,P7,P8,P9>(p1,p2,p3,p4,p5,p6,p7,p8,p9);
		}

		lua::Func get() { return _func; }

	private:

		lua::Func   _func;
};

template<typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
struct Function<void(P1,P2,P3,P4,P5,P6,P7,P8,P9)>
{
	public:

		Function(){}
		Function(const Func &f):_func(f){}
		~Function(){}

		void operator()(P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7,P8 p8,P9 p9)
		{
			_func.call(p1,p2,p3,p4,p5,p6,p7,p8,p9);
		}

		lua::Func get() { return _func; }

	private:

		lua::Func   _func;
};

//---------------------------------------------------------------------

}//namespace lua
