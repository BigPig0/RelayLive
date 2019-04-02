/**
@file   WrapperProxy.hpp
@brief  To get all of parameters.
*/


#pragma once


#include "luapp/Standard.hpp"
#include "luapp/stl/functional.hpp"


namespace lua{
namespace wrapper{


//------------------------------------------------------------

struct Proxy
{
	virtual ~Proxy(){}
	virtual int Do(lua::Handle)=0;
};

//------------------------------------------------------------

template<typename R>
struct ProxyReturn : public Proxy
{
	void returnValue(lua::Handle L,R num)
	{
		PushVarToLua(L,num);
	}
};

//------------------------------------------------------------

template<typename R>
struct Proxy00 : public ProxyReturn<R>
{
	typedef R(*Func)();

	Proxy00(){}
	Proxy00(Func fn):func00(fn){}

	Func    func00;

	int Do(lua::Handle L)
	{
		this->returnValue(L,func00());
		return (int)1;
	}
};

template<>
struct Proxy00<void> : public Proxy
{
	typedef void R;
	typedef R(*Func)();

	Proxy00(){}
	Proxy00(Func fn):func00(fn){}

	Func    func00;

	int Do(lua::Handle)
	{
		func00();
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename R,typename A1>
struct Proxy01 : public ProxyReturn<R>
{
	typedef R (*Func)(A1);

	Proxy01(){}
	Proxy01(Func fn):func01(fn){}

	Func    func01;

	int Do(lua::Handle L)
	{
		A1      p1;
		lua::CheckVarFromLua(L,&p1,1);
		this->returnValue(L,func01(p1));
		return (int)1;
	}
};

template<typename A1>
struct Proxy01<void,A1> : public Proxy
{
	typedef void (*Func)(A1);

	Proxy01(){}
	Proxy01(Func fn):func01(fn){}

	Func    func01;

	int Do(lua::Handle L)
	{
		A1      p1;
		lua::CheckVarFromLua(L,&p1,1);
		func01(p1);
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename R,typename A1,typename A2>
struct Proxy02 : public ProxyReturn<R>
{
	typedef R (*Func)(A1,A2);

	Proxy02(){}
	Proxy02(Func fn):func02(fn){}

	Func    func02;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		this->returnValue(L,func02(p1,p2));
		return (int)1;
	}
};

template<typename A1,typename A2>
struct Proxy02<void,A1,A2> : public Proxy
{
	typedef void (*Func)(A1,A2);

	Proxy02(){}
	Proxy02(Func fn):func02(fn){}

	Func    func02;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		func02(p1,p2);
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename R,typename A1,typename A2,typename A3>
struct Proxy03 : public ProxyReturn<R>
{
	typedef R (*Func)(A1,A2,A3);

	Proxy03(){}
	Proxy03(Func fn):func03(fn){}

	Func    func03;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		this->returnValue(L,func03(p1,p2,p3));
		return (int)1;
	}
};

template<typename A1,typename A2,typename A3>
struct Proxy03<void,A1,A2,A3> : public Proxy
{
	typedef void (*Func)(A1,A2,A3);

	Proxy03(){}
	Proxy03(Func fn):func03(fn){}

	Func    func03;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		func03(p1,p2,p3);
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename R,typename A1,typename A2,typename A3,typename A4>
struct Proxy04 : public ProxyReturn<R>
{
	typedef R (*Func)(A1,A2,A3,A4);

	Proxy04(){}
	Proxy04(Func fn):func04(fn){}

	Func    func04;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		this->returnValue(L,func04(p1,p2,p3,p4));
		return (int)1;
	}
};

template<typename A1,typename A2,typename A3,typename A4>
struct Proxy04<void,A1,A2,A3,A4> : public Proxy
{
	typedef void (*Func)(A1,A2,A3,A4);

	Proxy04(){}
	Proxy04(Func fn):func04(fn){}

	Func    func04;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		func04(p1,p2,p3,p4);
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5>
struct Proxy05 : public ProxyReturn<R>
{
	typedef R (*Func)(A1,A2,A3,A4,A5);

	Proxy05(){}
	Proxy05(Func fn):func05(fn){}

	Func    func05;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		this->returnValue(L,func05(p1,p2,p3,p4,p5));
		return (int)1;
	}
};

template<typename A1,typename A2,typename A3,typename A4,typename A5>
struct Proxy05<void,A1,A2,A3,A4,A5> : public Proxy
{
	typedef void (*Func)(A1,A2,A3,A4,A5);

	Proxy05(){}
	Proxy05(Func fn):func05(fn){}

	Func    func05;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		func05(p1,p2,p3,p4,p5);
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6>
struct Proxy06 : public ProxyReturn<R>
{
	typedef R (*Func)(A1,A2,A3,A4,A5,A6);

	Proxy06(){}
	Proxy06(Func fn):func06(fn){}

	Func    func06;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		A6      p6;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		lua::CheckVarFromLua(L,&p6,6);
		this->returnValue(L,func06(p1,p2,p3,p4,p5,p6));
		return (int)1;
	}
};

template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6>
struct Proxy06<void,A1,A2,A3,A4,A5,A6> : public Proxy
{
	typedef void (*Func)(A1,A2,A3,A4,A5,A6);

	Proxy06(){}
	Proxy06(Func fn):func06(fn){}

	Func    func06;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		A6      p6;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		lua::CheckVarFromLua(L,&p6,6);
		func06(p1,p2,p3,p4,p5,p6);
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7>
struct Proxy07 : public ProxyReturn<R>
{
	typedef R (*Func)(A1,A2,A3,A4,A5,A6,A7);

	Proxy07(){}
	Proxy07(Func fn):func07(fn){}

	Func    func07;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		A6      p6;
		A7      p7;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		lua::CheckVarFromLua(L,&p6,6);
		lua::CheckVarFromLua(L,&p7,7);
		this->returnValue(L,func07(p1,p2,p3,p4,p5,p6,p7));
		return (int)1;
	}
};

template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7>
struct Proxy07<void,A1,A2,A3,A4,A5,A6,A7> : public Proxy
{
	typedef void (*Func)(A1,A2,A3,A4,A5,A6,A7);

	Proxy07(){}
	Proxy07(Func fn):func07(fn){}

	Func    func07;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		A6      p6;
		A7      p7;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		lua::CheckVarFromLua(L,&p6,6);
		lua::CheckVarFromLua(L,&p7,7);
		func07(p1,p2,p3,p4,p5,p6,p7);
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8>
struct Proxy08 : public ProxyReturn<R>
{
	typedef R (*Func)(A1,A2,A3,A4,A5,A6,A7,A8);

	Proxy08(){}
	Proxy08(Func fn):func08(fn){}

	Func    func08;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		A6      p6;
		A7      p7;
		A8      p8;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		lua::CheckVarFromLua(L,&p6,6);
		lua::CheckVarFromLua(L,&p7,7);
		lua::CheckVarFromLua(L,&p8,8);
		this->returnValue(L,func08(p1,p2,p3,p4,p5,p6,p7,p8));
		return (int)1;
	}
};

template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8>
struct Proxy08<void,A1,A2,A3,A4,A5,A6,A7,A8> : public Proxy
{
	typedef void (*Func)(A1,A2,A3,A4,A5,A6,A7,A8);

	Proxy08(){}
	Proxy08(Func fn):func08(fn){}

	Func    func08;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		A6      p6;
		A7      p7;
		A8      p8;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		lua::CheckVarFromLua(L,&p6,6);
		lua::CheckVarFromLua(L,&p7,7);
		lua::CheckVarFromLua(L,&p8,8);
		func08(p1,p2,p3,p4,p5,p6,p7,p8);
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8,typename A9>
struct Proxy09 : public ProxyReturn<R>
{
	typedef R (*Func)(A1,A2,A3,A4,A5,A6,A7,A8,A9);

	Proxy09(){}
	Proxy09(Func fn):func09(fn){}

	Func    func09;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		A6      p6;
		A7      p7;
		A8      p8;
		A9      p9;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		lua::CheckVarFromLua(L,&p6,6);
		lua::CheckVarFromLua(L,&p7,7);
		lua::CheckVarFromLua(L,&p8,8);
		lua::CheckVarFromLua(L,&p9,9);
		this->returnValue(L,func09(p1,p2,p3,p4,p5,p6,p7,p8,p9));
		return (int)1;
	}
};

template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8,typename A9>
struct Proxy09<void,A1,A2,A3,A4,A5,A6,A7,A8,A9> : public Proxy
{
	typedef void (*Func)(A1,A2,A3,A4,A5,A6,A7,A8,A9);

	Proxy09(){}
	Proxy09(Func fn):func09(fn){}

	Func    func09;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		A6      p6;
		A7      p7;
		A8      p8;
		A9      p9;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		lua::CheckVarFromLua(L,&p6,6);
		lua::CheckVarFromLua(L,&p7,7);
		lua::CheckVarFromLua(L,&p8,8);
		lua::CheckVarFromLua(L,&p9,9);
		func09(p1,p2,p3,p4,p5,p6,p7,p8,p9);
		return (int)1;
	}
};

//------------------------------------------------------------

template <typename R>
static Proxy* GetProxy(R(*f)())
{
	return dynamic_cast<Proxy*>(new Proxy00<R>(f));
}

template <typename R,typename A1>
static Proxy* GetProxy(R(*f)(A1))
{
	return dynamic_cast<Proxy*>(new Proxy01<R,A1>(f));
}

template <typename R,typename A1,typename A2>
static Proxy* GetProxy(R(*f)(A1,A2))
{
	return dynamic_cast<Proxy*>(new Proxy02<R,A1,A2>(f));
}

template <typename R,typename A1,typename A2,typename A3>
static Proxy* GetProxy(R(*f)(A1,A2,A3))
{
	return dynamic_cast<Proxy*>(new Proxy03<R,A1,A2,A3>(f));
}

template <typename R,typename A1,typename A2,typename A3,typename A4>
static Proxy* GetProxy(R(*f)(A1,A2,A3,A4))
{
	return dynamic_cast<Proxy*>(new Proxy04<R,A1,A2,A3,A4>(f));
}

template <typename R,typename A1,typename A2,typename A3,typename A4,typename A5>
static Proxy* GetProxy(R(*f)(A1,A2,A3,A4,A5))
{
	return dynamic_cast<Proxy*>(new Proxy05<R,A1,A2,A3,A4,A5>(f));
}

template <typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6>
static Proxy* GetProxy(R(*f)(A1,A2,A3,A4,A5,A6))
{
	return dynamic_cast<Proxy*>(new Proxy06<R,A1,A2,A3,A4,A5,A6>(f));
}

template <typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7>
static Proxy* GetProxy(R(*f)(A1,A2,A3,A4,A5,A6,A7))
{
	return dynamic_cast<Proxy*>(new Proxy07<R,A1,A2,A3,A4,A5,A6,A7>(f));
}

template <typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8>
static Proxy* GetProxy(R(*f)(A1,A2,A3,A4,A5,A6,A7,A8))
{
	return dynamic_cast<Proxy*>(new Proxy08<R,A1,A2,A3,A4,A5,A6,A7,A8>(f));
}

template <typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8,typename A9>
static Proxy* GetProxy(R(*f)(A1,A2,A3,A4,A5,A6,A7,A8,A9))
{
	return dynamic_cast<Proxy*>(new Proxy09<R,A1,A2,A3,A4,A5,A6,A7,A8,A9>(f));
}

//------------------------------------------------------------

template<typename C,typename R>
struct ProxyBind00 : public ProxyReturn<R>
{
	typedef R (C::*Func)();

	ProxyBind00(){}
	ProxyBind00(Func fn,C *c):func(fn),obj(c){}

	Func    func;
	C*      obj;

	int Do(lua::Handle L)
	{
		this->returnValue(L,(obj->*func)());
		return (int)1;
	}
};

template<typename C>
struct ProxyBind00<C,void> : public Proxy
{
	typedef void R;
	typedef R (C::*Func)();

	ProxyBind00(){}
	ProxyBind00(Func fn,C *c):func(fn),obj(c){}

	Func    func;
	C*      obj;

	int Do(lua::Handle)
	{
		(obj->*func)();
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename C,typename R,typename A1>
struct ProxyBind01 : public ProxyReturn<R>
{
	typedef R (C::*Func)(A1);

	ProxyBind01(){}
	ProxyBind01(Func fn,C *c):func(fn),obj(c){}

	Func    func;
	C*      obj;

	int Do(lua::Handle L)
	{
		A1      p1;
		lua::CheckVarFromLua(L,&p1,1);
		this->returnValue(L,(obj->*func)(p1));
		return (int)1;
	}
};

template<typename C,typename A1>
struct ProxyBind01<C,void,A1> : public Proxy
{
	typedef void (C::*Func)(A1);

	ProxyBind01(){}
	ProxyBind01(Func fn,C *c):func(fn),obj(c){}

	Func    func;
	C*      obj;

	int Do(lua::Handle L)
	{
		A1      p1;
		lua::CheckVarFromLua(L,&p1,1);
		(obj->*func)(p1);
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename C,typename R,typename A1,typename A2>
struct ProxyBind02 : public ProxyReturn<R>
{
	typedef R (C::*Func)(A1,A2);

	ProxyBind02(){}
	ProxyBind02(Func fn,C *c):func(fn),obj(c){}

	Func    func;
	C*      obj;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		this->returnValue(L,(obj->*func)(p1,p2));
		return (int)1;
	}
};

template<typename C,typename A1,typename A2>
struct ProxyBind02<C,void,A1,A2> : public Proxy
{
	typedef void (C::*Func)(A1,A2);

	ProxyBind02(){}
	ProxyBind02(Func fn,C *c):func(fn),obj(c){}

	Func    func;
	C*      obj;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		(obj->*func)(p1,p2);
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename C,typename R,typename A1,typename A2,typename A3>
struct ProxyBind03 : public ProxyReturn<R>
{
	typedef R (C::*Func)(A1,A2,A3);

	ProxyBind03(){}
	ProxyBind03(Func fn,C *c):func(fn),obj(c){}

	Func    func;
	C*      obj;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		this->returnValue(L,(obj->*func)(p1,p2,p3));
		return (int)1;
	}
};

template<typename C,typename A1,typename A2,typename A3>
struct ProxyBind03<C,void,A1,A2,A3> : public Proxy
{
	typedef void (C::*Func)(A1,A2,A3);

	ProxyBind03(){}
	ProxyBind03(Func fn,C *c):func(fn),obj(c){}

	Func    func;
	C*      obj;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		(obj->*func)(p1,p2,p3);
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename C,typename R,typename A1,typename A2,typename A3,typename A4>
struct ProxyBind04 : public ProxyReturn<R>
{
	typedef R (C::*Func)(A1,A2,A3,A4);

	ProxyBind04(){}
	ProxyBind04(Func fn,C *c):func(fn),obj(c){}

	Func    func;
	C*      obj;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		this->returnValue(L,(obj->*func)(p1,p2,p3,p4));
		return (int)1;
	}
};

template<typename C,typename A1,typename A2,typename A3,typename A4>
struct ProxyBind04<C,void,A1,A2,A3,A4> : public Proxy
{
	typedef void (C::*Func)(A1,A2,A3,A4);

	ProxyBind04(){}
	ProxyBind04(Func fn,C *c):func(fn),obj(c){}

	Func    func;
	C*      obj;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		(obj->*func)(p1,p2,p3,p4);
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename C,typename R,typename A1,typename A2,typename A3,typename A4,typename A5>
struct ProxyBind05 : public ProxyReturn<R>
{
	typedef R (C::*Func)(A1,A2,A3,A4,A5);

	ProxyBind05(){}
	ProxyBind05(Func fn,C *c):func(fn),obj(c){}

	Func    func;
	C*      obj;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		this->returnValue(L,(obj->*func)(p1,p2,p3,p4,p5));
		return (int)1;
	}
};

template<typename C,typename A1,typename A2,typename A3,typename A4,typename A5>
struct ProxyBind05<C,void,A1,A2,A3,A4,A5> : public Proxy
{
	typedef void (C::*Func)(A1,A2,A3,A4,A5);

	ProxyBind05(){}
	ProxyBind05(Func fn,C *c):func(fn),obj(c){}

	Func    func;
	C*      obj;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		(obj->*func)(p1,p2,p3,p4,p5);
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename C,typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6>
struct ProxyBind06 : public ProxyReturn<R>
{
	typedef R (C::*Func)(A1,A2,A3,A4,A5,A6);

	ProxyBind06(){}
	ProxyBind06(Func fn,C *c):func(fn),obj(c){}

	Func    func;
	C*      obj;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		A6      p6;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		lua::CheckVarFromLua(L,&p6,6);
		this->returnValue(L,(obj->*func)(p1,p2,p3,p4,p5,p6));
		return (int)1;
	}
};

template<typename C,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6>
struct ProxyBind06<C,void,A1,A2,A3,A4,A5,A6> : public Proxy
{
	typedef void (C::*Func)(A1,A2,A3,A4,A5,A6);

	ProxyBind06(){}
	ProxyBind06(Func fn,C *c):func(fn),obj(c){}

	Func    func;
	C*      obj;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		A6      p6;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		lua::CheckVarFromLua(L,&p6,6);
		(obj->*func)(p1,p2,p3,p4,p5,p6);
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename C,typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7>
struct ProxyBind07 : public ProxyReturn<R>
{
	typedef R (C::*Func)(A1,A2,A3,A4,A5,A6,A7);

	ProxyBind07(){}
	ProxyBind07(Func fn,C *c):func(fn),obj(c){}

	Func    func;
	C*      obj;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		A6      p6;
		A7      p7;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		lua::CheckVarFromLua(L,&p6,6);
		lua::CheckVarFromLua(L,&p7,7);
		this->returnValue(L,(obj->*func)(p1,p2,p3,p4,p5,p6,p7));
		return (int)1;
	}
};

template<typename C,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7>
struct ProxyBind07<C,void,A1,A2,A3,A4,A5,A6,A7> : public Proxy
{
	typedef void (C::*Func)(A1,A2,A3,A4,A5,A6,A7);

	ProxyBind07(){}
	ProxyBind07(Func fn,C *c):func(fn),obj(c){}

	Func    func;
	C*      obj;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		A6      p6;
		A7      p7;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		lua::CheckVarFromLua(L,&p6,6);
		lua::CheckVarFromLua(L,&p7,7);
		(obj->*func)(p1,p2,p3,p4,p5,p6,p7);
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename C,typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8>
struct ProxyBind08 : public ProxyReturn<R>
{
	typedef R (C::*Func)(A1,A2,A3,A4,A5,A6,A7,A8);

	ProxyBind08(){}
	ProxyBind08(Func fn,C *c):func(fn),obj(c){}

	Func    func;
	C*      obj;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		A6      p6;
		A7      p7;
		A8      p8;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		lua::CheckVarFromLua(L,&p6,6);
		lua::CheckVarFromLua(L,&p7,7);
		lua::CheckVarFromLua(L,&p8,8);
		this->returnValue(L,(obj->*func)(p1,p2,p3,p4,p5,p6,p7,p8));
		return (int)1;
	}
};

template<typename C,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8>
struct ProxyBind08<C,void,A1,A2,A3,A4,A5,A6,A7,A8> : public Proxy
{
	typedef void (C::*Func)(A1,A2,A3,A4,A5,A6,A7,A8);

	ProxyBind08(){}
	ProxyBind08(Func fn,C *c):func(fn),obj(c){}

	Func    func;
	C*      obj;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		A6      p6;
		A7      p7;
		A8      p8;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		lua::CheckVarFromLua(L,&p6,6);
		lua::CheckVarFromLua(L,&p7,7);
		lua::CheckVarFromLua(L,&p8,8);
		(obj->*func)(p1,p2,p3,p4,p5,p6,p7,p8);
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename C,typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8,typename A9>
struct ProxyBind09 : public ProxyReturn<R>
{
	typedef R (C::*Func)(A1,A2,A3,A4,A5,A6,A7,A8,A9);

	ProxyBind09(){}
	ProxyBind09(Func fn,C *c):func(fn),obj(c){}

	Func    func;
	C*      obj;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		A6      p6;
		A7      p7;
		A8      p8;
		A9      p9;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		lua::CheckVarFromLua(L,&p6,6);
		lua::CheckVarFromLua(L,&p7,7);
		lua::CheckVarFromLua(L,&p8,8);
		lua::CheckVarFromLua(L,&p9,9);
		this->returnValue(L,(obj->*func)(p1,p2,p3,p4,p5,p6,p7,p8,p9));
		return (int)1;
	}
};

template<typename C,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8,typename A9>
struct ProxyBind09<C,void,A1,A2,A3,A4,A5,A6,A7,A8,A9> : public Proxy
{
	typedef void (C::*Func)(A1,A2,A3,A4,A5,A6,A7,A8,A9);

	ProxyBind09(){}
	ProxyBind09(Func fn,C *c):func(fn),obj(c){}

	Func    func;
	C*      obj;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		A6      p6;
		A7      p7;
		A8      p8;
		A9      p9;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		lua::CheckVarFromLua(L,&p6,6);
		lua::CheckVarFromLua(L,&p7,7);
		lua::CheckVarFromLua(L,&p8,8);
		lua::CheckVarFromLua(L,&p9,9);
		(obj->*func)(p1,p2,p3,p4,p5,p6,p7,p8,p9);
		return (int)1;
	}
};

//------------------------------------------------------------

template <typename C,typename R>
static Proxy* GetProxy(R(C::*f)(),C *obj)
{
	return dynamic_cast<Proxy*>(new ProxyBind00<C,R>(f,obj));
}

template <typename C,typename R,typename A1>
static Proxy* GetProxy(R(C::*f)(A1),C *obj)
{
	return dynamic_cast<Proxy*>(new ProxyBind01<C,R,A1>(f,obj));
}

template <typename C,typename R,typename A1,typename A2>
static Proxy* GetProxy(R(C::*f)(A1,A2),C *obj)
{
	return dynamic_cast<Proxy*>(new ProxyBind02<C,R,A1,A2>(f,obj));
}

template <typename C,typename R,typename A1,typename A2,typename A3>
static Proxy* GetProxy(R(C::*f)(A1,A2,A3),C *obj)
{
	return dynamic_cast<Proxy*>(new ProxyBind03<C,R,A1,A2,A3>(f,obj));
}

template <typename C,typename R,typename A1,typename A2,typename A3,typename A4>
static Proxy* GetProxy(R(C::*f)(A1,A2,A3,A4),C *obj)
{
	return dynamic_cast<Proxy*>(new ProxyBind04<C,R,A1,A2,A3,A4>(f,obj));
}

template <typename C,typename R,typename A1,typename A2,typename A3,typename A4,typename A5>
static Proxy* GetProxy(R(C::*f)(A1,A2,A3,A4,A5),C *obj)
{
	return dynamic_cast<Proxy*>(new ProxyBind05<C,R,A1,A2,A3,A4,A5>(f,obj));
}

template <typename C,typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6>
static Proxy* GetProxy(R(C::*f)(A1,A2,A3,A4,A5,A6),C *obj)
{
	return dynamic_cast<Proxy*>(new ProxyBind06<C,R,A1,A2,A3,A4,A5,A6>(f,obj));
}

template <typename C,typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7>
static Proxy* GetProxy(R(C::*f)(A1,A2,A3,A4,A5,A6,A7),C *obj)
{
	return dynamic_cast<Proxy*>(new ProxyBind07<C,R,A1,A2,A3,A4,A5,A6,A7>(f,obj));
}

template <typename C,typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8>
static Proxy* GetProxy(R(C::*f)(A1,A2,A3,A4,A5,A6,A7,A8),C *obj)
{
	return dynamic_cast<Proxy*>(new ProxyBind08<C,R,A1,A2,A3,A4,A5,A6,A7,A8>(f,obj));
}

template <typename C,typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8,typename A9>
static Proxy* GetProxy(R(C::*f)(A1,A2,A3,A4,A5,A6,A7,A8,A9),C *obj)
{
	return dynamic_cast<Proxy*>(new ProxyBind09<C,R,A1,A2,A3,A4,A5,A6,A7,A8,A9>(f,obj));
}

//------------------------------------------------------------

template<typename R>
struct ProxyFunctor00 : public ProxyReturn<R>
{
	typedef std::function<R()> Func;

	ProxyFunctor00(){}
	ProxyFunctor00(Func fn):func00(fn){}

	Func    func00;

	int Do(lua::Handle L)
	{
		this->returnValue(L,func00());
		return (int)1;
	}
};

template<>
struct ProxyFunctor00<void> : public Proxy
{
	typedef void R;
	typedef std::function<R()> Func;

	ProxyFunctor00(){}
	ProxyFunctor00(Func fn):func00(fn){}

	Func    func00;

	int Do(lua::Handle)
	{
		func00();
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename R,typename A1>
struct ProxyFunctor01 : public ProxyReturn<R>
{
	typedef std::function<R(A1)> Func;

	ProxyFunctor01(){}
	ProxyFunctor01(Func fn):func01(fn){}

	Func    func01;

	int Do(lua::Handle L)
	{
		A1      p1;
		lua::CheckVarFromLua(L,&p1,1);
		this->returnValue(L,func01(p1));
		return (int)1;
	}
};

template<typename A1>
struct ProxyFunctor01<void,A1> : public Proxy
{
	typedef std::function<void(A1)> Func;

	ProxyFunctor01(){}
	ProxyFunctor01(Func fn):func01(fn){}

	Func    func01;

	int Do(lua::Handle L)
	{
		A1      p1;
		lua::CheckVarFromLua(L,&p1,1);
		func01(p1);
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename R,typename A1,typename A2>
struct ProxyFunctor02 : public ProxyReturn<R>
{
	typedef std::function<R(A1,A2)> Func;

	ProxyFunctor02(){}
	ProxyFunctor02(Func fn):func02(fn){}

	Func    func02;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		this->returnValue(L,func02(p1,p2));
		return (int)1;
	}
};

template<typename A1,typename A2>
struct ProxyFunctor02<void,A1,A2> : public Proxy
{
	typedef std::function<void(A1,A2)> Func;

	ProxyFunctor02(){}
	ProxyFunctor02(Func fn):func02(fn){}

	Func    func02;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		func02(p1,p2);
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename R,typename A1,typename A2,typename A3>
struct ProxyFunctor03 : public ProxyReturn<R>
{
	typedef std::function<R(A1,A2,A3)> Func;

	ProxyFunctor03(){}
	ProxyFunctor03(Func fn):func03(fn){}

	Func    func03;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		this->returnValue(L,func03(p1,p2,p3));
		return (int)1;
	}
};

template<typename A1,typename A2,typename A3>
struct ProxyFunctor03<void,A1,A2,A3> : public Proxy
{
	typedef std::function<void(A1,A2,A3)> Func;

	ProxyFunctor03(){}
	ProxyFunctor03(Func fn):func03(fn){}

	Func    func03;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		func03(p1,p2,p3);
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename R,typename A1,typename A2,typename A3,typename A4>
struct ProxyFunctor04 : public ProxyReturn<R>
{
	typedef std::function<R(A1,A2,A3,A4)> Func;

	ProxyFunctor04(){}
	ProxyFunctor04(Func fn):func04(fn){}

	Func    func04;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		this->returnValue(L,func04(p1,p2,p3,p4));
		return (int)1;
	}
};

template<typename A1,typename A2,typename A3,typename A4>
struct ProxyFunctor04<void,A1,A2,A3,A4> : public Proxy
{
	typedef std::function<void(A1,A2,A3,A4)> Func;

	ProxyFunctor04(){}
	ProxyFunctor04(Func fn):func04(fn){}

	Func    func04;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		func04(p1,p2,p3,p4);
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5>
struct ProxyFunctor05 : public ProxyReturn<R>
{
	typedef std::function<R(A1,A2,A3,A4,A5)> Func;

	ProxyFunctor05(){}
	ProxyFunctor05(Func fn):func05(fn){}

	Func    func05;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		this->returnValue(L,func05(p1,p2,p3,p4,p5));
		return (int)1;
	}
};

template<typename A1,typename A2,typename A3,typename A4,typename A5>
struct ProxyFunctor05<void,A1,A2,A3,A4,A5> : public Proxy
{
	typedef std::function<void(A1,A2,A3,A4,A5)> Func;

	ProxyFunctor05(){}
	ProxyFunctor05(Func fn):func05(fn){}

	Func    func05;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		func05(p1,p2,p3,p4,p5);
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6>
struct ProxyFunctor06 : public ProxyReturn<R>
{
	typedef std::function<R(A1,A2,A3,A4,A5,A6)> Func;

	ProxyFunctor06(){}
	ProxyFunctor06(Func fn):func06(fn){}

	Func    func06;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		A6      p6;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		lua::CheckVarFromLua(L,&p6,6);
		this->returnValue(L,func06(p1,p2,p3,p4,p5,p6));
		return (int)1;
	}
};

template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6>
struct ProxyFunctor06<void,A1,A2,A3,A4,A5,A6> : public Proxy
{
	typedef std::function<void(A1,A2,A3,A4,A5,A6)> Func;

	ProxyFunctor06(){}
	ProxyFunctor06(Func fn):func06(fn){}

	Func    func06;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		A6      p6;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		lua::CheckVarFromLua(L,&p6,6);
		func06(p1,p2,p3,p4,p5,p6);
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7>
struct ProxyFunctor07 : public ProxyReturn<R>
{
	typedef std::function<R(A1,A2,A3,A4,A5,A6,A7)> Func;

	ProxyFunctor07(){}
	ProxyFunctor07(Func fn):func07(fn){}

	Func    func07;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		A6      p6;
		A7      p7;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		lua::CheckVarFromLua(L,&p6,6);
		lua::CheckVarFromLua(L,&p7,7);
		this->returnValue(L,func07(p1,p2,p3,p4,p5,p6,p7));
		return (int)1;
	}
};

template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7>
struct ProxyFunctor07<void,A1,A2,A3,A4,A5,A6,A7> : public Proxy
{
	typedef std::function<void(A1,A2,A3,A4,A5,A6,A7)> Func;

	ProxyFunctor07(){}
	ProxyFunctor07(Func fn):func07(fn){}

	Func    func07;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		A6      p6;
		A7      p7;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		lua::CheckVarFromLua(L,&p6,6);
		lua::CheckVarFromLua(L,&p7,7);
		func07(p1,p2,p3,p4,p5,p6,p7);
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8>
struct ProxyFunctor08 : public ProxyReturn<R>
{
	typedef std::function<R(A1,A2,A3,A4,A5,A6,A7,A8)> Func;

	ProxyFunctor08(){}
	ProxyFunctor08(Func fn):func08(fn){}

	Func    func08;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		A6      p6;
		A7      p7;
		A8      p8;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		lua::CheckVarFromLua(L,&p6,6);
		lua::CheckVarFromLua(L,&p7,7);
		lua::CheckVarFromLua(L,&p8,8);
		this->returnValue(L,func08(p1,p2,p3,p4,p5,p6,p7,p8));
		return (int)1;
	}
};

template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8>
struct ProxyFunctor08<void,A1,A2,A3,A4,A5,A6,A7,A8> : public Proxy
{
	typedef std::function<void(A1,A2,A3,A4,A5,A6,A7,A8)> Func;

	ProxyFunctor08(){}
	ProxyFunctor08(Func fn):func08(fn){}

	Func    func08;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		A6      p6;
		A7      p7;
		A8      p8;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		lua::CheckVarFromLua(L,&p6,6);
		lua::CheckVarFromLua(L,&p7,7);
		lua::CheckVarFromLua(L,&p8,8);
		func08(p1,p2,p3,p4,p5,p6,p7,p8);
		return (int)1;
	}
};

//------------------------------------------------------------

template<typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8,typename A9>
struct ProxyFunctor09 : public ProxyReturn<R>
{
	typedef std::function<R(A1,A2,A3,A4,A5,A6,A7,A8,A9)> Func;

	ProxyFunctor09(){}
	ProxyFunctor09(Func fn):func09(fn){}

	Func    func09;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		A6      p6;
		A7      p7;
		A8      p8;
		A9      p9;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		lua::CheckVarFromLua(L,&p6,6);
		lua::CheckVarFromLua(L,&p7,7);
		lua::CheckVarFromLua(L,&p8,8);
		lua::CheckVarFromLua(L,&p9,9);
		this->returnValue(L,func09(p1,p2,p3,p4,p5,p6,p7,p8,p9));
		return (int)1;
	}
};

template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8,typename A9>
struct ProxyFunctor09<void,A1,A2,A3,A4,A5,A6,A7,A8,A9> : public Proxy
{
	typedef std::function<void(A1,A2,A3,A4,A5,A6,A7,A8,A9)> Func;

	ProxyFunctor09(){}
	ProxyFunctor09(Func fn):func09(fn){}

	Func    func09;

	int Do(lua::Handle L)
	{
		A1      p1;
		A2      p2;
		A3      p3;
		A4      p4;
		A5      p5;
		A6      p6;
		A7      p7;
		A8      p8;
		A9      p9;
		lua::CheckVarFromLua(L,&p1,1);
		lua::CheckVarFromLua(L,&p2,2);
		lua::CheckVarFromLua(L,&p3,3);
		lua::CheckVarFromLua(L,&p4,4);
		lua::CheckVarFromLua(L,&p5,5);
		lua::CheckVarFromLua(L,&p6,6);
		lua::CheckVarFromLua(L,&p7,7);
		lua::CheckVarFromLua(L,&p8,8);
		lua::CheckVarFromLua(L,&p9,9);
		func09(p1,p2,p3,p4,p5,p6,p7,p8,p9);
		return (int)1;
	}
};

//------------------------------------------------------------

template <typename R>
static Proxy* GetProxy(std::function<R()> f)
{
	return dynamic_cast<Proxy*>(new ProxyFunctor00<R>(f));
}

template <typename R,typename A1>
static Proxy* GetProxy(std::function<R(A1)> f)
{
	return dynamic_cast<Proxy*>(new ProxyFunctor01<R,A1>(f));
}

template <typename R,typename A1,typename A2>
static Proxy* GetProxy(std::function<R(A1,A2)> f)
{
	return dynamic_cast<Proxy*>(new ProxyFunctor02<R,A1,A2>(f));
}

template <typename R,typename A1,typename A2,typename A3>
static Proxy* GetProxy(std::function<R(A1,A2,A3)> f)
{
	return dynamic_cast<Proxy*>(new ProxyFunctor03<R,A1,A2,A3>(f));
}

template <typename R,typename A1,typename A2,typename A3,typename A4>
static Proxy* GetProxy(std::function<R(A1,A2,A3,A4)> f)
{
	return dynamic_cast<Proxy*>(new ProxyFunctor04<R,A1,A2,A3,A4>(f));
}

template <typename R,typename A1,typename A2,typename A3,typename A4,typename A5>
static Proxy* GetProxy(std::function<R(A1,A2,A3,A4,A5)> f)
{
	return dynamic_cast<Proxy*>(new ProxyFunctor05<R,A1,A2,A3,A4,A5>(f));
}

template <typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6>
static Proxy* GetProxy(std::function<R(A1,A2,A3,A4,A5,A6)> f)
{
	return dynamic_cast<Proxy*>(new ProxyFunctor06<R,A1,A2,A3,A4,A5,A6>(f));
}

template <typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7>
static Proxy* GetProxy(std::function<R(A1,A2,A3,A4,A5,A6,A7)> f)
{
	return dynamic_cast<Proxy*>(new ProxyFunctor07<R,A1,A2,A3,A4,A5,A6,A7>(f));
}

template <typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8>
static Proxy* GetProxy(std::function<R(A1,A2,A3,A4,A5,A6,A7,A8)> f)
{
	return dynamic_cast<Proxy*>(new ProxyFunctor08<R,A1,A2,A3,A4,A5,A6,A7,A8>(f));
}

template <typename R,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8,typename A9>
static Proxy* GetProxy(std::function<R(A1,A2,A3,A4,A5,A6,A7,A8,A9)> f)
{
	return dynamic_cast<Proxy*>(new ProxyFunctor09<R,A1,A2,A3,A4,A5,A6,A7,A8,A9>(f));
}

//------------------------------------------------------------

}//namespace wrapper
}//namespace lua
