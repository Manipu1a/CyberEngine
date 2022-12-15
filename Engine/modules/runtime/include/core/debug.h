#pragma once

#ifdef CB_DEBUG
    #define CB_ENABLE_ASSERTS
#endif

#ifdef CB_ENABLE_ASSERTS
    #define CB_CHECK(x) if(!(x)) {__debugbreak();}
    #define CB_ASSERTS(x, ...) if(!(x)) {CB_ERROR("Assertion Failed: {0}", __VA_ARGS__);  __debugbreak();}
    #define CB_CORE_ASSERTS(x, ...) { if(!(x)) {CB_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__);  __debugbreak();}}
#else
    #define CB_CHECK(x)
    #define CB_ASSERTS(x, ...)
    #define CB_CORE_ASSERTS(x, ...)
#endif

#if defined(_WINDOWS)
#define CHECK_HRESULT(exp)                                                              \
    do                                                                                  \
    {                                                                                   \
        HRESULT hres = (exp);                                                           \
        if(!SUCCEEDED(hres))                                                            \
        {                                                                               \
            CB_ERROR("[0]: FAILED with HRESULT: [1]", #exp, (uint32_t)hres);            \
            assert(false);                                                              \
        }                                                                               \
    } while (0)
#endif