/**
 * @file      sharedptr.hpp
 * @brief     A fake std::shared_ptr.
 * @author    ToyAuthor
 * @copyright Public Domain
 *
 * http://github.com/ToyAuthor/luapp
 */

#pragma once


#include "luapp/Config.hpp"

#ifdef _LUAPP_CPP11_

#include <memory>

#else

#include "luapp/Log.hpp"

namespace std{


namespace _shared_ptr{

struct RefCount
{
	RefCount():refs(1){}

	~RefCount()
	{
		refs = -1;
	}

	/*
	 * refs < 0 : RefCount is disable.
	 * refs > 0 : Write down how many shared_ptr shared this pointer.
	 * refs = 0 : Just only one pointer hold this memory.
	 */
	int     refs;
};

}


template<typename T>
class shared_ptr
{
	public:

		shared_ptr():_ptr(0),_counter(0)
		{
			;
		}

		shared_ptr(const shared_ptr &model):_ptr(0),_counter(0)
		{
			take_over(const_cast<shared_ptr&>(model));
		}

		explicit shared_ptr(T* ptr):_ptr(ptr),_counter(0)
		{
			#ifdef _LUAPP_CHECK_CAREFUL_
				if( ! ptr )
				{
					lua::Log<<"error:don't input null pointer."<<lua::End;
				}
			#endif
		}

		~shared_ptr()
		{
			releaseRef();
		}

		void operator = (const shared_ptr &model)
		{
			take_over(const_cast<shared_ptr&>(model));
		}

		void operator = (T* ptr)
		{
			releaseRef();
			_ptr = ptr;
		}

		#ifdef _LUAPP_CPP11_
		// I recommand you include real std::shared_ptr.
		void operator = (std::nullptr_t)
		{
			releaseRef();
		}
		#endif

		bool operator == (const shared_ptr &_model)
		{
			shared_ptr &model = const_cast<shared_ptr&>(_model);
			return ( _ptr == model._ptr ) ? 1:0;
		}

		bool operator != (const shared_ptr &_model)
		{
			shared_ptr &model = const_cast<shared_ptr&>(_model);
			return ( _ptr != model._ptr ) ? 1:0;
		}

		T* operator->()
		{
			#ifdef _LUAPP_CHECK_CAREFUL_
				if( ! _ptr )
				{
					lua::Log<<"error:operator->() for null pointer."<<lua::End;
				}
			#endif

			return _ptr;
		}

		T& operator *()
		{
			#ifdef _LUAPP_CHECK_CAREFUL_
				if( ! _ptr )
				{
					lua::Log<<"error:operator *() for null pointer."<<lua::End;
				}
			#endif

			return *_ptr;
		}

		long use_count() const
		{
			if ( this->_counter )
			{
				if ( this->_counter->refs >= 0 )
				{
					return (this->_counter->refs)+1;
				}
			}

			return 0;
		}

		operator bool () const
		{
			return ( _ptr == 0 ) ? false:true;
		}

	private:

		void addRef()
		{
			if(_counter)
			{
				_counter->refs++;
			}
			else
			{
				_counter = new _shared_ptr::RefCount;
			}
		}

		void releaseRef()
		{
			if(_ptr)
			{
				if(_counter)
				{
					if( _counter->refs == 0 )
					{
						delete _ptr;
						delete _counter;
					}
					else
					{
						_counter->refs--;
					}
				}
				else
				{
					delete _ptr;
				}

				_ptr = 0;
				_counter = 0;
			}
		}

		void take_over(shared_ptr &model)
		{
			#ifdef _LUAPP_CHECK_CAREFUL_
				if(!model._ptr)
				{
					if( model._counter )
					{
						lua::Log<<"error:copy a break pointer."<<lua::End;
					}
					else
					{
						lua::Log<<"warning:Are you sure? Overwrite by a null shared_ptr?."<<lua::End;
					}
				}
			#endif

			releaseRef();

			model.addRef();

			_ptr     = model._ptr;
			_counter = model._counter;
		}

		T*                      _ptr;
		_shared_ptr::RefCount*  _counter;  // New it when some one share this memory. Release it when last one pointer destroy.
};



template<typename T>
inline ::std::shared_ptr<T> make_shared()
{
	return ::std::shared_ptr<T>(new T);
}

template<typename T,typename A1>
inline ::std::shared_ptr<T> make_shared(A1 a1)
{
	return ::std::shared_ptr<T>(new T(a1));
}

template<typename T,typename A1,typename A2>
inline ::std::shared_ptr<T> make_shared(A1 a1,A2 a2)
{
	return ::std::shared_ptr<T>(new T(a1,a2));
}

template<typename T,typename A1,typename A2,typename A3>
inline ::std::shared_ptr<T> make_shared(A1 a1,A2 a2,A3 a3)
{
	return ::std::shared_ptr<T>(new T(a1,a2,a3));
}

template<typename T,typename A1,typename A2,typename A3,typename A4>
inline ::std::shared_ptr<T> make_shared(A1 a1,A2 a2,A3 a3,A4 a4)
{
	return ::std::shared_ptr<T>(new T(a1,a2,a3,a4));
}

template<typename T,typename A1,typename A2,typename A3,typename A4,typename A5>
inline ::std::shared_ptr<T> make_shared(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5)
{
	return ::std::shared_ptr<T>(new T(a1,a2,a3,a4,a5));
}

template<typename T,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6>
inline ::std::shared_ptr<T> make_shared(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5,A6 a6)
{
	return ::std::shared_ptr<T>(new T(a1,a2,a3,a4,a5,a6));
}

template<typename T,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7>
inline ::std::shared_ptr<T> make_shared(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5,A6 a6,A7 a7)
{
	return ::std::shared_ptr<T>(new T(a1,a2,a3,a4,a5,a6,a7));
}

template<typename T,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8>
inline ::std::shared_ptr<T> make_shared(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5,A6 a6,A7 a7,A8 a8)
{
	return ::std::shared_ptr<T>(new T(a1,a2,a3,a4,a5,a6,a7,a8));
}

template<typename T,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8,typename A9>
inline ::std::shared_ptr<T> make_shared(A1 a1,A2 a2,A3 a3,A4 a4,A5 a5,A6 a6,A7 a7,A8 a8,A9 a9)
{
	return ::std::shared_ptr<T>(new T(a1,a2,a3,a4,a5,a6,a7,a8,a9));
}

}//namespace std

#endif
