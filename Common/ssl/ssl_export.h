#pragma once

#if ( defined _WIN32 )
#ifndef _SSL_API
#ifdef SSL_EXPORT
#define _SSL_API		_declspec(dllexport)
#else
#define _SSL_API
#endif
#endif
#elif ( defined __unix ) || ( defined __linux__ )
#ifndef _SSL_API
#define _SSL_API
#endif
#endif