/**
 * @file      Environment.hpp
 * @brief     A header file to detect the environment information.
 * @author    ToyAuthor
 * @copyright Public Domain
 * <pre>
 * https://github.com/ToyAuthor/ToyBox
 * https://bitbucket.org/ToyAuthor/toybox
 * </pre>
 */


#pragma once

#ifndef _TOY_ENVIRONMENT_HPP_
#define _TOY_ENVIRONMENT_HPP_


//------------------------------Operating system------------------------------start


#if defined(_WIN64)

	#define TOY_WIN64
	#define TOY_WINDOWS

#elif defined(_WIN32)

	#define TOY_WIN32
	#define TOY_WINDOWS

#elif defined(__ANDROID__)

	#define TOY_ANDROID

	// I don't know how to detect this. Please choose it by yourself.
	#define TOY_ANDR32
	//#define TOY_ANDR64

#elif __APPLE__

	#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR || TARGET_OS_EMBEDDED

		#error "Sorry, not support iPhone!"

	#elif TARGET_OS_MAC || __MACH__

		#define TOY_MAC

		#if defined(__i386__)
			#define TOY_MAC32
			#error "Sorry, support Mac OS 64bit only."
		#else
			#define TOY_MAC64
		#endif

	#else
		#define TOY_UNKNOWN_PLATFORM
	#endif

#elif defined(__linux__)

	#define TOY_LINUX

	#if defined(__i386__)
		#define TOY_LIN32
	#elif defined(__aarch64__)
		#define TOY_LIN64
	#elif defined(__arm__)
		#define TOY_LIN32
	#else
		#define TOY_LIN64
	#endif

#else

	#define TOY_UNKNOWN_PLATFORM

#endif


#if defined(TOY_LIN32) || defined(TOY_WIN32) || defined(TOY_ANDR32) || defined(TOY_MAC32)
	#define TOY_32_BIT
#else
	#define TOY_64_BIT
#endif

#if defined(TOY_UNKNOWN_PLATFORM)
	#error "This environment maybe not support!"
#endif

//------------------------------Operating system------------------------------end




//-------------------------Compiler-------------------------start

#if defined(TOY_WINDOWS)

	#if defined(_MSC_VER)
		#define TOY_MSVC        // Visual Studio.
	#elif defined(__MINGW32__)
		#define TOY_MINGW       // MinGW or TDM-GCC.
	#else
		#define TOY_UNKNOWN_COMPILER
	#endif

#elif defined(TOY_LINUX)

	#if defined(__GNUC__)
		#define TOY_GCC
	#else
		#define TOY_UNKNOWN_COMPILER
	#endif

#elif defined(TOY_ANDROID)

	#define TOY_NDK

#elif defined(TOY_MAC)

	#if defined(__GNUC__)
		#define TOY_GCC
	#elif defined(__llvm__)
		#define TOY_LLVM
	#else
		#define TOY_UNKNOWN_COMPILER
	#endif

#else
	#define TOY_UNKNOWN_COMPILER
#endif

#if defined(TOY_UNKNOWN_COMPILER)
	#error "This compiler maybe not support!"
#endif

//-------------------------Compiler-------------------------end


//------------------Visual Studio------------------start

#if defined(TOY_MSVC)

	#if (_MSC_VER == 1910)
		#define TOY_VC_2017
	#elif (_MSC_VER == 1900)
		#define TOY_VC_2015
	#elif (_MSC_VER == 1800)
		#define TOY_VC_2013
	#elif (_MSC_VER == 1700)
		#define TOY_VC_2012
	#elif (_MSC_VER == 1600)
		#define TOY_VC_2010
	#elif (_MSC_VER == 1500)
		#define TOY_VC_2008
	#elif (_MSC_VER == 1400)
		#define TOY_VC_2005
	#elif (_MSC_VER == 1310)
		#define TOY_VC_2003
	#elif (_MSC_VER == 1200)
		#define TOY_VC_6
	#elif (_MSC_VER <  1200)
		#error "Too old"
	#else
		#error "Can't detect the version of Visual Studio"
	#endif

#endif

//------------------Visual Studio------------------end

#endif//_TOY_ENVIRONMENT_HPP_
