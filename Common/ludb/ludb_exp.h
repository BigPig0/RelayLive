#ifndef _LUDB_EXPORT_H
#define _LUDB_EXPORT_H

#if ( defined _WIN32 )
#ifndef LUDB_API
#ifdef LUDB_EXPORT
#define LUDB_API		_declspec(dllexport)
#else
#define LUDB_API		extern
#endif
#endif
#elif ( defined __unix ) || ( defined __linux__ )
#ifndef LUDB_API
#define LUDB_API        extern
#endif
#endif

#endif