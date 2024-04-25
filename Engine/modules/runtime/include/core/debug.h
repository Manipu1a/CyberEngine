#pragma once
#include "CyberLog/Log.h"

#define CB_DEBUG

#ifdef CB_DEBUG
    #define CB_ENABLE_ASSERTS
#endif

#ifdef CB_ENABLE_ASSERTS
    #define cyber_check(x) if(!(x)) {__debugbreak();}
    #define cyber_assert(x, ...) if(!(x)) {CB_ERROR("Assertion Failed: {0}", __VA_ARGS__);  __debugbreak();}
    #define cyber_warn(x, ...) CB_WARN("Warn: {0}", __VA_ARGS__)
    #define cyber_core_assert(x, ...) { if(!(x)) {CB_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__);  __debugbreak();}}
#else
    #define cyber_check(x)
    #define cyber_assert(x, ...)
    #define cyber_warn(x, ...)
    #define cyber_core_assert(x, ...)
#endif

#if defined(_WINDOWS)
#define CHECK_HRESULT(exp)                                                              \
    do                                                                                  \
    {                                                                                   \
        HRESULT hres = (exp);                                                           \
        if(!SUCCEEDED(hres))                                                            \
        {                                                                               \
            cyber_assert(false, "{0}: FAILED with HRESULT: {1}", 15, (uint32_t)hres); \
        }                                                                               \
    } while (0)
#endif