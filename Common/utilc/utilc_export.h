#ifndef _UTIL_EXPORT_H
#define _UTIL_EXPORT_H

#if defined(_WIN32) || defined(WIN32)        /**Windows*/
    #define WINDOWS_IMPL
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(BSD)    /**Linux*/
    #define LINUX_IMPL
    #if defined(__APPLE__)
        #define APPLE_IMPL
    #endif
#endif

#if defined(WINDOWS_IMPL)
    #ifndef _UTILC_API
        #ifdef UTIL_EXPORT
            #define _UTILC_API		_declspec(dllexport)
        #else
            #define _UTILC_API		extern
        #endif
    #endif
#elif defined(LINUX_IMPL)
    #ifndef _UTILC_API
        #define _UTILC_API       extern
    #endif
#endif

#endif