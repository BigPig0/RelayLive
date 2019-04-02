/**
 * @file   Log.hpp
 * @brief  A wrapper for printing message.
 */

#pragma once

#include <cstdio>
#include <string>

namespace lua{
namespace log{


class Printer
{
	public:

		Printer(){}
		~Printer(){}

		const Printer& operator << (const int num) const
		{
			std::printf("%d",num);
			return *this;
		}

		const Printer& operator << (const long int num) const
		{
			std::printf("%ld",num);
			return *this;
		}

		const Printer& operator << (const long long num) const
		{
			#ifdef _WIN64
			std::printf("%lld",num);
			#elif defined(_WIN32)
			std::printf("%I64d",num);
			#else
			std::printf("%lld",num);
			#endif
			return *this;
		}

		const Printer& operator << (const float num) const
		{
			std::printf("%f",num);
			return *this;
		}

		const Printer& operator << (const double num) const
		{
			std::printf("%f",num);
			return *this;
		}

		const Printer& operator << (std::string str) const
		{
			std::printf("%s",str.c_str());
			return *this;
		}

		const Printer& operator << (std::wstring str) const
		{
			std::wprintf(L"%ls",str.c_str());
			return *this;
		}

		const Printer& operator << (std::string *str) const
		{
			std::printf("%s",str->c_str());
			return *this;
		}

		const Printer& operator << (std::wstring *str) const
		{
			std::wprintf(L"%ls",str->c_str());
			return *this;
		}

		const Printer& operator << (const Printer& (*func)(const Printer&)) const
		{
			return func(*this);
		}
};

}//namespace log

inline const ::lua::log::Printer& End(const ::lua::log::Printer& m)
{
	std::printf(" - luapp message\n");
	return m;
}

static ::lua::log::Printer      Log;

}//namespace lua
