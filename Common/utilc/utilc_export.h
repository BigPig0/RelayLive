#ifndef _UTIL_EXPORT_H
#define _UTIL_EXPORT_H

#if ( defined _WIN32 )
#ifndef _UTIL_API
#ifdef UTIL_EXPORT
#define _UTIL_API		_declspec(dllexport)
#else
#define _UTIL_API		extern
#endif
#endif
#elif ( defined __unix ) || ( defined __linux__ )
#ifndef _UTIL_API
#define _UTIL_API        extern
#endif
#endif

#endif