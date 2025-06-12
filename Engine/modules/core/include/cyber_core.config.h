#pragma once

#ifdef __cplusplus
    #define CYBER_CORE_EXTERN_C extern "C"
#else
    #define CYBER_CORE_EXTERN_C
#endif

#ifndef CYBER_CORE_EXPORT
    #if defined (_MSC_VER)
        #define CYBER_CORE_EXPORT __declspec(dllexport)
    #else
        #define CYBER_CORE_EXPORT
    #endif
#endif

#ifndef CYBER_CORE_IMPORT
    #if defined (_MSC_VER)
        #define CYBER_CORE_IMPORT __declspec(dllimport)
    #else
        #define CYBER_CORE_IMPORT
    #endif
#endif

#if defined (CYBER_API_EXPORT)
    #define CYBER_CORE_API CYBER_CORE_EXPORT
#else
    #define CYBER_CORE_API CYBER_CORE_IMPORT
#endif