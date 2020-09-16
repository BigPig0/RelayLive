#pragma once

#ifdef UVLOG_EXPORTS
#define UVLOG_API __declspec(dllexport)
#else
#define UVLOG_API
#endif

namespace uvLogPlus {
    enum class Level {
        All = 0,
        Trace,
        Debug,
        Info,
        Warn,
        Error,
        Fatal,
        OFF
    };

};