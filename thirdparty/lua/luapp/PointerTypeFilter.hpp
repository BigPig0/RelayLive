
#pragma once

namespace lua{

struct VoidPointerType{};

template<typename T>
struct PointerTypeFilter{};

template<>
struct PointerTypeFilter<void*>
{
	typedef struct ::lua::VoidPointerType    RawType;
	typedef void ClassType;
	typedef void ReturnType;
};

template<typename T>
struct PointerTypeFilter<T*>
{
	typedef T    RawType;
	typedef void ClassType;
	typedef void ReturnType;
};

template<typename R>
struct PointerTypeFilter<R(*)(void)>
{
	typedef void RawType;
	typedef void ClassType;
	typedef R    ReturnType;
};

template<typename R,typename P1>
struct PointerTypeFilter<R(*)(P1)>
{
	typedef void RawType;
	typedef void ClassType;
	typedef R    ReturnType;
};

template<typename R,typename P1,typename P2>
struct PointerTypeFilter<R(*)(P1,P2)>
{
	typedef void RawType;
	typedef void ClassType;
	typedef R    ReturnType;
};

template<typename R,typename P1,typename P2,typename P3>
struct PointerTypeFilter<R(*)(P1,P2,P3)>
{
	typedef void RawType;
	typedef void ClassType;
	typedef R    ReturnType;
};

template<typename R,typename P1,typename P2,typename P3,typename P4>
struct PointerTypeFilter<R(*)(P1,P2,P3,P4)>
{
	typedef void RawType;
	typedef void ClassType;
	typedef R    ReturnType;
};

template<typename R,typename P1,typename P2,typename P3,typename P4,typename P5>
struct PointerTypeFilter<R(*)(P1,P2,P3,P4,P5)>
{
	typedef void RawType;
	typedef void ClassType;
	typedef R    ReturnType;
};

template<typename R,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6>
struct PointerTypeFilter<R(*)(P1,P2,P3,P4,P5,P6)>
{
	typedef void RawType;
	typedef void ClassType;
	typedef R    ReturnType;
};

template<typename R,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7>
struct PointerTypeFilter<R(*)(P1,P2,P3,P4,P5,P6,P7)>
{
	typedef void RawType;
	typedef void ClassType;
	typedef R    ReturnType;
};

template<typename R,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7,typename P8>
struct PointerTypeFilter<R(*)(P1,P2,P3,P4,P5,P6,P7,P8)>
{
	typedef void RawType;
	typedef void ClassType;
	typedef R    ReturnType;
};

template<typename R,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7,typename P8,typename P9>
struct PointerTypeFilter<R(*)(P1,P2,P3,P4,P5,P6,P7,P8,P9)>
{
	typedef void RawType;
	typedef void ClassType;
	typedef R    ReturnType;
};

template<typename R,typename C>
struct PointerTypeFilter<R (C::*)()>
{
	typedef void RawType;
	typedef C    ClassType;
	typedef R    ReturnType;
};

template<typename R,typename C,typename P1>
struct PointerTypeFilter<R (C::*)(P1)>
{
	typedef void RawType;
	typedef C    ClassType;
	typedef R    ReturnType;
};

template<typename R,typename C,typename P1,typename P2>
struct PointerTypeFilter<R (C::*)(P1,P2)>
{
	typedef void RawType;
	typedef C    ClassType;
	typedef R    ReturnType;
};

template<typename R,typename C,typename P1,typename P2,typename P3>
struct PointerTypeFilter<R (C::*)(P1,P2,P3)>
{
	typedef void RawType;
	typedef C    ClassType;
	typedef R    ReturnType;
};

template<typename R,typename C,typename P1,typename P2,typename P3,typename P4>
struct PointerTypeFilter<R (C::*)(P1,P2,P3,P4)>
{
	typedef void RawType;
	typedef C    ClassType;
	typedef R    ReturnType;
};

template<typename R,typename C,typename P1,typename P2,typename P3,typename P4,typename P5>
struct PointerTypeFilter<R (C::*)(P1,P2,P3,P4,P5)>
{
	typedef void RawType;
	typedef C    ClassType;
	typedef R    ReturnType;
};

template<typename R,typename C,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6>
struct PointerTypeFilter<R (C::*)(P1,P2,P3,P4,P5,P6)>
{
	typedef void RawType;
	typedef C    ClassType;
	typedef R    ReturnType;
};

template<typename R,typename C,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7>
struct PointerTypeFilter<R (C::*)(P1,P2,P3,P4,P5,P6,P7)>
{
	typedef void RawType;
	typedef C    ClassType;
	typedef R    ReturnType;
};

template<typename R,typename C,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7,typename P8>
struct PointerTypeFilter<R (C::*)(P1,P2,P3,P4,P5,P6,P7,P8)>
{
	typedef void RawType;
	typedef C    ClassType;
	typedef R    ReturnType;
};

template<typename R,typename C,typename P1,typename P2,typename P3,typename P4,typename P5,typename P6,typename P7,typename P8,typename P9>
struct PointerTypeFilter<R (C::*)(P1,P2,P3,P4,P5,P6,P7,P8,P9)>
{
	typedef void RawType;
	typedef C    ClassType;
	typedef R    ReturnType;
};

}
