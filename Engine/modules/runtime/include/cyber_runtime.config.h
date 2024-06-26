#pragma once

#ifdef __cplusplus
    #define CYBER_RUNTIME_EXTERN_C extern "C"
#else
    #define CYBERCYBER_RUNTIME_EXTERN_C
#endif

#ifndef CYBER_RUNTIME_EXPORT
    #if defined (_MSC_VER)
        #define CYBER_RUNTIME_EXPORT __declspec(dllexport)
    #else
        #define CYBER_RUNTIME_EXPORT
    #endif
#endif

#ifndef CYBER_RUNTIME_IMPORT
    #if defined (_MSC_VER)
        #define CYBER_RUNTIME_IMPORT __declspec(dllimport)
    #else
        #define CYBER_RUNTIME_IMPORT
    #endif
#endif

#if defined (CYBER_API_EXPORT)
    #define CYBER_RUNTIME_API CYBER_RUNTIME_EXPORT
#else
    #define CYBER_RUNTIME_API CYBER_RUNTIME_IMPORT
#endif