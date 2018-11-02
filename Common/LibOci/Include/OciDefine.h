#pragma once

#ifdef LIBOCI_EXPORTS
#define LIBOCI_API __declspec(dllexport)
#else
#define LIBOCI_API
#endif // LIBOCI_EXPORTS

#define SQLT_CHR 1 
#define SQLT_INT 3 
#define SQLT_FLT 4 
#define SQLT_LNG 8 
#define SQLT_UIN 68 
#define SQLT_BLOB 113 
#define SQLT_ODT 156 

